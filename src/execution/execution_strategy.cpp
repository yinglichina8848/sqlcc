/**
 * @file execution_strategy.cpp
 * @brief 执行策略基类实现
 */

#include "execution_strategy.h"
#include "../core/execution_result.h"
#include "../core/execution_context.h"
#include <algorithm>

namespace sqlcc {

/**
 * @brief 创建错误执行结果
 */
ExecutionResult ExecutionStrategy::createErrorResult(
    const std::string& error_message) const {
    ExecutionResult result(false, error_message);
    result.add_error(error_message);
    return result;
}

/**
 * @brief 创建成功执行结果
 */
ExecutionResult ExecutionStrategy::createSuccessResult(
    const std::string& message) const {
    return ExecutionResult(true, message);
}

/**
 * @brief 检查是否具有所需权限
 */
bool ExecutionStrategy::hasRequiredPermissions(
    const ExecutionContext& context,
    const std::vector<std::string>& required_permissions) const {
    (void)context;
    (void)required_permissions;
    return true;
}

/**
 * @brief 验证数据库上下文
 */
bool ExecutionStrategy::validateDatabaseContext(const ExecutionContext& context) {
    return context.db_manager != nullptr;
}

/**
 * @brief 验证表是否存在
 */
bool ExecutionStrategy::validateTableExists(const std::string& table_name,
                                           const ExecutionContext& context) {
    if (!context.db_manager) {
        return false;
    }
    return context.db_manager->TableExists(table_name);
}

/**
 * @brief 更新执行统计信息
 */
void ExecutionStrategy::updateExecutionStats(ExecutionContext& context,
                                            size_t records_affected) {
    context.increment_rows_affected(records_affected);
}

/**
 * @brief 生成默认权限检查结果
 */
bool ExecutionStrategy::defaultPermissionCheck(const ExecutionContext& context) {
    (void)context;
    return true;
}

/**
 * @brief 匹配WHERE子句
 */
bool ExecutionStrategy::matchesWhereClause(
    const std::vector<std::string>& record,
    const sql_parser::WhereClause& where_clause,
    std::shared_ptr<TableMetadata> metadata) {
    (void)record;
    (void)where_clause;
    (void)metadata;
    return true;
}

/**
 * @brief 获取列值
 */
std::string ExecutionStrategy::getColumnValue(
    const std::vector<std::string>& record,
    const std::string& column_name,
    std::shared_ptr<TableMetadata> metadata) {
    (void)column_name;
    (void)metadata;
    if (!record.empty()) {
        return record[0];
    }
    return "";
}

/**
 * @brief 比较两个值
 */
bool ExecutionStrategy::compareValues(const std::string& left,
                                     const std::string& right,
                                     const std::string& op) {
    if (op == "=") {
        return left == right;
    } else if (op == "!=") {
        return left != right;
    } else if (op == ">") {
        return left > right;
    } else if (op == ">=") {
        return left >= right;
    } else if (op == "<") {
        return left < right;
    } else if (op == "<=") {
        return left <= right;
    }
    return false;
}

/**
 * @brief 验证列约束
 */
bool ExecutionStrategy::validateColumnConstraints(
    const std::vector<std::string>& record,
    std::shared_ptr<TableMetadata> metadata,
    const std::string& table_name) {
    (void)table_name;
    if (!metadata) {
        return false;
    }
    return true;
}

/**
 * @brief 检查主键约束
 */
bool ExecutionStrategy::checkPrimaryKeyConstraints(
    const std::vector<std::string>& record,
    std::shared_ptr<TableMetadata> metadata,
    const std::string& table_name) {
    (void)record;
    (void)metadata;
    (void)table_name;
    return true;
}

/**
 * @brief 检查唯一键约束
 */
bool ExecutionStrategy::checkUniqueKeyConstraints(
    const std::vector<std::string>& record,
    std::shared_ptr<TableMetadata> metadata,
    const std::string& table_name) {
    (void)record;
    (void)metadata;
    (void)table_name;
    return true;
}

/**
 * @brief 插入后维护索引
 */
void ExecutionStrategy::maintainIndexesOnInsert(
    const std::vector<std::string>& record,
    const std::string& table_name,
    int32_t page_id,
    size_t offset,
    ExecutionContext& context) {
    (void)record;
    (void)table_name;
    (void)page_id;
    (void)offset;
    (void)context;
}

/**
 * @brief 更新后维护索引
 */
void ExecutionStrategy::maintainIndexesOnUpdate(
    const std::vector<std::string>& old_record,
    const std::vector<std::string>& new_record,
    const std::string& table_name,
    int32_t page_id,
    size_t offset,
    ExecutionContext& context) {
    (void)old_record;
    (void)new_record;
    (void)table_name;
    (void)page_id;
    (void)offset;
    (void)context;
}

/**
 * @brief 删除后维护索引
 */
void ExecutionStrategy::maintainIndexesOnDelete(
    const std::vector<std::string>& record,
    const std::string& table_name,
    int32_t page_id,
    size_t offset,
    ExecutionContext& context) {
    (void)record;
    (void)table_name;
    (void)page_id;
    (void)offset;
    (void)context;
}

} // namespace sqlcc
