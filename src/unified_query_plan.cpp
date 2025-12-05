#include "unified_query_plan.h"
#include "sql_parser/ast_nodes.h"
#include <sstream>

namespace sqlcc {

// UnifiedQueryPlan 实现
UnifiedQueryPlan::UnifiedQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                                   std::shared_ptr<UserManager> user_manager,
                                   std::shared_ptr<SystemDatabase> system_db)
    : db_manager_(db_manager), user_manager_(user_manager),
      system_db_(system_db), status_(QueryPlanStatus::PENDING) {
  // 设置当前上下文
  current_database_ = db_manager_->GetCurrentDatabase();
  // TODO: 需要实现获取当前用户的机制
  current_user_ = "admin"; // 临时使用默认用户
}

bool UnifiedQueryPlan::buildPlan(std::unique_ptr<sql_parser::Statement> stmt) {
  clearError();
  statement_ = std::move(stmt);
  status_ = QueryPlanStatus::VALIDATING;

  // 添加公共步骤
  steps_.clear();
  steps_.push_back(QueryStep(
      QueryStepType::VALIDATION, "验证语句语法",
      [this]() { return validateStatement(); }, true));
  steps_.push_back(QueryStep(
      QueryStepType::VALIDATION, "验证数据库上下文",
      [this]() { return validateDatabaseContext(); }, true));
  steps_.push_back(QueryStep(
      QueryStepType::PERMISSION, "检查操作权限",
      [this]() { return checkPermission(operation_type_, target_object_); },
      true));
  steps_.push_back(QueryStep(
      QueryStepType::PRE_PROCESS, "预处理语句",
      [this]() { return preProcessStatement(); }, true));
  steps_.push_back(QueryStep(
      QueryStepType::PRE_PROCESS, "解析对象引用",
      [this]() { return resolveObjectReferences(); }, true));
  steps_.push_back(QueryStep(
      QueryStepType::PRE_PROCESS, "准备执行上下文",
      [this]() { return prepareExecutionContext(); }, true));

  // 添加特定执行器步骤
  if (!buildSpecificPlan()) {
    setError("构建特定执行计划失败");
    status_ = QueryPlanStatus::FAILED;
    return false;
  }

  // 添加后处理步骤
  steps_.push_back(QueryStep(
      QueryStepType::POST_PROCESS, "后处理语句",
      [this]() { return postProcessStatement(); }, true));
  steps_.push_back(QueryStep(
      QueryStepType::POST_PROCESS, "更新系统元数据",
      [this]() { return updateSystemMetadata(); }, true));
  steps_.push_back(QueryStep(
      QueryStepType::POST_PROCESS, "记录操作日志",
      [this]() { return logOperation(); }, true));
  steps_.push_back(QueryStep(
      QueryStepType::CLEANUP, "清理执行上下文", [this]() { return true; },
      true));

  status_ = QueryPlanStatus::PENDING;
  return true;
}

ExecutionResult UnifiedQueryPlan::executePlan() {
  status_ = QueryPlanStatus::EXECUTING;
  clearError();

  std::stringstream stats_stream;
  stats_stream << "执行步骤统计:\n";

  // 执行所有步骤
  for (size_t i = 0; i < steps_.size(); ++i) {
    const auto &step = steps_[i];
    stats_stream << "步骤 " << (i + 1) << ": " << step.description << " - ";

    if (!step.action()) {
      if (step.required) {
        stats_stream << "失败\n";
        status_ = QueryPlanStatus::FAILED;
        execution_stats_ = stats_stream.str();
        return {false, error_message_};
      } else {
        stats_stream << "跳过\n";
      }
    } else {
      stats_stream << "成功\n";
    }
  }

  // 执行特定计划
  auto result = executeSpecificPlan();
  if (!result.success) {
    status_ = QueryPlanStatus::FAILED;
    execution_stats_ = stats_stream.str();
    return result;
  }

  status_ = QueryPlanStatus::COMPLETED;
  execution_stats_ = stats_stream.str();
  return result;
}

