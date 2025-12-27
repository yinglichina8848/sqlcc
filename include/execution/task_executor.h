/**
 * @file task_executor.h
 * @brief 任务执行器头文件
 */

#ifndef SQLCC_EXECUTION_TASK_EXECUTOR_H
#define SQLCC_EXECUTION_TASK_EXECUTOR_H

#include <memory>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

#include "core/execution_result.h"
#include "execution/task_result.h"

namespace sqlcc {

class ExecutionContext;

// 任务状态
enum class TaskStatus {
    PENDING = 0,
    RUNNING = 1,
    COMPLETED = 2,
    FAILED = 3,
    CANCELLED = 4
};

// 任务优先级
enum class TaskPriority {
    LOW = 0,
    NORMAL = 1,
    HIGH = 2,
    CRITICAL = 3
};



// 任务接口
class Task {
public:
    Task() : task_id_(""), task_type_(TaskType::UNKNOWN), priority_(0), completed_(false) {}
    Task(const std::string& task_id, TaskType type, int priority = 0)
        : task_id_(task_id), task_type_(type), priority_(priority), completed_(false) {}
    virtual ~Task() = default;

    virtual std::shared_ptr<TaskResult> execute() = 0;
    virtual std::string getDescription() const { return task_id_; }
    virtual TaskPriority getPriority() const { return static_cast<TaskPriority>(priority_); }

    const std::string& getTaskId() const { return task_id_; }
    TaskType getTaskType() const { return task_type_; }
    int getPriorityValue() const { return priority_; }
    bool isCompleted() const { return completed_; }

    void setTaskId(const std::string& task_id) { task_id_ = task_id; }

private:
    std::string task_id_;
    TaskType task_type_;
    int priority_;
    bool completed_;
};

// 任务执行器 - 管理任务队列和执行
class TaskExecutor {
public:
    TaskExecutor();
    ~TaskExecutor();

    // 初始化和配置
    bool initialize(int num_threads = 4);
    void shutdown();

    // 任务提交
    int submitTask(std::unique_ptr<Task> task);
    int submitTask(std::function<void(ExecutionContext&)> func,
                   const std::string& description,
                   TaskPriority priority = TaskPriority::NORMAL);

    // 任务管理
    bool cancelTask(int task_id);
    TaskStatus getTaskStatus(int task_id) const;
    size_t getPendingTaskCount() const;
    size_t getRunningTaskCount() const;

    // 阻塞等待
    ExecutionResult waitForTask(int task_id);
    ExecutionResult waitForAllTasks();

    // 资源管理
    void setMaxConcurrentTasks(size_t max_tasks);
    void setTaskTimeout(std::chrono::seconds timeout);

private:
    // 任务队列
    struct TaskItem {
        int task_id;
        std::unique_ptr<Task> task;
        std::chrono::steady_clock::time_point submit_time;

        bool operator<(const TaskItem& other) const {
            return task->getPriority() < other.task->getPriority();
        }
    };

    // 工作线程
    void workerThread();
    void processTask(TaskItem& task_item);

    // 同步原语
    mutable std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::priority_queue<TaskItem> task_queue_;

    // 线程管理
    std::vector<std::thread> worker_threads_;
    std::atomic<bool> running_;

    // 配置
    size_t max_concurrent_tasks_;
    std::chrono::seconds task_timeout_;

    // 任务跟踪
    mutable std::mutex status_mutex_;
    std::unordered_map<int, TaskStatus> task_statuses_;
    std::atomic<int> next_task_id_;
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_TASK_EXECUTOR_H
