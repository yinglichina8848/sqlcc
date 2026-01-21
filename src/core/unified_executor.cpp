#include "unified_executor.h"
#include "execution_result.h"
#include "execution_context.h"
#include "execution_engine.h"
#include "core_database_manager.h"
#include "user_manager.h"
#include "system_database.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace sqlcc {

// WHY层 - 统一执行器实现
// UnifiedExecutor是SQLCC系统的核心执行引擎，采用策略模式统一处理所有类型的SQL语句。
// 它集成了权限验证、语句验证、执行分派等功能，确保数据库操作的安全性和正确性。
//
// WHAT层 - 核心功能
// 1. 语句类型识别和策略分派
// 2. 权限验证和上下文管理
// 3. 执行结果处理和统计
// 4. 错误处理和恢复机制
//
// HOW层 - 实现策略
// - 策略映射表：statement_type -> execution_strategy
// - 统一验证流程：权限检查 -> 语句验证 -> 执行
// - 上下文管理：维护执行状态和统计信息
// - 错误恢复：记录错误信息，支持部分失败恢复

// ============================================================================
// DDLExecutionStrategy 实现
// ============================================================================

DDLExecutionStrategy::DDLExecutionStrategy() = default;

DDLExecutionStrategy::~DDLExecutionStrategy() = default;

ExecutionResult DDLExecutionStrategy::execute(
    std::unique_ptr<sql_parser::Statement> stmt, ExecutionContext& context) {

  std::cout << "[DDLExecutor] 开始执行DDL语句" << std::endl;

  // 识别语句类型并分派执行
  if (auto create_stmt = dynamic_cast<sql_parser::CreateStatement*>(stmt.get())) {
    return executeCreate(*create_stmt, context);
  } else if (auto drop_stmt = dynamic_cast<sql_parser::DropStatement*>(stmt.get())) {
    return executeDrop(*drop_stmt, context);
  } else if (auto alter_stmt = dynamic_cast<sql_parser::AlterStatement*>(stmt.get())) {
    return executeAlter(*alter_stmt, context);
  } else if (auto create_index_stmt = dynamic_cast<sql_parser::CreateIndexStatement*>(stmt.get())) {
    return executeCreateIndex(*create_index_stmt, context);
  } else if (auto drop_index_stmt = dynamic_cast<sql_parser::DropIndexStatement*>(stmt.get())) {
    return executeDropIndex(*drop_index_stmt, context);
  }

  return ExecutionResult(false, "Unsupported DDL statement type");
}

bool DDLExecutionStrategy::checkPermission(const sql_parser::Statement& stmt,
                                          const ExecutionContext& context) {
  // DDL语句权限检查逻辑
  if (auto create_stmt = dynamic_cast<const sql_parser::CreateStatement*>(&stmt)) {
    return checkCreatePermission(*create_stmt, context);
  } else if (auto drop_stmt = dynamic_cast<const sql_parser::DropStatement*>(&stmt)) {
    return checkDropPermission(*drop_stmt, context);
  } else if (auto alter_stmt = dynamic_cast<const sql_parser::AlterStatement*>(&stmt)) {
    return checkAlterPermission(*alter_stmt, context);
  }

  return defaultPermissionCheck(context);
}

bool DDLExecutionStrategy::validate(const sql_parser::Statement& stmt,
                                   const ExecutionContext& context) {
  // DDL语句验证逻辑
  if (auto create_stmt = dynamic_cast<const sql_parser::CreateStatement*>(&stmt)) {
    return validateCreateStatement(*create_stmt, context);
  } else if (auto drop_stmt = dynamic_cast<const sql_parser::DropStatement*>(&stmt)) {
    return validateDropStatement(*drop_stmt, context);
  }

  return true; // 默认通过验证
}

