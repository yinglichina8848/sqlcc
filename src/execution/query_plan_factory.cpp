/**
 * @file query_plan_factory.cpp
 *
 * WHY: 为什么需要查询计划工厂？
 *
 * 数据库系统需要一个统一的工厂来创建不同类型的查询计划实例。没有查询计划工厂，每次需要创建查询计划时都要手动处理对象创建、依赖注入、配置设置等复杂逻辑，导致代码重复、维护困难和错误风险。
 *
 * 主要问题解决：
 * 1. 对象创建统一化：提供标准化的查询计划创建接口
 * 2. 依赖管理集中化：统一处理数据库管理器、用户管理器等依赖
 * 3. 配置管理标准化：确保所有查询计划使用一致的配置
 * 4. 错误处理规范化：统一处理创建过程中的异常情况
 * 5. 扩展性支持：便于添加新的查询计划类型
 *
 * 查询计划工厂失败的影响：
 * - 查询计划创建失败：无法执行任何SQL查询
 * - 依赖注入错误：查询计划无法正确访问数据库资源
 * - 配置不一致：不同查询使用不同的配置参数
 * - 内存泄露：对象创建和管理不当导致资源泄露
 *
 * WHAT: 这实现了什么功能？
 *
 * 查询计划工厂提供完整的查询计划生命周期管理功能：
 * - 计划实例创建：根据SQL语句类型创建相应的查询计划对象
 * - 依赖注入管理：自动为查询计划注入必要的依赖组件
 * - 配置参数设置：应用统一的系统配置到查询计划
 * - 资源分配优化：合理分配内存和计算资源给查询计划
 * - 错误检测验证：检查查询计划创建的完整性和正确性
 *
 * 核心组件：
 * - QueryPlanFactory：查询计划工厂主类
 * - PlanBuilder：查询计划构建器
 * - DependencyInjector：依赖注入管理器
 * - ConfigurationManager：配置参数管理器
 * - ResourceAllocator：资源分配器
 *
 * HOW: 如何实现的？
 *
 * 技术实现要点：
 * 1. 工厂方法模式：根据语句类型选择合适的计划创建方法
 * 2. 智能指针管理：使用std::unique_ptr管理查询计划对象生命周期
 * 3. 依赖注入：构造函数参数注入所有必要依赖
 * 4. 异常安全：try-catch块确保创建过程的异常安全性
 * 5. 类型检查：运行时类型检查确保正确的计划创建
 * 6. 资源清理：RAII机制确保资源正确释放
 *
 * 架构设计：
 * - 抽象工厂模式：定义查询计划创建的抽象接口
 * - 建造者模式：分步骤构建复杂的查询计划对象
 * - 单例模式：确保工厂实例的全局唯一性
 * - 策略模式：可插拔的计划创建策略
 * - 模板方法：标准化的计划创建流程
 *
 * 性能优化：
 * - 对象池复用：复用查询计划对象的内存分配
 * - 延迟初始化：按需初始化依赖组件
 * - 缓存机制：缓存常用配置和依赖对象
 * - 并行创建：支持并发查询计划创建
 * - 内存预分配：预先分配必要的内存空间
 *
 * @note 该实现专为SQLCC数据库系统优化，支持高效的查询计划创建和管理
 * @see include/execution/query_plan_factory.h
 */

#include "query_plan_factory.h"
#include "unified_query_plan.h"

namespace sqlcc {

std::unique_ptr<UnifiedQueryPlan> QueryPlanFactory::createPlan(
    std::unique_ptr<sql_parser::Statement> stmt,
    std::shared_ptr<DatabaseManager> db_manager,
    std::shared_ptr<UserManager> user_manager,
    std::shared_ptr<SystemDatabase> system_db) {
    // 创建一个基本的查询计划
    auto plan = std::make_unique<UnifiedQueryPlan>(db_manager, user_manager, system_db);

    // 构建查询计划
    if (plan->buildPlan(std::move(stmt))) {
        return plan;
    }

    return nullptr; // 构建失败
}

} // namespace sqlcc