bool UnifiedQueryPlan::validateStatement() {
  if (!statement_) {
    setError("语句为空");
    return false;
  }

  // 根据语句类型设置操作类型和目标对象
  if (auto create_stmt =
          dynamic_cast<sql_parser::CreateStatement *>(statement_.get())) {
    if (create_stmt->getObjectType() == sql_parser::CreateStatement::DATABASE) {
      operation_type_ = "CREATE_DATABASE";
      target_object_ = create_stmt->getObjectName();
    } else if (create_stmt->getObjectType() ==
               sql_parser::CreateStatement::TABLE) {
      operation_type_ = "CREATE_TABLE";
      target_object_ = create_stmt->getObjectName();
    }
  } else if (auto drop_stmt =
                 dynamic_cast<sql_parser::DropStatement *>(statement_.get())) {
    if (drop_stmt->getObjectType() == sql_parser::DropStatement::DATABASE) {
      operation_type_ = "DROP_DATABASE";
      target_object_ = drop_stmt->getObjectName();
    } else if (drop_stmt->getObjectType() == sql_parser::DropStatement::TABLE) {
      operation_type_ = "DROP_TABLE";
      target_object_ = drop_stmt->getObjectName();
    }
  } else if (dynamic_cast<sql_parser::SelectStatement *>(statement_.get())) {
    operation_type_ = "SELECT";
    target_object_ = "TABLE";
  } else if (dynamic_cast<sql_parser::InsertStatement *>(statement_.get())) {
    operation_type_ = "INSERT";
    target_object_ = "TABLE";
  } else if (dynamic_cast<sql_parser::UpdateStatement *>(statement_.get())) {
    operation_type_ = "UPDATE";
    target_object_ = "TABLE";
  } else if (dynamic_cast<sql_parser::DeleteStatement *>(statement_.get())) {
    operation_type_ = "DELETE";
    target_object_ = "TABLE";
  } else if (dynamic_cast<sql_parser::CreateUserStatement *>(
                 statement_.get())) {
    operation_type_ = "CREATE_USER";
    target_object_ = "USER";
  } else if (dynamic_cast<sql_parser::DropUserStatement *>(statement_.get())) {
    operation_type_ = "DROP_USER";
    target_object_ = "USER";
  } else if (dynamic_cast<sql_parser::GrantStatement *>(statement_.get())) {
    operation_type_ = "GRANT";
    target_object_ = "PRIVILEGE";
  } else if (dynamic_cast<sql_parser::RevokeStatement *>(statement_.get())) {
    operation_type_ = "REVOKE";
    target_object_ = "PRIVILEGE";
  } else if (dynamic_cast<sql_parser::UseStatement *>(statement_.get())) {
    operation_type_ = "USE";
    target_object_ = "DATABASE";
  } else if (dynamic_cast<sql_parser::ShowStatement *>(statement_.get())) {
    operation_type_ = "SHOW";
    target_object_ = "OBJECT";
  } else {
    operation_type_ = "UNKNOWN";
    target_object_ = "UNKNOWN";
  }

  return true;
}

bool UnifiedQueryPlan::validateDatabaseContext() {
  // 不需要数据库上下文的操作类型
  std::vector<std::string> no_context_operations = {
      "CREATE_DATABASE", "CREATE_USER", "DROP_USER", "GRANT", "REVOKE", "USE"};

  // 检查当前操作是否需要数据库上下文
  bool needs_context =
      std::find(no_context_operations.begin(), no_context_operations.end(),
                operation_type_) == no_context_operations.end();

  if (needs_context && current_database_.empty()) {
    setError("未选择数据库");
    return false;
  }
  return true;
}

bool UnifiedQueryPlan::validateTableExistence(const std::string &table_name) {
  // 检查数据库是否存在
  if (!db_manager_->DatabaseExists(current_database_)) {
    setError("数据库不存在: " + current_database_);
    return false;
  }

  // 检查表是否存在
  if (!db_manager_->TableExists(table_name)) {
    setError("表不存在: " + table_name);
    return false;
  }

  return true;
}

