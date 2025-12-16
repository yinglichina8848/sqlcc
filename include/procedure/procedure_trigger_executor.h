#ifndef SQLCC_PROCEDURE_PROCEDURE_TRIGGER_EXECUTOR_H
#define SQLCC_PROCEDURE_PROCEDURE_TRIGGER_EXECUTOR_H

#include "sql_executor.h"
#include "procedure/procedure_parser.h"
#include "procedure/procedure_vm.h"
#include "trigger/trigger_manager.h"
#include "sql_parser/ast_nodes.h"
#include <memory>
#include <unordered_map>
#include <string>

namespace sqlcc {
namespace procedure {

/**
 * @brief 存储过程和触发器执行器
 *
 * 负责执行存储过程和触发器相关的SQL语句，并集成到DML操作中
 */
class ProcedureTriggerExecutor {
public:
    static ProcedureTriggerExecutor& getInstance();

    /**
     * 初始化执行器
     * @param sql_executor SQL执行器实例
     */
    void initialize(SqlExecutor* sql_executor);

    /**
     * 执行CREATE PROCEDURE语句
     * @param stmt CREATE PROCEDURE语句
     * @return 执行结果
     */
    std::string executeCreateProcedure(sql_parser::CreateProcedureStatement* stmt);

    /**
     * 执行CREATE TRIGGER语句
     * @param stmt CREATE TRIGGER语句
     * @return 执行结果
     */
    std::string executeCreateTrigger(sql_parser::CreateTriggerStatement* stmt);

    /**
     * 执行CALL PROCEDURE语句
     * @param stmt CALL PROCEDURE语句
     * @return 执行结果
     */
    std::string executeCallProcedure(sql_parser::CallProcedureStatement* stmt);

    /**
     * 执行DROP PROCEDURE语句
     * @param stmt DROP PROCEDURE语句
     * @return 执行结果
     */
    std::string executeDropProcedure(sql_parser::DropProcedureStatement* stmt);

    /**
     * 执行DROP TRIGGER语句
     * @param stmt DROP TRIGGER语句
     * @return 执行结果
     */
    std::string executeDropTrigger(sql_parser::DropTriggerStatement* stmt);

    /**
     * 执行ALTER TRIGGER语句
     * @param stmt ALTER TRIGGER语句
     * @return 执行结果
     */
    std::string executeAlterTrigger(sql_parser::AlterTriggerStatement* stmt);

    /**
     * 执行INSERT语句（包含触发器）
     * @param stmt INSERT语句
     * @return 执行结果
     */
    std::string executeInsertWithTriggers(sql_parser::InsertStatement* stmt);

    /**
     * 执行UPDATE语句（包含触发器）
     * @param stmt UPDATE语句
     * @return 执行结果
     */
    std::string executeUpdateWithTriggers(sql_parser::UpdateStatement* stmt);

    /**
     * 执行DELETE语句（包含触发器）
     * @param stmt DELETE语句
     * @return 执行结果
     */
    std::string executeDeleteWithTriggers(sql_parser::DeleteStatement* stmt);

    /**
     * 获取最后错误信息
     */
    const std::string& getLastError() const;

    /**
     * 设置触发器执行器
     * @param executor 触发器执行器
     */
    void setTriggerExecutor(std::unique_ptr<trigger::TriggerExecutor> executor);

private:
    ProcedureTriggerExecutor();
    ~ProcedureTriggerExecutor();

    // 禁用拷贝
    ProcedureTriggerExecutor(const ProcedureTriggerExecutor&) = delete;
    ProcedureTriggerExecutor& operator=(const ProcedureTriggerExecutor&) = delete;

    /**
     * 触发DML操作的触发器
     * @param timing 触发时机
     * @param event 触发事件
     * @param table_name 表名
     * @param old_rows 旧行数据
     * @param new_rows 新行数据
     * @return 是否成功
     */
    bool fireDMLEvent(trigger::TriggerTiming timing, trigger::TriggerEvent event,
                     const std::string& table_name,
                     const std::vector<trigger::RowData>& old_rows,
                     const std::vector<trigger::RowData>& new_rows);

    /**
     * 将SQL行数据转换为触发器行数据
     * @param sql_rows SQL行数据
     * @param columns 列名列表
     * @return 触发器行数据
     */
    std::vector<trigger::RowData> convertToTriggerRows(
        const std::vector<std::vector<std::string>>& sql_rows,
        const std::vector<std::string>& columns);

    /**
     * 执行原始SQL语句（不包含触发器）
     * @param sql SQL语句
     * @return 执行结果
     */
    std::string executeRawSQL(const std::string& sql);

    SqlExecutor* sql_executor_;
    ProcedureParser procedure_parser_;
    ProcedureVM procedure_vm_;
    std::unordered_map<std::string, std::unique_ptr<ProcedureDefinition>> stored_procedures_;
    std::string last_error_;

    // 线程安全的存储过程存储
    mutable std::mutex procedures_mutex_;
};

} // namespace procedure
} // namespace sqlcc

#endif // SQLCC_PROCEDURE_PROCEDURE_TRIGGER_EXECUTOR_H