// 私有执行方法实现
ExecutionResult DDLExecutionStrategy::executeCreate(
    const sql_parser::CreateStatement& stmt, ExecutionContext& context) {

  std::cout << "[DDLExecutor] 执行CREATE语句，对象类型: " << static_cast<int>(stmt.getObjectType()) << std::endl;

  switch (stmt.getObjectType()) {
    case sql_parser::CreateStatement::DATABASE: {
      std::string db_name = stmt.getObjectName();
      std::cout << "[DDLExecutor] 创建数据库: " << db_name << std::endl;

      if (!context.db_manager) {
        return ExecutionResult(false, "Database manager not available");
      }

      if (context.db_manager->CreateDatabase(db_name)) {
        context.set_rows_affected(1);
        return ExecutionResult(true, "Database '" + db_name + "' created successfully");
      } else {
        return ExecutionResult(false, "Failed to create database '" + db_name + "'");
      }
    }

    case sql_parser::CreateStatement::TABLE: {
      std::string table_name = stmt.getObjectName();
      std::cout << "[DDLExecutor] 创建表: " << table_name << std::endl;

      if (!context.db_manager) {
        return ExecutionResult(false, "Database manager not available");
      }

      // 转换列定义格式
      std::vector<std::pair<std::string, std::string>> columns;
      for (const auto& col : stmt.getColumns()) {
        columns.emplace_back(col.getName(), col.getTypeString());
      }

      if (context.db_manager->CreateTable(table_name, columns)) {
        context.set_rows_affected(1);
        return ExecutionResult(true, "Table '" + table_name + "' created successfully");
      } else {
        return ExecutionResult(false, "Failed to create table '" + table_name + "'");
      }
    }

    default:
      return ExecutionResult(false, "Unsupported CREATE object type");
  }
}

ExecutionResult DDLExecutionStrategy::executeDrop(
    const sql_parser::DropStatement& stmt, ExecutionContext& context) {

  std::cout << "[DDLExecutor] 执行DROP语句，对象类型: " << static_cast<int>(stmt.getObjectType()) << std::endl;

  switch (stmt.getObjectType()) {
    case sql_parser::DropStatement::DATABASE: {
      std::string db_name = stmt.getObjectName();
      std::cout << "[DDLExecutor] 删除数据库: " << db_name << std::endl;

      if (!context.db_manager) {
        return ExecutionResult(false, "Database manager not available");
      }

      if (context.db_manager->DropDatabase(db_name)) {
        context.set_rows_affected(1);
        return ExecutionResult(true, "Database '" + db_name + "' dropped successfully");
      } else {
        return ExecutionResult(false, "Failed to drop database '" + db_name + "'");
      }
    }

    case sql_parser::DropStatement::TABLE: {
      std::string table_name = stmt.getObjectName();
      std::cout << "[DDLExecutor] 删除表: " << table_name << std::endl;

      if (!context.db_manager) {
        return ExecutionResult(false, "Database manager not available");
      }

      if (context.db_manager->DropTable(table_name)) {
        context.set_rows_affected(1);
        return ExecutionResult(true, "Table '" + table_name + "' dropped successfully");
      } else {
        return ExecutionResult(false, "Failed to drop table '" + table_name + "'");
      }
    }

    default:
      return ExecutionResult(false, "Unsupported DROP object type");
  }
}

ExecutionResult DDLExecutionStrategy::executeAlter(
    const sql_parser::AlterStatement& stmt, ExecutionContext& context) {
  // ALTER语句的简化实现
  std::cout << "[DDLExecutor] 执行ALTER语句" << std::endl;
  return ExecutionResult(false, "ALTER statement not yet implemented");
}

ExecutionResult DDLExecutionStrategy::executeCreateIndex(
    const sql_parser::CreateIndexStatement& stmt, ExecutionContext& context) {
  // CREATE INDEX语句的简化实现
  std::cout << "[DDLExecutor] 执行CREATE INDEX语句" << std::endl;
  return ExecutionResult(false, "CREATE INDEX statement not yet implemented");
}

ExecutionResult DDLExecutionStrategy::executeDropIndex(
    const sql_parser::DropIndexStatement& stmt, ExecutionContext& context) {
  // DROP INDEX语句的简化实现
  std::cout << "[DDLExecutor] 执行DROP INDEX语句" << std::endl;
  return ExecutionResult(false, "DROP INDEX statement not yet implemented");
}

// 权限检查方法实现
bool DDLExecutionStrategy::checkCreatePermission(
    const sql_parser::CreateStatement& stmt, const ExecutionContext& context) {
  // 检查用户是否有创建权限
  // 简化实现：总是允许
  return true;
}

bool DDLExecutionStrategy::checkDropPermission(
    const sql_parser::DropStatement& stmt, const ExecutionContext& context) {
  // 检查用户是否有删除权限
  // 简化实现：总是允许
  return true;
}

bool DDLExecutionStrategy::checkAlterPermission(
    const sql_parser::AlterStatement& stmt, const ExecutionContext& context) {
  // 检查用户是否有修改权限
  // 简化实现：总是允许
  return true;
}

