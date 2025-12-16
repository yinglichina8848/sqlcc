#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include "sql_parser/ast_nodes.h"
#include "storage/table_storage.h"

namespace sqlcc {
namespace execution {

/**
 * @brief JOIN结果行
 */
struct JoinResultRow {
  std::vector<std::string> left_row;
  std::vector<std::string> right_row;
  bool is_null_row;  // 用于LEFT/RIGHT JOIN的NULL填充

  JoinResultRow() : is_null_row(false) {}
  JoinResultRow(const std::vector<std::string>& left, const std::vector<std::string>& right)
      : left_row(left), right_row(right), is_null_row(false) {}
};

/**
 * @brief JOIN条件评估器
 */
class JoinConditionEvaluator {
public:
  /**
   * @brief 构造函数
   * @param left_columns 左表列名映射
   * @param right_columns 右表列名映射
   * @param condition JOIN条件表达式
   */
  JoinConditionEvaluator(
      const std::unordered_map<std::string, size_t>& left_columns,
      const std::unordered_map<std::string, size_t>& right_columns,
      std::unique_ptr<sql_parser::Expression> condition);

  /**
   * @brief 评估JOIN条件
   * @param left_row 左表行数据
   * @param right_row 右表行数据
   * @return 条件是否满足
   */
  bool evaluate(const std::vector<std::string>& left_row,
                const std::vector<std::string>& right_row) const;

private:
  std::unordered_map<std::string, size_t> left_columns_;
  std::unordered_map<std::string, size_t> right_columns_;
  std::unique_ptr<sql_parser::Expression> condition_;

  // 简化条件评估（实际应使用完整的表达式求值器）
  bool evaluateSimpleCondition(const std::vector<std::string>& left_row,
                              const std::vector<std::string>& right_row) const;
};

/**
 * @brief JOIN算法接口
 */
class JoinAlgorithm {
public:
  virtual ~JoinAlgorithm() = default;

  /**
   * @brief 执行JOIN操作
   * @param left_table 左表数据
   * @param right_table 右表数据
   * @param join_type JOIN类型
   * @param condition JOIN条件
   * @return JOIN结果
   */
  virtual std::vector<JoinResultRow> execute(
      const std::vector<std::vector<std::string>>& left_table,
      const std::vector<std::vector<std::string>>& right_table,
      sql_parser::JoinClause::JoinType join_type,
      const JoinConditionEvaluator* condition) = 0;

  /**
   * @brief 获取算法名称
   */
  virtual std::string getAlgorithmName() const = 0;

  /**
   * @brief 估算执行成本
   */
  virtual double estimateCost(size_t left_rows, size_t right_rows) const = 0;
};

/**
 * @brief Nested Loop JOIN算法
 */
class NestedLoopJoin : public JoinAlgorithm {
public:
  std::vector<JoinResultRow> execute(
      const std::vector<std::vector<std::string>>& left_table,
      const std::vector<std::vector<std::string>>& right_table,
      sql_parser::JoinClause::JoinType join_type,
      const JoinConditionEvaluator* condition) override;

  std::string getAlgorithmName() const override { return "Nested Loop Join"; }

  double estimateCost(size_t left_rows, size_t right_rows) const override {
    return static_cast<double>(left_rows) * right_rows;
  }
};

/**
 * @brief Hash JOIN算法
 */
class HashJoin : public JoinAlgorithm {
public:
  std::vector<JoinResultRow> execute(
      const std::vector<std::vector<std::string>>& left_table,
      const std::vector<std::vector<std::string>>& right_table,
      sql_parser::JoinClause::JoinType join_type,
      const JoinConditionEvaluator* condition) override;

  std::string getAlgorithmName() const override { return "Hash Join"; }

  double estimateCost(size_t left_rows, size_t right_rows) const override {
    return static_cast<double>(left_rows) + right_rows;
  }

private:
  /**
   * @brief 构建哈希表
   */
  std::unordered_multimap<std::string, size_t> buildHashTable(
      const std::vector<std::vector<std::string>>& table,
      const std::function<std::string(const std::vector<std::string>&)>& key_func);

  /**
   * @brief 简单哈希键生成（基于第一列）
   */
  static std::string simpleHashKey(const std::vector<std::string>& row) {
    return row.empty() ? "" : row[0];
  }
};

/**
 * @brief Merge JOIN算法
 */
class MergeJoin : public JoinAlgorithm {
public:
  std::vector<JoinResultRow> execute(
      const std::vector<std::vector<std::string>>& left_table,
      const std::vector<std::vector<std::string>>& right_table,
      sql_parser::JoinClause::JoinType join_type,
      const JoinConditionEvaluator* condition) override;

  std::string getAlgorithmName() const override { return "Merge Join"; }

  double estimateCost(size_t left_rows, size_t right_rows) const override {
    return static_cast<double>(left_rows) + right_rows;
  }

private:
  /**
   * @brief 排序表数据（假设按第一列排序）
   */
  static std::vector<std::vector<std::string>> sortTable(
      const std::vector<std::vector<std::string>>& table);
};

/**
 * @brief JOIN执行器
 */
class JoinExecutor {
public:
  /**
   * @brief 构造函数
   */
  JoinExecutor();

  /**
   * @brief 执行JOIN操作
   * @param left_table 左表数据
   * @param right_table 右表数据
   * @param join_clause JOIN子句
   * @param left_columns 左表列名映射
   * @param right_columns 右表列名映射
   * @return JOIN结果
   */
  std::vector<std::vector<std::string>> executeJoin(
      const std::vector<std::vector<std::string>>& left_table,
      const std::vector<std::vector<std::string>>& right_table,
      sql_parser::JoinClause& join_clause,
      const std::unordered_map<std::string, size_t>& left_columns,
      const std::unordered_map<std::string, size_t>& right_columns);

  /**
   * @brief 选择最优JOIN算法
   * @param left_rows 左表行数
   * @param right_rows 右表行数
   * @param join_type JOIN类型
   * @return 最优算法
   */
  std::unique_ptr<JoinAlgorithm> selectOptimalAlgorithm(
      size_t left_rows, size_t right_rows, sql_parser::JoinClause::JoinType join_type);

private:
  /**
   * @brief 创建条件评估器
   */
  std::unique_ptr<JoinConditionEvaluator> createConditionEvaluator(
      const std::unordered_map<std::string, size_t>& left_columns,
      const std::unordered_map<std::string, size_t>& right_columns,
      sql_parser::JoinClause& join_clause);

  /**
   * @brief 转换JoinResultRow为最终结果行
   */
  std::vector<std::string> convertToResultRow(
      const JoinResultRow& join_row,
      const std::unordered_map<std::string, size_t>& left_columns,
      const std::unordered_map<std::string, size_t>& right_columns);

  /**
   * @brief 处理不同JOIN类型的NULL填充
   */
  std::vector<JoinResultRow> handleJoinType(
      const std::vector<JoinResultRow>& inner_results,
      const std::vector<std::vector<std::string>>& left_table,
      const std::vector<std::vector<std::string>>& right_table,
      sql_parser::JoinClause::JoinType join_type);

  /**
   * @brief 创建NULL填充行
   */
  static std::vector<std::string> createNullRow(size_t column_count) {
    return std::vector<std::string>(column_count, "NULL");
  }

  // 算法实例
  NestedLoopJoin nested_loop_join_;
  HashJoin hash_join_;
  MergeJoin merge_join_;
};

} // namespace execution
} // namespace sqlcc
