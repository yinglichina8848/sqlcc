/**
 * @file unified_executor.cpp
 * @brief Implements the UnifiedExecutor, the core dispatcher for the SQL execution engine.
 *
 * @WHY
 * This file provides the concrete implementation of the Strategy design pattern outlined in the header.
 * While the header defines the "what" (the interfaces and classes), this file defines the "how" (the logic
 * that connects them). It's responsible for the practical wiring of the execution framework.
 *
 * @WHAT
 * This file implements:
 * 1.  `UnifiedExecutor::initializeStrategies()`: The method that populates the strategy map, creating the crucial
 *     link between a parsed statement's type and the concrete strategy object that knows how to execute it.
 * 2.  `UnifiedExecutor::execute()`: The primary entry point and orchestration method. It acts as a pipeline,
 *     performing a series of validations and checks before finally dispatching the statement to the appropriate strategy.
 * 3.  Helper methods for retrieving strategies and performing global checks.
 *
 * @HOW
 * The execution flow is orchestrated within the `execute` method:
 * 1.  **Context & Statement Validation**: It first ensures that the statement and execution context are valid.
 * 2.  **Global Checks**: It performs high-level permission and validation checks that apply to all statements.
 * 3.  **Strategy Lookup**: It retrieves the statement's type and uses it as a key to find the corresponding
 *     `ExecutionStrategy` object in the `strategies_` map.
 * 4.  **Strategy-Specific Checks**: It calls the chosen strategy's own `checkPermission` and `validate` methods. This
 *     allows for more granular checks (e.g., a `DMLExecutionStrategy` checking for table-level SELECT permissions).
 * 5.  **Delegation**: If all checks pass, it calls the strategy's `execute` method, handing off control for the
 *     actual execution.
 * 6.  **Post-Execution**: It records the final execution context for statistics and debugging purposes.
 *
 * This implementation ensures that the `UnifiedExecutor` remains a clean, high-level orchestrator, while the
 * complexities of each statement type are neatly encapsulated within their respective strategy classes.
 */

#include "unified_executor.h"
#include "core/execution_result.h"
#include "core/execution_context.h"
#include "execution_engine.h"
#include "core/core_database_manager.h"
#include "core/user_manager.h"
#include "core/system_database.h"
#include "execution_plan_generator.h"
#include "query_optimizer.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace sqlcc {

UnifiedExecutor::UnifiedExecutor(std::shared_ptr<DatabaseManager> db_manager)
    : ExecutionEngine(db_manager), db_manager_(db_manager) {
    initializeStrategies();
    initializeOptimizer();
}

UnifiedExecutor::UnifiedExecutor(std::shared_ptr<DatabaseManager> db_manager,
                                 std::shared_ptr<UserManager> user_manager,
                                 std::shared_ptr<SystemDatabase> system_db)
    : ExecutionEngine(db_manager), db_manager_(db_manager), user_manager_(user_manager), system_db_(system_db) {
    initializeStrategies();
    initializeOptimizer();
}

UnifiedExecutor::~UnifiedExecutor() = default;

void UnifiedExecutor::initializeStrategies() {
    // This method populates the strategy map, which is the core of the Strategy pattern.
    // It maps an abstract statement type to a concrete object responsible for its execution.
    
    // DDL Strategies
    strategies_[sql_parser::Statement::Type::CREATE] = std::make_unique<DDLExecutionStrategy>();
    strategies_[sql_parser::Statement::Type::DROP] = std::make_unique<DDLExecutionStrategy>();
    strategies_[sql_parser::Statement::Type::ALTER] = std::make_unique<DDLExecutionStrategy>();
    
    // DML Strategies
    strategies_[sql_parser::Statement::Type::SELECT] = std::make_unique<DMLExecutionStrategy>();
    strategies_[sql_parser::Statement::Type::INSERT] = std::make_unique<DMLExecutionStrategy>();
    strategies_[sql_parser::Statement::Type::UPDATE] = std::make_unique<DMLExecutionStrategy>();
    strategies_[sql_parser::Statement::Type::DELETE] = std::make_unique<DMLExecutionStrategy>();
    
    // NOTE: The following strategies are commented out because their corresponding `Statement::Type`
    // enums are not yet fully integrated into the parser. This is a known technical debt.
    // Once the parser is updated, these lines can be enabled to extend the executor's capabilities.
    // strategies_[sql_parser::Statement::Type::GRANT] = std::make_unique<DCLExecutionStrategy>();
    // strategies_[sql_parser::Statement::Type::REVOKE] = std::make_unique<DCLExecutionStrategy>();
    // strategies_[sql_parser::Statement::Type::USE] = std::make_unique<UtilityExecutionStrategy>();
    // strategies_[sql_parser::Statement::Type::SHOW] = std::make_unique<UtilityExecutionStrategy>();
    // strategies_[sql_parser::Statement::Type::DESCRIBE] = std::make_unique<UtilityExecutionStrategy>();
}

