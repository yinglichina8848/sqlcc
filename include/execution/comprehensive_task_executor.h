/**
 * @file comprehensive_task_executor.h
 * @brief 综合任务执行器头文件
 */

#ifndef SQLCC_EXECUTION_COMPREHENSIVE_TASK_EXECUTOR_H
#define SQLCC_EXECUTION_COMPREHENSIVE_TASK_EXECUTOR_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <chrono>

#include "execution/task_executor.h"
#include "core/execution_result.h"

namespace sqlcc {

class ExecutionContext;

// 任务统计信息
struct TaskStatistics {
    size_t total_tasks = 0;
    size_t completed_tasks = 0;
    size_t failed_tasks = 0;
    size_t cancelled_tasks = 0;
    std::chrono::milliseconds average_execution_time = std::chrono::milliseconds(0);
    double success_rate = 0.0;
};

// 综合任务执行器 - 提供高级任务管理功能
class ComprehensiveTaskExecutor : public TaskExecutor {
public:
    ComprehensiveTaskExecutor();
    ~ComprehensiveTaskExecutor() override = default;

    // 批量任务提交
    std::vector<int> submitBatchTasks(const std::vector<std::unique_ptr<Task>>& tasks);
    std::vector<int> submitBatchTasks(const std::vector<std::function<void(ExecutionContext&)>>& functions,
                                     const std::vector<std::string>& descriptions,
                                     TaskPriority priority = TaskPriority::NORMAL);

    // 任务依赖管理
    bool addTaskDependency(int task_id, int depends_on_task_id);
    bool removeTaskDependency(int task_id, int depends_on_task_id);
    std::vector<int> getTaskDependencies(int task_id) const;
    std::vector<int> getDependentTasks(int task_id) const;

    // 任务组管理
    int createTaskGroup(const std::string& group_name);
    bool addTaskToGroup(int task_id, int group_id);
    bool removeTaskFromGroup(int task_id, int group_id);
    ExecutionResult waitForTaskGroup(int group_id);
    TaskStatistics getTaskGroupStatistics(int group_id) const;

    // 资源限制
    void setMaxTasksPerMinute(size_t max_tasks);
    void setMaxConcurrentTasksPerGroup(size_t max_tasks);
    void setTaskTimeout(std::chrono::seconds timeout);

    // 监控和统计
    TaskStatistics getOverallStatistics() const;
    TaskStatistics getRecentStatistics(std::chrono::minutes window) const;
    std::vector<std::pair<int, TaskStatus>> getAllTaskStatuses() const;

    // 健康检查
    bool isHealthy() const;
    std::vector<std::string> getHealthWarnings() const;

    // 清理和维护
    void cleanupCompletedTasks();
    void cleanupFailedTasks(std::chrono::minutes max_age);
    void resetStatistics();

private:
    // 任务依赖图
    struct TaskNode {
        int task_id;
        std::vector<int> dependencies;
        std::vector<int> dependents;
        int group_id = -1;
        std::chrono::steady_clock::time_point start_time;
        std::chrono::steady_clock::time_point end_time;
        bool execution_started = false;
    };

    // 任务组
    struct TaskGroup {
        std::string name;
        std::vector<int> task_ids;
        TaskStatistics statistics;
    };

    // 内部状态
    mutable std::mutex dependency_mutex_;
    std::unordered_map<int, TaskNode> task_graph_;
    std::unordered_map<int, TaskGroup> task_groups_;

    // 配置
    size_t max_tasks_per_minute_;
    size_t max_concurrent_per_group_;
    std::chrono::seconds task_timeout_;

    // 统计数据
    mutable std::mutex stats_mutex_;
    TaskStatistics overall_stats_;
    std::vector<std::pair<std::chrono::steady_clock::time_point, TaskStatistics>> stats_history_;

    // 辅助方法
    bool canExecuteTask(int task_id) const;
    void updateTaskStatistics(int task_id, TaskStatus new_status);
    void updateGroupStatistics(int group_id);
    bool validateDependencyGraph() const;
    void cleanupOldStatistics(std::chrono::minutes max_age);
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_COMPREHENSIVE_TASK_EXECUTOR_H
