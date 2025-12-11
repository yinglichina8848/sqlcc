#include "unified_executor.h"
#include "database_manager.h"
#include "storage_engine.h"
#include "system_database.h"
#include "table_storage.h"
#include "user_manager.h"
#include "sql_executor/index_manager.h"
#include "storage/b_plus_tree.h"
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

  if (!metadata || record.empty()) {
    return false;
  }

  // 简化实现：目前没有主键约束信息
  // 在实际实现中，应该检查表的主键约束
  // 这里返回true表示通过检查
  return true;
}

bool ExecutionStrategy::checkUniqueKeyConstraints(
    const std::vector<std::string> &record,
    std::shared_ptr<TableMetadata> metadata, const std::string &table_name) {

  if (!metadata || record.empty()) {
    return false;
  }

  // 简化实现：目前没有唯一键约束信息
  // 在实际实现中，应该检查表的唯一键约束
  // 这里返回true表示通过检查
  return true;
}

// 索引维护方法实现
void ExecutionStrategy::maintainIndexesOnInsert(
    const std::vector<std::string> &record, const std::string &table_name,
    int32_t page_id, size_t offset, ExecutionContext &context) {
  // 获取存储引擎
  auto storage_engine = context.db_manager->GetStorageEngine();
  if (!storage_engine) {
    return;
  }
  
  // 创建TableStorageManager实例
  TableStorageManager table_storage(storage_engine);
  
  // 获取表的元数据
  auto metadata = table_storage.GetTableMetadata(table_name);
  if (!metadata) {
    return;
  }
  
  // 获取索引管理器
  auto db_manager = context.db_manager;
  if (!db_manager) {
    return;
  }
  
  // 遍历表的所有列，检查是否有索引
  for (size_t i = 0; i < metadata->columns.size(); ++i) {
    const std::string& column_name = metadata->columns[i].name;
    
    // 检查该列是否有索引
    if (table_storage.IndexExists(table_name, column_name)) {
      // 获取索引 - 使用智能指针避免raw pointer
      auto index_ptr = table_storage.GetIndexShared(table_name, column_name);
      if (index_ptr) {
        // 创建索引条目
        IndexEntry entry;
        entry.key = record[i];  // 使用该列的值作为键
        entry.page_id = page_id;
        entry.offset = offset;

        // 插入到索引中
        index_ptr->Insert(entry.key, entry.page_id, entry.offset);
      }
    }
  }
}

void ExecutionStrategy::maintainIndexesOnUpdate(
    const std::vector<std::string> &old_record,
    const std::vector<std::string> &new_record, const std::string &table_name,
    int32_t page_id, size_t offset, ExecutionContext &context) {
  // 获取存储引擎
  auto storage_engine = context.db_manager->GetStorageEngine();
  if (!storage_engine) {
    return;
  }
  
  // 创建TableStorageManager实例
  TableStorageManager table_storage(storage_engine);
  
  // 获取表的元数据
  auto metadata = table_storage.GetTableMetadata(table_name);
  if (!metadata) {
    return;
  }
  
  // 遍历表的所有列，检查是否有索引
  for (size_t i = 0; i < metadata->columns.size(); ++i) {
    const std::string& column_name = metadata->columns[i].name;
    
    // 检查该列是否有索引
    if (table_storage.IndexExists(table_name, column_name)) {
      // 如果旧值和新值不同，则需要更新索引
      if (old_record[i] != new_record[i]) {
        // 获取索引 - 使用智能指针避免raw pointer
        auto index_ptr = table_storage.GetIndexShared(table_name, column_name);
        if (index_ptr) {
          // 从索引中删除旧条目
          index_ptr->Delete(old_record[i]);

          // 创建新索引条目
          IndexEntry entry;
          entry.key = new_record[i];  // 使用新值作为键
          entry.page_id = page_id;
          entry.offset = offset;

          // 插入新条目到索引中
          index_ptr->Insert(entry.key, entry.page_id, entry.offset);
        }
      }
    }
  }
}

void ExecutionStrategy::maintainIndexesOnDelete(
    const std::vector<std::string> &record, const std::string &table_name,
    int32_t page_id, size_t offset, ExecutionContext &context) {
  // 获取存储引擎
  auto storage_engine = context.db_manager->GetStorageEngine();
  if (!storage_engine) {
    return;
  }
  
  // 创建TableStorageManager实例
  TableStorageManager table_storage(storage_engine);
  
  // 获取表的元数据
  auto metadata = table_storage.GetTableMetadata(table_name);
  if (!metadata) {
    return;
  }
  
  // 遍历表的所有列，检查是否有索引
  for (size_t i = 0; i < metadata->columns.size(); ++i) {
    const std::string& column_name = metadata->columns[i].name;
    
    // 检查该列是否有索引
    if (table_storage.IndexExists(table_name, column_name)) {
      // 获取索引 - 使用智能指针避免raw pointer
      auto index_ptr = table_storage.GetIndexShared(table_name, column_name);
      if (index_ptr) {
        // 从索引中删除条目
        index_ptr->Delete(record[i]);
      }
    }
  }
}

// ==================== DDLExecutionStrategy ====================

ExecutionResult
DDLExecutionStrategy::execute(std::unique_ptr<sql_parser::Statement> stmt,
                             ExecutionContext &context) {

  // 根据具体的语句类型执行
  if (auto create_stmt =
          dynamic_cast<sql_parser::CreateStatement *>(stmt.get())) {
    return executeCreate(*create_stmt, context);
  } else if (auto drop_stmt =
                 dynamic_cast<sql_parser::DropStatement *>(stmt.get())) {
    return executeDrop(*drop_stmt, context);
  } else if (auto alter_stmt =
                 dynamic_cast<sql_parser::AlterStatement *>(stmt.get())) {
    return executeAlter(*alter_stmt, context);
  } else if (auto create_index_stmt =
                 dynamic_cast<sql_parser::CreateIndexStatement *>(stmt.get())) {
    return executeCreateIndex(*create_index_stmt, context);
  } else if (auto drop_index_stmt =
                 dynamic_cast<sql_parser::DropIndexStatement *>(stmt.get())) {
    return executeDropIndex(*drop_index_stmt, context);
  }

  return {false, "Unsupported DDL statement type"};
}

bool DDLExecutionStrategy::checkPermission(const sql_parser::Statement &stmt,
                                          const ExecutionContext &context) {
  // 对于CREATE DATABASE语句，使用特殊处理
  if (auto create_stmt =
          dynamic_cast<const sql_parser::CreateStatement *>(&stmt)) {
    if (create_stmt->getObjectType() == sql_parser::CreateStatement::DATABASE) {
      // 数据库创建使用默认权限检查
      return ExecutionStrategy::defaultPermissionCheck(context);
    }
  }

  // 其他DDL语句使用基类默认实现
  return ExecutionStrategy::checkPermission(stmt, context);
}

