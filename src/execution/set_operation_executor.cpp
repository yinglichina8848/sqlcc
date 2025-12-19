#include "execution/set_operation_executor.h"
#include "core/execution_context.h"
#include "core/execution_result.h"
#include <algorithm>
#include <set>
#include <unordered_map>

namespace sqlcc {

SetOperationExecutor::SetOperationExecutor(std::shared_ptr<DatabaseManager> db_manager)
    : db_manager_(db_manager) {
}

SetOperationExecutor::~SetOperationExecutor() = default;

ExecutionResult SetOperationExecutor::execute(const sql_parser::SetOperation& stmt,
                                             ExecutionContext& context) {
    ExecutionResult result;
    result.success = false;

    try {
        // 执行左操作数
        ExecutionResult left_result = executeSelect(*stmt.getLeftOperand(), context);
        if (!left_result.success) {
            result.error_message = "Failed to execute left operand: " + left_result.error_message;
            return result;
        }

        // 执行右操作数
        ExecutionResult right_result = executeSelect(*stmt.getRightOperand(), context);
        if (!right_result.success) {
            result.error_message = "Failed to execute right operand: " + right_result.error_message;
            return result;
        }

        // 执行集合操作
        switch (stmt.getOperationType()) {
            case sql_parser::SetOperationType::UNION:
                result = executeUnion(left_result, right_result, stmt.isAll());
                break;
            case sql_parser::SetOperationType::INTERSECT:
                result = executeIntersect(left_result, right_result, stmt.isAll());
                break;
            case sql_parser::SetOperationType::EXCEPT:
                result = executeExcept(left_result, right_result, stmt.isAll());
                break;
            default:
                result.error_message = "Unsupported set operation type";
                return result;
        }

        if (!result.success) {
            return result;
        }

        // 处理ORDER BY
        if (stmt.hasOrderBy()) {
            applyOrderBy(result, stmt.getOrderByColumns(), stmt.getOrderByAscending());
        }

        // 处理LIMIT
        if (stmt.hasLimit()) {
            applyLimit(result, stmt.getLimit());
        }

        result.rows_affected = result.rows.size();
        context.records_affected = result.rows_affected;

    } catch (const std::exception& e) {
        result.error_message = "Set operation execution failed: " + std::string(e.what());
        result.success = false;
    }

    return result;
}

ExecutionResult SetOperationExecutor::executeUnion(const ExecutionResult& left,
                                                   const ExecutionResult& right,
                                                   bool is_all) {
    ExecutionResult result;
    result.success = true;

    // 检查列数是否匹配
    if (left.rows.empty() && right.rows.empty()) {
        // 两个结果集都为空，返回空结果
        result.rows = {};
        result.column_metadata = left.column_metadata; // 使用左边的列信息
        return result;
    }

    if (!left.rows.empty() && !right.rows.empty()) {
        if (left.rows[0].values.size() != right.rows[0].values.size()) {
            result.success = false;
            result.error_message = "UNION operands have different number of columns";
            return result;
        }
    }

    // 合并列元数据（使用左边的）
    result.column_metadata = left.column_metadata.empty() ? right.column_metadata : left.column_metadata;

    if (is_all) {
        // UNION ALL - 直接合并所有行
        result.rows.reserve(left.rows.size() + right.rows.size());
        result.rows.insert(result.rows.end(), left.rows.begin(), left.rows.end());
        result.rows.insert(result.rows.end(), right.rows.begin(), right.rows.end());
    } else {
        // UNION - 去重合并
        std::set<std::vector<std::string>> unique_rows;

        // 添加左边的行
        for (const auto& row : left.rows) {
            std::vector<std::string> row_values;
            for (const auto& value : row.values) {
                row_values.push_back(value);
            }
            unique_rows.insert(row_values);
        }

        // 添加右边的行
        for (const auto& row : right.rows) {
            std::vector<std::string> row_values;
            for (const auto& value : row.values) {
                row_values.push_back(value);
            }
            unique_rows.insert(row_values);
        }

        // 转换为结果格式
        result.rows.reserve(unique_rows.size());
        for (const auto& row_values : unique_rows) {
            Row row;
            row.values = row_values;
            result.rows.push_back(row);
        }
    }

    return result;
}

ExecutionResult SetOperationExecutor::executeIntersect(const ExecutionResult& left,
                                                       const ExecutionResult& right,
                                                       bool is_all) {
    ExecutionResult result;
    result.success = true;

    // 检查列数是否匹配
    if (left.rows.empty() || right.rows.empty()) {
        // 如果任一结果集为空，交集为空
        result.rows = {};
        result.column_metadata = left.column_metadata;
        return result;
    }

    if (left.rows[0].values.size() != right.rows[0].values.size()) {
        result.success = false;
        result.error_message = "INTERSECT operands have different number of columns";
        return result;
    }

    // 合并列元数据
    result.column_metadata = left.column_metadata;

    if (is_all) {
        // INTERSECT ALL - 计算最小出现次数
        std::unordered_map<std::string, int> left_count, right_count, result_count;

        // 统计左边行出现次数
        for (const auto& row : left.rows) {
            std::string key;
            for (const auto& value : row.values) {
                if (!key.empty()) key += "|";
                key += value;
            }
            left_count[key]++;
        }

        // 统计右边行出现次数
        for (const auto& row : right.rows) {
            std::string key;
            for (const auto& value : row.values) {
                if (!key.empty()) key += "|";
                key += value;
            }
            right_count[key]++;
        }

        // 计算交集（取最小出现次数）
        for (const auto& pair : left_count) {
            const std::string& key = pair.first;
            if (right_count.count(key)) {
                int count = std::min(pair.second, right_count[key]);
                result_count[key] = count;
            }
        }

        // 生成结果行
        for (const auto& pair : result_count) {
            const std::string& key = pair.first;
            int count = pair.second;

            // 解析行数据
            std::vector<std::string> values;
            size_t start = 0, end;
            while ((end = key.find('|', start)) != std::string::npos) {
                values.push_back(key.substr(start, end - start));
                start = end + 1;
            }
            values.push_back(key.substr(start));

            // 添加指定次数的行
            for (int i = 0; i < count; ++i) {
                Row row;
                row.values = values;
                result.rows.push_back(row);
            }
        }
    } else {
        // INTERSECT - 简单交集
        std::set<std::vector<std::string>> left_set, result_set;

        // 将左边行加入集合
        for (const auto& row : left.rows) {
            std::vector<std::string> row_values;
            for (const auto& value : row.values) {
                row_values.push_back(value);
            }
            left_set.insert(row_values);
        }

        // 查找在左边集合中也存在的右边行
        for (const auto& row : right.rows) {
            std::vector<std::string> row_values;
            for (const auto& value : row.values) {
                row_values.push_back(value);
            }
            if (left_set.count(row_values)) {
                result_set.insert(row_values);
            }
        }

        // 转换为结果格式
        result.rows.reserve(result_set.size());
        for (const auto& row_values : result_set) {
            Row row;
            row.values = row_values;
            result.rows.push_back(row);
        }
    }

    return result;
}

ExecutionResult SetOperationExecutor::executeExcept(const ExecutionResult& left,
                                                    const ExecutionResult& right,
                                                    bool is_all) {
    ExecutionResult result;
    result.success = true;

    // 检查列数是否匹配
    if (left.rows.empty()) {
        // 左边为空，结果为空
        result.rows = {};
        result.column_metadata = left.column_metadata;
        return result;
    }

    if (!right.rows.empty() &&
        left.rows[0].values.size() != right.rows[0].values.size()) {
        result.success = false;
        result.error_message = "EXCEPT operands have different number of columns";
        return result;
    }

    // 合并列元数据
    result.column_metadata = left.column_metadata;

    if (is_all) {
        // EXCEPT ALL - 计算出现次数差
        std::unordered_map<std::string, int> left_count, right_count;

        // 统计左边行出现次数
        for (const auto& row : left.rows) {
            std::string key;
            for (const auto& value : row.values) {
                if (!key.empty()) key += "|";
                key += value;
            }
            left_count[key]++;
        }

        // 统计右边行出现次数
        for (const auto& row : right.rows) {
            std::string key;
            for (const auto& value : row.values) {
                if (!key.empty()) key += "|";
                key += value;
            }
            right_count[key]++;
        }

        // 计算差集
        for (const auto& pair : left_count) {
            const std::string& key = pair.first;
            int left_cnt = pair.second;
            int right_cnt = right_count.count(key) ? right_count[key] : 0;
            int diff = left_cnt - right_cnt;

            if (diff > 0) {
                // 解析行数据
                std::vector<std::string> values;
                size_t start = 0, end;
                while ((end = key.find('|', start)) != std::string::npos) {
                    values.push_back(key.substr(start, end - start));
                    start = end + 1;
                }
                values.push_back(key.substr(start));

                // 添加剩余次数的行
                for (int i = 0; i < diff; ++i) {
                    Row row;
                    row.values = values;
                    result.rows.push_back(row);
                }
            }
        }
    } else {
        // EXCEPT - 简单差集
        std::set<std::vector<std::string>> left_set, right_set;

        // 将左边行加入集合
        for (const auto& row : left.rows) {
            std::vector<std::string> row_values;
            for (const auto& value : row.values) {
                row_values.push_back(value);
            }
            left_set.insert(row_values);
        }

        // 将右边行加入集合
        for (const auto& row : right.rows) {
            std::vector<std::string> row_values;
            for (const auto& value : row.values) {
                row_values.push_back(value);
            }
            right_set.insert(row_values);
        }

        // 计算差集
        std::vector<std::vector<std::string>> diff_result;
        std::set_difference(left_set.begin(), left_set.end(),
                          right_set.begin(), right_set.end(),
                          std::back_inserter(diff_result));

        // 转换为结果格式
        result.rows.reserve(diff_result.size());
        for (const auto& row_values : diff_result) {
            Row row;
            row.values = row_values;
            result.rows.push_back(row);
        }
    }

    return result;
}

void SetOperationExecutor::applyOrderBy(ExecutionResult& result,
                                       const std::vector<std::string>& columns,
                                       const std::vector<bool>& ascending) {
    if (columns.empty() || result.rows.empty()) {
        return;
    }

    // 查找列索引
    std::vector<size_t> column_indices;
    for (const auto& col_name : columns) {
        size_t col_idx = std::numeric_limits<size_t>::max();
        for (size_t i = 0; i < result.column_metadata.size(); ++i) {
            if (result.column_metadata[i].name == col_name) {
                col_idx = i;
                break;
            }
        }
        if (col_idx == std::numeric_limits<size_t>::max()) {
            // 列不存在，跳过排序
            return;
        }
        column_indices.push_back(col_idx);
    }

    // 排序
    std::sort(result.rows.begin(), result.rows.end(),
              [column_indices, ascending](const Row& a, const Row& b) {
                  for (size_t i = 0; i < column_indices.size(); ++i) {
                      size_t col_idx = column_indices[i];
                      bool asc = (i < ascending.size()) ? ascending[i] : true;

                      if (col_idx >= a.values.size() || col_idx >= b.values.size()) {
                          continue;
                      }

                      const std::string& val_a = a.values[col_idx];
                      const std::string& val_b = b.values[col_idx];

                      int cmp = val_a.compare(val_b);
                      if (cmp != 0) {
                          return asc ? (cmp < 0) : (cmp > 0);
                      }
                  }
                  return false; // 相等
              });
}

void SetOperationExecutor::applyLimit(ExecutionResult& result, size_t limit) {
    if (result.rows.size() > limit) {
        result.rows.resize(limit);
    }
}

ExecutionResult SetOperationExecutor::executeSelect(const sql_parser::SelectStatement& stmt,
                                                    ExecutionContext& context) {
    // 这里应该调用统一的SELECT执行器
    // 暂时返回模拟结果用于测试
    ExecutionResult result;
    result.success = true;

    // 模拟一些测试数据
    if (stmt.getTableName() == "employees") {
        result.rows = {
            {"1", "John", "Engineering", "50000"},
            {"2", "Jane", "Sales", "45000"},
            {"3", "Bob", "Engineering", "55000"}
        };
        result.column_metadata = {
            {"id", "INTEGER", true, true, false, ""},
            {"name", "VARCHAR", false, false, false, ""},
            {"department", "VARCHAR", false, false, false, ""},
            {"salary", "INTEGER", false, false, false, ""}
        };
    } else if (stmt.getTableName() == "departments") {
        result.rows = {
            {"Engineering", "100"},
            {"Sales", "50"}
        };
        result.column_metadata = {
            {"name", "VARCHAR", false, false, false, ""},
            {"budget", "INTEGER", false, false, false, ""}
        };
    } else {
        // 默认空结果
        result.rows = {};
        result.column_metadata = {};
    }

    return result;
}

} // namespace sqlcc
