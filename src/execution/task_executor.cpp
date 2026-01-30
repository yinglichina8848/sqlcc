#include "task_executor.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <ctime>

namespace sqlcc {
namespace execution {

// TaskResult implementation
TaskResult::TaskResult(const std::string& task_id) 
    : task_id_(task_id), success_(false), execution_time_(0), timestamp_(std::chrono::high_resolution_clock::now()) {
}

// Task implementation
Task::Task(const std::string& task_id, TaskType type, int priority)
    : task_id_(task_id), task_type_(type), priority_(priority), completed_(false), 
      created_at_(std::chrono::high_resolution_clock::now()) {
}

// NetworkTask implementation
NetworkTask::NetworkTask(const std::string& task_id, const std::string& request_data)
    : Task(task_id, TaskType::NETWORK_IO), request_data_(request_data) {
}

std::shared_ptr<TaskResult> NetworkTask::execute() {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 模拟网络任务处理
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    auto result = std::make_shared<TaskResult>(getTaskId());
    
    // 模拟处理结果
    std::stringstream ss;
    ss << "Processed network request: " << request_data_;
    result->setResultData(ss.str());
    result->setSuccess(true);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    result->setExecutionTime(duration);
    
    setResult(result);
    return result;
}

// SQLTask implementation
SQLTask::SQLTask(const std::string& task_id, const std::string& sql_statement)
    : Task(task_id, TaskType::SQL_EXECUTE), sql_statement_(sql_statement), transaction_id_(0) {
}

std::shared_ptr<TaskResult> SQLTask::execute() {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 模拟SQL执行
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    auto result = std::make_shared<TaskResult>(getTaskId());
    
    // 模拟执行结果
    std::stringstream ss;
    ss << "Executed SQL: " << sql_statement_ << " in transaction " << transaction_id_;
    result->setResultData(ss.str());
    result->setSuccess(true);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    result->setExecutionTime(duration);
    
    setResult(result);
    return result;
}

// WALTask implementation
WALTask::WALTask(const std::string& task_id, const std::string& log_data)
    : Task(task_id, TaskType::WAL_WRITE), log_data_(log_data), flush_required_(false) {
}

std::shared_ptr<TaskResult> WALTask::execute() {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 模拟WAL日志处理
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    
    auto result = std::make_shared<TaskResult>(getTaskId());
    
    // 模拟处理结果
    std::stringstream ss;
    ss << "Written to WAL log: " << log_data_;
    if (flush_required_) {
        ss << " (flushed to disk)";
    }
    result->setResultData(ss.str());
    result->setSuccess(true);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    result->setExecutionTime(duration);
    
    setResult(result);
    return result;
}

// TransactionTask implementation
TransactionTask::TransactionTask(const std::string& task_id, uint64_t transaction_id, Operation op)
    : Task(task_id, TaskType::TRANSACTION), transaction_id_(transaction_id), operation_(op) {
}

std::shared_ptr<TaskResult> TransactionTask::execute() {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 模拟事务处理
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    
    auto result = std::make_shared<TaskResult>(getTaskId());
    
    // 模拟处理结果
    std::stringstream ss;
    ss << "Transaction " << transaction_id_ << " ";
    switch (operation_) {
        case BEGIN:
            ss << "started";
            break;
        case COMMIT:
            ss << "committed";
            break;
        case ROLLBACK:
            ss << "rolled back";
            break;
    }
    result->setResultData(ss.str());
    result->setSuccess(true);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    result->setExecutionTime(duration);
    
    setResult(result);
    return result;
}

// TaskQueue implementation
TaskQueue::TaskQueue(size_t max_size)
    : max_size_(max_size) {
}

bool TaskQueue::push(std::unique_ptr<Task> task) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.size() >= max_size_) {
        return false; // 队列已满
    }
    queue_.push(std::move(task));
    condition_.notify_one();
    return true;
}

std::unique_ptr<Task> TaskQueue::pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [this] { return !queue_.empty(); });
    
    auto task = std::move(queue_.front());
    queue_.pop();
    return task;
}

size_t TaskQueue::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

bool TaskQueue::isEmpty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

// ThreadPool implementation
ThreadPool::ThreadPool(size_t num_threads)
    : stop_(false), active_threads_(0) {
    for (size_t i = 0; i < num_threads; ++i) {
        threads_.emplace_back([this] { workerThread(); });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        stop_.store(true);
    }
    condition_.notify_all();
    
    for (std::thread& worker : threads_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::execute(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (stop_.load()) {
            throw std::runtime_error("ThreadPool is stopped");
        }
        tasks_.emplace(task);
    }
    condition_.notify_one();
}

void ThreadPool::resize(size_t num_threads) {
    if (num_threads == threads_.size()) {
        return;
    }
    
    if (num_threads > threads_.size()) {
        // 增加线程数
        for (size_t i = threads_.size(); i < num_threads; ++i) {
            threads_.emplace_back([this] { workerThread(); });
        }
    } else {
        // 减少线程数
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            stop_.store(true);
        }
        condition_.notify_all();
        
        for (size_t i = num_threads; i < threads_.size(); ++i) {
            if (threads_[i].joinable()) {
                threads_[i].join();
            }
        }
        
        threads_.resize(num_threads);
        stop_.store(false);
    }
}

