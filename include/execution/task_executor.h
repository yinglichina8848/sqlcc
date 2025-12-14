#ifndef SQLCC_TASK_EXECUTOR_H
#define SQLCC_TASK_EXECUTOR_H

#include <memory>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <map>
#include <chrono>
#include <string>

namespace sqlcc {
namespace execution {

// 任务类型枚举
enum class TaskType {
    NETWORK,
    SQL_PARSE,
    SQL_EXECUTE,
    WAL_LOG,
    TRANSACTION,
    UNKNOWN
};

// 任务结果类
class TaskResult {
public:
    TaskResult(const std::string& task_id);
    
    bool isSuccess() const { return success_; }
    const std::string& getErrorMessage() const { return error_message_; }
    const std::string& getResultData() const { return result_data_; }
    std::chrono::milliseconds getExecutionTime() const { return execution_time_; }

    void setSuccess(bool success) { success_ = success; }
    void setErrorMessage(const std::string& error) { error_message_ = error; }
    void setResultData(const std::string& data) { result_data_ = data; }
    void setExecutionTime(std::chrono::milliseconds time) { execution_time_ = time; }

private:
    std::string task_id_;
    bool success_;
    std::string error_message_;
    std::string result_data_;
    std::chrono::milliseconds execution_time_;
    std::chrono::time_point<std::chrono::high_resolution_clock> timestamp_;
};

// 抽象任务类
class Task {
public:
    Task(const std::string& task_id, TaskType type, int priority = 0);
    virtual ~Task() = default;

    virtual std::shared_ptr<TaskResult> execute() = 0;
    
    const std::string& getTaskId() const { return task_id_; }
    TaskType getTaskType() const { return task_type_; }
    int getPriority() const { return priority_; }
    bool isCompleted() const { return completed_.load(); }
    
    std::shared_ptr<TaskResult> getResult() const { return result_; }
    void setResult(std::shared_ptr<TaskResult> result) { 
        result_ = result; 
        completed_.store(true);
    }

protected:
    std::string task_id_;
    TaskType task_type_;
    int priority_;
    std::atomic<bool> completed_;
    std::shared_ptr<TaskResult> result_;
    std::chrono::time_point<std::chrono::high_resolution_clock> created_at_;
};

// 网络任务类
class NetworkTask : public Task {
public:
    NetworkTask(const std::string& task_id, const std::string& request_data);
    virtual ~NetworkTask() = default;
    
    std::shared_ptr<TaskResult> execute() override;
    
    void setConnectionData(const std::string& data) { connection_data_ = data; }
    const std::string& getConnectionData() const { return connection_data_; }

private:
    std::string request_data_;
    std::string connection_data_;
};

// SQL任务类
class SQLTask : public Task {
public:
    SQLTask(const std::string& task_id, const std::string& sql_statement);
    virtual ~SQLTask() = default;
    
    std::shared_ptr<TaskResult> execute() override;
    
    void setTransactionId(uint64_t txn_id) { transaction_id_ = txn_id; }
    uint64_t getTransactionId() const { return transaction_id_; }

private:
    std::string sql_statement_;
    uint64_t transaction_id_;
};

// WAL任务类
class WALTask : public Task {
public:
    WALTask(const std::string& task_id, const std::string& log_data);
    virtual ~WALTask() = default;
    
    std::shared_ptr<TaskResult> execute() override;
    
    void setFlushRequired(bool flush) { flush_required_ = flush; }
    bool isFlushRequired() const { return flush_required_; }

private:
    std::string log_data_;
    bool flush_required_;
};

// 事务任务类
class TransactionTask : public Task {
public:
    enum Operation {
        BEGIN,
        COMMIT,
        ROLLBACK
    };
    
    TransactionTask(const std::string& task_id, uint64_t transaction_id, Operation op);
    virtual ~TransactionTask() = default;
    
    std::shared_ptr<TaskResult> execute() override;
    
    uint64_t getTransactionId() const { return transaction_id_; }
    Operation getOperation() const { return operation_; }

private:
    uint64_t transaction_id_;
    Operation operation_;
};

// 任务队列类
class TaskQueue {
public:
    explicit TaskQueue(size_t max_size = 1000);
    
    bool push(std::unique_ptr<Task> task);
    std::unique_ptr<Task> pop();
    size_t size() const;
    bool isEmpty() const;
    
private:
    std::queue<std::unique_ptr<Task>> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    const size_t max_size_;
};

// 线程池类
class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
    ~ThreadPool();
    
    void execute(std::function<void()> task);
    void resize(size_t num_threads);
    size_t getActiveThreadCount() const { return active_threads_.load(); }
    
private:
    void workerThread();
    
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
    std::atomic<size_t> active_threads_;
};

// 任务执行器类
class TaskExecutor {
public:
    explicit TaskExecutor(size_t num_threads = std::thread::hardware_concurrency());
    ~TaskExecutor();
    
    void start();
    void stop();
    bool submitTask(std::unique_ptr<Task> task);
    
    size_t getPendingTaskCount() const;
    size_t getActiveThreadCount() const;
    
private:
    void dispatchTask(std::unique_ptr<Task> task);
    void workerThread();
    
    std::unique_ptr<ThreadPool> thread_pool_;
    std::map<TaskType, std::unique_ptr<TaskQueue>> task_queues_;
    std::atomic<bool> is_running_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
};

} // namespace execution
} // namespace sqlcc

#endif // SQLCC_TASK_EXECUTOR_H