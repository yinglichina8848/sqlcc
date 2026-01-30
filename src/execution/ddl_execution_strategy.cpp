#include "ddl_execution_strategy.h"
#include "../core/execution_result.h"
#include "../core/execution_context.h"
#include "../core/permission_validator.h"
#include "../sql_parser/ast/ast_nodes.h"
#include "../core/core_database_manager.h"
#include <iostream>
#include <memory>
#include <sstream>

namespace sqlcc {

ExecutionResult DDLExecutionStrategy::execute(std::unique_ptr<sql_parser::Statement> stmt,
                                            ExecutionContext &context) {
    if (!stmt) {
        return createErrorResult("Statement is null");
    }

    // 根据语句类型调用相应的执行方法
    switch (stmt->type) {
        case sql_parser::StatementType::CREATE_TABLE_STATEMENT:
            return executeCreateTable(dynamic_cast<const sql_parser::CreateTableStatement&>(*stmt),
                                    context);
        case sql_parser::StatementType::DROP_TABLE_STATEMENT:
            return executeDropTable(dynamic_cast<const sql_parser::DropTableStatement&>(*stmt),
                                  context);
        case sql_parser::StatementType::ALTER_TABLE_STATEMENT:
            return executeAlterTable(dynamic_cast<const sql_parser::AlterTableStatement&>(*stmt),
                                   context);
        case sql_parser::StatementType::CREATE_INDEX_STATEMENT:
            return executeCreateIndex(dynamic_cast<const sql_parser::CreateIndexStatement&>(*stmt),
                                    context);
        case sql_parser::StatementType::DROP_INDEX_STATEMENT:
            return executeDropIndex(dynamic_cast<const sql_parser::DropIndexStatement&>(*stmt),
                                  context);
        default:
            return createErrorResult("Unsupported DDL statement type: " + 
                                   std::to_string(static_cast<int>(stmt->type)));
    }
}

bool DDLExecutionStrategy::checkPermission(const sql_parser::Statement& stmt,
                                         const ExecutionContext& context) {
    // 权限检查实现
    switch (stmt.type) {
        case sql_parser::StatementType::CREATE_TABLE_STATEMENT:
            return checkCreateTablePermission(dynamic_cast<const sql_parser::CreateTableStatement&>(stmt),
                                            context);
        case sql_parser::StatementType::DROP_TABLE_STATEMENT:
            return checkDropTablePermission(dynamic_cast<const sql_parser::DropTableStatement&>(stmt),
                                          context);
        case sql_parser::StatementType::ALTER_TABLE_STATEMENT:
            return checkAlterTablePermission(dynamic_cast<const sql_parser::AlterTableStatement&>(stmt),
                                           context);
        case sql_parser::StatementType::CREATE_INDEX_STATEMENT:
            return checkCreateIndexPermission(dynamic_cast<const sql_parser::CreateIndexStatement&>(stmt),
                                            context);
        case sql_parser::StatementType::DROP_INDEX_STATEMENT:
            return checkDropIndexPermission(dynamic_cast<const sql_parser::DropIndexStatement&>(stmt),
                                          context);
        default:
            return false;
    }
}

bool DDLExecutionStrategy::validate(const sql_parser::Statement& stmt,
                                  const ExecutionContext& context) {
    // 验证实现
    if (!context.db_manager) {
        return false;
    }

    switch (stmt.type) {
        case sql_parser::StatementType::CREATE_TABLE_STATEMENT: {
            const auto& create_table_stmt = dynamic_cast<const sql_parser::CreateTableStatement&>(stmt);
            // 验证表名是否已存在
            if (context.db_manager->GetTableMetadata(create_table_stmt.table_name)) {
                return false; // 表已存在
            }
            // 验证列定义是否有效
            for (const auto& column : create_table_stmt.columns) {
                if (column.name.empty()) {
                    return false;
                }
            }
            return true;
        }
        case sql_parser::StatementType::DROP_TABLE_STATEMENT: {
            const auto& drop_table_stmt = dynamic_cast<const sql_parser::DropTableStatement&>(stmt);
            // 验证表是否存在
            if (!context.db_manager->GetTableMetadata(drop_table_stmt.table_name)) {
                return false; // 表不存在
            }
            return true;
        }
        case sql_parser::StatementType::ALTER_TABLE_STATEMENT: {
            const auto& alter_table_stmt = dynamic_cast<const sql_parser::AlterTableStatement&>(stmt);
            // 验证表是否存在
            if (!context.db_manager->GetTableMetadata(alter_table_stmt.table_name)) {
                return false; // 表不存在
            }
            return true;
        }
        case sql_parser::StatementType::CREATE_INDEX_STATEMENT: {
            const auto& create_index_stmt = dynamic_cast<const sql_parser::CreateIndexStatement&>(stmt);
            // 验证表是否存在
            if (!context.db_manager->GetTableMetadata(create_index_stmt.table_name)) {
                return false; // 表不存在
            }
            return true;
        }
        case sql_parser::StatementType::DROP_INDEX_STATEMENT: {
            const auto& drop_index_stmt = dynamic_cast<const sql_parser::DropIndexStatement&>(stmt);
            // 验证索引是否存在（这里简化处理）
            return true;
        }
        default:
            return false;
    }
}

std::string DDLExecutionStrategy::getStrategyName() const {
    return "DDLExecutionStrategy";
}

// 私有方法实现
ExecutionResult DDLExecutionStrategy::executeCreateTable(const sql_parser::CreateTableStatement& stmt,
                                                      ExecutionContext &context) {
    // 创建表执行逻辑
    if (!context.db_manager) {
        return createErrorResult("Database manager is not available");
    }

    // 执行创建表操作
    auto result = context.db_manager->CreateTable(stmt.table_name, stmt.columns);
    if (result.success) {
        updateExecutionStats(context, 1); // 创建一个表
        return createSuccessResult("Create table successful");
    } else {
        return createErrorResult(result.error_message);
    }
}

ExecutionResult DDLExecutionStrategy::executeDropTable(const sql_parser::DropTableStatement& stmt,
                                                    ExecutionContext &context) {
    // 删除表执行逻辑
    if (!context.db_manager) {
        return createErrorResult("Database manager is not available");
    }

    // 执行删除表操作
    auto result = context.db_manager->DropTable(stmt.table_name);
    if (result.success) {
        updateExecutionStats(context, 1); // 删除一个表
        return createSuccessResult("Drop table successful");
    } else {
        return createErrorResult(result.error_message);
    }
}

ExecutionResult DDLExecutionStrategy::executeAlterTable(const sql_parser::AlterTableStatement& stmt,
                                                     ExecutionContext &context) {
    // 修改表执行逻辑
    if (!context.db_manager) {
        return createErrorResult("Database manager is not available");
    }

    // 根据ALTER操作类型执行相应操作
    auto result = context.db_manager->AlterTable(stmt.table_name, stmt.alter_operations);
    if (result.success) {
        updateExecutionStats(context, 1); // 修改一个表
        return createSuccessResult("Alter table successful");
    } else {
        return createErrorResult(result.error_message);
    }
}

ExecutionResult DDLExecutionStrategy::executeCreateIndex(const sql_parser::CreateIndexStatement& stmt,
                                                      ExecutionContext &context) {
    // 创建索引执行逻辑
    if (!context.db_manager) {
        return createErrorResult("Database manager is not available");
    }

    // 执行创建索引操作
    auto result = context.db_manager->CreateIndex(stmt.index_name, stmt.table_name, stmt.column_names);
    if (result.success) {
        updateExecutionStats(context, 1); // 创建一个索引
        return createSuccessResult("Create index successful");
    } else {
        return createErrorResult(result.error_message);
    }
}

ExecutionResult DDLExecutionStrategy::executeDropIndex(const sql_parser::DropIndexStatement& stmt,
                                                    ExecutionContext &context) {
    // 删除索引执行逻辑
    if (!context.db_manager) {
        return createErrorResult("Database manager is not available");
    }

    // 执行删除索引操作
    auto result = context.db_manager->DropIndex(stmt.index_name, stmt.table_name);
    if (result.success) {
        updateExecutionStats(context, 1); // 删除一个索引
        return createSuccessResult("Drop index successful");
    } else {
        return createErrorResult(result.error_message);
    }
}

} // namespace sqlcc