#ifndef SQLCC_SUBQUERY_EXECUTOR_H
#define SQLCC_SUBQUERY_EXECUTOR_H

#include "execution_context.h"
#include "execution_result.h"
#include "sql_parser/ast_nodes.h"
#include <memory>
#include <string>

namespace sqlcc {

// 前向声明
class SqlExecutor;
class DatabaseManager;
class UserManager;

/**
 * @brief 子查询执行器
 * 支持执行各种类型的子查询
 */
class SubqueryExecutor {
private:
  std::shared_ptr<SqlExecutor> sql_executor_;
  std::shared_ptr<DatabaseManager> db_manager_;
  std::shared_ptr<UserManager> user_manager_;
  ExecutionContext context_;

public:
  /**
   * @brief 构造函数
   */
  SubqueryExecutor(std::shared_ptr<SqlExecutor> sql_executor,
                   std::shared_ptr<DatabaseManager> db_manager,
                   std::shared_ptr<UserManager> user_manager,
                   const ExecutionContext &context);

  /**
   * @brief 析构函数
   */
  ~SubqueryExecutor();

  /**
   * @brief 执行子查询
   * @param subquery 子查询语句
   * @return 子查询执行结果
   */
  ExecutionResult
  execute_subquery(std::unique_ptr<sql_parser::SelectStatement> subquery);

  /**
   * @brief 执行相关子查询
   * @param subquery 子查询语句
   * @param outer_row 外部查询的当前行
   * @return 子查询执行结果
   */
  ExecutionResult execute_correlated_subquery(
      std::unique_ptr<sql_parser::SelectStatement> subquery,
      const Row &outer_row);

  /**
   * @brief 执行EXISTS子查询
   * @param subquery EXISTS子查询
   * @param outer_row 外部查询的当前行
   * @return EXISTS条件是否为真
   */
  bool
  execute_exists_subquery(std::unique_ptr<sql_parser::SelectStatement> subquery,
                          const Row &outer_row);

  /**
   * @brief 执行IN子查询
   * @param subquery IN子查询
   * @param outer_value 外部查询的值
   * @return IN条件是否为真
   */
  bool
  execute_in_subquery(std::unique_ptr<sql_parser::SelectStatement> subquery,
                      const Value &outer_value);

  /**
   * @brief 执行标量子查询
   * @param subquery 标量子查询
   * @param outer_row 外部查询的当前行
   * @return 子查询返回的标量值
   */
  std::optional<Value>
  execute_scalar_subquery(std::unique_ptr<sql_parser::SelectStatement> subquery,
                          const Row &outer_row);

  /**
   * @brief 执行ANY/ALL子查询
   * @param subquery ANY/ALL子查询
   * @param outer_value 外部查询的值
   * @param is_any 是否为ANY操作
   * @param comparison_op 比较操作符
   * @return ANY/ALL条件是否为真
   */
  bool execute_any_all_subquery(
      std::unique_ptr<sql_parser::SelectStatement> subquery,
      const Value &outer_value, bool is_any, const std::string &comparison_op);

  /**
   * @brief 设置执行上下文
   * @param context 执行上下文
   */
  void set_context(const ExecutionContext &context);

  /**
   * @brief 获取执行上下文
   * @return 执行上下文
   */
  const ExecutionContext &get_context() const;

private:
  /**
   * @brief 准备子查询执行上下文
   * @param outer_row 外部查询的当前行
   * @return 准备好的执行上下文
   */
  ExecutionContext prepare_subquery_context(const Row &outer_row);

  /**
   * @brief 替换子查询中的相关引用
   * @param subquery 子查询语句
   * @param outer_row 外部查询的当前行
   */
  void replace_correlated_references(sql_parser::SelectStatement &subquery,
                                     const Row &outer_row);

  /**
   * @brief 评估子查询结果
   * @param result 子查询执行结果
   * @param outer_value 外部查询的值
   * @param comparison_op 比较操作符
   * @return 比较结果
   */
  bool evaluate_subquery_result(const ExecutionResult &result,
                                const Value &outer_value,
                                const std::string &comparison_op);
};

} // namespace sqlcc

#endif // SQLCC_SUBQUERY_EXECUTOR_H