bool UnifiedQueryPlan::validateColumnExistence(const std::string &table_name,
                                               const std::string &column_name) {
  // 检查数据库是否存在
  if (!db_manager_->DatabaseExists(current_database_)) {
    setError("数据库不存在: " + current_database_);
    return false;
  }

  // 检查表是否存在
  if (!db_manager_->TableExists(table_name)) {
    setError("表不存在: " + table_name);
    return false;
  }

  // 检查列是否存在
  // TODO: 实现列存在性检查
  return true;
}

bool UnifiedQueryPlan::checkPermission(const std::string &operation,
                                       const std::string &resource) {
  // 统一权限检查逻辑
  if (operation == "CREATE_DATABASE" || operation == "DROP_DATABASE") {
    return checkDatabasePermission(operation);
  } else if (operation == "CREATE_TABLE" || operation == "DROP_TABLE" ||
             operation == "ALTER_TABLE" || operation == "INSERT" ||
             operation == "UPDATE" || operation == "DELETE" ||
             operation == "SELECT") {
    return checkTablePermission(operation, resource);
  } else if (operation == "CREATE_USER" || operation == "DROP_USER" ||
             operation == "GRANT" || operation == "REVOKE") {
    // DCL语句权限检查 - 目前默认允许
    return true;
  }

  return true; // 默认允许
}

bool UnifiedQueryPlan::checkDatabasePermission(const std::string &operation) {
  // 检查数据库级别权限
  // TODO: 实现数据库权限检查
  return true;
}

bool UnifiedQueryPlan::checkTablePermission(const std::string &operation,
                                            const std::string &table_name) {
  // 检查表级别权限
  // TODO: 实现表权限检查
  return true;
}

bool UnifiedQueryPlan::preProcessStatement() {
  // 预处理逻辑
  return true;
}

bool UnifiedQueryPlan::resolveObjectReferences() {
  // 解析对象引用
  return true;
}

bool UnifiedQueryPlan::prepareExecutionContext() {
  // 准备执行上下文
  return true;
}

bool UnifiedQueryPlan::postProcessStatement() {
  // 后处理逻辑
  return true;
}

bool UnifiedQueryPlan::updateSystemMetadata() {
  // 更新系统元数据
  return true;
}

bool UnifiedQueryPlan::logOperation() {
  // 记录操作日志
  return true;
}

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
  if (auto create_stmt =
          dynamic_cast<sql_parser::CreateStatement *>(statement_.get())) {
    return buildCreatePlan();
  } else if (auto drop_stmt =
                 dynamic_cast<sql_parser::DropStatement *>(statement_.get())) {
    return buildDropPlan();
  } else if (auto alter_stmt =
                 dynamic_cast<sql_parser::AlterStatement *>(statement_.get())) {
    return buildAlterPlan();
  }

  setError("不支持的DDL语句类型");
  return false;
}

ExecutionResult DDLQueryPlan::executeSpecificPlan() {
  if (auto create_stmt =
          dynamic_cast<sql_parser::CreateStatement *>(statement_.get())) {
    return executeCreatePlan();
  } else if (auto drop_stmt =
                 dynamic_cast<sql_parser::DropStatement *>(statement_.get())) {
    return executeDropPlan();
  } else if (auto alter_stmt =
                 dynamic_cast<sql_parser::AlterStatement *>(statement_.get())) {
    return executeAlterPlan();
  }

  return {false, "不支持的DDL语句类型"};
}

bool DDLQueryPlan::buildCreatePlan() {
  // 添加CREATE语句特定的执行步骤
  steps_.push_back(QueryStep(
      QueryStepType::EXECUTION, "执行CREATE操作", [this]() { return true; },
      true));
  return true;
}

bool DDLQueryPlan::buildDropPlan() {
  // 添加DROP语句特定的执行步骤
  steps_.push_back(QueryStep(
      QueryStepType::EXECUTION, "执行DROP操作", [this]() { return true; },
      true));
  return true;
}

