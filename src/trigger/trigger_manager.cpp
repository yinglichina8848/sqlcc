#include "trigger/trigger_manager.h"
#include <algorithm>
#include <sstream>

namespace sqlcc {
namespace trigger {

// ==================== TriggerDefinition Implementation ====================

TriggerDefinition::TriggerDefinition(const std::string& name, TriggerTiming timing,
                                   TriggerEvent event, TriggerLevel level,
                                   const std::string& table_name)
    : name_(name), timing_(timing), event_(event), level_(level),
      table_name_(table_name), status_(TriggerStatus::ENABLED) {}

TriggerDefinition::~TriggerDefinition() {}

std::string TriggerDefinition::timingToString(TriggerTiming timing) {
    switch (timing) {
        case TriggerTiming::BEFORE: return "BEFORE";
        case TriggerTiming::AFTER: return "AFTER";
        default: return "UNKNOWN";
    }
}

std::string TriggerDefinition::eventToString(TriggerEvent event) {
    switch (event) {
        case TriggerEvent::INSERT: return "INSERT";
        case TriggerEvent::UPDATE: return "UPDATE";
        case TriggerEvent::DELETE: return "DELETE";
        default: return "UNKNOWN";
    }
}

std::string TriggerDefinition::levelToString(TriggerLevel level) {
    switch (level) {
        case TriggerLevel::ROW: return "ROW";
        case TriggerLevel::STATEMENT: return "STATEMENT";
        default: return "UNKNOWN";
    }
}

std::string TriggerDefinition::statusToString(TriggerStatus status) {
    switch (status) {
        case TriggerStatus::ENABLED: return "ENABLED";
        case TriggerStatus::DISABLED: return "DISABLED";
        default: return "UNKNOWN";
    }
}

// ==================== RecursionGuard Implementation ====================

RecursionGuard::RecursionGuard() {}

RecursionGuard::~RecursionGuard() {}

bool RecursionGuard::enterTrigger(const std::string& trigger_name) {
    // 检查递归深度
    auto it = trigger_depth_.find(trigger_name);
    if (it != trigger_depth_.end()) {
        if (static_cast<size_t>(it->second) >= MAX_RECURSION_DEPTH) {
            return false; // 超过最大递归深度
        }
        it->second++;
    } else {
        trigger_depth_[trigger_name] = 1;
    }

    call_stack_.push_back(trigger_name);
    return true;
}

void RecursionGuard::exitTrigger(const std::string& trigger_name) {
    if (!call_stack_.empty() && call_stack_.back() == trigger_name) {
        call_stack_.pop_back();
    }

    auto it = trigger_depth_.find(trigger_name);
    if (it != trigger_depth_.end()) {
        it->second--;
        if (it->second <= 0) {
            trigger_depth_.erase(it);
        }
    }
}

bool RecursionGuard::isRecursive(const std::string& trigger_name) const {
    auto it = trigger_depth_.find(trigger_name);
    return it != trigger_depth_.end() && it->second > 0;
}

size_t RecursionGuard::getDepth() const {
    return call_stack_.size();
}

void RecursionGuard::reset() {
    call_stack_.clear();
    trigger_depth_.clear();
}

// ==================== TriggerManager Implementation ====================

TriggerManager& TriggerManager::getInstance() {
    static TriggerManager instance;
    return instance;
}

TriggerManager::TriggerManager()
    : sql_executor_(nullptr), last_error_("") {}

TriggerManager::~TriggerManager() {}

void TriggerManager::initialize(std::shared_ptr<SqlExecutor> executor) {
    std::lock_guard<std::mutex> lock(mutex_);
    sql_executor_ = std::move(executor);
}

bool TriggerManager::createTrigger(std::unique_ptr<TriggerDefinition> trigger) {
    if (!trigger) {
        last_error_ = "Null trigger definition";
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    const std::string& name = trigger->getName();
    if (triggers_.find(name) != triggers_.end()) {
        last_error_ = "Trigger '" + name + "' already exists";
        return false;
    }

    triggers_[name] = std::move(trigger);
    return true;
}

bool TriggerManager::dropTrigger(const std::string& trigger_name) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = triggers_.find(trigger_name);
    if (it == triggers_.end()) {
        last_error_ = "Trigger '" + trigger_name + "' does not exist";
        return false;
    }

    triggers_.erase(it);
    return true;
}

bool TriggerManager::enableTrigger(const std::string& trigger_name) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = triggers_.find(trigger_name);
    if (it == triggers_.end()) {
        last_error_ = "Trigger '" + trigger_name + "' does not exist";
        return false;
    }

    it->second->setStatus(TriggerStatus::ENABLED);
    return true;
}

bool TriggerManager::disableTrigger(const std::string& trigger_name) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = triggers_.find(trigger_name);
    if (it == triggers_.end()) {
        last_error_ = "Trigger '" + trigger_name + "' does not exist";
        return false;
    }

    it->second->setStatus(TriggerStatus::DISABLED);
    return true;
}

std::shared_ptr<const TriggerDefinition> TriggerManager::getTrigger(const std::string& trigger_name) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = triggers_.find(trigger_name);
    if (it != triggers_.end()) {
        return it->second;  // 智能指针会自动转换
    }
    return nullptr;
}