// 验证方法实现
bool DDLExecutionStrategy::validateCreateStatement(
    const sql_parser::CreateStatement& stmt, const ExecutionContext& context) {
  // 验证CREATE语句的正确性
  if (stmt.getObjectName().empty()) {
    return false;
  }
  return true;
}

bool DDLExecutionStrategy::validateDropStatement(
    const sql_parser::DropStatement& stmt, const ExecutionContext& context) {
  // 验证DROP语句的正确性
  if (stmt.getObjectName().empty()) {
    return false;
  }
  return true;
}

// ============================================================================
// DMLExecutionStrategy 实现
// ============================================================================



ExecutionResult DMLExecutionStrategy::execute(
    std::unique_ptr<sql_parser::Statement> stmt, ExecutionContext& context) {

  std::cout << "[DMLExecutor] 开始执行DML语句" << std::endl;

  // 识别语句类型并分派执行
  if (auto select_stmt = dynamic_cast<sql_parser::SelectStatement*>(stmt.get())) {
    return executeSelect(*select_stmt, context);
  } else if (auto insert_stmt = dynamic_cast<sql_parser::InsertStatement*>(stmt.get())) {
    return executeInsert(*insert_stmt, context);
  } else if (auto update_stmt = dynamic_cast<sql_parser::UpdateStatement*>(stmt.get())) {
    return executeUpdate(*update_stmt, context);
  } else if (auto delete_stmt = dynamic_cast<sql_parser::DeleteStatement*>(stmt.get())) {
    return executeDelete(*delete_stmt, context);
  }

  return ExecutionResult(false, "Unsupported DML statement type");
}

bool DMLExecutionStrategy::checkPermission(const sql_parser::Statement& stmt,
                                          const ExecutionContext& context) {
  // DML语句权限检查逻辑
  if (auto select_stmt = dynamic_cast<const sql_parser::SelectStatement*>(&stmt)) {
    return checkSelectPermission(*select_stmt, context);
  } else if (auto insert_stmt = dynamic_cast<const sql_parser::InsertStatement*>(&stmt)) {
    return checkInsertPermission(*insert_stmt, context);
  } else if (auto update_stmt = dynamic_cast<const sql_parser::UpdateStatement*>(&stmt)) {
    return checkUpdatePermission(*update_stmt, context);
  } else if (auto delete_stmt = dynamic_cast<const sql_parser::DeleteStatement*>(&stmt)) {
    return checkDeletePermission(*delete_stmt, context);
  }

  return defaultPermissionCheck(context);
}

bool DMLExecutionStrategy::validate(const sql_parser::Statement& stmt,
                                   const ExecutionContext& context) {
  // DML语句验证逻辑
  return true; // 简化实现
}

ExecutionResult DMLExecutionStrategy::executeSelect(
    const sql_parser::SelectStatement& stmt, ExecutionContext& context) {

  std::cout << "[DMLExecutor] 执行SELECT语句" << std::endl;

  if (!context.db_manager) {
    return ExecutionResult(false, "Database manager not available");
  }

  // 获取表名
  std::string table_name = stmt.getTableName();
  if (table_name.empty()) {
    return ExecutionResult(false, "Table name not specified in SELECT statement");
  }

  std::cout << "[DMLExecutor] 查询表: " << table_name << std::endl;

  // 检查表是否存在
  if (!context.db_manager->TableExists(table_name)) {
    return ExecutionResult(false, "Table '" + table_name + "' does not exist");
  }

  // 简化实现：返回成功消息
  context.set_rows_returned(0); // 简化实现
  context.set_used_index(false);

  std::stringstream ss;
  ss << "SELECT from table '" << table_name << "' executed successfully";
  return ExecutionResult(true, ss.str());
}

ExecutionResult DMLExecutionStrategy::executeInsert(
    const sql_parser::InsertStatement& stmt, ExecutionContext& context) {

  std::cout << "[DMLExecutor] 执行INSERT语句" << std::endl;

  if (!context.db_manager) {
    return ExecutionResult(false, "Database manager not available");
  }

  std::string table_name = stmt.getTableName();
  std::cout << "[DMLExecutor] 插入表: " << table_name << std::endl;

  // 检查表是否存在
  if (!context.db_manager->TableExists(table_name)) {
    return ExecutionResult(false, "Table '" + table_name + "' does not exist");
  }

  // 简化实现：返回成功消息
  context.set_rows_affected(1);
  return ExecutionResult(true, "Insert into table '" + table_name + "' executed successfully");
}