bool DDLQueryPlan::buildAlterPlan() {
  // 添加ALTER语句特定的执行步骤
  steps_.push_back(QueryStep(
      QueryStepType::EXECUTION, "执行ALTER操作", [this]() { return true; },
      true));
  return true;
}

ExecutionResult DDLQueryPlan::executeCreatePlan() {
  // 实现CREATE语句的执行逻辑
  return {true, "CREATE操作执行成功"};
}

ExecutionResult DDLQueryPlan::executeDropPlan() {
  // 实现DROP语句的执行逻辑
  return {true, "DROP操作执行成功"};
}

ExecutionResult DDLQueryPlan::executeAlterPlan() {
  // 实现ALTER语句的执行逻辑
  // 这里暂时返回成功，实际实现需要根据具体的ALTER操作类型进行处理
  return {true, "ALTER操作执行成功"};
}

// DMLQueryPlan 实现
DMLQueryPlan::DMLQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                           std::shared_ptr<UserManager> user_manager,
                           std::shared_ptr<SystemDatabase> system_db)
    : UnifiedQueryPlan(db_manager, user_manager, system_db) {}

bool DMLQueryPlan::buildSpecificPlan() {
  // TODO: 实现DML特定计划构建
  return true;
}

ExecutionResult DMLQueryPlan::executeSpecificPlan() {
  // TODO: 实现DML特定计划执行
  return {true, "DML执行成功"};
}

// DCLQueryPlan 实现
DCLQueryPlan::DCLQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                           std::shared_ptr<UserManager> user_manager,
                           std::shared_ptr<SystemDatabase> system_db)
    : UnifiedQueryPlan(db_manager, user_manager, system_db) {}

bool DCLQueryPlan::buildSpecificPlan() {
  // TODO: 实现DCL特定计划构建
  return true;
}

ExecutionResult DCLQueryPlan::executeSpecificPlan() {
  // TODO: 实现DCL特定计划执行
  return {true, "DCL执行成功"};
}

// UtilityQueryPlan 实现
UtilityQueryPlan::UtilityQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                                   std::shared_ptr<UserManager> user_manager,
                                   std::shared_ptr<SystemDatabase> system_db)
    : UnifiedQueryPlan(db_manager, user_manager, system_db) {}

bool UtilityQueryPlan::buildSpecificPlan() {
  // TODO: 实现工具特定计划构建
  return true;
}

ExecutionResult UtilityQueryPlan::executeSpecificPlan() {
  // TODO: 实现工具特定计划执行
  return {true, "工具执行成功"};
}

// QueryPlanFactory 实现
std::unique_ptr<UnifiedQueryPlan>
QueryPlanFactory::createPlan(std::unique_ptr<sql_parser::Statement> stmt,
                             std::shared_ptr<DatabaseManager> db_manager,
                             std::shared_ptr<UserManager> user_manager,
                             std::shared_ptr<SystemDatabase> system_db) {

  if (dynamic_cast<sql_parser::CreateStatement *>(stmt.get()) ||
      dynamic_cast<sql_parser::DropStatement *>(stmt.get()) ||
      dynamic_cast<sql_parser::AlterStatement *>(stmt.get())) {
    return std::make_unique<DDLQueryPlan>(db_manager, user_manager, system_db);
  } else if (dynamic_cast<sql_parser::SelectStatement *>(stmt.get()) ||
             dynamic_cast<sql_parser::InsertStatement *>(stmt.get()) ||
             dynamic_cast<sql_parser::UpdateStatement *>(stmt.get()) ||
             dynamic_cast<sql_parser::DeleteStatement *>(stmt.get())) {
    return std::make_unique<DMLQueryPlan>(db_manager, user_manager, system_db);
  } else if (dynamic_cast<sql_parser::CreateUserStatement *>(stmt.get()) ||
             dynamic_cast<sql_parser::DropUserStatement *>(stmt.get()) ||
             dynamic_cast<sql_parser::GrantStatement *>(stmt.get()) ||
             dynamic_cast<sql_parser::RevokeStatement *>(stmt.get())) {
    return std::make_unique<DCLQueryPlan>(db_manager, user_manager, system_db);
  } else if (dynamic_cast<sql_parser::CreateProcedureStatement *>(stmt.get()) ||
             dynamic_cast<sql_parser::CallProcedureStatement *>(stmt.get()) ||
             dynamic_cast<sql_parser::DropProcedureStatement *>(stmt.get())) {
    return std::make_unique<ProcedureQueryPlan>(db_manager, user_manager,
                                                system_db);
  } else if (dynamic_cast<sql_parser::CreateTriggerStatement *>(stmt.get()) ||
             dynamic_cast<sql_parser::DropTriggerStatement *>(stmt.get()) ||
             dynamic_cast<sql_parser::AlterTriggerStatement *>(stmt.get())) {
    return std::make_unique<TriggerQueryPlan>(db_manager, user_manager,
                                              system_db);
  } else {
    return std::make_unique<UtilityQueryPlan>(db_manager, user_manager,
                                              system_db);
  }
}

