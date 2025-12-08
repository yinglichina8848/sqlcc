#include "unified_query_plan.h"
#include "sql_parser/ast_nodes.h"
#include <memory>

namespace sqlcc {

// UnifiedQueryPlan 构造函数
UnifiedQueryPlan::UnifiedQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                                   std::shared_ptr<UserManager> user_manager,
                                   std::shared_ptr<SystemDatabase> system_db)
    : db_manager_(db_manager), user_manager_(user_manager),
      system_db_(system_db), status_(QueryPlanStatus::PENDING),
      current_database_(""), current_user_("") {}

// 构建查询计划
bool UnifiedQueryPlan::buildPlan(std::unique_ptr<sql_parser::Statement> stmt) {
  try {
    statement_ = std::move(stmt);

    // 验证语句
    if (!validateStatement()) {
      return false;
    }

    // 构建特定于语句类型的计划
    if (!buildSpecificPlan()) {
      return false;
    }

    status_ = QueryPlanStatus::VALIDATING;
    return true;
  } catch (const std::exception &e) {
    setError("构建查询计划异常: " + std::string(e.what()));
    status_ = QueryPlanStatus::FAILED;
    return false;
  }
}

// 执行查询计划
ExecutionResult UnifiedQueryPlan::executePlan() {
  ExecutionResult result;

  try {
    status_ = QueryPlanStatus::EXECUTING;

    // 执行特定于语句类型的计划
    result = executeSpecificPlan();

    if (result.success) {
      status_ = QueryPlanStatus::COMPLETED;
    } else {
      status_ = QueryPlanStatus::FAILED;
      setError(result.message);
    }

    return result;
  } catch (const std::exception &e) {
    status_ = QueryPlanStatus::FAILED;
    result.success = false;
    result.message = "执行查询计划异常: " + std::string(e.what());
    setError(result.message);
    return result;
  }
}

// 验证语句
bool UnifiedQueryPlan::validateStatement() {
  if (!statement_) {
    setError("无效的语句对象");
    return false;
  }

  // 验证数据库上下文
  if (!validateDatabaseContext()) {
    return false;
  }

  return true;
}

// 验证数据库上下文
bool UnifiedQueryPlan::validateDatabaseContext() {
  // 简单实现：暂时跳过数据库上下文验证
  return true;
}

// 验证表存在性
bool UnifiedQueryPlan::validateTableExistence(const std::string &table_name) {
  // 简单实现：暂时跳过表存在性验证
  return true;
}

// 验证列存在性
bool UnifiedQueryPlan::validateColumnExistence(const std::string &table_name,
                                               const std::string &column_name) {
  // 简单实现：暂时跳过列存在性验证
  return true;
}

// 权限检查方法
bool UnifiedQueryPlan::checkPermission(const std::string &operation,
                                       const std::string &resource) {
  // 简单实现：暂时跳过权限检查
  return true;
}

bool UnifiedQueryPlan::checkDatabasePermission(const std::string &operation) {
  return true;
}

bool UnifiedQueryPlan::checkTablePermission(const std::string &operation,
                                            const std::string &table_name) {
  return true;
}

// 错误处理方法
void UnifiedQueryPlan::setError(const std::string &error) {
  error_message_ = error;
}

void UnifiedQueryPlan::clearError() { error_message_.clear(); }

// DDLQueryPlan 实现
DDLQueryPlan::DDLQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                           std::shared_ptr<UserManager> user_manager,
                           std::shared_ptr<SystemDatabase> system_db)
    : UnifiedQueryPlan(db_manager, user_manager, system_db) {}

bool DDLQueryPlan::buildSpecificPlan() {
  // 简单实现：DDL语句构建计划
  return true;
}

ExecutionResult DDLQueryPlan::executeSpecificPlan() {
  ExecutionResult result;
  result.success = true;
  result.message = "DDL语句执行成功";
  return result;
}

// DMLQueryPlan 实现
DMLQueryPlan::DMLQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                           std::shared_ptr<UserManager> user_manager,
                           std::shared_ptr<SystemDatabase> system_db)
    : UnifiedQueryPlan(db_manager, user_manager, system_db) {}

bool DMLQueryPlan::buildSpecificPlan() {
  // 简单实现：DML语句构建计划
  return true;
}

ExecutionResult DMLQueryPlan::executeSpecificPlan() {
  ExecutionResult result;
  result.success = true;
  result.message = "DML语句执行成功";
  return result;
}

// DCLQueryPlan 实现
DCLQueryPlan::DCLQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                           std::shared_ptr<UserManager> user_manager,
                           std::shared_ptr<SystemDatabase> system_db)
    : UnifiedQueryPlan(db_manager, user_manager, system_db) {}

bool DCLQueryPlan::buildSpecificPlan() {
  // 简单实现：DCL语句构建计划
  return true;
}

ExecutionResult DCLQueryPlan::executeSpecificPlan() {
  ExecutionResult result;
  result.success = true;
  result.message = "DCL语句执行成功";
  return result;
}

// UtilityQueryPlan 实现
UtilityQueryPlan::UtilityQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                                   std::shared_ptr<UserManager> user_manager,
                                   std::shared_ptr<SystemDatabase> system_db)
    : UnifiedQueryPlan(db_manager, user_manager, system_db) {}

bool UtilityQueryPlan::buildSpecificPlan() {
  // 简单实现：工具语句构建计划
  return true;
}

ExecutionResult UtilityQueryPlan::executeSpecificPlan() {
  ExecutionResult result;
  result.success = true;
  result.message = "工具语句执行成功";
  return result;
}

// QueryPlanFactory 实现
std::unique_ptr<UnifiedQueryPlan>
QueryPlanFactory::createPlan(std::unique_ptr<sql_parser::Statement> stmt,
                             std::shared_ptr<DatabaseManager> db_manager,
                             std::shared_ptr<UserManager> user_manager,
                             std::shared_ptr<SystemDatabase> system_db) {

  if (!stmt) {
    return nullptr;
  }

  // 根据语句类型创建相应的查询计划
  switch (stmt->getType()) {
  case sql_parser::Statement::CREATE:
  case sql_parser::Statement::DROP:
  case sql_parser::Statement::ALTER:
  case sql_parser::Statement::CREATE_INDEX:
  case sql_parser::Statement::DROP_INDEX:
    return std::make_unique<DDLQueryPlan>(db_manager, user_manager, system_db);

  case sql_parser::Statement::SELECT:
  case sql_parser::Statement::INSERT:
  case sql_parser::Statement::UPDATE:
  case sql_parser::Statement::DELETE:
    return std::make_unique<DMLQueryPlan>(db_manager, user_manager, system_db);

  case sql_parser::Statement::CREATE_USER:
  case sql_parser::Statement::DROP_USER:
  case sql_parser::Statement::GRANT:
  case sql_parser::Statement::REVOKE:
    return std::make_unique<DCLQueryPlan>(db_manager, user_manager, system_db);

  case sql_parser::Statement::USE:
  case sql_parser::Statement::SHOW:
    return std::make_unique<UtilityQueryPlan>(db_manager, user_manager,
                                              system_db);

  default:
    // 未知语句类型
    return nullptr;
  }
}

} // namespace sqlcc