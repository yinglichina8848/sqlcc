/**
 * @file thread_pool.h
 * @brief SQLCC通用线程池 - 高并发任务调度引擎
 *
 * 线程池是数据库并发模型的核心。它通过预分配线程资源并复用工作线程，
 * 避免了频繁创建/销毁线程的系统调用开销。它驱动着数据库的网络连接处理、
 * 异步I/O、后台刷盘、以及并行查询执行。
 *
 * 📚 配套教材参考：
 * - [第11章：数据库并发控制](../../textbook/《数据库系统原理与开发实践》.md#第十一章并发控制)
 * - [11.1 多线程执行模型](../../textbook/《数据库系统原理_与开发实践》.md#111-多线程执行模型)
 * - [11.5 任务调度与负载均衡](../../textbook/《数据库系统原理与开发实践》.md#115-任务调度与负载均衡)
 *
 * WHY层 - 设计意图：
 *   1. **资源复用**：减少线程上下文切换和内核对象分配的开销。
 *   2. **流量削峰**：通过有限的线程数和无限的任务队列，平滑突发的高并发请求。
 *   3. **解耦任务与执行**：允许执行器将复杂任务拆分并异步提交，提高系统响应速度。
 *   4. **优雅降级**：当系统负载过高时，线程池提供统一的任务拒绝和排队机制。
 *
 * WHAT层 - 功能说明：
 *   - 动态任务提交：支持 Lambda、函数对象、成员函数等各种可调用对象的异步提交。
 *   - 异步结果获取：集成 `std::future`，支持非阻塞地获取任务返回值。
 *   - 生命周期管理：支持优雅关闭（Wait for tasks）和强制关闭（Shutdown now）。
 *   - 状态监控：实时反馈活跃线程数和队列任务深度。
 *
 * HOW层 - 实现机制：
 *   - 生产者-消费者模型：主线程（或调用方）作为生产者提交任务，内部工作线程作为消费者竞争执行。
 *   - 同步原语：使用 `std::mutex` 保护任务队列，`std::condition_variable` 实现线程的唤醒与睡眠（Wait/Notify）。
 *   - 任务封装：利用 `std::packaged_task` 将各种签名函数统一封装为 `std::function<void()>` 存储在队列中。
 *   - 原子计数：采用 `std::atomic` 维护任务计数，确保监控数据的准确性。
 *
 * 应用场景：
 * - **网络层**：每接入一个 Socket 连接，提交一个 handle_session 任务。
 * - **存储引擎**：后台 Checkpoint 线程提交脏页刷新任务。
 * - **查询执行**：Hash Join 或 并行 Scan 时，将分片扫描任务下发。
 *
 * @author SQLCC技术委员会
 * @version 1.2.6
 * @date 2026-02-02
 */

#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <memory>
#include <stdexcept>

namespace sqlcc {
namespace utils {

/**
 * @class ThreadPool
 * @brief 高性能线程池实现 - 现代 C++ 异步执行框架
 */
class ThreadPool {
public:
    /**
     * @brief 构造函数
     * @param thread_count 线程池大小。默认为硬件核心数，建议针对 I/O 密集型任务适当增加。
     */
    explicit ThreadPool(size_t thread_count = std::thread::hardware_concurrency());

    /**
     * @brief 析构函数 - 默认执行优雅关闭逻辑
     */
    ~ThreadPool();

    // 数据库组件通常作为单例或由管理类持有，禁止随意拷贝
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    /**
     * @brief 提交异步任务
     * 
     * WHY: 允许调用方提交任务后立即返回，稍后通过 future 获取结果。
     * HOW: 使用完美转发（Perfect Forwarding）接收参数。
     * 
     * @tparam F 可调用对象类型
     * @tparam Args 参数包
     * @param f 函数或 Lambda
     * @param args 函数参数
     * @return std::future<ResultType> 用于获取异步返回值的 Future 对象
     */
    template<class F, class... Args>
    auto submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;

    /**
     * @brief 获取正在执行任务的线程数
     */
    size_t active_threads() const;

    /**
     * @brief 获取当前任务队列深度
     * 队列过长通常意味着系统处理能力遇到瓶颈。
     */
    size_t queued_tasks() const;

    /**
     * @brief 阻塞等待所有已提交任务执行完毕
     */
    void wait();

    /**
     * @brief 优雅关闭
     * 不再接受新任务，直到队列中所有剩余任务处理完成后，销毁所有工作线程。
     */
    void shutdown();

    /**
     * @brief 强制关闭
     * 立即通知所有线程停止工作，未完成的任务将被丢弃。
     */
    void shutdown_now();

private:
    /**
     * @brief 工作线程主循环
     * 采用抢占式获取任务模式。
     */
    void worker_thread();

    using Task = std::function<void()>;

    std::vector<std::thread> workers_;
    std::queue<Task> tasks_;

    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::condition_variable completion_condition_;

    std::atomic<bool> stop_;
    std::atomic<size_t> active_task_count_;
    std::atomic<size_t> total_task_count_;
};

/**
 * 模板函数实现 - 必须位于头文件
 */
template<class F, class... Args>
auto ThreadPool::submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
    using return_type = typename std::invoke_result<F, Args...>::type;

    // 将函数绑定为 packaged_task，以便能够通过 future 获取结果
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> result = task->get_future();

    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (stop_) {
            throw std::runtime_error("ThreadPool is stopped - cannot submit new task");
        }

        // 封装为 void 任务存入队列
        tasks_.emplace([task]() { (*task)(); });
        total_task_count_++;
    }

    // 唤醒一个正在等待任务的工作线程
    condition_.notify_one();
    return result;
}

} // namespace utils
} // namespace sqlcc