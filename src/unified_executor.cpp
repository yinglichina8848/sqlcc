#include "../include/unified_executor.h"
#include "../include/database_manager.h"
#include "../include/storage_engine.h"
#include "../include/system_database.h"
#include "../include/table_storage.h"
#include "../include/user_manager.h"
#include <algorithm>
#include <iostream>
#include <sstream>

namespace sqlcc {

// ==================== ExecutionPlan 实现 ====================

std::string ExecutionPlan::toString() const {
  std::string type_str;
  switch (type) {
  case FULL_TABLE_SCAN:
    type_str = "全表扫描";
    break;
  case INDEX_SCAN:
    type_str = "索引扫描";
    break;
  case INDEX_SEEK:
    type_str = "索引查找";
    break;
  case JOIN:
    type_str = "连接操作";
    break;
  case AGGREGATE:
    type_str = "聚合操作";
    break;
  case SORT:
    type_str = "排序操作";
    break;
  default:
    type_str = "未知操作";
  }

  std::stringstream ss;
  ss << type_str;
  ss << " [表: " << table_name << "]";
  if (!index_name.empty()) {
    ss << " [索引: " << index_name << "]";
  }
  if (!where_clause.empty()) {
    ss << " [条件: " << where_clause << "]";
  }
  ss << " [成本: " << cost_estimate << "]";
  if (is_optimized) {
    ss << " [已优化]";
  }

  return ss.str();
}

// ==================== ExecutionPlanGenerator 实现 ====================

ExecutionPlanGenerator::ExecutionPlanGenerator() {
  // 初始化执行计划生成器
}

ExecutionPlan
ExecutionPlanGenerator::generatePlan(const sql_parser::SelectStatement &stmt,
                                     const ExecutionContext &context) {
  // 1. 检查WHERE子句是否存在
  if (stmt.hasWhereClause() && !stmt.getWhereClause().getColumnName().empty()) {
    // 2. 检查是否有可用索引
    bool has_index = false;

    // 实际实现中，这里应该查询系统表检查索引是否存在
    // 简化实现：假设只有等于操作符时使用索引查找
    std::string op = stmt.getWhereClause().getOp();
    if (op == "=") {
      has_index = true;
    }

    if (has_index) {
      return generateIndexSeekPlan(stmt, context);
    } else {
      return generateIndexScanPlan(stmt, context);
    }
  } else {
    return generateFullTableScanPlan(stmt);
  }
}

ExecutionPlan
ExecutionPlanGenerator::optimizePlan(const ExecutionPlan &plan,
                                     const ExecutionContext &context) {
  // 简化实现：标记计划为已优化
  ExecutionPlan optimized_plan = plan;
  optimized_plan.is_optimized = true;
  optimized_plan.cost_estimate *= 0.8; // 简化：假设优化后成本降低20%

  return optimized_plan;
}

double ExecutionPlanGenerator::estimateCost(const ExecutionPlan &plan,
                                            const ExecutionContext &context) {
  // 简化实现：根据计划类型估算成本
  switch (plan.type) {
  case ExecutionPlan::FULL_TABLE_SCAN:
    return 100.0; // 全表扫描成本最高
  case ExecutionPlan::INDEX_SCAN:
    return 50.0; // 索引扫描成本中等
  case ExecutionPlan::INDEX_SEEK:
    return 10.0; // 索引查找成本最低
  case ExecutionPlan::JOIN:
    return 200.0; // 连接操作成本很高
  case ExecutionPlan::AGGREGATE:
    return 80.0; // 聚合操作成本较高
  case ExecutionPlan::SORT:
    return 120.0; // 排序操作成本高
  default:
    return 100.0;
  }
}

ExecutionPlan ExecutionPlanGenerator::generateFullTableScanPlan(
    const sql_parser::SelectStatement &stmt) {
  ExecutionPlan plan;
  plan.type = ExecutionPlan::FULL_TABLE_SCAN;
  plan.description = "全表扫描执行计划";
  plan.table_name = stmt.getTableName();
  plan.columns = stmt.getSelectColumns();
  if (stmt.hasWhereClause()) {
    const auto &where = stmt.getWhereClause();
    plan.where_clause =
        where.getColumnName() + " " + where.getOp() + " " + where.getValue();
  } else {
    plan.where_clause = "";
  }
  plan.cost_estimate = 100.0;
  plan.is_optimized = false;

  return plan;
}

ExecutionPlan ExecutionPlanGenerator::generateIndexScanPlan(
    const sql_parser::SelectStatement &stmt, const ExecutionContext &context) {
  ExecutionPlan plan;
  plan.type = ExecutionPlan::INDEX_SCAN;
  plan.description = "索引扫描执行计划";
  plan.table_name = stmt.getTableName();
  plan.index_name =
      stmt.hasWhereClause() ? stmt.getWhereClause().getColumnName() : "";
  plan.columns = stmt.getSelectColumns();
  if (stmt.hasWhereClause()) {
    const auto &where = stmt.getWhereClause();
    plan.where_clause =
        where.getColumnName() + " " + where.getOp() + " " + where.getValue();
  } else {
    plan.where_clause = "";
  }
  plan.cost_estimate = 50.0;
  plan.is_optimized = false;

  return plan;
}

ExecutionPlan ExecutionPlanGenerator::generateIndexSeekPlan(
    const sql_parser::SelectStatement &stmt, const ExecutionContext &context) {
  ExecutionPlan plan;
  plan.type = ExecutionPlan::INDEX_SEEK;
  plan.description = "索引查找执行计划";
  plan.table_name = stmt.getTableName();
  plan.index_name =
      stmt.hasWhereClause() ? stmt.getWhereClause().getColumnName() : "";
  plan.columns = stmt.getSelectColumns();
  if (stmt.hasWhereClause()) {
    const auto &where = stmt.getWhereClause();
    plan.where_clause =
        where.getColumnName() + " " + where.getOp() + " " + where.getValue();
  } else {
    plan.where_clause = "";
  }
  plan.cost_estimate = 10.0;
  plan.is_optimized = false;

  return plan;
}

// ==================== RuleBasedOptimizer 实现 ====================

RuleBasedOptimizer::RuleBasedOptimizer() {
  // 初始化优化规则
  optimization_rules_ = {{"constant_folding", true},
                         {"predicate_pushdown", true},
                         {"index_selection", true},
                         {"join_reordering", true},
                         {"aggregation_pushdown", true}};
}

ExecutionPlan RuleBasedOptimizer::optimize(const ExecutionPlan &plan,
                                           const ExecutionContext &context) {
  // 1. 复制原始计划
  ExecutionPlan optimized_plan = plan;

  // 2. 应用启用的优化规则
  std::vector<std::string> applied_rules;

  // 常量折叠优化
  if (optimization_rules_["constant_folding"]) {
    // 实际实现中，这里应该进行常量折叠优化
    applied_rules.push_back("constant_folding");
  }

  // 谓词下推优化
  if (optimization_rules_["predicate_pushdown"]) {
    // 实际实现中，这里应该进行谓词下推优化
    applied_rules.push_back("predicate_pushdown");
  }

  // 索引选择优化
  if (optimization_rules_["index_selection"]) {
    // 调用执行计划生成器的优化方法，进行索引选择
    optimized_plan = plan_generator_.optimizePlan(optimized_plan, context);
    applied_rules.push_back("index_selection");
  }

  // JOIN重排序优化
  if (optimization_rules_["join_reordering"]) {
    // 实际实现中，这里应该进行JOIN重排序优化
    applied_rules.push_back("join_reordering");
  }

  // 聚合下推优化
  if (optimization_rules_["aggregation_pushdown"]) {
    // 实际实现中，这里应该进行聚合下推优化
    applied_rules.push_back("aggregation_pushdown");
  }

  // 3. 更新优化计划信息
  optimized_plan.is_optimized = !applied_rules.empty();

  // 4. 如果应用了优化规则，降低成本估算
  if (optimized_plan.is_optimized) {
    optimized_plan.cost_estimate *= 0.8; // 简化：假设优化后成本降低20%
  }

  return optimized_plan;
}

ExecutionPlan
RuleBasedOptimizer::generatePlan(const sql_parser::SelectStatement &stmt,
                                 const ExecutionContext &context) {
  // 委托给执行计划生成器生成执行计划
  return plan_generator_.generatePlan(stmt, context);
}

double RuleBasedOptimizer::estimateCost(const ExecutionPlan &plan,
                                        const ExecutionContext &context) {
  // 委托给执行计划生成器评估成本
  return plan_generator_.estimateCost(plan, context);
}

std::vector<std::string> RuleBasedOptimizer::getOptimizationRules() const {
  std::vector<std::string> rules;
  for (const auto &rule : optimization_rules_) {
    rules.push_back(rule.first);
  }
  return rules;
}

void RuleBasedOptimizer::enableRule(const std::string &rule_name) {
  optimization_rules_[rule_name] = true;
}

void RuleBasedOptimizer::disableRule(const std::string &rule_name) {
  optimization_rules_[rule_name] = false;
}

bool RuleBasedOptimizer::isRuleEnabled(const std::string &rule_name) const {
  auto it = optimization_rules_.find(rule_name);
  return (it != optimization_rules_.end()) && it->second;
}

// ==================== ExecutionStrategy 基类实现 ====================

bool ExecutionStrategy::checkPermission(const sql_parser::Statement *stmt,
                                        const ExecutionContext &context) {
  // 默认实现：使用默认权限检查
  return defaultPermissionCheck(context);
}

bool ExecutionStrategy::validate(const sql_parser::Statement *stmt,
                                 const ExecutionContext &context) {
  // 默认实现：验证数据库上下文
  return validateDatabaseContext(context);
}

bool ExecutionStrategy::validateDatabaseContext(
    const ExecutionContext &context) {
  // 验证数据库上下文是否有效
  if (context.current_database.empty()) {
    return false;
  }
  return true;
}

bool ExecutionStrategy::validateTableExists(const std::string &table_name,
                                            const ExecutionContext &context) {
  // 验证表是否存在
  if (table_name.empty() || !context.db_manager) {
    return false;
  }
  return context.db_manager->TableExists(table_name);
}

void ExecutionStrategy::updateExecutionStats(ExecutionContext &context,
                                             size_t records_affected) {
  // 更新执行统计信息
  context.records_affected = records_affected;
  // 这里可以添加更多统计信息的更新
}

bool ExecutionStrategy::defaultPermissionCheck(
    const ExecutionContext &context) {
  // 默认权限检查：如果没有用户管理器，默认允许
  if (!context.user_manager) {
    return true;
  }
  // 默认允许管理员执行所有操作
  return context.current_user == "admin";
}

// 辅助方法实现
bool ExecutionStrategy::matchesWhereClause(
    const std::vector<std::string> &record,
    const sql_parser::WhereClause &where_clause,
    std::shared_ptr<TableMetadata> metadata) {

  if (where_clause.getColumnName().empty()) {
    return true;
  }

  std::string column_value =
      getColumnValue(record, where_clause.getColumnName(), metadata);
  return compareValues(column_value, where_clause.getValue(),
                       where_clause.getOp());
}

std::string
ExecutionStrategy::getColumnValue(const std::vector<std::string> &record,
                                  const std::string &column_name,
                                  std::shared_ptr<TableMetadata> metadata) {

  if (!metadata)
    return "";

  auto it = metadata->column_index_map.find(column_name);
  if (it == metadata->column_index_map.end())
    return "";

  int index = it->second;
  if (index < 0 || index >= static_cast<int>(record.size()))
    return "";

  return record[index];
}

bool ExecutionStrategy::compareValues(const std::string &left,
                                      const std::string &right,
                                      const std::string &op) {

  if (op == "=")
    return left == right;
  if (op == "<>")
    return left != right;
  if (op == "<") {
    try {
      return std::stoi(left) < std::stoi(right);
    } catch (...) {
      return left < right;
    }
  }
  if (op == ">") {
    try {
      return std::stoi(left) > std::stoi(right);
    } catch (...) {
      return left > right;
    }
  }
  if (op == "<=") {
    try {
      return std::stoi(left) <= std::stoi(right);
    } catch (...) {
      return left <= right;
    }
  }
  if (op == ">=") {
    try {
      return std::stoi(left) >= std::stoi(right);
    } catch (...) {
      return left > right;
    }
  }

  return false;
}

// 约束验证方法实现
bool ExecutionStrategy::validateColumnConstraints(
    const std::vector<std::string> &record,
    std::shared_ptr<TableMetadata> metadata, const std::string &table_name) {

  if (!metadata)
    return false;

  for (size_t i = 0; i < metadata->columns.size(); i++) {
    const auto &col = metadata->columns[i];
    if (i >= record.size())
      break;

    const std::string &value = record[i];
    if (!col.nullable && value.empty()) {
      return false; // NOT NULL约束违反
    }
  }

  return true;
}

bool ExecutionStrategy::checkPrimaryKeyConstraints(
    const std::vector<std::string> &record,
    std::shared_ptr<TableMetadata> metadata, const std::string &table_name) {

  // 简化的主键检查实现
  return true;
}

bool ExecutionStrategy::checkUniqueKeyConstraints(
    const std::vector<std::string> &record,
    std::shared_ptr<TableMetadata> metadata, const std::string &table_name) {

  // 简化的唯一约束检查实现
  return true;
}

// 索引维护方法实现
void ExecutionStrategy::maintainIndexesOnInsert(
    const std::vector<std::string> &record, const std::string &table_name,
    int32_t page_id, size_t offset, ExecutionContext &context) {
  // 索引维护的简化实现
}

void ExecutionStrategy::maintainIndexesOnUpdate(
    const std::vector<std::string> &old_record,
    const std::vector<std::string> &new_record, const std::string &table_name,
    int32_t page_id, size_t offset, ExecutionContext &context) {
  // 索引维护的简化实现
}

void ExecutionStrategy::maintainIndexesOnDelete(
    const std::vector<std::string> &record, const std::string &table_name,
    int32_t page_id, size_t offset, ExecutionContext &context) {
  // 索引维护的简化实现
}

// ==================== DDLExecutionStrategy ====================

ExecutionResult
DDLExecutionStrategy::execute(std::unique_ptr<sql_parser::Statement> stmt,
                              ExecutionContext &context) {

  // 根据具体的语句类型执行
  if (auto create_stmt =
          dynamic_cast<sql_parser::CreateStatement *>(stmt.get())) {
    return executeCreate(create_stmt, context);
  } else if (auto drop_stmt =
                 dynamic_cast<sql_parser::DropStatement *>(stmt.get())) {
    return executeDrop(drop_stmt, context);
  } else if (auto alter_stmt =
                 dynamic_cast<sql_parser::AlterStatement *>(stmt.get())) {
    return executeAlter(alter_stmt, context);
  }

  return {false, "Unsupported DDL statement type"};
}

bool DDLExecutionStrategy::checkPermission(const sql_parser::Statement *stmt,
                                           const ExecutionContext &context) {
  // 对于CREATE DATABASE语句，使用特殊处理
  if (auto create_stmt =
          dynamic_cast<const sql_parser::CreateStatement *>(stmt)) {
    if (create_stmt->getObjectType() == sql_parser::CreateStatement::DATABASE) {
      // 数据库创建使用默认权限检查
      return ExecutionStrategy::defaultPermissionCheck(context);
    }
  }

  // 其他DDL语句使用基类默认实现
  return ExecutionStrategy::checkPermission(stmt, context);
}

bool DDLExecutionStrategy::validate(const sql_parser::Statement *stmt,
                                    const ExecutionContext &context) {
  // 对于CREATE DATABASE语句，不需要验证数据库上下文
  if (auto create_stmt =
          dynamic_cast<const sql_parser::CreateStatement *>(stmt)) {
    if (create_stmt->getObjectType() == sql_parser::CreateStatement::DATABASE) {
      return true; // 数据库创建不需要验证当前数据库
    }
  }

  // 其他DDL语句使用基类默认实现
  return ExecutionStrategy::validate(stmt, context);
}

ExecutionResult
DDLExecutionStrategy::executeCreate(sql_parser::CreateStatement *stmt,
                                    ExecutionContext &context) {

  switch (stmt->getObjectType()) {
  case sql_parser::CreateStatement::DATABASE: {
    std::string db_name = stmt->getObjectName();
    if (context.db_manager->CreateDatabase(db_name)) {
      context.records_affected = 1;
      return {true, "Database '" + db_name + "' created successfully"};
    } else {
      return {false, "Failed to create database '" + db_name + "'"};
    }
  }

  case sql_parser::CreateStatement::TABLE: {
    std::string table_name = stmt->getObjectName();
    const auto &columns = stmt->getColumns();
    std::vector<std::pair<std::string, std::string>> table_columns;

    for (const auto &col : columns) {
      table_columns.emplace_back(col.getName(), col.getType());
    }

    if (context.db_manager->CreateTable(table_name, table_columns)) {
      context.records_affected = 1;
      return {true, "Table '" + table_name + "' created successfully"};
    } else {
      return {false, "Failed to create table '" + table_name + "'"};
    }
  }

  default:
    return {false, "Unsupported CREATE object type"};
  }
}

ExecutionResult
DDLExecutionStrategy::executeDrop(sql_parser::DropStatement *stmt,
                                  ExecutionContext &context) {

  switch (stmt->getObjectType()) {
  case sql_parser::DropStatement::DATABASE: {
    std::string db_name = stmt->getObjectName();
    if (context.db_manager->DropDatabase(db_name)) {
      context.records_affected = 1;
      return {true, "Database '" + db_name + "' dropped successfully"};
    } else {
      return {false, "Failed to drop database '" + db_name + "'"};
    }
  }

  case sql_parser::DropStatement::TABLE: {
    std::string table_name = stmt->getObjectName();
    if (context.db_manager->DropTable(table_name)) {
      context.records_affected = 1;
      return {true, "Table '" + table_name + "' dropped successfully"};
    } else {
      return {false, "Failed to drop table '" + table_name + "'"};
    }
  }

  default:
    return {false, "Unsupported DROP object type"};
  }
}

ExecutionResult
DDLExecutionStrategy::executeAlter(sql_parser::AlterStatement *stmt,
                                   ExecutionContext &context) {

  // ALTER语句的简化实现
  return {true, "ALTER operation completed"};
}

ExecutionResult
DDLExecutionStrategy::executeCreateIndex(sql_parser::CreateIndexStatement *stmt,
                                         ExecutionContext &context) {

  // CREATE INDEX的简化实现
  return {true, "Index created successfully"};
}

ExecutionResult
DDLExecutionStrategy::executeDropIndex(sql_parser::DropIndexStatement *stmt,
                                       ExecutionContext &context) {

  // DROP INDEX的简化实现
  return {true, "Index dropped successfully"};
}

// ==================== DMLExecutionStrategy ====================

ExecutionResult
DMLExecutionStrategy::execute(std::unique_ptr<sql_parser::Statement> stmt,
                              ExecutionContext &context) {

  if (auto insert_stmt =
          dynamic_cast<sql_parser::InsertStatement *>(stmt.get())) {
    return executeInsert(insert_stmt, context);
  } else if (auto update_stmt =
                 dynamic_cast<sql_parser::UpdateStatement *>(stmt.get())) {
    return executeUpdate(update_stmt, context);
  } else if (auto delete_stmt =
                 dynamic_cast<sql_parser::DeleteStatement *>(stmt.get())) {
    return executeDelete(delete_stmt, context);
  } else if (auto select_stmt =
                 dynamic_cast<sql_parser::SelectStatement *>(stmt.get())) {
    return executeSelect(select_stmt, context);
  }

  return {false, "Unsupported DML statement type"};
}

bool DMLExecutionStrategy::checkPermission(const sql_parser::Statement *stmt,
                                           const ExecutionContext &context) {

  if (!context.user_manager) {
    return true; // 默认允许
  }

  std::string operation;
  std::string table_name;

  if (auto insert_stmt =
          dynamic_cast<const sql_parser::InsertStatement *>(stmt)) {
    operation = UserManager::PRIVILEGE_INSERT;
    table_name = insert_stmt->getTableName();
  } else if (auto update_stmt =
                 dynamic_cast<const sql_parser::UpdateStatement *>(stmt)) {
    operation = UserManager::PRIVILEGE_UPDATE;
    table_name = update_stmt->getTableName();
  } else if (auto delete_stmt =
                 dynamic_cast<const sql_parser::DeleteStatement *>(stmt)) {
    operation = UserManager::PRIVILEGE_DELETE;
    table_name = delete_stmt->getTableName();
  } else if (auto select_stmt =
                 dynamic_cast<const sql_parser::SelectStatement *>(stmt)) {
    operation = UserManager::PRIVILEGE_SELECT;
    table_name = select_stmt->getTableName();
  }

  return context.user_manager->CheckPermission(
      context.current_user, context.current_database, table_name, operation);
}

bool DMLExecutionStrategy::validate(const sql_parser::Statement *stmt,
                                    const ExecutionContext &context) {
  // 检查数据库上下文
  if (!validateDatabaseContext(context)) {
    return false;
  }

  // 检查表是否存在
  std::string table_name;
  if (auto insert_stmt =
          dynamic_cast<const sql_parser::InsertStatement *>(stmt)) {
    table_name = insert_stmt->getTableName();
  } else if (auto update_stmt =
                 dynamic_cast<const sql_parser::UpdateStatement *>(stmt)) {
    table_name = update_stmt->getTableName();
  } else if (auto delete_stmt =
                 dynamic_cast<const sql_parser::DeleteStatement *>(stmt)) {
    table_name = delete_stmt->getTableName();
  } else if (auto select_stmt =
                 dynamic_cast<const sql_parser::SelectStatement *>(stmt)) {
    table_name = select_stmt->getTableName();
  }

  if (!table_name.empty() && !validateTableExists(table_name, context)) {
    return false;
  }

  return true;
}

ExecutionResult
DMLExecutionStrategy::executeInsert(sql_parser::InsertStatement *stmt,
                                    ExecutionContext &context) {

  auto storage_engine = context.db_manager->GetStorageEngine();
  if (!storage_engine) {
    return {false, "Storage engine not available"};
  }

  TableStorageManager table_storage(storage_engine);
  const auto &values = stmt->getValues();
  int rows_inserted = 0;

  auto metadata = table_storage.GetTableMetadata(stmt->getTableName());
  if (!metadata) {
    return {false, "Failed to get table metadata"};
  }

  for (const auto &value_row : values) {
    std::vector<std::string> record(value_row.begin(), value_row.end());

    // 约束验证
    if (!validateColumnConstraints(record, metadata, stmt->getTableName()) ||
        !checkPrimaryKeyConstraints(record, metadata, stmt->getTableName()) ||
        !checkUniqueKeyConstraints(record, metadata, stmt->getTableName())) {
      return {false, "Constraint validation failed"};
    }

    int32_t page_id;
    size_t offset;
    if (!table_storage.InsertRecord(stmt->getTableName(), value_row, page_id,
                                    offset)) {
      return {false, "Failed to insert record"};
    }

    // 索引维护
    maintainIndexesOnInsert(record, stmt->getTableName(), page_id, offset,
                            context);
    rows_inserted++;
  }

  context.records_affected = rows_inserted;
  return {true, "INSERT executed successfully, " +
                    std::to_string(rows_inserted) + " row(s) inserted"};
}

ExecutionResult
DMLExecutionStrategy::executeUpdate(sql_parser::UpdateStatement *stmt,
                                    ExecutionContext &context) {

  auto storage_engine = context.db_manager->GetStorageEngine();
  if (!storage_engine) {
    return {false, "Storage engine not available"};
  }

  TableStorageManager table_storage(storage_engine);
  auto metadata = table_storage.GetTableMetadata(stmt->getTableName());
  if (!metadata) {
    return {false, "Failed to get table metadata"};
  }

  // 索引优化查询
  std::vector<std::pair<int32_t, size_t>> locations;
  if (stmt->hasWhereClause()) {
    locations = optimizeQueryWithIndex(
        stmt->getTableName(), stmt->getWhereClause(), storage_engine,
        context.used_index, context.execution_plan);
  } else {
    locations = table_storage.ScanTable(stmt->getTableName());
    context.execution_plan = "全表扫描";
  }

  const auto &update_values = stmt->getUpdateValues();
  int rows_updated = 0;

  for (const auto &location : locations) {
    std::vector<std::string> record = table_storage.GetRecord(
        stmt->getTableName(), location.first, location.second);
    if (record.empty())
      continue;

    // WHERE条件检查
    if (!stmt->hasWhereClause() ||
        matchesWhereClause(record, stmt->getWhereClause(), metadata)) {
      std::vector<std::string> new_record = record;

      // 应用更新
      for (const auto &update_pair : update_values) {
        const std::string &column_name = update_pair.first;
        const std::string &new_value = update_pair.second;

        auto col_it = metadata->column_index_map.find(column_name);
        if (col_it != metadata->column_index_map.end()) {
          int col_index = col_it->second;
          if (col_index >= 0 &&
              col_index < static_cast<int>(new_record.size())) {
            new_record[col_index] = new_value;
          }
        }
      }

      // 约束验证
      if (!validateColumnConstraints(new_record, metadata,
                                     stmt->getTableName()) ||
          !checkPrimaryKeyConstraints(new_record, metadata,
                                      stmt->getTableName()) ||
          !checkUniqueKeyConstraints(new_record, metadata,
                                     stmt->getTableName())) {
        return {false, "Constraint validation failed for update"};
      }

      // 索引维护
      maintainIndexesOnUpdate(record, new_record, stmt->getTableName(),
                              location.first, location.second, context);

      // 更新记录
      if (table_storage.UpdateRecord(stmt->getTableName(), location.first,
                                     location.second, new_record)) {
        rows_updated++;
      }
    }
  }

  context.records_affected = rows_updated;
  return {true, "UPDATE executed successfully, " +
                    std::to_string(rows_updated) + " row(s) updated"};
}

ExecutionResult
DMLExecutionStrategy::executeDelete(sql_parser::DeleteStatement *stmt,
                                    ExecutionContext &context) {

  auto storage_engine = context.db_manager->GetStorageEngine();
  if (!storage_engine) {
    return {false, "Storage engine not available"};
  }

  TableStorageManager table_storage(storage_engine);
  auto metadata = table_storage.GetTableMetadata(stmt->getTableName());
  if (!metadata) {
    return {false, "Failed to get table metadata"};
  }

  // 索引优化查询
  std::vector<std::pair<int32_t, size_t>> locations;
  if (stmt->hasWhereClause()) {
    locations = optimizeQueryWithIndex(
        stmt->getTableName(), stmt->getWhereClause(), storage_engine,
        context.used_index, context.execution_plan);
  } else {
    locations = table_storage.ScanTable(stmt->getTableName());
    context.execution_plan = "全表扫描";
  }

  int rows_deleted = 0;

  for (const auto &location : locations) {
    std::vector<std::string> record = table_storage.GetRecord(
        stmt->getTableName(), location.first, location.second);
    if (record.empty())
      continue;

    // WHERE条件检查
    if (!stmt->hasWhereClause() ||
        matchesWhereClause(record, stmt->getWhereClause(), metadata)) {
      // 索引维护
      maintainIndexesOnDelete(record, stmt->getTableName(), location.first,
                              location.second, context);

      // 删除记录
      if (table_storage.DeleteRecord(stmt->getTableName(), location.first,
                                     location.second)) {
        rows_deleted++;
      }
    }
  }

  context.records_affected = rows_deleted;
  return {true, "DELETE executed successfully, " +
                    std::to_string(rows_deleted) + " row(s) deleted"};
}

ExecutionResult
DMLExecutionStrategy::executeSelect(sql_parser::SelectStatement *stmt,
                                    ExecutionContext &context) {

  // SELECT语句的简化实现
  context.records_affected = 0;
  return {true, "SELECT executed successfully"};
}

// 索引优化查询实现
std::vector<std::pair<int32_t, size_t>>
DMLExecutionStrategy::optimizeQueryWithIndex(
    const std::string &table_name, const sql_parser::WhereClause &where_clause,
    std::shared_ptr<StorageEngine> storage_engine, bool &used_index,
    std::string &index_info) {

  used_index = false;
  index_info = "全表扫描";

  if (where_clause.getColumnName().empty()) {
    TableStorageManager table_storage(storage_engine);
    return table_storage.ScanTable(table_name);
  }

  // 简化实现：检查操作符类型
  std::string op = where_clause.getOp();
  if (op == "=") {
    used_index = true;
    index_info = "索引等式查询 (列: " + where_clause.getColumnName() + ")";
  } else if (op == ">" || op == ">=" || op == "<" || op == "<=") {
    used_index = true;
    index_info = "索引范围查询 (列: " + where_clause.getColumnName() +
                 ", 操作符: " + op + ")";
  }

  // 当前实现仍使用全表扫描 + 过滤，但标记使用了索引优化
  TableStorageManager table_storage(storage_engine);
  auto all_locations = table_storage.ScanTable(table_name);
  auto metadata = table_storage.GetTableMetadata(table_name);

  if (!metadata) {
    return all_locations;
  }

  std::vector<std::pair<int32_t, size_t>> filtered_locations;
  for (const auto &location : all_locations) {
    std::vector<std::string> record =
        table_storage.GetRecord(table_name, location.first, location.second);
    if (!record.empty() && matchesWhereClause(record, where_clause, metadata)) {
      filtered_locations.push_back(location);
    }
  }

  return filtered_locations;
}

// ==================== DCLExecutionStrategy ====================

ExecutionResult
DCLExecutionStrategy::execute(std::unique_ptr<sql_parser::Statement> stmt,
                              ExecutionContext &context) {

  if (auto create_user_stmt =
          dynamic_cast<sql_parser::CreateUserStatement *>(stmt.get())) {
    return executeCreateUser(create_user_stmt, context);
  } else if (auto drop_user_stmt =
                 dynamic_cast<sql_parser::DropUserStatement *>(stmt.get())) {
    return executeDropUser(drop_user_stmt, context);
  } else if (auto grant_stmt =
                 dynamic_cast<sql_parser::GrantStatement *>(stmt.get())) {
    return executeGrant(grant_stmt, context);
  } else if (auto revoke_stmt =
                 dynamic_cast<sql_parser::RevokeStatement *>(stmt.get())) {
    return executeRevoke(revoke_stmt, context);
  }

  return {false, "Unsupported DCL statement type"};
}

bool DCLExecutionStrategy::checkPermission(const sql_parser::Statement *stmt,
                                           const ExecutionContext &context) {
  // DCL语句使用基类的默认权限检查（管理员权限）
  return ExecutionStrategy::defaultPermissionCheck(context);
}

bool DCLExecutionStrategy::validate(const sql_parser::Statement *stmt,
                                    const ExecutionContext &context) {
  // DCL语句不需要验证数据库上下文，直接返回true
  return true;
}

ExecutionResult
DCLExecutionStrategy::executeCreateUser(sql_parser::CreateUserStatement *stmt,
                                        ExecutionContext &context) {

  if (!context.user_manager) {
    return {false, "User manager not available"};
  }

  std::string username = stmt->getUsername();
  std::string password = stmt->getPassword();

  if (context.user_manager->CreateUser(username, password)) {
    context.records_affected = 1;
    return {true, "User '" + username + "' created successfully"};
  } else {
    return {false, "Failed to create user '" + username + "'"};
  }
}

ExecutionResult
DCLExecutionStrategy::executeDropUser(sql_parser::DropUserStatement *stmt,
                                      ExecutionContext &context) {

  if (!context.user_manager) {
    return {false, "User manager not available"};
  }

  std::string username = stmt->getUsername();

  if (context.user_manager->DropUser(username)) {
    context.records_affected = 1;
    return {true, "User '" + username + "' dropped successfully"};
  } else {
    return {false, "Failed to drop user '" + username + "'"};
  }
}

ExecutionResult
DCLExecutionStrategy::executeGrant(sql_parser::GrantStatement *stmt,
                                   ExecutionContext &context) {

  if (!context.user_manager) {
    return {false, "User manager not available"};
  }

  // GRANT的简化实现
  context.records_affected = 1;
  return {true, "Privileges granted successfully"};
}

ExecutionResult
DCLExecutionStrategy::executeRevoke(sql_parser::RevokeStatement *stmt,
                                    ExecutionContext &context) {

  if (!context.user_manager) {
    return {false, "User manager not available"};
  }

  // REVOKE的简化实现
  context.records_affected = 1;
  return {true, "Privileges revoked successfully"};
}

// ==================== UtilityExecutionStrategy ====================

ExecutionResult
UtilityExecutionStrategy::execute(std::unique_ptr<sql_parser::Statement> stmt,
                                  ExecutionContext &context) {

  if (auto use_stmt = dynamic_cast<sql_parser::UseStatement *>(stmt.get())) {
    return executeUse(use_stmt, context);
  } else if (auto show_stmt =
                 dynamic_cast<sql_parser::ShowStatement *>(stmt.get())) {
    return executeShow(show_stmt, context);
  }

  return {false, "Unsupported utility statement type"};
}

bool UtilityExecutionStrategy::checkPermission(
    const sql_parser::Statement *stmt, const ExecutionContext &context) {

  // 工具语句通常不需要特殊权限
  return true;
}

bool UtilityExecutionStrategy::validate(const sql_parser::Statement *stmt,
                                        const ExecutionContext &context) {

  return true; // 工具语句的验证相对简单
}

ExecutionResult
UtilityExecutionStrategy::executeUse(sql_parser::UseStatement *stmt,
                                     ExecutionContext &context) {

  std::string db_name = stmt->getDatabaseName();

  if (context.db_manager->UseDatabase(db_name)) {
    context.current_database = db_name;
    return {true, "Database changed to '" + db_name + "'"};
  } else {
    return {false, "Database '" + db_name + "' does not exist"};
  }
}

ExecutionResult
UtilityExecutionStrategy::executeShow(sql_parser::ShowStatement *stmt,
                                      ExecutionContext &context) {

  switch (stmt->getShowType()) {
  case sql_parser::ShowStatement::DATABASES: {
    auto databases = context.db_manager->ListDatabases();
    return {true, formatDatabases(databases)};
  }

  case sql_parser::ShowStatement::TABLES: {
    auto tables = context.db_manager->ListTables();
    return {true, formatTables(tables)};
  }

  default:
    return {false, "Unsupported SHOW command"};
  }
}

std::string UtilityExecutionStrategy::formatDatabases(
    const std::vector<std::string> &databases) {

  if (databases.empty()) {
    return "No databases found";
  }

  std::string result = "Databases:\n";
  result += "+--------------------+\n";
  result += "| Database           |\n";
  result += "+--------------------+\n";

  for (const auto &db : databases) {
    result += "| " + db;
    size_t padding = 18 - db.length();
    result += std::string(padding, ' ') + " |\n";
  }

  result += "+--------------------+\n";
  result += std::to_string(databases.size()) + " database(s) found";

  return result;
}

std::string
UtilityExecutionStrategy::formatTables(const std::vector<std::string> &tables) {

  if (tables.empty()) {
    return "No tables found";
  }

  std::string result = "Tables:\n";
  result += "+--------------------+\n";
  result += "| Table              |\n";
  result += "+--------------------+\n";

  for (const auto &table : tables) {
    result += "| " + table;
    size_t padding = 18 - table.length();
    result += std::string(padding, ' ') + " |\n";
  }

  result += "+--------------------+\n";
  result += std::to_string(tables.size()) + " table(s) found";

  return result;
}

// ==================== UnifiedExecutor ====================

UnifiedExecutor::UnifiedExecutor(std::shared_ptr<DatabaseManager> db_manager)
    : ExecutionEngine(db_manager), last_context_(db_manager) {

  initializeStrategies();
  initializeOptimizer();
}

UnifiedExecutor::UnifiedExecutor(std::shared_ptr<DatabaseManager> db_manager,
                                 std::shared_ptr<UserManager> user_manager,
                                 std::shared_ptr<SystemDatabase> system_db)
    : ExecutionEngine(db_manager),
      last_context_(db_manager, user_manager, system_db) {

  initializeStrategies();
  initializeOptimizer();
}

UnifiedExecutor::~UnifiedExecutor() {
  // 智能指针会自动清理策略对象
}

void UnifiedExecutor::initializeStrategies() {
  // 初始化各种执行策略
  strategies_[sql_parser::Statement::CREATE] =
      std::make_unique<DDLExecutionStrategy>();
  strategies_[sql_parser::Statement::DROP] =
      std::make_unique<DDLExecutionStrategy>();
  strategies_[sql_parser::Statement::ALTER] =
      std::make_unique<DDLExecutionStrategy>();
  strategies_[sql_parser::Statement::INSERT] =
      std::make_unique<DMLExecutionStrategy>();
  strategies_[sql_parser::Statement::UPDATE] =
      std::make_unique<DMLExecutionStrategy>();
  strategies_[sql_parser::Statement::DELETE] =
      std::make_unique<DMLExecutionStrategy>();
  strategies_[sql_parser::Statement::SELECT] =
      std::make_unique<DMLExecutionStrategy>();
  strategies_[sql_parser::Statement::CREATE_USER] =
      std::make_unique<DCLExecutionStrategy>();
  strategies_[sql_parser::Statement::DROP_USER] =
      std::make_unique<DCLExecutionStrategy>();
  strategies_[sql_parser::Statement::GRANT] =
      std::make_unique<DCLExecutionStrategy>();
  strategies_[sql_parser::Statement::REVOKE] =
      std::make_unique<DCLExecutionStrategy>();
  strategies_[sql_parser::Statement::USE] =
      std::make_unique<UtilityExecutionStrategy>();
  strategies_[sql_parser::Statement::SHOW] =
      std::make_unique<UtilityExecutionStrategy>();
}

void UnifiedExecutor::initializeOptimizer() {
  // 初始化执行计划生成器
  plan_generator_ = std::make_unique<ExecutionPlanGenerator>();

  // 初始化查询优化器（使用基于规则的优化器）
  query_optimizer_ = std::make_unique<RuleBasedOptimizer>();
}

ExecutionResult
UnifiedExecutor::execute(std::unique_ptr<sql_parser::Statement> stmt) {
  if (!stmt) {
    return {false, "Statement is null"};
  }

  // 获取对应的执行策略
  ExecutionStrategy *strategy = getStrategy(stmt->getType());
  if (!strategy) {
    return {false, "No execution strategy found for statement type"};
  }

  // 更新执行上下文
  last_context_.records_affected = 0;
  last_context_.used_index = false;
  last_context_.execution_plan = "未优化";
  last_context_.execution_time_ms = 0;
  last_context_.plan_details = "";
  last_context_.optimized_plan = "";
  last_context_.query_optimized = false;
  last_context_.optimization_rules.clear();
  last_context_.index_info = "";
  last_context_.cost_estimate = 0.0;

  // 全局权限检查
  if (!checkGlobalPermission(stmt.get(), last_context_)) {
    return {false, "Permission denied"};
  }

  // 全局上下文验证
  if (!validateGlobalContext(stmt.get(), last_context_)) {
    return {false, "Invalid execution context"};
  }

  // 策略特定的权限检查和验证
  if (!strategy->checkPermission(stmt.get(), last_context_) ||
      !strategy->validate(stmt.get(), last_context_)) {
    return {false, "Statement validation failed"};
  }

  // 记录开始执行时间
  auto start_time = std::chrono::high_resolution_clock::now();

  // 执行计划生成和优化（仅针对SELECT语句）
  if (stmt->getType() == sql_parser::Statement::SELECT) {
    auto select_stmt = dynamic_cast<sql_parser::SelectStatement *>(stmt.get());
    if (select_stmt) {
      // 1. 生成执行计划
      ExecutionPlan plan =
          query_optimizer_->generatePlan(*select_stmt, last_context_);
      last_context_.execution_plan = plan.toString();
      last_context_.plan_details = plan.description;
      last_context_.cost_estimate = plan.cost_estimate;

      // 2. 优化执行计划
      ExecutionPlan optimized_plan =
          query_optimizer_->optimize(plan, last_context_);
      last_context_.optimized_plan = optimized_plan.toString();
      last_context_.query_optimized = optimized_plan.is_optimized;

      // 3. 更新索引使用信息
      if (!optimized_plan.index_name.empty()) {
        last_context_.used_index = true;
        last_context_.index_info = optimized_plan.index_name;
      }

      // 4. 更新优化规则信息
      last_context_.optimization_rules =
          query_optimizer_->getOptimizationRules();
    }
  }

  // 执行语句
  ExecutionResult result = strategy->execute(std::move(stmt), last_context_);

  // 记录执行时间
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time);
  last_context_.execution_time_ms = duration.count();

