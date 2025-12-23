#pragma once

#include "execution_ast/ast_interface.h"
#include "core/database_manager.h"
#include "core/user_manager.h"
#include "sql_executor.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace sqlcc {

class SubqueryExecutor {
public:
  // 构造函数
  SubqueryExecutor(std::shared_ptr<SqlExecutor> sql_executor,
                   std::shared_ptr<DatabaseManager> db_manager,
                   std::shared_ptr<UserManager> user_manager,
                   const ExecutionContext &context);

  // 析构函数
  ~SubqueryExecutor();

  // 执行基本子查询
  ExecutionResult execute_subquery(
      std::unique_ptr<sql_parser::SelectStatement> subquery);

  // 执行相关子查询
  ExecutionResult execute_correlated_subquery(
      std::unique_ptr<sql_parser::SelectStatement> subquery,
      const Row &outer_row);

  // 执行EXISTS子查询
  bool execute_exists_subquery(
      std::unique_ptr<sql_parser::SelectStatement> subquery,
      const Row &outer_row);

  // 执行IN子查询
  bool execute_in_subquery(
      std::unique_ptr<sql_parser::SelectStatement> subquery,
      const Value &outer_value);

  // 执行标量子查询
  std::optional<Value> execute_scalar_subquery(
      std::unique_ptr<sql_parser::SelectStatement> subquery,
      const Row &outer_row);

  // 执行ANY/ALL子查询
  bool execute_any_all_subquery(
      std::unique_ptr<sql_parser::SelectStatement> subquery,
      const Value &outer_value, bool is_any, const std::string &comparison_op);

  // 设置执行上下文
  void set_context(const ExecutionContext &context);

  // 获取执行上下文
  const ExecutionContext &get_context() const;

private:
  // 准备子查询执行上下文
  ExecutionContext prepare_subquery_context(const Row &outer_row);

  // 替换相关引用
  void replace_correlated_references(
      sql_parser::SelectStatement &subquery, const Row &outer_row);

  // 评估子查询结果
  bool evaluate_subquery_result(
      const ExecutionResult &result, const Value &outer_value,
      const std::string &comparison_op);

private:
  std::shared_ptr<SqlExecutor> sql_executor_;
  std::shared_ptr<DatabaseManager> db_manager_;
  std::shared_ptr<UserManager> user_manager_;
  ExecutionContext context_;
};

} // namespace sqlcc
