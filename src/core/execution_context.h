/**
 * @file execution_context.h
 * @brief SQLCC执行上下文
 * @author SQLCC Team
 * @date 2026-02-11
 * @copyright Copyright (c) 2026
 *
 * 文件用途说明：
 * 本文件定义了SQL执行上下文，管理执行期间的状态信息。
 * 采用前置声明模式减少头文件依赖，使用接口而非具体实现。
 */

#ifndef SQLCC_EXECUTION_CONTEXT_H
#define SQLCC_EXECUTION_CONTEXT_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sqlcc {

// 前向声明 - 使用前置声明替代直接包含
class DatabaseManager;

// 前向声明
class UserManager;
class SystemDatabase;
class PermissionValidator;

/**
 * WHY: 为什么需要执行上下文？
 *
 * 数据库系统执行SQL语句时面临复杂的上下文管理问题：
 * - 用户身份和权限：每个操作都需要知道当前用户及其权限
 * - 数据库上下文：操作需要在正确的数据库和模式中执行
 * - 事务状态：操作可能处于事务中，需要维护一致性
 * - 执行统计：需要收集执行时间、影响行数等性能指标
 * - 错误处理：执行过程中可能出现错误，需要统一管理
 * - 资源管理：需要管理数据库连接、缓存等资源
 *
 * 传统方法的问题：
 * 1. 参数传递复杂：每个函数都需要传递大量上下文参数
 * 2. 状态不一致：不同组件维护各自的状态副本
 * 3. 耦合度高：业务逻辑与上下文管理紧密耦合
 * 4. 扩展性差：新增上下文信息需要修改大量接口
 * 5. 线程安全：多线程环境下上下文状态容易混乱
 *
 * 执行上下文的核心价值：
 * 1. 统一状态管理：所有执行相关状态集中管理
 * 2. 简化接口设计：通过上下文对象传递，避免大量参数
 * 3. 保证一致性：单一数据源避免状态不一致
 * 4. 提高可维护性：上下文逻辑集中，易于修改和扩展
 * 5. 支持并发：为每个执行线程提供独立的上下文
 * 6. 便于监控：集中收集执行统计信息
 * 7. 权限控制：统一的权限验证和访问控制
 *
 * 执行上下文在数据库系统中的关键作用：
 * - 身份验证：确保用户有权限执行特定操作
 * - 资源隔离：不同用户的操作在各自上下文中执行
 * - 审计追踪：记录用户操作历史和执行结果
 * - 性能监控：收集执行统计用于性能分析
 * - 错误诊断：提供详细的错误上下文信息
 * - 事务管理：维护事务状态和边界控制
 *
 * 🏗️ 设计模式：执行上下文架构设计
 *
 * 设计模式应用：
 * 1. 单例模式(Singleton Pattern)：线程本地上下文
 *    - 每个线程有独立的执行上下文实例
 *    - 避免线程间状态干扰
 *    - 提供全局访问接口
 *
 * 2. 建造者模式(Builder Pattern)：上下文构建
 *    - 复杂的上下文对象通过建造者创建
 *    - 支持可选参数和默认值
 *    - 提高对象构造的灵活性
 *
 * 3. 享元模式(Flyweight Pattern)：共享只读状态
 *    - 用户信息等只读数据可以共享
 *    - 减少内存占用
 *    - 提高性能
 *
 * 4. 策略模式(Strategy Pattern)：可插拔验证器
 *    - 权限验证策略可配置
 *    - 支持不同安全策略
 *    - 便于扩展和定制
 *
 * SOLID原则体现：
 * - 单一职责：专职负责执行上下文管理
 * - 开闭原则：新上下文信息通过扩展实现
 * - 里氏替换：子类可替换父类使用
 * - 接口隔离：客户端依赖具体接口
 * - 依赖倒置：高层不依赖具体实现
 *
 * WHAT: 执行上下文 - SQL执行的统一状态管理容器
 *
 * 核心功能：
 * - 用户身份管理：维护当前用户和权限信息
 * - 数据库上下文：管理当前数据库和连接信息
 * - 事务状态跟踪：记录事务执行状态和ID
 * - 执行统计收集：汇总执行时间、影响行数等指标
 * - 权限验证集成：提供统一的权限检查接口
 * - 错误状态管理：记录和传播执行错误信息
 *
 * 上下文层次：
 * - 会话级上下文：用户登录后的全局上下文
 * - 事务级上下文：事务执行期间的状态
 * - 语句级上下文：单个SQL语句的执行状态
 * - 操作级上下文：具体数据库操作的上下文
 *
 * 接口设计：
 * - 用户管理：get/set_current_user(), get/set_current_database()
 * - 事务控制：is_transactional(), get_transaction_id()
 * - 执行监控：get_execution_time_ms(), get_rows_affected()
 * - 状态查询：has_error(), get_error_message()
 * - 资源访问：get_db_manager(), get_user_manager()
 *
 * HOW: 执行上下文的实现机制和生命周期
 *
 * 上下文创建流程：
 * 1. 用户认证：验证用户身份和权限
 * 2. 上下文初始化：创建执行上下文对象
 * 3. 状态设置：设置用户、数据库、事务等信息
 * 4. 资源关联：绑定数据库管理器等资源
 * 5. 权限验证器：配置权限检查策略
 *
 * 上下文生命周期：
 * 1. 创建阶段：用户连接建立时创建上下文
 * 2. 执行阶段：每次SQL执行时更新状态
 * 3. 事务阶段：事务开始/提交/回滚时更新
 * 4. 清理阶段：连接断开时清理资源
 *
 * 线程安全策略：
 * - 线程本地存储：每个线程独立的上下文实例
 * - 原子操作：状态更新的原子性保证
 * - 锁机制：保护共享资源的并发访问
 * - 不可变对象：只读状态的线程安全共享
 *
 * 内存管理优化：
 * - 对象池：重用上下文对象减少分配开销
 * - 延迟初始化：按需初始化大型对象
 * - 引用计数：智能指针管理资源生命周期
 * - 缓存策略：热点数据的内存缓存
 *
 * 性能优化技术：
 * - 零拷贝：避免不必要的数据复制
 * - 批量更新：减少原子操作的次数
 * - 内联函数：关键路径的性能优化
 * - 预分配：避免运行时的内存分配
 * - 统计聚合：高效的指标收集算法
 *
 * 扩展性设计：
 * - 插件架构：支持自定义上下文扩展
 * - 配置化管理：可配置的上下文策略
 * - 事件驱动：上下文生命周期事件通知
 * - 监控集成：上下文性能监控和统计
 * - 多租户支持：租户级别的上下文隔离
 *
 * 错误处理和恢复：
 * - 异常安全：上下文操作的异常安全性保证
 * - 状态一致性：保证上下文状态的一致性
 * - 错误传播：统一的错误信息传递机制
 * - 自动恢复：常见错误的自动修复机制
 * - 日志记录：详细的错误信息记录
 *
 * 监控和诊断：
 * - 性能指标：上下文创建时间、内存使用等
 * - 健康检查：上下文状态的完整性验证
 * - 统计信息：用户活动、执行统计等
 * - 告警机制：异常情况的自动告警
 * - 调试支持：上下文状态的详细调试信息
 */
