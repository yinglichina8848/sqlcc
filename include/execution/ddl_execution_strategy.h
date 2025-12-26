/**
#include "sql_parser/ast_node.h"
#include "sql_parser/ast_nodes.h"
 * @file ddl_execution_strategy.h
 * @brief DDL执行策略头文件
 */

#ifndef SQLCC_EXECUTION_DDL_EXECUTION_STRATEGY_H
#define SQLCC_EXECUTION_DDL_EXECUTION_STRATEGY_H

#include <memory>
#include <string>

#include "execution/execution_strategy.h"
#include "core/execution_result.h"

namespace sqlcc {

namespace sql_parser {
class Statement;
class CreateTableStatement;
class DropTableStatement;
class AlterTableStatement;
class CreateIndexStatement;
class DropIndexStatement;
} // namespace sql_parser

class ExecutionContext;

// DDL执行策略 - 处理数据定义语言语句
class DDLExecutionStrategy : public ExecutionStrategy {
public:
    DDLExecutionStrategy();
    ~DDLExecutionStrategy() override = default;

    // 执行DDL语句
    ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                           ExecutionContext& context) override;
    bool checkPermission(const sql_parser::Statement& stmt,
                        const ExecutionContext& context) override;
    bool validate(const sql_parser::Statement& stmt,
                 const ExecutionContext& context) override;
    std::string getStrategyName() const override;

private:
    // DDL语句处理
    ExecutionResult executeCreateTable(const sql_parser::CreateTableStatement& stmt,
                                      ExecutionContext& context);
    ExecutionResult executeDropTable(const sql_parser::DropTableStatement& stmt,
                                    ExecutionContext& context);
    ExecutionResult executeAlterTable(const sql_parser::AlterTableStatement& stmt,
                                     ExecutionContext& context);
    ExecutionResult executeCreateIndex(const sql_parser::CreateIndexStatement& stmt,
                                      ExecutionContext& context);
    ExecutionResult executeDropIndex(const sql_parser::DropIndexStatement& stmt,
                                    ExecutionContext& context);

    // 验证方法
    bool validateTableName(const std::string& table_name, const ExecutionContext& context) const;
    bool validateIndexName(const std::string& index_name, const ExecutionContext& context) const;
    bool validateColumnDefinitions(const std::vector<sql_parser::ColumnDefinition>& columns) const;
    bool checkTableExists(const std::string& table_name, const ExecutionContext& context) const;
    bool checkIndexExists(const std::string& index_name, const ExecutionContext& context) const;

    // 权限检查
    bool hasCreateTablePermission(const ExecutionContext& context) const;
    bool hasDropTablePermission(const std::string& table_name, const ExecutionContext& context) const;
    bool hasAlterTablePermission(const std::string& table_name, const ExecutionContext& context) const;
    bool hasCreateIndexPermission(const ExecutionContext& context) const;
    bool hasDropIndexPermission(const std::string& index_name, const ExecutionContext& context) const;
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_DDL_EXECUTION_STRATEGY_H
