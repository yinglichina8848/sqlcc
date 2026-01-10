#include "utils/thread_pool.h"
#include <iostream>
#include <algorithm>

namespace sqlcc {
namespace utils {

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