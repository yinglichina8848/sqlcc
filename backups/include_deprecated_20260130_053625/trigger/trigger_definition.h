/**
 * @file trigger_definition.h
 * @brief 触发器定义类定义
 *
 * Why: 需要专门的类来定义和存储触发器元数据
 * What: TriggerDefinition类封装触发器的基本属性和配置
 * How: 提供触发器属性的存储和管理功能
 */

#pragma once

#include <string>

namespace sqlcc {
namespace trigger {

/**
 * @brief 触发器事件类型
 */
enum class TriggerEvent {
    INSERT,
    UPDATE,
    DELETE
};

/**
 * @brief 触发器时机
 */
enum class TriggerTiming {
    BEFORE,
    AFTER
};

/**
 * @brief 触发器级别
 */
enum class TriggerLevel {
    ROW,
    STATEMENT
};

/**
 * @brief 触发器状态
 */
enum class TriggerStatus {
    ENABLED,
    DISABLED
};

/**
 * @brief 触发器定义
 *
 * 存储触发器的元数据信息，包括名称、事件、时机、级别等
 */
class TriggerDefinition {
public:
    /**
     * @brief 构造函数
     * @param name 触发器名称
     * @param timing 触发时机
     * @param event 触发事件
     * @param level 触发级别
     * @param table_name 关联表名
     */
    TriggerDefinition(const std::string& name, TriggerTiming timing,
                     TriggerEvent event, TriggerLevel level,
                     const std::string& table_name);

    /**
     * @brief 析构函数
     */
    ~TriggerDefinition();

    // Getters
    const std::string& getName() const { return name_; }
    TriggerTiming getTiming() const { return timing_; }
    TriggerEvent getEvent() const { return event_; }
    TriggerLevel getLevel() const { return level_; }
    const std::string& getTableName() const { return table_name_; }
    TriggerStatus getStatus() const { return status_; }
    const std::string& getCondition() const { return condition_; }
    const std::string& getBody() const { return body_; }
    const std::string& getDefiner() const { return definer_; }

    // Setters
    void setStatus(TriggerStatus status) { status_ = status; }
    void setCondition(const std::string& condition) { condition_ = condition; }
    void setBody(const std::string& body) { body_ = body; }
    void setDefiner(const std::string& definer) { definer_ = definer; }

    // 字符串转换
    static std::string timingToString(TriggerTiming timing);
    static std::string eventToString(TriggerEvent event);
    static std::string levelToString(TriggerLevel level);
    static std::string statusToString(TriggerStatus status);

private:
    std::string name_;           ///< 触发器名称
    TriggerTiming timing_;       ///< 触发时机
    TriggerEvent event_;         ///< 触发事件
    TriggerLevel level_;         ///< 触发级别
    std::string table_name_;     ///< 关联表名
    TriggerStatus status_;       ///< 触发器状态
    std::string condition_;      ///< 触发条件
    std::string body_;           ///< 触发器主体
    std::string definer_;        ///< 定义者
};

} // namespace trigger
} // namespace sqlcc
