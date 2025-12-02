#ifndef SQLCC_EXECUTION_CONTEXT_H
#define SQLCC_EXECUTION_CONTEXT_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sqlcc {

// 前向声明
class UserManager;
class DatabaseManager;
class SystemDatabase;
class PermissionValidator;

/**
 * @brief 执行上下文
 * 用于传递用户信息、当前数据库上下文和执行状态
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