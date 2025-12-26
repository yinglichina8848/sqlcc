/**
#include "sql_parser/ast_node.h"
#include "sql_parser/ast_nodes.h"
 * @file unified_executor.h
 * @brief 统一执行器头文件
 */

#ifndef SQLCC_EXECUTION_UNIFIED_EXECUTOR_H
#define SQLCC_EXECUTION_UNIFIED_EXECUTOR_H

#include <memory>
#include <vector>

#include "execution/execution_strategy.h"
#include "execution/aggregate_engine.h"
#include "execution/group_by_executor.h"
#include "core/execution_result.h"

namespace sqlcc {

namespace sql_parser {
class Statement;
class SelectStatement;
} // namespace sql_parser

class ExecutionContext;

// 统一执行器 - 统一管理各种SQL执行策略
class UnifiedExecutor : public ExecutionStrategy {
public:
    UnifiedExecutor();
    ~UnifiedExecutor() override = default;

    // 执行策略选择和执行
    ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                           ExecutionContext& context) override;
    bool checkPermission(const sql_parser::Statement& stmt,
                        const ExecutionContext& context) override;
    bool validate(const sql_parser::Statement& stmt,
                 const ExecutionContext& context) override;
    std::string getStrategyName() const override;

private:
    // 执行引擎
    std::unique_ptr<AggregateEngine> aggregate_engine_;
    std::unique_ptr<GroupByExecutor> group_by_executor_;

    // 策略选择
    ExecutionResult executeSelect(const sql_parser::SelectStatement& stmt,
                                 ExecutionContext& context);
    ExecutionResult executeInsert(const sql_parser::Statement& stmt,
                                 ExecutionContext& context);
    ExecutionResult executeUpdate(const sql_parser::Statement& stmt,
                                 ExecutionContext& context);
    ExecutionResult executeDelete(const sql_parser::Statement& stmt,
                                 ExecutionContext& context);

    // 查询优化
    bool requiresAggregation(const sql_parser::SelectStatement& stmt) const;
    bool requiresGrouping(const sql_parser::SelectStatement& stmt) const;
    bool canUseSimpleExecution(const sql_parser::SelectStatement& stmt) const;
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_UNIFIED_EXECUTOR_H
