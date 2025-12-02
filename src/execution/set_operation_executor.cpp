#include "execution/set_operation_executor.h"
#include "exception.h"
#include "sql_parser/set_operation_node.h"
#include <algorithm>
#include <memory>
#include <unordered_set>

namespace sqlcc {

// 使用命名空间别名简化代码
using sql_parser::SetOperationType;

SetOperationExecutor::SetOperationExecutor(
    std::shared_ptr<SqlExecutor> sql_executor)
    : sql_executor_(sql_executor),
      memory_limit_(1024 * 1024 * 1024) { // 默认1GB内存限制
}

ExecutionResult
SetOperationExecutor::execute(const SetOperationNode &operation) {
  // 重置执行统计
  stats_ = ExecutionStats{};

  auto start_time = std::chrono::steady_clock::now();

  try {
    // 2. 检查内存使用
    // MemoryManager::check_memory_limit(memory_limit_);

    // 3. 执行左操作数
    auto left_start = std::chrono::steady_clock::now();
    auto left_result = execute_subquery(operation.getLeftOperand());
    auto left_end = std::chrono::steady_clock::now();
    stats_.left_execution_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(left_end -
                                                              left_start);

    // 4. 执行右操作数
    auto right_start = std::chrono::steady_clock::now();
    auto right_result = execute_subquery(operation.getRightOperand());
    auto right_end = std::chrono::steady_clock::now();
    stats_.right_execution_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(right_end -
                                                              right_start);

    // 5. 验证结果集兼容性
    if (!validate_result_compatibility(left_result, right_result)) {
      throw IncompatibleResultException(
          "Incompatible result sets for set operation");
    }

    // 6. 根据操作类型执行相应的集合操作
    ExecutionResult result;
    auto operation_start = std::chrono::steady_clock::now();

    switch (operation.getOperationType()) {
    case SetOperationType::UNION:
      result = execute_union(operation, left_result, right_result);
      break;
    case SetOperationType::INTERSECT:
      result = execute_intersect(operation, left_result, right_result);
      break;
    case SetOperationType::EXCEPT:
      result = execute_except(operation, left_result, right_result);
      break;
    default:
      throw UnsupportedOperationException("Unsupported set operation type");
    }

    auto operation_end = std::chrono::steady_clock::now();
    stats_.operation_execution_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(operation_end -
                                                              operation_start);

    // 7. 更新统计信息
    auto end_time = std::chrono::steady_clock::now();
    stats_.total_execution_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                              start_time);
    stats_.rows_processed = left_result.rows.size() + right_result.rows.size();
    // stats_.memory_used = MemoryManager::get_current_usage(); //
    // 暂时注释，MemoryManager未实现

    return result;

  } catch (const std::exception &e) {
    // 记录错误信息
    stats_.has_error = true;
    stats_.error_message = e.what();
    throw;
  }
}

void SetOperationExecutor::set_memory_limit(size_t limit_bytes) {
  memory_limit_ = limit_bytes;
}

ExecutionStats SetOperationExecutor::get_stats() const { return stats_; }

ExecutionResult
SetOperationExecutor::execute_union(const SetOperationNode &operation,
                                    const ExecutionResult &left,
                                    const ExecutionResult &right) {
  if (operation.isAll()) {
    return ResultSetCombiner::union_all(left, right);
  } else {
    return ResultSetCombiner::union_distinct(left, right);
  }
}

ExecutionResult
SetOperationExecutor::execute_intersect(const SetOperationNode &operation,
                                        const ExecutionResult &left,
                                        const ExecutionResult &right) {
  return ResultSetCombiner::intersect(left, right, operation.isAll());
}

ExecutionResult
SetOperationExecutor::execute_except(const SetOperationNode &operation,
                                     const ExecutionResult &left,
                                     const ExecutionResult &right) {
  return ResultSetCombiner::except(left, right, operation.isAll());
}

ExecutionResult
SetOperationExecutor::execute_subquery(SelectStatement *subquery) {
  if (!subquery) {
    throw InvalidOperationException("Null subquery in set operation");
  }

  // 目前简化实现，直接返回空结果
  // TODO: 实现真正的子查询执行逻辑
  return ExecutionResult(true, "Subquery executed successfully");
}

bool SetOperationExecutor::validate_result_compatibility(
    const ExecutionResult &left, const ExecutionResult &right) {
  // 检查列数是否相同
  if (left.column_metadata.size() != right.column_metadata.size()) {
    return false;
  }

  // 检查每列的数据类型是否兼容
  for (size_t i = 0; i < left.column_metadata.size(); ++i) {
    const auto &left_col = left.column_metadata[i];
    const auto &right_col = right.column_metadata[i];

    // 基本类型检查（可以扩展为更复杂的类型兼容性检查）
    if (left_col.data_type != right_col.data_type) {
      return false;
    }

    // 检查列名（可选，但建议保持一致性）
    if (left_col.name != right_col.name) {
      // 这里可以记录警告，但不阻止操作执行
      // 在实际实现中可以考虑使用更灵活的策略
    }
  }

  return true;
}

