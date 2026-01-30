#ifndef SQLCC_QUERY_OPTIMIZER_H
#define SQLCC_QUERY_OPTIMIZER_H

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "../execution_context.h"
#include "../sql_parser/ast/ast_nodes.h"

namespace sqlcc {

// 前向声明ExecutionPlan类型，避免循环依赖
struct ExecutionPlan;

/**
 * @brief 查询优化器接口
 * 负责查询计划的优化
 */
class QueryOptimizer {
public:
    virtual ~QueryOptimizer() = default;

    // 优化查询计划
    virtual ExecutionPlan optimize(const ExecutionPlan& plan,
                                   const ExecutionContext& context) = 0;

    // 生成执行计划
    virtual ExecutionPlan generatePlan(const sql_parser::SelectStatement& stmt,
                                       const ExecutionContext& context) = 0;

    // 评估执行计划成本
    virtual double estimateCost(const ExecutionPlan& plan,
                                const ExecutionContext& context) = 0;

    // 获取优化规则
    virtual std::vector<std::string> getOptimizationRules() const = 0;

    // 启用/禁用特定优化规则
    virtual void enableRule(const std::string& rule_name) = 0;
    virtual void disableRule(const std::string& rule_name) = 0;

    // 检查特定优化规则是否启用
    virtual bool isRuleEnabled(const std::string& rule_name) const = 0;
};

// 前向声明ExecutionPlanGenerator，因为我们只有头文件定义
class ExecutionPlanGenerator;

/**
 * @brief 基于规则的查询优化器
 * 实现基于规则的查询优化
 */
class RuleBasedOptimizer : public QueryOptimizer {
public:
    RuleBasedOptimizer();
    ~RuleBasedOptimizer() override = default;

    // 优化查询计划
    ExecutionPlan optimize(const ExecutionPlan& plan,
                           const ExecutionContext& context) override;

    // 生成执行计划
    ExecutionPlan generatePlan(const sql_parser::SelectStatement& stmt,
                               const ExecutionContext& context) override;

    // 评估执行计划成本
    double estimateCost(const ExecutionPlan& plan,
                        const ExecutionContext& context) override;

    // 获取优化规则
    std::vector<std::string> getOptimizationRules() const override;

    // 启用/禁用特定优化规则
    void enableRule(const std::string& rule_name) override;
    void disableRule(const std::string& rule_name) override;

    // 检查特定优化规则是否启用
    bool isRuleEnabled(const std::string& rule_name) const override;

private:
    // 优化规则
    std::unordered_map<std::string, bool> optimization_rules_;

    // 执行计划生成器指针（前向声明）
    std::unique_ptr<ExecutionPlanGenerator> plan_generator_;
};

} // namespace sqlcc

#endif // SQLCC_QUERY_OPTIMIZER_H