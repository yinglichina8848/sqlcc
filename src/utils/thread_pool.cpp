#include "thread_pool.h"
#include <iostream>
#include <algorithm>

namespace sqlcc {
namespace utils {

/**
 * @class ThreadPool
 * @brief 通用工作线程池 - 实现多核并发处理和任务调度
 *
 * WHY层 - 设计意图：
 *   在高负载数据库操作中，频繁地创建和销毁线程会带来显著的系统开销（Context Switch, Stack Allocation）。
 *   线程池通过维护一组常驻线程，实现了线程资源的重用，
 *   并通过任务队列（Task Queue）平滑处理突发流量，防止系统过载。
 *
 * WHAT层 - 功能说明：
 *   提供任务提交接口（enqueue），支持异步执行 lambda 或 std::function。
 *   支持优雅关闭（shutdown）和立即强制关闭（shutdown_now）。
 *   支持同步等待（wait），直到所有提交的任务执行完毕。
 *   提供活跃线程数和队列任务数的实时统计。
 *
 * HOW层 - 实现机制：
 *   1. 生产者-消费者模式：主线程提交任务，工作线程从 tasks_ 队列中竞争任务。
 *   2. 同步原语：使用 std::mutex 保护任务队列，std::condition_variable 实现线程的阻塞等待与唤醒。
 *   3. 原子计数：active_task_count_ 使用 std::atomic 确保在无锁情况下统计执行状态。
 *   4. 任务隔离：使用 try-catch 块包裹任务执行过程，确保单个任务崩溃不会导致整个线程池死锁或终止。
 */
ThreadPool::ThreadPool(size_t thread_count) : stop_(false), active_task_count_(0), total_task_count_(0) {
    if (thread_count == 0) {
        thread_count = 1; // 至少一个线程
    }

    // 创建工作线程
    workers_.reserve(thread_count);
    for (size_t i = 0; i < thread_count; ++i) {
        workers_.emplace_back([this]() { worker_thread(); });
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::worker_thread() {
    while (true) {
        Task task;

        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.wait(lock, [this]() {
                return stop_ || !tasks_.empty();
            });

            if (stop_ && tasks_.empty()) {
                return;
            }

            if (!tasks_.empty()) {
                task = std::move(tasks_.front());
                tasks_.pop();
                active_task_count_++;
            }
        }

        if (task) {
            try {
                task();
            } catch (const std::exception& e) {
                std::cerr << "Exception in thread pool task: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Unknown exception in thread pool task" << std::endl;
            }

            active_task_count_--;
        }

        // 检查是否所有任务都已完成
        if (active_task_count_.load() == 0 && tasks_.empty() && total_task_count_.load() > 0) {
            completion_condition_.notify_all();
        }
    }
}

size_t ThreadPool::active_threads() const {
    return active_task_count_.load();
}

size_t ThreadPool::queued_tasks() const {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    return tasks_.size();
}

void ThreadPool::wait() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    completion_condition_.wait(lock, [this]() {
        return tasks_.empty() && active_task_count_.load() == 0;
    });
}

void ThreadPool::shutdown() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    condition_.notify_all();

    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::shutdown_now() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
        // 清空任务队列
        while (!tasks_.empty()) {
            tasks_.pop();
        }
    }
    condition_.notify_all();

    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

} // namespace utils
} // namespace sqlcc
