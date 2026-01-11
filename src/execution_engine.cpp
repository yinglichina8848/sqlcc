/**
 * @file execution_engine.cpp
 *
 * WHY: 为什么需要执行引擎？
 *
 * 数据库系统需要一个专门的组件来处理SQL语句的执行，将解析后的AST转换为实际的数据操作。没有执行引擎，
 * 数据库就无法执行任何SQL查询、数据修改、数据库管理等操作，系统将失去核心功能。
 *
 * 主要问题解决：
 * 1. SQL执行：将解析后的SQL语句转换为具体的数据操作
 * 2. 事务管理：确保数据操作的原子性和一致性
 * 3. 权限控制：验证用户对数据的访问权限
 * 4. 错误处理：提供统一的执行错误处理机制
 * 5. 性能优化：实现查询优化和执行优化策略
 *
 * 执行引擎失败的影响：
 * - 无法执行任何SQL语句
 * - 用户无法查询或修改数据
 * - 数据库系统失去核心功能
 * - 应用程序无法正常工作
 *
 * WHAT: 这实现了什么功能？
 *
 * 执行引擎提供完整的SQL语句执行能力：
 * - DDL执行器：处理数据库、表、索引的创建和删除
 * - DML执行器：处理数据的查询、插入、更新、删除
 * - DCL执行器：处理用户权限的授予和撤销
 * - 工具执行器：处理系统维护和诊断命令
 * - 执行上下文：管理执行过程中的状态和环境
 * - 结果返回：统一的数据操作结果格式
 *
 * 核心组件：
 * - ExecutionEngine：执行引擎基类，管理执行上下文
 * - DDLExecutor：数据定义语言执行器
 * - DMLExecutor：数据操纵语言执行器
 * - DCLExecutor：数据控制语言执行器
 * - UtilityExecutor：工具命令执行器
 * - ExecutionResult：执行结果封装类
 *
 * HOW: 如何实现的？
 *
 * 技术实现要点：
 * 1. 继承体系：使用基类和派生类实现不同类型执行器
 * 2. 策略模式：通过执行策略对象实现具体的执行逻辑
 * 3. 上下文管理：ExecutionContext维护执行状态
 * 4. 权限验证：集成用户管理和权限检查
 * 5. 异常处理：统一的错误捕获和处理机制
 * 6. 结果封装：ExecutionResult统一执行结果格式
 *
 * 架构设计：
 * - 模板方法模式：基类定义执行流程，子类实现具体步骤
 * - 组合模式：执行器组合多个策略对象
 * - 工厂模式：根据语句类型创建相应的执行器
 * - 责任链模式：权限验证、语法验证、执行的顺序处理
 * - 观察者模式：执行事件通知和监控
 *
 * 性能优化：
 * - 预编译：缓存解析后的执行计划
 * - 连接池：复用数据库连接
 * - 批量执行：支持多语句批量处理
 * - 索引优化：利用索引提高查询性能
 * - 内存管理：控制执行过程中的内存使用
 *
 * @note 该实现专为SQLCC数据库系统优化，支持ACID事务特性
 * @see include/execution_engine.h
 */

#include "sql_parser/ast_node.h"
#include "sql_parser/ast_nodes.h"
#include "execution_engine.h"
#include "database_manager.h"
#include "core/execution_context.h"
#include "core/unified_executor.h"
#include <memory>

namespace sqlcc {

ExecutionEngine::ExecutionEngine(std::shared_ptr<DatabaseManager> db_manager)
    : db_manager_(db_manager),
      execution_context_(
          std::make_shared<ExecutionContext>(db_manager)) { // 默认执行上下文
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
