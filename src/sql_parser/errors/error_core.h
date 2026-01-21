#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ast/ast_fwd.h"

namespace sqlcc {
namespace sql_parser {
namespace errors {

/**
 * @brief 错误类型枚举
 */
enum class ErrorType {
    // 语法错误
    SYNTAX_ERROR,
    // 语义错误
    SEMANTIC_ERROR,
    // 类型错误
    TYPE_ERROR,
    // 约束错误
    CONSTRAINT_ERROR,
    // 权限错误
    PERMISSION_ERROR,
    // 运行时错误
    RUNTIME_ERROR,
    // 内部错误
    RUNTIME_INTERNAL_ERROR,
    // 网络错误
    NETWORK_ERROR,
    // 磁盘错误
    DISK_ERROR,
    // 配置错误
    CONFIG_ERROR,
    // 其他错误
    UNKNOWN_ERROR
};

/**
 * @brief 错误严重程度枚举
 */
enum class Severity {
    INFO,
    WARNING,
    ERROR,
    FATAL
};

/**
 * @brief 解析错误类
 *
 * 表示在SQL解析过程中发生的各种错误信息
 */
class ParseError {
public:
    /**
     * @brief 构造函数
     *
     * @param type 错误类型
     * @param severity 错误严重程度
     * @param message 错误消息
     * @param location 错误位置
     */
    ParseError(ErrorType type, Severity severity,
               const std::string& message,
               const ast::SourceLocation& location);

    /**
     * @brief 析构函数
     */
    ~ParseError() = default;

    /**
     * @brief 获取错误类型
     *
     * @return 错误类型
     */
    ErrorType getType() const;

    /**
     * @brief 获取错误严重程度
     *
     * @return 错误严重程度
     */
    Severity getSeverity() const;

    /**
     * @brief 获取错误消息
     *
     * @return 错误消息
     */
    const std::string& getMessage() const;

    /**
     * @brief 获取错误位置
     *
     * @return 错误位置
     */
    const ast::SourceLocation& getLocation() const;

    /**
     * @brief 设置建议修复方案
     *
     * @param suggestion 建议修复方案
     */
    void setSuggestion(const std::string& suggestion);

    /**
     * @brief 获取建议修复方案
     *
     * @return 建议修复方案
     */
    const std::string& getSuggestion() const;

    /**
     * @brief 设置上下文信息
     *
     * @param context 上下文信息
     */
    void setContext(const std::string& context);

    /**
     * @brief 获取上下文信息
     *
     * @return 上下文信息
     */
    const std::string& getContext() const;

    /**
     * @brief 转换为字符串表示
     *
     * @return 字符串表示
     */
    std::string toString() const;

    /**
     * @brief 转换为JSON表示
     *
     * @return JSON字符串
     */
    std::string toJson() const;

    /**
     * @brief 检查是否为致命错误
     *
     * @return 如果是致命错误返回true
     */
    bool isFatal() const;

    /**
     * @brief 检查是否为警告
     *
     * @return 如果是警告返回true
     */
    bool isWarning() const;

private:
    ErrorType type_;
    Severity severity_;
    std::string message_;
    ast::SourceLocation location_;
    std::string suggestion_;
    std::string context_;
};

/**
 * @brief 错误收集器类
 *
 * 用于收集和管理解析过程中的所有错误信息
 */
class ErrorCollector {
public:
    /**
     * @brief 构造函数
     */
    ErrorCollector();

    /**
     * @brief 析构函数
     */
    ~ErrorCollector() = default;

    /**
     * @brief 添加错误
     *
     * @param error 错误对象
     */
    void addError(std::unique_ptr<ParseError> error);

    /**
     * @brief 添加警告
     *
     * @param message 警告消息
     * @param location 警告位置
     */
    void addWarning(const std::string& message,
                   const ast::SourceLocation& location);

    /**
     * @brief 添加信息
     *
     * @param message 信息消息
     * @param location 信息位置
     */
    void addInfo(const std::string& message,
                const ast::SourceLocation& location);

    /**
     * @brief 检查是否有错误
     *
     * @return 如果有错误返回true
     */
    bool hasErrors() const;

    /**
     * @brief 检查是否有致命错误
     *
     * @return 如果有致命错误返回true
     */
    bool hasFatalErrors() const;

    /**
     * @brief 检查是否有警告
     *
     * @return 如果有警告返回true
     */
    bool hasWarnings() const;

    /**
     * @brief 获取错误数量
     *
     * @return 错误数量
     */
    size_t getErrorCount() const;

    /**
     * @brief 获取警告数量
     *
     * @return 警告数量
     */
    size_t getWarningCount() const;

    /**
     * @brief 获取所有错误
     *
     * @return 错误列表的引用
     */
    const std::vector<std::unique_ptr<ParseError>>& getErrors() const;

    /**
     * @brief 获取所有警告
     *
     * @return 警告列表的引用
     */
    const std::vector<std::unique_ptr<ParseError>>& getWarnings() const;

    /**
     * @brief 获取指定类型的错误
     *
     * @param type 错误类型
     * @return 指定类型的错误列表
     */
    std::vector<std::unique_ptr<ParseError>> getErrorsByType(ErrorType type) const;

    /**
     * @brief 获取指定严重程度的错误
     *
     * @param severity 错误严重程度
     * @return 指定严重程度的错误列表
     */
    std::vector<std::unique_ptr<ParseError>> getErrorsBySeverity(Severity severity) const;

    /**
     * @brief 清除所有错误和警告
     */
    void clear();

    /**
     * @brief 清除所有错误
     */
    void clearErrors();

    /**
     * @brief 清除所有警告
     */
    void clearWarnings();

    /**
     * @brief 转换为字符串表示
     *
     * @return 字符串表示
     */
    std::string toString() const;

    /**
     * @brief 转换为JSON表示
     *
     * @return JSON字符串
     */
    std::string toJson() const;
};

} // namespace errors
} // namespace sql_parser
} // namespace sqlcc