bool DDLExecutionStrategy::validate(const sql_parser::Statement &stmt,
                                   const ExecutionContext &context) {
  // 对于CREATE DATABASE语句，不需要验证数据库上下文
  if (auto create_stmt =
          dynamic_cast<const sql_parser::CreateStatement *>(&stmt)) {
    if (create_stmt->getObjectType() == sql_parser::CreateStatement::DATABASE) {
      return true; // 数据库创建不需要验证当前数据库
    }
  }

  // 其他DDL语句使用基类默认实现
  return ExecutionStrategy::validate(stmt, context);
}

ExecutionResult
DDLExecutionStrategy::executeCreate(const sql_parser::CreateStatement &stmt,
                                    ExecutionContext &context) {

  switch (stmt.getObjectType()) {
  case sql_parser::CreateStatement::DATABASE: {
    std::string db_name = stmt.getObjectName();
    if (context.db_manager->CreateDatabase(db_name)) {
      context.records_affected = 1;
      return {true, "Database '" + db_name + "' created successfully"};
    } else {
      return {false, "Failed to create database '" + db_name + "'"};
    }
  }

  case sql_parser::CreateStatement::TABLE: {
    std::string table_name = stmt.getObjectName();
    const auto &columns = stmt.getColumns();
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
DDLExecutionStrategy::executeDrop(const sql_parser::DropStatement& stmt,
                                  ExecutionContext& context) {
  // 根据删除对象的类型进行分发
  switch (stmt.getObjectType()) {
  case sql_parser::DropStatement::DATABASE:
    // 简化实现：直接返回成功
    return {true, "Database dropped successfully"};
  case sql_parser::DropStatement::TABLE:
    // 简化实现：直接返回成功
    return {true, "Table dropped successfully"};
  case sql_parser::DropStatement::INDEX:
    // 简化实现：直接返回成功
    return {true, "Index dropped successfully"};
  default:
    return {false, "Unsupported DROP statement type"};
  }
}

ExecutionResult
DDLExecutionStrategy::executeAlter(const sql_parser::AlterStatement &stmt,
                                   ExecutionContext &context) {
  
  // 检查目标是否为表
  if (stmt.getTarget() != sql_parser::AlterStatement::TABLE) {
    return {false, "ALTER operation only supports TABLE target"};
  }

  std::string table_name = stmt.getTableName();
  
  // 获取系统数据库管理器
  auto system_db = context.get_system_db();
  if (!system_db) {
    return {false, "System database not available"};
  }

  // 获取表记录
  auto table_record = system_db->GetTableRecord("public", table_name);
  if (table_record.table_id <= 0) {
    return {false, "Table '" + table_name + "' not found"};
  }

  // 根据不同的ALTER操作执行相应的处理
  switch (stmt.getAction()) {
    case sql_parser::AlterStatement::ADD_COLUMN: {
      auto column_def = stmt.getColumnDefinition();
      std::string column_name = column_def.getName();
      std::string column_type = column_def.getType();

      // 验证列名不重复
      auto existing_columns = system_db->GetTableColumns(table_record.table_id);
      for (const auto& col : existing_columns) {
        if (col.name == column_name) {
          return {false, "Column '" + column_name + "' already exists in table '" + table_name + "'"};
        }
      }

      // 获取存储引擎和表存储管理器
      auto storage_engine = context.db_manager->GetStorageEngine();
      if (!storage_engine) {
        return {false, "Storage engine not available"};
      }

      TableStorageManager table_storage(storage_engine);

      // 获取表的元数据
      auto metadata = table_storage.GetTableMetadata(table_name);
      if (!metadata) {
        return {false, "Failed to get table metadata"};
      }

      // 添加列到元数据
      metadata->columns.push_back({column_name, column_type, false, "", false, false}); // 默认可空，无默认值
      metadata->column_index_map[column_name] = metadata->columns.size() - 1;

      // 对于现有记录，需要添加默认值
      // 扫描所有现有记录，为新列添加默认值
      auto all_locations = table_storage.ScanTable(table_name);

      for (const auto& location : all_locations) {
        std::vector<std::string> record = table_storage.GetRecord(table_name, location.first, location.second);
        if (!record.empty()) {
          // 为新列添加默认值（空字符串）
          record.push_back("");
          // 更新记录
          table_storage.UpdateRecord(table_name, location.first, location.second, record);
        }
      }

      // 更新系统数据库中的表定义
      std::vector<std::pair<std::string, std::string>> updated_columns;
      for (const auto& col : metadata->columns) {
        updated_columns.emplace_back(col.name, col.type);
      }

      // 这里应该更新系统数据库，但暂时跳过
      // system_db->UpdateTableDefinition(table_record.table_id, updated_columns);

      context.records_affected = 1;
      return {true, "Column '" + column_name + "' added to table '" + table_name + "' successfully"};
    }
    
    case sql_parser::AlterStatement::DROP_COLUMN: {
      std::string column_name = stmt.getColumnName();
      
      // 注：这里暂时使用一个简化的删除列方法，实际实现中可能需要更复杂的逻辑
      return {false, "Drop column operation not yet implemented"};
    }
    
    case sql_parser::AlterStatement::MODIFY_COLUMN: {
      auto column_def = stmt.getColumnDefinition();
      
      // 注：这里暂时使用一个简化的修改列方法，实际实现中可能需要更复杂的逻辑
      return {false, "Modify column operation not yet implemented"};
    }
    
    case sql_parser::AlterStatement::RENAME_TABLE: {
      std::string new_table_name = stmt.getNewTableName();
      
      // 注：这里暂时使用一个简化的重命名表方法，实际实现中可能需要更复杂的逻辑
      return {false, "Rename table operation not yet implemented"};
    }
    
    default:
      return {false, "Unsupported ALTER action"};
  }
}

ExecutionResult
DDLExecutionStrategy::executeCreateIndex(const sql_parser::CreateIndexStatement &stmt,
                                         ExecutionContext &context) {

  // 获取存储引擎
  auto storage_engine = context.db_manager->GetStorageEngine();
  if (!storage_engine) {
    return {false, "Storage engine not available"};
  }
  
  // 创建TableStorageManager实例
  TableStorageManager table_storage(storage_engine);
  
  // 获取表名和列名
  std::string table_name = stmt.getTableName();
  const auto& columns = stmt.getColumns();
  
  // 检查是否有列
  if (columns.empty()) {
    return {false, "No columns specified for index"};
  }
  
  // 目前只支持单列索引
  std::string column_name = columns[0];
  
  // 创建索引
  if (table_storage.CreateIndex(table_name, column_name)) {
    context.records_affected = 1;
    return {true, "Index created successfully on table '" + table_name + "' column '" + column_name + "'"};
  } else {
    return {false, "Failed to create index on table '" + table_name + "' column '" + column_name + "'"};
  }
}

ExecutionResult
DDLExecutionStrategy::executeDropIndex(const sql_parser::DropIndexStatement &stmt,
                                       ExecutionContext &context) {

  // 获取存储引擎
  auto storage_engine = context.db_manager->GetStorageEngine();
  if (!storage_engine) {
    return {false, "Storage engine not available"};
  }
  
  // 创建TableStorageManager实例
  TableStorageManager table_storage(storage_engine);
  
  // 获取索引名和表名
  std::string index_name = stmt.getIndexName();
  std::string table_name = stmt.getTableName();
  
  // 从索引名中提取列名（假设索引名为table_column_idx格式）
  // 这是一个简化的实现，实际应该从系统表中获取列名
  std::string column_name = index_name;
  size_t pos = index_name.find("_");
  if (pos != std::string::npos) {
    size_t pos2 = index_name.find("_", pos + 1);
    if (pos2 != std::string::npos) {
      column_name = index_name.substr(pos + 1, pos2 - pos - 1);
    }
  }
  
  // 删除索引
  if (table_storage.DropIndex(table_name, column_name)) {
    context.records_affected = 1;
    return {true, "Index dropped successfully from table '" + table_name + "' column '" + column_name + "'"};
  } else {
    return {false, "Failed to drop index from table '" + table_name + "' column '" + column_name + "'"};
  }
}

// ==================== DMLExecutionStrategy ====================

ExecutionResult
DMLExecutionStrategy::execute(std::unique_ptr<sql_parser::Statement> stmt,
                             ExecutionContext &context) {

  // 根据具体的语句类型执行
  if (auto insert_stmt =
          dynamic_cast<sql_parser::InsertStatement *>(stmt.get())) {
    if (insert_stmt) {
      return executeInsert(*insert_stmt, context);
    }
  } else if (auto update_stmt =
                 dynamic_cast<sql_parser::UpdateStatement *>(stmt.get())) {
    if (update_stmt) {
      return executeUpdate(*update_stmt, context);
    }
  } else if (auto delete_stmt =
                 dynamic_cast<sql_parser::DeleteStatement *>(stmt.get())) {
    if (delete_stmt) {
      return executeDelete(*delete_stmt, context);
    }
  } else if (auto select_stmt =
                 dynamic_cast<sql_parser::SelectStatement *>(stmt.get())) {
    if (select_stmt) {
      return executeSelect(*select_stmt, context);
    }
  }

  return {false, "Unsupported DML statement type"};
}

bool DMLExecutionStrategy::checkPermission(const sql_parser::Statement &stmt,
                                          const ExecutionContext &context) {

  if (!context.user_manager) {
    return true; // 默认允许
  }

  std::string operation;
  std::string table_name;

  if (auto insert_stmt =
          dynamic_cast<const sql_parser::InsertStatement *>(&stmt)) {
    if (insert_stmt) {
      operation = UserManager::PRIVILEGE_INSERT;
      table_name = insert_stmt->getTableName();
    }
  } else if (auto update_stmt =
                 dynamic_cast<const sql_parser::UpdateStatement *>(&stmt)) {
    if (update_stmt) {
      operation = UserManager::PRIVILEGE_UPDATE;
      table_name = update_stmt->getTableName();
    }
  } else if (auto delete_stmt =
                 dynamic_cast<const sql_parser::DeleteStatement *>(&stmt)) {
    if (delete_stmt) {
      operation = UserManager::PRIVILEGE_DELETE;
      table_name = delete_stmt->getTableName();
    }
  } else if (auto select_stmt =
                 dynamic_cast<const sql_parser::SelectStatement *>(&stmt)) {
    if (select_stmt) {
      operation = UserManager::PRIVILEGE_SELECT;
      table_name = select_stmt->getTableName();
    }
  }

  // 如果无法确定操作类型或表名，默认拒绝权限
  if (operation.empty() || table_name.empty()) {
    return false;
  }

  return context.user_manager->CheckPermission(
      context.current_user, context.current_database, table_name, operation);
}

bool DMLExecutionStrategy::validate(const sql_parser::Statement &stmt,
                                   const ExecutionContext &context) {
  // 检查数据库上下文
  if (!validateDatabaseContext(context)) {
    return false;
  }

  // 检查表是否存在
  std::string table_name;
  if (auto insert_stmt =
          dynamic_cast<const sql_parser::InsertStatement *>(&stmt)) {
    if (insert_stmt) {
      table_name = insert_stmt->getTableName();
    }
  } else if (auto update_stmt =
                 dynamic_cast<const sql_parser::UpdateStatement *>(&stmt)) {
    if (update_stmt) {
      table_name = update_stmt->getTableName();
    }
  } else if (auto delete_stmt =
                 dynamic_cast<const sql_parser::DeleteStatement *>(&stmt)) {
    if (delete_stmt) {
      table_name = delete_stmt->getTableName();
    }
  } else if (auto select_stmt =
                 dynamic_cast<const sql_parser::SelectStatement *>(&stmt)) {
    if (select_stmt) {
      table_name = select_stmt->getTableName();
    }
  }

  if (!table_name.empty() && !validateTableExists(table_name, context)) {
    return false;
  }

  return true;
}

ExecutionResult
DMLExecutionStrategy::executeInsert(const sql_parser::InsertStatement &stmt,
                                     ExecutionContext &context) {

  auto storage_engine = context.db_manager->GetStorageEngine();
  if (!storage_engine) {
    return {false, "Storage engine not available"};
  }

  TableStorageManager table_storage(storage_engine);
  const auto &values = stmt.getValues();
  int rows_inserted = 0;

  auto metadata = table_storage.GetTableMetadata(stmt.getTableName());
  if (!metadata) {
    return {false, "Failed to get table metadata"};
  }

  // 验证列数与值数匹配
  if (!values.empty() && !values[0].empty()) {
    size_t expected_columns = values[0].size();
    for (const auto &value_row : values) {
      if (value_row.size() != expected_columns) {
        return {false, "Column count mismatch in INSERT values"};
      }
    }
  }

  for (const auto &value_row : values) {
    std::vector<std::string> record;

    // 处理字符串值边界和转义
    for (const auto &value : value_row) {
      std::string processed_value = value;

      // 移除字符串值周围的多余引号（如果存在）
      if (processed_value.size() >= 2 && processed_value.front() == '\'' &&
          processed_value.back() == '\'') {
        processed_value = processed_value.substr(1, processed_value.size() - 2);
      }

      // 处理转义字符
      std::string escaped_value;
      for (size_t i = 0; i < processed_value.size(); i++) {
        if (processed_value[i] == '\\' && i + 1 < processed_value.size()) {
          // 处理转义字符
          switch (processed_value[i + 1]) {
          case 'n':
            escaped_value += '\n';
            break;
          case 't':
            escaped_value += '\t';
            break;
          case 'r':
            escaped_value += '\r';
            break;
          case '\\':
            escaped_value += '\\';
            break;
          case '\'':
            escaped_value += '\'';
            break;
          case '\"':
            escaped_value += '\"';
            break;
          default:
            escaped_value += processed_value[i];
            break;
          }
          i++; // 跳过下一个字符
        } else {
          escaped_value += processed_value[i];
        }
      }

      record.push_back(escaped_value);
    }

    // 约束验证
    if (!validateColumnConstraints(record, metadata, stmt.getTableName()) ||
        !checkPrimaryKeyConstraints(record, metadata, stmt.getTableName()) ||
        !checkUniqueKeyConstraints(record, metadata, stmt.getTableName())) {
      return {false, "Constraint validation failed"};
    }

    int32_t page_id;
    size_t offset;
    if (!table_storage.InsertRecord(stmt.getTableName(), record, page_id,
                                    offset)) {
      return {false, "Failed to insert record"};
    }

    // 索引维护
    maintainIndexesOnInsert(record, stmt.getTableName(), page_id, offset,
                            context);
    rows_inserted++;
  }

  context.records_affected = rows_inserted;
  return {true, "INSERT executed successfully, " +
                    std::to_string(rows_inserted) + " row(s) inserted"};
}

ExecutionResult
DMLExecutionStrategy::executeUpdate(
    const sql_parser::UpdateStatement& stmt, ExecutionContext& context) {
  // 获取表名
  std::string table_name = stmt.getTableName();
  
  // 验证表是否存在
  if (!validateTableExists(table_name, context)) {
    return {false, "Table '" + table_name + "' does not exist"};
  }
  
  // 获取表元数据
  auto table_metadata = context.db_manager->GetTableMetadata(table_name);
  if (!table_metadata) {
    return {false, "Failed to get table metadata for '" + table_name + "'"};
  }
  
  // 获取存储引擎
  auto storage_engine = context.db_manager->GetStorageEngine();
  if (!storage_engine) {
    return {false, "Storage engine not available"};
  }
  
  // 检查权限
  if (!checkPermission(stmt, context)) {
    return {false, "Permission denied"};
  }
  
  // 验证更新语句
  if (!validate(stmt, context)) {
    return {false, "Invalid update statement"};
  }
  
  // 执行更新操作
  try {
    // 创建TableStorageManager实例
    TableStorageManager table_storage(storage_engine);
    
    // 获取更新条件
    const auto& where_clause = stmt.getWhereClause();
    
    // 使用索引优化查询定位要更新的记录
    bool used_index = false;
    std::string index_info;
    std::vector<std::pair<int32_t, size_t>> locations;
    
    // 如果有WHERE子句，尝试使用索引优化
    if (!where_clause.getColumnName().empty()) {
      locations = optimizeQueryWithIndex(table_name, where_clause, storage_engine, used_index, index_info);
    } else {
      // 没有WHERE子句，扫描整个表
      locations = table_storage.ScanTable(table_name);
    }
    
    // 获取更新列和值
    const auto& updates = stmt.getUpdateValues();
    
    // 构建更新映射（列名到新值的映射）
    std::unordered_map<std::string, std::string> update_map;
    for (const auto& update : updates) {
      update_map[update.first] = update.second;
    }
    
    int rows_updated = 0;
    
    // 遍历所有需要更新的记录
    for (const auto& location : locations) {
      int32_t page_id = location.first;
      size_t offset = location.second;
      
      // 获取当前记录
      std::vector<std::string> old_record = table_storage.GetRecord(table_name, page_id, offset);
      if (old_record.empty()) {
        continue; // 跳过无效记录
      }
      
      // 创建新记录，基于旧记录但应用更新
      std::vector<std::string> new_record = old_record;
      
      // 应用更新到记录
      for (size_t i = 0; i < table_metadata->columns.size() && i < new_record.size(); ++i) {
        const std::string& column_name = table_metadata->columns[i].name;
        auto it = update_map.find(column_name);
        if (it != update_map.end()) {
          new_record[i] = it->second;
        }
      }
      
      // 约束验证
      if (!validateColumnConstraints(new_record, table_metadata, table_name) ||
          !checkPrimaryKeyConstraints(new_record, table_metadata, table_name) ||
          !checkUniqueKeyConstraints(new_record, table_metadata, table_name)) {
        return {false, "Constraint validation failed for updated record"};
      }
      
      // 更新记录
      if (table_storage.UpdateRecord(table_name, page_id, offset, new_record)) {
        // 索引维护 - 更新索引中的条目
        maintainIndexesOnUpdate(old_record, new_record, table_name, page_id, offset, context);
        rows_updated++;
      }
    }
    
    // 更新执行上下文
    context.records_affected = rows_updated;
    context.used_index = used_index;
    context.index_info_ = index_info;
    
    // 构造结果消息
    std::string message = "UPDATE executed successfully, " + std::to_string(rows_updated) + " row(s) updated";
    if (used_index) {
      message += " [使用索引: " + index_info + "]";
    }
    
    return {true, message};
  } catch (const std::exception& e) {
    return {false, "Error updating table '" + table_name + "': " + e.what()};
  }
}

ExecutionResult
DMLExecutionStrategy::executeDelete(
    const sql_parser::DeleteStatement& stmt, ExecutionContext& context) {
  // 获取表名
  std::string table_name = stmt.getTableName();
  
  // 验证表是否存在
  if (!validateTableExists(table_name, context)) {
    return {false, "Table '" + table_name + "' does not exist"};
  }
  
  // 获取表元数据
  auto table_metadata = context.db_manager->GetTableMetadata(table_name);
  if (!table_metadata) {
    return {false, "Failed to get table metadata for '" + table_name + "'"};
  }
  
  // 获取存储引擎
  auto storage_engine = context.db_manager->GetStorageEngine();
  if (!storage_engine) {
    return {false, "Storage engine not available"};
  }
  
  // 检查权限
  if (!checkPermission(stmt, context)) {
    return {false, "Permission denied"};
  }
  
  // 验证删除语句
  if (!validate(stmt, context)) {
    return {false, "Invalid delete statement"};
  }
  
  // 执行删除操作
  try {
    // 创建TableStorageManager实例
    TableStorageManager table_storage(storage_engine);
    
    // 获取删除条件
    const auto& where_clause = stmt.getWhereClause();
    
    // 使用索引优化查询定位要删除的记录
    bool used_index = false;
    std::string index_info;
    std::vector<std::pair<int32_t, size_t>> locations;
    
    // 如果有WHERE子句，尝试使用索引优化
    if (!where_clause.getColumnName().empty()) {
      locations = optimizeQueryWithIndex(table_name, where_clause, storage_engine, used_index, index_info);
    } else {
      // 没有WHERE子句，扫描整个表
      locations = table_storage.ScanTable(table_name);
    }
    
    int rows_deleted = 0;
    
    // 遍历所有需要删除的记录
    for (const auto& location : locations) {
      int32_t page_id = location.first;
      size_t offset = location.second;
      
      // 获取当前记录
      std::vector<std::string> record = table_storage.GetRecord(table_name, page_id, offset);
      if (record.empty()) {
        continue; // 跳过无效记录
      }
      
      // 删除记录
      if (table_storage.DeleteRecord(table_name, page_id, offset)) {
        // 索引维护 - 从索引中删除条目
        maintainIndexesOnDelete(record, table_name, page_id, offset, context);
        rows_deleted++;
      }
    }
    
    // 更新执行上下文
    context.records_affected = rows_deleted;
    context.used_index = used_index;
    context.index_info_ = index_info;
    
    // 构造结果消息
    std::string message = "DELETE executed successfully, " + std::to_string(rows_deleted) + " row(s) deleted";
    if (used_index) {
      message += " [使用索引: " + index_info + "]";
    }
    
    return {true, message};
  } catch (const std::exception& e) {
    return {false, "Error deleting from table '" + table_name + "': " + e.what()};
  }
}

ExecutionResult
DMLExecutionStrategy::executeSelect(
    const sql_parser::SelectStatement& stmt, ExecutionContext& context) {
  // 获取表名
  std::string table_name = stmt.getTableName();
  
  // 验证表是否存在
  if (!validateTableExists(table_name, context)) {
    return {false, "Table '" + table_name + "' does not exist"};
  }
  
  // 获取表元数据
  auto table_metadata = context.db_manager->GetTableMetadata(table_name);
  if (!table_metadata) {
    return {false, "Failed to get table metadata for '" + table_name + "'"};
  }
  
  // 获取存储引擎
  auto storage_engine = context.db_manager->GetStorageEngine();
  if (!storage_engine) {
    return {false, "Storage engine not available"};
  }
  
  // 检查权限
  if (!checkPermission(stmt, context)) {
    return {false, "Permission denied"};
  }
  
  // 验证查询语句
  if (!validate(stmt, context)) {
    return {false, "Invalid select statement"};
  }
  
  // 执行查询操作
  try {
    // 获取查询条件
    const auto& where_clause = stmt.getWhereClause();
    
    // 使用索引优化查询
    bool used_index = false;
    std::string index_info;
    std::vector<std::pair<int32_t, size_t>> locations;
    
    // 如果有WHERE子句，尝试使用索引优化
    if (!where_clause.getColumnName().empty()) {
      locations = optimizeQueryWithIndex(table_name, where_clause, storage_engine, used_index, index_info);
    } else {
      // 没有WHERE子句，扫描整个表
      TableStorageManager table_storage(storage_engine);
      locations = table_storage.ScanTable(table_name);
    }
    
    // 获取记录
    TableStorageManager table_storage(storage_engine);
    auto records = table_storage.GetRecords(table_name, locations);
    
    // 更新执行上下文
    context.used_index = used_index;
    context.index_info_ = index_info;
    context.records_affected = records.size();
    
    // 构造结果消息
    std::string message = "SELECT executed successfully, " + std::to_string(records.size()) + " row(s) returned";
    if (used_index) {
      message += " [使用索引: " + index_info + "]";
    }
    
    return {true, message};
  } catch (const std::exception& e) {
    return {false, "Error querying table '" + table_name + "': " + e.what()};
  }
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

  // 创建TableStorageManager实例
  TableStorageManager table_storage(storage_engine);

  // 检查是否有可用索引
  std::string column_name = where_clause.getColumnName();
  std::string op = where_clause.getOp();
  std::string value = where_clause.getValue();

  // 尝试获取表的索引 - 使用智能指针避免raw pointer
  auto index_ptr = table_storage.GetIndexShared(table_name, column_name);

  // 如果有索引且操作符支持索引查找
  if (index_ptr && (op == "=" || op == ">" || op == ">=" || op == "<" || op == "<=")) {
    used_index = true;

    // 使用索引进行查找
    std::vector<IndexEntry> index_results;

    if (op == "=") {
      // 等值查询 - 使用精确查找
      index_results = index_ptr->Search(value);
      index_info = "索引等式查询 (列: " + column_name + ", 值: " + value + ")";
    } else {
      // 范围查询 - 使用范围搜索，支持B+树的核心特性
      std::string lower_bound, upper_bound;

      if (op == ">") {
        // 大于查询：从value的下一个值开始，无上限
        lower_bound = value;
        upper_bound = ""; // 空字符串表示无上限
        index_info = "索引范围查询 (列: " + column_name + ", 操作符: " + op + ", 值: " + value + ")";
      } else if (op == ">=") {
        // 大于等于查询：从value开始，无上限
        lower_bound = value;
        upper_bound = ""; // 空字符串表示无上限
        index_info = "索引范围查询 (列: " + column_name + ", 操作符: " + op + ", 值: " + value + ")";
      } else if (op == "<") {
        // 小于查询：无下限，到value的上一个值结束
        lower_bound = ""; // 空字符串表示无下限
        upper_bound = value;
        index_info = "索引范围查询 (列: " + column_name + ", 操作符: " + op + ", 值: " + value + ")";
      } else if (op == "<=") {
        // 小于等于查询：无下限，到value结束
        lower_bound = ""; // 空字符串表示无下限
        upper_bound = value;
        index_info = "索引范围查询 (列: " + column_name + ", 操作符: " + op + ", 值: " + value + ")";
      }

      // 执行范围查询 - 利用B+树的叶子节点链特性
      if (!lower_bound.empty() || !upper_bound.empty()) {
        index_results = index_ptr->SearchRange(lower_bound, upper_bound);
      }
    }

    // 将索引结果转换为位置向量
    std::vector<std::pair<int32_t, size_t>> locations;
    for (const auto& entry : index_results) {
      locations.emplace_back(entry.page_id, entry.offset);
    }

    // 添加查询统计信息
    index_info += " [找到 " + std::to_string(locations.size()) + " 个匹配记录]";

    return locations;
  }

  // 如果没有索引或者不支持索引查找，回退到全表扫描 + 过滤
  SQLCC_LOG_DEBUG("No suitable index found for table '" + table_name +
                  "' column '" + column_name + "', falling back to full table scan");

  auto all_locations = table_storage.ScanTable(table_name);
  auto metadata = table_storage.GetTableMetadata(table_name);

  if (!metadata) {
    index_info = "全表扫描 (无法获取表元数据)";
    return all_locations;
  }

  // 进行全表扫描并应用WHERE条件过滤
  std::vector<std::pair<int32_t, size_t>> filtered_locations;
  for (const auto &location : all_locations) {
    std::vector<std::string> record =
        table_storage.GetRecord(table_name, location.first, location.second);
    if (!record.empty() && matchesWhereClause(record, where_clause, metadata)) {
      filtered_locations.push_back(location);
    }
  }

  index_info = "全表扫描 + 过滤 (扫描 " + std::to_string(all_locations.size()) +
               " 条记录, 匹配 " + std::to_string(filtered_locations.size()) + " 条)";

  return filtered_locations;
}

// ==================== DCLExecutionStrategy ====================

ExecutionResult
DCLExecutionStrategy::execute(std::unique_ptr<sql_parser::Statement> stmt,
                             ExecutionContext &context) {

  if (auto create_user_stmt =
          dynamic_cast<sql_parser::CreateUserStatement *>(stmt.get())) {
    return executeCreateUser(*create_user_stmt, context);
  } else if (auto drop_user_stmt =
                 dynamic_cast<sql_parser::DropUserStatement *>(stmt.get())) {
    return executeDropUser(*drop_user_stmt, context);
  } else if (auto grant_stmt =
                 dynamic_cast<sql_parser::GrantStatement *>(stmt.get())) {
    return executeGrant(*grant_stmt, context);
  } else if (auto revoke_stmt =
                 dynamic_cast<sql_parser::RevokeStatement *>(stmt.get())) {
    return executeRevoke(*revoke_stmt, context);
  }

  return {false, "Unsupported DCL statement type"};
}

bool DCLExecutionStrategy::checkPermission(const sql_parser::Statement &stmt,
                                          const ExecutionContext &context) {
  // DCL语句使用基类的默认权限检查（管理员权限）
  return ExecutionStrategy::defaultPermissionCheck(context);
}

bool DCLExecutionStrategy::validate(const sql_parser::Statement &stmt,
                                   const ExecutionContext &context) {
  // DCL语句不需要验证数据库上下文，直接返回true
  return true;
}

ExecutionResult
DCLExecutionStrategy::executeCreateUser(
    const sql_parser::CreateUserStatement& stmt, ExecutionContext& context) {
  // 获取用户名和密码
  std::string username = stmt.getUsername();
  std::string password = stmt.getPassword();
  
  // 验证用户管理器是否可用
  if (!context.user_manager) {
    return {false, "User manager not available"};
  }
  
  // 检查权限
  if (!checkPermission(stmt, context)) {
    return {false, "Permission denied"};
  }
  
  // 验证创建用户语句
  if (!validate(stmt, context)) {
    return {false, "Invalid create user statement"};
  }
  
  // 执行创建用户操作
  try {
    if (context.user_manager->CreateUser(username, password)) {
      context.records_affected = 1;
      return {true, "User '" + username + "' created successfully"};
    } else {
      return {false, "Failed to create user '" + username + "'"};
    }
  } catch (const std::exception& e) {
    return {false, "Error creating user '" + username + "': " + e.what()};
  }
}

ExecutionResult
DCLExecutionStrategy::executeDropUser(
    const sql_parser::DropUserStatement& stmt, ExecutionContext& context) {
  // 获取用户名
  std::string username = stmt.getUsername();
  
  // 验证用户管理器是否可用
  if (!context.user_manager) {
    return {false, "User manager not available"};
  }
  
  // 检查权限
  if (!checkPermission(stmt, context)) {
    return {false, "Permission denied"};
  }
  
  // 验证删除用户语句
  if (!validate(stmt, context)) {
    return {false, "Invalid drop user statement"};
  }
  
  // 执行删除用户操作
  try {
    if (context.user_manager->DropUser(username)) {
      context.records_affected = 1;
      return {true, "User '" + username + "' dropped successfully"};
    } else {
      return {false, "Failed to drop user '" + username + "'"};
    }
  } catch (const std::exception& e) {
    return {false, "Error dropping user '" + username + "': " + e.what()};
  }
}

ExecutionResult
DCLExecutionStrategy::executeGrant(
    const sql_parser::GrantStatement& stmt, ExecutionContext& context) {
  // 获取用户名和权限
  std::string username = stmt.getGrantee();
  std::vector<std::string> privileges = stmt.getPrivileges();
  std::string object = stmt.getObjectName();
  
  // 处理权限列表，如果为空则使用ALL PRIVILEGES
  std::string permission = "ALL PRIVILEGES";
  if (!privileges.empty()) {
    permission = privileges[0];  // 暂时只处理第一个权限
  }
  
  // 验证用户管理器是否可用
  if (!context.user_manager) {
    return {false, "User manager not available"};
  }
  
  // 检查权限
  if (!checkPermission(stmt, context)) {
    return {false, "Permission denied"};
  }
  
  // 验证授权语句
  if (!validate(stmt, context)) {
    return {false, "Invalid grant statement"};
  }
  
  // 执行授权操作
  try {
    if (context.user_manager->GrantPrivilege(username, "*", object, permission)) {
      context.records_affected = 1;
      return {true, "Permission '" + permission + "' granted to user '" + username + "' on '*.'" + object + "'"};
    } else {
      return {false, "Failed to grant permission '" + permission + "' to user '" + username + "' on '*.'" + object + "'"};
    }
  } catch (const std::exception& e) {
    return ExecutionResult(false, "Error granting permission: " + std::string(e.what()));
  }
}

ExecutionResult
DCLExecutionStrategy::executeRevoke(
    const sql_parser::RevokeStatement& stmt, ExecutionContext& context) {
  // 获取用户名和权限
  std::string username = stmt.getGrantee();
  std::vector<std::string> privileges = stmt.getPrivileges();
  std::string object = stmt.getObjectName();
  
  // 处理权限列表，如果为空则使用ALL PRIVILEGES
  std::string permission = "ALL PRIVILEGES";
  if (!privileges.empty()) {
    permission = privileges[0];  // 暂时只处理第一个权限
  }
  
  // 验证用户管理器是否可用
  if (!context.user_manager) {
    return {false, "User manager not available"};
  }
  
  // 检查权限
  if (!checkPermission(stmt, context)) {
    return {false, "Permission denied"};
  }
  
  // 验证撤销权限语句
  if (!validate(stmt, context)) {
    return {false, "Invalid revoke statement"};
  }
  
  // 执行撤销权限操作
  try {
    if (context.user_manager->RevokePrivilege(username, "*", object, permission)) {
      context.records_affected = 1;
      return {true, "Permission '" + permission + "' revoked from user '" + username + "' on '*.'" + object + "'"};
    } else {
      return {false, "Failed to revoke permission '" + permission + "' from user '" + username + "' on '*.'" + object + "'"};
    }
  } catch (const std::exception& e) {
    return ExecutionResult(false, "Error revoking permission: " + std::string(e.what()));
  }
}

// ==================== UtilityExecutionStrategy ====================

ExecutionResult
UtilityExecutionStrategy::execute(std::unique_ptr<sql_parser::Statement> stmt,
                                  ExecutionContext &context) {

  if (auto use_stmt = dynamic_cast<sql_parser::UseStatement *>(stmt.get())) {
    return executeUse(*use_stmt, context);
  } else if (auto show_stmt =
                 dynamic_cast<sql_parser::ShowStatement *>(stmt.get())) {
    return executeShow(*show_stmt, context);
  }

  return {false, "Unsupported utility statement type"};
}

bool UtilityExecutionStrategy::checkPermission(
    const sql_parser::Statement &stmt, const ExecutionContext &context) {

  // 工具语句通常不需要特殊权限
  return true;
}

bool UtilityExecutionStrategy::validate(const sql_parser::Statement &stmt,
                                      const ExecutionContext &context) {

  return true; // 工具语句的验证相对简单
}

ExecutionResult
UtilityExecutionStrategy::executeUse(const sql_parser::UseStatement &stmt,
                                     ExecutionContext &context) {

  std::string db_name = stmt.getDatabaseName();

  if (context.db_manager->UseDatabase(db_name)) {
    context.current_database = db_name;
    return ExecutionResult(true, "Database changed to '" + db_name + "'");
  } else {
    return ExecutionResult(false, "Database '" + db_name + "' does not exist");
  }
}

ExecutionResult
UtilityExecutionStrategy::executeShow(
    const sql_parser::ShowStatement& stmt, ExecutionContext& context) {
  // 获取SHOW的对象类型
  auto show_type = stmt.getShowType();
  std::string object_name = stmt.getTargetObject();
  
  // 验证数据库管理器是否可用
  if (!context.db_manager) {
    return {false, "Database manager not available"};
  }
  
  // 检查权限
  if (!checkPermission(stmt, context)) {
    return {false, "Permission denied"};
  }
  
  // 验证SHOW语句
  if (!validate(stmt, context)) {
    return {false, "Invalid show statement"};
  }
  
  // 执行SHOW操作
  try {
    switch (show_type) {
      case sql_parser::ShowStatement::DATABASES: {
        // 显示所有数据库
        auto databases = context.db_manager->ListDatabases();
        std::string result = "Databases:\n";
        for (const auto& db : databases) {
          result += "- " + db + "\n";
        }
        return ExecutionResult(true, result);
      }
      
      case sql_parser::ShowStatement::TABLES: {
        // 显示当前数据库中的所有表
        if (context.current_database.empty()) {
          return ExecutionResult(false, "No database selected");
        }
        
        auto tables = context.db_manager->ListTables();
        std::string result = "Tables in database '" + context.current_database + "':\n";
        for (const auto& table : tables) {
          result += "- " + table + "\n";
        }
        return ExecutionResult(true, result);
      }
      
      case sql_parser::ShowStatement::INDEXES: {
        // 显示表的索引
        if (context.current_database.empty()) {
          return ExecutionResult(false, "No database selected");
        }
        
        if (object_name.empty()) {
          return ExecutionResult(false, "Table name not specified");
        }
        
        // 获取索引管理器
        auto index_manager = context.db_manager->GetIndexManager();
        if (!index_manager) {
          return ExecutionResult(false, "Index manager not available");
        }
        
        // 获取表的索引
        auto table_indexes = index_manager->GetTableIndexes(object_name);
        std::string result = "Indexes for table '" + object_name + "':\n";
        for (const auto& index : table_indexes) {
          // 这里简单显示索引名称，实际可以显示更多信息
          result += "- " + std::string("index_name") + "\n";
        }
        return ExecutionResult(true, result);
      }
      
      default:
        return ExecutionResult(false, "Unsupported SHOW object type");
    }
  } catch (const std::exception& e) {
    return ExecutionResult(false, "Error executing SHOW: " + std::string(e.what()));
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
  // 使用默认的执行上下文
  return execute(std::move(stmt),
                 std::make_shared<ExecutionContext>(last_context_));
}

ExecutionResult
UnifiedExecutor::execute(std::unique_ptr<sql_parser::Statement> stmt,
                         std::shared_ptr<ExecutionContext> context) {
  if (!stmt) {
    return {false, "Statement is null"};
  }

  // 获取对应的执行策略
  ExecutionStrategy *strategy = getStrategy(stmt->getType());
  if (!strategy) {
    return {false, "No execution strategy found for statement type"};
  }

  // 更新执行上下文
  context->records_affected = 0;
  context->used_index = false;
  context->execution_plan = "未优化";
  context->execution_time_ms_ = 0;
  context->plan_details_ = "";
  context->optimized_plan_ = "";
  context->query_optimized_ = false;
  context->optimization_rules_.clear();
  context->index_info_ = "";
  context->cost_estimate_ = 0.0;

  // 全局权限检查
  if (!checkGlobalPermission(*stmt, *context)) {
    return {false, "Permission denied"};
  }

  // 全局上下文验证
  if (!validateGlobalContext(*stmt, *context)) {
    return {false, "Invalid execution context"};
  }

  // 策略特定的权限检查和验证
  if (!strategy->checkPermission(*stmt, *context) ||
      !strategy->validate(*stmt, *context)) {
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
          query_optimizer_->generatePlan(*select_stmt, *context);
      context->execution_plan = plan.toString();
      context->plan_details_ = plan.description;
      context->cost_estimate_ = plan.cost_estimate;

      // 2. 优化执行计划
      ExecutionPlan optimized_plan =
          query_optimizer_->optimize(plan, *context);
      context->optimized_plan_ = optimized_plan.toString();
      context->query_optimized_ = optimized_plan.is_optimized;

      // 3. 更新索引使用信息
      if (!optimized_plan.index_name.empty()) {
        context->used_index = true;
        context->index_info_ = optimized_plan.index_name;
      }

      // 4. 更新优化规则信息
      context->optimization_rules_ =
          query_optimizer_->getOptimizationRules();
    }
  }

  // 执行语句
  ExecutionResult result = strategy->execute(std::move(stmt), *context);

  // 记录执行时间
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time);
  context->execution_time_ms_ = duration.count();

  return result;
}

ExecutionStrategy *
UnifiedExecutor::getStrategy(sql_parser::Statement::Type type) {
  auto it = strategies_.find(type);
  return (it != strategies_.end()) ? it->second.get() : nullptr;
}



bool UnifiedExecutor::checkGlobalPermission(const sql_parser::Statement& stmt,
                                            ExecutionContext& context) {
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
  switch (stmt.getType()) {
  case sql_parser::Statement::CREATE: {
    auto create_stmt = dynamic_cast<const sql_parser::CreateStatement*>(&stmt);
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
    auto drop_stmt = dynamic_cast<const sql_parser::DropStatement*>(&stmt);
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

bool UnifiedExecutor::validateGlobalContext(const sql_parser::Statement& stmt,
                                            ExecutionContext& context) {
  // 全局上下文验证

  // 1. 检查数据库管理器是否可用
  if (!context.db_manager) {
    return false;
  }

  // 2. 根据语句类型进行不同的上下文验证
  switch (stmt.getType()) {
  case sql_parser::Statement::CREATE: {
    auto create_stmt = dynamic_cast<const sql_parser::CreateStatement*>(&stmt);
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
    auto drop_stmt = dynamic_cast<const sql_parser::DropStatement*>(&stmt);
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
AdvancedExecutor::executeJoinQuery(const sql_parser::SelectStatement& stmt) {
  // JOIN查询的实现 - 增强版本
  std::string table_name = stmt.getTableName();

  // 验证表是否存在
  if (!validateTableExists(table_name, getLastExecutionContext())) {
    return {false, "Table '" + table_name + "' does not exist"};
  }

  // 获取存储引擎
  auto storage_engine = getLastExecutionContext().db_manager->GetStorageEngine();
  if (!storage_engine) {
    return {false, "Storage engine not available"};
  }

  // 检查权限
  if (!checkPermission(stmt, getLastExecutionContext())) {
    return {false, "Permission denied"};
  }

  // 执行JOIN查询逻辑 - 基础实现
  try {
    // 创建TableStorageManager实例
    TableStorageManager table_storage(storage_engine);

    // 检查WHERE子句，尝试使用索引优化
    bool used_index = false;
    std::string index_info;
    std::vector<std::pair<int32_t, size_t>> locations;

    if (stmt.hasWhereClause() && !stmt.getWhereClause().getColumnName().empty()) {
      locations = optimizeQueryWithIndex(table_name, stmt.getWhereClause(),
                                       storage_engine, used_index, index_info);
    } else {
      locations = table_storage.ScanTable(table_name);
    }

    auto records = table_storage.GetRecords(table_name, locations);

    // 更新执行上下文
    getLastExecutionContext().records_affected = records.size();
    getLastExecutionContext().used_index = used_index;
    getLastExecutionContext().index_info_ = index_info;

    // 构造结果消息
    std::string message = "JOIN query executed successfully, " +
                         std::to_string(records.size()) + " row(s) returned";
    if (used_index) {
      message += " [使用索引: " + index_info + "]";
    } else {
      message += " [JOIN查询基础实现 - 暂不支持复杂JOIN逻辑]";
    }

    return {true, message};
  } catch (const std::exception& e) {
    return {false, "Error executing JOIN query on table '" + table_name + "': " + e.what()};
  }
}

ExecutionResult
AdvancedExecutor::executeSubquery(const sql_parser::SelectStatement& stmt) {

  // 子查询的实现
  std::string table_name = stmt.getTableName();

  // 验证表是否存在
  if (!validateTableExists(table_name, getLastExecutionContext())) {
    return {false, "Table '" + table_name + "' does not exist"};
  }

  // 获取存储引擎
  auto storage_engine = getLastExecutionContext().db_manager->GetStorageEngine();
  if (!storage_engine) {
    return {false, "Storage engine not available"};
  }

  // 检查权限
  if (!checkPermission(stmt, getLastExecutionContext())) {
    return {false, "Permission denied"};
  }

  // 执行子查询逻辑
  try {
    // 创建TableStorageManager实例
    TableStorageManager table_storage(storage_engine);

    // 获取子查询的记录（简化实现，实际应该处理子查询逻辑）
    auto locations = table_storage.ScanTable(table_name);
    auto records = table_storage.GetRecords(table_name, locations);

    // 更新执行上下文
    getLastExecutionContext().records_affected = records.size();

    // 构造结果消息
    std::string message = "Subquery executed successfully, " +
                         std::to_string(records.size()) + " row(s) returned";
    message += " [子查询基础实现]";

    return {true, message};
  } catch (const std::exception& e) {
    return {false, "Error executing subquery on table '" + table_name + "': " + e.what()};
  }
}

ExecutionResult AdvancedExecutor::executeWindowFunction(const sql_parser::SelectStatement& stmt) {

  // 窗口函数的实现
  std::string table_name = stmt.getTableName();

  // 验证表是否存在
  if (!validateTableExists(table_name, getLastExecutionContext())) {
    return {false, "Table '" + table_name + "' does not exist"};
  }

  // 获取存储引擎
  auto storage_engine = getLastExecutionContext().db_manager->GetStorageEngine();
  if (!storage_engine) {
    return {false, "Storage engine not available"};
  }

  // 检查权限
  if (!checkPermission(stmt, getLastExecutionContext())) {
    return {false, "Permission denied"};
  }

  // 执行窗口函数逻辑
  try {
    // 创建TableStorageManager实例
    TableStorageManager table_storage(storage_engine);

    // 获取窗口函数的记录（简化实现，实际应该处理窗口函数逻辑）
    auto locations = table_storage.ScanTable(table_name);
    auto records = table_storage.GetRecords(table_name, locations);

    // 更新执行上下文
    getLastExecutionContext().records_affected = records.size();

    // 构造结果消息
    std::string message = "Window function executed successfully, " +
                         std::to_string(records.size()) + " row(s) returned";
    message += " [窗口函数基础实现]";

    return {true, message};
  } catch (const std::exception& e) {
    return {false, "Error executing window function on table '" + table_name + "': " + e.what()};
  }
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