class ExecutionContext {
public:
  // 基本上下文信息
  std::string current_user;      // 当前用户名（兼容旧代码）
  std::string current_database;  // 当前数据库名（兼容旧代码）
  std::string current_user_;     // 当前用户名
  std::string current_database_; // 当前数据库名
  bool is_transactional_;        // 是否处于事务中
  std::string transaction_id_;   // 事务ID
  bool read_only_;               // 是否为只读执行

  // 执行统计信息
  size_t records_affected;   // 影响的行数（兼容旧代码）
  size_t rows_affected_;     // 影响的行数
  size_t rows_returned_;     // 返回的行数
  size_t execution_time_ms_; // 执行时间（毫秒）

  // 执行计划相关
  bool used_index;                              // 是否使用了索引（兼容旧代码）
  std::string execution_plan;                   // 执行计划（兼容旧代码）
  bool used_index_;                             // 是否使用了索引
  std::string execution_plan_;                  // 执行计划
  std::string plan_details_;                    // 执行计划详情
  std::string optimized_plan_;                  // 优化后的执行计划
  bool query_optimized_;                        // 查询是否被优化
  std::vector<std::string> optimization_rules_; // 使用的优化规则
  std::string index_info_;                      // 索引使用详情
  double cost_estimate_;                        // 成本估算

  // 执行状态
  bool has_error_;            // 是否有错误
  std::string error_message_; // 错误信息

  // 管理器指针
  std::shared_ptr<DatabaseManager> db_manager;  // 数据库管理器（兼容旧代码）
  std::shared_ptr<UserManager> user_manager;    // 用户管理器（兼容旧代码）
  std::shared_ptr<SystemDatabase> system_db;    // 系统数据库（兼容旧代码）
  std::shared_ptr<DatabaseManager> db_manager_; // 数据库管理器
  std::shared_ptr<UserManager> user_manager_;   // 用户管理器
  std::shared_ptr<SystemDatabase> system_db_;   // 系统数据库

  // 权限验证器
  std::shared_ptr<PermissionValidator> permission_validator_;

public:
  /**
   * @brief 构造函数
   */
  ExecutionContext();

  /**
   * @brief 带参数的构造函数
   */
  ExecutionContext(const std::string &current_user,
                   const std::string &current_database = "",
                   bool is_transactional = false);

  /**
   * @brief 带管理器的构造函数（兼容旧代码）
   */
  ExecutionContext(std::shared_ptr<DatabaseManager> db_manager,
                   std::shared_ptr<UserManager> user_manager = nullptr,
                   std::shared_ptr<SystemDatabase> system_db = nullptr);

  /**
   * @brief 析构函数
   */
  ~ExecutionContext();

  // ==== 获取器和设置器 ====