// ResultSetCombiner 实现
ExecutionResult ResultSetCombiner::union_all(const ExecutionResult &left,
                                             const ExecutionResult &right) {
  ExecutionResult result;

  // 复制列元数据（使用左结果集的元数据）
  result.column_metadata = left.column_metadata;

  // 预分配内存以提高性能
  result.rows.reserve(left.rows.size() + right.rows.size());

  // 添加左结果集的所有行
  result.rows.insert(result.rows.end(), left.rows.begin(), left.rows.end());

  // 添加右结果集的所有行
  result.rows.insert(result.rows.end(), right.rows.begin(), right.rows.end());

  return result;
}

ExecutionResult
ResultSetCombiner::union_distinct(const ExecutionResult &left,
                                  const ExecutionResult &right) {
  ExecutionResult result;
  result.column_metadata = left.column_metadata;

  // 使用哈希表进行去重
  std::unordered_set<RowKey, RowKey::Hash> seen_rows;

  // 处理左结果集
  for (const auto &row : left.rows) {
    RowKey key = generate_row_key(row, left.column_metadata);
    if (seen_rows.insert(key).second) {
      result.rows.push_back(row);
    }
  }

  // 处理右结果集
  for (const auto &row : right.rows) {
    RowKey key = generate_row_key(row, right.column_metadata);
    if (seen_rows.insert(key).second) {
      result.rows.push_back(row);
    }
  }

  return result;
}

ExecutionResult ResultSetCombiner::intersect(const ExecutionResult &left,
                                             const ExecutionResult &right,
                                             bool all) {
  ExecutionResult result;
  result.column_metadata = left.column_metadata;

  if (all) {
    // INTERSECT ALL：保留重复
    std::unordered_map<RowKey, size_t, RowKey::Hash> right_counts;

    // 统计右结果集中每行的出现次数
    for (const auto &row : right.rows) {
      RowKey key = generate_row_key(row, right.column_metadata);
      right_counts[key]++;
    }

    // 处理左结果集
    for (const auto &row : left.rows) {
      RowKey key = generate_row_key(row, left.column_metadata);
      auto it = right_counts.find(key);
      if (it != right_counts.end() && it->second > 0) {
        result.rows.push_back(row);
        it->second--;
      }
    }
  } else {
    // INTERSECT DISTINCT：去重
    std::unordered_set<RowKey, RowKey::Hash> right_set;
    std::unordered_set<RowKey, RowKey::Hash> result_set;

    // 构建右结果集的集合
    for (const auto &row : right.rows) {
      RowKey key = generate_row_key(row, right.column_metadata);
      right_set.insert(key);
    }

    // 处理左结果集
    for (const auto &row : left.rows) {
      RowKey key = generate_row_key(row, left.column_metadata);
      if (right_set.count(key) > 0 && result_set.insert(key).second) {
        result.rows.push_back(row);
      }
    }
  }

  return result;
}

ExecutionResult ResultSetCombiner::except(const ExecutionResult &left,
                                          const ExecutionResult &right,
                                          bool all) {
  ExecutionResult result;
  result.column_metadata = left.column_metadata;

  if (all) {
    // EXCEPT ALL：保留重复
    std::unordered_map<RowKey, size_t, RowKey::Hash> right_counts;

    // 统计右结果集中每行的出现次数
    for (const auto &row : right.rows) {
      RowKey key = generate_row_key(row, right.column_metadata);
      right_counts[key]++;
    }

    // 处理左结果集
    for (const auto &row : left.rows) {
      RowKey key = generate_row_key(row, left.column_metadata);
      auto it = right_counts.find(key);
      if (it == right_counts.end() || it->second == 0) {
        result.rows.push_back(row);
      } else {
        it->second--;
      }
    }
  } else {
    // EXCEPT DISTINCT：去重
    std::unordered_set<RowKey, RowKey::Hash> right_set;
    std::unordered_set<RowKey, RowKey::Hash> result_set;

    // 构建右结果集的集合
    for (const auto &row : right.rows) {
      RowKey key = generate_row_key(row, right.column_metadata);
      right_set.insert(key);
    }

    // 处理左结果集
    for (const auto &row : left.rows) {
      RowKey key = generate_row_key(row, left.column_metadata);
      if (right_set.count(key) == 0 && result_set.insert(key).second) {
        result.rows.push_back(row);
      }
    }
  }

  return result;
}

RowKey ResultSetCombiner::generate_row_key(
    const Row &row, const std::vector<ColumnMeta> &column_metadata) {
  RowKey key;
  key.values.reserve(row.values.size());

  for (size_t i = 0; i < row.values.size(); ++i) {
    // 这里简化处理，实际实现中需要根据数据类型进行适当的键生成
    // 对于复杂类型（如BLOB、TEXT等）需要特殊处理
    key.values.push_back(row.values[i]);
  }

  return key;
}

// RowKey::Hash实现
size_t RowKey::Hash::operator()(const RowKey &key) const {
  size_t hash = 0;
  for (const auto &value : key.values) {
    // 根据Value类型计算哈希值
    switch (value.type) {
    case Value::Type::INT:
      hash_combine(hash, std::hash<int64_t>()(value.int_val));
      break;
    case Value::Type::DOUBLE:
      hash_combine(hash, std::hash<double>()(value.double_val));
      break;
    case Value::Type::STRING:
      hash_combine(hash, std::hash<std::string>()(value.str_val));
      break;
    }
  }
  return hash;
}

// 辅助函数：哈希组合
void hash_combine(std::size_t &seed, std::size_t value) {
  seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

} // namespace sqlcc