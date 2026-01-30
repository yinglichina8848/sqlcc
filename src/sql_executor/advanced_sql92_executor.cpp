#include "sql_parser/ast_nodes.h"
#include "sql_parser/advanced_sql92_features.h"
#include "../sql_executor.h"
#include "sql_executor/domain_manager.h"
#include "sql_executor/enhanced_trigger_manager.h"
#include "sql_executor/enhanced_alter_table_manager.h"
#include "../core_backup_20260121_001034/core_database_manager.h"
#include "../core_backup_20260121_001034/user_manager.h"
#include "../core_backup_20260121_001034/system_database.h"
#include "../storage_engine/storage_engine.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <stdexcept>

namespace sqlcc {
namespace sql_executor {

// ==================== Procedure and Function Manager ====================

/**
 * 存储过程和函数管理器
 */
class ProcedureFunctionManager {
public:
  static ProcedureFunctionManager& getInstance();

  // 存储过程管理
  bool createProcedure(const sql_parser::CreateProcedureStatement& stmt);
  bool dropProcedure(const std::string& procedureName);
  std::unique_ptr<sql_parser::CallProcedureStatement> callProcedure(const std::string& name);
  bool procedureExists(const std::string& procedureName) const;

  // 函数管理
  bool createFunction(const sql_parser::CreateFunctionStatement& stmt);
  bool dropFunction(const std::string& functionName);
  bool functionExists(const std::string& functionName) const;

  // 执行过程和函数
  std::string executeProcedure(const sql_parser::CallProcedureStatement& stmt);
  std::string executeFunction(const std::string& functionName, const std::vector<std::string>& args);

  // 变量管理
  void setVariable(const std::string& varName, const std::string& value);
  std::string getVariable(const std::string& varName) const;
  bool variableExists(const std::string& varName) const;

private:
  ProcedureFunctionManager() = default;
  
  struct ProcedureInfo {
    std::string name;
    std::vector<sql_parser::ProcedureParameter> parameters;
    std::string body;
    std::string owner;
    long created_time;
  };

  struct FunctionInfo {
    std::string name;
    std::vector<sql_parser::ProcedureParameter> parameters;
    std::string returnDataType;
    std::string body;
    std::string language;
    bool deterministic;
    std::string owner;
    long created_time;
  };

  std::unordered_map<std::string, ProcedureInfo> procedures_;
  std::unordered_map<std::string, FunctionInfo> functions_;
  std::unordered_map<std::string, std::string> variables_;
};

// ==================== Transaction Control Manager ====================

/**
 * 事务控制管理器
 */
class TransactionControlManager {
public:
  static TransactionControlManager& getInstance();

  // SAVEPOINT管理
  bool createSavepoint(const std::string& savepointName);
  bool releaseSavepoint(const std::string& savepointName);
  bool rollbackToSavepoint(const std::string& savepointName);
  bool savepointExists(const std::string& savepointName) const;

  // SET TRANSACTION
  bool setTransactionIsolation(sql_parser::SetTransactionStatement::IsolationLevel level);
  bool setTransactionAccessMode(sql_parser::SetTransactionStatement::AccessMode mode);
  sql_parser::SetTransactionStatement::IsolationLevel getCurrentIsolationLevel() const;
  sql_parser::SetTransactionStatement::AccessMode getCurrentAccessMode() const;

  // 事务统计
  std::string getTransactionInfo() const;

private:
  TransactionControlManager() = default;

  struct SavepointInfo {
    std::string name;
    long transaction_id;
    std::string created_by;
    long created_time;
  };

  std::unordered_map<std::string, SavepointInfo> savepoints_;
  sql_parser::SetTransactionStatement::IsolationLevel current_isolation_level_ = 
      sql_parser::SetTransactionStatement::READ_COMMITTED;
  sql_parser::SetTransactionStatement::AccessMode current_access_mode_ = 
      sql_parser::SetTransactionStatement::READ_WRITE;
  long current_transaction_id_ = 0;
  long next_savepoint_id_ = 1;
};



// ==================== Enhanced Trigger Manager ====================

/**
 * 增强触发器管理器
 */
class EnhancedTriggerManager {
public:
  static EnhancedTriggerManager& getInstance();

