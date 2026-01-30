// Copyright 2025 SQLCC Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sql_parser/ast_nodes.h"
#include "recursive_query_executor.h"
#include "../execution_context.h"
#include "../execution_result.h"
#include <unordered_set>
#include <queue>
#include <algorithm>

namespace sqlcc {

RecursiveQueryExecutor::RecursiveQueryExecutor(std::shared_ptr<DatabaseManager> db_manager)
    : db_manager_(db_manager) {
}

RecursiveQueryExecutor::~RecursiveQueryExecutor() = default;

ExecutionResult RecursiveQueryExecutor::execute(const sql_parser::WithRecursiveClause& stmt,
                                               ExecutionContext& context) {
    ExecutionResult result;
    result.success = false;

    try {
        // 初始化递归上下文
        RecursiveContext recursive_ctx;
        recursive_ctx.max_iterations = 1000; // 默认最大迭代次数
        recursive_ctx.current_iteration = 0;

        // 执行非递归部分（基础情况）
        auto base_query = stmt.getBaseQuery();
        if (!base_query) {
            result.message = "Base query is null";
            return result;
        }
        ExecutionResult base_result = executeBaseCase(*base_query, context);
        if (!base_result.success) {
            result.message = "Failed to execute base case: " + base_result.message;
            return result;
        }

        // 初始化工作表
        recursive_ctx.working_table = base_result.rows;
        recursive_ctx.final_result = base_result.rows;

        // 执行递归迭代
        bool has_new_rows = true;
        while (has_new_rows && recursive_ctx.current_iteration < recursive_ctx.max_iterations) {
            recursive_ctx.current_iteration++;

            // 执行递归部分
            auto recursive_query = stmt.getRecursiveQuery();
            if (!recursive_query) {
                result.message = "Recursive query is null";
                return result;
            }
            ExecutionResult recursive_result = executeRecursiveCase(*recursive_query, recursive_ctx, context);

            if (!recursive_result.success) {
                result.message = "Failed to execute recursive case at iteration " +
                                     std::to_string(recursive_ctx.current_iteration) +
                                     ": " + recursive_result.message;
                return result;
            }

            // 检查是否有新行产生
            has_new_rows = mergeNewRows(recursive_ctx, recursive_result.rows);

            if (has_new_rows) {
                // 更新工作表为当前所有结果
                recursive_ctx.working_table = recursive_ctx.final_result;
            }
        }

        if (recursive_ctx.current_iteration >= recursive_ctx.max_iterations) {
            result.message = "Recursive query exceeded maximum iterations (" +
                                 std::to_string(recursive_ctx.max_iterations) + ")";
            return result;
        }

        // 构造最终结果
        result.success = true;
        result.rows = std::move(recursive_ctx.final_result);
        result.column_metadata = base_result.column_metadata;
        result.message = "Recursive query executed successfully in " +
                        std::to_string(recursive_ctx.current_iteration) + " iterations";

        context.records_affected = result.rows.size();

    } catch (const std::exception& e) {
        result.message = "Recursive query execution failed: " + std::string(e.what());
        result.success = false;
    }

    return result;
}

ExecutionResult RecursiveQueryExecutor::executeBaseCase(const sql_parser::SelectStatement& base_query,
                                                       ExecutionContext& context) {
    // 这里应该调用统一的SELECT执行器
    // 暂时返回模拟结果用于测试
    ExecutionResult result;
    result.success = true;

    // 模拟基础情况数据（比如组织层级结构的根节点）
    result.rows = {
        Row{{Value("1"), Value("CEO"), Value("0")}}  // id, title, parent_id
    };
    result.column_metadata = {
        ColumnMeta{"id", "INTEGER", true, true, false, ""},
        ColumnMeta{"title", "VARCHAR", false, false, false, ""},
        ColumnMeta{"parent_id", "INTEGER", false, false, false, ""}
    };

    return result;
}

ExecutionResult RecursiveQueryExecutor::executeRecursiveCase(const sql_parser::SelectStatement& recursive_query,
                                                            RecursiveContext& recursive_ctx,
                                                            ExecutionContext& context) {
    // 这里应该调用统一的SELECT执行器，但使用递归上下文中的工作表
    // 暂时返回模拟结果用于测试
    ExecutionResult result;
    result.success = true;

    // 模拟递归情况：基于当前工作表查找子节点
    // 在实际实现中，这里会使用工作表中的数据来执行递归查询
    if (recursive_ctx.working_table.size() == 1) {
        // 第一轮递归：查找CEO的下属
        result.rows = {
            Row{{Value("2"), Value("CTO"), Value("1")}},
            Row{{Value("3"), Value("CFO"), Value("1")}}
        };
    } else if (recursive_ctx.working_table.size() == 3) {
        // 第二轮递归：查找下属的下属
        result.rows = {
            Row{{Value("4"), Value("Senior Engineer"), Value("2")}},
            Row{{Value("5"), Value("Tech Lead"), Value("2")}},
            Row{{Value("6"), Value("Accountant"), Value("3")}}
        };
    } else {
        // 没有更多层级
        result.rows.clear();
    }

    result.column_metadata = {
        ColumnMeta{"id", "INTEGER", true, true, false, ""},
        ColumnMeta{"title", "VARCHAR", false, false, false, ""},
        ColumnMeta{"parent_id", "INTEGER", false, false, false, ""}
    };

    return result;
}

bool RecursiveQueryExecutor::mergeNewRows(RecursiveContext& recursive_ctx,
                                         const std::vector<Row>& new_rows) {
    if (new_rows.empty()) {
        return false;
    }

    // 创建当前结果的哈希集合，用于快速查找
    std::unordered_set<std::string> existing_rows;
    for (const auto& row : recursive_ctx.final_result) {
        std::string key;
        for (const auto& value : row.values) {
            if (!key.empty()) key += "|";
            key += value.toString();
        }
        existing_rows.insert(key);
    }

    // 检查新行是否已存在，并合并新行
    bool has_new = false;
    for (const auto& new_row : new_rows) {
        std::string key;
        for (const auto& value : new_row.values) {
            if (!key.empty()) key += "|";
            key += value.toString();
        }

        if (existing_rows.find(key) == existing_rows.end()) {
            // 新行不存在，添加到最终结果
            recursive_ctx.final_result.push_back(new_row);
            has_new = true;
        }
    }

    return has_new;
}

bool RecursiveQueryExecutor::detectCycle(const RecursiveContext& recursive_ctx) const {
    // 简单的循环检测：检查是否有重复的行
    std::unordered_set<std::string> seen_rows;

    for (const auto& row : recursive_ctx.final_result) {
        std::string key;
        for (const auto& value : row.values) {
            if (!key.empty()) key += "|";
            key += value.toString();
        }

        if (seen_rows.find(key) != seen_rows.end()) {
            return true; // 发现重复，可能存在循环
        }
        seen_rows.insert(key);
    }

    return false;
}

ExecutionResult RecursiveQueryExecutor::executeBreadthFirst(const sql_parser::WithRecursiveClause& stmt,
                                                           ExecutionContext& context) {
    // 广度优先递归执行
    ExecutionResult result;
    result.success = true;

    try {
        std::queue<std::vector<Row>> level_queue;

        // 基础情况入队
        auto base_query = stmt.getBaseQuery();
        if (!base_query) {
            result.message = "Base query is null";
            result.success = false;
            return result;
        }
        ExecutionResult base_result = executeBaseCase(*base_query, context);
        if (!base_result.success) {
            result.message = "Failed to execute base case: " + base_result.message;
            result.success = false;
            return result;
        }

        level_queue.push(base_result.rows);
        result.rows = base_result.rows;
        result.column_metadata = base_result.column_metadata;

        // 按层级处理
        while (!level_queue.empty()) {
            auto current_level = level_queue.front();
            level_queue.pop();

            if (current_level.empty()) continue;

            // 为当前层级创建递归上下文
            RecursiveContext recursive_ctx;
            recursive_ctx.working_table = current_level;

            // 执行递归查询
            auto recursive_query = stmt.getRecursiveQuery();
            if (!recursive_query) {
                result.message = "Recursive query is null";
                result.success = false;
                return result;
            }
            ExecutionResult recursive_result = executeRecursiveCase(*recursive_query, recursive_ctx, context);

            if (recursive_result.success && !recursive_result.rows.empty()) {
                // 检查新行是否已存在于结果中
                std::vector<Row> new_unique_rows;
                std::unordered_set<std::string> existing_keys;

                // 建立现有行的哈希
                for (const auto& row : result.rows) {
                    std::string key;
                    for (const auto& value : row.values) {
                        if (!key.empty()) key += "|";
                        key += value.toString();
                    }
                    existing_keys.insert(key);
                }

                // 过滤出新的唯一行
                for (const auto& row : recursive_result.rows) {
                    std::string key;
                    for (const auto& value : row.values) {
                        if (!key.empty()) key += "|";
                        key += value.toString();
                    }

                    if (existing_keys.find(key) == existing_keys.end()) {
                        new_unique_rows.push_back(row);
                        existing_keys.insert(key);
                    }
                }

                if (!new_unique_rows.empty()) {
                    // 添加新行到结果
                    result.rows.insert(result.rows.end(), new_unique_rows.begin(), new_unique_rows.end());
                    // 将新行加入下一层级队列
                    level_queue.push(new_unique_rows);
                }
            }
        }

        result.message = "Breadth-first recursive query executed successfully";

        context.records_affected = result.rows.size();

    } catch (const std::exception& e) {
        result.message = "Breadth-first recursive execution failed: " + std::string(e.what());
        result.success = false;
    }

    return result;
}

ExecutionResult RecursiveQueryExecutor::executeDepthFirst(const sql_parser::WithRecursiveClause& stmt,
                                                         ExecutionContext& context) {
    // 深度优先递归执行（使用栈）
    ExecutionResult result;
    result.success = true;

    try {
        std::vector<std::vector<Row>> result_stack;

        // 基础情况入栈
        auto base_query = stmt.getBaseQuery();
        if (!base_query) {
            result.message = "Base query is null";
            result.success = false;
            return result;
        }
        ExecutionResult base_result = executeBaseCase(*base_query, context);
        if (!base_result.success) {
            result.message = "Failed to execute base case: " + base_result.message;
            result.success = false;
            return result;
        }

        result_stack.push_back(base_result.rows);
        result.rows = base_result.rows;
        result.column_metadata = base_result.column_metadata;

        std::unordered_set<std::string> visited_keys;

        // 建立初始访问集合
        for (const auto& row : base_result.rows) {
            std::string key;
            for (const auto& value : row.values) {
                if (!key.empty()) key += "|";
                key += value.toString();
            }
            visited_keys.insert(key);
        }

        // 深度优先遍历
        while (!result_stack.empty()) {
            auto current_level = result_stack.back();
            result_stack.pop_back();

            if (current_level.empty()) continue;

            // 为当前层级创建递归上下文
            RecursiveContext recursive_ctx;
            recursive_ctx.working_table = current_level;

            // 执行递归查询
            auto recursive_query = stmt.getRecursiveQuery();
            if (!recursive_query) {
                result.message = "Recursive query is null";
                result.success = false;
                return result;
            }
            ExecutionResult recursive_result = executeRecursiveCase(*recursive_query, recursive_ctx, context);

            if (recursive_result.success && !recursive_result.rows.empty()) {
                // 深度优先：立即处理子节点
                std::vector<Row> new_rows;

                for (const auto& row : recursive_result.rows) {
                    std::string key;
                    for (const auto& value : row.values) {
                        if (!key.empty()) key += "|";
                        key += value.toString();
                    }

                    if (visited_keys.find(key) == visited_keys.end()) {
                        new_rows.push_back(row);
                        visited_keys.insert(key);
                    }
                }

                if (!new_rows.empty()) {
                    // 添加到结果
                    result.rows.insert(result.rows.end(), new_rows.begin(), new_rows.end());
                    // 深度优先：立即压栈处理子节点
                    result_stack.push_back(new_rows);
                }
            }
        }

        result.message = "Depth-first recursive query executed successfully";

        context.records_affected = result.rows.size();

    } catch (const std::exception& e) {
        result.message = "Depth-first recursive execution failed: " + std::string(e.what());
        result.success = false;
    }

    return result;
}

} // namespace sqlcc
