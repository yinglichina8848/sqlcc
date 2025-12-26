/**
#include "sql_parser/ast_node.h"
 * @file execution_strategy.h
 * @brief 执行策略基类头文件
 */

#ifndef SQLCC_EXECUTION_EXECUTION_STRATEGY_H
#define SQLCC_EXECUTION_EXECUTION_STRATEGY_H

#include <memory>
#include <string>

#include "core/execution_result.h"

namespace sqlcc {

namespace sql_parser {
class Statement;
} // namespace sql_parser

class ExecutionContext;

// 执行策略基类 - 定义SQL语句执行的策略模式
class ExecutionStrategy {
public:
    ExecutionStrategy();
    virtual ~ExecutionStrategy() = default;

    // 执行SQL语句
    virtual ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                                   ExecutionContext& context) = 0;

    // 权限检查
    virtual bool checkPermission(const sql_parser::Statement& stmt,
                                const ExecutionContext& context) = 0;

    // 语句验证
    virtual bool validate(const sql_parser::Statement& stmt,
                         const ExecutionContext& context) = 0;

    // 获取策略名称
    virtual std::string getStrategyName() const = 0;

protected:
    // 辅助方法
    bool hasRequiredPermissions(const ExecutionContext& context,
                               const std::vector<std::string>& required_permissions) const;

    ExecutionResult createErrorResult(const std::string& error_message,
                                     ExecutionResult::ErrorCode error_code = ExecutionResult::ErrorCode::EXECUTION_ERROR) const;

    ExecutionResult createSuccessResult(const std::string& message = "") const;
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_EXECUTION_STRATEGY_H
