#include "ast_nodes.h"
#include <algorithm>
#include <stdexcept>
#include <unordered_set>

namespace sqlcc {
namespace sql_parser {

// ==================== PrimaryKeyValidator 实现 ====================

PrimaryKeyValidator::PrimaryKeyValidator(const std::vector<std::string>& columns,
                                       const std::string& constraint_name)
    : columns_(columns), constraint_name_(constraint_name) {}

PrimaryKeyValidator::~PrimaryKeyValidator() {}

bool PrimaryKeyValidator::validate(const std::vector<std::string>& record,
                                 std::shared_ptr<TableMetadata> metadata,
                                 const std::string& table_name) const {
    // 检查主键列是否为空
    for (const auto& col_name : columns_) {
        auto col_index = metadata->column_index_map.find(col_name);
        if (col_index == metadata->column_index_map.end()) {
            throw std::runtime_error("Primary key column '" + col_name + "' not found in table '" + table_name + "'");
        }

        size_t index = col_index->second;
        if (index >= record.size()) {
            throw std::runtime_error("Primary key validation failed: record size mismatch");
        }

        // 检查是否为空（主键不允许为空）
        if (record[index].empty()) {
            throw std::runtime_error("Primary key constraint violation: column '" + col_name +
                                   "' cannot be null in table '" + table_name + "'");
        }
    }

    // TODO: 检查主键值的唯一性（需要在表级别维护唯一索引）
    // 这里简化实现，实际应该查询存储引擎检查是否已存在相同的主键值

    return true;
}

std::string PrimaryKeyValidator::getConstraintName() const {
    return constraint_name_.empty() ? "PRIMARY KEY" : constraint_name_;
}

// ==================== UniqueKeyValidator 实现 ====================

UniqueKeyValidator::UniqueKeyValidator(const std::vector<std::string>& columns,
                                     const std::string& constraint_name)
    : columns_(columns), constraint_name_(constraint_name) {}

UniqueKeyValidator::~UniqueKeyValidator() {}

bool UniqueKeyValidator::validate(const std::vector<std::string>& record,
                                std::shared_ptr<TableMetadata> metadata,
                                const std::string& table_name) const {
    // 检查唯一键列是否存在
    for (const auto& col_name : columns_) {
        auto col_index = metadata->column_index_map.find(col_name);
        if (col_index == metadata->column_index_map.end()) {
            throw std::runtime_error("Unique key column '" + col_name + "' not found in table '" + table_name + "'");
        }
    }

    // 构造唯一键值组合
    std::vector<std::string> key_values;
    for (const auto& col_name : columns_) {
        auto col_index = metadata->column_index_map.find(col_name);
        size_t index = col_index->second;
        if (index >= record.size()) {
            throw std::runtime_error("Unique key validation failed: record size mismatch");
        }
        key_values.push_back(record[index]);
    }

    // 检查唯一性
    if (!checkUniqueness(key_values)) {
        std::string error_msg = "Unique constraint violation in table '" + table_name + "': ";
        for (size_t i = 0; i < columns_.size(); ++i) {
            if (i > 0) error_msg += ", ";
            error_msg += columns_[i] + "='" + key_values[i] + "'";
        }
        throw std::runtime_error(error_msg);
    }

    return true;
}

bool UniqueKeyValidator::checkUniqueness(const std::vector<std::string>& key_values) const {
    // TODO: 实现真正的唯一性检查逻辑
    // 这里应该：
    // 1. 获取存储引擎实例
    // 2. 扫描表查找是否存在相同的键值组合
    // 3. 或者使用索引进行快速查找
    // 目前简化实现：假设唯一性检查总是通过
    // 实际实现需要与存储引擎集成

    return true;
}

std::string UniqueKeyValidator::getConstraintName() const {
    return constraint_name_.empty() ? "UNIQUE KEY" : constraint_name_;
}

// ==================== ForeignKeyValidator 实现 ====================

ForeignKeyValidator::ForeignKeyValidator(const std::vector<std::string>& columns,
                                       const std::string& referenced_table,
                                       const std::vector<std::string>& referenced_columns,
                                       const std::string& constraint_name)
    : columns_(columns), referenced_table_(referenced_table),
      referenced_columns_(referenced_columns), constraint_name_(constraint_name),
      on_delete_action_(RESTRICT), on_update_action_(RESTRICT) {}

void ForeignKeyValidator::setOnDeleteAction(CascadeAction action) {
    on_delete_action_ = action;
}

void ForeignKeyValidator::setOnUpdateAction(CascadeAction action) {
    on_update_action_ = action;
}

ForeignKeyValidator::CascadeAction ForeignKeyValidator::getOnDeleteAction() const {
    return on_delete_action_;
}

ForeignKeyValidator::CascadeAction ForeignKeyValidator::getOnUpdateAction() const {
    return on_update_action_;
}

ForeignKeyValidator::~ForeignKeyValidator() {}

bool ForeignKeyValidator::validate(const std::vector<std::string>& record,
                                 std::shared_ptr<TableMetadata> metadata,
                                 const std::string& table_name) const {
    // 提取外键列的值
    std::vector<std::string> fk_values;
    for (const auto& col_name : columns_) {
        auto col_index = metadata->column_index_map.find(col_name);
        if (col_index == metadata->column_index_map.end()) {
            throw std::runtime_error("Foreign key column '" + col_name + "' not found in table '" + table_name + "'");
        }

        size_t index = col_index->second;
        if (index >= record.size()) {
            throw std::runtime_error("Foreign key validation failed: record size mismatch");
        }

        fk_values.push_back(record[index]);
    }

    // 检查引用完整性（简化实现）
    if (!checkReferenceExists(fk_values, referenced_table_)) {
        std::string error_msg = "Foreign key constraint violation in table '" + table_name +
                               "': referenced record not found in table '" + referenced_table_ + "'";
        throw std::runtime_error(error_msg);
    }

    return true;
}

std::string ForeignKeyValidator::getConstraintName() const {
    return constraint_name_.empty() ? "FOREIGN KEY" : constraint_name_;
}

bool ForeignKeyValidator::checkReferenceExists(const std::vector<std::string>& values,
                                             const std::string& ref_table) const {
    // TODO: 实现真正的引用检查逻辑
    // 这里应该查询存储引擎检查被引用表中是否存在对应的记录

    // 简化实现：假设引用总是存在
    // 实际实现需要：
    // 1. 获取被引用表的存储引擎
    // 2. 查询是否存在匹配的记录
    // 3. 如果不存在，根据级联操作决定如何处理

    // 检查是否有NULL值（外键可以为NULL）
    for (const auto& value : values) {
        if (value.empty()) {
            return true;  // NULL值被认为是有效的
        }
    }

    return true;  // 简化实现，总是返回true
}

// ==================== CheckConstraintValidator 实现 ====================

CheckConstraintValidator::CheckConstraintValidator(const std::string& expression,
                                                 const std::string& constraint_name)
    : expression_(expression), constraint_name_(constraint_name) {}

CheckConstraintValidator::~CheckConstraintValidator() {}

bool CheckConstraintValidator::validate(const std::vector<std::string>& record,
                                      std::shared_ptr<TableMetadata> metadata,
                                      const std::string& table_name) const {
    // 评估检查表达式
    if (!evaluateExpression(expression_, record, metadata)) {
        throw std::runtime_error("Check constraint violation in table '" + table_name +
                               "': expression '" + expression_ + "' evaluated to false");
    }

    return true;
}

std::string CheckConstraintValidator::getConstraintName() const {
    return constraint_name_.empty() ? "CHECK" : constraint_name_;
}

bool CheckConstraintValidator::evaluateExpression(const std::string& expression,
                                                const std::vector<std::string>& record,
                                                std::shared_ptr<TableMetadata> metadata) const {
    // TODO: 实现完整的表达式评估器
    // 这里提供简化实现，支持基本的比较操作

    // 简化实现：假设表达式总是为真
    // 实际应该解析表达式如 "age > 18" 或 "status IN ('active', 'inactive')"

    // 示例：解析简单的 "column > value" 表达式
    std::string expr = expression;
    // 移除空格
    expr.erase(std::remove_if(expr.begin(), expr.end(), ::isspace), expr.end());

    // 查找操作符
    size_t gt_pos = expr.find('>');
    size_t lt_pos = expr.find('<');
    size_t eq_pos = expr.find('=');

    if (gt_pos != std::string::npos) {
        std::string col_name = expr.substr(0, gt_pos);
        std::string value_str = expr.substr(gt_pos + 1);

        auto col_index = metadata->column_index_map.find(col_name);
        if (col_index != metadata->column_index_map.end()) {
            size_t index = col_index->second;
            if (index < record.size()) {
                try {
                    double col_value = std::stod(record[index]);
                    double check_value = std::stod(value_str);
                    return col_value > check_value;
                } catch (const std::exception&) {
                    // 类型转换失败，简化处理
                    return true;
                }
            }
        }
    }

    // 默认返回true（简化实现）
    return true;
}

// ==================== ConstraintManager 实现 ====================

ConstraintManager& ConstraintManager::getInstance() {
    static ConstraintManager instance;
    return instance;
}

ConstraintManager::ConstraintManager() {}

void ConstraintManager::addValidator(const std::string& table_name,
                                   std::unique_ptr<ConstraintValidator> validator) {
    validators_[table_name].push_back(std::move(validator));
}

void ConstraintManager::removeValidator(const std::string& table_name,
                                      const std::string& constraint_name) {
    auto table_it = validators_.find(table_name);
    if (table_it != validators_.end()) {
        auto& table_validators = table_it->second;
        table_validators.erase(
            std::remove_if(table_validators.begin(), table_validators.end(),
                [&constraint_name](const std::unique_ptr<ConstraintValidator>& validator) {
                    return validator->getConstraintName() == constraint_name;
                }),
            table_validators.end());
    }
}

bool ConstraintManager::validateRecord(const std::vector<std::string>& record,
                                     std::shared_ptr<TableMetadata> metadata,
                                     const std::string& table_name) const {
    auto table_it = validators_.find(table_name);
    if (table_it != validators_.end()) {
        for (const auto& validator : table_it->second) {
            if (!validator->validate(record, metadata, table_name)) {
                return false;
            }
        }
    }
    return true;
}

std::vector<const ConstraintValidator*> ConstraintManager::getValidators(const std::string& table_name) const {
    std::vector<const ConstraintValidator*> result;
    auto table_it = validators_.find(table_name);
    if (table_it != validators_.end()) {
        for (const auto& validator : table_it->second) {
            result.push_back(validator.get());
        }
    }
    return result;
}

void ConstraintManager::clearValidators(const std::string& table_name) {
    validators_.erase(table_name);
}

} // namespace sql_parser
} // namespace sqlcc
