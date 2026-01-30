/**
 * WHY: 为什么需要专门的DML执行策略？
 *
 * DML语句是数据库系统的"心脏"，负责数据的查询和修改操作，
 * 传统方案存在诸多技术挑战：
 * - 查询复杂度控制：复杂查询的性能优化和执行计划选择
 * - 数据一致性保障：多表操作的事务一致性和并发控制
 * - 结果集处理优化：大结果集的内存管理和分页处理
 * - 索引利用效率：查询条件对索引的有效利用和优化
 * - 并发访问协调：多用户并发DML操作的冲突解决
 * - 缓存策略优化：查询结果缓存和中间结果重用
 * - 资源使用监控：DML操作的CPU、内存、I/O资源监控
 *
 * DML执行策略的核心价值：
 * 1. 查询性能优化：智能的查询执行计划选择和优化
 * 2. 数据一致性保障：严格的事务隔离级别和锁管理
 * 3. 结果集智能处理：大结果集的分页、排序、去重优化
 * 4. 索引高效利用：自动选择最优索引和查询路径
 * 5. 并发安全执行：多用户并发操作的安全协调
 * 6. 缓存策略优化：查询结果缓存和计算结果重用
 * 7. 资源监控管理：DML操作的资源使用监控和限制
 *
 * 🏗️ 设计模式：策略模式(Strategy Pattern) + 模板方法模式(Template Method Pattern) + 迭代器模式(Iterator Pattern)
 *
 * DML执行策略作为策略模式的应用：
 * - 算法封装：将不同DML操作的处理算法封装在独立的策略中
 * - 运行时选择：根据DML语句类型和数据特征动态选择算法
 * - 算法替换：可以透明地替换或扩展新的DML处理算法
 * - 代码复用：避免重复实现相似的DML处理逻辑
 * - 易于测试：每个DML处理策略都可以独立测试
 *
 * SOLID原则体现：
 * - 单一职责：DML执行策略专门负责数据操作语言语句的执行和管理
 * - 开闭原则：新DML语句类型通过扩展现有类实现
 * - 里氏替换：不同DML实现可以互相替换
 * - 接口隔离：DML接口精确定义数据操作契约
 * - 依赖倒置：执行器依赖抽象的存储和查询接口
 *
 * WHAT: DML执行策略系统 - 数据操作语言统一执行框架
 *
 * 核心功能：
 * - SELECT语句执行：复杂查询语句的解析和执行优化
 * - INSERT语句执行：数据插入操作的事务安全处理
 * - UPDATE语句执行：数据更新操作的条件匹配和修改
 * - DELETE语句执行：数据删除操作的级联处理和约束检查
 * - 查询结果处理：结果集的排序、分页、去重和格式化
 * - 事务一致性保障：DML操作的事务原子性和隔离性
 * - 性能监控统计：DML执行过程的性能监控和统计
 *
 * 系统组件：
 * - DMLExecutionStrategy：核心执行策略，协调整个DML操作过程
 * - SelectExecutor：SELECT执行器，处理查询语句的执行
 * - InsertExecutor：INSERT执行器，处理数据插入操作
 * - UpdateExecutor：UPDATE执行器，处理数据更新操作
 * - DeleteExecutor：DELETE执行器，处理数据删除操作
 * - QueryOptimizer：查询优化器，生成最优的查询执行计划
 * - ResultProcessor：结果处理器，处理查询结果的后处理
 * - TransactionManager：事务管理器，管理DML操作的事务
 *
 * SELECT语句类型支持：
 * - 简单SELECT：单表查询，带WHERE条件、ORDER BY、LIMIT等
 * - 多表JOIN查询：INNER JOIN、LEFT JOIN、RIGHT JOIN、FULL OUTER JOIN
 * - 子查询处理：标量子查询、表子查询、相关子查询
 * - 聚合查询：GROUP BY、HAVING、聚合函数（SUM、AVG、COUNT等）
 * - 窗口函数查询：ROW_NUMBER、RANK、窗口聚合等高级查询
 * - 集合操作查询：UNION、INTERSECT、EXCEPT等集合操作
 * - 递归查询：WITH RECURSIVE子句的递归查询处理
 *
 * INSERT语句类型支持：
 * - 单行插入：INSERT INTO table VALUES (...) 语法
 * - 多行插入：INSERT INTO table VALUES (...), (...) 语法
 * - 列指定插入：INSERT INTO table (col1, col2) VALUES (...) 语法
 * - SELECT插入：INSERT INTO table SELECT ... FROM ... 语法
 * - 批量插入：大量数据的批量插入优化处理
 * - 约束检查：PRIMARY KEY、UNIQUE、CHECK等约束验证
 *
 * UPDATE语句类型支持：
 * - 单表更新：UPDATE table SET ... WHERE ... 语法
 * - 多表更新：UPDATE table1, table2 SET ... WHERE ... 语法
 * - 条件更新：基于复杂WHERE条件的更新操作
 * - 批量更新：大量数据的批量更新优化处理
 * - 级联更新：外键约束下的级联更新处理
 * - 触发器触发：UPDATE操作的触发器自动执行
 *
 * DELETE语句类型支持：
 * - 单表删除：DELETE FROM table WHERE ... 语法
 * - 多表删除：DELETE t1 FROM table1 t1 JOIN table2 t2 ON ... 语法
 * - 条件删除：基于复杂WHERE条件的删除操作
 * - 级联删除：外键约束下的级联删除处理
 * - 批量删除：大量数据的批量删除优化处理
 * - 触发器触发：DELETE操作的触发器自动执行
 *
 * 执行流程：
 * - 语句解析验证：解析DML语句并验证语法和语义正确性
 * - 权限预检查：在执行前验证用户的操作权限
 * - 查询优化规划：生成最优的查询执行计划和访问路径
 * - 事务开始管理：启动事务并设置适当的隔离级别
 * - 索引路径选择：选择最优的索引和访问方法
 * - 数据操作执行：执行具体的DML操作（增删改查）
 * - 约束条件检查：验证数据完整性约束和业务规则
 * - 触发器自动执行：执行相关的数据触发器逻辑
 * - 事务提交确认：提交事务并确认操作成功
 * - 结果集返回：返回操作结果和受影响的行数
 *
 * 性能优化策略：
 * - 查询计划缓存：缓存常用的查询执行计划
 * - 索引自动选择：基于统计信息自动选择最优索引
 * - 结果集分页：大结果集的分页获取和处理
 * - 批量操作优化：INSERT、UPDATE、DELETE的批量处理
 * - 并行查询执行：多核CPU的并行查询处理
 * - 内存临时表：复杂查询的内存临时表优化
 * - I/O优化：磁盘I/O操作的预读和缓存优化
 *
 * 内存管理策略：
 * - 结果集流式：大结果集的流式处理和内存控制
 * - 临时对象池：复用查询过程中的临时对象
 * - 缓存策略：查询结果和中间计算结果的缓存
 * - 内存监控：实时监控DML操作的内存使用情况
 * - 垃圾回收：及时清理不再需要的临时数据
 * - 内存限制：防止单个DML操作占用过多内存
 *
 * 并发控制机制：
 * - 行级锁定：精确的行级锁控制并发访问
 * - 表级锁定：必要时的表级锁保证数据一致性
 * - 乐观并发：版本控制的乐观并发控制
 * - 死锁检测：主动检测和解决死锁情况
 * - 锁升级策略：动态调整锁的粒度和范围
 * - 隔离级别：不同操作的适当事务隔离级别
 *
 * 接口设计：
 * - 执行接口：DML语句的主要执行接口
 * - 验证接口：DML语句的验证和检查接口
 * - 优化接口：查询优化的配置和控制接口
 * - 监控接口：DML执行的监控和统计接口
 * - 扩展接口：新DML类型的扩展接口
 *
 * HOW: DML执行策略系统的实现机制
 *
 * 策略模式实现：
 * 1. 抽象策略基类：定义DML执行的通用接口和行为
 * 2. 具体策略实现：SELECT、INSERT、UPDATE、DELETE等具体策略
 * 3. 策略选择器：根据DML语句类型选择合适的执行策略
 * 4. 上下文管理：维护DML执行的上下文和状态
 * 5. 结果封装：统一的DML结果封装和返回
 *
 * SELECT执行实现：
 * 1. 查询解析：解析SELECT语句的各个子句
 * 2. 表访问规划：确定表的访问顺序和连接方式
 * 3. 索引选择：基于WHERE条件选择最优索引
 * 4. 执行计划生成：生成详细的查询执行计划
 * 5. 结果计算：执行计划并计算最终结果
 * 6. 后处理排序：应用ORDER BY和LIMIT子句
 *
 * INSERT执行实现：
 * 1. 数据解析：解析INSERT语句的值和列定义
 * 2. 约束检查：验证数据完整性约束
 * 3. 索引维护：更新相关索引结构
 * 4. 触发器执行：执行INSERT触发器
 * 5. 数据存储：将数据存储到物理存储中
 * 6. 统计更新：更新表的统计信息
 *
 * UPDATE执行实现：
 * 1. 条件解析：解析UPDATE的WHERE条件
 * 2. 目标定位：找到需要更新的数据行
 * 3. 约束验证：验证新数据满足约束条件
 * 4. 数据修改：执行实际的数据更新操作
 * 5. 索引更新：更新受影响的索引
 * 6. 触发器执行：执行UPDATE触发器
 *
 * DELETE执行实现：
 * 1. 条件解析：解析DELETE的WHERE条件
 * 2. 目标定位：找到需要删除的数据行
 * 3. 约束检查：检查外键约束和引用完整性
 * 4. 级联删除：处理级联删除操作
 * 5. 数据删除：执行实际的数据删除操作
 * 6. 索引清理：清理相关索引项
 *
 * 查询优化实现：
 * 1. 统计信息收集：收集表的统计信息
 * 2. 代价估算模型：基于统计信息的代价计算
 * 3. 计划枚举：枚举可能的查询执行计划
 * 4. 最优计划选择：选择代价最小的执行计划
 * 5. 动态重优化：运行时根据实际情况调整计划
 * 6. 学习优化：基于历史执行的机器学习优化
 *
 * 结果处理实现：
 * 1. 结果集构建：从存储层获取数据构建结果集
 * 2. 数据类型转换：将存储格式转换为用户期望格式
 * 3. 排序处理：应用ORDER BY子句的排序
 * 4. 分页处理：应用LIMIT和OFFSET的分页
 * 5. 去重处理：应用DISTINCT的去重操作
 * 6. 格式化输出：最终结果的格式化和返回
 *
 * 事务管理实现：
 * 1. 事务启动：为DML操作启动新事务
 * 2. 隔离级别设置：设置适当的事务隔离级别
 * 3. 锁管理：管理事务所需的各种锁
 * 4. 保存点设置：在关键操作点设置保存点
 * 5. 回滚处理：异常情况下的回滚操作
 * 6. 提交确认：事务成功完成后的提交
 *
 * 并发控制实现：
 * 1. 锁获取策略：根据操作类型获取相应锁
 * 2. 锁兼容性检查：检查锁之间的兼容性
 * 3. 等待队列管理：管理锁等待的队列
 * 4. 死锁检测：使用等待图检测死锁
 * 5. 锁释放策略：及时释放不再需要的锁
 * 6. 升级和降级：锁粒度的动态调整
 *
 * 错误处理实现：
 * 1. 语法错误：DML语句语法错误的处理
 * 2. 语义错误：DML语句语义错误的处理
 * 3. 权限错误：权限不足导致的执行错误
 * 4. 约束错误：数据约束违反导致的错误
 * 5. 并发错误：并发冲突导致的执行错误
 * 6. 资源错误：资源不足导致的执行错误
 *
 * 扩展性设计：
 * - 插件架构：支持第三方查询优化器插件
 * - 自定义函数：支持用户自定义的聚合函数
 * - 多数据源：支持跨数据源的DML操作
 * - 分布式DML：支持分布式环境下的DML操作
 * - AI优化：基于机器学习的查询优化
 *
 * 调试和诊断：
 * - 执行跟踪：详细记录DML执行的每一步过程
 * - 性能分析：分析DML操作的性能瓶颈和优化机会
 * - 查询计划可视化：查询执行计划的可视化展示
 * - 统计信息收集：DML操作的详细统计信息
 * - 错误诊断：详细的错误诊断和解决建议
 * - 审计日志：完整的DML操作审计日志记录
 */

