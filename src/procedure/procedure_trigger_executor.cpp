#include "procedure/procedure_trigger_executor.h"
#include <algorithm>
#include <sstream>

namespace sqlcc {
namespace procedure {

ProcedureTriggerExecutor& ProcedureTriggerExecutor::getInstance() {
    static ProcedureTriggerExecutor instance;
    return instance;
}

ProcedureTriggerExecutor::ProcedureTriggerExecutor()
    : sql_executor_(nullptr), last_error_("") {}

ProcedureTriggerExecutor::~ProcedureTriggerExecutor() {}

void ProcedureTriggerExecutor::initialize(SqlExecutor* sql_executor) {
    sql_executor_ = sql_executor;
    // Create ProcedureVM with a null executor initially
    procedure_vm_ = std::make_unique<ProcedureVM>(nullptr);
    trigger::TriggerManager::getInstance().initialize(sql_executor);
}

std::string ProcedureTriggerExecutor::executeCreateProcedure(sql_parser::CreateProcedureStatement* stmt) {
    if (!stmt) {
        last_error_ = "Null CREATE PROCEDURE statement";
        return "ERROR: Null statement";
    }

    std::lock_guard<std::mutex> lock(procedures_mutex_);

    const std::string& procedure_name = stmt->getName();

    // 检查过程是否已存在
    if (stored_procedures_.find(procedure_name) != stored_procedures_.end()) {
        last_error_ = "Procedure '" + procedure_name + "' already exists";
        return "ERROR: Procedure already exists";
    }

    // 解析过程体
    const std::string& body = stmt->getBody();
    auto ast = procedure_parser_.parse(body);
    if (!ast) {
        last_error_ = "Failed to parse procedure body: " + procedure_parser_.getErrorMessage();
        return "ERROR: Parse error - " + procedure_parser_.getErrorMessage();
    }

    // 转换为ProcedureDefinition
    auto procedure_def = dynamic_cast<ProcedureDefinition*>(ast.get());
    if (!procedure_def) {
        last_error_ = "Invalid procedure definition";
        return "ERROR: Invalid procedure definition";
    }

    // 设置过程名称
    procedure_def->setName(procedure_name);

    // 存储过程定义
    stored_procedures_[procedure_name] = std::unique_ptr<ProcedureDefinition>(procedure_def);
    ast.release(); // 释放所有权，因为我们已经转移了

    return "PROCEDURE '" + procedure_name + "' created successfully";
}

std::string ProcedureTriggerExecutor::executeCreateTrigger(sql_parser::CreateTriggerStatement* stmt) {
    if (!stmt) {
        last_error_ = "Null CREATE TRIGGER statement";
        return "ERROR: Null statement";
    }

    const auto& trigger_def = stmt->getTriggerDefinition();

    // 转换触发器定义
    auto trigger = std::make_unique<trigger::TriggerDefinition>(
        trigger_def.getName(),
        (trigger_def.getTiming() == sql_parser::TriggerDefinition::BEFORE) ?
            trigger::TriggerTiming::BEFORE : trigger::TriggerTiming::AFTER,
        (trigger_def.getEvent() == sql_parser::TriggerDefinition::INSERT) ?
            trigger::TriggerEvent::INSERT :
            (trigger_def.getEvent() == sql_parser::TriggerDefinition::UPDATE) ?
                trigger::TriggerEvent::UPDATE : trigger::TriggerEvent::DELETE,
        (trigger_def.getLevel() == sql_parser::TriggerDefinition::ROW) ?
            trigger::TriggerLevel::ROW : trigger::TriggerLevel::STATEMENT,
        trigger_def.getTableName()
    );

    trigger->setCondition(trigger_def.getCondition());
    trigger->setBody(trigger_def.getBody());
    trigger->setDefiner(trigger_def.getDefiner());

    // 注册触发器
    if (!trigger::TriggerManager::getInstance().createTrigger(std::move(trigger))) {
        last_error_ = trigger::TriggerManager::getInstance().getLastError();
        return "ERROR: " + last_error_;
    }

    return "TRIGGER '" + trigger_def.getName() + "' created successfully";
}

std::string ProcedureTriggerExecutor::executeCallProcedure(sql_parser::CallProcedureStatement* stmt) {
    if (!stmt) {
        last_error_ = "Null CALL PROCEDURE statement";
        return "ERROR: Null statement";
    }

    const std::string& procedure_name = stmt->getName();

    std::lock_guard<std::mutex> lock(procedures_mutex_);

    // 查找存储过程
    auto it = stored_procedures_.find(procedure_name);
    if (it == stored_procedures_.end()) {
        last_error_ = "Procedure '" + procedure_name + "' does not exist";
        return "ERROR: Procedure does not exist";
    }

    // 创建执行上下文
    ProcedureContext context(sql_executor_);

    // 设置参数（暂时不支持参数传递）
    // TODO: 实现参数传递

    // 执行过程
    if (!procedure_vm_.execute(it->second.get(), context)) {
        last_error_ = procedure_vm_.getLastError();
        return "ERROR: Procedure execution failed - " + last_error_;
    }

    // 返回执行结果
    const Value& return_value = context.getReturnValue();
    if (return_value.getType() != Value::NULL_VALUE) {
        return "PROCEDURE '" + procedure_name + "' executed successfully, returned: " + return_value.toString();
    }

    return "PROCEDURE '" + procedure_name + "' executed successfully";
}

std::string ProcedureTriggerExecutor::executeDropProcedure(sql_parser::DropProcedureStatement* stmt) {
    if (!stmt) {
        last_error_ = "Null DROP PROCEDURE statement";
        return "ERROR: Null statement";
    }

    std::lock_guard<std::mutex> lock(procedures_mutex_);

    const std::string& procedure_name = stmt->getName();

    auto it = stored_procedures_.find(procedure_name);
    if (it == stored_procedures_.end()) {
        last_error_ = "Procedure '" + procedure_name + "' does not exist";
        return "ERROR: Procedure does not exist";
    }

    stored_procedures_.erase(it);
    return "PROCEDURE '" + procedure_name + "' dropped successfully";
}

std::string ProcedureTriggerExecutor::executeDropTrigger(sql_parser::DropTriggerStatement* stmt) {
    if (!stmt) {
        last_error_ = "Null DROP TRIGGER statement";
        return "ERROR: Null statement";
    }

    const std::string& trigger_name = stmt->getName();

    if (!trigger::TriggerManager::getInstance().dropTrigger(trigger_name)) {
        last_error_ = trigger::TriggerManager::getInstance().getLastError();
        return "ERROR: " + last_error_;
    }

    return "TRIGGER '" + trigger_name + "' dropped successfully";
}

std::string ProcedureTriggerExecutor::executeAlterTrigger(sql_parser::AlterTriggerStatement* stmt) {
    if (!stmt) {
        last_error_ = "Null ALTER TRIGGER statement";
        return "ERROR: Null statement";
    }

    const std::string& trigger_name = stmt->getName();
    bool enable = (stmt->getAction() == sql_parser::AlterTriggerStatement::ENABLE);

    bool success = enable ?
        trigger::TriggerManager::getInstance().enableTrigger(trigger_name) :
        trigger::TriggerManager::getInstance().disableTrigger(trigger_name);

    if (!success) {
        last_error_ = trigger::TriggerManager::getInstance().getLastError();
        return "ERROR: " + last_error_;
    }

    std::string action_str = enable ? "enabled" : "disabled";
    return "TRIGGER '" + trigger_name + "' " + action_str + " successfully";
}

std::string ProcedureTriggerExecutor::executeInsertWithTriggers(sql_parser::InsertStatement* stmt) {
    if (!stmt) {
        last_error_ = "Null INSERT statement";
        return "ERROR: Null statement";
    }

    const std::string& table_name = stmt->getTableName();

    // 触发BEFORE INSERT触发器
    std::vector<trigger::RowData> new_rows = convertToTriggerRows(
        stmt->getValues(), stmt->getColumns());

    if (!fireDMLEvent(trigger::TriggerTiming::BEFORE, trigger::TriggerEvent::INSERT,
                     table_name, {}, new_rows)) {
        return "ERROR: BEFORE INSERT trigger failed";
    }

    // 执行原始INSERT操作
    std::string result = executeRawSQL(buildInsertSQL(stmt));
    if (result.find("ERROR") == 0) {
        return result;
    }

    // 触发AFTER INSERT触发器
    if (!fireDMLEvent(trigger::TriggerTiming::AFTER, trigger::TriggerEvent::INSERT,
                     table_name, {}, new_rows)) {
        return "ERROR: AFTER INSERT trigger failed";
    }

    return result;
}

std::string ProcedureTriggerExecutor::executeUpdateWithTriggers(sql_parser::UpdateStatement* stmt) {
    if (!stmt) {
        last_error_ = "Null UPDATE statement";
        return "ERROR: Null statement";
    }

    const std::string& table_name = stmt->getTableName();

    // 获取旧行数据（简化实现，实际需要从数据库查询）
    std::vector<trigger::RowData> old_rows; // TODO: 实现获取旧行数据

    // 构造新行数据（简化实现）
    std::vector<trigger::RowData> new_rows; // TODO: 实现构造新行数据

    // 触发BEFORE UPDATE触发器
    if (!fireDMLEvent(trigger::TriggerTiming::BEFORE, trigger::TriggerEvent::UPDATE,
                     table_name, old_rows, new_rows)) {
        return "ERROR: BEFORE UPDATE trigger failed";
    }

    // 执行原始UPDATE操作
    std::string result = executeRawSQL(buildUpdateSQL(stmt));
    if (result.find("ERROR") == 0) {
        return result;
    }

    // 触发AFTER UPDATE触发器
    if (!fireDMLEvent(trigger::TriggerTiming::AFTER, trigger::TriggerEvent::UPDATE,
                     table_name, old_rows, new_rows)) {
        return "ERROR: AFTER UPDATE trigger failed";
    }

    return result;
}

std::string ProcedureTriggerExecutor::executeDeleteWithTriggers(sql_parser::DeleteStatement* stmt) {
    if (!stmt) {
        last_error_ = "Null DELETE statement";
        return "ERROR: Null statement";
    }

    const std::string& table_name = stmt->getTableName();

    // 获取旧行数据（简化实现，实际需要从数据库查询）
    std::vector<trigger::RowData> old_rows; // TODO: 实现获取旧行数据

    // 触发BEFORE DELETE触发器
    if (!fireDMLEvent(trigger::TriggerTiming::BEFORE, trigger::TriggerEvent::DELETE,
                     table_name, old_rows, {})) {
        return "ERROR: BEFORE DELETE trigger failed";
    }

    // 执行原始DELETE操作
    std::string result = executeRawSQL(buildDeleteSQL(stmt));
    if (result.find("ERROR") == 0) {
        return result;
    }

    // 触发AFTER DELETE触发器
    if (!fireDMLEvent(trigger::TriggerTiming::AFTER, trigger::TriggerEvent::DELETE,
                     table_name, old_rows, {})) {
        return "ERROR: AFTER DELETE trigger failed";
    }

    return result;
}

const std::string& ProcedureTriggerExecutor::getLastError() const {
    return last_error_;
}

void ProcedureTriggerExecutor::setTriggerExecutor(std::unique_ptr<trigger::TriggerExecutor> executor) {
    trigger::TriggerManager::getInstance().setTriggerExecutor(std::move(executor));
}

bool ProcedureTriggerExecutor::fireDMLEvent(trigger::TriggerTiming timing, trigger::TriggerEvent event,
                                          const std::string& table_name,
                                          const std::vector<trigger::RowData>& old_rows,
                                          const std::vector<trigger::RowData>& new_rows) {
    return trigger::TriggerManager::getInstance().fireTriggers(timing, event, table_name, old_rows, new_rows);
}

std::vector<trigger::RowData> ProcedureTriggerExecutor::convertToTriggerRows(
    const std::vector<std::vector<std::string>>& sql_rows,
    const std::vector<std::string>& columns) {

    std::vector<trigger::RowData> trigger_rows;

    for (const auto& sql_row : sql_rows) {
        if (sql_row.size() == columns.size()) {
            trigger_rows.emplace_back(columns, sql_row);
        }
    }

    return trigger_rows;
}

std::string ProcedureTriggerExecutor::executeRawSQL(const std::string& sql) {
    if (!sql_executor_) {
        last_error_ = "SQL executor not initialized";
        return "ERROR: SQL executor not initialized";
    }

    try {
        return sql_executor_->Execute(sql);
    } catch (const std::exception& e) {
        last_error_ = std::string("SQL execution error: ") + e.what();
        return "ERROR: " + last_error_;
    }
}

std::string ProcedureTriggerExecutor::buildInsertSQL(sql_parser::InsertStatement* stmt) {
    if (!stmt) return "";

    std::stringstream sql;
    sql << "INSERT INTO " << stmt->getTableName();

    const auto& columns = stmt->getColumns();
    if (!columns.empty()) {
        sql << " (";
        for (size_t i = 0; i < columns.size(); ++i) {
            if (i > 0) sql << ", ";
            sql << columns[i];
        }
        sql << ")";
    }

    sql << " VALUES ";
    const auto& values = stmt->getValues();
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) sql << ", ";
        sql << "(";
        for (size_t j = 0; j < values[i].size(); ++j) {
            if (j > 0) sql << ", ";
            sql << "'" << values[i][j] << "'";  // 简单字符串处理
        }
        sql << ")";
    }

    return sql.str();
}

std::string ProcedureTriggerExecutor::buildUpdateSQL(sql_parser::UpdateStatement* stmt) {
    if (!stmt) return "";

    std::stringstream sql;
    sql << "UPDATE " << stmt->getTableName() << " SET ";

    const auto& updates = stmt->getUpdateValues();
    size_t count = 0;
    for (const auto& update : updates) {
        if (count > 0) sql << ", ";
        sql << update.first << " = '" << update.second << "'";  // 简单处理
        count++;
    }

    if (stmt->hasWhereClause()) {
        const auto& where = stmt->getWhereClause();
        sql << " WHERE " << where.getColumnName() << " " << where.getOp() << " '" << where.getValue() << "'";
    }

    return sql.str();
}

std::string ProcedureTriggerExecutor::buildDeleteSQL(sql_parser::DeleteStatement* stmt) {
    if (!stmt) return "";

    std::stringstream sql;
    sql << "DELETE FROM " << stmt->getTableName();

    if (stmt->hasWhereClause()) {
        const auto& where = stmt->getWhereClause();
        sql << " WHERE " << where.getColumnName() << " " << where.getOp() << " '" << where.getValue() << "'";
    }

    return sql.str();
}

} // namespace procedure
} // namespace sqlcc
