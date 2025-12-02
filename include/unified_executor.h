#ifndef SQLCC_UNIFIED_EXECUTOR_H
#define SQLCC_UNIFIED_EXECUTOR_H

#include "execution_context.h" // 使用统一的ExecutionContext定义
#include "execution_engine.h"
#include "sql_parser/ast_nodes.h"
#include "system_database.h"
#include "user_manager.h"
#include <functional>
#include <memory>
#include <unordered_map>

namespace sqlcc {

// ExecutionContext 已在 execution_context.h 中定义

/**
 * @brief 执行策略接口
 * 定义不同类型语句的执行策略
 */
class ExecutionStrategy {
public:
  virtual ~ExecutionStrategy() = default;

  /**
   * 执行语句
   * @param stmt 要执行的语句
   * @param context 执行上下文
   * @return 执行结果
   */
  virtual ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                                  ExecutionContext &context) = 0;

  /**
   * 检查执行权限
   * @param stmt 要执行的语句
   * @param context 执行上下文
   * @return 是否有权限执行
   */
  virtual bool checkPermission(const sql_parser::Statement *stmt,
                               const ExecutionContext &context);

  /**
   * 验证语句和上下文
   * @param stmt 要执行的语句
   * @param context 执行上下文
   * @return 验证结果
   */
  virtual bool validate(const sql_parser::Statement *stmt,
                        const ExecutionContext &context);

protected:
  /**
   * 验证数据库上下文
   * @param context 执行上下文
   * @return 验证结果
   */
  bool validateDatabaseContext(const ExecutionContext &context);

  /**
   * 验证表是否存在
   * @param table_name 表名
   * @param context 执行上下文
   * @return 验证结果
   */
  bool validateTableExists(const std::string &table_name,
                           const ExecutionContext &context);

  /**
   * 更新执行统计信息
   * @param context 执行上下文
   * @param records_affected 影响的记录数
   */
  void updateExecutionStats(ExecutionContext &context, size_t records_affected);

  /**
   * 生成默认权限检查结果
   * @param context 执行上下文
   * @return 默认权限检查结果
   */
  bool defaultPermissionCheck(const ExecutionContext &context);

  // 辅助方法
  bool matchesWhereClause(const std::vector<std::string> &record,
                          const sql_parser::WhereClause &where_clause,
                          std::shared_ptr<TableMetadata> metadata);

  std::string getColumnValue(const std::vector<std::string> &record,
                             const std::string &column_name,
                             std::shared_ptr<TableMetadata> metadata);

  bool compareValues(const std::string &left, const std::string &right,
                     const std::string &op);

  // 约束验证方法
  bool validateColumnConstraints(const std::vector<std::string> &record,
                                 std::shared_ptr<TableMetadata> metadata,
                                 const std::string &table_name);

  bool checkPrimaryKeyConstraints(const std::vector<std::string> &record,
                                  std::shared_ptr<TableMetadata> metadata,
                                  const std::string &table_name);

  bool checkUniqueKeyConstraints(const std::vector<std::string> &record,
                                 std::shared_ptr<TableMetadata> metadata,
                                 const std::string &table_name);

  // 索引维护方法
  void maintainIndexesOnInsert(const std::vector<std::string> &record,
                               const std::string &table_name, int32_t page_id,
                               size_t offset, ExecutionContext &context);

  void maintainIndexesOnUpdate(const std::vector<std::string> &old_record,
                               const std::vector<std::string> &new_record,
                               const std::string &table_name, int32_t page_id,
                               size_t offset, ExecutionContext &context);

  void maintainIndexesOnDelete(const std::vector<std::string> &record,
                               const std::string &table_name, int32_t page_id,
                               size_t offset, ExecutionContext &context);