/**
 * @file dml_execution_strategy.h
 * @brief DML执行策略头文件
 */

#ifndef SQLCC_EXECUTION_DML_EXECUTION_STRATEGY_H
#define SQLCC_EXECUTION_DML_EXECUTION_STRATEGY_H

#include "../sql_parser/ast/ast_node.h"
#include "../sql_parser/ast/ast_nodes.h"
#include <string>
#include <vector>
#include <memory>

#include "../core/execution_result.h"
#include "execution_strategy.h"
#include "../core/execution_context.h"

namespace sqlcc {

struct TableMetadata;

// DML执行策略类
class DMLExecutionStrategy : public ExecutionStrategy {
public:
    DMLExecutionStrategy();
    ~DMLExecutionStrategy() override = default;

    ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                           ExecutionContext &context) override;
    bool checkPermission(const sql_parser::Statement& stmt,
                        const ExecutionContext &context) override;
    bool validate(const sql_parser::Statement& stmt,
                 const ExecutionContext &context) override;
    std::string getStrategyName() const override { return "DMLExecutionStrategy"; }

private:
    ExecutionResult executeInsert(const sql_parser::InsertStatement& stmt,
                                 ExecutionContext &context);
    ExecutionResult executeUpdate(const sql_parser::UpdateStatement& stmt,
                                 ExecutionContext &context);
    ExecutionResult executeDelete(const sql_parser::DeleteStatement& stmt,
                                 ExecutionContext &context);
    ExecutionResult executeSelect(const sql_parser::SelectStatement& stmt,
                                 ExecutionContext &context);

    ExecutionResult executeJoinSelect(const sql_parser::SelectStatement& stmt,
                                     ExecutionContext &context);
    ExecutionResult executeGroupBySelect(const sql_parser::SelectStatement& stmt,
                                        ExecutionContext &context);
    ExecutionResult executeAggregateSelect(const sql_parser::SelectStatement& stmt,
                                          ExecutionContext &context);
    ExecutionResult executeSimpleSelect(const sql_parser::SelectStatement& stmt,
                                       ExecutionContext &context);
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_DML_EXECUTION_STRATEGY_H
