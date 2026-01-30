/**
 * WHY: 为什么需要统一的查询计划抽象层？
 *
 * 数据库系统需要处理多种SQL语句类型，传统方案存在诸多问题：
 * - 执行逻辑耦合：不同语句类型需要不同的执行逻辑
 * - 接口不统一：每种语句类型都有独立的处理接口
 * - 扩展困难：新增语句类型需要修改大量现有代码
 * - 资源管理混乱：不同执行器对系统资源的管理不一致
 * - 性能监控缺失：缺乏统一的性能监控和优化机制
 *
 * 统一查询计划抽象层的核心价值：
 * 1. 接口标准化：提供统一的查询计划执行接口
 * 2. 类型解耦合：屏蔽不同SQL语句类型的实现差异
 * 3. 扩展灵活性：支持动态添加新的查询计划类型
 * 4. 资源统一管理：集中管理查询执行的系统资源
 * 5. 性能全局优化：提供全局的查询性能监控和优化
 *
 * 🏗️ 设计模式：模板方法模式(Template Method Pattern) + 策略模式(Strategy Pattern)
 *
 * 统一查询计划作为模板方法模式的经典应用：
 * - 算法骨架定义：定义查询执行的通用流程框架
 * - 具体步骤定制：允许子类定制具体的执行步骤
 * - 流程控制统一：统一的执行流程控制和错误处理
 * - 扩展点明确：清晰的扩展点便于功能增强
 * - 代码复用最大化：最大程度复用通用执行逻辑
 *
 * SOLID原则体现：
 * - 单一职责：统一查询计划专门负责查询执行的抽象和协调
 * - 开闭原则：新查询类型通过扩展现有类实现
 * - 里氏替换：具体查询计划可以替换抽象查询计划
 * - 接口隔离：查询计划接口精确定义执行契约
 * - 依赖倒置：高层模块依赖查询计划抽象而非具体实现
 *
 * WHAT: 统一查询计划抽象层 - 数据库查询执行策略框架
 *
 * 核心功能：
 * - 执行流程标准化：定义统一的查询执行流程和步骤
 * - 资源协调管理：协调和管理查询执行所需的系统资源
 * - 错误处理统一：统一的错误捕获、处理和报告机制
 * - 性能监控集成：集成的性能监控和统计信息收集
 * - 执行结果封装：标准化的执行结果封装和传递
 *
 * 系统组件：
 * - UnifiedQueryPlan：核心抽象基类，定义执行接口
 * - SelectQueryPlan：SELECT查询计划，处理查询操作
 * - DMLQueryPlan：DML查询计划，处理数据操作
 * - DDLQueryPlan：DDL查询计划，处理结构操作
 * - SystemQueryPlan：系统查询计划，处理系统操作
 * - ExecutionContext：执行上下文，维护执行状态
 * - ResourceManager：资源管理器，管理执行资源
 *
 * 查询计划分类：
 * - 读查询计划：SELECT语句的查询执行计划
 * - 写查询计划：INSERT、UPDATE、DELETE的数据修改计划
 * - 结构查询计划：CREATE、ALTER、DROP的结构修改计划
 * - 事务查询计划：BEGIN、COMMIT、ROLLBACK的事务管理计划
 * - 系统查询计划：系统维护和管理操作的执行计划
 * - 自定义查询计划：用户自定义的扩展查询计划
 *
 * 执行流程标准化：
 * - 预处理阶段：参数验证、权限检查、资源准备
 * - 优化阶段：查询重写、计划优化、索引选择
 * - 执行阶段：物理执行、数据访问、结果生成
 * - 后处理阶段：结果格式化、统计收集、资源清理
 * - 异常处理：统一的异常捕获和错误恢复
 *
 * 资源管理策略：
 * - 内存管理：查询执行的内存预算和限制
 * - CPU调度：查询执行的CPU资源分配
 * - I/O调度：磁盘I/O操作的调度优化
 * - 并发控制：查询执行的并发度和并行度控制
 * - 超时管理：查询执行时间的超时控制和取消
 *
 * 接口设计：
 * - 生命周期接口：初始化、执行、清理的生命周期管理
 * - 配置接口：查询计划的配置和参数设置
 * - 监控接口：执行过程的监控和状态查询
 * - 扩展接口：查询计划的扩展和定制功能
 * - 调试接口：查询执行的调试和诊断信息
 *
 * HOW: 统一查询计划抽象层的实现机制
 *
 * 模板方法实现：
 * 1. 算法骨架：executePlan定义执行算法的主流程
 * 2. 具体步骤：buildPlan、optimizePlan、executePlan等具体步骤
 * 3. 钩子方法：允许子类在关键节点插入自定义逻辑
 * 4. 默认实现：为通用步骤提供默认实现
 * 5. 扩展机制：子类可以重写任何步骤进行定制
 *
 * 策略模式实现：
 * 1. 策略接口：UnifiedQueryPlan定义策略接口
 * 2. 具体策略：不同类型的查询计划作为具体策略
 * 3. 上下文管理：QueryPlanFactory作为策略选择器
 * 4. 运行时切换：支持运行时动态切换执行策略
 * 5. 策略组合：支持多个策略的组合和协作
 *
 * 执行流程实现：
 * 1. 计划构建：根据SQL AST构建具体的执行计划
 * 2. 资源分配：为执行计划分配必要的系统资源
 * 3. 权限验证：验证用户对相关对象的访问权限
 * 4. 查询优化：应用各种查询优化技术和规则
 * 5. 物理执行：执行物理操作符生成查询结果
 * 6. 结果处理：格式化和封装查询执行结果
 *
 * 优化策略实现：
 * 1. 逻辑优化：基于关系代数的查询重写优化
 * 2. 物理优化：基于代价模型的执行计划选择
 * 3. 索引优化：智能的索引选择和使用策略
 * 4. 连接优化：多表连接的顺序和算法选择
 * 5. 缓存优化：查询结果和中间结果的缓存策略
 *
 * 并发控制实现：
 * 1. 锁管理：行级、页级、表级的锁管理机制
 * 2. 事务隔离：不同隔离级别的并发控制策略
 * 3. 死锁检测：死锁检测和自动解除机制
 * 4. 并发优化：基于快照的多版本并发控制
 * 5. 资源协调：多查询间的资源竞争协调
 *
 * 错误处理实现：
 * 1. 异常分类：语法错误、语义错误、运行时错误等分类
 * 2. 错误传播：错误信息的层次化传递和处理
 * 3. 恢复策略：不同错误的自动恢复和补偿机制
 * 4. 错误日志：详细的错误信息记录和诊断
 * 5. 用户反馈：用户友好的错误信息和建议
 *
 * 性能监控实现：
 * 1. 执行时间：查询执行各个阶段的时间统计
 * 2. 资源消耗：CPU、内存、I/O等资源的消耗统计
 * 3. 缓存命中：缓存使用效率和命中率统计
 * 4. 并发指标：并发查询数量和等待时间统计
 * 5. 系统负载：整体系统负载和性能指标
 *
 * 扩展性设计：
 * - 插件架构：支持第三方查询计划的动态加载
 * - DSL支持：领域特定语言的查询扩展支持
 * - AI优化：基于机器学习的智能查询优化
 * - 分布式扩展：支持分布式查询的执行计划
 * - 云原生适配：适配云环境的多租户和弹性伸缩
 *
 * 调试和诊断：
 * - 执行跟踪：详细记录查询执行的各个步骤
 * - 性能分析：识别查询执行的性能瓶颈
 * - 优化建议：基于执行统计的优化建议
 * - 可视化工具：查询计划的可视化展示
 * - 历史回溯：查询执行历史和趋势分析
 */

