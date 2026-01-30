#include "dcl_execution_strategy.h"
#include "../core/execution_result.h"
#include "../core/execution_context.h"
#include "../core/permission_validator.h"
#include "../sql_parser/ast/ast_nodes.h"
#include "../core/core_database_manager.h"
#include <iostream>
#include <memory>
#include <sstream>

namespace sqlcc {

ExecutionResult DCLExecutionStrategy::execute(std::unique_ptr<sql_parser::Statement> stmt,
                                            ExecutionContext &context) {
    if (!stmt) {
        return createErrorResult("Statement is null");
    }

    switch (stmt->type) {
        case sql_parser::Statement::Type::CREATE_USER: {
            auto create_stmt = dynamic_cast<sql_parser::CreateUserStatement*>(stmt.get());
            if (create_stmt) {
                return executeCreateUser(*create_stmt, context);
            }
            break;
        }
        case sql_parser::Statement::Type::DROP_USER: {
            auto drop_stmt = dynamic_cast<sql_parser::DropUserStatement*>(stmt.get());
            if (drop_stmt) {
                return executeDropUser(*drop_stmt, context);
            }
            break;
        }
        case sql_parser::Statement::Type::GRANT: {
            auto grant_stmt = dynamic_cast<sql_parser::GrantStatement*>(stmt.get());
            if (grant_stmt) {
                return executeGrant(*grant_stmt, context);
            }
            break;
        }
        case sql_parser::Statement::Type::REVOKE: {
            auto revoke_stmt = dynamic_cast<sql_parser::RevokeStatement*>(stmt.get());
            if (revoke_stmt) {
                return executeRevoke(*revoke_stmt, context);
            }
            break;
        }
        default:
            return createErrorResult("Unsupported DCL statement type");
    }

    return createErrorResult("Failed to execute DCL statement");
}

bool DCLExecutionStrategy::checkPermission(const sql_parser::Statement& stmt,
                                         const ExecutionContext &context) {
    // 对于DCL操作，需要管理员权限
    if (context.getUserManager()) {
        auto currentUser = context.getCurrentUser();
        if (currentUser && currentUser->is_admin) {
            return true;
        }
    }
    return false;
}

bool DCLExecutionStrategy::validate(const sql_parser::Statement& stmt,
                                  const ExecutionContext &context) {
    // 验证DCL语句的基本有效性
    switch (stmt.type) {
        case sql_parser::Statement::Type::CREATE_USER:
        case sql_parser::Statement::Type::DROP_USER:
        case sql_parser::Statement::Type::GRANT:
        case sql_parser::Statement::Type::REVOKE:
            return true;
        default:
            return false;
    }
}

ExecutionResult DCLExecutionStrategy::executeCreateUser(const sql_parser::CreateUserStatement& stmt,
                                                      ExecutionContext &context) {
    if (auto db_manager = context.getDatabaseManager()) {
        auto userManager = db_manager->getUserManager();
        if (userManager) {
            // 创建新用户
            User newUser;
            newUser.username = stmt.username;
            newUser.password = stmt.password;  // 在实际实现中应进行哈希处理
            newUser.is_admin = stmt.is_admin;
            
            if (userManager->createUser(newUser)) {
                return createSuccessResult("User created successfully");
            } else {
                return createErrorResult("Failed to create user: username may already exist");
            }
        }
    }
    return createErrorResult("User manager not available");
}

ExecutionResult DCLExecutionStrategy::executeDropUser(const sql_parser::DropUserStatement& stmt,
                                                     ExecutionContext &context) {
    if (auto db_manager = context.getDatabaseManager()) {
        auto userManager = db_manager->getUserManager();
        if (userManager) {
            if (userManager->dropUser(stmt.username)) {
                return createSuccessResult("User dropped successfully");
            } else {
                return createErrorResult("Failed to drop user: user may not exist");
            }
        }
    }
    return createErrorResult("User manager not available");
}

ExecutionResult DCLExecutionStrategy::executeGrant(const sql_parser::GrantStatement& stmt,
                                                  ExecutionContext &context) {
    if (auto db_manager = context.getDatabaseManager()) {
        auto userManager = db_manager->getUserManager();
        if (userManager) {
            if (userManager->grantPermission(stmt.username, stmt.permission, stmt.resource)) {
                return createSuccessResult("Permission granted successfully");
            } else {
                return createErrorResult("Failed to grant permission");
            }
        }
    }
    return createErrorResult("User manager not available");
}

ExecutionResult DCLExecutionStrategy::executeRevoke(const sql_parser::RevokeStatement& stmt,
                                                   ExecutionContext &context) {
    if (auto db_manager = context.getDatabaseManager()) {
        auto userManager = db_manager->getUserManager();
        if (userManager) {
            if (userManager->revokePermission(stmt.username, stmt.permission, stmt.resource)) {
                return createSuccessResult("Permission revoked successfully");
            } else {
                return createErrorResult("Failed to revoke permission");
            }
        }
    }
    return createErrorResult("User manager not available");
}

} // namespace sqlcc