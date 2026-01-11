/**
 * WHY: 为什么需要专门的集合操作执行器？
 *
 * 集合操作是关系数据库的核心特性，传统的处理方式存在诸多问题：
 * - 集合操作逻辑分散：UNION、INTERSECT、EXCEPT等操作逻辑分散在不同模块
 * - 性能优化困难：缺乏针对集合操作的专门优化策略和算法
 * - 结果去重复杂：ALL和DISTINCT语义的处理不够统一和高效
 * - 排序处理不佳：ORDER BY和LIMIT在集合操作中的应用不够灵活
 * - 类型兼容性检查：集合操作两侧结果集的类型兼容性验证不够严格
 * - 内存使用不优：大结果集的集合操作内存管理策略不够高效
 *
 * 集合操作执行器的核心价值：
 * 1. 统一操作接口：为所有集合操作提供统一的执行接口和处理逻辑
 * 2. 性能优化策略：针对不同集合操作实现专门的优化算法
 * 3. 结果处理优化：高效的去重、排序、限制等结果处理机制
 * 4. 类型安全保证：严格的类型兼容性检查和错误处理
 * 5. 内存管理优化：大结果集处理的内存优化和流式处理能力
 * 6. 可扩展架构：支持新的集合操作类型和优化策略的扩展
 *
 * 🏗️ 设计模式：策略模式(Strategy Pattern) + 组合模式(Composite Pattern) + 迭代器模式(Iterator Pattern)
 *
 * 集合操作执行器作为策略模式的应用：
 * - 算法封装：将不同的集合操作算法封装在独立的策略类中
 * - 运行时选择：根据操作类型动态选择合适的执行策略
 * - 算法替换：可以透明地替换或扩展新的集合操作算法
 * - 代码复用：避免重复实现相似的集合操作逻辑
 * - 易于测试：每个策略都可以独立测试和验证
 *
 * SOLID原则体现：
 * - 单一职责：集合操作执行器专门负责集合操作的执行和管理
 * - 开闭原则：新集合操作类型通过扩展现有类实现
 * - 里氏替换：不同集合操作实现可以互相替换
 * - 接口隔离：集合操作接口精确定义执行契约
 * - 依赖倒置：执行器依赖抽象的集合操作接口而非具体实现
 *
 * WHAT: 集合操作执行器系统 - SQL集合操作统一执行框架
 *
 * 核心功能：
 * - 集合操作执行：执行UNION、INTERSECT、EXCEPT等集合操作
 * - 结果去重处理：支持ALL和DISTINCT两种去重模式的处理
 * - 排序和限制：集合操作结果的ORDER BY和LIMIT处理
 * - 类型兼容性检查：确保集合操作两侧结果集的类型兼容
 * - 性能优化策略：针对不同场景的集合操作优化算法
 * - 错误处理统一：集合操作执行异常的统一捕获和处理
 *
 * 系统组件：
 * - SetOperationExecutor：核心集合操作执行器，管理所有集合操作
 * - UnionStrategy：UNION操作的具体实现策略
 * - IntersectStrategy：INTERSECT操作的具体实现策略
 * - ExceptStrategy：EXCEPT操作的具体实现策略
 * - ResultProcessor：集合操作结果的后处理组件
 * - TypeValidator：集合操作的类型兼容性验证器
 *
 * 集合操作类型：
 * - UNION：两个结果集的并集操作，去除重复行
 * - UNION ALL：两个结果集的并集操作，保留重复行
 * - INTERSECT：两个结果集的交集操作，只保留共同行
 * - INTERSECT ALL：两个结果集的交集操作，保留重复次数
 * - EXCEPT：两个结果集的差集操作，从第一个结果集中减去第二个结果集
 * - EXCEPT ALL：两个结果集的差集操作，考虑重复次数
 *
 * 执行流程：
 * - 解析集合操作：解析集合操作的语法结构和语义信息
 * - 验证类型兼容：检查左右两个查询结果集的类型兼容性
 * - 执行子查询：分别执行集合操作左右两侧的子查询
 * - 应用集合操作：根据操作类型执行相应的集合操作算法
 * - 处理去重模式：根据ALL/DISTINCT关键字处理重复行
 * - 应用排序限制：执行ORDER BY和LIMIT子句
 * - 返回最终结果：封装和返回集合操作的最终结果
 *
 * 去重处理策略：
 * - DISTINCT模式：使用哈希表或排序算法去除重复行
 * - ALL模式：保留所有行，包括重复行
 * - 混合模式：部分操作支持ALL，部分强制DISTINCT
 * - 性能优化：根据数据量选择最优的去重算法
 * - 内存管理：大结果集的流式去重处理
 *
 * 类型兼容性：
 * - 列数匹配：左右结果集的列数必须相同
 * - 类型兼容：对应列的数据类型必须兼容或可转换
 * - 名称匹配：列名可以不同，但类型必须匹配
 * - NULL处理：NULL值的类型兼容性特殊处理
 * - 精度保持：数值类型的精度和范围保持一致性
 *
 * 性能优化策略：
 * - 算法选择：根据数据量和操作类型选择最优算法
 * - 索引优化：利用现有索引加速集合操作
 * - 并行处理：大结果集的并行集合操作处理
 * - 内存优化：结果集的分页和流式处理
 * - 缓存策略：常用集合操作结果的缓存优化
 *
 * 接口设计：
 * - 执行接口：集合操作的主要执行接口
 * - 配置接口：集合操作参数和策略的配置接口
 * - 监控接口：集合操作执行性能和状态的监控接口
 * - 扩展接口：新集合操作类型和优化策略的扩展接口
 *
 * HOW: 集合操作执行器的实现机制
 *
 * 策略模式实现：
 * 1. 抽象策略基类：定义集合操作的通用接口和行为
 * 2. 具体策略实现：UNION、INTERSECT、EXCEPT的具体实现
 * 3. 策略选择器：根据操作类型选择合适的执行策略
 * 4. 上下文管理：维护集合操作的执行上下文和状态
 * 5. 结果封装：统一的集合操作结果封装和返回
 *
 * 集合操作算法：
 * 1. 哈希算法：使用哈希表实现快速的集合操作
 * 2. 排序算法：利用排序实现有序的集合操作
 * 3. 归并算法：多路归并的集合操作处理
 * 4. 索引算法：利用数据库索引加速集合操作
 * 5. 并行算法：多线程并行处理的集合操作
 *
 * 去重处理实现：
 * 1. 哈希去重：使用哈希表记录已出现行，快速查找重复
 * 2. 排序去重：对结果集排序后合并相邻重复行
 * 3. 位图去重：使用位图标记出现过的行号
 * 4. Bloom过滤器：大结果集的概率性去重预处理
 * 5. 外部排序：无法放入内存的超大结果集去重
 *
 * 内存管理实现：
 * 1. 流式处理：大结果集的分批处理和内存控制
 * 2. 分页缓存：结果集的分页存储和按需加载
 * 3. 临时文件：超出内存限制时使用临时文件存储
 * 4. 垃圾回收：及时清理不再需要的中间结果
 * 5. 内存池：复用内存分配优化性能
 *
 * 类型验证实现：
 * 1. 静态类型检查：编译时验证集合操作的类型兼容性
 * 2. 动态类型转换：运行时进行必要的类型转换
 * 3. 兼容性矩阵：定义不同数据类型的兼容性规则
 * 4. 错误报告：详细的类型不匹配错误信息
 * 5. 类型推导：自动推导结果集的最终类型
 *
 * 并发控制实现：
 * 1. 线程安全：确保集合操作在多线程环境的安全执行
 * 2. 锁优化：最小化锁的使用范围和时间
 * 3. 读写分离：读操作和写操作的分离处理
 * 4. 事务支持：集合操作的事务一致性保证
 * 5. 死锁预防：避免集合操作产生的死锁情况
 *
 * 错误处理实现：
 * 1. 类型错误：集合操作的类型不兼容错误
 * 2. 资源错误：内存不足或磁盘空间不足错误
 * 3. 执行错误：集合操作执行过程中的运行时错误
 * 4. 语法错误：集合操作语法的解析错误
 * 5. 性能错误：集合操作执行超时或性能异常
 *
 * 扩展性设计：
 * - 插件架构：支持第三方集合操作算法的动态加载
 * - 自定义操作：支持用户定义的集合操作类型
 * - 多语言支持：不同编程语言实现的集合操作
 * - 分布式扩展：支持分布式环境下的集合操作
 * - AI优化：基于机器学习的集合操作优化
 *
 * 调试和监控：
 * - 执行跟踪：详细记录集合操作的执行过程和中间状态
 * - 性能分析：分析集合操作的性能瓶颈和优化机会
 * - 内存分析：监控集合操作的内存使用模式和峰值
 * - 结果验证：验证集合操作结果的正确性和完整性
 * - 可视化工具：集合操作执行流程和数据流的可视化
 */

#pragma once

#include <memory>
#include <vector>
#include <string>
#include "sql_parser/set_operation.h"
#include "core/core_database_manager.h"
#include "core/execution_context.h"
#include "core/execution_result.h"
#include "sql_parser/ast_nodes.h"

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