  /**
   * @brief 获取当前用户名
   */
  const std::string &get_current_user() const;

  /**
   * @brief 设置当前用户名
   */
  void set_current_user(const std::string &user);

  /**
   * @brief 获取当前数据库名
   */
  const std::string &get_current_database() const;

  /**
   * @brief 设置当前数据库名
   */
  void set_current_database(const std::string &database);

  /**
   * @brief 检查是否处于事务中
   */
  bool is_transactional() const;

  /**
   * @brief 设置事务状态
   */
  void set_transactional(bool is_transactional);

  /**
   * @brief 获取事务ID
   */
  const std::string &get_transaction_id() const;

  /**
   * @brief 设置事务ID
   */
  void set_transaction_id(const std::string &transaction_id);

  /**
   * @brief 检查是否为只读执行
   */
  bool is_read_only() const;

  /**
   * @brief 设置只读状态
   */
  void set_read_only(bool read_only);

  /**
   * @brief 获取影响的行数
   */
  size_t get_rows_affected() const;

  /**
   * @brief 设置影响的行数
   */
  void set_rows_affected(size_t rows);

  /**
   * @brief 增加影响的行数
   */
  void increment_rows_affected(size_t rows = 1);

  /**
   * @brief 获取返回的行数
   */
  size_t get_rows_returned() const;

  /**
   * @brief 设置返回的行数
   */
  void set_rows_returned(size_t rows);

  /**
   * @brief 获取执行时间
   */
  size_t get_execution_time_ms() const;

  /**
   * @brief 设置执行时间
   */
  void set_execution_time_ms(size_t time);

  /**
   * @brief 检查是否使用了索引
   */
  bool is_used_index() const;

  /**
   * @brief 设置是否使用了索引
   */
  void set_used_index(bool used_index);

  /**
   * @brief 获取执行计划
   */
  const std::string &get_execution_plan() const;

  /**
   * @brief 设置执行计划
   */
  void set_execution_plan(const std::string &execution_plan);

  /**
   * @brief 获取执行计划详情
   */
  const std::string &get_plan_details() const;

  /**
   * @brief 设置执行计划详情
   */
  void set_plan_details(const std::string &plan_details);

  /**
   * @brief 获取优化后的执行计划
   */
  const std::string &get_optimized_plan() const;

  /**
   * @brief 设置优化后的执行计划
   */
  void set_optimized_plan(const std::string &optimized_plan);

  /**
   * @brief 检查查询是否被优化
   */
  bool is_query_optimized() const;

  /**
   * @brief 设置查询是否被优化
   */
  void set_query_optimized(bool query_optimized);

  /**
   * @brief 获取使用的优化规则
   */
  const std::vector<std::string> &get_optimization_rules() const;

  /**
   * @brief 设置使用的优化规则
   */
  void
  set_optimization_rules(const std::vector<std::string> &optimization_rules);

  /**
   * @brief 获取索引使用详情
   */
  const std::string &get_index_info() const;

  /**
   * @brief 设置索引使用详情
   */
  void set_index_info(const std::string &index_info);

  /**
   * @brief 获取成本估算
   */
  double get_cost_estimate() const;

  /**
   * @brief 设置成本估算
   */
  void set_cost_estimate(double cost_estimate);

  /**
   * @brief 检查是否有错误
   */
  bool has_error() const;

  /**
   * @brief 设置错误状态
   */
  void set_error(bool has_error, const std::string &error_message = "");

  /**
   * @brief 获取错误信息
   */
  const std::string &get_error_message() const;

  /**
   * @brief 清除错误状态
   */
  void clear_error();

  // ==== 管理器相关 ====

  /**
   * @brief 获取数据库管理器
   */
  std::shared_ptr<DatabaseManager> get_db_manager() const;

  /**
   * @brief 设置数据库管理器
   */
  void set_db_manager(std::shared_ptr<DatabaseManager> db_manager);

  /**
   * @brief 获取用户管理器
   */
  std::shared_ptr<UserManager> get_user_manager() const;

  /**
   * @brief 设置用户管理器
   */
  void set_user_manager(std::shared_ptr<UserManager> user_manager);

  /**
   * @brief 获取系统数据库
   */
  std::shared_ptr<SystemDatabase> get_system_db() const;

  /**
   * @brief 设置系统数据库
   */
  void set_system_db(std::shared_ptr<SystemDatabase> system_db);

  // ==== 权限验证相关 ====

  /**
   * @brief 设置权限验证器
   */
  void set_permission_validator(std::shared_ptr<PermissionValidator> validator);

  /**
   * @brief 获取权限验证器
   */
  std::shared_ptr<PermissionValidator> get_permission_validator() const;

  // ==== 上下文操作 ====

  /**
   * @brief 重置上下文
   */
  void reset();

  /**
   * @brief 复制上下文
   */
  std::shared_ptr<ExecutionContext> clone() const;

  // ==== 调试信息 ====

  /**
   * @brief 获取上下文的字符串表示
   */
  std::string to_string() const;
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_CONTEXT_H