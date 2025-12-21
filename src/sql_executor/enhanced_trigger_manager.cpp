#include "sql_executor/enhanced_trigger_manager.h"
#include <iostream>
#include <ctime>
#include <sstream>

// EnhancedTriggerManager 实现
namespace sqlcc {
namespace sql_executor {

EnhancedTriggerManager& EnhancedTriggerManager::getInstance() {
  static EnhancedTriggerManager instance;
  return instance;
}

bool EnhancedTriggerManager::createTrigger(const sql_parser::CreateTriggerStatement& stmt) {
  const auto& def = stmt.getTriggerDefinition();
  std::string triggerName = def.getName();

  if (triggerExists(triggerName)) {
    return false; // 触发器已存在
  }

  TriggerInfo info;
  info.name = triggerName;
  info.tableName = def.getTableName();
  info.eventType = def.getEventType();
  info.timing = def.getTiming();
  info.body = def.getBody();
  info.whenCondition = def.getWhenCondition();
  info.enabled = true;
  info.created_time = time(nullptr);
  info.created_by = "current_user"; // TODO: 从上下文获取当前用户
  info.dependencies = def.getDependencies();

  triggers_[triggerName] = info;
  table_triggers_[info.tableName].push_back(triggerName);

  return true;
}

bool EnhancedTriggerManager::alterTrigger(const sql_parser::AlterTriggerStatement& stmt) {
  std::string triggerName = stmt.getTriggerName();

  if (!triggerExists(triggerName)) {
    return false; // 触发器不存在
  }

  // TODO: 实现触发器修改逻辑
  return true;
}

bool EnhancedTriggerManager::dropTrigger(const sql_parser::DropTriggerStatement& stmt) {
  std::string triggerName = stmt.getTriggerName();

  if (!triggerExists(triggerName)) {
    return false; // 触发器不存在
  }

  // 从表触发器列表中移除
  const auto& info = triggers_[triggerName];
  auto& tableList = table_triggers_[info.tableName];
  tableList.erase(std::remove(tableList.begin(), tableList.end(), triggerName), tableList.end());

  triggers_.erase(triggerName);
  return true;
}

bool EnhancedTriggerManager::triggerExists(const std::string& triggerName) const {
  return triggers_.find(triggerName) != triggers_.end();
}

bool EnhancedTriggerManager::executeTrigger(const std::string& triggerName, const std::string& eventType) {
  if (!triggerExists(triggerName)) {
    return false;
  }

  const auto& info = triggers_[triggerName];
  if (!info.enabled) {
    return true; // 禁用的触发器不执行，但不算错误
  }

  // 检查事件类型匹配
  if (info.eventType != eventType) {
    return true; // 事件类型不匹配，不执行
  }

  // TODO: 执行触发器逻辑
  std::cout << "Executing trigger: " << triggerName << " for event: " << eventType << std::endl;

  return true;
}

bool EnhancedTriggerManager::executeTriggersForTable(const std::string& tableName, const std::string& eventType) {
  auto it = table_triggers_.find(tableName);
  if (it == table_triggers_.end()) {
    return true; // 表没有触发器
  }

  bool success = true;
  for (const auto& triggerName : it->second) {
    if (!executeTrigger(triggerName, eventType)) {
      success = false;
    }
  }

  return success;
}

std::string EnhancedTriggerManager::getTriggerInfo(const std::string& triggerName) const {
  if (!triggerExists(triggerName)) {
    return "Trigger '" + triggerName + "' not found";
  }

  const auto& info = triggers_.at(triggerName);
  std::stringstream ss;
  ss << "Trigger: " << info.name << "\n";
  ss << "Table: " << info.tableName << "\n";
  ss << "Event: " << info.eventType << "\n";
  ss << "Timing: " << info.timing << "\n";
  ss << "Enabled: " << (info.enabled ? "Yes" : "No") << "\n";
  ss << "Created: " << info.created_time << "\n";

  return ss.str();
}

std::vector<std::string> EnhancedTriggerManager::listTriggersForTable(const std::string& tableName) const {
  auto it = table_triggers_.find(tableName);
  if (it == table_triggers_.end()) {
    return {};
  }
  return it->second;
}

std::vector<std::string> EnhancedTriggerManager::listAllTriggers() const {
  std::vector<std::string> triggerNames;
  triggerNames.reserve(triggers_.size());
  for (const auto& pair : triggers_) {
    triggerNames.push_back(pair.first);
  }
  return triggerNames;
}

bool EnhancedTriggerManager::hasTriggerDependencies(const std::string& triggerName) const {
  if (!triggerExists(triggerName)) {
    return false;
  }

  const auto& info = triggers_.at(triggerName);
  return !info.dependencies.empty();
}

std::vector<std::string> EnhancedTriggerManager::getTriggerDependencies(const std::string& triggerName) const {
  if (!triggerExists(triggerName)) {
    return {};
  }

  return triggers_.at(triggerName).dependencies;
}

bool EnhancedTriggerManager::enableTrigger(const std::string& triggerName) {
  if (!triggerExists(triggerName)) {
    return false;
  }

  triggers_[triggerName].enabled = true;
  return true;
}

bool EnhancedTriggerManager::disableTrigger(const std::string& triggerName) {
  if (!triggerExists(triggerName)) {
    return false;
  }

  triggers_[triggerName].enabled = false;
  return true;
}

bool EnhancedTriggerManager::isTriggerEnabled(const std::string& triggerName) const {
  if (!triggerExists(triggerName)) {
    return false;
  }

  return triggers_.at(triggerName).enabled;
}

} // namespace sql_executor
} // namespace sqlcc
