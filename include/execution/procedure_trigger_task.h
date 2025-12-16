#ifndef SQLCC_PROCEDURE_TRIGGER_TASK_H
#define SQLCC_PROCEDURE_TRIGGER_TASK_H

#include "execution/task_executor.h"
#include "procedure/procedure_trigger_executor.h"
#include "sql_parser/ast_nodes.h"
#include <memory>
#include <string>

namespace sqlcc {
namespace execution {

/**
 * @brief 存储过程任务类
 *
 * 处理存储过程调用任务
 */
class ProcedureCallTask : public Task {
public:
    ProcedureCallTask(const std::string& task_id, sql_parser::CallProcedureStatement* stmt);
    virtual ~ProcedureCallTask() = default;

    std::shared_ptr<TaskResult> execute() override;

private:
    sql_parser::CallProcedureStatement* stmt_;
};

/**
 * @brief 触发器任务类
 *
 * 处理触发器执行任务
 */
class TriggerExecuteTask : public Task {
public:
    TriggerExecuteTask(const std::string& task_id,
                      trigger::TriggerTiming timing,
                      trigger::TriggerEvent event,
                      const std::string& table_name);
    virtual ~TriggerExecuteTask() = default;

    std::shared_ptr<TaskResult> execute() override;

    void setRowData(const std::vector<trigger::RowData>& old_rows,
                   const std::vector<trigger::RowData>& new_rows);

private:
    trigger::TriggerTiming timing_;
    trigger::TriggerEvent event_;
    std::string table_name_;
    std::vector<trigger::RowData> old_rows_;
    std::vector<trigger::RowData> new_rows_;
};

/**
 * @brief 存储过程/触发器创建任务类
 *
 * 处理CREATE PROCEDURE和CREATE TRIGGER语句
 */
class ProcedureTriggerCreateTask : public Task {
public:
    enum CreateType {
        CREATE_PROCEDURE,
        CREATE_TRIGGER
    };

    ProcedureTriggerCreateTask(const std::string& task_id, CreateType type,
                              std::unique_ptr<sql_parser::CreateStatement> stmt);
    virtual ~ProcedureTriggerCreateTask() = default;

    std::shared_ptr<TaskResult> execute() override;

private:
    CreateType type_;
    std::unique_ptr<sql_parser::CreateStatement> stmt_;
};

/**
 * @brief 存储过程/触发器删除任务类
 *
 * 处理DROP PROCEDURE和DROP TRIGGER语句
 */
class ProcedureTriggerDropTask : public Task {
public:
    enum DropType {
        DROP_PROCEDURE,
        DROP_TRIGGER
    };

    ProcedureTriggerDropTask(const std::string& task_id, DropType type,
                            std::unique_ptr<sql_parser::DropStatement> stmt);
    virtual ~ProcedureTriggerDropTask() = default;

    std::shared_ptr<TaskResult> execute() override;

private:
    DropType type_;
    std::unique_ptr<sql_parser::DropStatement> stmt_;
};

/**
 * @brief 触发器修改任务类
 *
 * 处理ALTER TRIGGER语句
 */
class TriggerAlterTask : public Task {
public:
    TriggerAlterTask(const std::string& task_id, std::unique_ptr<sql_parser::AlterTriggerStatement> stmt);
    virtual ~TriggerAlterTask() = default;

    std::shared_ptr<TaskResult> execute() override;

private:
    std::unique_ptr<sql_parser::AlterTriggerStatement> stmt_;
};

/**
 * @brief DML操作任务类（包含触发器）
 *
 * 处理包含触发器的INSERT/UPDATE/DELETE操作
 */
class DMLWithTriggerTask : public Task {
public:
    enum DMLType {
        INSERT,
        UPDATE,
        DELETE
    };

    DMLWithTriggerTask(const std::string& task_id, DMLType type,
                      std::unique_ptr<sql_parser::Statement> stmt);
    virtual ~DMLWithTriggerTask() = default;

    std::shared_ptr<TaskResult> execute() override;

private:
    DMLType type_;
    std::unique_ptr<sql_parser::Statement> stmt_;
};

} // namespace execution
} // namespace sqlcc

#endif // SQLCC_PROCEDURE_TRIGGER_TASK_H
