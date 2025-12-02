#ifndef SQLCC_JOIN_EXECUTOR_H
#define SQLCC_JOIN_EXECUTOR_H

#include "execution_result.h"
#include "sql_parser/ast_nodes.h"
#include <chrono>
#include <memory>
#include <optional>
#include <string>

namespace sqlcc {

// 前向声明
class SqlExecutor;

/**
 * @brief JOIN类型枚举
 */
enum class JoinType {
  INNER_JOIN,
  LEFT_JOIN,
  RIGHT_JOIN,
  FULL_JOIN,
  CROSS_JOIN,
  NATURAL_JOIN
};

/**
 * @brief JOIN执行统计信息
 */
struct JoinExecutionStats {
  size_t left_rows = 0;
  size_t right_rows = 0;
  size_t rows_processed = 0;
  size_t result_rows = 0;
  std::chrono::milliseconds execution_time{};
  std::chrono::milliseconds left_scan_time{};
  std::chrono::milliseconds right_scan_time{};
  std::chrono::milliseconds join_time{};
  bool has_error = false;
  std::optional<std::string> error_message;
};

/**
 * @brief JOIN执行器类
 * 实现各种JOIN算法，包括Nested Loop JOIN、Hash JOIN等
 */
class JoinExecutor {
public:
  /**
   * @brief 构造函数
   * @param sql_executor SQL执行器实例
   */
  explicit JoinExecutor(std::shared_ptr<SqlExecutor> sql_executor);

  /**
   * @brief 禁止拷贝和移动
   */
  JoinExecutor(const JoinExecutor &) = delete;
  JoinExecutor &operator=(const JoinExecutor &) = delete;
  JoinExecutor(JoinExecutor &&) = delete;
  JoinExecutor &operator=(JoinExecutor &&) = delete;

  /**
   * @brief 执行JOIN操作
   * @param left_result 左表结果
   * @param right_result 右表结果
   * @param join_type JOIN类型
   * @param join_condition JOIN条件
   * @return JOIN后的结果
   */
  ExecutionResult execute(const ExecutionResult &left_result,
                          const ExecutionResult &right_result,
                          JoinType join_type,
                          const std::string &join_condition);

  /**
   * @brief 获取执行统计信息
   * @return 执行统计信息
   */
  JoinExecutionStats get_stats() const;

private:
  /**
   * @brief 执行Nested Loop JOIN算法
   * @param left_result 左表结果
   * @param right_result 右表结果
   * @param join_type JOIN类型
   * @param join_condition JOIN条件
   * @return JOIN后的结果
   */
  ExecutionResult execute_nested_loop_join(const ExecutionResult &left_result,
                                           const ExecutionResult &right_result,
                                           JoinType join_type,
                                           const std::string &join_condition);

  /**
   * @brief 检查两个行是否满足JOIN条件
   * @param left_row 左表行
   * @param right_row 右表行
   * @param left_meta 左表元数据
   * @param right_meta 右表元数据
   * @param join_condition JOIN条件
   * @return 是否满足条件
   */
  bool match_join_condition(const Row &left_row, const Row &right_row,
                            const std::vector<ColumnMeta> &left_meta,
                            const std::vector<ColumnMeta> &right_meta,
                            const std::string &join_condition);

  /**
   * @brief 合并两个行
   * @param left_row 左表行
   * @param right_row 右表行
   * @param join_type JOIN类型
   * @return 合并后的行
   */
  Row merge_rows(const Row &left_row, const Row &right_row, JoinType join_type);

  /**
   * @brief 合并列元数据
   * @param left_meta 左表元数据
   * @param right_meta 右表元数据
   * @return 合并后的列元数据
   */
  std::vector<ColumnMeta>
  merge_column_metadata(const std::vector<ColumnMeta> &left_meta,
                        const std::vector<ColumnMeta> &right_meta);

  std::shared_ptr<SqlExecutor> sql_executor_;
  JoinExecutionStats stats_;
};

} // namespace sqlcc

#endif // SQLCC_JOIN_EXECUTOR_H