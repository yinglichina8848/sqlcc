#ifndef SQLCC_SQL_PARSER_TRIGGER_PARSER_H
#define SQLCC_SQL_PARSER_TRIGGER_PARSER_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace sqlcc {
namespace sql_parser {

/**
 * @brief 触发器解析器
 *
 * 负责解析触发器相关的SQL语句，包括：
 * - CREATE TRIGGER语句解析
 * - DROP TRIGGER语句解析
 * - ALTER TRIGGER语句解析
 * - 触发器条件和动作解析
 */
class TriggerParser {
public:
    /**
     * @brief 构造函数
     */
    TriggerParser();

    /**
     * @brief 析构函数
     */
    ~TriggerParser();

    /**
     * @brief 解析CREATE TRIGGER语句
     * @param sql CREATE TRIGGER语句
     * @return 解析是否成功
     */
    bool ParseCreateTrigger(const std::string& sql);

    /**
     * @brief 解析DROP TRIGGER语句
     * @param sql DROP TRIGGER语句
     * @return 解析是否成功
     */
    bool ParseDropTrigger(const std::string& sql);

    /**
     * @brief 解析ALTER TRIGGER语句
     * @param sql ALTER TRIGGER语句
     * @return 解析是否成功
     */
    bool ParseAlterTrigger(const std::string& sql);

    /**
     * @brief 解析触发器名称
     * @param sql 包含触发器名称的SQL语句
     * @return 触发器名称
     */
    std::string ParseTriggerName(const std::string& sql);

    /**
     * @brief 解析触发器类型（BEFORE/AFTER）
     * @param sql 触发器定义语句
     * @return 触发器类型
     */
    std::string ParseTriggerType(const std::string& sql);

    /**
     * @brief 解析触发器事件（INSERT/UPDATE/DELETE）
     * @param sql 触发器定义语句
     * @return 触发器事件
     */
    std::string ParseTriggerEvent(const std::string& sql);

    /**
     * @brief 解析触发器表名
     * @param sql 触发器定义语句
     * @return 表名
     */
    std::string ParseTriggerTable(const std::string& sql);

    /**
     * @brief 解析触发器条件
     * @param sql 触发器定义语句
     * @return 触发器条件
     */
    std::string ParseTriggerCondition(const std::string& sql);

    /**
     * @brief 解析触发器动作
     * @param sql 触发器定义语句
     * @return 触发器动作
     */
    std::string ParseTriggerAction(const std::string& sql);

    /**
     * @brief 验证触发器语法
     * @param sql 触发器定义语句
     * @return 是否语法正确
     */
    bool ValidateTriggerSyntax(const std::string& sql);

    /**
     * @brief 获取解析后的触发器信息
     * @return 触发器信息映射
     */
    std::unordered_map<std::string, std::string> GetTriggerInfo() const;

    /**
     * @brief 获取解析错误信息
     * @return 错误信息
     */
    std::string GetErrorMessage() const;

    /**
     * @brief 检查是否有解析错误
     * @return 是否有错误
     */
    bool HasError() const;

    /**
     * @brief 重置解析器状态
     */
    void Reset();

    /**
     * @brief 解析FOR EACH ROW子句
     * @param sql 触发器定义语句
     * @return 是否为行级触发器
     */
    bool ParseForEachRow(const std::string& sql);

    /**
     * @brief 解析FOR EACH STATEMENT子句
     * @param sql 触发器定义语句
     * @return 是否为语句级触发器
     */
    bool ParseForEachStatement(const std::string& sql);

    /**
     * @brief 解析WHEN条件子句
     * @param sql 触发器定义语句
     * @return WHEN条件表达式
     */
    std::string ParseWhenCondition(const std::string& sql);

    /**
     * @brief 解析触发器函数调用
     * @param sql 触发器动作语句
     * @return 函数调用信息
     */
    std::unordered_map<std::string, std::string> ParseTriggerFunction(const std::string& sql);

private:
    std::unordered_map<std::string, std::string> trigger_info_;
    std::string error_message_;
    bool has_error_;

    // 私有辅助方法
    bool IsValidTriggerName(const std::string& name);
    bool IsValidTableName(const std::string& name);
    bool IsValidTriggerType(const std::string& type);
    bool IsValidTriggerEvent(const std::string& event);
    std::string TrimQuotes(const std::string& str);
    std::string ExtractSubstring(const std::string& sql, const std::string& start_keyword, const std::string& end_keyword = "");
    std::vector<std::string> SplitByKeywords(const std::string& sql, const std::vector<std::string>& keywords);
    void SetError(const std::string& message);
    bool ParseTriggerDefinition(const std::string& sql);
    bool ParseTriggerBody(const std::string& sql);
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_TRIGGER_PARSER_H