#ifndef SQLCC_ERROR_HANDLER_H
#define SQLCC_ERROR_HANDLER_H

#include <string>
#include <map>
#include <memory>
#include <sstream>
#include <functional>

namespace sqlcc {

/**
 * @brief 错误级别枚举
 */
enum class ErrorLevel {
    INFO,      // 信息级别
    WARNING,   // 警告级别
    ERROR,     // 错误级别
    FATAL      // 致命错误级别
};

/**
 * @brief 错误代码枚举
 */
enum class ErrorCode {
    // 通用错误
    SUCCESS = 0,
    UNKNOWN_ERROR = 1000,
    INVALID_PARAMETER = 1001,
    RESOURCE_NOT_FOUND = 1002,
    PERMISSION_DENIED = 1003,
    
    // SQL解析错误
    SQL_SYNTAX_ERROR = 2000,
    SQL_SEMANTIC_ERROR = 2001,
    SQL_TYPE_MISMATCH = 2002,
    
    // 数据库错误
    DATABASE_NOT_EXIST = 3000,
    DATABASE_ALREADY_EXISTS = 3001,
    TABLE_NOT_EXIST = 3002,
    TABLE_ALREADY_EXISTS = 3003,
    COLUMN_NOT_EXIST = 3004,
    COLUMN_ALREADY_EXISTS = 3005,
    
    // 约束错误
    CONSTRAINT_VIOLATION = 4000,
    NOT_NULL_VIOLATION = 4001,
    UNIQUE_VIOLATION = 4002,
    PRIMARY_KEY_VIOLATION = 4003,
    FOREIGN_KEY_VIOLATION = 4004,
    
    // 事务错误
    TRANSACTION_ERROR = 5000,
    DEADLOCK_DETECTED = 5001,
    CONCURRENCY_CONFLICT = 5002,
    
    // 系统错误
    SYSTEM_ERROR = 6000,
    MEMORY_ALLOCATION_FAILED = 6001,
    DISK_IO_ERROR = 6002,
    NETWORK_ERROR = 6003
};

/**
 * @brief 错误信息结构体
 */
struct ErrorInfo {
    ErrorCode code;
    ErrorLevel level;
    std::string message;
    std::string details;
    std::string module;
    std::string timestamp;
    
    ErrorInfo(ErrorCode c, ErrorLevel l, const std::string& msg, 
              const std::string& det = "", const std::string& mod = "")
        : code(c), level(l), message(msg), details(det), module(mod) {
        // 设置时间戳
        // TODO: 实现时间戳生成
        timestamp = "";
    }
    
    std::string toString() const {
        std::stringstream ss;
        ss << "[" << module << "] " << levelToString(level) << " " 
           << codeToString(code) << ": " << message;
        if (!details.empty()) {
            ss << " (" << details << ")";
        }
        return ss.str();
    }
    
private:
    static std::string levelToString(ErrorLevel level) {
        switch (level) {
            case ErrorLevel::INFO: return "INFO";
            case ErrorLevel::WARNING: return "WARNING";
            case ErrorLevel::ERROR: return "ERROR";
            case ErrorLevel::FATAL: return "FATAL";
            default: return "UNKNOWN";
        }
    }
    
