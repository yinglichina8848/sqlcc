#include "sql_parser/ast_nodes.h"
#pragma once

#include <memory>
#include <vector>
#include <string>
#include "sql_parser/recursive_query.h"
#include "../database_manager.h"
#include "core/execution_context.h"
#include "core/execution_result.h"

namespace sqlcc {

class RecursiveQueryExecutor {
public:
    explicit RecursiveQueryExecutor(std::shared_ptr<DatabaseManager> db_manager);
    ~RecursiveQueryExecutor();

    // 执行递归查询
    ExecutionResult execute(const sql_parser::WithRecursiveClause& stmt, ExecutionContext& context);

    // 广度优先执行
    ExecutionResult executeBreadthFirst(const sql_parser::WithRecursiveClause& stmt, ExecutionContext& context);

    // 深度优先执行
    ExecutionResult executeDepthFirst(const sql_parser::WithRecursiveClause& stmt, ExecutionContext& context);

private:
    std::shared_ptr<DatabaseManager> db_manager_;

    // 递归上下文
    struct RecursiveContext {
        std::vector<Row> working_table;     // 当前迭代的工作表
        std::vector<Row> final_result;      // 最终结果集
        size_t max_iterations = 1000;       // 最大迭代次数
        size_t current_iteration = 0;       // 当前迭代次数
    };

    // 执行基础情况
    ExecutionResult executeBaseCase(const sql_parser::SelectStatement& base_query, ExecutionContext& context);

    // 执行递归情况
    ExecutionResult executeRecursiveCase(const sql_parser::SelectStatement& recursive_query,
                                       RecursiveContext& recursive_ctx,
                                       ExecutionContext& context);

    // 合并新行到结果集
    bool mergeNewRows(RecursiveContext& recursive_ctx, const std::vector<Row>& new_rows);

    // 循环检测
    bool detectCycle(const RecursiveContext& recursive_ctx) const;
};

} // namespace sqlcc
