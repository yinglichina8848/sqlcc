/**
 * @file unified_executor.h
 * @brief Defines the core SQL execution framework using the Strategy design pattern.
 *
 * @WHY
 * A database must execute many different types of SQL statements (e.g., DDL, DML, DCL). A single, monolithic
 * `execute` function with a giant `switch` statement would be unmaintainable, hard to test, and difficult to
 * extend with new statement types.
 *
 * The **Strategy Design Pattern** is the ideal solution. It allows us to:
 * 1.  **Decouple**: Separate the high-level execution algorithm (the "dispatcher") from the low-level implementation
 *     details of each statement type.
 * 2.  **Promote Single Responsibility**: Each strategy class is responsible for only one category of statements
 *     (e.g., `DDLExecutionStrategy` only handles DDL).
 * 3.  **Enhance Extensibility**: Adding support for new SQL statements becomes as simple as creating a new strategy
 *     class and registering it, without modifying the existing execution engine.
 *
 * @WHAT
 * This file defines the key components of this strategy-based execution engine:
 * 1.  **`ExecutionStrategy` (The Strategy Interface)**: An abstract base class that defines the common interface for all
 *     execution algorithms. It declares the essential `execute`, `checkPermission`, and `validate` methods that every
 *     concrete strategy must implement.
 * 2.  **Concrete Strategy Implementations**:
 *     - `DDLExecutionStrategy`: Handles Data Definition Language (CREATE, ALTER, DROP).
 *     - `DMLExecutionStrategy`: Handles Data Manipulation Language (SELECT, INSERT, UPDATE, DELETE).
 *     - `DCLExecutionStrategy`: Handles Data Control Language (GRANT, REVOKE).
 *     - `UtilityExecutionStrategy`: Handles utility commands (USE, SHOW).
 * 3.  **`UnifiedExecutor` (The Context/Dispatcher)**: The main execution engine. It holds a map of statement types
 *     to their corresponding strategy objects. When its `execute` method is called, it inspects the parsed AST
 *     statement, looks up the correct strategy in its map, and delegates the execution to that strategy.
 *
 * @HOW
 * 1.  An instance of `UnifiedExecutor` is created at system startup.
 * 2.  In its constructor, it populates a map (`strategies_`) where keys are statement types (e.g., `Statement::Type::CREATE`)
 *     and values are instances of the corresponding strategy (e.g., `std::make_unique<DDLExecutionStrategy>()`).
 * 3.  The SQL parser produces an Abstract Syntax Tree (AST) for a given SQL query, with the root being a subclass of `sql_parser::Statement`.
 * 4.  This `Statement` object is passed to the `UnifiedExecutor::execute` method.
 * 5.  The executor identifies the statement's type, retrieves the correct strategy object from its map, and calls that strategy's `execute` method, passing the statement and the current `ExecutionContext`.
 * 6.  The chosen strategy then handles the specific logic for executing that statement.
 */

#ifndef SQLCC_UNIFIED_EXECUTOR_H
#define SQLCC_UNIFIED_EXECUTOR_H

#include "../core/execution_context.h" // Centralized definition for ExecutionContext
#include "execution_engine.h"
#include "execution_plan_generator.h"
#include "query_optimizer.h"
#include "../sql_parser/ast/ast_nodes.h"
#include "../core/system_database.h"
#include "../core/user_manager.h"
#include <functional>
#include <map>
#include <memory>
#include <unordered_map>

namespace sqlcc {

/**
 * @brief The abstract "Strategy" interface for executing different categories of SQL statements.
 *
 * @details This class defines the contract that all concrete execution strategies must follow. It ensures
 * that the `UnifiedExecutor` can treat all strategies uniformly, regardless of the complexity of the
 * operations they perform.
 */
class ExecutionStrategy {
public:
  virtual ~ExecutionStrategy() = default;

  /**
   * @brief Executes the given SQL statement. This is the core method of the strategy pattern.
   * @param stmt A unique_ptr to the parsed AST statement. The strategy takes ownership.
   * @param context The execution context (transaction, user, etc.) for this operation.
   * @return An ExecutionResult containing the outcome.
   */
  virtual ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                                  ExecutionContext &context) = 0;

