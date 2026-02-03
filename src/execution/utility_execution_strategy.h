/**
 * @file utility_execution_strategy.h
 * @brief 工具执行策略头文件
 */

#ifndef SQLCC_EXECUTION_UTILITY_EXECUTION_STRATEGY_H
#define SQLCC_EXECUTION_UTILITY_EXECUTION_STRATEGY_H

#include <string>
#include <memory>

#include "execution_strategy.h"
#include "core/execution_result.h"
#include "core/execution_context.h"
#include "sql_parser/ast/ast_nodes.h"

namespace sqlcc {

// 工具执行策略类
class UtilityExecutionStrategy : public ExecutionStrategy {
public:
    UtilityExecutionStrategy() = default;
    ~UtilityExecutionStrategy() override = default;

    ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                           ExecutionContext &context) override;
    bool checkPermission(const sql_parser::Statement& stmt,
                        const ExecutionContext &context) override;
    bool validate(const sql_parser::Statement& stmt,
                 const ExecutionContext &context) override;
    std::string getStrategyName() const override { return "UtilityExecutionStrategy"; }

private:
    ExecutionResult executeUse(const sql_parser::UseStatement& stmt,
                              ExecutionContext &context);
    ExecutionResult executeShow(const sql_parser::ShowStatement& stmt,
                               ExecutionContext &context);
    // TODO: DescribeStatement is not defined, use ShowStatement::COLUMNS instead
    // ExecutionResult executeDescribe(const sql_parser::DescribeStatement& stmt,
    //                                ExecutionContext &context);
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_UTILITY_EXECUTION_STRATEGY_H