  // 权限检查辅助方法
  bool checkCreatePermission(const sql_parser::CreateStatement *stmt,
                             const ExecutionContext &context);
  bool checkSelectPermission(const sql_parser::SelectStatement *stmt,
                             const ExecutionContext &context);
  bool checkInsertPermission(const sql_parser::InsertStatement *stmt,
                             const ExecutionContext &context);
  bool checkUpdatePermission(const sql_parser::UpdateStatement *stmt,
                             const ExecutionContext &context);
  bool checkDeletePermission(const sql_parser::DeleteStatement *stmt,
                             const ExecutionContext &context);
  bool checkDropPermission(const sql_parser::DropStatement *stmt,
                           const ExecutionContext &context);
  bool checkAlterPermission(const sql_parser::AlterStatement *stmt,
                            const ExecutionContext &context);
  bool checkUsePermission(const sql_parser::UseStatement *stmt,
                          const ExecutionContext &context);
  bool checkCreateIndexPermission(const sql_parser::CreateIndexStatement *stmt,
                                  const ExecutionContext &context);
  bool checkDropIndexPermission(const sql_parser::DropIndexStatement *stmt,
                                const ExecutionContext &context);
  bool checkCreateUserPermission(const sql_parser::CreateUserStatement *stmt,
                                 const ExecutionContext &context);
  bool checkDropUserPermission(const sql_parser::DropUserStatement *stmt,
                               const ExecutionContext &context);
  bool checkGrantPermission(const sql_parser::GrantStatement *stmt,
                            const ExecutionContext &context);
  bool checkRevokePermission(const sql_parser::RevokeStatement *stmt,
                             const ExecutionContext &context);
  bool checkShowPermission(const sql_parser::ShowStatement *stmt,
                           const ExecutionContext &context);
};

/**
 * @brief DDL执行策略 - 处理数据定义语言
 */
class DDLExecutionStrategy : public ExecutionStrategy {
public:
  ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                          ExecutionContext &context) override;

  bool checkPermission(const sql_parser::Statement *stmt,
                       const ExecutionContext &context) override;

  bool validate(const sql_parser::Statement *stmt,
                const ExecutionContext &context) override;

private:
  ExecutionResult executeCreate(sql_parser::CreateStatement *stmt,
                                ExecutionContext &context);

  ExecutionResult executeDrop(sql_parser::DropStatement *stmt,
                              ExecutionContext &context);

  ExecutionResult executeAlter(sql_parser::AlterStatement *stmt,
                               ExecutionContext &context);

  ExecutionResult executeCreateIndex(sql_parser::CreateIndexStatement *stmt,
                                     ExecutionContext &context);

  ExecutionResult executeDropIndex(sql_parser::DropIndexStatement *stmt,
                                   ExecutionContext &context);
};

/**
 * @brief DML执行策略 - 处理数据操作语言
 */
class DMLExecutionStrategy : public ExecutionStrategy {
public:
  ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                          ExecutionContext &context) override;

  bool checkPermission(const sql_parser::Statement *stmt,
                       const ExecutionContext &context) override;

  bool validate(const sql_parser::Statement *stmt,
                const ExecutionContext &context) override;

  // 索引优化查询方法
  std::vector<std::pair<int32_t, size_t>>
  optimizeQueryWithIndex(const std::string &table_name,
                         const sql_parser::WhereClause &where_clause,
                         std::shared_ptr<StorageEngine> storage_engine,
                         bool &used_index, std::string &index_info);

private:
  ExecutionResult executeInsert(sql_parser::InsertStatement *stmt,
                                ExecutionContext &context);

  ExecutionResult executeUpdate(sql_parser::UpdateStatement *stmt,
                                ExecutionContext &context);

  ExecutionResult executeDelete(sql_parser::DeleteStatement *stmt,
                                ExecutionContext &context);

  ExecutionResult executeSelect(sql_parser::SelectStatement *stmt,
                                ExecutionContext &context);
};

/**
 * @brief DCL执行策略 - 处理数据控制语言
 */
class DCLExecutionStrategy : public ExecutionStrategy {
public:
  ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                          ExecutionContext &context) override;

  bool checkPermission(const sql_parser::Statement *stmt,
                       const ExecutionContext &context) override;

  bool validate(const sql_parser::Statement *stmt,
                const ExecutionContext &context) override;

private:
  ExecutionResult executeCreateUser(sql_parser::CreateUserStatement *stmt,
                                    ExecutionContext &context);

  ExecutionResult executeDropUser(sql_parser::DropUserStatement *stmt,
                                  ExecutionContext &context);

  ExecutionResult executeGrant(sql_parser::GrantStatement *stmt,
                               ExecutionContext &context);

  ExecutionResult executeRevoke(sql_parser::RevokeStatement *stmt,
                                ExecutionContext &context);
};

/**
 * @brief 工具执行策略 - 处理USE, SHOW等语句
 */
class UtilityExecutionStrategy : public ExecutionStrategy {
public:
  ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                          ExecutionContext &context) override;

  bool checkPermission(const sql_parser::Statement *stmt,
                       const ExecutionContext &context) override;

  bool validate(const sql_parser::Statement *stmt,
                const ExecutionContext &context) override;