  /**
   * @brief Checks if the user in the context has permission to execute the statement.
   * @param stmt A const reference to the statement (ownership is not transferred).
   * @param context The execution context containing user and resource information.
   * @return True if permission is granted, false otherwise.
   */
  virtual bool checkPermission(const sql_parser::Statement& stmt,
                               const ExecutionContext &context) {
    // Default implementation allows all operations. Concrete strategies should override this.
    return true;
  }

  /**
   * @brief Validates the statement against the current database schema and context.
   * @param stmt A const reference to the statement.
   * @param context The execution context.
   * @return True if the statement is valid (e.g., table exists, columns are correct), false otherwise.
   */
  virtual bool validate(const sql_parser::Statement& stmt,
                        const ExecutionContext &context) {
    // Default implementation considers all statements valid. Concrete strategies should override this.
    return true;
  }

protected:
  // Common helper methods available to all concrete strategies.

  /**
   * @brief Helper to verify that a database is currently selected.
   * @param context The current execution context.
   * @return True if a database is selected, false otherwise.
   */
  bool validateDatabaseContext(const ExecutionContext &context);

  /**
   * @brief Helper to verify that a table exists in the current database.
   * @param table_name The name of the table to check.
   * @param context The current execution context.
   * @return True if the table exists, false otherwise.
   */
  bool validateTableExists(const std::string &table_name,
                           const ExecutionContext &context);

  /**
   * @brief Updates execution statistics in the context.
   * @param context The context to update.
   * @param records_affected The number of records affected by the operation.
   */
  void updateExecutionStats(ExecutionContext &context, size_t records_affected);

  /**
   * @brief Provides a default permission check, typically for admin users.
   * @param context The current execution context.
   * @return True if the user has overriding permissions.
   */
  bool defaultPermissionCheck(const ExecutionContext &context);

  // Helper methods for DML operations
  bool matchesWhereClause(const std::vector<std::string> &record,
                          const sql_parser::WhereClause &where_clause,
                          std::shared_ptr<TableMetadata> metadata);

  std::string getColumnValue(const std::vector<std::string> &record,
                             const std::string &column_name,
                             std::shared_ptr<TableMetadata> metadata);

  bool compareValues(const std::string &left, const std::string &right,
                     const std::string &op);

  // Helper methods for constraint validation
  bool validateColumnConstraints(const std::vector<std::string> &record,
                                 std::shared_ptr<TableMetadata> metadata,
                                 const std::string &table_name);

  bool checkPrimaryKeyConstraints(const std::vector<std::string> &record,
                                  std::shared_ptr<TableMetadata> metadata,
                                  const std::string &table_name);

  bool checkUniqueKeyConstraints(const std::vector<std::string> &record,
                                 std::shared_ptr<TableMetadata> metadata,
                                 const std::string &table_name);

  // Helper methods for index maintenance
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

  // Granular permission check helpers
  bool checkCreatePermission(const sql_parser::CreateStatement& stmt,
                             const ExecutionContext& context);
  bool checkSelectPermission(const sql_parser::SelectStatement& stmt,
                             const ExecutionContext& context);
  // ... other permission helpers ...
};

/**
 * @brief Concrete Strategy for handling Data Definition Language (DDL) statements.
 * Responsible for CREATE, DROP, ALTER operations that modify schema.
 */
class DDLExecutionStrategy : public ExecutionStrategy {
public:
  DDLExecutionStrategy();
  ~DDLExecutionStrategy();

  ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                          ExecutionContext &context) override;

  bool checkPermission(const sql_parser::Statement& stmt,
                       const ExecutionContext &context) override;

  bool validate(const sql_parser::Statement& stmt,
                const ExecutionContext &context) override;

private:
  // DDL-specific execution methods
  ExecutionResult executeCreate(const sql_parser::CreateStatement& stmt,
                                ExecutionContext &context);
  // ... other DDL methods ...
};

