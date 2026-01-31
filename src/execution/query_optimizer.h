#ifndef SQLCC_QUERY_OPTIMIZER_H
#define SQLCC_QUERY_OPTIMIZER_H

/**
 * WHY: 为什么需要查询优化器？
 *
 * 数据库系统接收到用户的SQL查询后，需要将高级的声明式查询转换为低级的物理执行计划。
 * 查询优化器是数据库性能的关键组件，直接影响查询响应时间。
 *
 * 原始查询执行的低效性：
 * - 用户SQL是声明式的（what to get），而非命令式的（how to get）
 * - 同一查询可能有多种执行方式，执行效率可能相差几个数量级
 * - 例如：SELECT * FROM t1 JOIN t2 ON t1.a = t2.a WHERE t1.b > 100
 *   - 可以先过滤t1再JOIN，也可以先JOIN再过滤
 *   - 可以使用嵌套循环、HASH JOIN或MERGE JOIN
 *   - 不同执行计划可能导致秒级 vs 毫秒级的性能差异
 *
 * 优化器的核心价值：
 * 1. 性能提升：自动选择最优执行路径，减少查询时间
 * 2. 资源利用：合理分配CPU、内存、I/O资源
 * 3. 自动化：无需用户了解底层存储结构
 * 4. 自适应：根据数据统计信息动态调整策略
 *
 * 查询优化的分类：
 * - 规则优化（RBO）：基于预定义规则的优化
 * - 成本优化（CBO）：基于统计信息的成本模型优化
 * - 自适应优化：根据执行时反馈动态调整
 *
 * WHAT: 查询优化器 - 执行计划的生成和优化引擎
 *
 * 核心功能：
 * - 计划生成：从AST生成初始执行计划
 * - 计划优化：通过规则和成本模型优化执行计划
 * - 成本估算：评估不同计划的执行成本
 * - 索引选择：选择最优的索引访问路径
 * - 连接排序：确定多表连接的最优顺序
 *
 * 优化规则示例：
 * - 谓词下推：将过滤条件尽可能靠近数据源
 * - 列裁剪：只读取查询需要的列
 * - 连接重排：小表先连接减少中间结果
 * - 子查询解嵌套：将相关子查询转换为JOIN
 * - 聚合优化：提前进行GROUP BY和聚合操作
 *
 * 优化器组件：
 * - 语法优化器：基于规则的等价变换
 * - 成本估算器：基于统计信息的成本计算
 * - 计划生成器：生成物理执行计划
 * - 索引选择器：选择最优索引路径
 * - 连接优化器：确定最优连接策略
 *
 * HOW: 查询优化器的实现机制
 *
 * 1. 规则优化（Rule-Based Optimization, RBO）：
 *    预定义优化规则，按照优先级顺序应用：
 *    - 规则1：投影下推 - 减少数据传输量
 *    - 规则2：谓词下推 - 减少数据读取量
 *    - 规则3：连接重排 - 小表驱动大表
 *    - 规则4：子查询解嵌套 - 避免嵌套执行
 *    - 规则5：索引选择 - 利用索引加速查询
 *
 * 2. 成本优化（Cost-Based Optimization, CBO）：
 *    基于成本模型选择最优计划：
 *    - 收集统计信息：表大小、列基数、索引选择性
 *    - 计算访问成本：I/O成本 + CPU成本
 *    - 计算连接成本：嵌套循环 vs Hash Join vs Merge Join
 *    - 选择最小成本计划
 *
 * 3. 物理计划生成：
 *    将逻辑计划转换为物理执行算子：
 *    - Table Scan → Index Scan / Table Scan
 *    - Filter → Selection
 *    - Join → Nested Loop Join / Hash Join / Merge Join
 *    - Sort → External Sort
 *    - Aggregate → Hash Aggregate / Stream Aggregate
 *
 * 4. 成本模型：
 *    成本 = I/O成本 + CPU成本 + 网络成本
 *    - I/O成本：读取页面的数量 × 页面读取时间
 *    - CPU成本：处理元组的数量 × 每元组处理时间
 *    - 网络成本：数据传输量 × 网络传输时间
 *
 * 设计模式：
 * - 策略模式：不同优化策略可替换
 * - 模板方法：优化流程的框架固定，具体步骤可扩展
 * - 责任链：优化规则按链式应用
 * - 工厂模式：创建不同类型的执行计划
 */

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "../core/execution_context.h"
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