    static std::string codeToString(ErrorCode code) {
        switch (code) {
            case ErrorCode::SUCCESS: return "SUCCESS";
            case ErrorCode::UNKNOWN_ERROR: return "UNKNOWN_ERROR";
            case ErrorCode::INVALID_PARAMETER: return "INVALID_PARAMETER";
            case ErrorCode::RESOURCE_NOT_FOUND: return "RESOURCE_NOT_FOUND";
            case ErrorCode::PERMISSION_DENIED: return "PERMISSION_DENIED";
            case ErrorCode::SQL_SYNTAX_ERROR: return "SQL_SYNTAX_ERROR";
            case ErrorCode::SQL_SEMANTIC_ERROR: return "SQL_SEMANTIC_ERROR";
            case ErrorCode::SQL_TYPE_MISMATCH: return "SQL_TYPE_MISMATCH";
            case ErrorCode::DATABASE_NOT_EXIST: return "DATABASE_NOT_EXIST";
            case ErrorCode::DATABASE_ALREADY_EXISTS: return "DATABASE_ALREADY_EXISTS";
            case ErrorCode::TABLE_NOT_EXIST: return "TABLE_NOT_EXIST";
            case ErrorCode::TABLE_ALREADY_EXISTS: return "TABLE_ALREADY_EXISTS";
            case ErrorCode::COLUMN_NOT_EXIST: return "COLUMN_NOT_EXIST";
            case ErrorCode::COLUMN_ALREADY_EXISTS: return "COLUMN_ALREADY_EXISTS";
            case ErrorCode::CONSTRAINT_VIOLATION: return "CONSTRAINT_VIOLATION";
            case ErrorCode::NOT_NULL_VIOLATION: return "NOT_NULL_VIOLATION";
            case ErrorCode::UNIQUE_VIOLATION: return "UNIQUE_VIOLATION";
            case ErrorCode::PRIMARY_KEY_VIOLATION: return "PRIMARY_KEY_VIOLATION";
            case ErrorCode::FOREIGN_KEY_VIOLATION: return "FOREIGN_KEY_VIOLATION";
            case ErrorCode::TRANSACTION_ERROR: return "TRANSACTION_ERROR";
            case ErrorCode::DEADLOCK_DETECTED: return "DEADLOCK_DETECTED";
            case ErrorCode::CONCURRENCY_CONFLICT: return "CONCURRENCY_CONFLICT";
            case ErrorCode::SYSTEM_ERROR: return "SYSTEM_ERROR";
            case ErrorCode::MEMORY_ALLOCATION_FAILED: return "MEMORY_ALLOCATION_FAILED";
            case ErrorCode::DISK_IO_ERROR: return "DISK_IO_ERROR";
            case ErrorCode::NETWORK_ERROR: return "NETWORK_ERROR";
            default: return "UNKNOWN_CODE";
        }
    }
};

/**
 * @brief 统一错误处理器
 * 
 * 提供标准化的错误处理机制，解决不同执行器错误处理不一致的问题
 */
class ErrorHandler {
public:
    static ErrorHandler& getInstance() {
        static ErrorHandler instance;
        return instance;
    }
    
    /**
     * @brief 记录错误
     */
    void logError(const ErrorInfo& error);
    
    /**
     * @brief 创建错误信息
     */
    ErrorInfo createError(ErrorCode code, ErrorLevel level, 
                         const std::string& message, 
                         const std::string& details = "",
                         const std::string& module = "");
    
    /**
     * @brief 创建SQL语法错误
     */
    ErrorInfo createSQLSyntaxError(const std::string& details, const std::string& module = "SQL_PARSER");
    
    /**
     * @brief 创建数据库不存在错误
     */
    ErrorInfo createDatabaseNotFoundError(const std::string& db_name, const std::string& module = "SQL_EXECUTOR");
    
    /**
     * @brief 创建表不存在错误
     */
    ErrorInfo createTableNotFoundError(const std::string& table_name, const std::string& module = "SQL_EXECUTOR");
    
    /**
     * @brief 创建权限拒绝错误
     */
    ErrorInfo createPermissionDeniedError(const std::string& operation, const std::string& resource, 
                                         const std::string& module = "SQL_EXECUTOR");
    
    /**
     * @brief 创建约束违反错误
     */
    ErrorInfo createConstraintViolationError(const std::string& constraint_type, 
                                            const std::string& details, 
                                            const std::string& module = "SQL_EXECUTOR");
    
    /**
     * @brief 获取最后一次错误
     */
    ErrorInfo getLastError() const;
    
    /**
     * @brief 清除错误记录
     */
    void clearErrors();
    
    /**
     * @brief 设置错误回调函数
     */
    void setErrorCallback(std::function<void(const ErrorInfo&)> callback);

private:
    ErrorHandler() = default;
    ~ErrorHandler() = default;
    
    std::vector<ErrorInfo> error_history_;
    std::function<void(const ErrorInfo&)> error_callback_;
    
    // 防止复制
    ErrorHandler(const ErrorHandler&) = delete;
    ErrorHandler& operator=(const ErrorHandler&) = delete;
};

/**
 * @brief 错误处理辅助宏
 */
#define SQLCC_ERROR(code, level, message, details, module) \
    ErrorHandler::getInstance().createError(code, level, message, details, module)

#define SQLCC_SYNTAX_ERROR(details, module) \
    ErrorHandler::getInstance().createSQLSyntaxError(details, module)

#define SQLCC_DATABASE_NOT_FOUND(db_name, module) \
    ErrorHandler::getInstance().createDatabaseNotFoundError(db_name, module)

#define SQLCC_TABLE_NOT_FOUND(table_name, module) \
    ErrorHandler::getInstance().createTableNotFoundError(table_name, module)

#define SQLCC_PERMISSION_DENIED(operation, resource, module) \
    ErrorHandler::getInstance().createPermissionDeniedError(operation, resource, module)

#define SQLCC_CONSTRAINT_VIOLATION(constraint_type, details, module) \
    ErrorHandler::getInstance().createConstraintViolationError(constraint_type, details, module)

} // namespace sqlcc

#endif // SQLCC_ERROR_HANDLER_H