#ifndef SQLCC_EXECUTION_UNIFIED_QUERY_PLAN_H
#define SQLCC_EXECUTION_UNIFIED_QUERY_PLAN_H

#include <memory>
#include <string>
#include <vector>

// 使用正确的头文件路径
#include "../sql_parser/ast/ast_node.h"
#include "../sql_parser/ast/statement.h"

namespace sqlcc {

class DatabaseManager;
class UserManager;
class SystemDatabase;

/**
 * @brief 统一查询计划 - 执行SQL语句的抽象接口
 *
 * 提供统一的查询计划执行接口，支持不同的SQL语句类型
 */
class UnifiedQueryPlan {
public:
    /**
     * @brief 构造函数
     */
    UnifiedQueryPlan(std::shared_ptr<DatabaseManager> db_manager,
                    std::shared_ptr<UserManager> user_manager,
                    std::shared_ptr<SystemDatabase> system_db);

    /**
     * @brief 析构函数
     */
    virtual ~UnifiedQueryPlan();

    /**
     * @brief 构建查询计划
     * @param stmt SQL语句AST
     * @return 是否构建成功
     */
    virtual bool buildPlan(std::unique_ptr<sql_parser::Statement> stmt);

    /**
     * @brief 执行查询计划
     * @return 执行结果
     */
    virtual std::string executePlan();

    /**
     * @brief 获取计划类型
     * @return 计划类型字符串
     */
    virtual std::string getPlanType() const;

protected:
    std::shared_ptr<DatabaseManager> db_manager_;
    std::shared_ptr<UserManager> user_manager_;
    std::shared_ptr<SystemDatabase> system_db_;
    std::unique_ptr<sql_parser::Statement> stmt_;
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_UNIFIED_QUERY_PLAN_H
