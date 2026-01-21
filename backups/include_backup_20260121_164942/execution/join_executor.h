/**
 * WHY: 为什么需要专门的JOIN执行器？
 *
 * JOIN操作是关系数据库的核心特性，也是性能瓶颈的主要来源，
 * 传统方案存在诸多技术挑战：
 * - JOIN算法选择复杂：不同数据量和数据分布需要不同的JOIN算法
 * - 内存使用效率低下：大表JOIN容易造成内存溢出和性能下降
 * - 并发执行困难：多表JOIN的并发执行需要复杂的同步机制
 * - 结果集管理复杂：JOIN结果的去重、排序、限制等处理困难
 * - 类型安全保证：JOIN操作的类型兼容性和NULL值处理复杂
 * - 扩展性设计缺失：难以添加新的JOIN类型和优化策略
 * - 调试诊断不便：JOIN执行过程缺乏有效的监控和诊断
 *
 * JOIN执行器的核心价值：
 * 1. 算法智能选择：根据数据特征自动选择最优JOIN算法
 * 2. 性能优化策略：针对不同JOIN类型实现专门的优化算法
 * 3. 内存管理优化：大表JOIN的流式处理和内存复用机制
 * 4. 并发安全控制：多线程环境下的JOIN操作安全执行
 * 5. 结果集智能管理：JOIN结果的去重、排序、限制等优化处理
 * 6. 类型安全保证：严格的类型兼容性检查和错误处理
 * 7. 可扩展架构：支持新JOIN类型和自定义优化策略
 *
 * 🏗️ 设计模式：策略模式(Strategy Pattern) + 模板方法模式(Template Method Pattern) + 工厂模式(Factory Pattern)
 *
 * JOIN执行器作为策略模式的应用：
 * - 算法封装：将不同JOIN算法封装在独立的策略类中
 * - 运行时选择：根据JOIN类型和数据特征动态选择算法
 * - 算法替换：可以透明地替换或扩展新的JOIN算法
 * - 代码复用：避免重复实现相似的JOIN逻辑
 * - 易于测试：每个算法都可以独立测试和验证
 *
 * SOLID原则体现：
 * - 单一职责：JOIN执行器专门负责表连接操作的执行和管理
 * - 开闭原则：新JOIN算法通过扩展现有类实现
 * - 里氏替换：不同JOIN实现可以互相替换
 * - 接口隔离：JOIN接口精确定义连接契约
 * - 依赖倒置：执行器依赖抽象的存储和查询接口
 *
 * WHAT: JOIN执行器系统 - SQL表连接统一执行框架
 *
 * 核心功能：
 * - 多类型JOIN支持：支持INNER、LEFT、RIGHT、FULL OUTER等JOIN类型
 * - 智能算法选择：根据表大小和数据特征选择最优JOIN算法
 * - 条件评估优化：高效的JOIN条件评估和结果过滤
 * - 结果集处理：JOIN结果的去重、排序、限制等后处理
 * - 内存优化管理：大表JOIN的流式处理和内存控制
 * - 并发执行支持：多表JOIN的并行计算和结果合并
 * - 性能监控统计：JOIN执行过程的性能监控和统计
 *
 * 系统组件：
 * - JoinExecutor：核心执行器，协调整个JOIN操作过程
 * - JoinAlgorithm：JOIN算法接口，定义算法的通用行为
 * - NestedLoopJoin：嵌套循环JOIN算法，适用于小表连接
 * - HashJoin：哈希JOIN算法，适用于大表等值连接
 * - MergeJoin：归并JOIN算法，适用于有序数据连接
 * - JoinConditionEvaluator：JOIN条件评估器，处理连接条件的计算
 * - ResultProcessor：结果处理器，处理JOIN结果的后处理逻辑
 *
 * JOIN类型支持：
 * - INNER JOIN：只返回两个表中满足连接条件的行
 * - LEFT JOIN：返回左表所有行和右表满足条件的行
 * - RIGHT JOIN：返回右表所有行和左表满足条件的行
 * - FULL OUTER JOIN：返回两个表中所有行，无论是否满足条件
 * - CROSS JOIN：返回两个表的笛卡尔积
 * - NATURAL JOIN：基于同名列的等值连接
 * - USING子句：指定连接列的简化语法
 *
 * JOIN算法选择：
 * - Nested Loop Join：简单暴力，适用于小表连接
 * - Hash Join：构建哈希表，适用于大表等值连接
 * - Merge Join：利用排序，适用于有序数据连接
 * - Index Join：利用索引，适用于索引覆盖的连接
 * - Sort-Merge Join：排序后归并，适用于无索引的有序连接
 * - Hybrid Join：根据情况动态组合多种算法
 *
 * 执行流程：
 * - 预处理阶段：分析JOIN类型、条件和表统计信息
 * - 算法选择阶段：根据数据特征选择最优JOIN算法
 * - 执行准备阶段：构建哈希表、排序数据或准备索引
 * - 连接执行阶段：执行具体的JOIN算法计算连接结果
 * - 结果处理阶段：应用NULL填充、去重、排序等后处理
 * - 结果返回阶段：返回最终的JOIN结果集
 *
 * 性能优化策略：
 * - 表顺序优化：选择最优的表连接顺序
 * - 索引利用优化：充分利用现有索引加速连接
 * - 谓词下推优化：将WHERE条件推入JOIN操作
 * - 中间结果缓存：缓存中间JOIN结果避免重复计算
 * - 并行处理优化：多线程并行执行JOIN操作
 * - 内存预分配：预分配内存减少动态分配开销
 *
 * 内存管理策略：
 * - 流式处理：大表JOIN的分块处理，避免内存溢出
 * - 分页缓存：结果集的分页存储和按需加载
 * - 对象复用：复用JOIN过程中的临时对象
 * - 垃圾回收：及时清理不再需要的中间结果
 * - 内存监控：实时监控JOIN操作的内存使用情况
 * - 内存限制：设置JOIN操作的最大内存使用限制
 *
 * 并发控制机制：
 * - 线程安全：确保JOIN操作在多线程环境的安全执行
 * - 结果同步：多线程JOIN结果的同步和合并
 * - 资源隔离：不同JOIN操作间的资源隔离
 * - 死锁预防：避免JOIN操作产生的死锁情况
 * - 优先级调度：重要JOIN操作的优先级处理
 * - 负载均衡：平衡多线程间的JOIN计算负载
 *
 * 接口设计：
 * - 执行接口：JOIN操作的主要执行接口
 * - 配置接口：JOIN参数和策略的配置接口
 * - 监控接口：JOIN执行性能和状态的监控接口
 * - 扩展接口：新JOIN算法和优化策略的扩展接口
 *
 * HOW: JOIN执行器系统的实现机制
 *
 * 策略模式实现：
 * 1. 抽象策略基类：定义JOIN算法的通用接口和行为
 * 2. 具体策略实现：NestedLoop、Hash、Merge等具体算法
 * 3. 策略选择器：根据JOIN类型和数据特征选择算法
 * 4. 上下文管理：维护JOIN执行的上下文和状态
 * 5. 结果封装：统一的JOIN结果封装和返回
 *
 * Nested Loop实现：
 * 1. 外层循环：遍历左表（驱动表）的每一行
 * 2. 内层循环：对每一行，在右表中查找匹配行
 * 3. 条件评估：对每对行组合评估JOIN条件
 * 4. 结果收集：收集满足条件的行组合
 * 5. 性能优化：通过索引加速内层查找
 *
 * Hash Join实现：
 * 1. 构建阶段：选择小表构建哈希表
 * 2. 探测阶段：遍历大表，在哈希表中查找匹配
 * 3. 哈希函数：选择合适的哈希函数分散数据
 * 4. 冲突处理：处理哈希冲突的策略
 * 5. 内存管理：控制哈希表的内存使用
 *
 * Merge Join实现：
 * 1. 排序阶段：确保两个表按连接键排序
 * 2. 归并阶段：像归并排序一样同时遍历两个表
 * 3. 匹配处理：处理一对多和多对多的情况
 * 4. 边界处理：正确处理表的边界情况
 * 5. 索引利用：利用现有索引避免排序
 *
 * 条件评估实现：
 * 1. 表达式解析：解析JOIN ON子句的条件表达式
 * 2. 类型转换：处理不同数据类型的比较
 * 3. NULL处理：正确处理NULL值的比较语义
 * 4. 短路求值：优化条件评估的性能
 * 5. 错误处理：条件评估过程中的异常处理
 *
 * 结果处理实现：
 * 1. NULL填充：根据JOIN类型填充NULL值
 * 2. 列合并：合并左右表的列到结果行
 * 3. 去重处理：消除重复的JOIN结果
 * 4. 排序应用：应用ORDER BY子句的排序
 * 5. 限制应用：应用LIMIT子句的限制
 *
 * 内存优化实现：
 * 1. 分页读取：大表的分页读取和处理
 * 2. 缓冲区复用：复用数据读取缓冲区
 * 3. 结果流式：流式生成JOIN结果
 * 4. 对象池：使用对象池减少分配开销
 * 5. 内存监控：实时监控内存使用情况
 *
 * 并发优化实现：
 * 1. 任务分解：将JOIN操作分解为独立任务
 * 2. 线程池：利用线程池执行并发任务
 * 3. 结果同步：同步多线程的执行结果
 * 4. 负载均衡：平衡各线程的计算负载
 * 5. 资源协调：协调多线程间的资源使用
 *
 * 错误处理实现：
 * 1. 数据错误：JOIN操作中的数据类型错误
 * 2. 内存错误：内存不足导致的JOIN中断
 * 3. 并发错误：多线程执行中的同步错误
 * 4. 超时错误：JOIN操作超时的错误
 * 5. 资源错误：资源不足导致的错误
 *
 * 扩展性设计：
 * - 插件架构：支持第三方JOIN算法的动态加载
 * - 自定义算法：用户自定义的JOIN算法实现
 * - 多数据源：支持跨数据源的JOIN操作
 * - 分布式JOIN：支持分布式环境下的JOIN操作
 * - AI优化：基于机器学习的JOIN优化
 *
 * 调试和诊断：
 * - 执行跟踪：详细记录JOIN执行的每一步过程
 * - 性能分析：分析JOIN算法的性能瓶颈和优化机会
 * - 内存分析：监控JOIN操作的内存使用模式和峰值
 * - 结果验证：验证JOIN结果的正确性和完整性
 * - 可视化工具：JOIN执行过程和结果的可视化展示
 */

