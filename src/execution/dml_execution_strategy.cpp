#include "dml_execution_strategy.h"
#include "execution_result.h"
#include "../core/database_context.h"
#include "../core/permissions.h"
#include "../sql_parser/ast_nodes.h"
#include "../core/core_database_manager.h"
#include "execution_context.h"
#include <iostream>
#include <memory>
#include <sstream>

namespace sqlcc {

ExecutionResult DMLExecutionStrategy::execute(std::unique_ptr<sql_parser::Statement> stmt,
                                            ExecutionContext &context) {
    if (!stmt) {
        return createErrorResult("Statement is null");
    }

    // 根据语句类型调用相应的执行方法
    switch (stmt->type) {
        case sql_parser::StatementType::INSERT_STATEMENT:
            return executeInsert(dynamic_cast<const sql_parser::InsertStatement&>(*stmt),
                               context);
        case sql_parser::StatementType::UPDATE_STATEMENT:
            return executeUpdate(dynamic_cast<const sql_parser::UpdateStatement&>(*stmt),
                               context);
        case sql_parser::StatementType::DELETE_STATEMENT:
            return executeDelete(dynamic_cast<const sql_parser::DeleteStatement&>(*stmt),
                               context);
        case sql_parser::StatementType::SELECT_STATEMENT:
            return executeSelect(dynamic_cast<const sql_parser::SelectStatement&>(*stmt),
                               context);
        default:
            return createErrorResult("Unsupported DML statement type: " + 
                                   std::to_string(static_cast<int>(stmt->type)));
    }
}

bool DMLExecutionStrategy::checkPermission(const sql_parser::Statement& stmt,
                                         const ExecutionContext& context) {
    // 权限检查实现
    switch (stmt.type) {
        case sql_parser::StatementType::INSERT_STATEMENT:
            return checkInsertPermission(dynamic_cast<const sql_parser::InsertStatement&>(stmt),
                                       context);
        case sql_parser::StatementType::UPDATE_STATEMENT:
            return checkUpdatePermission(dynamic_cast<const sql_parser::UpdateStatement&>(stmt),
                                       context);
        case sql_parser::StatementType::DELETE_STATEMENT:
            return checkDeletePermission(dynamic_cast<const sql_parser::DeleteStatement&>(stmt),
                                       context);
        case sql_parser::StatementType::SELECT_STATEMENT:
            return checkSelectPermission(dynamic_cast<const sql_parser::SelectStatement&>(stmt),
                                       context);
        default:
            return false;
    }
}

bool DMLExecutionStrategy::validate(const sql_parser::Statement& stmt,
                                  const ExecutionContext& context) {
    // 验证实现
    if (!context.db_manager) {
        return false;
    }

    switch (stmt.type) {
        case sql_parser::StatementType::INSERT_STATEMENT: {
            const auto& insert_stmt = dynamic_cast<const sql_parser::InsertStatement&>(stmt);
            // 验证表是否存在
            if (!validateTableExists(insert_stmt.table_name, context)) {
                return false;
            }
            // 验证插入的列和值是否匹配
            auto metadata = context.db_manager->GetTableMetadata(insert_stmt.table_name);
            if (!metadata) {
                return false;
            }
            if (!insert_stmt.columns.empty() && insert_stmt.values.size() != insert_stmt.columns.size()) {
                return false;
            }
            return true;
        }
        case sql_parser::StatementType::SELECT_STATEMENT: {
            const auto& select_stmt = dynamic_cast<const sql_parser::SelectStatement&>(stmt);
            // 验证表是否存在
            for (const auto& table : select_stmt.from_clause.tables) {
                if (!validateTableExists(table.name, context)) {
                    return false;
                }
            }
            return true;
        }
        case sql_parser::StatementType::UPDATE_STATEMENT: {
            const auto& update_stmt = dynamic_cast<const sql_parser::UpdateStatement&>(stmt);
            // 验证表是否存在
            if (!validateTableExists(update_stmt.table_name, context)) {
                return false;
            }
            return true;
        }
        case sql_parser::StatementType::DELETE_STATEMENT: {
            const auto& delete_stmt = dynamic_cast<const sql_parser::DeleteStatement&>(stmt);
            // 验证表是否存在
            if (!validateTableExists(delete_stmt.table_name, context)) {
                return false;
            }
            return true;
        }
        default:
            return false;
    }
}

std::string DMLExecutionStrategy::getStrategyName() const {
    return "DMLExecutionStrategy";
}

// 私有方法实现
ExecutionResult DMLExecutionStrategy::executeInsert(const sql_parser::InsertStatement& stmt,
                                                  ExecutionContext &context) {
    // 插入语句执行逻辑
    if (!context.db_manager) {
        return createErrorResult("Database manager is not available");
    }

    // 执行插入操作
    auto result = context.db_manager->InsertRecord(stmt.table_name, stmt.columns, stmt.values);
    if (result.success) {
        updateExecutionStats(context, 1); // 影响一行
        return createSuccessResult("Insert successful");
    } else {
        return createErrorResult(result.error_message);
    }
}

ExecutionResult DMLExecutionStrategy::executeUpdate(const sql_parser::UpdateStatement& stmt,
                                                  ExecutionContext &context) {
    // 更新语句执行逻辑
    if (!context.db_manager) {
        return createErrorResult("Database manager is not available");
    }

    // 执行更新操作
    auto result = context.db_manager->UpdateRecords(stmt.table_name, stmt.set_clause, stmt.where_clause);
    if (result.success) {
        updateExecutionStats(context, result.rows_affected);
        return createSuccessResult("Update successful, " + std::to_string(result.rows_affected) + " rows affected");
    } else {
        return createErrorResult(result.error_message);
    }
}

ExecutionResult DMLExecutionStrategy::executeDelete(const sql_parser::DeleteStatement& stmt,
                                                  ExecutionContext &context) {
    // 删除语句执行逻辑
    if (!context.db_manager) {
        return createErrorResult("Database manager is not available");
    }

    // 执行删除操作
    auto result = context.db_manager->DeleteRecords(stmt.table_name, stmt.where_clause);
    if (result.success) {
        updateExecutionStats(context, result.rows_affected);
        return createSuccessResult("Delete successful, " + std::to_string(result.rows_affected) + " rows affected");
    } else {
        return createErrorResult(result.error_message);
    }
}

ExecutionResult DMLExecutionStrategy::executeSelect(const sql_parser::SelectStatement& stmt,
                                                  ExecutionContext &context) {
    // 选择语句执行逻辑
    if (!context.db_manager) {
        return createErrorResult("Database manager is not available");
    }

    // 根据SELECT语句的复杂程度选择不同的执行方法
    if (stmt.join_clause.has_value()) {
        return executeJoinSelect(stmt, context);
    } else if (stmt.group_by_clause.has_value()) {
        return executeGroupBySelect(stmt, context);
    } else if (stmt.aggregate_functions.size() > 0) {
        return executeAggregateSelect(stmt, context);
    } else {
        return executeSimpleSelect(stmt, context);
    }
}

ExecutionResult DMLExecutionStrategy::executeJoinSelect(const sql_parser::SelectStatement& stmt,
                                                      ExecutionContext &context) {
    // JOIN查询执行逻辑
    if (!context.db_manager) {
        return createErrorResult("Database manager is not available");
    }

    // 执行JOIN查询
    auto result = context.db_manager->ExecuteJoinQuery(stmt);
    if (result.success) {
        updateExecutionStats(context, result.rows_affected);
        return result; // 返回查询结果
    } else {
        return createErrorResult(result.error_message);
    }
}

ExecutionResult DMLExecutionStrategy::executeGroupBySelect(const sql_parser::SelectStatement& stmt,
                                                         ExecutionContext &context) {
    // GROUP BY查询执行逻辑
    if (!context.db_manager) {
        return createErrorResult("Database manager is not available");
    }

    // 执行GROUP BY查询
    auto result = context.db_manager->ExecuteGroupByQuery(stmt);
    if (result.success) {
        updateExecutionStats(context, result.rows_affected);
        return result; // 返回查询结果
    } else {
        return createErrorResult(result.error_message);
    }
}

ExecutionResult DMLExecutionStrategy::executeAggregateSelect(const sql_parser::SelectStatement& stmt,
                                                           ExecutionContext &context) {
    // 聚合查询执行逻辑
    if (!context.db_manager) {
        return createErrorResult("Database manager is not available");
    }

    // 执行聚合查询
    auto result = context.db_manager->ExecuteAggregateQuery(stmt);
    if (result.success) {
        updateExecutionStats(context, result.rows_affected);
        return result; // 返回查询结果
    } else {
        return createErrorResult(result.error_message);
    }
}

ExecutionResult DMLExecutionStrategy::executeSimpleSelect(const sql_parser::SelectStatement& stmt,
                                                        ExecutionContext &context) {
    // 简单查询执行逻辑
    if (!context.db_manager) {
        return createErrorResult("Database manager is not available");
    }

    // 执行简单查询
    auto result = context.db_manager->ExecuteSimpleQuery(stmt);
    if (result.success) {
        updateExecutionStats(context, result.rows_affected);
        return result; // 返回查询结果
    } else {
        return createErrorResult(result.error_message);
    }
}

} // namespace sqlcc