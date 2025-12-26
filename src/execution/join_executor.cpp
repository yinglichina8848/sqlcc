#include "sql_parser/ast_node.h"
#include "execution/join_executor.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>

namespace sqlcc {
namespace execution {

// ==================== JoinConditionEvaluator 实现 ====================

JoinConditionEvaluator::JoinConditionEvaluator(
    const std::unordered_map<std::string, size_t>& left_columns,
    const std::unordered_map<std::string, size_t>& right_columns,
    std::unique_ptr<sql_parser::Expression> condition)
    : left_columns_(left_columns), right_columns_(right_columns), condition_(std::move(condition)) {}

bool JoinConditionEvaluator::evaluate(const std::vector<std::string>& left_row,
                                     const std::vector<std::string>& right_row) const {
  if (!condition_) {
    return true; // 无条件JOIN，总是匹配
  }

  return evaluateSimpleCondition(left_row, right_row);
}

bool JoinConditionEvaluator::evaluateSimpleCondition(const std::vector<std::string>& left_row,
                                                    const std::vector<std::string>& right_row) const {
  // 简化的条件评估实现
  // 这里应该实现完整的表达式求值器，但目前使用简化的等值JOIN条件

  // 假设条件是 left_table.column = right_table.column 的形式
  // 这里只是一个简化实现，实际应该解析完整的WHERE表达式

  // 对于简化实现，我们假设第一个条件是等值连接
  // 实际实现中需要解析AST条件表达式

  return true; // 简化：总是返回true，实际应该根据条件判断
}

// ==================== NestedLoopJoin 实现 ====================

std::vector<JoinResultRow> NestedLoopJoin::execute(
    const std::vector<std::vector<std::string>>& left_table,
    const std::vector<std::vector<std::string>>& right_table,
    sql_parser::JoinClause::JoinType join_type,
    const JoinConditionEvaluator* condition) {

  std::vector<JoinResultRow> results;

  // CROSS JOIN特殊处理：生成笛卡尔积，无条件匹配
  if (join_type == sql_parser::JoinClause::CROSS_JOIN) {
    for (const auto& left_row : left_table) {
      for (const auto& right_row : right_table) {
        results.emplace_back(left_row, right_row);
      }
    }
    return results;
  }

  // Nested Loop JOIN: 对左表每行，扫描右表所有行
  for (const auto& left_row : left_table) {
    bool left_matched = false;

    for (const auto& right_row : right_table) {
      bool match = !condition || condition->evaluate(left_row, right_row);

      if (match) {
        results.emplace_back(left_row, right_row);
        left_matched = true;
      }
    }

    // 处理LEFT JOIN和FULL JOIN的未匹配行
    if (!left_matched && (join_type == sql_parser::JoinClause::LEFT_JOIN ||
                         join_type == sql_parser::JoinClause::FULL_JOIN)) {
      results.emplace_back(left_row, std::vector<std::string>(right_table.empty() ? 0 : right_table[0].size(), "NULL"));
    }
  }

  // 处理RIGHT JOIN和FULL JOIN的右表未匹配行
  if (join_type == sql_parser::JoinClause::RIGHT_JOIN ||
      join_type == sql_parser::JoinClause::FULL_JOIN) {

    for (const auto& right_row : right_table) {
      bool right_matched = false;

      for (const auto& left_row : left_table) {
        bool match = !condition || condition->evaluate(left_row, right_row);
        if (match) {
          right_matched = true;
          break;
        }
      }

      if (!right_matched) {
        results.emplace_back(std::vector<std::string>(left_table.empty() ? 0 : left_table[0].size(), "NULL"), right_row);
      }
    }
  }

  return results;
}

// ==================== HashJoin 实现 ====================

std::vector<JoinResultRow> HashJoin::execute(
    const std::vector<std::vector<std::string>>& left_table,
    const std::vector<std::vector<std::string>>& right_table,
    sql_parser::JoinClause::JoinType join_type,
    const JoinConditionEvaluator* condition) {

  std::vector<JoinResultRow> results;

  if (left_table.empty() || right_table.empty()) {
    return results;
  }

  // 选择较小的表作为构建表（通常是右表）
  const auto& build_table = right_table;
  const auto& probe_table = left_table;

  // 构建哈希表
  auto hash_table = buildHashTable(build_table, simpleHashKey);

  // 探测阶段
  for (size_t probe_idx = 0; probe_idx < probe_table.size(); ++probe_idx) {
    const auto& probe_row = probe_table[probe_idx];
    std::string probe_key = simpleHashKey(probe_row);

    auto range = hash_table.equal_range(probe_key);
    bool probe_matched = false;

    for (auto it = range.first; it != range.second; ++it) {
      const auto& build_row = build_table[it->second];
      bool match = !condition || condition->evaluate(probe_row, build_row);

      if (match) {
        // 对于INNER JOIN，将左表行放在前面
        if (join_type == sql_parser::JoinClause::INNER_JOIN ||
            join_type == sql_parser::JoinClause::LEFT_JOIN) {
          results.emplace_back(probe_row, build_row);
        } else {
          // 对于RIGHT JOIN，调整顺序
          results.emplace_back(build_row, probe_row);
        }
        probe_matched = true;
      }
    }

    // 处理LEFT JOIN未匹配的情况
    if (!probe_matched && join_type == sql_parser::JoinClause::LEFT_JOIN) {
      results.emplace_back(probe_row, std::vector<std::string>(build_table[0].size(), "NULL"));
    }
  }

  // 处理RIGHT JOIN未匹配的情况
  if (join_type == sql_parser::JoinClause::RIGHT_JOIN ||
      join_type == sql_parser::JoinClause::FULL_JOIN) {
    // 创建一个集合来跟踪已匹配的右表行
    std::unordered_set<size_t> matched_right_indices;

    // 标记所有已匹配的右表行
    for (const auto& result : results) {
      // 找到右表行在build_table中的索引
      for (size_t i = 0; i < build_table.size(); ++i) {
        if (build_table[i] == result.right_row) {
          matched_right_indices.insert(i);
          break;
        }
      }
    }

    // 添加未匹配的右表行
    for (size_t i = 0; i < build_table.size(); ++i) {
      if (matched_right_indices.find(i) == matched_right_indices.end()) {
        results.emplace_back(std::vector<std::string>(left_table.empty() ? 0 : left_table[0].size(), "NULL"), build_table[i]);
      }
    }
  }

  // 处理FULL JOIN的去重（移除重复的NULL填充）
  if (join_type == sql_parser::JoinClause::FULL_JOIN) {
    // 由于我们在INNER JOIN阶段已经添加了匹配的行，这里只需要确保没有重复
    // 这个简化实现中，我们依赖于前面的逻辑避免重复
  }

  return results;
}

std::unordered_multimap<std::string, size_t> HashJoin::buildHashTable(
    const std::vector<std::vector<std::string>>& table,
    const std::function<std::string(const std::vector<std::string>&)>& key_func) {

  std::unordered_multimap<std::string, size_t> hash_table;

  for (size_t i = 0; i < table.size(); ++i) {
    std::string key = key_func(table[i]);
    hash_table.emplace(key, i);
  }

  return hash_table;
}

// ==================== MergeJoin 实现 ====================

std::vector<JoinResultRow> MergeJoin::execute(
    const std::vector<std::vector<std::string>>& left_table,
    const std::vector<std::vector<std::string>>& right_table,
    sql_parser::JoinClause::JoinType join_type,
    const JoinConditionEvaluator* condition) {

  std::vector<JoinResultRow> results;

  if (left_table.empty() || right_table.empty()) {
    return results;
  }

  // 排序两个表（假设按第一列排序）
  auto sorted_left = sortTable(left_table);
  auto sorted_right = sortTable(right_table);

  size_t left_idx = 0;
  size_t right_idx = 0;

  while (left_idx < sorted_left.size() && right_idx < sorted_right.size()) {
    const auto& left_row = sorted_left[left_idx];
    const auto& right_row = sorted_right[right_idx];

    std::string left_key = left_row.empty() ? "" : left_row[0];
    std::string right_key = right_row.empty() ? "" : right_row[0];

    if (left_key < right_key) {
      // 左表当前行小于右表，处理LEFT JOIN
      if (join_type == sql_parser::JoinClause::LEFT_JOIN ||
          join_type == sql_parser::JoinClause::FULL_JOIN) {
        results.emplace_back(left_row, std::vector<std::string>(right_row.size(), "NULL"));
      }
      ++left_idx;
    } else if (left_key > right_key) {
      // 右表当前行小于左表，处理RIGHT JOIN
      if (join_type == sql_parser::JoinClause::RIGHT_JOIN ||
          join_type == sql_parser::JoinClause::FULL_JOIN) {
        results.emplace_back(std::vector<std::string>(left_row.size(), "NULL"), right_row);
      }
      ++right_idx;
    } else {
      // 相等，找到所有匹配的行
      size_t left_start = left_idx;
      while (left_idx < sorted_left.size() &&
             (sorted_left[left_idx].empty() ? "" : sorted_left[left_idx][0]) == left_key) {
        ++left_idx;
      }

      size_t right_start = right_idx;
      while (right_idx < sorted_right.size() &&
             (sorted_right[right_idx].empty() ? "" : sorted_right[right_idx][0]) == right_key) {
        ++right_idx;
      }

      // 生成笛卡尔积
      for (size_t l = left_start; l < left_idx; ++l) {
        for (size_t r = right_start; r < right_idx; ++r) {
          bool match = !condition || condition->evaluate(sorted_left[l], sorted_right[r]);
          if (match) {
            results.emplace_back(sorted_left[l], sorted_right[r]);
          }
        }
      }
    }
  }

  // 处理剩余的行（LEFT JOIN的情况）
  while (left_idx < sorted_left.size()) {
    if (join_type == sql_parser::JoinClause::LEFT_JOIN ||
        join_type == sql_parser::JoinClause::FULL_JOIN) {
      results.emplace_back(sorted_left[left_idx],
                          std::vector<std::string>(sorted_right.empty() ? 0 : sorted_right[0].size(), "NULL"));
    }
    ++left_idx;
  }

  // 处理剩余的行（RIGHT JOIN的情况）
  while (right_idx < sorted_right.size()) {
    if (join_type == sql_parser::JoinClause::RIGHT_JOIN ||
        join_type == sql_parser::JoinClause::FULL_JOIN) {
      results.emplace_back(std::vector<std::string>(sorted_left.empty() ? 0 : sorted_left[0].size(), "NULL"),
                          sorted_right[right_idx]);
    }
    ++right_idx;
  }

  return results;
}

std::vector<std::vector<std::string>> MergeJoin::sortTable(
    const std::vector<std::vector<std::string>>& table) {

  auto sorted_table = table;

  // 按第一列排序
  std::sort(sorted_table.begin(), sorted_table.end(),
            [](const std::vector<std::string>& a, const std::vector<std::string>& b) {
              std::string key_a = a.empty() ? "" : a[0];
              std::string key_b = b.empty() ? "" : b[0];
              return key_a < key_b;
            });

  return sorted_table;
}

// ==================== JoinExecutor 实现 ====================

JoinExecutor::JoinExecutor() {}

std::vector<std::vector<std::string>> JoinExecutor::executeJoin(
    const std::vector<std::vector<std::string>>& left_table,
    const std::vector<std::vector<std::string>>& right_table,
    sql_parser::JoinClause& join_clause,
    const std::unordered_map<std::string, size_t>& left_columns,
    const std::unordered_map<std::string, size_t>& right_columns) {

  // 选择最优算法
  auto algorithm = selectOptimalAlgorithm(left_table.size(), right_table.size(), join_clause.getJoinType());

  // 创建条件评估器
  auto condition = createConditionEvaluator(left_columns, right_columns, join_clause);

  // 执行JOIN
  std::vector<JoinResultRow> join_results = algorithm->execute(
      left_table, right_table, join_clause.getJoinType(), condition.get());

  // 处理不同JOIN类型的NULL填充
  auto processed_results = handleJoinType(join_results, left_table, right_table, join_clause.getJoinType());

  // 转换为最终结果格式
  std::vector<std::vector<std::string>> final_results;
  for (const auto& join_row : processed_results) {
    final_results.push_back(convertToResultRow(join_row, left_columns, right_columns));
  }

  return final_results;
}

std::unique_ptr<JoinAlgorithm> JoinExecutor::selectOptimalAlgorithm(
    size_t left_rows, size_t right_rows, sql_parser::JoinClause::JoinType join_type) {

  // CROSS JOIN总是使用Nested Loop，因为需要笛卡尔积
  if (join_type == sql_parser::JoinClause::CROSS_JOIN) {
    return std::make_unique<NestedLoopJoin>();
  }

  // 简单的算法选择逻辑
  // 实际实现中应该考虑更多因素，如索引、内存等

  if (left_rows < 1000 && right_rows < 1000) {
    // 小表使用Nested Loop
    return std::make_unique<NestedLoopJoin>();
  } else if (left_rows > 10000 || right_rows > 10000) {
    // 大表使用Merge Join（假设数据是有序的）
    return std::make_unique<MergeJoin>();
  } else {
    // 中等大小表使用Hash Join
    return std::make_unique<HashJoin>();
  }
}

std::unique_ptr<JoinConditionEvaluator> JoinExecutor::createConditionEvaluator(
    const std::unordered_map<std::string, size_t>& left_columns,
    const std::unordered_map<std::string, size_t>& right_columns,
    sql_parser::JoinClause& join_clause) {

  // 从JOIN子句中提取条件表达式
  // 这里简化实现，实际应该解析完整的ON条件
  return std::make_unique<JoinConditionEvaluator>(
      left_columns, right_columns, join_clause.takeCondition());
}

std::vector<std::string> JoinExecutor::convertToResultRow(
    const JoinResultRow& join_row,
    const std::unordered_map<std::string, size_t>& left_columns,
    const std::unordered_map<std::string, size_t>& right_columns) {

  std::vector<std::string> result;

  // 合并左右表的列
  result.insert(result.end(), join_row.left_row.begin(), join_row.left_row.end());
  result.insert(result.end(), join_row.right_row.begin(), join_row.right_row.end());

  return result;
}

std::vector<JoinResultRow> JoinExecutor::handleJoinType(
    const std::vector<JoinResultRow>& inner_results,
    const std::vector<std::vector<std::string>>& left_table,
    const std::vector<std::vector<std::string>>& right_table,
    sql_parser::JoinClause::JoinType join_type) {

  std::vector<JoinResultRow> results = inner_results;

  if (join_type == sql_parser::JoinClause::LEFT_JOIN ||
      join_type == sql_parser::JoinClause::FULL_JOIN) {

    // 找出左表中未匹配的行
    for (const auto& left_row : left_table) {
      bool matched = false;
      for (const auto& result : inner_results) {
        if (result.left_row == left_row) {
          matched = true;
          break;
        }
      }

      if (!matched) {
        results.emplace_back(left_row, createNullRow(right_table.empty() ? 0 : right_table[0].size()));
      }
    }
  }

  if (join_type == sql_parser::JoinClause::RIGHT_JOIN ||
      join_type == sql_parser::JoinClause::FULL_JOIN) {

    // 找出右表中未匹配的行
    for (const auto& right_row : right_table) {
      bool matched = false;
      for (const auto& result : inner_results) {
        if (result.right_row == right_row) {
          matched = true;
          break;
        }
      }

      if (!matched) {
        results.emplace_back(createNullRow(left_table.empty() ? 0 : left_table[0].size()), right_row);
      }
    }
  }

  return results;
}

} // namespace execution
} // namespace sqlcc
