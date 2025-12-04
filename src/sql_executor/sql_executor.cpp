#include "sql_executor.h"
#include "core/permission_validator.h"
#include "database_manager.h"
#include "execution_engine.h"
#include "sql_parser/ast_nodes.h"
#include "sql_parser/parser_new.h"
#include "system_database.h"
#include "user_manager.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

namespace sqlcc {

// 构造函数实现
SqlExecutor::SqlExecutor() {
  db_manager_ = std::make_shared<DatabaseManager>("./data", 1024, 16, 64);
  user_manager_ = std::make_shared<UserManager>();
  InitializeSystemDatabase();
  InitializePermissionValidator();
}

// 新增构造函数：接受DatabaseManager实例
SqlExecutor::SqlExecutor(std::shared_ptr<DatabaseManager> db_manager)
    : db_manager_(db_manager) {
  user_manager_ = std::make_shared<UserManager>();
  InitializeSystemDatabase();
  InitializePermissionValidator();
}

SqlExecutor::~SqlExecutor() = default;

// 执行SQL语句
std::string SqlExecutor::Execute(const std::string &sql) {
  ClearError();
  execution_stats_.clear();

  try {
    // 解析SQL语句
    auto stmt = ParseSQL(sql);
    if (!stmt) {
      SetError("SQL解析失败");
      return "Error: " + GetLastError();
    }

    // 权限验证 - 暂时跳过权限验证，直接执行语句
    // TODO: 实现完整的权限验证系统
    // auto permission_result =
    // permission_validator_->validateStatement(std::move(stmt), current_user_,
    // current_database_); if (!permission_result.allowed) {
    //     SetError("权限验证失败: " + permission_result.message);
    //     return "Error: " + GetLastError();
    // }

    // 创建统一查询计划
    auto query_plan = CreateQueryPlan(std::move(stmt));
    if (!query_plan) {
      SetError("创建查询计划失败");
      return "Error: " + GetLastError();
    }

    // 构建查询计划
    if (!query_plan->buildPlan(std::move(stmt))) {
      SetError("构建查询计划失败: " + query_plan->getErrorMessage());
      return "Error: " + GetLastError();
    }

    // 执行查询计划
    ExecutionResult result = query_plan->executePlan();

    // 保存执行统计信息
    execution_stats_ = query_plan->getExecutionStats();

    // 更新当前数据库（如果是USE语句）
    UpdateCurrentDatabase(sql);

    // 返回结果
    if (result.success) {
      return result.message.empty() ? "Query executed successfully"
                                    : result.message;
    } else {
      SetError(result.message);
      return "Error: " + result.message;
    }
  } catch (const std::exception &e) {
    SetError("Exception occurred: " + std::string(e.what()));
    return "Error: " + GetLastError();
  }
}

std::string SqlExecutor::ExecuteFile(const std::string &file_path) {
  std::ifstream file(file_path);
  if (!file.is_open()) {
    SetError("Failed to open file: " + file_path);
    return "Error: " + GetLastError();
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  file.close();

  return Execute(buffer.str());
}

// 获取最后一次执行的错误信息
std::string SqlExecutor::GetLastError() const { return last_error_; }

// 获取执行统计信息
std::string SqlExecutor::GetExecutionStats() const { return execution_stats_; }

// 设置错误信息
void SqlExecutor::SetError(const std::string &error) { last_error_ = error; }

// 清除错误信息
void SqlExecutor::ClearError() { last_error_.clear(); }

// 初始化系统数据库
bool SqlExecutor::InitializeSystemDatabase() {
  try {
    system_db_ = std::make_shared<SystemDatabase>(db_manager_);
    return true;
  } catch (const std::exception &e) {
    SetError("初始化系统数据库失败: " + std::string(e.what()));
    return false;
  }
}

// 解析SQL语句
std::unique_ptr<sql_parser::Statement>
SqlExecutor::ParseSQL(const std::string &sql) {
  try {
    sql_parser::ParserNew parser(sql);
    auto statements = parser.parse();

    if (statements.empty()) {
      SetError("Empty statement");
      return nullptr;
    }

    // 处理第一条语句（简单起见）
    return std::move(statements[0]);
  } catch (const std::exception &e) {
    SetError("SQL解析异常: " + std::string(e.what()));
    return nullptr;
  }
}

// 创建统一查询计划
std::unique_ptr<UnifiedQueryPlan>
SqlExecutor::CreateQueryPlan(std::unique_ptr<sql_parser::Statement> stmt) {
  try {
    return QueryPlanFactory::createPlan(std::move(stmt), db_manager_,
                                        user_manager_, system_db_);
  } catch (const std::exception &e) {
    SetError("创建查询计划异常: " + std::string(e.what()));
    return nullptr;
  }
}

// 初始化权限验证器
bool SqlExecutor::InitializePermissionValidator() {
  try {
    permission_validator_ =
        std::make_unique<PermissionValidator>(user_manager_, db_manager_);
    current_user_ = "root"; // 默认用户
    current_database_ = ""; // 默认无数据库
    return true;
  } catch (const std::exception &e) {
    SetError("初始化权限验证器失败: " + std::string(e.what()));
    return false;
  }
}

// 更新当前数据库（处理USE语句）
void SqlExecutor::UpdateCurrentDatabase(const std::string &sql) {
  // 简单检查是否是USE语句
  std::string upper_sql = sql;
  std::transform(upper_sql.begin(), upper_sql.end(), upper_sql.begin(),
                 ::toupper);

  if (upper_sql.find("USE ") == 0) {
    // 提取数据库名
    size_t start = 4; // "USE "的长度
    size_t end = upper_sql.find(';');
    if (end == std::string::npos) {
      end = upper_sql.length();
    }

    std::string db_name = sql.substr(start, end - start);
    // 去除前后空格
    db_name.erase(0, db_name.find_first_not_of(" \t\n\r\f\v"));
    db_name.erase(db_name.find_last_not_of(" \t\n\r\f\v") + 1);

    if (!db_name.empty()) {
      current_database_ = db_name;
      // 更新权限验证器的默认数据库
      permission_validator_->setDefaultDatabase(current_database_);
    }
  }
}

} // namespace sqlcc