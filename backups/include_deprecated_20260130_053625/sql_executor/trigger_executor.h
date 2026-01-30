#ifndef SQLCC_SQL_EXECUTOR_TRIGGER_EXECUTOR_H
#define SQLCC_SQL_EXECUTOR_TRIGGER_EXECUTOR_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace sqlcc {
namespace sql_executor {

/**
 * @brief 触发器执行器
 *
 * 负责执行触发器相关的SQL操作，包括：
 * - 触发器的创建、删除和管理
 * - 触发器条件的检查和执行
 * - 触发器执行上下文的管理
 * - 触发器递归调用的防止
 */
class TriggerExecutor {
public:
    /**
     * @brief 构造函数
     */
    TriggerExecutor();

    /**
     * @brief 析构函数
     */
    ~TriggerExecutor();

    /**
     * @brief 创建触发器
     * @param trigger_name 触发器名称
     * @param table_name 关联的表名
     * @param trigger_type 触发器类型 (BEFORE/AFTER, INSERT/UPDATE/DELETE)
     * @param trigger_body 触发器执行体
     * @return 创建是否成功
     */
    bool CreateTrigger(const std::string& trigger_name,
                      const std::string& table_name,
                      const std::string& trigger_type,
                      const std::string& trigger_body);

    /**
     * @brief 删除触发器
     * @param trigger_name 触发器名称
     * @return 删除是否成功
     */
    bool DropTrigger(const std::string& trigger_name);

    /**
     * @brief 执行触发器
     * @param trigger_name 触发器名称
     * @param event_data 事件数据
     * @return 执行是否成功
     */
    bool ExecuteTrigger(const std::string& trigger_name,
                       const std::unordered_map<std::string, std::string>& event_data);

    /**
     * @brief 检查触发器是否存在
     * @param trigger_name 触发器名称
     * @return 是否存在
     */
    bool TriggerExists(const std::string& trigger_name);

    /**
     * @brief 获取触发器列表
     * @param table_name 表名（可选，用于筛选）
     * @return 触发器名称列表
     */
    std::vector<std::string> ListTriggers(const std::string& table_name = "");

    /**
     * @brief 启用触发器
     * @param trigger_name 触发器名称
     * @return 操作是否成功
     */
    bool EnableTrigger(const std::string& trigger_name);

    /**
     * @brief 禁用触发器
     * @param trigger_name 触发器名称
     * @return 操作是否成功
     */
    bool DisableTrigger(const std::string& trigger_name);

    /**
     * @brief 获取触发器定义
     * @param trigger_name 触发器名称
     * @return 触发器定义字符串
     */
    std::string GetTriggerDefinition(const std::string& trigger_name);

    /**
     * @brief 验证触发器语法
     * @param trigger_body 触发器体
     * @return 是否语法正确
     */
    bool ValidateTriggerSyntax(const std::string& trigger_body);

    /**
     * @brief 处理表事件（INSERT/UPDATE/DELETE）
     * @param table_name 表名
     * @param event_type 事件类型
     * @param event_data 事件数据
     * @return 处理是否成功
     */
    bool HandleTableEvent(const std::string& table_name,
                         const std::string& event_type,
                         const std::unordered_map<std::string, std::string>& event_data);

    /**
     * @brief 检查触发器递归
     * @param trigger_name 当前触发器名称
     * @return 是否存在递归调用
     */
    bool CheckTriggerRecursion(const std::string& trigger_name);

    /**
     * @brief 获取触发器执行统计
     * @param trigger_name 触发器名称
     * @return 统计信息
     */
    std::unordered_map<std::string, int> GetTriggerStatistics(const std::string& trigger_name);

private:
    // 私有辅助方法
    bool IsValidTriggerType(const std::string& trigger_type);
    bool IsValidTriggerName(const std::string& trigger_name);
    std::string GenerateTriggerId(const std::string& trigger_name);
    void LogTriggerExecution(const std::string& trigger_name, bool success);
    bool ExecuteTriggerBody(const std::string& trigger_body,
                           const std::unordered_map<std::string, std::string>& context);
};

} // namespace sql_executor
} // namespace sqlcc

#endif // SQLCC_SQL_EXECUTOR_TRIGGER_EXECUTOR_H