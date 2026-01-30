#include "src/sql_parser/ast/ast_node.h"
#ifndef SQLCC_UNIFIED_EXECUTOR_H
#define SQLCC_UNIFIED_EXECUTOR_H

#include "src/core/execution_context.h" // 使用统一的ExecutionContext定义
#include "src/execution_engine.h"
#include "src/sql_parser/ast/ast_nodes.h"
#include "src/core/system_database.h"
#include "src/core/user_manager.h"

// Include execution strategy headers
#include "src/execution/execution_strategy.h"
#include "src/execution/ddl_execution_strategy.h"
#include "src/execution/dml_execution_strategy.h"
#include "src/execution/dcl_execution_strategy.h"
#include "src/execution/utility_execution_strategy.h"
#include "src/execution/aggregate_engine.h"
#include "src/execution/group_by_executor.h"
#include "src/execution/execution_plan_generator.h"
#include "src/execution/query_optimizer.h"

#include <functional>
#include <map>
#include <memory>
#include <unordered_map>

namespace sqlcc {

/**
 * @brief 执行计划
 * 描述查询的执行方式
 */
struct ExecutionPlan {
  enum Type { FULL_TABLE_SCAN, INDEX_SCAN, INDEX_SEEK, JOIN, AGGREGATE, SORT };

  Type type;
  std::string description;
  std::string table_name;
  std::string index_name;
  std::vector<std::string> columns;
  std::string where_clause;
  double cost_estimate;
  bool is_optimized;

  // 生成执行计划描述
  std::string toString() const;
};

/**
 * @brief 统一执行器
 * 使用策略模式统一处理所有类型的SQL语句
 */
class UnifiedExecutor : public ExecutionEngine {
public:
  UnifiedExecutor(std::shared_ptr<DatabaseManager> db_manager);
  UnifiedExecutor(std::shared_ptr<DatabaseManager> db_manager,
                  std::shared_ptr<UserManager> user_manager,
                  std::shared_ptr<SystemDatabase> system_db);

  // 获取DatabaseManager
  std::shared_ptr<DatabaseManager> getDatabaseManager() const { return db_manager_; }

  ~UnifiedExecutor() override;

  ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt) override;

  /**
   * @brief 执行SQL语句，带有执行上下文
   * @param stmt 要执行的语句
   * @param context 执行上下文
   * @return 执行结果
   */
  ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                          std::shared_ptr<ExecutionContext> context);

  // 获取执行统计信息
  const ExecutionContext &getLastExecutionContext() const {
    return last_context_;
  }

private:
  // 数据库管理器
  std::shared_ptr<DatabaseManager> db_manager_;

  // 用户管理器
  std::shared_ptr<UserManager> user_manager_;

  // 系统数据库
  std::shared_ptr<SystemDatabase> system_db_;

  // 策略映射
  std::unordered_map<sql_parser::Statement::Type,
                     std::unique_ptr<ExecutionStrategy>>
      strategies_;

  // 最后一次执行上下文
  ExecutionContext last_context_;

  // 执行计划生成器
  std::unique_ptr<ExecutionPlanGenerator> plan_generator_;

  // 查询优化器
  std::unique_ptr<QueryOptimizer> query_optimizer_;

  // 初始化策略
  void initializeStrategies();

  // 初始化执行计划生成器和查询优化器
  void initializeOptimizer();

  // 获取语句对应的策略
  ExecutionStrategy *getStrategy(sql_parser::Statement::Type type);

  // 权限检查统一入口
  bool checkGlobalPermission(const sql_parser::Statement& stmt,
                             ExecutionContext& context);

  // 上下文验证统一入口
  bool validateGlobalContext(const sql_parser::Statement& stmt,
                             ExecutionContext& context);
};

/**
 * @brief 高级执行器 - 支持复杂查询的执行器
 * 为未来的JOIN、子查询、窗口函数等高级功能预留接口
 */
class AdvancedExecutor : public UnifiedExecutor {
public:
  AdvancedExecutor(std::shared_ptr<DatabaseManager> db_manager);
  AdvancedExecutor(std::shared_ptr<DatabaseManager> db_manager,
                   std::shared_ptr<UserManager> user_manager,
                   std::shared_ptr<SystemDatabase> system_db);

  // 高级查询支持
  // TODO: 添加JOIN、子查询、窗口函数等高级功能
};

} // namespace sqlcc

#endif // SQLCC_UNIFIED_EXECUTOR_H