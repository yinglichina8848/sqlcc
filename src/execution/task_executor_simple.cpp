#include "task_executor.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <ctime>

namespace sqlcc {

// TaskExecutor implementation
TaskExecutor::TaskExecutor() 
    : running_(false), max_concurrent_tasks_(4), task_timeout_(std::chrono::seconds(30)), next_task_id_(0) {
}

TaskExecutor::~TaskExecutor() {
    shutdown();
}

bool TaskExecutor::initialize(int num_threads) {
    if (num_threads <= 0) num_threads = 4;
    
    running_.store(true);
    
    // 创建工作线程
    for (int i = 0; i < num_threads; ++i) {
        worker_threads_.emplace_back([this]() { workerThread(); });
    }
    
    return true;
}

void TaskExecutor::shutdown() {
    if (!running_.load()) return;
    
    running_.store(false);
    queue_cv_.notify_all();
    
    // 等待所有工作线程结束
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    worker_threads_.clear();
}

int TaskExecutor::submitTask(std::unique_ptr<Task> task) {
    if (!running_.load()) return -1;
    
    int task_id = next_task_id_.fetch_add(1);
    task->setTaskId(std::to_string(task_id));
    
    TaskItem item;
    item.task_id = task_id;
    item.task = std::move(task);
    item.submit_time = std::chrono::steady_clock::now();
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        task_queue_.push(std::move(item));
        task_statuses_[task_id] = TaskStatus::PENDING;
    }
    
    queue_cv_.notify_one();
    return task_id;
}

int TaskExecutor::submitTask(std::function<void(ExecutionContext&)> func,
                           const std::string& description,
                           TaskPriority priority) {
    // 创建一个包装任务来执行函数
    // 这里简化实现，实际可能需要一个FunctionTask类
    return -1; // 暂时未实现
}

bool TaskExecutor::cancelTask(int task_id) {
    std::lock_guard<std::mutex> lock(status_mutex_);
    auto it = task_statuses_.find(task_id);
    if (it != task_statuses_.end() && it->second == TaskStatus::PENDING) {
        it->second = TaskStatus::CANCELLED;
        return true;
    }
    return false;
}

TaskStatus TaskExecutor::getTaskStatus(int task_id) const {
    std::lock_guard<std::mutex> lock(status_mutex_);
    auto it = task_statuses_.find(task_id);
    if (it != task_statuses_.end()) {
        return it->second;
    }
    return TaskStatus::FAILED; // 任务不存在
}

size_t TaskExecutor::getPendingTaskCount() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return task_queue_.size();
}

size_t TaskExecutor::getRunningTaskCount() const {
    // 简化实现，返回近似值
    return 0;
}

ExecutionResult TaskExecutor::waitForTask(int task_id) {
    // 简化实现
    ExecutionResult result(true, "Success");
    return result;
}

ExecutionResult TaskExecutor::waitForAllTasks() {
    // 简化实现
    ExecutionResult result(true, "Success");
    return result;
}

void TaskExecutor::setMaxConcurrentTasks(size_t max_tasks) {
    max_concurrent_tasks_ = max_tasks;
}

void TaskExecutor::setTaskTimeout(std::chrono::seconds timeout) {
    task_timeout_ = timeout;
}

void TaskExecutor::workerThread() {
    while (running_.load()) {
        TaskItem item;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { return !task_queue_.empty() || !running_.load(); });
            
            if (!running_.load() && task_queue_.empty()) {
                break;
            }
            
            if (!task_queue_.empty()) {
                item = std::move(const_cast<TaskItem&>(task_queue_.top()));
                task_queue_.pop();
                
                // 更新任务状态
                {
                    std::lock_guard<std::mutex> lock(status_mutex_);
                    task_statuses_[item.task_id] = TaskStatus::RUNNING;
                }
            }
        }
        
        if (item.task) {
            try {
                // 执行任务
                auto result = item.task->execute();
                
                // 更新任务状态
                {
                    std::lock_guard<std::mutex> lock(status_mutex_);
                    task_statuses_[item.task_id] = TaskStatus::COMPLETED;
                }
            } catch (...) {
                // 任务执行失败
                std::lock_guard<std::mutex> lock(status_mutex_);
                task_statuses_[item.task_id] = TaskStatus::FAILED;
            }
        }
    }
}

void TaskExecutor::processTask(TaskItem& task_item) {
    // 在workerThread中直接处理，不需要单独的函数
}

} // namespace sqlcc