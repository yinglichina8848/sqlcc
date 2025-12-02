#include "error_handler.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace sqlcc {

void ErrorHandler::logError(const ErrorInfo& error) {
    error_history_.push_back(error);
    
    // 调用错误回调函数
    if (error_callback_) {
        error_callback_(error);
    }
    
    // 输出到标准错误（可选）
    if (error.level == ErrorLevel::ERROR || error.level == ErrorLevel::FATAL) {
        std::cerr << error.toString() << std::endl;
    }
}

ErrorInfo ErrorHandler::createError(ErrorCode code, ErrorLevel level, 
                                   const std::string& message, 
                                   const std::string& details,
                                   const std::string& module) {
    // 生成时间戳
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    
    ErrorInfo error(code, level, message, details, module);
    error.timestamp = ss.str();
    
    logError(error);
    return error;
}

ErrorInfo ErrorHandler::createSQLSyntaxError(const std::string& details, const std::string& module) {
    std::string message = "SQL语法错误";
    if (!details.empty()) {
        message += ": " + details;
    }
    return createError(ErrorCode::SQL_SYNTAX_ERROR, ErrorLevel::ERROR, message, details, module);
}

ErrorInfo ErrorHandler::createDatabaseNotFoundError(const std::string& db_name, const std::string& module) {
    std::string message = "数据库不存在: " + db_name;
    return createError(ErrorCode::DATABASE_NOT_EXIST, ErrorLevel::ERROR, message, "", module);
}

ErrorInfo ErrorHandler::createTableNotFoundError(const std::string& table_name, const std::string& module) {
    std::string message = "表不存在: " + table_name;
    return createError(ErrorCode::TABLE_NOT_EXIST, ErrorLevel::ERROR, message, "", module);
}

ErrorInfo ErrorHandler::createPermissionDeniedError(const std::string& operation, const std::string& resource, 
                                                   const std::string& module) {
    std::string message = "权限拒绝: 无法执行 " + operation + " 操作于 " + resource;
    return createError(ErrorCode::PERMISSION_DENIED, ErrorLevel::ERROR, message, "", module);
}

ErrorInfo ErrorHandler::createConstraintViolationError(const std::string& constraint_type, 
                                                       const std::string& details, 
                                                       const std::string& module) {
    std::string message = "约束违反: " + constraint_type + " 约束";
    if (!details.empty()) {
        message += " (" + details + ")";
    }
    return createError(ErrorCode::CONSTRAINT_VIOLATION, ErrorLevel::ERROR, message, details, module);
}

ErrorInfo ErrorHandler::getLastError() const {
    if (error_history_.empty()) {
        return ErrorInfo(ErrorCode::SUCCESS, ErrorLevel::INFO, "没有错误记录", "", "ERROR_HANDLER");
    }
    return error_history_.back();
}

void ErrorHandler::clearErrors() {
    error_history_.clear();
}

void ErrorHandler::setErrorCallback(std::function<void(const ErrorInfo&)> callback) {
    error_callback_ = callback;
}

} // namespace sqlcc