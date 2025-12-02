#include "execution/join_executor.h"
#include "exception.h"
#include <algorithm>
#include <memory>
#include <sstream>

namespace sqlcc {

JoinExecutor::JoinExecutor(std::shared_ptr<SqlExecutor> sql_executor)
    : sql_executor_(sql_executor) {}

ExecutionResult JoinExecutor::execute(const ExecutionResult &left_result,
                                      const ExecutionResult &right_result,
                                      JoinType join_type,
                                      const std::string &join_condition) {
  // 重置统计信息
  stats_ = JoinExecutionStats{};

  auto start_time = std::chrono::steady_clock::now();

  try {
    // 记录输入行数
    stats_.left_rows = left_result.rows.size();
    stats_.right_rows = right_result.rows.size();

    // 执行JOIN操作
    // 目前只实现了Nested Loop JOIN，后续可以扩展其他算法
    ExecutionResult result = execute_nested_loop_join(
        left_result, right_result, join_type, join_condition);

    // 更新统计信息
    auto end_time = std::chrono::steady_clock::now();
    stats_.execution_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                              start_time);
    stats_.result_rows = result.rows.size();

    return result;

  } catch (const std::exception &e) {
    stats_.has_error = true;
    stats_.error_message = e.what();
    throw;
  }
}

JoinExecutionStats JoinExecutor::get_stats() const { return stats_; }

ExecutionResult JoinExecutor::execute_nested_loop_join(
    const ExecutionResult &left_result, const ExecutionResult &right_result,
    JoinType join_type, const std::string &join_condition) {
  ExecutionResult result;

  // 合并列元数据
  result.column_metadata = merge_column_metadata(left_result.column_metadata,
                                                 right_result.column_metadata);

  // 获取左右表的列数
  size_t left_col_count = left_result.column_metadata.size();
  size_t right_col_count = right_result.column_metadata.size();

  // 执行Nested Loop JOIN
  auto join_start = std::chrono::steady_clock::now();

  // 遍历左表的每一行
  for (const auto &left_row : left_result.rows) {
    bool matched = false;

    // 遍历右表的每一行
    for (const auto &right_row : right_result.rows) {
      stats_.rows_processed++;

      // 检查JOIN条件
      if (join_condition.empty() ||
          match_join_condition(left_row, right_row, left_result.column_metadata,
                               right_result.column_metadata, join_condition)) {
        // 匹配成功，合并行并添加到结果中
        result.rows.push_back(merge_rows(left_row, right_row, join_type));
        matched = true;
      }
    }

    // 处理LEFT JOIN，如果左表行没有匹配的右表行，也要添加到结果中
    if ((join_type == JoinType::LEFT_JOIN ||
         join_type == JoinType::FULL_JOIN) &&
        !matched) {
      // 右表行全为NULL
      Row right_row;
      right_row.values.resize(right_col_count);
      result.rows.push_back(merge_rows(left_row, right_row, join_type));
    }
  }

  // 处理RIGHT JOIN，如果右表行没有匹配的左表行，也要添加到结果中
  if (join_type == JoinType::RIGHT_JOIN || join_type == JoinType::FULL_JOIN) {
    for (const auto &right_row : right_result.rows) {
      bool matched = false;

      for (const auto &left_row : left_result.rows) {
        if (join_condition.empty() ||
            match_join_condition(
                left_row, right_row, left_result.column_metadata,
                right_result.column_metadata, join_condition)) {
          matched = true;
          break;
        }
      }

      if (!matched) {
        // 左表行全为NULL
        Row left_row;
        left_row.values.resize(left_col_count);
        result.rows.push_back(merge_rows(left_row, right_row, join_type));
      }
    }
  }

  auto join_end = std::chrono::steady_clock::now();
  stats_.join_time = std::chrono::duration_cast<std::chrono::milliseconds>(
      join_end - join_start);

  // 设置结果状态
  result.success = true;
  result.message = "JOIN operation completed successfully";

  return result;
}