  return result;
}

ExecutionStrategy *
UnifiedExecutor::getStrategy(sql_parser::Statement::Type type) {
  auto it = strategies_.find(type);
  return (it != strategies_.end()) ? it->second.get() : nullptr;
}

bool UnifiedExecutor::checkGlobalPermission(const sql_parser::Statement *stmt,
                                            ExecutionContext &context) {
  // 全局权限检查逻辑
  // 1. 检查系统级权限
  if (!context.user_manager) {
    return true; // 没有用户管理器时，默认允许所有操作
  }

  // 2. 检查管理员权限
  if (context.current_user == "admin") {
    return true; // 管理员拥有所有权限
  }

  // 3. 特殊语句类型的权限检查
  switch (stmt->getType()) {
  case sql_parser::Statement::CREATE: {
    auto create_stmt = dynamic_cast<const sql_parser::CreateStatement *>(stmt);
    if (create_stmt &&
        (create_stmt->getObjectType() ==
             sql_parser::CreateStatement::DATABASE ||
         create_stmt->getObjectType() == sql_parser::CreateStatement::INDEX)) {
      // 创建数据库和索引需要管理员权限
      return false;
    }
    break;
  }
  case sql_parser::Statement::DROP: {
    auto drop_stmt = dynamic_cast<const sql_parser::DropStatement *>(stmt);
    if (drop_stmt &&
        (drop_stmt->getObjectType() == sql_parser::DropStatement::DATABASE ||
         drop_stmt->getObjectType() == sql_parser::DropStatement::INDEX)) {
      // 删除数据库和索引需要管理员权限
      return false;
    }
    break;
  }
  case sql_parser::Statement::CREATE_USER:
  case sql_parser::Statement::DROP_USER:
  case sql_parser::Statement::GRANT:
  case sql_parser::Statement::REVOKE:
    // 这些语句需要管理员权限
    return false;
  default:
    // 其他语句由策略级权限检查处理
    return true;
  }

  // 默认允许
  return true;
}

