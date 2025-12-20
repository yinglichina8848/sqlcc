#pragma once

#include <memory>
#include <vector>
#include <string>
#include "sql_parser/set_operation.h"
#include "../database_manager.h"
#include "core/execution_context.h"
#include "core/execution_result.h"

namespace sqlcc {

class SetOperationExecutor {
public:
    explicit SetOperationExecutor(std::shared_ptr<DatabaseManager> db_manager);
    ~SetOperationExecutor();

    // 执行集合操作
    ExecutionResult execute(const sql_parser::SetOperation& stmt, ExecutionContext& context);

private:
    std::shared_ptr<DatabaseManager> db_manager_;

    // 集合操作实现
    ExecutionResult executeUnion(const ExecutionResult& left, const ExecutionResult& right, bool is_all);
    ExecutionResult executeIntersect(const ExecutionResult& left, const ExecutionResult& right, bool is_all);
    ExecutionResult executeExcept(const ExecutionResult& left, const ExecutionResult& right, bool is_all);

    // 辅助函数
    void applyOrderBy(ExecutionResult& result, const std::vector<std::string>& columns, const std::vector<bool>& ascending);
    void applyLimit(ExecutionResult& result, size_t limit);

    // SELECT执行器（暂时模拟）
    ExecutionResult executeSelect(const sql_parser::SelectStatement& stmt, ExecutionContext& context);
};

} // namespace sqlcc
