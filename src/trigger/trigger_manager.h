/**
 * @file trigger_manager.h
 * @brief 触发器管理器类定义
 *
 * Why: 需要一个统一的触发器管理接口
 * What: TriggerManager类提供触发器的注册、注销和执行管理
 * How: 使用单例模式管理所有触发器，提供线程安全操作
 */

#pragma once

#include "src/trigger/trigger_definition.h"
#include "src/trigger/trigger_executor.h"
#include "src/trigger/recursion_guard.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace sqlcc {

class SqlExecutor;

namespace trigger {

/**
 * @brief 触发器管理器
 *
 * 负责触发器的注册、注销、触发和递归防护
 */
class TriggerManager {
public:
    /**
     * @brief 获取单例实例
     */
    static TriggerManager& getInstance();

    /**
     * @brief 初始化管理器
     * @param executor SQL执行器
     */
    void initialize(std::shared_ptr<SqlExecutor> executor);

    /**
     * @brief 创建触发器
     * @param trigger 触发器定义
     * @return 是否成功
     */
    bool createTrigger(std::unique_ptr<TriggerDefinition> trigger);

    /**
     * @brief 删除触发器
     * @param trigger_name 触发器名称
     * @return 是否成功
     */
    bool dropTrigger(const std::string& trigger_name);

    /**
     * @brief 启用触发器
     * @param trigger_name 触发器名称
     * @return 是否成功
     */
    bool enableTrigger(const std::string& trigger_name);

    /**
     * @brief 禁用触发器
     * @param trigger_name 触发器名称
     * @return 是否成功
     */
    bool disableTrigger(const std::string& trigger_name);

    /**
     * @brief 获取触发器
     * @param trigger_name 触发器名称
     * @return 触发器定义的智能指针，如果不存在返回nullptr
     */
    std::shared_ptr<const TriggerDefinition> getTrigger(const std::string& trigger_name) const;

    /**
     * @brief 获取表的所有触发器
     * @param table_name 表名
     * @return 触发器智能指针列表
     */
    std::vector<std::shared_ptr<const TriggerDefinition>> getTriggersForTable(const std::string& table_name) const;

    /**
     * @brief 触发事件处理
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
     * @brief 获取最后错误信息
     */
    const std::string& getLastError() const;

    /**
     * @brief 设置触发器执行器
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
     * @brief 执行单个触发器
     * @param trigger 触发器定义
     * @param old_row 旧行数据
     * @param new_row 新行数据
     * @return 执行结果
     */
    bool executeTrigger(std::shared_ptr<const TriggerDefinition> trigger,
                       std::shared_ptr<const RowData> old_row, std::shared_ptr<const RowData> new_row);

    /**
     * @brief 检查触发条件
     * @param trigger 触发器
     * @param old_row 旧行数据
     * @param new_row 新行数据
     * @return 是否满足条件
     */
    bool checkTriggerCondition(std::shared_ptr<const TriggerDefinition> trigger,
                              std::shared_ptr<const RowData> old_row, std::shared_ptr<const RowData> new_row);

    /**
     * @brief 执行触发器SQL（默认实现）
     * @param trigger 触发器定义
     * @param old_row 旧行数据
     * @param new_row 新行数据
     * @return 执行结果
     */
    bool executeTriggerSql(std::shared_ptr<const TriggerDefinition> trigger,
                          std::shared_ptr<const RowData> old_row, std::shared_ptr<const RowData> new_row);

    std::shared_ptr<SqlExecutor> sql_executor_;
    std::unordered_map<std::string, std::shared_ptr<TriggerDefinition>> triggers_;
    std::unique_ptr<TriggerExecutor> trigger_executor_;
    RecursionGuard recursion_guard_;
    mutable std::mutex mutex_;
    std::string last_error_;
};

} // namespace trigger
} // namespace sqlcc