void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.wait(lock, [this] { return stop_.load() || !tasks_.empty(); });
            
            if (stop_.load() && tasks_.empty()) {
                return;
            }
            
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        
        active_threads_.fetch_add(1);
        try {
            if (task) {
                task();
            }
        } catch (...) {
            // 忽略任务执行中的异常
        }
        active_threads_.fetch_sub(1);
    }
}

// TaskScheduler implementation
TaskScheduler::TaskScheduler(const ThreadPoolConfig& config)
    : config_(config), running_(false) {
    // 初始化任务队列
    task_queues_[TaskType::NETWORK_IO] = std::make_unique<TaskQueue>();
    task_queues_[TaskType::SQL_PARSE] = std::make_unique<TaskQueue>();
    task_queues_[TaskType::SQL_EXECUTE] = std::make_unique<TaskQueue>();
    task_queues_[TaskType::PROCEDURE_CALL] = std::make_unique<TaskQueue>();
    task_queues_[TaskType::TRIGGER_EXECUTE] = std::make_unique<TaskQueue>();
    task_queues_[TaskType::STORAGE_IO] = std::make_unique<TaskQueue>();
    task_queues_[TaskType::WAL_WRITE] = std::make_unique<TaskQueue>();
    task_queues_[TaskType::MAINTENANCE] = std::make_unique<TaskQueue>();

    // 初始化线程池
    thread_pools_[TaskType::NETWORK_IO] = std::make_unique<ThreadPool>(config_.network_threads);
    thread_pools_[TaskType::SQL_PARSE] = std::make_unique<ThreadPool>(config_.query_threads);
    thread_pools_[TaskType::SQL_EXECUTE] = std::make_unique<ThreadPool>(config_.query_threads);
    thread_pools_[TaskType::PROCEDURE_CALL] = std::make_unique<ThreadPool>(config_.query_threads);
    thread_pools_[TaskType::TRIGGER_EXECUTE] = std::make_unique<ThreadPool>(config_.query_threads);
    thread_pools_[TaskType::STORAGE_IO] = std::make_unique<ThreadPool>(config_.storage_threads);
    thread_pools_[TaskType::WAL_WRITE] = std::make_unique<ThreadPool>(config_.wal_threads);
    thread_pools_[TaskType::MAINTENANCE] = std::make_unique<ThreadPool>(config_.maintenance_threads);

    // 设置任务到线程池的映射
    task_to_pool_mapping_[TaskType::NETWORK_IO] = TaskType::NETWORK_IO;
    task_to_pool_mapping_[TaskType::SQL_PARSE] = TaskType::SQL_PARSE;
    task_to_pool_mapping_[TaskType::SQL_EXECUTE] = TaskType::SQL_EXECUTE;
    task_to_pool_mapping_[TaskType::PROCEDURE_CALL] = TaskType::PROCEDURE_CALL;
    task_to_pool_mapping_[TaskType::TRIGGER_EXECUTE] = TaskType::TRIGGER_EXECUTE;
    task_to_pool_mapping_[TaskType::STORAGE_IO] = TaskType::STORAGE_IO;
    task_to_pool_mapping_[TaskType::WAL_WRITE] = TaskType::WAL_WRITE;
    task_to_pool_mapping_[TaskType::MAINTENANCE] = TaskType::MAINTENANCE;
}

TaskScheduler::~TaskScheduler() {
    stop();
}

void TaskScheduler::start() {
    if (running_.exchange(true)) {
        return; // 已经在运行
    }

    // 启动各个线程池的工作线程
    for (const auto& pair : thread_pools_) {
        TaskType pool_type = pair.first;
        size_t pool_size = 0;
        switch (pool_type) {
            case TaskType::NETWORK_IO: pool_size = config_.network_threads; break;
            case TaskType::SQL_PARSE:
            case TaskType::SQL_EXECUTE:
            case TaskType::PROCEDURE_CALL:
            case TaskType::TRIGGER_EXECUTE: pool_size = config_.query_threads; break;
            case TaskType::STORAGE_IO: pool_size = config_.storage_threads; break;
            case TaskType::WAL_WRITE: pool_size = config_.wal_threads; break;
            case TaskType::MAINTENANCE: pool_size = config_.maintenance_threads; break;
            default: pool_size = 1; break;
        }

        for (size_t i = 0; i < pool_size; ++i) {
            pair.second->execute([this, pool_type]() { workerThread(pool_type); });
        }
    }
}

void TaskScheduler::stop() {
    running_.store(false);

    // 停止所有线程池 - 通过设置stop标志让工作线程退出
    // 线程池会在析构函数中正确停止
}