private:
  ExecutionResult executeUse(sql_parser::UseStatement *stmt,
                             ExecutionContext &context);

  ExecutionResult executeShow(sql_parser::ShowStatement *stmt,
                              ExecutionContext &context);

  std::string formatDatabases(const std::vector<std::string> &databases);
  std::string formatTables(const std::vector<std::string> &tables);
};

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
 * @brief 执行计划生成器
 * 负责生成和优化执行计划
 */
class ExecutionPlanGenerator {
public:
  ExecutionPlanGenerator();
  ~ExecutionPlanGenerator() = default;

  // 生成执行计划
  ExecutionPlan generatePlan(const sql_parser::SelectStatement &stmt,
                             const ExecutionContext &context);

  // 优化执行计划
  ExecutionPlan optimizePlan(const ExecutionPlan &plan,
                             const ExecutionContext &context);

  // 评估执行计划成本
  double estimateCost(const ExecutionPlan &plan,
                      const ExecutionContext &context);

private:
  // 生成全表扫描计划
  ExecutionPlan
  generateFullTableScanPlan(const sql_parser::SelectStatement &stmt);

  // 生成索引扫描计划
  ExecutionPlan generateIndexScanPlan(const sql_parser::SelectStatement &stmt,
                                      const ExecutionContext &context);

  // 生成索引查找计划
  ExecutionPlan generateIndexSeekPlan(const sql_parser::SelectStatement &stmt,
                                      const ExecutionContext &context);
};

/**
 * @brief 查询优化器接口
 * 负责查询计划的优化
 */
class QueryOptimizer {
public:
  virtual ~QueryOptimizer() = default;

  // 优化查询计划
  virtual ExecutionPlan optimize(const ExecutionPlan &plan,
                                 const ExecutionContext &context) = 0;

  // 生成执行计划
  virtual ExecutionPlan generatePlan(const sql_parser::SelectStatement &stmt,
                                     const ExecutionContext &context) = 0;

  // 评估执行计划成本
  virtual double estimateCost(const ExecutionPlan &plan,
                              const ExecutionContext &context) = 0;

  // 获取优化规则
  virtual std::vector<std::string> getOptimizationRules() const = 0;

  // 启用/禁用特定优化规则
  virtual void enableRule(const std::string &rule_name) = 0;
  virtual void disableRule(const std::string &rule_name) = 0;

  // 检查特定优化规则是否启用
  virtual bool isRuleEnabled(const std::string &rule_name) const = 0;
};

/**
 * @brief 基于规则的查询优化器
 * 实现基于规则的查询优化
 */
class RuleBasedOptimizer : public QueryOptimizer {
public:
  RuleBasedOptimizer();
  ~RuleBasedOptimizer() override = default;

  // 优化查询计划
  ExecutionPlan optimize(const ExecutionPlan &plan,
                         const ExecutionContext &context) override;

  // 生成执行计划
  ExecutionPlan generatePlan(const sql_parser::SelectStatement &stmt,
                             const ExecutionContext &context) override;

  // 评估执行计划成本
  double estimateCost(const ExecutionPlan &plan,
                      const ExecutionContext &context) override;

  // 获取优化规则
  std::vector<std::string> getOptimizationRules() const override;

  // 启用/禁用特定优化规则
  void enableRule(const std::string &rule_name) override;
  void disableRule(const std::string &rule_name) override;

  // 检查特定优化规则是否启用
  bool isRuleEnabled(const std::string &rule_name) const override;

private:
  // 优化规则
  std::unordered_map<std::string, bool> optimization_rules_;

  // 执行计划生成器
  ExecutionPlanGenerator plan_generator_;
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
  bool checkGlobalPermission(const sql_parser::Statement *stmt,
                             ExecutionContext &context);

  // 上下文验证统一入口
  bool validateGlobalContext(const sql_parser::Statement *stmt,
                             ExecutionContext &context);
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
  ExecutionResult
  executeComplexQuery(std::unique_ptr<sql_parser::Statement> stmt);

  // JOIN查询支持
  ExecutionResult executeJoinQuery(sql_parser::SelectStatement *stmt);

  // 子查询支持
  ExecutionResult executeSubquery(sql_parser::SelectStatement *stmt);

  // 窗口函数支持
  ExecutionResult executeWindowFunction(sql_parser::SelectStatement *stmt);

private:
  // 查询优化器接口
  ExecutionResult
  optimizeAndExecute(std::unique_ptr<sql_parser::Statement> stmt);

  // 结果后处理
  ExecutionResult postProcessResult(ExecutionResult &&result,
                                    const ExecutionContext &context);
};

} // namespace sqlcc

#endif // SQLCC_UNIFIED_EXECUTOR_H
