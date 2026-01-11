#include "sql_parser/ast_node.h"
#ifndef SQLCC_SQL_EXECUTOR_H
#define SQLCC_SQL_EXECUTOR_H

#include "core/permission_validator.h"
#include "core/system_database.h"
#include "core/user_manager.h"
#include "core/core_database_manager.h"
#include "sql_parser/parser.h"
#include "sql_parser/parser_new.h"
#include "unified_query_plan.h"
#include "view_manager.h"
#include <memory>
#include <string>

namespace sqlcc {



/**
 * WHY: 为什么需要统一的SQL执行器而不是分离的DDL/DML执行器？
 *
 * 早期设计中DDL和DML执行器分离，导致：
 * - 查询计划不统一，优化机会丢失
 * - 错误处理不一致，用户体验差
 * - 代码重复，维护成本高
 *
 * 统一执行器优势：
 * 1. 单一入口：所有SQL语句通过统一接口
 * 2. 统一规划：基于成本的查询优化
 * 3. 一致性：错误处理和事务管理统一
 * 4. 可扩展：易于添加新SQL特性
 *
 * 架构设计：
 * - 解析器：将SQL转换为AST
 * - 查询规划器：生成最优执行计划
 * - 执行引擎：实际执行查询计划
 * - 存储引擎：管理数据持久化
 *
 * 🏗️ 设计模式：统一执行器架构设计
 *
 * 设计模式应用：
 * 1. 外观模式(Facade Pattern)：统一SQL执行接口
 *    - 隐藏底层复杂性（解析器、优化器、执行引擎）
 *    - 提供简洁一致的API
 *    - 解耦客户端和子系统
 *
 * 2. 管道模式(Pipeline Pattern)：SQL执行流水线
 *    - 解析 → 优化 → 执行 → 返回
 *    - 每个阶段职责单一
 *    - 支持插件化扩展
 *
 * 3. 策略模式(Strategy Pattern)：查询优化策略
 *    - 可插拔的优化规则
 *    - 运行时选择最优策略
 *    - 支持不同的执行计划
 *
 * SOLID原则体现：
 * - 单一职责：每个组件职责清晰
 * - 开闭原则：新功能通过扩展实现
 * - 依赖倒置：高层不依赖具体实现
 *
 * WHAT: SQL执行器类 - 重构版本
 *
 * 使用统一查询计划架构，整合DDL/DML/DCL执行器公共逻辑
 * 解决执行器分离过度、缺少统一查询计划、错误处理不一致的问题
 */
class SqlExecutor {
public:
  SqlExecutor();
  // 新增：接受DatabaseManager的构造函数，用于共享数据库实例
  SqlExecutor(std::shared_ptr<DatabaseManager> db_manager);
  ~SqlExecutor();

  /**
   * WHAT: Execute - SQL语句执行主入口
   *
   * 处理完整的SQL语句执行流程，从解析到结果返回。
   * 支持DDL、DML、DCL等多种SQL语句类型。
   *
   * HOW: SQL执行流水线
   * 1. 权限验证：检查用户执行权限
   * 2. 语法解析：将SQL转换为AST
   * 3. 查询规划：生成最优执行计划
   * 4. 计划执行：实际执行查询操作
   * 5. 结果返回：格式化并返回执行结果
   *
   * 执行流程：
   * - 单条语句：直接解析执行
   * - 多条语句：分批处理，保持事务一致性
   * - 错误处理：捕获异常，返回错误信息
   *
   * @param sql SQL语句字符串
   * @return 执行结果消息
   */
  std::string Execute(const std::string &sql);

  /**
   * WHAT: Execute - AST驱动SQL执行入口
   *
   * 直接接受解析后的AST节点，生成查询计划并执行。
   * 这是优化的执行路径，避免重复解析开销。
   *
   * HOW: AST驱动执行流程
   * 1. 验证AST节点有效性
   * 2. 创建执行上下文
   * 3. 生成查询计划
   * 4. 执行查询计划
   * 5. 返回执行结果
   *
   * @param stmt 解析后的AST语句节点
   * @return 执行结果消息
   */
  std::string Execute(const sqlcc::sql_parser::Statement* stmt);

