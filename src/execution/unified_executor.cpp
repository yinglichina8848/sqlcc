#include "../../backups/core_backup_20260121_001034/unified_executor.h"
#include "../../backups/core_backup_20260121_001034/execution_result.h"
#include "../../backups/core_backup_20260121_001034/execution_context.h"
#include "execution_engine.h"
#include "../../backups/core_backup_20260121_001034/core_database_manager.h"
#include "../../backups/core_backup_20260121_001034/user_manager.h"
#include "../../backups/core_backup_20260121_001034/system_database.h"
#include "../core/execution_strategy.h"
#include "ddl_execution_strategy.h"
#include "dml_execution_strategy.h"
#include "dcl_execution_strategy.h"
#include "utility_execution_strategy.h"
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
    // 创建各种执行策略实例
    strategies_[sql_parser::Statement::Type::CREATE] = std::make_unique<DDLExecutionStrategy>();
    strategies_[sql_parser::Statement::Type::DROP] = std::make_unique<DDLExecutionStrategy>();
    strategies_[sql_parser::Statement::Type::ALTER] = std::make_unique<DDLExecutionStrategy>();
    
    strategies_[sql_parser::Statement::Type::SELECT] = std::make_unique<DMLExecutionStrategy>();
    strategies_[sql_parser::Statement::Type::INSERT] = std::make_unique<DMLExecutionStrategy>();
    strategies_[sql_parser::Statement::Type::UPDATE] = std::make_unique<DMLExecutionStrategy>();
    strategies_[sql_parser::Statement::Type::DELETE] = std::make_unique<DMLExecutionStrategy>();
    
    // TODO: These statement types are not defined in Statement::Type yet
    // strategies_[sql_parser::Statement::Type::GRANT] = std::make_unique<DCLExecutionStrategy>();
    // strategies_[sql_parser::Statement::Type::REVOKE] = std::make_unique<DCLExecutionStrategy>();
    // strategies_[sql_parser::Statement::Type::USE] = std::make_unique<UtilityExecutionStrategy>();
    // strategies_[sql_parser::Statement::Type::SHOW] = std::make_unique<UtilityExecutionStrategy>();
    // strategies_[sql_parser::Statement::Type::DESCRIBE] = std::make_unique<UtilityExecutionStrategy>();
}

void UnifiedExecutor::initializeOptimizer() {
    // Since ExecutionPlanGenerator and QueryOptimizer are interfaces, we'll initialize them later
    // when concrete implementations are available
    // For now, we just ensure the members are properly declared in the header
}

ExecutionResult UnifiedExecutor::execute(std::unique_ptr<sql_parser::Statement> stmt) {
    ExecutionContext context;
    context.db_manager = db_manager_;
    context.user_manager = user_manager_;
    context.system_db = system_db_;
    
    return execute(std::move(stmt), std::make_shared<ExecutionContext>(context));
}

ExecutionResult UnifiedExecutor::execute(std::unique_ptr<sql_parser::Statement> stmt,
                                        std::shared_ptr<ExecutionContext> context) {
    if (!stmt) {
        return {false, "Statement is null"};
    }
    
    if (!context) {
        return {false, "ExecutionContext is null"};
    }
    
    // 检查全局权限
    if (!checkGlobalPermission(*stmt, *context)) {
        return {false, "Global permission check failed for statement type: " + 
                       std::to_string(static_cast<int>(stmt->getType()))};
    }
    
    // 获取语句类型
    auto stmt_type = stmt->getType();
    
    // 获取对应的策略
    ExecutionStrategy* strategy = getStrategy(stmt_type);
    if (!strategy) {
        return {false, "No strategy found for statement type: " + std::to_string(static_cast<int>(stmt_type))};
    }
    
    // 检查策略特定的权限
    if (!strategy->checkPermission(*stmt, *context)) {
        return {false, "Permission denied for statement type: " + std::to_string(static_cast<int>(stmt_type))};
    }
    
    // 验证上下文
    if (!strategy->validate(*stmt, *context)) {
        return {false, "Validation failed for statement type: " + std::to_string(static_cast<int>(stmt_type))};
    }
    
    // 执行语句
    ExecutionResult result = strategy->execute(std::move(stmt), *context);
    
    // 更新最后执行上下文
    last_context_ = *context;
    
    return result;
}

ExecutionStrategy* UnifiedExecutor::getStrategy(sql_parser::Statement::Type type) {
    auto it = strategies_.find(type);
    if (it != strategies_.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool UnifiedExecutor::checkGlobalPermission(const sql_parser::Statement& stmt,
                                           ExecutionContext& context) {
    // 全局权限检查逻辑
    // 这里可以根据语句类型和当前用户进行权限检查
    return true; // 默认允许，实际实现应根据具体需求
}

bool UnifiedExecutor::validateGlobalContext(const sql_parser::Statement& stmt,
                                          ExecutionContext& context) {
    // 全局上下文验证逻辑
    // 检查执行环境是否满足基本要求
    if (!context.db_manager) {
        return false;
    }
    return true; // 默认通过，实际实现应根据具体需求
}

// getLastExecutionContext() is implemented inline in the header file

} // namespace sqlcc