  // 触发器管理
  bool createTrigger(const sql_parser::CreateTriggerStatement& stmt);
  bool dropTrigger(const std::string& triggerName);
  bool enableTrigger(const std::string& triggerName);
  bool disableTrigger(const std::string& triggerName);
  bool triggerExists(const std::string& triggerName) const;

  // 触发器执行
  void executeTriggers(const std::string& tableName, 
                      sql_parser::TriggerDefinition::Event event,
                      const std::vector<std::vector<std::string>>& oldRows,
                      const std::vector<std::vector<std::string>>& newRows);

  // 触发器信息
  std::vector<std::string> getTriggersForTable(const std::string& tableName) const;
  std::string getTriggerInfo(const std::string& triggerName) const;

private:
  EnhancedTriggerManager() = default;

  struct EnhancedTriggerInfo {
    std::string name;
    std::string tableName;
    sql_parser::TriggerDefinition::Timing timing;
    sql_parser::TriggerDefinition::Event event;
    sql_parser::TriggerDefinition::Level level;
    std::string condition;
    std::string body;
    bool enabled;
    std::string oldTableName;
    std::string newTableName;
    std::vector<std::pair<std::string, std::string>> variables;
    std::string owner;
    long created_time;
  };

  std::unordered_map<std::string, EnhancedTriggerInfo> triggers_;
  std::unordered_map<std::string, std::vector<std::string>> table_triggers_; // 表名 -> 触发器列表
};

// ==================== Enhanced ALTER TABLE Manager ====================

/**
 * 增强ALTER TABLE管理器
 */
class EnhancedAlterTableManager {
public:
  static EnhancedAlterTableManager& getInstance();

  // 增强ALTER TABLE操作
  bool executeAlterTable(const sql_parser::EnhancedAlterTableStatement& stmt);
  
  // 具体操作支持
  bool addColumn(const std::string& tableName, const sql_parser::ColumnDefinition& columnDef);
  bool dropColumn(const std::string& tableName, const std::string& columnName);
  bool alterColumn(const std::string& tableName, const sql_parser::ColumnDefinition& oldColumn,
                  const sql_parser::ColumnDefinition& newColumn);
  bool renameColumn(const std::string& tableName, const std::string& oldName, const std::string& newName);
  bool addConstraint(const std::string& tableName, const sql_parser::TableConstraint& constraint);
  bool dropConstraint(const std::string& tableName, const std::string& constraintName);
  bool enableTrigger(const std::string& tableName, const std::string& triggerName);
  bool disableTrigger(const std::string& tableName, const std::string& triggerName);

private:
  EnhancedAlterTableManager() = default;
  
