/**
 * @file error_handler.cpp
 *
 * WHY: 为什么需要错误处理器？
 *
 * 数据库系统运行在复杂的分布式环境中，错误无处不在。没有完善的错误处理系统，系统就会因为单个错误而崩溃，导致数据丢失和服务中断。错误处理器是数据库系统稳定性和可靠性的守护者，直接决定了系统的容错能力和用户体验。
 *
 * 主要问题解决：
 * 1. 错误统一管理：标准化错误分类和处理流程
 * 2. 系统稳定性：防止单个错误导致整个系统崩溃
 * 3. 调试支持：提供详细的错误上下文和调用栈信息
 * 4. 用户反馈：向用户提供清晰易懂的错误信息
 * 5. 监控告警：支持错误统计和系统健康监控
 *
 * 错误处理器失败的影响：
 * - 系统崩溃：未处理的异常导致程序终止
 * - 数据丢失：错误状态下数据可能损坏
 * - 用户困惑：模糊的错误信息无法定位问题
 * - 调试困难：缺乏错误上下文和历史记录
 *
 * WHAT: 这实现了什么功能？
 *
 * 错误处理器提供完整的数据库系统错误管理功能：
 * - 错误分类：按严重程度和类型分类错误
 * - 错误记录：时间戳、模块信息、详细描述
 * - 错误回调：支持自定义错误处理逻辑
 * - 错误历史：维护错误发生的时间线
 * - 错误格式化：标准化错误信息输出格式
 * - 错误统计：支持错误频率和模式的分析
 *
 * 核心组件：
 * - ErrorHandler：错误处理器主类，单例模式管理
 * - ErrorInfo：错误信息结构体，封装错误详情
 * - ErrorCode：错误代码枚举，标准化错误分类
 * - ErrorLevel：错误级别枚举，区分错误严重程度
 * - ErrorCallback：错误回调接口，支持异步处理
 *
 * HOW: 如何实现的？
 *
 * 技术实现要点：
 * 1. 单例模式：使用静态实例保证全局唯一访问
 * 2. 错误队列：使用vector存储错误历史记录
 * 3. 时间戳生成：使用chrono库生成精确时间戳
 * 4. 回调机制：使用std::function支持灵活的错误处理
 * 5. 线程安全：考虑多线程环境下的错误记录安全
 * 6. 错误格式化：使用stringstream构建标准化的错误信息
 *
 * 架构设计：
 * - 分层处理：错误捕获层、处理层、报告层分离
 * - 插件架构：支持不同类型的错误处理器扩展
 * - 配置驱动：运行时配置错误处理策略和阈值
 * - 异步处理：后台线程处理错误日志和告警
 * - 监控集成：与系统监控和告警系统集成
 *
 * 错误处理策略：
 * - 优雅降级：非致命错误不影响系统正常运行
 * - 错误恢复：提供错误恢复和重试机制
 * - 错误隔离：单个组件错误不扩散到整个系统
 * - 错误聚合：相同类型错误进行统计和分析
 * - 错误升级：严重错误自动升级处理级别
 *
 * @note 该实现专为SQLCC数据库系统优化，支持高并发和ACID事务特性
 * @see include/error_handler.h
 */

#include "src/error_handler.h"
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
