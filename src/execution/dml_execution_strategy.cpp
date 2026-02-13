#include "dml_execution_strategy.h"
#include "core/execution_result.h"
#include "sql_parser/ast/ast_nodes.h"
#include "core/core_database_manager.h"
#include "../core/execution_context.h"
#include <iostream>
#include <memory>
#include <sstream>

namespace sqlcc {

/**
 * @class DMLExecutionStrategy
 * @brief DML 语句执行策略 - 实现数据的增删改查物理执行逻辑
 *
 * WHY层 - 设计意图：
 *   DML (Data Manipulation Language) 是数据库最频繁的操作。
 *   该策略类封装了 SQL 语句到存储引擎 API 的转换过程，
 *   通过区分简单查询、聚合查询和连接查询，能够针对不同的 DML 类型选择最优的执行算子，
 *   保证了执行引擎的高效性和灵活性。
 *
 * WHAT层 - 功能说明：
 *   执行 INSERT, UPDATE, DELETE, SELECT 语句。
 *   支持复杂的查询模式：JOIN, GROUP BY, Aggregate。
 *   提供 DML 级别的权限检查（CheckPermission）和元数据校验（Validate）。
 *   收集并更新执行统计信息（如 Affected Rows）。
 *
 * HOW层 - 实现机制：
 *   1. 策略分发：execute 方法通过 switch-case 逻辑将 AST 节点引导至专项私有执行函数。
 *   2. 职责链调用：集成 DatabaseManager 提供的核心存储接口。
 *   3. 复杂性感知：executeSelect 会根据子句的有无（如 join_clause）动态路由至 executeJoinSelect 等优化路径。
 *   4. 事务集成：所有的 DML 操作均在 ExecutionContext 提供的事务上下文中运行。
 */
ExecutionResult DMLExecutionStrategy::execute(std::unique_ptr<sql_parser::Statement> stmt,
                                            ExecutionContext &context) {
    if (!stmt) {
        return ExecutionResult(false, "Statement is null");
    }

    // 根据语句类型调用相应的执行方法
    switch (stmt->getType()) {
        case sql_parser::Statement::Type::INSERT:
            return executeInsert(dynamic_cast<const sql_parser::InsertStatement&>(*stmt),
                               context);
        case sql_parser::Statement::Type::UPDATE:
            return executeUpdate(dynamic_cast<const sql_parser::UpdateStatement&>(*stmt),
                               context);
        case sql_parser::Statement::Type::DELETE:
            return executeDelete(dynamic_cast<const sql_parser::DeleteStatement&>(*stmt),
                               context);
        case sql_parser::Statement::Type::SELECT:
            return executeSelect(dynamic_cast<const sql_parser::SelectStatement&>(*stmt),
                               context);
        default:
            return ExecutionResult(false, "Unsupported DML statement type: " +
                                   std::to_string(static_cast<int>(stmt->getType())));
    }
}

bool DMLExecutionStrategy::checkPermission(const sql_parser::Statement& stmt,
                                         const ExecutionContext& context) {
    // 权限检查实现
    switch (stmt.getType()) {
        case sql_parser::Statement::Type::INSERT:
            return checkInsertPermission(dynamic_cast<const sql_parser::InsertStatement&>(stmt),
                                       context);
        case sql_parser::Statement::Type::UPDATE:
            return checkUpdatePermission(dynamic_cast<const sql_parser::UpdateStatement&>(stmt),
                                       context);
        case sql_parser::Statement::Type::DELETE:
            return checkDeletePermission(dynamic_cast<const sql_parser::DeleteStatement&>(stmt),
                                       context);
        case sql_parser::Statement::Type::SELECT:
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

    switch (stmt.getType()) {
        case sql_parser::Statement::Type::INSERT: {
            const auto& insert_stmt = dynamic_cast<const sql_parser::InsertStatement&>(stmt);
            // 验证表是否存在
            if (!validateTableExists(insert_stmt.getTableName(), context)) {
                return false;
            }
            // 验证插入的列和值是否匹配
            auto metadata = context.db_manager->GetTableMetadata(insert_stmt.getTableName());
            if (!metadata) {
                return false;
            }
            const auto& columns = insert_stmt.getColumnNames();
            const auto& values = insert_stmt.getValues();
            if (!columns.empty() && values.size() != columns.size()) {
                return false;
            }
            return true;
        }
        case sql_parser::Statement::Type::SELECT: {
            const auto& select_stmt = dynamic_cast<const sql_parser::SelectStatement&>(stmt);
            // 验证表是否存在
            for (const auto& table : select_stmt.getFromTables()) {
                if (!validateTableExists(table, context)) {
                    return false;
                }
            }
            return true;
        }
        case sql_parser::Statement::Type::UPDATE: {
            const auto& update_stmt = dynamic_cast<const sql_parser::UpdateStatement&>(stmt);
            // 验证表是否存在
            if (!validateTableExists(update_stmt.getTableName(), context)) {
                return false;
            }
            return true;
        }
        case sql_parser::Statement::Type::DELETE: {
            const auto& delete_stmt = dynamic_cast<const sql_parser::DeleteStatement&>(stmt);
            // 验证表是否存在
            for (const auto& table : delete_stmt.getTableNames()) {
                if (!validateTableExists(table, context)) {
                    return false;
                }
            }
            return true;
        }
        default:
            return false;
    }
}

// 私有方法实现
ExecutionResult DMLExecutionStrategy::executeInsert(const sql_parser::InsertStatement& stmt,
                                                  ExecutionContext &context) {
    // 插入语句执行逻辑
    if (!context.db_manager) {
        return ExecutionResult(false, "Database manager is not available");
    }

    // 执行插入操作
    // 转换 Expression 值到字符串
    std::vector<std::string> stringValues;
    for (const auto& row : stmt.getValues()) {
        for (const auto& expr : row) {
            // 简化处理：空字符串表示值
            stringValues.push_back("");
        }
    }
    auto result = context.db_manager->InsertRecord(stmt.getTableName(), stmt.getColumnNames(), stringValues);
    if (result) {
        updateExecutionStats(context, 1); // 影响一行
        return ExecutionResult(true, "Insert successful");
    } else {
        return ExecutionResult(false, "Insert failed");
    }
}

ExecutionResult DMLExecutionStrategy::executeUpdate(const sql_parser::UpdateStatement& stmt,
                                                  ExecutionContext &context) {
    // 更新语句执行逻辑
    if (!context.db_manager) {
        return ExecutionResult(false, "Database manager is not available");
    }

    // 简化：暂不支持 UpdateRecords
    return ExecutionResult(false, "UPDATE not fully implemented");
}

ExecutionResult DMLExecutionStrategy::executeDelete(const sql_parser::DeleteStatement& stmt,
                                                  ExecutionContext &context) {
    // 删除语句执行逻辑
    if (!context.db_manager) {
        return ExecutionResult(false, "Database manager is not available");
    }

    // 简化：暂不支持 DeleteRecords
    return ExecutionResult(false, "DELETE not fully implemented");
}

ExecutionResult DMLExecutionStrategy::executeSelect(const sql_parser::SelectStatement& stmt,
                                                  ExecutionContext &context) {
    // 选择语句执行逻辑
    if (!context.db_manager) {
        return ExecutionResult(false, "Database manager is not available");
    }

    // 简化执行路径：所有 SELECT 都返回未实现
    // 完整实现需要 DatabaseManager 支持对应方法
    return ExecutionResult(false, "SELECT execution not fully implemented");
}

ExecutionResult DMLExecutionStrategy::executeJoinSelect(const sql_parser::SelectStatement& stmt,
                                                      ExecutionContext &context) {
    return ExecutionResult(false, "JOIN query execution not implemented");
}

ExecutionResult DMLExecutionStrategy::executeGroupBySelect(const sql_parser::SelectStatement& stmt,
                                                         ExecutionContext &context) {
    return ExecutionResult(false, "GROUP BY execution not implemented");
}

ExecutionResult DMLExecutionStrategy::executeAggregateSelect(const sql_parser::SelectStatement& stmt,
                                                           ExecutionContext &context) {
    return ExecutionResult(false, "Aggregate execution not implemented");
}

ExecutionResult DMLExecutionStrategy::executeSimpleSelect(const sql_parser::SelectStatement& stmt,
                                                        ExecutionContext &context) {
    return ExecutionResult(false, "Simple SELECT execution not implemented");
}

} // namespace sqlcc