/**
 * @brief Concrete Strategy for handling Data Manipulation Language (DML) statements.
 * Responsible for SELECT, INSERT, UPDATE, DELETE operations that modify data.
 */
class DMLExecutionStrategy : public ExecutionStrategy {
public:
  ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                          ExecutionContext &context) override;

  // ... other DML-specific overrides and methods ...
};

/**
 * @brief Concrete Strategy for handling Data Control Language (DCL) statements.
 * Responsible for GRANT, REVOKE operations that manage permissions.
 */
class DCLExecutionStrategy : public ExecutionStrategy {
public:
  ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                          ExecutionContext &context) override;

  // ... other DCL-specific overrides and methods ...
};

/**
 * @brief Concrete Strategy for handling utility statements.
 * Responsible for USE, SHOW, and other non-data, non-schema commands.
 */
class UtilityExecutionStrategy : public ExecutionStrategy {
public:
  ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                          ExecutionContext &context) override;
  // ... other utility-specific overrides and methods ...
};

/**
 * @brief A helper engine specifically for processing aggregate functions (COUNT, SUM, AVG, etc.).
 * Used by the `DMLExecutionStrategy` during SELECT queries.
 */
class AggregateEngine {
  // ... implementation ...
};

/**
 * @brief A helper engine specifically for processing GROUP BY clauses.
 * Works in tandem with the AggregateEngine.
 */
class GroupByExecutor {
  // ... implementation ...
};

/**
 * @brief Represents a query execution plan, typically generated by the QueryOptimizer.
 * This struct describes the high-level steps the engine will take to execute a query.
 */
struct ExecutionPlan {
  enum Type { FULL_TABLE_SCAN, INDEX_SCAN, INDEX_SEEK, JOIN, AGGREGATE, SORT };
  // ... implementation ...
  std::string toString() const;
};

/**
 * @brief The primary execution engine, acting as the "Context" in the Strategy pattern.
 *
 * @details This class orchestrates the entire execution process. It does not know how to
 * execute any specific statement; instead, it holds a collection of `ExecutionStrategy`
 * objects and delegates the work to the correct one based on the parsed statement's type.
 */
class UnifiedExecutor : public ExecutionEngine {
public:
  UnifiedExecutor(std::shared_ptr<DatabaseManager> db_manager,
                  std::shared_ptr<UserManager> user_manager,
                  std::shared_ptr<SystemDatabase> system_db);

  ~UnifiedExecutor() override;

  /**
   * @brief Main entry point for executing any SQL statement.
   * @param stmt The parsed AST of the SQL statement.
   * @return The result of the execution.
   */
  ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt) override;

  /**
   * @brief Overloaded execute method that takes an explicit execution context.
   * @param stmt The parsed AST of the SQL statement.
   * @param context A shared pointer to the execution context for this query.
   * @return The result of the execution.
   */
  ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt,
                          std::shared_ptr<ExecutionContext> context);

private:
  std::shared_ptr<DatabaseManager> db_manager_;
  std::shared_ptr<UserManager> user_manager_;
  std::shared_ptr<SystemDatabase> system_db_;

  // The map that holds the registered strategies. This is the core of the Strategy pattern.
  std::unordered_map<sql_parser::Statement::Type,
                     std::unique_ptr<ExecutionStrategy>>
      strategies_;

  ExecutionContext last_context_;
  std::unique_ptr<ExecutionPlanGenerator> plan_generator_;
  std::unique_ptr<QueryOptimizer> query_optimizer_;

  // Initializes the strategies_ map.
  void initializeStrategies();

  // Retrieves the correct strategy for a given statement type.
  ExecutionStrategy *getStrategy(sql_parser::Statement::Type type);

  // Global pre-execution checks.
  bool checkGlobalPermission(const sql_parser::Statement& stmt,
                             ExecutionContext& context);
  bool validateGlobalContext(const sql_parser::Statement& stmt,
                             ExecutionContext& context);
};

// ... content of AdvancedExecutor etc. ...

} // namespace sqlcc

#endif // SQLCC_UNIFIED_EXECUTOR_H