#include "dcl_execution_strategy.h"
#include "core/execution_result.h"
#include "core/execution_context.h"
#include "core/permission_validator.h"
#include "sql_parser/ast/ast_nodes.h"
#include "core/core_database_manager.h"
#include <iostream>
#include <memory>
#include <sstream>

namespace sqlcc {

ExecutionResult DCLExecutionStrategy::execute(std::unique_ptr<sql_parser::Statement> stmt,
                                            ExecutionContext &context) {
    if (!stmt) {
        return createErrorResult("Statement is null");
    }

    switch (stmt->getType()) {
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
    if (context.get_user_manager()) {
        auto currentUser = context.get_current_user();
        if (!currentUser.empty()) {
            // 检查用户是否有ADMIN或SUPERUSER角色
            auto userManager = context.get_user_manager();
            std::vector<User> users = userManager->ListUsers();
            for (const auto& user : users) {
                if (user.username == currentUser &&
                    (user.role == UserManager::ROLE_ADMIN || user.role == UserManager::ROLE_SUPERUSER)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool DCLExecutionStrategy::validate(const sql_parser::Statement& stmt,
                                  const ExecutionContext &context) {
    // 验证DCL语句的基本有效性
    switch (stmt.getType()) {
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
    if (auto userManager = context.get_user_manager()) {
        if (userManager) {
            // 创建新用户
            if (userManager->CreateUser(stmt.getUserName(), stmt.getPassword())) {
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
    if (auto userManager = context.get_user_manager()) {
        if (userManager) {
            if (userManager->DropUser(stmt.getUserName())) {
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
    if (auto userManager = context.get_user_manager()) {
        if (userManager) {
            // 处理GRANT语句
            const auto& grantee = stmt.getGrantee();
            const auto& privileges = stmt.getPrivileges();
            const auto& objectType = stmt.getObjectType();
            const auto& objectName = stmt.getObjectName();

            {
                for (const auto& privilege : privileges) {
                    if (objectType == "TABLE") {
                        // 解析表名，格式可能是 "database.table" 或 "table"
                        size_t dotPos = objectName.find('.');
                        std::string database = context.get_current_database();
                        std::string table = objectName;

                        if (dotPos != std::string::npos) {
                            database = objectName.substr(0, dotPos);
                            table = objectName.substr(dotPos + 1);
                        }

                        if (!userManager->GrantPrivilege(grantee, database, table, privilege)) {
                            return createErrorResult("Failed to grant privilege to user " + grantee);
                        }
                    } else if (objectType == "DATABASE") {
                        if (!userManager->GrantPrivilege(grantee, objectName, "", privilege)) {
                            return createErrorResult("Failed to grant privilege to user " + grantee);
                        }
                    } else {
                        return createErrorResult("Unsupported object type: " + objectType);
                    }
                }
            }

            return createSuccessResult("Permission granted successfully");
        }
    }
    return createErrorResult("User manager not available");
}

ExecutionResult DCLExecutionStrategy::executeRevoke(const sql_parser::RevokeStatement& stmt,
                                                   ExecutionContext &context) {
    if (auto userManager = context.get_user_manager()) {
        if (userManager) {
            // 处理REVOKE语句
            const auto& grantee = stmt.getGrantee();
            const auto& privileges = stmt.getPrivileges();
            const auto& objectType = stmt.getObjectType();
            const auto& objectName = stmt.getObjectName();

            {
                for (const auto& privilege : privileges) {
                    if (objectType == "TABLE") {
                        // 解析表名，格式可能是 "database.table" 或 "table"
                        size_t dotPos = objectName.find('.');
                        std::string database = context.get_current_database();
                        std::string table = objectName;

                        if (dotPos != std::string::npos) {
                            database = objectName.substr(0, dotPos);
                            table = objectName.substr(dotPos + 1);
                        }

                        if (!userManager->RevokePrivilege(grantee, database, table, privilege)) {
                            return createErrorResult("Failed to revoke privilege from user " + grantee);
                        }
                    } else if (objectType == "DATABASE") {
                        if (!userManager->RevokePrivilege(grantee, objectName, "", privilege)) {
                            return createErrorResult("Failed to revoke privilege from user " + grantee);
                        }
                    } else {
                        return createErrorResult("Unsupported object type: " + objectType);
                    }
                }
            }

            return createSuccessResult("Permission revoked successfully");
        }
    }
    return createErrorResult("User manager not available");
}

} // namespace sqlcc