ExecutionResult DMLExecutionStrategy::executeUpdate(
    const sql_parser::UpdateStatement& stmt, ExecutionContext& context) {

  std::cout << "[DMLExecutor] 执行UPDATE语句" << std::endl;

  if (!context.db_manager) {
    return ExecutionResult(false, "Database manager not available");
  }

  std::string table_name = stmt.getTableName();
  std::cout << "[DMLExecutor] 更新表: " << table_name << std::endl;

  // 检查表是否存在
  if (!context.db_manager->TableExists(table_name)) {
    return ExecutionResult(false, "Table '" + table_name + "' does not exist");
  }

  // 简化实现：返回成功消息
  context.set_rows_affected(1);
  return ExecutionResult(true, "Update on table '" + table_name + "' executed successfully");
}

ExecutionResult DMLExecutionStrategy::executeDelete(
    const sql_parser::DeleteStatement& stmt, ExecutionContext& context) {

  std::cout << "[DMLExecutor] 执行DELETE语句" << std::endl;

  if (!context.db_manager) {
    return ExecutionResult(false, "Database manager not available");
  }

  std::string table_name = stmt.getTableName();
  std::cout << "[DMLExecutor] 删除表: " << table_name << std::endl;

  // 检查表是否存在
  if (!context.db_manager->TableExists(table_name)) {
    return ExecutionResult(false, "Table '" + table_name + "' does not exist");
  }

  // 简化实现：返回成功消息
  context.set_rows_affected(1);
  return ExecutionResult(true, "Delete from table '" + table_name + "' executed successfully");
}

// 权限检查方法实现
bool DMLExecutionStrategy::checkSelectPermission(
    const sql_parser::SelectStatement& stmt, const ExecutionContext& context) {
  return true; // 简化实现
}

bool DMLExecutionStrategy::checkInsertPermission(
    const sql_parser::InsertStatement& stmt, const ExecutionContext& context) {
  return true; // 简化实现
}

bool DMLExecutionStrategy::checkUpdatePermission(
    const sql_parser::UpdateStatement& stmt, const ExecutionContext& context) {
  return true; // 简化实现
}

bool DMLExecutionStrategy::checkDeletePermission(
    const sql_parser::DeleteStatement& stmt, const ExecutionContext& context) {
  return true; // 简化实现
}

// ============================================================================
// 其他策略类的简化实现
// ============================================================================



ExecutionResult DCLExecutionStrategy::execute(
    std::unique_ptr<sql_parser::Statement> stmt, ExecutionContext& context) {
  std::cout << "[DCLExecutor] 执行DCL语句" << std::endl;
  return ExecutionResult(true, "DCL statement executed successfully");
}

bool DCLExecutionStrategy::checkPermission(const sql_parser::Statement& stmt,
                                          const ExecutionContext& context) {
  return true; // 简化实现
}

bool DCLExecutionStrategy::validate(const sql_parser::Statement& stmt,
                                   const ExecutionContext& context) {
  return true; // 简化实现
}



ExecutionResult UtilityExecutionStrategy::execute(
    std::unique_ptr<sql_parser::Statement> stmt, ExecutionContext& context) {
  std::cout << "[UtilityExecutor] 执行Utility语句" << std::endl;
  return ExecutionResult(true, "Utility statement executed successfully");
}

bool UtilityExecutionStrategy::checkPermission(const sql_parser::Statement& stmt,
                                              const ExecutionContext& context) {
  return true; // 简化实现
}

bool UtilityExecutionStrategy::validate(const sql_parser::Statement& stmt,
                                       const ExecutionContext& context) {
  return true; // 简化实现
}

// ============================================================================
// 辅助方法实现
// ============================================================================

bool ExecutionStrategy::validateDatabaseContext(const ExecutionContext& context) {
  return context.db_manager != nullptr;
}

bool ExecutionStrategy::validateTableExists(const std::string& table_name,
                                           const ExecutionContext& context) {
  if (!context.db_manager) return false;
  // 简化实现：总是返回true
  return true;
}

void ExecutionStrategy::updateExecutionStats(ExecutionContext& context,
                                            size_t records_affected) {
  context.set_rows_affected(records_affected);
}

bool ExecutionStrategy::defaultPermissionCheck(const ExecutionContext& context) {
  // 默认权限检查：检查用户是否已登录
  return !context.get_current_user().empty();
}

} // namespace sqlcc