std::vector<std::shared_ptr<const TriggerDefinition>> TriggerManager::getTriggersForTable(const std::string& table_name) const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::shared_ptr<const TriggerDefinition>> result;
    for (const auto& pair : triggers_) {
        if (pair.second->getTableName() == table_name &&
            pair.second->getStatus() == TriggerStatus::ENABLED) {
            result.push_back(pair.second);
        }
    }
    return result;
}

bool TriggerManager::fireTriggers(TriggerTiming timing, TriggerEvent event,
                                 const std::string& table_name,
                                 const std::vector<RowData>& old_rows,
                                 const std::vector<RowData>& new_rows) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 获取匹配的触发器
    std::vector<std::shared_ptr<const TriggerDefinition>> matching_triggers;
    for (const auto& pair : triggers_) {
        std::shared_ptr<const TriggerDefinition> trigger = pair.second;
        if (trigger->getTiming() == timing &&
            trigger->getEvent() == event &&
            trigger->getTableName() == table_name &&
            trigger->getStatus() == TriggerStatus::ENABLED) {
            matching_triggers.push_back(trigger);
        }
    }

    if (matching_triggers.empty()) {
        return true; // 没有匹配的触发器，直接返回成功
    }

    // 执行触发器
    bool success = true;
    for (std::shared_ptr<const TriggerDefinition> trigger : matching_triggers) {
        if (trigger->getLevel() == TriggerLevel::ROW) {
            // 行级触发器 - 为每一行执行
            size_t max_rows = std::max(old_rows.size(), new_rows.size());
            for (size_t i = 0; i < max_rows; ++i) {
                std::shared_ptr<const RowData> old_row = (i < old_rows.size()) ?
                    std::make_shared<RowData>(old_rows[i]) : nullptr;
                std::shared_ptr<const RowData> new_row = (i < new_rows.size()) ?
                    std::make_shared<RowData>(new_rows[i]) : nullptr;

                if (!executeTrigger(trigger, old_row, new_row)) {
                    success = false;
                    // 决定是否继续执行其他触发器
                    // 这里暂时继续执行，但记录错误
                }
            }
        } else {
            // 语句级触发器 - 执行一次
            if (!executeTrigger(trigger, nullptr, nullptr)) {
                success = false;
            }
        }
    }

    return success;
}

const std::string& TriggerManager::getLastError() const {
    return last_error_;
}

void TriggerManager::setTriggerExecutor(std::unique_ptr<TriggerExecutor> executor) {
    std::lock_guard<std::mutex> lock(mutex_);
    trigger_executor_ = std::move(executor);
}

bool TriggerManager::executeTrigger(std::shared_ptr<const TriggerDefinition> trigger,
                                   std::shared_ptr<const RowData> old_row, std::shared_ptr<const RowData> new_row) {
    if (!trigger) {
        last_error_ = "Null trigger";
        return false;
    }

    // 检查递归防护
    if (!recursion_guard_.enterTrigger(trigger->getName())) {
        last_error_ = "Trigger recursion detected for '" + trigger->getName() + "'";
        return false;
    }

    bool success = false;

    try {
        // 检查触发条件
        if (!checkTriggerCondition(trigger, old_row, new_row)) {
            success = true; // 条件不满足，认为是成功的（不执行）
        } else if (trigger_executor_) {
            // 执行触发器
            success = trigger_executor_->executeTrigger(trigger.get(), old_row.get(), new_row.get());
            if (!success) {
                last_error_ = "Trigger execution failed for '" + trigger->getName() + "'";
            }
        } else {
            // 没有执行器，默认执行SQL语句
            success = executeTriggerSql(trigger, old_row, new_row);
        }
    } catch (const std::exception& e) {
        last_error_ = std::string("Trigger execution exception: ") + e.what();
        success = false;
    }

    recursion_guard_.exitTrigger(trigger->getName());
    return success;
}

bool TriggerManager::checkTriggerCondition(std::shared_ptr<const TriggerDefinition> trigger,
                                          std::shared_ptr<const RowData> old_row, std::shared_ptr<const RowData> new_row) {
    if (!trigger) {
        return false;
    }

    const std::string& condition = trigger->getCondition();
    if (condition.empty()) {
        return true; // 没有条件，始终执行
    }

    if (!trigger_executor_) {
        // 没有执行器，简单检查条件是否为TRUE
        std::string upper_condition = condition;
        std::transform(upper_condition.begin(), upper_condition.end(),
                      upper_condition.begin(), ::toupper);
        return upper_condition == "TRUE" || upper_condition == "1";
    }

    return trigger_executor_->evaluateCondition(condition, old_row.get(), new_row.get());
}

bool TriggerManager::executeTriggerSql(std::shared_ptr<const TriggerDefinition> trigger,
                                      std::shared_ptr<const RowData> old_row, std::shared_ptr<const RowData> new_row) {
    if (!trigger || !sql_executor_) {
        return false;
    }

    const std::string& body = trigger->getBody();
    if (body.empty()) {
        return true;
    }

    try {
        // 这里应该解析并执行触发器体中的SQL语句
        // 暂时简单处理，直接执行SQL
        // TODO: 实现完整的SQL解析和执行
        return true;
    } catch (const std::exception& e) {
        last_error_ = std::string("Trigger SQL execution error: ") + e.what();
        return false;
    }
}

} // namespace trigger
} // namespace sqlcc
