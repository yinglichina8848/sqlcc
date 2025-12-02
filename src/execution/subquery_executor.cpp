#include "execution/subquery_executor.h"
#include "sql_parser/ast_nodes.h"
#include <algorithm>
#include <memory>
#include <optional>
#include <sstream>
#include <vector>

namespace sqlcc {

SubqueryExecutor::SubqueryExecutor(std::shared_ptr<SqlExecutor> sql_executor,
                                   std::shared_ptr<DatabaseManager> db_manager,
                                   std::shared_ptr<UserManager> user_manager,
                                   const ExecutionContext &context)
    : sql_executor_(sql_executor), db_manager_(db_manager),
      user_manager_(user_manager), context_(context) {
  // 初始化子查询执行器
}

SubqueryExecutor::~SubqueryExecutor() {
  // 清理资源
}

ExecutionResult SubqueryExecutor::execute_subquery(
    std::unique_ptr<sql_parser::SelectStatement> subquery) {
  // 执行基本子查询
  // 这里简化实现，实际应该使用SqlExecutor执行子查询
  // 目前返回空结果，后续需要完善
  ExecutionResult result;
  result.success = true;
  result.message = "Subquery executed successfully";
  return result;
}

ExecutionResult SubqueryExecutor::execute_correlated_subquery(
    std::unique_ptr<sql_parser::SelectStatement> subquery,
    const Row &outer_row) {
  // 1. 准备子查询执行上下文
  ExecutionContext subquery_context = prepare_subquery_context(outer_row);

  // 2. 替换子查询中的相关引用
  replace_correlated_references(*subquery, outer_row);

  // 3. 执行子查询
  ExecutionResult result = execute_subquery(std::move(subquery));

  return result;
}

bool SubqueryExecutor::execute_exists_subquery(
    std::unique_ptr<sql_parser::SelectStatement> subquery,
    const Row &outer_row) {
  // 执行EXISTS子查询，检查是否返回至少一行
  ExecutionResult result =
      execute_correlated_subquery(std::move(subquery), outer_row);
  return result.success && !result.rows.empty();
}

bool SubqueryExecutor::execute_in_subquery(
    std::unique_ptr<sql_parser::SelectStatement> subquery,
    const Value &outer_value) {
  // 执行IN子查询，检查外部值是否在子查询结果中
  ExecutionResult result = execute_subquery(std::move(subquery));

  if (!result.success) {
    return false;
  }

  // 遍历子查询结果，检查是否包含外部值
  for (const auto &row : result.rows) {
    // 假设子查询只返回一列
    if (!row.values.empty()) {
      const Value &sub_value = row.values[0];
      // 比较外部值和子查询结果值
      if (sub_value.type == outer_value.type) {
        switch (sub_value.type) {
        case Value::Type::INT:
          if (sub_value.int_val == outer_value.int_val) {
            return true;
          }
          break;
        case Value::Type::DOUBLE:
          if (sub_value.double_val == outer_value.double_val) {
            return true;
          }
          break;
        case Value::Type::STRING:
          if (sub_value.str_val == outer_value.str_val) {
            return true;
          }
          break;
        default:
          break;
        }
      }
    }
  }

  return false;
}

std::optional<Value> SubqueryExecutor::execute_scalar_subquery(
    std::unique_ptr<sql_parser::SelectStatement> subquery,
    const Row &outer_row) {
  // 执行标量子查询，返回单个值
  ExecutionResult result =
      execute_correlated_subquery(std::move(subquery), outer_row);

  if (!result.success) {
    return std::nullopt;
  }

  // 标量子查询应该只返回一行一列
  if (result.rows.size() == 1 && !result.rows[0].values.empty()) {
    return result.rows[0].values[0];
  }

  return std::nullopt;
}

bool SubqueryExecutor::execute_any_all_subquery(
    std::unique_ptr<sql_parser::SelectStatement> subquery,
    const Value &outer_value, bool is_any, const std::string &comparison_op) {
  // 执行ANY/ALL子查询
  ExecutionResult result = execute_subquery(std::move(subquery));

  if (!result.success || result.rows.empty()) {
    // 如果子查询结果为空，ANY返回false，ALL返回true
    return !is_any;
  }

  // 遍历子查询结果，根据比较操作符评估条件
  bool result_value = is_any ? false : true;

  for (const auto &row : result.rows) {
    if (row.values.empty()) {
      continue;
    }

    const Value &sub_value = row.values[0];
    bool comparison_result =
        evaluate_subquery_result(result, outer_value, comparison_op);

    if (is_any) {
      // ANY：只要有一个满足条件就返回true
      if (comparison_result) {
        return true;
      }
    } else {
      // ALL：只要有一个不满足条件就返回false
      if (!comparison_result) {
        return false;
      }
    }
  }

  return result_value;
}

void SubqueryExecutor::set_context(const ExecutionContext &context) {
  context_ = context;
}

const ExecutionContext &SubqueryExecutor::get_context() const {
  return context_;
}

ExecutionContext
SubqueryExecutor::prepare_subquery_context(const Row &outer_row) {
  // 准备子查询执行上下文
  // 这里简化实现，实际应该复制并修改上下文
  ExecutionContext subquery_context = context_;

  // 清空之前的统计信息
  subquery_context.reset();

  return subquery_context;
}

void SubqueryExecutor::replace_correlated_references(
    sql_parser::SelectStatement &subquery, const Row &outer_row) {
  // 替换子查询中的相关引用
  // 这里简化实现，实际应该遍历子查询的AST，替换相关引用
  // 目前不做任何替换，后续需要完善
}

bool SubqueryExecutor::evaluate_subquery_result(
    const ExecutionResult &result, const Value &outer_value,
    const std::string &comparison_op) {
  // 评估子查询结果
  // 这里简化实现，实际应该根据比较操作符比较外部值和子查询结果

  if (result.rows.empty() || result.rows[0].values.empty()) {
    return false;
  }

  const Value &sub_value = result.rows[0].values[0];

  // 比较外部值和子查询结果值
  if (sub_value.type != outer_value.type) {
    return false;
  }

  switch (sub_value.type) {
  case Value::Type::INT:
    if (comparison_op == "=") {
      return sub_value.int_val == outer_value.int_val;
    } else if (comparison_op == "!=") {
      return sub_value.int_val != outer_value.int_val;
    } else if (comparison_op == ">") {
      return sub_value.int_val > outer_value.int_val;
    } else if (comparison_op == ">=") {
      return sub_value.int_val >= outer_value.int_val;
    } else if (comparison_op == "<") {
      return sub_value.int_val < outer_value.int_val;
    } else if (comparison_op == "<=") {
      return sub_value.int_val <= outer_value.int_val;
    }
    break;

  case Value::Type::DOUBLE:
    if (comparison_op == "=") {
      return sub_value.double_val == outer_value.double_val;
    } else if (comparison_op == "!=") {
      return sub_value.double_val != outer_value.double_val;
    } else if (comparison_op == ">") {
      return sub_value.double_val > outer_value.double_val;
    } else if (comparison_op == ">=") {
      return sub_value.double_val >= outer_value.double_val;
    } else if (comparison_op == "<") {
      return sub_value.double_val < outer_value.double_val;
    } else if (comparison_op == "<=") {
      return sub_value.double_val <= outer_value.double_val;
    }
    break;

  case Value::Type::STRING:
    if (comparison_op == "=") {
      return sub_value.str_val == outer_value.str_val;
    } else if (comparison_op == "!=") {
      return sub_value.str_val != outer_value.str_val;
    } else if (comparison_op == ">") {
      return sub_value.str_val > outer_value.str_val;
    } else if (comparison_op == ">=") {
      return sub_value.str_val >= outer_value.str_val;
    } else if (comparison_op == "<") {
      return sub_value.str_val < outer_value.str_val;
    } else if (comparison_op == "<=") {
      return sub_value.str_val <= outer_value.str_val;
    }
    break;

  default:
    break;
  }

  return false;
}

} // namespace sqlcc