void UnifiedExecutor::initializeOptimizer() {
    // This is a placeholder for initializing the query optimizer and plan generator.
    // In a complete implementation, this would involve creating concrete instances of these
    // components, which would then be used within the strategies (especially DML).
    plan_generator_ = nullptr;
    query_optimizer_ = nullptr;
}

ExecutionResult UnifiedExecutor::execute(std::unique_ptr<sql_parser::Statement> stmt) {
    // This is a convenience overload. It creates a default ExecutionContext and calls
    // the primary `execute` method.
    ExecutionContext context;
    context.db_manager = db_manager_;
    context.user_manager = user_manager_;
    context.system_db = system_db_;
    
    return execute(std::move(stmt), std::make_shared<ExecutionContext>(context));
}

ExecutionResult UnifiedExecutor::execute(std::unique_ptr<sql_parser::Statement> stmt,
                                        std::shared_ptr<ExecutionContext> context) {
    // --- Execution Pipeline ---

    // Step 1: Basic sanity checks.
    if (!stmt) {
        return {false, "Statement is null"};
    }
    
    if (!context) {
        return {false, "ExecutionContext is null"};
    }
    
    // Step 2: Perform global permission checks that apply to all statement types.
    // This might include checks for system-level access or maintenance mode.
    if (!checkGlobalPermission(*stmt, *context)) {
        return {false, "Global permission check failed for statement type: " + 
                       std::to_string(static_cast<int>(stmt->getType()))};
    }
    
    // Step 3: Get the statement type from the parsed AST node.
    auto stmt_type = stmt->getType();
    
    // Step 4: Look up the appropriate strategy object from our map.
    ExecutionStrategy* strategy = getStrategy(stmt_type);
    if (!strategy) {
        return {false, "No strategy found for statement type: " + std::to_string(static_cast<int>(stmt_type))};
    }
    
    // Step 5: Delegate permission checking to the chosen strategy.
    // This allows for fine-grained checks (e.g., table-level permissions for DML).
    if (!strategy->checkPermission(*stmt, *context)) {
        return {false, "Permission denied for statement type: " + std::to_string(static_cast<int>(stmt_type))};
    }
    
    // Step 6: Delegate validation to the chosen strategy.
    // This allows for context-aware validation (e.g., ensuring a table exists before a SELECT).
    if (!strategy->validate(*stmt, *context)) {
        return {false, "Validation failed for statement type: " + std::to_string(static_cast<int>(stmt_type))};
    }
    
    // Step 7: All checks passed. Delegate the actual execution to the strategy.
    ExecutionResult result = strategy->execute(std::move(stmt), *context);
    
    // Step 8: Store the context of the last execution for debugging and statistics.
    last_context_ = *context;
    
    return result;
}

ExecutionStrategy* UnifiedExecutor::getStrategy(sql_parser::Statement::Type type) {
    auto it = strategies_.find(type);
    if (it != strategies_.end()) {
        return it->second.get();
    }
    // Return nullptr if no strategy is registered for this statement type.
    return nullptr;
}

bool UnifiedExecutor::checkGlobalPermission(const sql_parser::Statement& stmt,
                                           ExecutionContext& context) {
    // This is a placeholder for global permission logic.
    // For example, you could check if the database is in a read-only state
    // and block all DML/DDL operations.
    // For now, we default to allowing the operation to proceed to the next stage.
    return true;
}

bool UnifiedExecutor::validateGlobalContext(const sql_parser::Statement& stmt,
                                          ExecutionContext& context) {
    // This is a placeholder for global context validation.
    // It ensures that essential system components are available.
    if (!context.db_manager) {
        return false;
    }
    // In a real system, you might also check for a valid transaction manager, etc.
    return true;
}

} // namespace sqlcc