#include "include/execution/task_result.h"
#include <sstream>

namespace sqlcc {

TaskResult::TaskResult(bool success, TaskType task_type)
    : success_(success),
      task_type_(task_type),
      affected_rows_(0),
      time_recorded_(false) {
}

void TaskResult::set_success(bool success) {
    success_ = success;
}

bool TaskResult::get_success() const {
    return success_;
}

void TaskResult::set_error_message(const std::string& error_msg) {
    error_message_ = error_msg;
    if (!error_msg.empty()) {
        success_ = false;
    }
}

std::string TaskResult::get_error_message() const {
    return error_message_;
}

void TaskResult::set_task_type(TaskType type) {
    task_type_ = type;
}

TaskType TaskResult::get_task_type() const {
    return task_type_;
}

void TaskResult::set_start_time() {
    start_time_ = std::chrono::steady_clock::now();
    time_recorded_ = true;
}

void TaskResult::set_end_time() {
    if (time_recorded_) {
        end_time_ = std::chrono::steady_clock::now();
    }
}

long long TaskResult::get_execution_time_ms() const {
    if (!time_recorded_) {
        return 0;
    }

    auto duration = end_time_ - start_time_;
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

void TaskResult::set_affected_rows(size_t rows) {
    affected_rows_ = rows;
}

size_t TaskResult::get_affected_rows() const {
    return affected_rows_;
}

void TaskResult::set_result_message(const std::string& message) {
    result_message_ = message;
}

std::string TaskResult::get_result_message() const {
    return result_message_;
}

bool TaskResult::has_error() const {
    return !error_message_.empty();
}

std::string TaskResult::get_summary() const {
    std::stringstream ss;

    ss << "TaskResult Summary:\n";
    ss << "  Type: ";
    switch (task_type_) {
        case TaskType::UNKNOWN: ss << "UNKNOWN"; break;
        case TaskType::DDL_EXECUTE: ss << "DDL_EXECUTE"; break;
        case TaskType::DML_EXECUTE: ss << "DML_EXECUTE"; break;
        case TaskType::QUERY_EXECUTE: ss << "QUERY_EXECUTE"; break;
        case TaskType::TRANSACTION: ss << "TRANSACTION"; break;
        case TaskType::PROCEDURE_CALL: ss << "PROCEDURE_CALL"; break;
        case TaskType::TRIGGER_EXECUTE: ss << "TRIGGER_EXECUTE"; break;
        case TaskType::SQL_EXECUTE: ss << "SQL_EXECUTE"; break;
        case TaskType::SYSTEM_MAINTAIN: ss << "SYSTEM_MAINTAIN"; break;
    }
    ss << "\n";

    ss << "  Success: " << (success_ ? "true" : "false") << "\n";
    ss << "  Execution Time: " << get_execution_time_ms() << " ms\n";
    ss << "  Affected Rows: " << affected_rows_ << "\n";

    if (!error_message_.empty()) {
        ss << "  Error: " << error_message_ << "\n";
    }

    if (!result_message_.empty()) {
        ss << "  Message: " << result_message_ << "\n";
    }

    return ss.str();
}

} // namespace sqlcc
