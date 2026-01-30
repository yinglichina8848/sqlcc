#ifndef SQLCC_TRIGGER_SQL_TRIGGER_EXECUTOR_H
#define SQLCC_TRIGGER_SQL_TRIGGER_EXECUTOR_H

#include "src/sql_parser/trigger_manager.h"
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

class SqlExecutor;

namespace trigger {

/**
 * @brief SQL触发器执行器
 *
 * 实现具体的触发器SQL执行逻辑，支持：
 * - :OLD 和 :NEW 变量引用
 * - 条件判断执行
 * - SQL语句执行
 * - 错误处理和回滚
 */
class SQLTriggerExecutor : public TriggerExecutor {
public:
    SQLTriggerExecutor();
    ~SQLTriggerExecutor() override;

    /**
     * 执行触发器
     * @param trigger 触发器定义
     * @param old_row 旧行数据 (UPDATE/DELETE时有效)
     * @param new_row 新行数据 (INSERT/UPDATE时有效)
     * @return 执行结果
     */
    bool executeTrigger(const TriggerDefinition* trigger,
                       const RowData* old_row,
                       const RowData* new_row) override;

    /**
     * 检查触发条件
     * @param condition 条件表达式
     * @param old_row 旧行数据
     * @param new_row 新行数据
     * @return 条件是否满足
     */
    bool evaluateCondition(const std::string& condition,
                          const RowData* old_row,
                          const RowData* new_row) override;

    /**
     * 设置SQL执行器
     * @param executor SQL执行器
     */
    void setSqlExecutor(std::shared_ptr<SqlExecutor> executor);

    /**
     * 获取最后错误信息
     */
    const std::string& getLastError() const;

private:
    /**
     * 执行触发器SQL体
     * @param trigger_sql 触发器SQL代码
     * @param old_row 旧行数据
     * @param new_row 新行数据
     * @return 执行结果
     */
    bool executeTriggerSQL(const std::string& trigger_sql,
                          const RowData* old_row,
                          const RowData* new_row);

    /**
     * 替换触发器变量引用
     * @param sql 原始SQL
     * @param old_row 旧行数据
     * @param new_row 新行数据
     * @return 替换后的SQL
     */
    std::string substituteTriggerVariables(const std::string& sql,
                                         const RowData* old_row,
                                         const RowData* new_row);

    /**
     * 解析和执行SQL语句列表
     * @param sql_list 分号分隔的SQL语句列表
     * @return 执行结果
     */
    bool executeSQLStatements(const std::string& sql_list);

    /**
     * 执行单个SQL语句
     * @param sql SQL语句
     * @return 执行结果
     */
    bool executeSingleSQL(const std::string& sql);

    /**
     * 获取列在行数据中的索引
     * @param columns 列名列表
     * @param column_name 列名
     * @return 列索引，如果不存在返回-1
     */
    int getColumnIndex(const std::vector<std::string>& columns,
                      const std::string& column_name) const;

    /**
     * 格式化行数据用于变量替换
     * @param row 行数据
     * @return 格式化的值字符串
     */
    std::string formatRowValue(const RowData* row) const;

    /**
     * 转义SQL字符串值
     * @param value 原始值
     * @return 转义后的SQL字符串
     */
    std::string escapeSQLString(const std::string& value) const;

    /**
     * 分割SQL语句
     * @param sql_list SQL语句列表
     * @return 分割后的SQL语句向量
     */
    std::vector<std::string> splitSQLStatements(const std::string& sql_list) const;

    std::shared_ptr<SqlExecutor> sql_executor_;
    std::string last_error_;
    static const std::string OLD_PREFIX;
    static const std::string NEW_PREFIX;
};

} // namespace trigger
} // namespace sqlcc

#endif // SQLCC_TRIGGER_SQL_TRIGGER_EXECUTOR_H
