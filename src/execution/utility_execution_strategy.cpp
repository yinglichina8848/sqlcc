#include "utility_execution_strategy.h"
#include "../core/execution_result.h"
#include "../core/execution_context.h"
#include "../core/permission_validator.h"
#include "../sql_parser/ast/ast_nodes.h"
#include "../core/core_database_manager.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <algorithm>

namespace sqlcc {

ExecutionResult UtilityExecutionStrategy::execute(std::unique_ptr<sql_parser::Statement> stmt,
                                                ExecutionContext &context) {
    if (!stmt) {
        return createErrorResult("Statement is null");
    }

    switch (stmt->getType()) {
        case sql_parser::Statement::Type::USE: {
            auto use_stmt = dynamic_cast<sql_parser::UseStatement*>(stmt.get());
            if (use_stmt) {
                return executeUse(*use_stmt, context);
            }
            break;
        }
        case sql_parser::Statement::Type::SHOW: {
            auto show_stmt = dynamic_cast<sql_parser::ShowStatement*>(stmt.get());
            if (show_stmt) {
                return executeShow(*show_stmt, context);
            }
            break;
        }
        // TODO: DescribeStatement is not defined, use ShowStatement::COLUMNS instead
        // case sql_parser::Statement::Type::DESCRIBE: {
        //     auto desc_stmt = dynamic_cast<sql_parser::DescribeStatement*>(stmt.get());
        //     if (desc_stmt) {
        //         return executeDescribe(*desc_stmt, context);
        //     }
        //     break;
        // }
        default:
            return createErrorResult("Unsupported utility statement type");
    }

    return createErrorResult("Failed to execute utility statement");
}

bool UtilityExecutionStrategy::checkPermission(const sql_parser::Statement& stmt,
                                             const ExecutionContext &context) {
    // 对于大多数实用程序命令，普通用户可以执行
    return true;
}

bool UtilityExecutionStrategy::validate(const sql_parser::Statement& stmt,
                                      const ExecutionContext &context) {
    // 验证实用程序语句的基本有效性
    switch (stmt.getType()) {
        case sql_parser::Statement::Type::USE:
        case sql_parser::Statement::Type::SHOW:
        // TODO: DescribeStatement is not defined
        // case sql_parser::Statement::Type::DESCRIBE:
            return true;
        default:
            return false;
    }
}

ExecutionResult UtilityExecutionStrategy::executeUse(const sql_parser::UseStatement& stmt,
                                                   ExecutionContext &context) {
    if (auto db_manager = context.get_db_manager()) {
        // 尝试切换到指定的数据库
        if (db_manager->UseDatabase(stmt.getDatabaseName())) {
            // 更新执行上下文中的当前数据库
            context.set_current_database(stmt.getDatabaseName());
            return createSuccessResult("Database changed to " + stmt.getDatabaseName());
        } else {
            return createErrorResult("Database '" + stmt.getDatabaseName() + "' does not exist");
        }
    }
    return createErrorResult("Database manager not available");
}

ExecutionResult UtilityExecutionStrategy::executeShow(const sql_parser::ShowStatement& stmt,
                                                    ExecutionContext &context) {
    if (auto db_manager = context.get_db_manager()) {
        std::ostringstream result_stream;

        if (stmt.getShowType() == sql_parser::ShowStatement::ShowType::DATABASES) {
            // 显示所有数据库
            auto databases = db_manager->ListDatabases();
            result_stream << "Databases:\n";
            for (const auto& db : databases) {
                result_stream << "- " << db << "\n";
            }
        } else if (stmt.getShowType() == sql_parser::ShowStatement::ShowType::TABLES) {
            // 显示当前数据库中的所有表
            auto current_db = context.get_current_database();
            if (!current_db.empty()) {
                auto tables = db_manager->ListTables();
                result_stream << "Tables in " << current_db << ":\n";
                for (const auto& table : tables) {
                    result_stream << "- " << table << "\n";
                }
            } else {
                return createErrorResult("No database selected. Use USE command first.");
            }
        } else if (stmt.getShowType() == sql_parser::ShowStatement::ShowType::COLUMNS) {
            // 显示指定表的列信息
            auto current_db = context.get_current_database();
            if (!current_db.empty() && !stmt.getTargetObject().empty()) {
                auto table_metadata = db_manager->GetTableMetadata(stmt.getTargetObject());
                if (table_metadata) {
                    result_stream << "Columns in " << current_db << "." << stmt.getTargetObject() << ":\n";
                    // TODO: Get columns from table_metadata
                    // for (const auto& column : table_metadata->columns) {
                    //     result_stream << "- " << column.name << " (" << column.type << ")\n";
                    // }
                } else {
                    return createErrorResult("Table '" + stmt.getTargetObject() + "' does not exist or has no schema");
                }
            } else {
                return createErrorResult("No database selected or table name not specified.");
            }
        }

        return createSuccessResult(result_stream.str());
    }
    return createErrorResult("Database manager not available");
}

// TODO: DescribeStatement is not defined, use ShowStatement::COLUMNS instead
// ExecutionResult UtilityExecutionStrategy::executeDescribe(const sql_parser::DescribeStatement& stmt,
//                                                         ExecutionContext &context) {
//     if (auto db_manager = context.getDatabaseManager()) {
//         auto current_db = context.getCurrentDatabase();
//         if (!current_db.empty() && !stmt.table_name.empty()) {
//             auto table_schema = db_manager->getTableSchema(current_db, stmt.table_name);
//             if (!table_schema.empty()) {
//                 std::ostringstream result_stream;
//                 result_stream << "Structure of table " << stmt.table_name << ":\n";
//                 result_stream << "Columns:\n";
//                 for (const auto& column : table_schema) {
//                     result_stream << "  " << column.name << " " << column.type;
//                     if (column.is_primary_key) {
//                         result_stream << " PRIMARY KEY";
//                     }
//                     if (column.is_not_null) {
//                         result_stream << " NOT NULL";
//                     }
//                     if (!column.default_value.empty()) {
//                         result_stream << " DEFAULT " << column.default_value;
//                     }
//                     result_stream << "\n";
//                 }
//                 
//                 // 获取表的索引信息
//                 auto indexes = db_manager->getTableIndexes(current_db, stmt.table_name);
//                 if (!indexes.empty()) {
//                     result_stream << "Indexes:\n";
//                     for (const auto& index : indexes) {
//                         result_stream << "  " << index.name << " ON (" << index.columns << ")\n";
//                     }
//                 }
//                 
//                 return createSuccessResult(result_stream.str());
//             } else {
//                 return createErrorResult("Table '" + stmt.table_name + "' does not exist or has no schema");
//             }
//         } else {
//             return createErrorResult("Database and table name must be specified for DESCRIBE command");
//         }
//     }
//     return createErrorResult("Database manager not available");
// }

} // namespace sqlcc