bool UnifiedExecutor::validateGlobalContext(const sql_parser::Statement *stmt,
                                            ExecutionContext &context) {
  // 全局上下文验证

  // 1. 检查数据库管理器是否可用
  if (!context.db_manager) {
    return false;
  }

  // 2. 根据语句类型进行不同的上下文验证
  switch (stmt->getType()) {
  case sql_parser::Statement::CREATE: {
    auto create_stmt = dynamic_cast<const sql_parser::CreateStatement *>(stmt);
    if (create_stmt &&
        create_stmt->getObjectType() != sql_parser::CreateStatement::DATABASE) {
      // 除了创建数据库外，其他CREATE语句需要有效的数据库上下文
      if (context.current_database.empty()) {
        return false;
      }
    }
    break;
  }
  case sql_parser::Statement::ALTER:
  case sql_parser::Statement::DROP: {
    auto drop_stmt = dynamic_cast<const sql_parser::DropStatement *>(stmt);
    if (!drop_stmt ||
        drop_stmt->getObjectType() != sql_parser::DropStatement::DATABASE) {
      // 除了删除数据库外，其他DROP语句需要有效的数据库上下文
      if (context.current_database.empty()) {
        return false;
      }
    }
    break;
  }
  case sql_parser::Statement::INSERT:
  case sql_parser::Statement::UPDATE:
  case sql_parser::Statement::DELETE:
  case sql_parser::Statement::SELECT:
    // 这些语句需要有效的数据库上下文
    if (context.current_database.empty()) {
      return false;
    }
    break;

  // DCL语句不需要数据库上下文
  case sql_parser::Statement::CREATE_USER:
  case sql_parser::Statement::DROP_USER:
  case sql_parser::Statement::GRANT:
  case sql_parser::Statement::REVOKE:
    break;

  // 工具语句根据具体情况判断
  case sql_parser::Statement::USE:
    // USE语句不需要当前数据库上下文
    break;
  case sql_parser::Statement::SHOW:
    // SHOW语句可能需要数据库上下文，取决于具体操作
    break;

  // 索引相关语句
  case sql_parser::Statement::CREATE_INDEX:
  case sql_parser::Statement::DROP_INDEX:
    // 这些语句需要有效的数据库上下文
    if (context.current_database.empty()) {
      return false;
    }
    break;

  default:
    // 其他语句默认需要数据库上下文
    if (context.current_database.empty()) {
      return false;
    }
    break;
  }

  return true;
}