#include "sql_parser/ast_nodes.h"
#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include "sql_parser/ast_nodes.h"
#include "storage/table_storage.h"

namespace sqlcc {
namespace execution {

/**
 * @brief JOIN结果行
 */
struct JoinResultRow {
  std::vector<std::string> left_row;
  std::vector<std::string> right_row;
  bool is_null_row;  // 用于LEFT/RIGHT JOIN的NULL填充

  JoinResultRow() : is_null_row(false) {}
  JoinResultRow(const std::vector<std::string>& left, const std::vector<std::string>& right)
      : left_row(left), right_row(right), is_null_row(false) {}
};

/**
 * @brief JOIN条件评估器
 */
class JoinConditionEvaluator {
public:
  /**
   * @brief 构造函数
   * @param left_columns 左表列名映射
   * @param right_columns 右表列名映射
   * @param condition JOIN条件表达式
   */
  JoinConditionEvaluator(
      const std::unordered_map<std::string, size_t>& left_columns,
      const std::unordered_map<std::string, size_t>& right_columns,
      std::unique_ptr<sql_parser::Expression> condition);

  /**
   * @brief 评估JOIN条件
   * @param left_row 左表行数据
   * @param right_row 右表行数据
   * @return 条件是否满足
   */
  bool evaluate(const std::vector<std::string>& left_row,
                const std::vector<std::string>& right_row) const;

private:
  std::unordered_map<std::string, size_t> left_columns_;
  std::unordered_map<std::string, size_t> right_columns_;
  std::unique_ptr<sql_parser::Expression> condition_;

