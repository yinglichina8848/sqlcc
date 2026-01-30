#ifndef SQLCC_EXECUTION_PLAN_GENERATOR_H
#define SQLCC_EXECUTION_PLAN_GENERATOR_H

#include <memory>
#include <string>
#include <vector>

#include "../core/execution_context.h"
#include "../sql_parser/ast/ast_nodes.h"

namespace sqlcc {

// 前向声明ExecutionPlan类型，避免循环依赖
struct ExecutionPlan;

/**
 * @brief 执行计划生成器
 * 负责生成和优化执行计划
 */
class ExecutionPlanGenerator {
public:
    ExecutionPlanGenerator();
    ~ExecutionPlanGenerator() = default;

    // 生成执行计划
    ExecutionPlan generatePlan(const sql_parser::SelectStatement& stmt,
                               const ExecutionContext& context);

    // 优化执行计划
    ExecutionPlan optimizePlan(const ExecutionPlan& plan,
                               const ExecutionContext& context);

    // 评估执行计划成本
    double estimateCost(const ExecutionPlan& plan,
                        const ExecutionContext& context);

private:
    // 生成全表扫描计划
    ExecutionPlan
    generateFullTableScanPlan(const sql_parser::SelectStatement& stmt);

    // 生成索引扫描计划
    ExecutionPlan generateIndexScanPlan(const sql_parser::SelectStatement& stmt,
                                        const ExecutionContext& context);

    // 生成索引查找计划
    ExecutionPlan generateIndexSeekPlan(const sql_parser::SelectStatement& stmt,
                                        const ExecutionContext& context);
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_PLAN_GENERATOR_H