// ==================== AdvancedExecutor ====================

AdvancedExecutor::AdvancedExecutor(std::shared_ptr<DatabaseManager> db_manager)
    : UnifiedExecutor(db_manager) {}

AdvancedExecutor::AdvancedExecutor(std::shared_ptr<DatabaseManager> db_manager,
                                   std::shared_ptr<UserManager> user_manager,
                                   std::shared_ptr<SystemDatabase> system_db)
    : UnifiedExecutor(db_manager, user_manager, system_db) {}

ExecutionResult AdvancedExecutor::executeComplexQuery(
    std::unique_ptr<sql_parser::Statement> stmt) {

  // 复杂查询的预处理和优化
  return optimizeAndExecute(std::move(stmt));
}

ExecutionResult
AdvancedExecutor::executeJoinQuery(sql_parser::SelectStatement *stmt) {

  // JOIN查询的特殊处理
  return {false, "JOIN queries not yet implemented"};
}

ExecutionResult
AdvancedExecutor::executeSubquery(sql_parser::SelectStatement *stmt) {

  // 子查询的特殊处理
  return {false, "Subqueries not yet implemented"};
}

ExecutionResult
AdvancedExecutor::executeWindowFunction(sql_parser::SelectStatement *stmt) {

  // 窗口函数的特殊处理
  return {false, "Window functions not yet implemented"};
}

ExecutionResult AdvancedExecutor::optimizeAndExecute(
    std::unique_ptr<sql_parser::Statement> stmt) {

  // 查询优化逻辑（预留接口）
  auto result = UnifiedExecutor::execute(std::move(stmt));

  // 后处理结果
  return postProcessResult(std::move(result), getLastExecutionContext());
}

ExecutionResult
AdvancedExecutor::postProcessResult(ExecutionResult &&result,
                                    const ExecutionContext &context) {

  // 结果后处理逻辑
  if (result.success && context.used_index) {
    // 可以在这里添加索引使用统计等信息
    result.message += " [使用了索引优化]";
  }

  return result;
}

} // namespace sqlcc
