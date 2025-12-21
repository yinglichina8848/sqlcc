/**
 * @file dml_execution_strategy.h
 * @brief DML执行策略头文件
 */

#ifndef SQLCC_EXECUTION_DML_EXECUTION_STRATEGY_H
#define SQLCC_EXECUTION_DML_EXECUTION_STRATEGY_H

#include <string>
#include <vector>
#include <memory>

#include "core/execution_result.h"
#include "execution/execution_strategy.h"

namespace sqlcc {

class TableMetadata;

// DML执行策略类
class DMLExecutionStrategy : public ExecutionStrategy {
public:
    DMLExecutionStrategy();
    ~DMLExecutionStrategy() override = default;

    ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                           ExecutionContext &context) override;
    bool checkPermission(const sql_parser::Statement& stmt,
                        const ExecutionContext &context) override;
    bool validate(const sql_parser::Statement& stmt,
                 const ExecutionContext &context) override;

private:
    ExecutionResult executeInsert(const sql_parser::InsertStatement& stmt,
                                 ExecutionContext &context);
    ExecutionResult executeUpdate(const sql_parser::UpdateStatement& stmt,
                                 ExecutionContext &context);
    ExecutionResult executeDelete(const sql_parser::DeleteStatement& stmt,
                                 ExecutionContext &context);
    ExecutionResult executeSelect(const sql_parser::SelectStatement& stmt,
                                 ExecutionContext &context);

    ExecutionResult executeJoinSelect(const sql_parser::SelectStatement& stmt,
                                     ExecutionContext &context);
    ExecutionResult executeGroupBySelect(const sql_parser::SelectStatement& stmt,
                                        ExecutionContext &context);
    ExecutionResult executeAggregateSelect(const sql_parser::SelectStatement& stmt,
                                          ExecutionContext &context);
    ExecutionResult executeSimpleSelect(const sql_parser::SelectStatement& stmt,
                                       ExecutionContext &context);
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_DML_EXECUTION_STRATEGY_H
