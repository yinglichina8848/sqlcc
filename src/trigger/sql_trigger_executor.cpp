#include "sql_trigger_executor.h"
#include "sql_executor.h"
#include <algorithm>
#include <regex>
#include <sstream>
#include <iostream>

namespace sqlcc {
namespace trigger {

const std::string SQLTriggerExecutor::OLD_PREFIX = ":OLD.";
const std::string SQLTriggerExecutor::NEW_PREFIX = ":NEW.";

SQLTriggerExecutor::SQLTriggerExecutor()
    : sql_executor_(nullptr), last_error_("") {}

SQLTriggerExecutor::~SQLTriggerExecutor() {}

bool SQLTriggerExecutor::executeTrigger(const TriggerDefinition* trigger,
                                       const RowData* old_row,
                                       const RowData* new_row) {
    if (!trigger) {
        last_error_ = "Trigger definition is null";
        return false;
    }

    // 检查触发条件（如果有的话）
    if (!trigger->getCondition().empty()) {
        if (!evaluateCondition(trigger->getCondition(), old_row, new_row)) {
            // 条件不满足，跳过触发器执行
            return true;
        }
    }

    // 执行触发器SQL体
    return executeTriggerSQL(trigger->getBody(), old_row, new_row);
}

bool SQLTriggerExecutor::evaluateCondition(const std::string& condition,
                                          const RowData* old_row,
                                          const RowData* new_row) {
    if (condition.empty()) {
        return true; // 没有条件，默认为真
    }

    try {
        // 替换变量引用
        std::string processed_condition = substituteTriggerVariables(condition, old_row, new_row);

        // 简单条件评估（这里可以扩展为更复杂的表达式求值）
        // 目前只支持简单的比较操作

        // TODO: 实现完整的条件表达式求值器
        // 目前返回true作为简化实现
        return true;

    } catch (const std::exception& e) {
        last_error_ = std::string("Condition evaluation error: ") + e.what();
        return false;
    }
}

void SQLTriggerExecutor::setSqlExecutor(std::shared_ptr<SqlExecutor> executor) {
    sql_executor_ = executor;
}

const std::string& SQLTriggerExecutor::getLastError() const {
    return last_error_;
}

bool SQLTriggerExecutor::executeTriggerSQL(const std::string& trigger_sql,
                                          const RowData* old_row,
                                          const RowData* new_row) {
    if (trigger_sql.empty()) {
        return true; // 空SQL体视为成功
    }

    try {
        // 替换触发器变量
        std::string processed_sql = substituteTriggerVariables(trigger_sql, old_row, new_row);

        // 分割并执行SQL语句
        return executeSQLStatements(processed_sql);

    } catch (const std::exception& e) {
        last_error_ = std::string("Trigger SQL execution error: ") + e.what();
        return false;
    }
}

std::string SQLTriggerExecutor::substituteTriggerVariables(const std::string& sql,
                                                          const RowData* old_row,
                                                          const RowData* new_row) {
    std::string result = sql;

    // 替换 :OLD.column_name 引用
    if (old_row) {
        std::regex old_pattern(":OLD\\.(\\w+)");
        std::smatch match;
        std::string::const_iterator search_start(result.cbegin());

        while (std::regex_search(search_start, result.cend(), match, old_pattern)) {
            std::string column_name = match[1].str();
            std::string replacement = "NULL";

            int index = getColumnIndex(old_row->columns, column_name);
            if (index >= 0 && index < static_cast<int>(old_row->values.size())) {
                replacement = escapeSQLString(old_row->values[index]);
            }

            size_t pos = match.position() + (search_start - result.cbegin());
            result.replace(pos, match.length(), replacement);
            search_start = result.cbegin() + pos + replacement.length();
        }
    } else {
        // 如果没有旧行，将所有 :OLD. 引用替换为 NULL
        std::regex old_pattern(":OLD\\.\\w+");
        result = std::regex_replace(result, old_pattern, "NULL");
    }

    // 替换 :NEW.column_name 引用
    if (new_row) {
        std::regex new_pattern(":NEW\\.(\\w+)");
        std::smatch match;
        std::string::const_iterator search_start(result.cbegin());

        while (std::regex_search(search_start, result.cend(), match, new_pattern)) {
            std::string column_name = match[1].str();
            std::string replacement = "NULL";

            int index = getColumnIndex(new_row->columns, column_name);
            if (index >= 0 && index < static_cast<int>(new_row->values.size())) {
                replacement = escapeSQLString(new_row->values[index]);
            }

            size_t pos = match.position() + (search_start - result.cbegin());
            result.replace(pos, match.length(), replacement);
            search_start = result.cbegin() + pos + replacement.length();
        }
    } else {
        // 如果没有新行，将所有 :NEW. 引用替换为 NULL
        std::regex new_pattern(":NEW\\.\\w+");
        result = std::regex_replace(result, new_pattern, "NULL");
    }

    return result;
}

bool SQLTriggerExecutor::executeSQLStatements(const std::string& sql_list) {
    std::vector<std::string> statements = splitSQLStatements(sql_list);

    for (const std::string& sql : statements) {
        std::string trimmed_sql = sql;
        // 移除前后的空白字符
        trimmed_sql.erase(trimmed_sql.begin(),
                         std::find_if(trimmed_sql.begin(), trimmed_sql.end(),
                                    [](unsigned char ch) { return !std::isspace(ch); }));
        trimmed_sql.erase(std::find_if(trimmed_sql.rbegin(), trimmed_sql.rend(),
                                     [](unsigned char ch) { return !std::isspace(ch); }).base(),
                         trimmed_sql.end());

        if (!trimmed_sql.empty()) {
            if (!executeSingleSQL(trimmed_sql)) {
                return false;
            }
        }
    }

    return true;
}

bool SQLTriggerExecutor::executeSingleSQL(const std::string& sql) {
    if (!sql_executor_) {
        last_error_ = "SQL executor not initialized";
        return false;
    }

    try {
        std::string result = sql_executor_->Execute(sql);

        // 检查执行结果是否表示错误
        if (result.find("ERROR") == 0 || result.find("error") == 0) {
            last_error_ = "SQL execution failed: " + result;
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        last_error_ = std::string("SQL execution exception: ") + e.what();
        return false;
    }
}

int SQLTriggerExecutor::getColumnIndex(const std::vector<std::string>& columns,
                                      const std::string& column_name) const {
    auto it = std::find(columns.begin(), columns.end(), column_name);
    if (it != columns.end()) {
        return std::distance(columns.begin(), it);
    }
    return -1;
}

std::string SQLTriggerExecutor::formatRowValue(const RowData* row) const {
    if (!row || row->values.empty()) {
        return "NULL";
    }

    // 对于行级触发器，我们可能需要返回整个行
    // 这里简化为返回第一个值
    return escapeSQLString(row->values[0]);
}

std::string SQLTriggerExecutor::escapeSQLString(const std::string& value) const {
    std::string result = value;

    // 转义单引号
    size_t pos = 0;
    while ((pos = result.find("'", pos)) != std::string::npos) {
        result.replace(pos, 1, "''");
        pos += 2; // 跳过两个单引号
    }

    // 添加包围的单引号
    return "'" + result + "'";
}

std::vector<std::string> SQLTriggerExecutor::splitSQLStatements(const std::string& sql_list) const {
    std::vector<std::string> statements;
    std::string current_statement;
    bool in_string = false;
    char string_delimiter = '\0';

    for (size_t i = 0; i < sql_list.length(); ++i) {
        char c = sql_list[i];

        if (!in_string) {
            if (c == '\'' || c == '"') {
                in_string = true;
                string_delimiter = c;
            } else if (c == ';') {
                // 找到语句分隔符
                if (!current_statement.empty()) {
                    statements.push_back(current_statement);
                    current_statement.clear();
                }
                continue;
            }
        } else {
            if (c == string_delimiter) {
                // 检查是否是转义的引号
                if (i + 1 < sql_list.length() && sql_list[i + 1] == string_delimiter) {
                    // 这是转义的引号，跳过下一个字符
                    i++;
                } else {
                    in_string = false;
                    string_delimiter = '\0';
                }
            }
        }

        current_statement += c;
    }

    // 添加最后一个语句（如果没有分号结尾）
    if (!current_statement.empty()) {
        statements.push_back(current_statement);
    }

    return statements;
}

} // namespace trigger
} // namespace sqlcc
