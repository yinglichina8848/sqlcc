/**
 * @file dcl_execution_strategy.h
 * @brief DCL执行策略头文件
 */

#ifndef SQLCC_EXECUTION_DCL_EXECUTION_STRATEGY_H
#define SQLCC_EXECUTION_DCL_EXECUTION_STRATEGY_H

#include <string>
#include <memory>

#include "src/execution/execution_strategy.h"
#include "src/core/execution_result.h"
#include "src/sql_parser/ast/ast_nodes.h"

namespace sqlcc {

// DCL执行策略类
class DCLExecutionStrategy : public ExecutionStrategy {
public:
    DCLExecutionStrategy() = default;
    ~DCLExecutionStrategy() override = default;

    ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                           ExecutionContext &context) override;
    bool checkPermission(const sql_parser::Statement& stmt,
                        const ExecutionContext &context) override;
    bool validate(const sql_parser::Statement& stmt,
                 const ExecutionContext &context) override;
    std::string getStrategyName() const override { return "DCLExecutionStrategy"; }

private:
    ExecutionResult executeCreateUser(const sql_parser::CreateUserStatement& stmt,
                                     ExecutionContext &context);
    ExecutionResult executeDropUser(const sql_parser::DropUserStatement& stmt,
                                   ExecutionContext &context);
    ExecutionResult executeGrant(const sql_parser::GrantStatement& stmt,
                                ExecutionContext &context);
    ExecutionResult executeRevoke(const sql_parser::RevokeStatement& stmt,
                                 ExecutionContext &context);
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_DCL_EXECUTION_STRATEGY_H