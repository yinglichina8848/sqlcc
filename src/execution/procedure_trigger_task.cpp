#include "sql_parser/ast_node.h"
#include "sql_parser/ast_nodes.h"
#include "execution/procedure_trigger_task.h"
#include "execution/task_result.h"
#include "procedure/procedure_trigger_executor.h"
#include <chrono>

namespace sqlcc {
namespace execution {

// ProcedureCallTask implementation
ProcedureCallTask::ProcedureCallTask(const std::string& task_id, std::unique_ptr<sql_parser::CallProcedureStatement> stmt)
    : Task(task_id, execution::TaskType::PROCEDURE_CALL), stmt_(std::move(stmt)) {}

std::shared_ptr<TaskResult> ProcedureCallTask::execute() {
    auto result = std::make_shared<TaskResult>(getTaskId());
    auto start_time = std::chrono::high_resolution_clock::now();

    try {
        // 执行存储过程调用
        std::string execution_result = procedure::ProcedureTriggerExecutor::getInstance()
            .executeCallProcedure(stmt_.get());

        result->setSuccess(true);
        result->setResultData(execution_result);

    } catch (const std::exception& e) {
        result->setSuccess(false);
        result->setErrorMessage(std::string("Procedure call failed: ") + e.what());
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    result->setExecutionTime(execution_time);

    return result;
}

// TriggerExecuteTask implementation
TriggerExecuteTask::TriggerExecuteTask(const std::string& task_id,
                                     trigger::TriggerTiming timing,
                                     trigger::TriggerEvent event,
                                     const std::string& table_name)
    : Task(task_id, TaskType::TRIGGER_EXECUTE),
      timing_(timing), event_(event), table_name_(table_name) {}

std::shared_ptr<TaskResult> TriggerExecuteTask::execute() {
    auto result = std::make_shared<TaskResult>(getTaskId());
    auto start_time = std::chrono::high_resolution_clock::now();

    try {
        // 触发触发器执行
        bool success = procedure::ProcedureTriggerExecutor::getInstance()
            .fireDMLEvent(timing_, event_, table_name_, old_rows_, new_rows_);

        result->setSuccess(success);
        if (success) {
            result->setResultData("Trigger executed successfully");
        } else {
            result->setErrorMessage("Trigger execution failed");
        }

    } catch (const std::exception& e) {
        result->setSuccess(false);
        result->setErrorMessage(std::string("Trigger execution error: ") + e.what());
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    result->setExecutionTime(execution_time);

    return result;
}

void TriggerExecuteTask::setRowData(const std::vector<trigger::RowData>& old_rows,
                                   const std::vector<trigger::RowData>& new_rows) {
    old_rows_ = old_rows;
    new_rows_ = new_rows;
}

// ProcedureTriggerCreateTask implementation
ProcedureTriggerCreateTask::ProcedureTriggerCreateTask(const std::string& task_id,
                                                     CreateType type,
                                                     std::unique_ptr<sql_parser::CreateStatement> stmt)
    : Task(task_id, TaskType::SQL_EXECUTE), type_(type), stmt_(std::move(stmt)) {}

std::shared_ptr<TaskResult> ProcedureTriggerCreateTask::execute() {
    auto result = std::make_shared<TaskResult>(getTaskId());
    auto start_time = std::chrono::high_resolution_clock::now();

    try {
        std::string execution_result;

        if (type_ == CREATE_PROCEDURE) {
            // 执行CREATE PROCEDURE
            auto* proc_stmt = dynamic_cast<sql_parser::CreateProcedureStatement*>(stmt_.get());
            execution_result = procedure::ProcedureTriggerExecutor::getInstance()
                .executeCreateProcedure(proc_stmt);
        } else {
            // 执行CREATE TRIGGER
            auto* trig_stmt = dynamic_cast<sql_parser::CreateTriggerStatement*>(stmt_.get());
            execution_result = procedure::ProcedureTriggerExecutor::getInstance()
                .executeCreateTrigger(trig_stmt);
        }

        result->setSuccess(true);
        result->setResultData(execution_result);

    } catch (const std::exception& e) {
        result->setSuccess(false);
        result->setErrorMessage(std::string("Create statement failed: ") + e.what());
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    result->setExecutionTime(execution_time);

    return result;
}

// ProcedureTriggerDropTask implementation
ProcedureTriggerDropTask::ProcedureTriggerDropTask(const std::string& task_id,
                                                 DropType type,
                                                 std::unique_ptr<sql_parser::DropStatement> stmt)
    : Task(task_id, TaskType::SQL_EXECUTE), type_(type), stmt_(std::move(stmt)) {}

std::shared_ptr<TaskResult> ProcedureTriggerDropTask::execute() {
    auto result = std::make_shared<TaskResult>(getTaskId());
    auto start_time = std::chrono::high_resolution_clock::now();

    try {
        std::string execution_result;

        if (type_ == DROP_PROCEDURE) {
            // 执行DROP PROCEDURE
            auto* proc_stmt = dynamic_cast<sql_parser::DropProcedureStatement*>(stmt_.get());
            execution_result = procedure::ProcedureTriggerExecutor::getInstance()
                .executeDropProcedure(proc_stmt);
        } else {
            // 执行DROP TRIGGER
            auto* trig_stmt = dynamic_cast<sql_parser::DropTriggerStatement*>(stmt_.get());
            execution_result = procedure::ProcedureTriggerExecutor::getInstance()
                .executeDropTrigger(trig_stmt);
        }

        result->setSuccess(true);
        result->setResultData(execution_result);

    } catch (const std::exception& e) {
        result->setSuccess(false);
        result->setErrorMessage(std::string("Drop statement failed: ") + e.what());
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    result->setExecutionTime(execution_time);

    return result;
}

// TriggerAlterTask implementation
TriggerAlterTask::TriggerAlterTask(const std::string& task_id,
                                 std::unique_ptr<sql_parser::AlterTriggerStatement> stmt)
    : Task(task_id, TaskType::SQL_EXECUTE), stmt_(std::move(stmt)) {}

std::shared_ptr<TaskResult> TriggerAlterTask::execute() {
    auto result = std::make_shared<TaskResult>(getTaskId());
    auto start_time = std::chrono::high_resolution_clock::now();

    try {
        // 执行ALTER TRIGGER
        std::string execution_result = procedure::ProcedureTriggerExecutor::getInstance()
            .executeAlterTrigger(stmt_.get());

        result->setSuccess(true);
        result->setResultData(execution_result);

    } catch (const std::exception& e) {
        result->setSuccess(false);
        result->setErrorMessage(std::string("Alter trigger failed: ") + e.what());
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    result->setExecutionTime(execution_time);

    return result;
}

// DMLWithTriggerTask implementation
DMLWithTriggerTask::DMLWithTriggerTask(const std::string& task_id,
                                     DMLType type,
                                     std::unique_ptr<sql_parser::Statement> stmt)
    : Task(task_id, TaskType::SQL_EXECUTE), type_(type), stmt_(std::move(stmt)) {}

std::shared_ptr<TaskResult> DMLWithTriggerTask::execute() {
    auto result = std::make_shared<TaskResult>(getTaskId());
    auto start_time = std::chrono::high_resolution_clock::now();

    try {
        std::string execution_result;

        if (type_ == INSERT) {
            // 执行INSERT with triggers
            auto* insert_stmt = dynamic_cast<sql_parser::InsertStatement*>(stmt_.get());
            execution_result = procedure::ProcedureTriggerExecutor::getInstance()
                .executeInsertWithTriggers(insert_stmt);
        } else if (type_ == UPDATE) {
            // 执行UPDATE with triggers
            auto* update_stmt = dynamic_cast<sql_parser::UpdateStatement*>(stmt_.get());
            execution_result = procedure::ProcedureTriggerExecutor::getInstance()
                .executeUpdateWithTriggers(update_stmt);
        } else if (type_ == DELETE) {
            // 执行DELETE with triggers
            auto* delete_stmt = dynamic_cast<sql_parser::DeleteStatement*>(stmt_.get());
            execution_result = procedure::ProcedureTriggerExecutor::getInstance()
                .executeDeleteWithTriggers(delete_stmt);
        }

        // 检查执行结果是否成功
        if (execution_result.find("ERROR") == 0) {
            result->setSuccess(false);
            result->setErrorMessage(execution_result);
        } else {
            result->setSuccess(true);
            result->setResultData(execution_result);
        }

    } catch (const std::exception& e) {
        result->setSuccess(false);
        result->setErrorMessage(std::string("DML operation failed: ") + e.what());
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    result->setExecutionTime(execution_time);

    return result;
}

} // namespace execution
} // namespace sqlcc
