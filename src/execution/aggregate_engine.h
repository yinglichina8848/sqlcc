/**
 * @file aggregate_engine.h
 * @brief 聚合引擎头文件
 */

#ifndef SQLCC_EXECUTION_AGGREGATE_ENGINE_H
#define SQLCC_EXECUTION_AGGREGATE_ENGINE_H

#include "../sql_parser/ast/ast_node.h"
#include "../sql_parser/ast/ast_nodes.h"
#include <vector>
#include <memory>
#include <unordered_map>

#include "../core/execution_strategy.h"
#include "../../backups/core_backup_20260121_001034/execution_result.h"

namespace sqlcc {

namespace sql_parser {
class SelectStatement;
class Expression;
} // namespace sql_parser

class ExecutionContext;

// 聚合引擎 - 处理聚合函数查询
class AggregateEngine {
public:
    AggregateEngine();
    ~AggregateEngine() = default;

    // 执行聚合查询
    ExecutionResult executeAggregateQuery(const sql_parser::SelectStatement& stmt,
                                         ExecutionContext& context);

    // 聚合函数处理
    enum AggregateFunction {
        COUNT,
        SUM,
        AVG,
        MIN,
        MAX,
        COUNT_DISTINCT
    };

    // 聚合结果
    struct AggregateResult {
        AggregateFunction function;
        std::string column_name;
        std::shared_ptr<void> value; // 存储实际值
        size_t count = 0; // 对于COUNT函数
    };

private:
    // 分组和聚合
    using GroupKey = std::vector<std::shared_ptr<void>>;
    using AggregateResults = std::unordered_map<GroupKey, std::vector<AggregateResult>>;

    // 聚合处理
    ExecutionResult processGroupBy(const sql_parser::SelectStatement& stmt,
                                  ExecutionContext& context);
    ExecutionResult processSimpleAggregate(const sql_parser::SelectStatement& stmt,
                                          ExecutionContext& context);

    // 辅助方法
    std::vector<AggregateFunction> identifyAggregateFunctions(const sql_parser::SelectStatement& stmt);
    GroupKey createGroupKey(const std::vector<sql_parser::Expression*>& group_by_exprs,
                           const std::unordered_map<std::string, std::shared_ptr<void>>& row_data);
    void updateAggregateResult(AggregateResult& result, const std::shared_ptr<void>& value);
    std::shared_ptr<void> finalizeAggregateResult(const AggregateResult& result);
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_AGGREGATE_ENGINE_H
