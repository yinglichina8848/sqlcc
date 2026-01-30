#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

namespace sqlcc {
namespace sql_executor {

// Forward declarations
namespace sql_parser {
class AlterTableStatement;
class AddColumnAction;
class DropColumnAction;
class ModifyColumnAction;
class RenameColumnAction;
class AddConstraintAction;
class DropConstraintAction;
class RenameTableAction;
} // namespace sql_parser

/**
 * Enhanced Alter Table Manager - 增强ALTER TABLE管理器
 * 处理ALTER TABLE语句的高级功能，包括列操作、约束操作、表重命名等
 */
class EnhancedAlterTableManager {
public:
    static EnhancedAlterTableManager& getInstance();

    // 主要ALTER TABLE操作
    bool alterTable(const sql_parser::AlterTableStatement& stmt);

    // 列操作
    bool addColumn(const std::string& tableName, const sql_parser::AddColumnAction& action);
    bool dropColumn(const std::string& tableName, const sql_parser::DropColumnAction& action);
    bool modifyColumn(const std::string& tableName, const sql_parser::ModifyColumnAction& action);
    bool renameColumn(const std::string& tableName, const sql_parser::RenameColumnAction& action);

    // 约束操作
    bool addConstraint(const std::string& tableName, const sql_parser::AddConstraintAction& action);
    bool dropConstraint(const std::string& tableName, const sql_parser::DropConstraintAction& action);

    // 表操作
    bool renameTable(const sql_parser::RenameTableAction& action);

    // 验证操作
    bool validateAlterTable(const sql_parser::AlterTableStatement& stmt);
    bool canAddColumn(const std::string& tableName, const std::string& columnName) const;
    bool canDropColumn(const std::string& tableName, const std::string& columnName) const;
    bool canModifyColumn(const std::string& tableName, const std::string& columnName) const;

    // 依赖管理
    std::vector<std::string> getColumnDependencies(const std::string& tableName, const std::string& columnName) const;
    bool hasColumnDependencies(const std::string& tableName, const std::string& columnName) const;

    // 统计信息
    std::string getAlterTableStatistics() const;

private:
    EnhancedAlterTableManager() = default;

    struct AlterOperation {
        std::string tableName;
        std::string operationType;
        std::string details;
        long timestamp;
        bool success;
    };

    std::vector<AlterOperation> operation_history_;
    long next_operation_id_ = 1;

    // 内部辅助方法
    bool executeColumnAction(const std::string& tableName, const std::string& actionType, const std::string& columnName);
    bool executeConstraintAction(const std::string& tableName, const std::string& actionType, const std::string& constraintName);
    void recordOperation(const std::string& tableName, const std::string& operationType,
                        const std::string& details, bool success);
};

} // namespace sql_executor
} // namespace sqlcc
