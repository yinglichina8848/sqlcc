#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

namespace sqlcc {
namespace sql_executor {

// Forward declarations
namespace sql_parser {
class CreateTriggerStatement;
class AlterTriggerStatement;
class DropTriggerStatement;
} // namespace sql_parser

/**
 * Enhanced Trigger Manager - 增强触发器管理器
 * 处理CREATE/ALTER/DROP TRIGGER语句的高级功能
 */
class EnhancedTriggerManager {
public:
    static EnhancedTriggerManager& getInstance();

    // 触发器管理
    bool createTrigger(const sql_parser::CreateTriggerStatement& stmt);
    bool alterTrigger(const sql_parser::AlterTriggerStatement& stmt);
    bool dropTrigger(const sql_parser::DropTriggerStatement& stmt);
    bool triggerExists(const std::string& triggerName) const;

    // 触发器执行
    bool executeTrigger(const std::string& triggerName, const std::string& eventType);
    bool executeTriggersForTable(const std::string& tableName, const std::string& eventType);

    // 触发器信息查询
    std::string getTriggerInfo(const std::string& triggerName) const;
    std::vector<std::string> listTriggersForTable(const std::string& tableName) const;
    std::vector<std::string> listAllTriggers() const;

    // 触发器依赖管理
    bool hasTriggerDependencies(const std::string& triggerName) const;
    std::vector<std::string> getTriggerDependencies(const std::string& triggerName) const;

    // 触发器状态管理
    bool enableTrigger(const std::string& triggerName);
    bool disableTrigger(const std::string& triggerName);
    bool isTriggerEnabled(const std::string& triggerName) const;

private:
    EnhancedTriggerManager() = default;

    struct TriggerInfo {
        std::string name;
        std::string tableName;
        std::string eventType; // INSERT, UPDATE, DELETE
        std::string timing;    // BEFORE, AFTER, INSTEAD OF
        std::string body;
        std::string whenCondition;
        bool enabled = true;
        long created_time;
        std::string created_by;
        std::vector<std::string> dependencies;
    };

    std::unordered_map<std::string, TriggerInfo> triggers_;
    std::unordered_map<std::string, std::vector<std::string>> table_triggers_;
    long next_trigger_id_ = 1;
};

} // namespace sql_executor
} // namespace sqlcc