  /**
   * @brief 执行文件中的SQL语句
   * @param file_path 文件路径
   * @return 执行结果消息
   */
  std::string ExecuteFile(const std::string &file_path);

  /**
   * @brief 验证语句的有效性
   * @param stmt 要验证的AST语句节点
   * @return 验证结果
   */
  bool validateStatement(const sqlcc::sql_parser::Statement* stmt);

  /**
   * @brief 检查语句是否需要事务支持
   * @param stmt 要检查的AST语句节点
   * @return 是否需要事务
   */
  bool requiresTransaction(const sqlcc::sql_parser::Statement* stmt);

  /**
   * @brief 与存储引擎集成的执行方法
   * @param stmt AST语句节点
   * @param context 执行上下文
   * @param pages_accessed 页面访问计数
   * @return 执行结果
   */
  ExecutionResult executeWithStorageEngine(
      const sqlcc::sql_parser::Statement* stmt,
      ExecutionContext& context,
      size_t& pages_accessed);

  /**
   * @brief 获取最后一次执行的错误信息
   * @return 错误信息
   */
  std::string GetLastError() const;

  /**
   * @brief 获取执行统计信息
   * @return 统计信息字符串
   */
  std::string GetExecutionStats() const;

private:
  std::shared_ptr<DatabaseManager> db_manager_;
  std::shared_ptr<UserManager> user_manager_;
  std::shared_ptr<SystemDatabase> system_db_;
  std::unique_ptr<ViewManager> view_manager_;
  std::unique_ptr<PermissionValidator> permission_validator_;
  std::shared_ptr<StorageEngine> storage_engine_;
  std::shared_ptr<TransactionManager> transaction_manager_;
  std::string last_error_;
  std::string execution_stats_;
  std::string current_user_;
  std::string current_database_;

  // 组件初始化
  void initializeComponents();

  /**
   * @brief 设置错误信息
   * @param error 错误信息
   */
  void SetError(const std::string &error);

  /**
   * @brief 清除错误信息
   */
  void ClearError();

  /**
   * @brief 初始化系统数据库
   */
  bool InitializeSystemDatabase();

  /**
   * @brief 解析SQL语句
   * @param sql SQL语句
   * @return 解析后的语句对象
   */
  std::unique_ptr<sql_parser::Statement> ParseSQL(const std::string &sql);

  /**
   * @brief 创建统一查询计划
   * @param stmt 解析后的语句
   * @return 查询计划对象
   */
  std::unique_ptr<UnifiedQueryPlan>
  CreateQueryPlan(std::unique_ptr<sql_parser::Statement> stmt);

  /**
   * @brief 初始化权限验证器
   */
  bool InitializePermissionValidator();

  /**
   * @brief 更新当前数据库
   * @param sql SQL语句
   */
  void UpdateCurrentDatabase(const std::string &sql);

  /**
   * @brief 去除字符串两端的空白字符
   * @param str 要处理的字符串
   */
  void TrimString(std::string &str);

  /**
   * @brief 执行语句的主要逻辑
   * @param sql SQL语句
   * @return 执行结果
   */
  std::string ExecuteStatement(const std::string& sql);

  // DDL语句处理方法
  std::string ExecuteCreateTable(const std::string& sql);
  std::string ExecuteCreateDatabase(const std::string& sql);
  std::string ExecuteDropTable(const std::string& sql);
  std::string ExecuteDropDatabase(const std::string& sql);
  std::string ExecuteAlterTable(const std::string& sql);
  std::string ExecuteCreateIndex(const std::string& sql);
  std::string ExecuteDropIndex(const std::string& sql);

  // DML语句处理方法
  std::string ExecuteInsert(const std::string& sql);
  std::string ExecuteUpdate(const std::string& sql);
  std::string ExecuteDelete(const std::string& sql);

  // DQL语句处理方法
  std::string ExecuteSelect(const std::string& sql);

  // DCL语句处理方法
  std::string ExecuteGrant(const std::string& sql);
  std::string ExecuteRevoke(const std::string& sql);
  std::string ExecuteCreateUser(const std::string& sql);
  std::string ExecuteDropUser(const std::string& sql);
};

} // namespace sqlcc

#endif // SQLCC_SQL_EXECUTOR_H