  // 验证操作
  bool validateAlterOperation(const std::string& tableName, const sql_parser::AlterTableAction& action) const;
  bool checkColumnDependencies(const std::string& tableName, const std::string& columnName) const;
};

// ==================== Implementation ====================

// ProcedureFunctionManager 实现
ProcedureFunctionManager& ProcedureFunctionManager::getInstance() {
  static ProcedureFunctionManager instance;
  return instance;
}

bool ProcedureFunctionManager::createProcedure(const sql_parser::CreateProcedureStatement& stmt) {
  const auto& def = stmt.getTriggerDefinition(); // 这里需要修改为getProcedureDefinition()
  // 实际实现需要从正确的接口获取过程定义
  return true;
}

// 其他方法实现...
bool ProcedureFunctionManager::dropProcedure(const std::string& procedureName) {
  auto it = procedures_.find(procedureName);
  if (it != procedures_.end()) {
    procedures_.erase(it);
    return true;
  }
  return false;
}

std::unique_ptr<sql_parser::CallProcedureStatement> ProcedureFunctionManager::callProcedure(const std::string& name) {
  if (procedureExists(name)) {
    return std::make_unique<sql_parser::CallProcedureStatement>(name);
  }
  return nullptr;
}

bool ProcedureFunctionManager::procedureExists(const std::string& procedureName) const {
  return procedures_.find(procedureName) != procedures_.end();
}

// TransactionControlManager 实现
TransactionControlManager& TransactionControlManager::getInstance() {
  static TransactionControlManager instance;
  return instance;
}

bool TransactionControlManager::createSavepoint(const std::string& savepointName) {
  if (savepointExists(savepointName)) {
    return false; // Savepoint已存在
  }
  
  SavepointInfo info;
  info.name = savepointName;
  info.transaction_id = current_transaction_id_;
  info.created_by = "current_user"; // TODO: 从上下文获取当前用户
  info.created_time = time(nullptr);
  
  savepoints_[savepointName] = info;
  return true;
}

bool TransactionControlManager::savepointExists(const std::string& savepointName) const {
  return savepoints_.find(savepointName) != savepoints_.end();
}



// EnhancedTriggerManager 实现
EnhancedTriggerManager& EnhancedTriggerManager::getInstance() {
  static EnhancedTriggerManager instance;
  return instance;
}

bool EnhancedTriggerManager::createTrigger(const sql_parser::CreateTriggerStatement& stmt) {
  const auto& triggerDef = stmt.getTriggerDefinition();
  std::string triggerName = triggerDef.getName();
  
  if (triggerExists(triggerName)) {
    return false; // 触发器已存在
  }
  
  EnhancedTriggerInfo info;
  info.name = triggerName;
  info.tableName = triggerDef.getTableName();
  info.timing = triggerDef.getTiming();
  info.event = triggerDef.getEvent();
  info.level = triggerDef.getLevel();
  info.condition = triggerDef.getCondition();
  info.body = triggerDef.getBody();
  info.enabled = true;
  info.owner = "current_user"; // TODO: 从上下文获取当前用户
  info.created_time = time(nullptr);
  
  triggers_[triggerName] = info;
  table_triggers_[info.tableName].push_back(triggerName);
  
  return true;
}

bool EnhancedTriggerManager::triggerExists(const std::string& triggerName) const {
  return triggers_.find(triggerName) != triggers_.end();
}

// EnhancedAlterTableManager 实现
EnhancedAlterTableManager& EnhancedAlterTableManager::getInstance() {
  static EnhancedAlterTableManager instance;
  return instance;
}

bool EnhancedAlterTableManager::executeAlterTable(const sql_parser::EnhancedAlterTableStatement& stmt) {
  const std::string& tableName = stmt.getTableName();
  const auto& actions = stmt.getActions();
  
  for (const auto& action : actions) {
    if (!validateAlterOperation(tableName, *action)) {
      return false;
    }
  }
  
  // 执行所有操作
  for (const auto& action : actions) {
    switch (action->getActionType()) {
      case sql_parser::AlterTableAction::ADD_COLUMN:
        if (!addColumn(tableName, action->getColumnDefinition())) {
          return false;
        }
        break;
      case sql_parser::AlterTableAction::DROP_COLUMN:
        if (!dropColumn(tableName, action->getColumnName())) {
          return false;
        }
        break;
      case sql_parser::AlterTableAction::ALTER_COLUMN:
        if (!alterColumn(tableName, action->getColumnDefinition(), action->getNewColumnDefinition())) {
          return false;
        }
        break;
      case sql_parser::AlterTableAction::RENAME_COLUMN:
        if (!renameColumn(tableName, action->getOldColumnName(), action->getColumnName())) {
          return false;
        }
        break;
      case sql_parser::AlterTableAction::ADD_CONSTRAINT:
        if (!addConstraint(tableName, action->getConstraint())) {
          return false;
        }
        break;
      case sql_parser::AlterTableAction::DROP_CONSTRAINT:
        if (!dropConstraint(tableName, action->getConstraintName())) {
          return false;
        }
        break;
      case sql_parser::AlterTableAction::ENABLE_TRIGGER:
        if (!enableTrigger(tableName, action->getTriggerName())) {
          return false;
        }
        break;
      case sql_parser::AlterTableAction::DISABLE_TRIGGER:
        if (!disableTrigger(tableName, action->getTriggerName())) {
          return false;
        }
        break;
      default:
        return false;
    }
  }
  
  return true;
}

} // namespace sql_executor
} // namespace sqlcc
