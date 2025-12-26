#include "sql_parser/ast_node.h"
#ifndef SQLCC_EXECUTION_ENGINE_H
#define SQLCC_EXECUTION_ENGINE_H

#include "core/execution_context.h" // 包含ExecutionContext定义
#include "core/execution_result.h"  // 包含完整的ExecutionResult定义
#include "core/system_database.h"
#include "core/user_manager.h"
#include "sql_parser/ast_nodes.h"
#include "storage/b_plus_tree.h"
#include "storage/table_storage.h"
#include "storage_engine.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sqlcc {

class DatabaseManager;

// ExecutionResult结构体已在execution_result.h中定义

/**
 * @brief 执行引擎接口
 */
class ExecutionEngine {
protected:
  std::shared_ptr<DatabaseManager> db_manager_;
  std::shared_ptr<ExecutionContext> execution_context_; // 执行上下文

public:
  ExecutionEngine(std::shared_ptr<DatabaseManager> db_manager);
  virtual ~ExecutionEngine() = default;

  /**
   * 执行SQL语句
   */
  virtual ExecutionResult
  execute(std::unique_ptr<sql_parser::Statement> stmt) = 0;

  /**
   * 执行SQL语句，带执行上下文
   * 默认实现使用默认执行上下文
   */
  virtual ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                                  std::shared_ptr<ExecutionContext> context) {
    // 默认实现：调用无上下文版本
    return execute(std::move(stmt));
  }

  /**
   * 设置执行上下文
   */
  virtual void set_execution_context(std::shared_ptr<ExecutionContext> context);

  /**
   * 获取执行上下文
   */
  virtual std::shared_ptr<ExecutionContext> get_execution_context() const;
};

/**
 * @brief DDL执行器 - 处理数据定义语言
 */
class DDLExecutor : public ExecutionEngine {
public:
  DDLExecutor(std::shared_ptr<DatabaseManager> db_manager);
  DDLExecutor(std::shared_ptr<DatabaseManager> db_manager,
              std::shared_ptr<SystemDatabase> system_db,
              std::shared_ptr<UserManager> user_manager);

  ExecutionResult
  execute(std::unique_ptr<sqlcc::sql_parser::Statement> stmt) override;

private:
  ExecutionResult executeCreate(std::unique_ptr<sqlcc::sql_parser::CreateStatement> stmt);
  ExecutionResult executeDrop(std::unique_ptr<sqlcc::sql_parser::DropStatement> stmt);
  ExecutionResult executeAlter(std::unique_ptr<sqlcc::sql_parser::AlterStatement> stmt);
  ExecutionResult
  executeCreateIndex(std::unique_ptr<sqlcc::sql_parser::CreateIndexStatement> stmt);
  ExecutionResult executeDropIndex(std::unique_ptr<sqlcc::sql_parser::DropIndexStatement> stmt);

  // 权限检查
  bool checkDDLPermission(const std::string &operation,
                          const std::string &resource);

  std::shared_ptr<SystemDatabase> system_db_;
  std::shared_ptr<UserManager> user_manager_;
};

/**
 * @brief DML执行器 - 处理数据操作语言
 */
class DMLExecutor : public ExecutionEngine {
public:
  DMLExecutor(std::shared_ptr<DatabaseManager> db_manager);
  DMLExecutor(std::shared_ptr<DatabaseManager> db_manager,
              std::shared_ptr<UserManager> user_manager);

  ExecutionResult
  execute(std::unique_ptr<sqlcc::sql_parser::Statement> stmt) override;

  // 公开的辅助方法（用于WHERE条件评估，可以被外部访问）
  bool compareValues(const std::string &left, const std::string &right,
                     const std::string &op);

public:
  // 索引优化查询方法 (设为公开以便测试)
  std::vector<std::pair<int32_t, size_t>>
  optimizeQueryWithIndex(const std::string &table_name,
                         const sql_parser::WhereClause &where_clause,
                         TableStorageManager &table_storage, bool &used_index,
                         std::string &index_info);

private:
  ExecutionResult executeInsert(std::unique_ptr<sqlcc::sql_parser::InsertStatement> stmt);
  ExecutionResult executeUpdate(std::unique_ptr<sqlcc::sql_parser::UpdateStatement> stmt);
  ExecutionResult executeDelete(std::unique_ptr<sqlcc::sql_parser::DeleteStatement> stmt);

  // 权限检查
  bool checkDMLPermission(const std::string &operation,
                          const std::string &table_name);

  // 辅助方法
  bool matchesWhereClause(const std::vector<std::string> &record,
                          const sqlcc::sql_parser::WhereClause &where_clause,
                          std::shared_ptr<TableMetadata> metadata);
  std::string getColumnValue(const std::vector<std::string> &record,
                             const std::string &column_name,
                             std::shared_ptr<TableMetadata> metadata);

  // WHERE条件评估辅助方法
  // TODO: 支持AND/OR组合条件
  // TODO: 支持IN操作符
  // TODO: 支持BETWEEN操作符
  // TODO: 支持LIKE模式匹配

  // 约束验证方法
  bool validateColumnConstraints(const std::vector<std::string> &record,
                                 std::shared_ptr<TableMetadata> metadata,
                                 const std::string &table_name);
  bool checkUniqueConstraints(const std::vector<std::string> &record,
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
                               size_t offset);
  void maintainIndexesOnUpdate(const std::vector<std::string> &old_record,
                               const std::vector<std::string> &new_record,
                               const std::string &table_name, int32_t page_id,
                               size_t offset);
  void maintainIndexesOnDelete(const std::vector<std::string> &record,
                               const std::string &table_name, int32_t page_id,
                               size_t offset);

  std::shared_ptr<UserManager> user_manager_;
};

/**
 * @brief DCL执行器 - 处理数据控制语言
 */
class DCLExecutor : public ExecutionEngine {
public:
  DCLExecutor(std::shared_ptr<DatabaseManager> db_manager,
              std::shared_ptr<UserManager> user_manager);

  ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt) override;

private:
  ExecutionResult executeCreateUser(std::unique_ptr<sql_parser::CreateUserStatement> stmt);
  ExecutionResult executeDropUser(std::unique_ptr<sql_parser::DropUserStatement> stmt);
  ExecutionResult executeGrant(std::unique_ptr<sql_parser::GrantStatement> stmt);
  ExecutionResult executeRevoke(std::unique_ptr<sql_parser::RevokeStatement> stmt);

  std::shared_ptr<UserManager> user_manager_;
};

/**
 * @brief 工具执行器 - 处理USE, SHOW等语句
 */
class UtilityExecutor : public ExecutionEngine {
public:
  UtilityExecutor(std::shared_ptr<DatabaseManager> db_manager);
  UtilityExecutor(std::shared_ptr<DatabaseManager> db_manager,
                  std::shared_ptr<SystemDatabase> system_db);

  ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt) override;

private:
  ExecutionResult executeShow(std::unique_ptr<sql_parser::ShowStatement> stmt);
  std::string formatDatabases(const std::vector<std::string> &databases);
  std::string formatTables(const std::vector<std::string> &tables);

  std::shared_ptr<SystemDatabase> system_db_;
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_ENGINE_H