  // 简化条件评估（实际应使用完整的表达式求值器）
  bool evaluateSimpleCondition(const std::vector<std::string>& left_row,
                              const std::vector<std::string>& right_row) const;
};

/**
 * @brief JOIN算法接口
 */
class JoinAlgorithm {
public:
  virtual ~JoinAlgorithm() = default;

  /**
   * @brief 执行JOIN操作
   * @param left_table 左表数据
   * @param right_table 右表数据
   * @param join_type JOIN类型
   * @param condition JOIN条件
   * @return JOIN结果
   */
  virtual std::vector<JoinResultRow> execute(
      const std::vector<std::vector<std::string>>& left_table,
      const std::vector<std::vector<std::string>>& right_table,
      sql_parser::JoinClause::JoinType join_type,
      const JoinConditionEvaluator* condition) = 0;

  /**
   * @brief 获取算法名称
   */
  virtual std::string getAlgorithmName() const = 0;

  /**
   * @brief 估算执行成本
   */
  virtual double estimateCost(size_t left_rows, size_t right_rows) const = 0;
};

/**
 * @brief Nested Loop JOIN算法
 */
class NestedLoopJoin : public JoinAlgorithm {
public:
  std::vector<JoinResultRow> execute(
      const std::vector<std::vector<std::string>>& left_table,
      const std::vector<std::vector<std::string>>& right_table,
      sql_parser::JoinClause::JoinType join_type,
      const JoinConditionEvaluator* condition) override;

