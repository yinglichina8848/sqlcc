/**
 * @file group_by_executor.h
 * @brief GROUP BY执行器头文件
 */

#ifndef SQLCC_EXECUTION_GROUP_BY_EXECUTOR_H
#define SQLCC_EXECUTION_GROUP_BY_EXECUTOR_H

#include "../sql_parser/ast/ast_node.h"
#include "../sql_parser/ast/ast_nodes.h"
#include <vector>
#include <memory>
#include <unordered_map>

#include "aggregate_engine.h"
#include "../core_backup_20260121_001034/execution_result.h"

namespace sqlcc {

namespace sql_parser {
class SelectStatement;
class Expression;
} // namespace sql_parser

class ExecutionContext;

// GROUP BY执行器 - 处理分组查询
class GroupByExecutor {
public:
    GroupByExecutor();
    ~GroupByExecutor() = default;

    // 执行GROUP BY查询
    ExecutionResult executeGroupBy(const sql_parser::SelectStatement& stmt,
                                  ExecutionContext& context);

private:
    // 分组键和数据
    using GroupKey = std::vector<std::shared_ptr<void>>;
    using GroupData = std::vector<std::unordered_map<std::string, std::shared_ptr<void>>>;

    // 分组处理
    ExecutionResult processGrouping(const sql_parser::SelectStatement& stmt,
                                   ExecutionContext& context);
    ExecutionResult applyAggregatesToGroups(const sql_parser::SelectStatement& stmt,
                                           const std::unordered_map<GroupKey, GroupData>& groups,
                                           ExecutionContext& context);

    // 辅助方法
    GroupKey createGroupKey(const std::vector<sql_parser::Expression*>& group_by_exprs,
                           const std::unordered_map<std::string, std::shared_ptr<void>>& row_data);
    bool groupKeysEqual(const GroupKey& key1, const GroupKey& key2) const;
    size_t computeGroupKeyHash(const GroupKey& key) const;
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_GROUP_BY_EXECUTOR_H
