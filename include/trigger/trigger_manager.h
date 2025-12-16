#ifndef SQLCC_TRIGGER_TRIGGER_MANAGER_H
#define SQLCC_TRIGGER_TRIGGER_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <functional>

namespace sqlcc {

class SqlExecutor;

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
 * @brief 行数据表示
 */
struct RowData {
    std::vector<std::string> columns;
    std::vector<std::string> values;

    RowData() = default;
    RowData(const std::vector<std::string>& cols, const std::vector<std::string>& vals)
        : columns(cols), values(vals) {}
};

/**
 * @brief 触发器定义
 */
class TriggerDefinition {
public:
    TriggerDefinition(const std::string& name, TriggerTiming timing,
                     TriggerEvent event, TriggerLevel level,
                     const std::string& table_name);

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
    std::string name_;
    TriggerTiming timing_;
    TriggerEvent event_;
    TriggerLevel level_;
    std::string table_name_;
    TriggerStatus status_;
    std::string condition_;
    std::string body_;
    std::string definer_;
};

/**
 * @brief 触发器执行器接口
 */
class TriggerExecutor {
public:
    virtual ~TriggerExecutor() = default;

    /**
     * 执行触发器
     * @param trigger 触发器定义
     * @param old_row 旧行数据 (UPDATE/DELETE时有效)
     * @param new_row 新行数据 (INSERT/UPDATE时有效)
     * @return 执行结果
     */
    virtual bool executeTrigger(const TriggerDefinition* trigger,
                               const RowData* old_row,
                               const RowData* new_row) = 0;

    /**
     * 检查触发条件
     * @param condition 条件表达式
     * @param old_row 旧行数据
     * @param new_row 新行数据
     * @return 条件是否满足
     */
    virtual bool evaluateCondition(const std::string& condition,
                                  const RowData* old_row,
                                  const RowData* new_row) = 0;
};

/**
 * @brief 递归防护机制
 */
class RecursionGuard {
public:
    RecursionGuard();
    ~RecursionGuard();

    /**
     * 进入触发器执行上下文
     * @param trigger_name 触发器名称
     * @return 如果允许执行返回true，否则返回false (防止递归)
     */
    bool enterTrigger(const std::string& trigger_name);

    /**
     * 离开触发器执行上下文
     * @param trigger_name 触发器名称
     */
    void exitTrigger(const std::string& trigger_name);

    /**
     * 检查是否在递归调用中
     * @param trigger_name 触发器名称
     * @return 如果在递归中返回true
     */
    bool isRecursive(const std::string& trigger_name) const;

    /**
     * 获取当前调用深度
     */
    size_t getDepth() const;

    /**
     * 重置防护状态
     */
    void reset();

private:
    std::vector<std::string> call_stack_;
    std::unordered_map<std::string, int> trigger_depth_;
    static const size_t MAX_RECURSION_DEPTH = 10;
};

/**
 * @brief 触发器管理器
 *
 * 负责触发器的注册、注销、触发和递归防护
 */
class TriggerManager {
public:
    static TriggerManager& getInstance();

    /**
     * 初始化管理器
     * @param executor SQL执行器
     */
    void initialize(SqlExecutor* executor);

    /**
     * 创建触发器
     * @param trigger 触发器定义
     * @return 是否成功
     */
    bool createTrigger(std::unique_ptr<TriggerDefinition> trigger);

    /**
     * 删除触发器
     * @param trigger_name 触发器名称
     * @return 是否成功
     */
    bool dropTrigger(const std::string& trigger_name);

    /**
     * 启用触发器
     * @param trigger_name 触发器名称
     * @return 是否成功
     */
    bool enableTrigger(const std::string& trigger_name);

    /**
     * 禁用触发器
     * @param trigger_name 触发器名称
     * @return 是否成功
     */
    bool disableTrigger(const std::string& trigger_name);

    /**
     * 获取触发器
     * @param trigger_name 触发器名称
     * @return 触发器定义指针，如果不存在返回nullptr
     */
    const TriggerDefinition* getTrigger(const std::string& trigger_name) const;

    /**
     * 获取表的所有触发器
     * @param table_name 表名
     * @return 触发器列表
     */
    std::vector<const TriggerDefinition*> getTriggersForTable(const std::string& table_name) const;

    /**
     * 触发事件处理
     * @param timing 触发时机
     * @param event 触发事件
     * @param table_name 表名
     * @param old_rows 旧行数据
     * @param new_rows 新行数据
     * @return 是否成功
     */
    bool fireTriggers(TriggerTiming timing, TriggerEvent event,
                     const std::string& table_name,
                     const std::vector<RowData>& old_rows,
                     const std::vector<RowData>& new_rows);

    /**
     * 获取最后错误信息
     */
    const std::string& getLastError() const;

    /**
     * 设置触发器执行器
     * @param executor 触发器执行器
     */
    void setTriggerExecutor(std::unique_ptr<TriggerExecutor> executor);

private:
    TriggerManager();
    ~TriggerManager();

    // 禁用拷贝
    TriggerManager(const TriggerManager&) = delete;
    TriggerManager& operator=(const TriggerManager&) = delete;

    /**
     * 执行单个触发器
     * @param trigger 触发器定义
     * @param old_row 旧行数据
     * @param new_row 新行数据
     * @return 执行结果
     */
    bool executeTrigger(const TriggerDefinition* trigger,
                       const RowData* old_row, const RowData* new_row);

    /**
     * 检查触发条件
     * @param trigger 触发器
     * @param old_row 旧行数据
     * @param new_row 新行数据
     * @return 是否满足条件
     */
    bool checkTriggerCondition(const TriggerDefinition* trigger,
                              const RowData* old_row, const RowData* new_row);

    /**
     * 执行触发器SQL（默认实现）
     * @param trigger 触发器定义
     * @param old_row 旧行数据
     * @param new_row 新行数据
     * @return 执行结果
     */
    bool executeTriggerSql(const TriggerDefinition* trigger,
                          const RowData* old_row, const RowData* new_row);

    SqlExecutor* sql_executor_;
    std::unordered_map<std::string, std::unique_ptr<TriggerDefinition>> triggers_;
    std::unique_ptr<TriggerExecutor> trigger_executor_;
    RecursionGuard recursion_guard_;
    mutable std::mutex mutex_;
    std::string last_error_;
};

} // namespace trigger
} // namespace sqlcc

#endif // SQLCC_TRIGGER_TRIGGER_MANAGER_H
