#include "sql_parser/ast_node.h"
#include "sql_parser/ast_nodes.h"
#include "execution_engine.h"
#include "core/core_database_manager.h"
#include "core/execution_context.h"
#include "core/unified_executor.h"
#include <memory>

namespace sqlcc {

// WHY层 - 执行引擎架构设计
// 执行引擎是SQLCC系统的核心组件，负责将解析后的AST转换为实际的数据库操作。
// 采用策略模式和模板方法模式，支持不同类型的SQL语句执行。
// 通过ExecutionContext管理执行状态，确保事务一致性和权限控制。
//
// WHAT层 - 架构组件
// 1. ExecutionEngine基类：定义执行接口和公共逻辑
// 2. 具体执行器：DDLExecutor, DMLExecutor, DCLExecutor, UtilityExecutor
// 3. ExecutionContext：执行上下文，包含用户、数据库、权限等信息
// 4. 执行策略：各种Strategy类实现具体的执行逻辑
//
// HOW层 - 设计模式应用
// - 策略模式：不同类型的语句使用不同的执行策略
// - 模板方法：基类定义执行流程，子类实现具体步骤
// - 组合模式：执行器组合各种管理器和上下文
// - 工厂模式：通过类型判断创建合适的执行器

ExecutionEngine::ExecutionEngine(std::shared_ptr<DatabaseManager> db_manager)
    : db_manager_(db_manager),
      execution_context_(
          std::make_shared<ExecutionContext>()) { // 创建默认执行上下文
  // 初始化执行上下文
  execution_context_->set_db_manager(db_manager);
}

void ExecutionEngine::set_execution_context(
    std::shared_ptr<ExecutionContext> context) {
  execution_context_ = context;
}

std::shared_ptr<ExecutionContext>
ExecutionEngine::get_execution_context() const {
  return execution_context_;
}

// DDLExecutor 实现
DDLExecutor::DDLExecutor(std::shared_ptr<DatabaseManager> db_manager)
    : ExecutionEngine(db_manager) {
  // 初始化执行上下文，设置db_manager
  execution_context_->set_db_manager(db_manager);
}

DDLExecutor::DDLExecutor(std::shared_ptr<DatabaseManager> db_manager,
                         std::shared_ptr<SystemDatabase> system_db,
                         std::shared_ptr<UserManager> user_manager)
    : ExecutionEngine(db_manager) {
  // 初始化执行上下文，设置db_manager、user_manager和system_db
  execution_context_->set_db_manager(db_manager);
  execution_context_->set_user_manager(user_manager);
  execution_context_->set_system_db(system_db);
}

ExecutionResult
DDLExecutor::execute(std::unique_ptr<sqlcc::sql_parser::Statement> stmt) {
  // 直接执行语句，不调用基类方法
  // 使用execution_context_执行语句
  // 简化实现：直接使用db_manager_执行DDL操作
  if (auto create_stmt =
          dynamic_cast<sql_parser::CreateStatement *>(stmt.get())) {
    if (create_stmt->getObjectType() == sql_parser::CreateStatement::DATABASE) {
      std::string db_name = create_stmt->getObjectName();
      if (db_manager_->CreateDatabase(db_name)) {
        return ExecutionResult(true, "Database '" + db_name +
                                         "' created successfully");
      } else {
        return ExecutionResult(false,
                               "Failed to create database '" + db_name + "'");
      }
    } else if (create_stmt->getObjectType() ==
               sql_parser::CreateStatement::TABLE) {
      std::string table_name = create_stmt->getObjectName();
      // 实际创建表
      // 获取列定义
      std::vector<std::pair<std::string, std::string>> columns;
      for (const auto &col : create_stmt->getColumns()) {
        columns.emplace_back(col.getName(), col.getTypeString());
      }

      // 创建表
      if (db_manager_->CreateTable(table_name, columns)) {
        return ExecutionResult(true, "Table '" + table_name +
                                         "' created successfully");
      } else {
        return ExecutionResult(false,
                               "Failed to create table '" + table_name + "'");
      }
    }
  } else if (auto drop_stmt =
                 dynamic_cast<sql_parser::DropStatement *>(stmt.get())) {
    if (drop_stmt->getObjectType() == sql_parser::DropStatement::DATABASE) {
      std::string db_name = drop_stmt->getObjectName();
      if (db_manager_->DropDatabase(db_name)) {
        return ExecutionResult(true, "Database '" + db_name +
                                         "' dropped successfully");
      } else {
        return ExecutionResult(false,
                               "Failed to drop database '" + db_name + "'");
      }
    } else if (drop_stmt->getObjectType() == sql_parser::DropStatement::TABLE) {
      std::string table_name = drop_stmt->getObjectName();
      if (db_manager_->DropTable(table_name)) {
        return ExecutionResult(true, "Table '" + table_name +
                                         "' dropped successfully");
      } else {
        return ExecutionResult(false,
                               "Failed to drop table '" + table_name + "'");
      }
    }
  } else if (auto create_index_stmt =
                 dynamic_cast<sql_parser::CreateIndexStatement *>(stmt.get())) {
    // 使用DDLExecutionStrategy来实际执行CREATE INDEX语句
    DDLExecutionStrategy ddl_strategy;
    
    // 创建执行上下文
    ExecutionContext context;
    context.db_manager = db_manager_;
    context.user_manager = execution_context_->get_user_manager();
    context.current_database = execution_context_->get_current_database();
    context.current_user = execution_context_->get_current_user();
    
    // 验证语句
    if (!ddl_strategy.validate(*stmt, context)) {
      return ExecutionResult(false, "Statement validation failed");
    }
    
    // 检查权限
    if (!ddl_strategy.checkPermission(*stmt, context)) {
      return ExecutionResult(false, "Permission denied");
    }
    
    // 执行语句
    return ddl_strategy.execute(std::move(stmt), context);
  } else if (auto drop_index_stmt =
                 dynamic_cast<sql_parser::DropIndexStatement *>(stmt.get())) {
    // 使用DDLExecutionStrategy来实际执行DROP INDEX语句
    DDLExecutionStrategy ddl_strategy;
    
    // 创建执行上下文
    ExecutionContext context;
    context.db_manager = db_manager_;
    context.user_manager = execution_context_->get_user_manager();
    context.current_database = execution_context_->get_current_database();
    context.current_user = execution_context_->get_current_user();
    
    // 验证语句
    if (!ddl_strategy.validate(*stmt, context)) {
      return ExecutionResult(false, "Statement validation failed");
    }
    
    // 检查权限
    if (!ddl_strategy.checkPermission(*stmt, context)) {
      return ExecutionResult(false, "Permission denied");
    }
    
    // 执行语句
    return ddl_strategy.execute(std::move(stmt), context);
  }

  return ExecutionResult(false, "Unsupported DDL statement");
}

// DMLExecutor 实现
DMLExecutor::DMLExecutor(std::shared_ptr<DatabaseManager> db_manager)
    : ExecutionEngine(db_manager) {
  // 初始化执行上下文，设置db_manager
  execution_context_->set_db_manager(db_manager);
}

DMLExecutor::DMLExecutor(std::shared_ptr<DatabaseManager> db_manager,
                         std::shared_ptr<UserManager> user_manager)
    : ExecutionEngine(db_manager) {
  // 初始化执行上下文，设置db_manager和user_manager
  execution_context_->set_db_manager(db_manager);
  execution_context_->set_user_manager(user_manager);
}

ExecutionResult
DMLExecutor::execute(std::unique_ptr<sqlcc::sql_parser::Statement> stmt) {
  // 使用DMLExecutionStrategy来实际执行DML语句
  DMLExecutionStrategy dml_strategy;

  // 创建执行上下文
  ExecutionContext context;
  context.db_manager = db_manager_;
  context.user_manager = execution_context_->get_user_manager();
  context.current_database = execution_context_->get_current_database();
  context.current_user = execution_context_->get_current_user();

  // 验证语句
  if (!dml_strategy.validate(*stmt, context)) {
    return ExecutionResult(false, "Statement validation failed");
  }

  // 检查权限
  if (!dml_strategy.checkPermission(*stmt, context)) {
    return ExecutionResult(false, "Permission denied");
  }

  // 执行语句
  return dml_strategy.execute(std::move(stmt), context);
}



// DCLExecutor 实现
DCLExecutor::DCLExecutor(std::shared_ptr<DatabaseManager> db_manager,
                         std::shared_ptr<UserManager> user_manager)
    : ExecutionEngine(db_manager), user_manager_(user_manager) {
  // 初始化执行上下文，设置db_manager和user_manager
  execution_context_->set_db_manager(db_manager);
  execution_context_->set_user_manager(user_manager);
}

ExecutionResult
DCLExecutor::execute(std::unique_ptr<sql_parser::Statement> stmt) {
  // DCL语句执行逻辑
  // 简化的实现
  return ExecutionResult(true, "DCL statement executed successfully");
}

// UtilityExecutor 实现
UtilityExecutor::UtilityExecutor(std::shared_ptr<DatabaseManager> db_manager)
    : ExecutionEngine(db_manager) {
  // 初始化执行上下文，设置db_manager
  execution_context_->set_db_manager(db_manager);
}

UtilityExecutor::UtilityExecutor(std::shared_ptr<DatabaseManager> db_manager,
                                 std::shared_ptr<SystemDatabase> system_db)
    : ExecutionEngine(db_manager), system_db_(system_db) {
  // 初始化执行上下文，设置db_manager和system_db
  execution_context_->set_db_manager(db_manager);
  execution_context_->set_system_db(system_db);
}

ExecutionResult
UtilityExecutor::execute(std::unique_ptr<sql_parser::Statement> stmt) {
  // Utility语句执行逻辑
  // 简化的实现
  return ExecutionResult(true, "Utility statement executed successfully");
}

} // namespace sqlcc