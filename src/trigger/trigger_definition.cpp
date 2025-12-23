#include "trigger/trigger_definition.h"

namespace sqlcc {
namespace trigger {

TriggerDefinition::TriggerDefinition(const std::string& name,
                                   TriggerTiming timing,
                                   TriggerEvent event,
                                   TriggerLevel level,
                                   const std::string& table_name)
    : name_(name),
      timing_(timing),
      event_(event),
      level_(level),
      table_name_(table_name),
      status_(TriggerStatus::ENABLED),
      condition_(""),
      body_(""),
      definer_("") {
}

TriggerDefinition::~TriggerDefinition() {
    // 清理工作（如果需要）
}

// 静态方法实现
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

} // namespace trigger
} // namespace sqlcc
