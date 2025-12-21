#include "sql_executor/transaction_control_manager.h"
#include <iostream>
#include <ctime>
#include <sstream>

// TransactionControlManager 实现
namespace sqlcc {
namespace sql_executor {

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

bool TransactionControlManager::releaseSavepoint(const std::string& savepointName) {
  if (!savepointExists(savepointName)) {
    return false; // Savepoint不存在
  }

  savepoints_.erase(savepointName);
  return true;
}

bool TransactionControlManager::rollbackToSavepoint(const std::string& savepointName) {
  if (!savepointExists(savepointName)) {
    return false; // Savepoint不存在
  }

  // TODO: 实际的回滚逻辑
  // 这里应该执行回滚到指定savepoint的操作
  return true;
}

bool TransactionControlManager::savepointExists(const std::string& savepointName) const {
  return savepoints_.find(savepointName) != savepoints_.end();
}

bool TransactionControlManager::setTransactionIsolation(sql_parser::SetTransactionStatement::IsolationLevel level) {
  current_isolation_level_ = level;
  return true;
}

bool TransactionControlManager::setTransactionAccessMode(sql_parser::SetTransactionStatement::AccessMode mode) {
  current_access_mode_ = mode;
  return true;
}

sql_parser::SetTransactionStatement::IsolationLevel TransactionControlManager::getCurrentIsolationLevel() const {
  return current_isolation_level_;
}

sql_parser::SetTransactionStatement::AccessMode TransactionControlManager::getCurrentAccessMode() const {
  return current_access_mode_;
}

std::string TransactionControlManager::getTransactionInfo() const {
  std::stringstream ss;
  ss << "Transaction ID: " << current_transaction_id_ << "\n";
  ss << "Isolation Level: " << static_cast<int>(current_isolation_level_) << "\n";
  ss << "Access Mode: " << static_cast<int>(current_access_mode_) << "\n";
  ss << "Savepoints: " << savepoints_.size() << "\n";

  return ss.str();
}

} // namespace sql_executor
} // namespace sqlcc
