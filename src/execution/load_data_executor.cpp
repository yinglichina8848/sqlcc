#include "sql_parser/ast_nodes.h"
#include "execution/load_data_executor.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <iomanip>

namespace sqlcc {

LoadDataExecutor::LoadDataExecutor(std::shared_ptr<StorageEngine> storage_engine,
                                 std::shared_ptr<SqlExecutor> sql_executor)
    : storage_engine_(storage_engine), sql_executor_(sql_executor) {
}

LoadDataExecutor::~LoadDataExecutor() = default;

ExecutionResult LoadDataExecutor::execute(const sql_parser::LoadDataStatement& stmt) {
    ExecutionResult result;
    result.success = false;

    try {
        // 重置统计信息
        stats_ = LoadStats();

        // 1. 验证表是否存在
        if (!checkTableExists(stmt.table_name)) {
            result.error_message() = "Table '" + stmt.table_name + "' does not exist";
            return result;
        }

        // 2. 获取表元数据
        auto table_meta = getTableMetadata(stmt.table_name);
        if (!table_meta) {
            result.error_message() = "Failed to get metadata for table '" + stmt.table_name + "'";
            return result;
        }

        // 3. 检查文件权限
        if (!checkFilePermissions(stmt.file_name, stmt.is_local)) {
            result.error_message() = "Cannot access file '" + stmt.file_name + "'";
            return result;
        }

        // 4. 打开文件
        std::ifstream file;
        if (!openFile(stmt.file_name, stmt.is_local, file)) {
            result.error_message() = "Failed to open file '" + stmt.file_name + "'";
            return result;
        }

        // 5. 逐行处理文件
        std::string line;
        size_t line_number = 0;
        auto start_time = std::chrono::steady_clock::now();

        // 进度报告间隔（每1000行报告一次）
        const size_t progress_interval = 1000;

        while (readLine(file, line)) {
            line_number++;
            stats_.total_lines++;

            // 跳过指定行数的开头行
            if (line_number <= static_cast<size_t>(stmt.ignore_lines)) {
                stats_.skipped_lines++;
                continue;
            }

            // 进度报告
            if (line_number % progress_interval == 0) {
                auto current_time = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
                double rows_per_sec = static_cast<double>(stats_.inserted_rows) / (elapsed.count() / 1000.0);

                std::cout << "[LOAD DATA PROGRESS] Processed " << line_number << " lines, "
                         << "inserted " << stats_.inserted_rows << " rows, "
                         << std::fixed << std::setprecision(1) << rows_per_sec << " rows/sec"
                         << std::endl;
            }

            // 解析行数据
            auto raw_fields = parseFields(line, stmt);

            // 验证和转换数据
            std::vector<std::string> processed_row;
            if (!validateAndConvertRow(raw_fields, processed_row, stmt, table_meta)) {
                stats_.failed_rows++;
                stats_.errors.push_back("Line " + std::to_string(line_number) + ": validation failed");
                if (stmt.replace_or_ignore != "IGNORE") {
                    // 如果不是IGNORE模式，继续处理下一行
                    continue;
                }
            }

            // 应用SET表达式
            if (!applySetExpressions(processed_row, stmt)) {
                stats_.failed_rows++;
                stats_.errors.push_back("Line " + std::to_string(line_number) + ": SET expression failed");
                continue;
            }

            // 验证约束
            if (!validateConstraints(processed_row, table_meta)) {
                stats_.failed_rows++;
                stats_.errors.push_back("Line " + std::to_string(line_number) + ": constraint violation");
                continue;
            }

            // 插入数据
            if (insertRow(processed_row, stmt, table_meta)) {
                stats_.inserted_rows++;
            } else {
                stats_.failed_rows++;
                stats_.errors.push_back("Line " + std::to_string(line_number) + ": insert failed");
            }
        }

        // 关闭文件
        closeFile(file);

        // 设置结果
        result.success = true;
        result.rows_affected = stats_.inserted_rows;
        result.message = "Loaded " + std::to_string(stats_.inserted_rows) + " rows into table '" +
                        stmt.table_name + "'";

        if (stats_.warnings > 0) {
            result.message += " (" + std::to_string(stats_.warnings) + " warnings)";
        }

        if (!stats_.errors.empty() && stats_.errors.size() <= 10) {
            result.message += "\nErrors: ";
            for (size_t i = 0; i < stats_.errors.size(); ++i) {
                if (i > 0) result.message += "; ";
                result.message += stats_.errors[i];
            }
        }

    } catch (const std::exception& e) {
        result.error_message() = "LOAD DATA execution failed: " + std::string(e.what());
        result.success = false;
    }

    return result;
}

bool LoadDataExecutor::openFile(const std::string& filename, bool is_local, std::ifstream& file) {
    try {
        file.open(filename, std::ios::in);
        return file.is_open();
    } catch (const std::exception& e) {
        logError("Failed to open file '" + filename + "': " + e.what());
        return false;
    }
}

bool LoadDataExecutor::readLine(std::ifstream& file, std::string& line) {
    return std::getline(file, line).good();
}

void LoadDataExecutor::closeFile(std::ifstream& file) {
    if (file.is_open()) {
        file.close();
    }
}

std::vector<std::string> LoadDataExecutor::parseFields(const std::string& line, const sql_parser::LoadDataStatement& stmt) {
    std::vector<std::string> fields;

    if (line.empty()) {
        return fields;
    }

    // 处理行起始符
    std::string processed_line = line;
    if (!stmt.lines_starting_by.empty()) {
        if (processed_line.find(stmt.lines_starting_by) == 0) {
            processed_line = processed_line.substr(stmt.lines_starting_by.length());
        }
    }

    // 处理行终止符
    if (!stmt.lines_terminated_by.empty()) {
        size_t pos = processed_line.find(stmt.lines_terminated_by);
        if (pos != std::string::npos) {
            processed_line = processed_line.substr(0, pos);
        }
    }

    // 解析字段
    std::string current_field;
    bool in_quotes = false;
    char quote_char = '\0';

    for (size_t i = 0; i < processed_line.length(); ++i) {
        char c = processed_line[i];

        // 处理包围符
        if (!stmt.fields_enclosed_by.empty() && c == stmt.fields_enclosed_by[0]) {
            if (!in_quotes) {
                in_quotes = true;
                quote_char = c;
                if (!stmt.fields_optionally_enclosed) {
                    continue; // 跳过包围符
                }
            } else if (in_quotes && quote_char == c) {
                in_quotes = false;
                quote_char = '\0';
                if (!stmt.fields_optionally_enclosed) {
                    continue; // 跳过包围符
                }
            }
        }

        // 处理字段终止符
        if (!in_quotes && !stmt.fields_terminated_by.empty()) {
            bool is_terminator = true;
            for (size_t j = 0; j < stmt.fields_terminated_by.length(); ++j) {
                if (i + j >= processed_line.length() || processed_line[i + j] != stmt.fields_terminated_by[j]) {
                    is_terminator = false;
                    break;
                }
            }

            if (is_terminator) {
                // 找到字段终止符
                fields.push_back(current_field);
                current_field.clear();
                i += stmt.fields_terminated_by.length() - 1; // 跳过终止符
                continue;
            }
        }

        current_field += c;
    }

    // 添加最后一个字段
    if (!current_field.empty() || !fields.empty()) {
        fields.push_back(current_field);
    }

    return fields;
}

std::string LoadDataExecutor::unescapeField(const std::string& field, char escape_char) {
    if (escape_char == '\0') {
        return field;
    }

    std::string result;
    for (size_t i = 0; i < field.length(); ++i) {
        if (field[i] == escape_char && i + 1 < field.length()) {
            // 处理转义字符
            char next = field[i + 1];
            switch (next) {
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case '0': result += '\0'; break;
                default: result += next; break;
            }
            i++; // 跳过转义字符
        } else {
            result += field[i];
        }
    }
    return result;
}

std::string LoadDataExecutor::removeEnclosure(const std::string& field,
                                             const std::string& enclosure,
                                             bool optionally) {
    if (enclosure.empty()) {
        return field;
    }

    std::string result = field;
    char enc_char = enclosure[0];

    // 检查是否被包围符包围
    if (result.length() >= 2 && result.front() == enc_char && result.back() == enc_char) {
        result = result.substr(1, result.length() - 2);
    } else if (!optionally) {
        // 如果不是可选包围，则这是错误的数据格式
        logWarning("Field not properly enclosed: " + field);
    }

    return result;
}

bool LoadDataExecutor::validateAndConvertRow(const std::vector<std::string>& raw_fields,
                                           std::vector<std::string>& processed_row,
                                           const sql_parser::LoadDataStatement& stmt,
                                           std::shared_ptr<TableMetadata> table_meta) {
    // 获取列信息
    const auto& columns = table_meta->columns;

    // 确定要使用的列
    std::vector<size_t> column_indices;
    if (!stmt.column_list.empty()) {
        // 使用指定的列映射
        for (const auto& col_name : stmt.column_list) {
            auto it = std::find_if(columns.begin(), columns.end(),
                                 [&col_name](const TableColumn& col) {
                                     return col.name == col_name;
                                 });
            if (it == columns.end()) {
                logError("Unknown column '" + col_name + "' in column list");
                return false;
            }
            column_indices.push_back(std::distance(columns.begin(), it));
        }
    } else {
        // 使用所有列
        for (size_t i = 0; i < columns.size(); ++i) {
            column_indices.push_back(i);
        }
    }

    // 检查字段数量
    if (raw_fields.size() != column_indices.size()) {
        logError("Field count mismatch: expected " + std::to_string(column_indices.size()) +
                ", got " + std::to_string(raw_fields.size()));
        return false;
    }

    // 处理每个字段
    processed_row.resize(columns.size(), ""); // 初始化为空值

    for (size_t i = 0; i < raw_fields.size(); ++i) {
        size_t col_idx = column_indices[i];
        const auto& column = columns[col_idx];

        std::string field = raw_fields[i];

        // 移除包围符
        field = removeEnclosure(field, stmt.fields_enclosed_by, stmt.fields_optionally_enclosed);

        // 反转义
        if (!stmt.fields_escaped_by.empty()) {
            field = unescapeField(field, stmt.fields_escaped_by[0]);
        }

        // 基本类型验证和转换
        if (field.empty() && !column.nullable) {
            logError("Column '" + column.name + "' cannot be NULL");
            return false;
        }

        // TODO: 添加更完整的类型验证和转换逻辑
        processed_row[col_idx] = field;
    }

    return true;
}

bool LoadDataExecutor::applySetExpressions(std::vector<std::string>& row,
                                         const sql_parser::LoadDataStatement& stmt) {
    if (stmt.set_expressions.empty()) {
        return true;
    }

    try {
        auto table_meta = getTableMetadata(stmt.table_name);
        if (!table_meta) {
            logError("Cannot get table metadata for SET expressions");
            return false;
        }

        const auto& columns = table_meta->columns;

        for (const auto& set_expr : stmt.set_expressions) {
            const std::string& column_name = set_expr.first;
            const std::string& expression = set_expr.second;

            // 查找列索引
            size_t col_index = std::numeric_limits<size_t>::max();
            for (size_t i = 0; i < columns.size(); ++i) {
                if (columns[i].name == column_name) {
                    col_index = i;
                    break;
                }
            }

            if (col_index == std::numeric_limits<size_t>::max()) {
                logError("Unknown column '" + column_name + "' in SET expression");
                return false;
            }

            // 解析和计算SET表达式
            std::string result = evaluateSetExpression(expression, row, columns);
            row[col_index] = result;
        }

        return true;
    } catch (const std::exception& e) {
        logError("Error applying SET expressions: " + std::string(e.what()));
        return false;
    }
}

std::string LoadDataExecutor::evaluateSetExpression(const std::string& expression,
                                                   const std::vector<std::string>& row,
                                                   const std::vector<TableColumn>& columns) {
    // 简化的表达式求值器实现
    // 支持基本的算术运算和列引用

    std::string expr = expression;
    // 移除空格
    expr.erase(std::remove_if(expr.begin(), expr.end(), ::isspace), expr.end());

    // 处理列引用 (@column_name -> 列值)
    for (size_t i = 0; i < columns.size(); ++i) {
        const std::string& col_name = columns[i].name;
        std::string placeholder = "@" + col_name;

        size_t pos = 0;
        while ((pos = expr.find(placeholder, pos)) != std::string::npos) {
            expr.replace(pos, placeholder.length(), row[i]);
            pos += row[i].length();
        }
    }

    // 处理简单的算术表达式
    return evaluateArithmeticExpression(expr);
}

std::string LoadDataExecutor::evaluateArithmeticExpression(const std::string& expr) {
    // 超简化的算术表达式求值器
    // 只支持基本的数值运算

    try {
        // 检查是否是纯数字
        if (expr.find_first_not_of("0123456789.-+") == std::string::npos) {
            // 处理简单的加法，如 "50000 + 5000"
            size_t plus_pos = expr.find('+');
            if (plus_pos != std::string::npos) {
                double left = std::stod(expr.substr(0, plus_pos));
                double right = std::stod(expr.substr(plus_pos + 1));
                return std::to_string(left + right);
            }

            // 处理简单的乘法，如 "50000 * 1.1"
            size_t mult_pos = expr.find('*');
            if (mult_pos != std::string::npos) {
                double left = std::stod(expr.substr(0, mult_pos));
                double right = std::stod(expr.substr(mult_pos + 1));
                return std::to_string(left * right);
            }

            // 处理简单的除法，如 "50000 / 2"
            size_t div_pos = expr.find('/');
            if (div_pos != std::string::npos) {
                double left = std::stod(expr.substr(0, div_pos));
                double right = std::stod(expr.substr(div_pos + 1));
                if (right != 0) {
                    return std::to_string(left / right);
                }
            }

            // 处理简单的减法，如 "50000 - 5000"
            size_t minus_pos = expr.find('-');
            if (minus_pos != std::string::npos && minus_pos > 0) { // 避免负数开头
                double left = std::stod(expr.substr(0, minus_pos));
                double right = std::stod(expr.substr(minus_pos + 1));
                return std::to_string(left - right);
            }
        }

        // 如果无法解析，返回原表达式
        return expr;
    } catch (const std::exception&) {
        // 如果解析失败，返回原表达式
        return expr;
    }
}

bool LoadDataExecutor::validateConstraints(const std::vector<std::string>& row,
                                         std::shared_ptr<TableMetadata> table_meta) {
    try {
        const auto& columns = table_meta->columns;

        // 1. 检查NOT NULL约束
        for (size_t i = 0; i < row.size() && i < columns.size(); ++i) {
            const auto& column = columns[i];
            const std::string& value = row[i];

            if (!column.nullable && (value.empty() || value == "NULL")) {
                logError("NOT NULL constraint violation for column '" + column.name + "'");
                return false;
            }
        }

        // 2. 检查数据类型约束（简化实现）
        for (size_t i = 0; i < row.size() && i < columns.size(); ++i) {
            const auto& column = columns[i];
            const std::string& value = row[i];

            if (value.empty() || value == "NULL") {
                continue; // NULL值跳过类型检查
            }

            // 基本类型验证
            if (!validateDataType(value, column.type)) {
                logError("Data type constraint violation for column '" + column.name +
                        "': expected " + column.type + ", got '" + value + "'");
                return false;
            }
        }

        // 3. 检查主键约束（简化：检查是否重复）
        // 注意：这里需要查询现有数据来检查唯一性
        // 暂时跳过，实际实现需要访问存储引擎

        // 4. 检查外键约束（简化）
        // 暂时跳过，需要更复杂的实现

        return true;
    } catch (const std::exception& e) {
        logError("Constraint validation failed: " + std::string(e.what()));
        return false;
    }
}

bool LoadDataExecutor::validateDataType(const std::string& value, const std::string& type) {
    // 简化的数据类型验证
    try {
        if (type.find("INT") == 0) {
            // 整数类型
            std::stoi(value);
            return true;
        } else if (type.find("VARCHAR") == 0 || type.find("CHAR") == 0 || type == "TEXT") {
            // 字符串类型 - 总是有效
            return true;
        } else if (type.find("DECIMAL") == 0 || type.find("NUMERIC") == 0) {
            // 十进制类型
            std::stod(value);
            return true;
        } else if (type == "DATE" || type == "TIME" || type == "TIMESTAMP" || type == "DATETIME") {
            // 日期时间类型 - 简化检查
            // 这里应该实现更完整的日期时间验证
            return !value.empty();
        } else if (type == "BOOLEAN" || type == "BOOL") {
            // 布尔类型
            return value == "0" || value == "1" || value == "true" || value == "false" ||
                   value == "TRUE" || value == "FALSE";
        } else {
            // 未知类型，假设有效
            return true;
        }
    } catch (const std::exception&) {
        return false;
    }
}

bool LoadDataExecutor::insertRow(const std::vector<std::string>& row,
                               const sql_parser::LoadDataStatement& stmt,
                               std::shared_ptr<TableMetadata> table_meta) {
    try {
        // 使用存储引擎插入数据
        // TODO: 这里需要使用正确的StorageEngine接口
        // 目前先返回true以通过编译
        (void)stmt; // 避免未使用警告
        (void)row; // 避免未使用警告
        return true;
    } catch (const std::exception& e) {
        logError("Insert failed: " + std::string(e.what()));
        return false;
    }
}

std::shared_ptr<TableMetadata> LoadDataExecutor::getTableMetadata(const std::string& table_name) {
    try {
        // TODO: 这里需要使用正确的StorageEngine接口
        // 目前先返回nullptr以通过编译
        (void)table_name; // 避免未使用警告
        return nullptr;
    } catch (const std::exception& e) {
        logError("Failed to get table metadata: " + std::string(e.what()));
        return nullptr;
    }
}

bool LoadDataExecutor::checkTableExists(const std::string& table_name) {
    try {
        // TODO: 这里需要使用正确的StorageEngine接口
        // 目前先返回false以通过编译
        (void)table_name; // 避免未使用警告
        return false;
    } catch (const std::exception& e) {
        logError("Failed to check table existence: " + std::string(e.what()));
        return false;
    }
}

bool LoadDataExecutor::checkFilePermissions(const std::string& filename, bool is_local) {
    try {
        // 基本文件存在性检查
        std::filesystem::path file_path(filename);
        return std::filesystem::exists(file_path) && std::filesystem::is_regular_file(file_path);
    } catch (const std::exception& e) {
        logError("File permission check failed: " + std::string(e.what()));
        return false;
    }
}

void LoadDataExecutor::logWarning(const std::string& message) {
    stats_.warnings++;
    std::cout << "[LOAD DATA WARNING] " << message << std::endl;
}

void LoadDataExecutor::logError(const std::string& message) {
    std::cout << "[LOAD DATA ERROR] " << message << std::endl;
}

} // namespace sqlcc