bool TaskScheduler::submitTask(std::unique_ptr<Task> task) {
    if (!running_.load()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.total_tasks_submitted++;
    stats_.queued_tasks++;

    dispatchTask(std::move(task));
    return true;
}

TaskStats TaskScheduler::getStats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

size_t TaskScheduler::getPendingTaskCount(TaskType type) const {
    auto it = task_queues_.find(type);
    return (it != task_queues_.end()) ? it->second->size() : 0;
}

size_t TaskScheduler::getActiveThreadCount(TaskType type) const {
    auto it = thread_pools_.find(type);
    return (it != thread_pools_.end()) ? it->second->getActiveThreadCount() : 0;
}

void TaskScheduler::dispatchTask(std::unique_ptr<Task> task) {
    TaskType type = task->getTaskType();
    auto it = task_queues_.find(type);
    if (it != task_queues_.end()) {
        it->second->push(std::move(task));
    }
}

void TaskScheduler::workerThread(TaskType type) {
    while (running_.load()) {
        auto queue_it = task_queues_.find(type);
        if (queue_it == task_queues_.end()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        std::unique_ptr<Task> task = queue_it->second->pop();
        if (!task) {
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_.queued_tasks--;
            stats_.active_threads++;
        }

        auto start_time = std::chrono::high_resolution_clock::now();
        try {
            auto result = task->execute();
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

            // 更新统计信息
            {
                std::lock_guard<std::mutex> lock(stats_mutex_);
                stats_.total_tasks_completed++;
                stats_.average_execution_time_ms =
                    (stats_.average_execution_time_ms + duration.count()) / 2;
                if (static_cast<uint64_t>(duration.count()) > stats_.max_execution_time_ms) {
                    stats_.max_execution_time_ms = duration.count();
                }

                if (result && !result->isSuccess()) {
                    stats_.total_tasks_failed++;
                }
            }

        } catch (...) {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_.total_tasks_failed++;
        }

        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_.active_threads--;
        }
    }
}



// TaskExecutor implementation (legacy compatibility)
TaskExecutor::TaskExecutor(size_t num_threads)
    : is_running_(false) {
    thread_pool_ = std::make_unique<ThreadPool>(num_threads);

    // 初始化不同类型的任务队列
    task_queues_[TaskType::NETWORK_IO] = std::make_unique<TaskQueue>();
    task_queues_[TaskType::SQL_PARSE] = std::make_unique<TaskQueue>();
    task_queues_[TaskType::SQL_EXECUTE] = std::make_unique<TaskQueue>();
    task_queues_[TaskType::WAL_WRITE] = std::make_unique<TaskQueue>();
    task_queues_[TaskType::MAINTENANCE] = std::make_unique<TaskQueue>();
}

TaskExecutor::~TaskExecutor() {
    stop();
}

void TaskExecutor::start() {
    if (is_running_.exchange(true)) {
        return; // 已经在运行
    }
    
    // 启动工作线程
    // 获取线程池中的线程数量
    size_t num_threads = 4; // 默认4个线程
    // 启动工作线程
    for (size_t i = 0; i < num_threads; ++i) {
        thread_pool_->execute([this] { workerThread(); });
    }
}

void TaskExecutor::stop() {
    is_running_.store(false);
    condition_.notify_all();
}

bool TaskExecutor::submitTask(std::unique_ptr<Task> task) {
    if (!is_running_.load()) {
        return false;
    }
    
    dispatchTask(std::move(task));
    return true;
}

size_t TaskExecutor::getPendingTaskCount() const {
    size_t count = 0;
    for (const auto& pair : task_queues_) {
        count += pair.second->size();
    }
    return count;
}

size_t TaskExecutor::getActiveThreadCount() const {
    // 确保thread_pool_存在且有效
    if (!thread_pool_) {
        return 0;
    }
    try {
        return thread_pool_->getActiveThreadCount();
    } catch (...) {
        return 0; // 如果调用失败，返回0
    }
}

void TaskExecutor::dispatchTask(std::unique_ptr<Task> task) {
    TaskType type = task->getTaskType();
    
    auto it = task_queues_.find(type);
    if (it != task_queues_.end()) {
        it->second->push(std::move(task));
    } else {
        // 如果找不到对应的任务队列，放入默认队列
        task_queues_.begin()->second->push(std::move(task));
    }
    
    condition_.notify_one();
}

void TaskExecutor::workerThread() {
    while (is_running_.load()) {
        std::unique_ptr<Task> task;
        
        // 尝试从各个队列中获取任务
        for (auto& pair : task_queues_) {
            if (!pair.second->isEmpty()) {
                task = pair.second->pop();
                break;
            }
        }
        
        if (task) {
            // 执行任务
            try {
                task->execute();
            } catch (...) {
                // 忽略任务执行中的异常
            }
        } else {
            // 没有任务可执行，短暂休眠
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

} // namespace execution
} // namespace sqlcc
