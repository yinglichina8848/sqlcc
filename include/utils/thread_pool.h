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
 * @brief 线程池实现 - 支持任务提交和异步执行
 *
 * 提供高效的任务调度和执行，支持：
 * - 动态任务提交
 * - 结果获取（future）
 * - 线程安全
 * - 优雅关闭
 */
class ThreadPool {
public:
    /**
     * @brief 构造函数
     * @param thread_count 线程数量，默认为硬件并发数
     */
    explicit ThreadPool(size_t thread_count = std::thread::hardware_concurrency());

    /**
     * @brief 析构函数
     * 等待所有任务完成并关闭线程池
     */
    ~ThreadPool();

    // 禁用拷贝
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    /**
     * @brief 提交任务到线程池
     * @tparam F 函数类型
     * @tparam Args 参数类型
     * @param f 要执行的函数
     * @param args 函数参数
     * @return std::future 异步结果
     */
    template<class F, class... Args>
    auto submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;

    /**
     * @brief 获取线程池状态
     * @return 当前活跃线程数
     */
    size_t active_threads() const;

    /**
     * @brief 获取等待任务数
     * @return 队列中等待的任务数
     */
    size_t queued_tasks() const;

    /**
     * @brief 等待所有任务完成
     */
    void wait();

    /**
     * @brief 关闭线程池
     * 不再接受新任务，等待现有任务完成
     */
    void shutdown();

    /**
     * @brief 强制关闭线程池
     * 立即停止所有任务
     */
    void shutdown_now();

private:
    /**
     * @brief 工作线程函数
     */
    void worker_thread();

    // 任务类型定义
    using Task = std::function<void()>;

    // 线程集合
    std::vector<std::thread> workers_;

    // 任务队列
    std::queue<Task> tasks_;

    // 同步原语
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::condition_variable completion_condition_;

    // 控制变量
    std::atomic<bool> stop_;
    std::atomic<size_t> active_task_count_;
    std::atomic<size_t> total_task_count_;
};

// 模板函数实现
template<class F, class... Args>
auto ThreadPool::submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
    using return_type = typename std::invoke_result<F, Args...>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> result = task->get_future();

    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (stop_) {
            throw std::runtime_error("ThreadPool is stopped");
        }

        tasks_.emplace([task]() { (*task)(); });
        total_task_count_++;
    }

    condition_.notify_one();
    return result;
}

} // namespace utils
} // namespace sqlcc