bool JoinExecutor::match_join_condition(
    const Row &left_row, const Row &right_row,
    const std::vector<ColumnMeta> &left_meta,
    const std::vector<ColumnMeta> &right_meta,
    const std::string &join_condition) {
  // 简化实现：目前只支持简单的等式连接，如 "left.col1 = right.col2"
  // TODO: 实现更复杂的JOIN条件解析和匹配

  if (join_condition.empty()) {
    return true; // 无条件JOIN（CROSS JOIN）
  }

  // 查找等式运算符
  size_t eq_pos = join_condition.find('=');
  if (eq_pos == std::string::npos) {
    // 只支持等式连接
    throw Exception("Only equality join conditions are supported");
  }

  // 解析左右列名
  std::string left_col_str = join_condition.substr(0, eq_pos);
  std::string right_col_str = join_condition.substr(eq_pos + 1);

  // 去除空格
  auto trim = [](std::string &str) {
    str.erase(0, str.find_first_not_of(" \t"));
    str.erase(str.find_last_not_of(" \t") + 1);
  };

  trim(left_col_str);
  trim(right_col_str);

  // 解析左列的表名和列名
  size_t left_dot_pos = left_col_str.find('.');
  std::string left_table_name, left_column_name;
  if (left_dot_pos != std::string::npos) {
    left_table_name = left_col_str.substr(0, left_dot_pos);
    left_column_name = left_col_str.substr(left_dot_pos + 1);
  } else {
    left_column_name = left_col_str;
  }

  // 解析右列的表名和列名
  size_t right_dot_pos = right_col_str.find('.');
  std::string right_table_name, right_column_name;
  if (right_dot_pos != std::string::npos) {
    right_table_name = right_col_str.substr(0, right_dot_pos);
    right_column_name = right_col_str.substr(right_dot_pos + 1);
  } else {
    right_column_name = right_col_str;
  }

  // 查找左列的索引
  size_t left_col_idx = -1;
  for (size_t i = 0; i < left_meta.size(); ++i) {
    if (left_meta[i].name == left_column_name) {
      left_col_idx = i;
      break;
    }
  }

  if (left_col_idx == -1) {
    throw Exception("Left column not found: " + left_column_name);
  }

  // 查找右列的索引
  size_t right_col_idx = -1;
  for (size_t i = 0; i < right_meta.size(); ++i) {
    if (right_meta[i].name == right_column_name) {
      right_col_idx = i;
      break;
    }
  }

  if (right_col_idx == -1) {
    throw Exception("Right column not found: " + right_column_name);
  }

  // 比较左右列的值
  const Value &left_val = left_row.values[left_col_idx];
  const Value &right_val = right_row.values[right_col_idx];

  return left_val == right_val;
}

Row JoinExecutor::merge_rows(const Row &left_row, const Row &right_row,
                             JoinType join_type) {
  Row merged_row;

  // 合并左表行和右表行
  merged_row.values.reserve(left_row.values.size() + right_row.values.size());

  // 添加左表行的值
  merged_row.values.insert(merged_row.values.end(), left_row.values.begin(),
                           left_row.values.end());

  // 添加右表行的值
  merged_row.values.insert(merged_row.values.end(), right_row.values.begin(),
                           right_row.values.end());

  return merged_row;
}

std::vector<ColumnMeta>
JoinExecutor::merge_column_metadata(const std::vector<ColumnMeta> &left_meta,
                                    const std::vector<ColumnMeta> &right_meta) {
  std::vector<ColumnMeta> merged_meta;

  // 合并左表和右表的列元数据
  merged_meta.reserve(left_meta.size() + right_meta.size());

  // 添加左表的列元数据
  merged_meta.insert(merged_meta.end(), left_meta.begin(), left_meta.end());

  // 添加右表的列元数据
  merged_meta.insert(merged_meta.end(), right_meta.begin(), right_meta.end());

  return merged_meta;
}

} // namespace sqlcc