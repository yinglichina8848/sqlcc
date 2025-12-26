/**
 * @file task_result.h
 * @brief 任务结果类定义
 */

#ifndef SQLCC_EXECUTION_TASK_RESULT_H
#define SQLCC_EXECUTION_TASK_RESULT_H

#include <string>
#include <memory>
#include <chrono>

namespace sqlcc {
namespace execution {

// 任务类型枚举
enum class TaskType {
    NETWORK,
    SQL_PARSE,
    SQL_EXECUTE,
    WAL_LOG,
    TRANSACTION,
    PROCEDURE_CALL,
    TRIGGER_EXECUTE,
    UNKNOWN
};

// 任务结果类
class TaskResult {
public:
    TaskResult(const std::string& task_id) 
        : task_id_(task_id), success_(false), execution_time_(0) {}
    
    bool isSuccess() const { return success_; }
    const std::string& getErrorMessage() const { return error_message_; }
    const std::string& getResultData() const { return result_data_; }
    std::chrono::milliseconds getExecutionTime() const { return execution_time_; }

    void setSuccess(bool success) { success_ = success; }
    void setErrorMessage(const std::string& error) { error_message_ = error; }
    void setResultData(const std::string& data) { result_data_ = data; }
    void setExecutionTime(std::chrono::milliseconds time) { execution_time_ = time; }
    void setTaskId(const std::string& task_id) { task_id_ = task_id; }
    const std::string& getTaskId() const { return task_id_; }

private:
    std::string task_id_;
    bool success_;
    std::string error_message_;
    std::string result_data_;
    std::chrono::milliseconds execution_time_;
};

} // namespace execution
} // namespace sqlcc

#endif // SQLCC_EXECUTION_TASK_RESULT_H