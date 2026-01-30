#include "src/sql_parser/ast/ast_node.h"
#include "src/sql_executor/sql_executor.h"
#include "src/core/core_database_manager.h"
#include "src/core/user_manager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace sqlcc {

SqlExecutor::SqlExecutor()
    : db_manager_(std::make_shared<DatabaseManager>("./data")),
      user_manager_(std::make_shared<UserManager>()) {}

SqlExecutor::SqlExecutor(std::shared_ptr<DatabaseManager> db_manager)
    : db_manager_(db_manager),
      user_manager_(std::make_shared<UserManager>()) {}

SqlExecutor::~SqlExecutor() = default;

std::string SqlExecutor::Execute(const std::string &sql) {
    ClearError();

    // 去除空白字符
    std::string trimmed_sql = sql;
    TrimString(trimmed_sql);

    if (trimmed_sql.empty()) {
        return "OK";
    }

    // 简化的实现：直接返回成功消息
    return "EXECUTED: " + trimmed_sql;
}

std::string SqlExecutor::ExecuteFile(const std::string &file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        SetError("Cannot open file: " + file_path);
        return "ERROR: Cannot open file: " + file_path;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    return Execute(content);
}

std::string SqlExecutor::GetLastError() const {
    return last_error_;
}

std::string SqlExecutor::GetExecutionStats() const {
    return execution_stats_;
}

void SqlExecutor::SetError(const std::string &error) {
    last_error_ = error;
}

void SqlExecutor::ClearError() {
    last_error_.clear();
}

bool SqlExecutor::InitializeSystemDatabase() {
    // 简化的实现
    return true;
}

std::unique_ptr<sql_parser::Statement> SqlExecutor::ParseSQL(const std::string &sql) {
    // 简化的实现
    return nullptr;
}

std::unique_ptr<UnifiedQueryPlan>
SqlExecutor::CreateQueryPlan(std::unique_ptr<sql_parser::Statement> stmt) {
    // 简化的实现
    return nullptr;
}

bool SqlExecutor::InitializePermissionValidator() {
    // 简化的实现
    return true;
}

void SqlExecutor::UpdateCurrentDatabase(const std::string &sql) {
    // 简单的实现：检查是否是USE语句
    std::string upper_sql = sql;
    std::transform(upper_sql.begin(), upper_sql.end(), upper_sql.begin(), ::toupper);

    if (upper_sql.find("USE ") == 0) {
        size_t start = upper_sql.find("USE ") + 4;
        size_t end = upper_sql.find(';', start);
        if (end == std::string::npos) {
            end = upper_sql.length();
        }
        std::string db_name = upper_sql.substr(start, end - start);
        TrimString(db_name);
        current_database_ = db_name;
    }
}

void SqlExecutor::TrimString(std::string &str) {
    // 去除左边空白
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start != std::string::npos) {
        str = str.substr(start);
    } else {
        str.clear();
        return;
    }

    // 去除右边空白
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    if (end != std::string::npos) {
        str = str.substr(0, end + 1);
    }
}

} // namespace sqlcc
