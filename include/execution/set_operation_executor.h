#ifndef SQLCC_SET_OPERATION_EXECUTOR_H
#define SQLCC_SET_OPERATION_EXECUTOR_H

#include "execution_result.h"
#include "sql_parser/set_operation_node.h"
#include <chrono>
#include <memory>
#include <optional>

namespace sqlcc {

// 前向声明
class SqlExecutor;

// 使用命名空间别名简化代码
using sql_parser::SelectStatement;
using sql_parser::SetOperationNode;
using sql_parser::SetOperationType;

// 执行统计信息
struct ExecutionStats {
  size_t rows_processed = 0;
  size_t memory_used = 0;
  std::chrono::milliseconds total_execution_time{};
  std::chrono::milliseconds left_execution_time{};
  std::chrono::milliseconds right_execution_time{};
  std::chrono::milliseconds operation_execution_time{};
  bool has_error = false;
  std::optional<std::string> error_message;
};

// 行键结构（用于去重操作）
struct RowKey {
  std::vector<Value> values;

  bool operator==(const RowKey &other) const { return values == other.values; }

  struct Hash {
    size_t operator()(const RowKey &key) const;
  };
};

// 集合操作执行器
class SetOperationExecutor {
public:
  // 构造函数
  explicit SetOperationExecutor(std::shared_ptr<SqlExecutor> sql_executor);

  // 禁止拷贝和移动
  SetOperationExecutor(const SetOperationExecutor &) = delete;
  SetOperationExecutor &operator=(const SetOperationExecutor &) = delete;
  SetOperationExecutor(SetOperationExecutor &&) = delete;
  SetOperationExecutor &operator=(SetOperationExecutor &&) = delete;

  // 执行集合操作
  ExecutionResult execute(const SetOperationNode &operation);

  // 设置内存限制（字节）
  void set_memory_limit(size_t limit_bytes);

  // 获取执行统计信息
  ExecutionStats get_stats() const;

private:
  // 执行UNION操作
  ExecutionResult execute_union(const SetOperationNode &operation,
                                const ExecutionResult &left,
                                const ExecutionResult &right);

  // 执行INTERSECT操作
  ExecutionResult execute_intersect(const SetOperationNode &operation,
                                    const ExecutionResult &left,
                                    const ExecutionResult &right);

  // 执行EXCEPT操作
  ExecutionResult execute_except(const SetOperationNode &operation,
                                 const ExecutionResult &left,
                                 const ExecutionResult &right);

  // 执行子查询
  ExecutionResult execute_subquery(SelectStatement *subquery);

  // 验证结果集兼容性
  bool validate_result_compatibility(const ExecutionResult &left,
                                     const ExecutionResult &right);

  std::shared_ptr<SqlExecutor> sql_executor_;
  size_t memory_limit_;
  ExecutionStats stats_;
};

// 结果集合并器（静态工具类）
class ResultSetCombiner {
public:
  // 禁止实例化
  ResultSetCombiner() = delete;

  // 合并两个结果集（UNION ALL）
  static ExecutionResult union_all(const ExecutionResult &left,
                                   const ExecutionResult &right);

  // 合并两个结果集并去重（UNION）
  static ExecutionResult union_distinct(const ExecutionResult &left,
                                        const ExecutionResult &right);

  // 求交集（INTERSECT）
  static ExecutionResult intersect(const ExecutionResult &left,
                                   const ExecutionResult &right, bool all);

  // 求差集（EXCEPT）
  static ExecutionResult except(const ExecutionResult &left,
                                const ExecutionResult &right, bool all);

  // 流式处理支持
  class StreamingProcessor {
  public:
    virtual ~StreamingProcessor() = default;

    // 处理单行数据
    virtual void process_row(const Row &row) = 0;

    // 获取最终结果
    virtual ExecutionResult get_result() = 0;
  };

  // 创建流式处理器
  // 暂时注释掉，因为当前没有实现且未使用
  // static std::unique_ptr<StreamingProcessor>
  // create_streaming_processor(SetOperationType operation_type, bool all);

private:
  // 生成行键（用于去重操作）
  static RowKey
  generate_row_key(const Row &row,
                   const std::vector<ColumnMeta> &column_metadata);
};

// 集合操作专用异常
class SetOperationException : public std::runtime_error {
public:
  explicit SetOperationException(const std::string &message)
      : std::runtime_error("SetOperationException: " + message) {}
};

class IncompatibleResultException : public SetOperationException {
public:
  explicit IncompatibleResultException(const std::string &message)
      : SetOperationException("IncompatibleResult: " + message) {}
};

class MemoryLimitExceededException : public SetOperationException {
public:
  explicit MemoryLimitExceededException(const std::string &message)
      : SetOperationException("MemoryLimitExceeded: " + message) {}
};

class InvalidOperationException : public SetOperationException {
public:
  explicit InvalidOperationException(const std::string &message)
      : SetOperationException("InvalidOperation: " + message) {}
};

class UnsupportedOperationException : public SetOperationException {
public:
  explicit UnsupportedOperationException(const std::string &message)
      : SetOperationException("UnsupportedOperation: " + message) {}
};

// 辅助函数声明
void hash_combine(std::size_t &seed, std::size_t value);

} // namespace sqlcc

#endif // SQLCC_SET_OPERATION_EXECUTOR_H