// ProcedureQueryPlan 实现
ProcedureQueryPlan::ProcedureQueryPlan(
    std::shared_ptr<DatabaseManager> db_manager,
    std::shared_ptr<UserManager> user_manager,
    std::shared_ptr<SystemDatabase> system_db)
    : UnifiedQueryPlan(db_manager, user_manager, system_db) {}

bool ProcedureQueryPlan::buildSpecificPlan() {
  // 根据语句类型构建特定的执行计划
  if (auto create_proc = dynamic_cast<sql_parser::CreateProcedureStatement *>(
          statement_.get())) {
    return buildCreateProcedurePlan();
  } else if (auto call_proc =
                 dynamic_cast<sql_parser::CallProcedureStatement *>(
                     statement_.get())) {
    return buildCallProcedurePlan();
  } else if (auto drop_proc =
                 dynamic_cast<sql_parser::DropProcedureStatement *>(
                     statement_.get())) {
    return buildDropProcedurePlan();
  }
  return false;
}

bool ProcedureQueryPlan::buildCreateProcedurePlan() {
  // 添加创建存储过程的执行步骤
  steps_.push_back(QueryStep(
      QueryStepType::EXECUTION, "创建存储过程", [this]() { return true; },
      true));
  steps_.push_back(QueryStep(
      QueryStepType::POST_PROCESS, "更新存储过程元数据",
      [this]() { return true; }, true));
  return true;
}

bool ProcedureQueryPlan::buildCallProcedurePlan() {
  // 添加调用存储过程的执行步骤
  steps_.push_back(QueryStep(
      QueryStepType::EXECUTION, "调用存储过程", [this]() { return true; },
      true));
  return true;
}

bool ProcedureQueryPlan::buildDropProcedurePlan() {
  // 添加删除存储过程的执行步骤
  steps_.push_back(QueryStep(
      QueryStepType::EXECUTION, "删除存储过程", [this]() { return true; },
      true));
  steps_.push_back(QueryStep(
      QueryStepType::POST_PROCESS, "更新存储过程元数据",
      [this]() { return true; }, true));
  return true;
}

ExecutionResult ProcedureQueryPlan::executeSpecificPlan() {
  // 根据语句类型执行特定的计划
  if (auto create_proc = dynamic_cast<sql_parser::CreateProcedureStatement *>(
          statement_.get())) {
    return executeCreateProcedurePlan();
  } else if (auto call_proc =
                 dynamic_cast<sql_parser::CallProcedureStatement *>(
                     statement_.get())) {
    return executeCallProcedurePlan();
  } else if (auto drop_proc =
                 dynamic_cast<sql_parser::DropProcedureStatement *>(
                     statement_.get())) {
    return executeDropProcedurePlan();
  }
  return {false, "不支持的存储过程操作"};
}

ExecutionResult ProcedureQueryPlan::executeCreateProcedurePlan() {
  // 实现创建存储过程的执行逻辑
  return {true, "存储过程创建成功"};
}