  std::string getAlgorithmName() const override { return "Nested Loop Join"; }

  double estimateCost(size_t left_rows, size_t right_rows) const override {
    return static_cast<double>(left_rows) * right_rows;
  }
};

/**
 * @brief Hash JOIN算法
 */
class HashJoin : public JoinAlgorithm {
public:
  std::vector<JoinResultRow> execute(
      const std::vector<std::vector<std::string>>& left_table,
      const std::vector<std::vector<std::string>>& right_table,
      sql_parser::JoinClause::JoinType join_type,
      const JoinConditionEvaluator* condition) override;

  std::string getAlgorithmName() const override { return "Hash Join"; }

  double estimateCost(size_t left_rows, size_t right_rows) const override {
    return static_cast<double>(left_rows) + right_rows;
  }

private:
  /**
   * @brief 构建哈希表
   */
  std::unordered_multimap<std::string, size_t> buildHashTable(
      const std::vector<std::vector<std::string>>& table,
      const std::function<std::string(const std::vector<std::string>&)>& key_func);

  /**
   * @brief 简单哈希键生成（基于第一列）
   */
  static std::string simpleHashKey(const std::vector<std::string>& row) {
    return row.empty() ? "" : row[0];
  }
};

/**
 * @brief Merge JOIN算法
 */
class MergeJoin : public JoinAlgorithm {
public:
  std::vector<JoinResultRow> execute(
      const std::vector<std::vector<std::string>>& left_table,
      const std::vector<std::vector<std::string>>& right_table,
      sql_parser::JoinClause::JoinType join_type,
      const JoinConditionEvaluator* condition) override;

  std::string getAlgorithmName() const override { return "Merge Join"; }

  double estimateCost(size_t left_rows, size_t right_rows) const override {
    return static_cast<double>(left_rows) + right_rows;
  }

private:
  /**
   * @brief 排序表数据（假设按第一列排序）
   */
  static std::vector<std::vector<std::string>> sortTable(
      const std::vector<std::vector<std::string>>& table);
};

/**
 * @brief JOIN执行器
 */
class JoinExecutor {
public:
  /**
   * @brief 构造函数
   */
  JoinExecutor();

  /**
   * @brief 执行JOIN操作
   * @param left_table 左表数据
   * @param right_table 右表数据
   * @param join_clause JOIN子句
   * @param left_columns 左表列名映射
   * @param right_columns 右表列名映射
   * @return JOIN结果
   */
  std::vector<std::vector<std::string>> executeJoin(
      const std::vector<std::vector<std::string>>& left_table,
      const std::vector<std::vector<std::string>>& right_table,
      sql_parser::JoinClause& join_clause,
      const std::unordered_map<std::string, size_t>& left_columns,
      const std::unordered_map<std::string, size_t>& right_columns);

  /**
   * @brief 选择最优JOIN算法
   * @param left_rows 左表行数
   * @param right_rows 右表行数
   * @param join_type JOIN类型
   * @return 最优算法
   */
  std::unique_ptr<JoinAlgorithm> selectOptimalAlgorithm(
      size_t left_rows, size_t right_rows, sql_parser::JoinClause::JoinType join_type);

private:
  /**
   * @brief 创建条件评估器
   */
  std::unique_ptr<JoinConditionEvaluator> createConditionEvaluator(
      const std::unordered_map<std::string, size_t>& left_columns,
      const std::unordered_map<std::string, size_t>& right_columns,
      sql_parser::JoinClause& join_clause);

  /**
   * @brief 转换JoinResultRow为最终结果行
   */
  std::vector<std::string> convertToResultRow(
      const JoinResultRow& join_row,
      const std::unordered_map<std::string, size_t>& left_columns,
      const std::unordered_map<std::string, size_t>& right_columns);

  /**
   * @brief 处理不同JOIN类型的NULL填充
   */
  std::vector<JoinResultRow> handleJoinType(
      const std::vector<JoinResultRow>& inner_results,
      const std::vector<std::vector<std::string>>& left_table,
      const std::vector<std::vector<std::string>>& right_table,
      sql_parser::JoinClause::JoinType join_type);

  /**
   * @brief 创建NULL填充行
   */
  static std::vector<std::string> createNullRow(size_t column_count) {
    return std::vector<std::string>(column_count, "NULL");
  }

  // 算法实例
  NestedLoopJoin nested_loop_join_;
  HashJoin hash_join_;
  MergeJoin merge_join_;
};

} // namespace execution
} // namespace sqlcc