ExecutionResult ProcedureQueryPlan::executeCallProcedurePlan() {
  // 实现调用存储过程的执行逻辑
  return {true, "存储过程调用成功"};
}

ExecutionResult ProcedureQueryPlan::executeDropProcedurePlan() {
  // 实现删除存储过程的执行逻辑
  return {true, "存储过程删除成功"};
}

// TriggerQueryPlan 实现
TriggerQueryPlan::TriggerQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                                   std::shared_ptr<UserManager> user_manager,
                                   std::shared_ptr<SystemDatabase> system_db)
    : UnifiedQueryPlan(db_manager, user_manager, system_db),
      trigger_def_("", sql_parser::TriggerDefinition::BEFORE,
                   sql_parser::TriggerDefinition::INSERT,
                   sql_parser::TriggerDefinition::ROW, "") {}

bool TriggerQueryPlan::buildSpecificPlan() {
  // 根据语句类型构建特定的执行计划
  if (auto create_trigger = dynamic_cast<sql_parser::CreateTriggerStatement *>(
          statement_.get())) {
    return buildCreateTriggerPlan();
  } else if (auto drop_trigger =
                 dynamic_cast<sql_parser::DropTriggerStatement *>(
                     statement_.get())) {
    return buildDropTriggerPlan();
  } else if (auto alter_trigger =
                 dynamic_cast<sql_parser::AlterTriggerStatement *>(
                     statement_.get())) {
    return buildAlterTriggerPlan();
  }
  return false;
}

bool TriggerQueryPlan::buildCreateTriggerPlan() {
  // 添加创建触发器的执行步骤
  steps_.push_back(QueryStep(
      QueryStepType::EXECUTION, "创建触发器", [this]() { return true; }, true));
  steps_.push_back(QueryStep(
      QueryStepType::POST_PROCESS, "更新触发器元数据",
      [this]() { return true; }, true));
  return true;
}

bool TriggerQueryPlan::buildDropTriggerPlan() {
  // 添加删除触发器的执行步骤
  steps_.push_back(QueryStep(
      QueryStepType::EXECUTION, "删除触发器", [this]() { return true; }, true));
  steps_.push_back(QueryStep(
      QueryStepType::POST_PROCESS, "更新触发器元数据",
      [this]() { return true; }, true));
  return true;
}

bool TriggerQueryPlan::buildAlterTriggerPlan() {
  // 添加修改触发器的执行步骤
  steps_.push_back(QueryStep(
      QueryStepType::EXECUTION, "修改触发器", [this]() { return true; }, true));
  steps_.push_back(QueryStep(
      QueryStepType::POST_PROCESS, "更新触发器元数据",
      [this]() { return true; }, true));
  return true;
}

ExecutionResult TriggerQueryPlan::executeSpecificPlan() {
  // 根据语句类型执行特定的计划
  if (auto create_trigger = dynamic_cast<sql_parser::CreateTriggerStatement *>(
          statement_.get())) {
    return executeCreateTriggerPlan();
  } else if (auto drop_trigger =
                 dynamic_cast<sql_parser::DropTriggerStatement *>(
                     statement_.get())) {
    return executeDropTriggerPlan();
  } else if (auto alter_trigger =
                 dynamic_cast<sql_parser::AlterTriggerStatement *>(
                     statement_.get())) {
    return executeAlterTriggerPlan();
  }
  return {false, "不支持的触发器操作"};
}

ExecutionResult TriggerQueryPlan::executeCreateTriggerPlan() {
  // 实现创建触发器的执行逻辑
  return {true, "触发器创建成功"};
}

ExecutionResult TriggerQueryPlan::executeDropTriggerPlan() {
  // 实现删除触发器的执行逻辑
  return {true, "触发器删除成功"};
}

ExecutionResult TriggerQueryPlan::executeAlterTriggerPlan() {
  // 实现修改触发器的执行逻辑
  return {true, "触发器修改成功"};
}

} // namespace sqlcc