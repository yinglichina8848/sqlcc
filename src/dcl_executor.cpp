#include "execution_engine.h"
#include "core/user_manager.h"
#include "core/system_database.h"
#include "core/execution_context.h"
#include <iostream>

namespace sqlcc {

DCLExecutor::DCLExecutor(std::shared_ptr<DatabaseManager> db_manager,
                         std::shared_ptr<UserManager> user_manager)
    : ExecutionEngine(db_manager), user_manager_(user_manager) {}

ExecutionResult DCLExecutor::execute(std::unique_ptr<sql_parser::Statement> stmt) {
    if (auto create_user_stmt = dynamic_cast<sql_parser::CreateUserStatement*>(stmt.get())) {
        // 转移所有权到具体执行函数
        return executeCreateUser(std::unique_ptr<sql_parser::CreateUserStatement>(static_cast<sql_parser::CreateUserStatement*>(stmt.release())));
    } else if (auto drop_user_stmt = dynamic_cast<sql_parser::DropUserStatement*>(stmt.get())) {
        return executeDropUser(std::unique_ptr<sql_parser::DropUserStatement>(static_cast<sql_parser::DropUserStatement*>(stmt.release())));
    } else if (auto grant_stmt = dynamic_cast<sql_parser::GrantStatement*>(stmt.get())) {
        return executeGrant(std::unique_ptr<sql_parser::GrantStatement>(static_cast<sql_parser::GrantStatement*>(stmt.release())));
    } else if (auto revoke_stmt = dynamic_cast<sql_parser::RevokeStatement*>(stmt.get())) {
        return executeRevoke(std::unique_ptr<sql_parser::RevokeStatement>(static_cast<sql_parser::RevokeStatement*>(stmt.release())));
    }
    
    return {false, "Unsupported DCL statement type"};
}

ExecutionResult DCLExecutor::executeCreateUser(std::unique_ptr<sql_parser::CreateUserStatement> stmt) {
    if (!user_manager_) {
        return {false, "User manager not available"};
    }
    
    std::string username = stmt->getUsername();
    std::string password = stmt->getPassword();
    // CreateUserStatement没有getRole方法，使用默认角色
    std::string role = UserManager::ROLE_USER;    
    // 检查用户是否已存在
    auto users = user_manager_->ListUsers();
    for (const auto& user : users) {
        if (user.username == username) {
            return {false, "User '" + username + "' already exists"};
        }
    }
    
    // 创建用户
    if (user_manager_->CreateUser(username, password, role)) {
        // 同步到系统数据库
        if (user_manager_->GetSystemDatabase()) {
            user_manager_->GetSystemDatabase()->CreateUserRecord(username, password, role);
        }
        return {true, "User '" + username + "' created successfully"};
    } else {
        return {false, "Failed to create user '" + username + "': " + user_manager_->GetLastError()};
    }
}

ExecutionResult DCLExecutor::executeDropUser(std::unique_ptr<sql_parser::DropUserStatement> stmt) {
    if (!user_manager_) {
        return {false, "User manager not available"};
    }
    
    std::string username = stmt->getUsername();
    
    // 检查用户是否存在
    auto users = user_manager_->ListUsers();
    bool user_exists = false;
    for (const auto& user : users) {
        if (user.username == username) {
            user_exists = true;
            break;
        }
    }
    
    if (!user_exists) {
        return {false, "User '" + username + "' does not exist"};
    }
    
    // 删除用户
    if (user_manager_->DropUser(username)) {
        // 同步到系统数据库
        if (user_manager_->GetSystemDatabase()) {
            user_manager_->GetSystemDatabase()->DropUserRecord(username);
        }
        return {true, "User '" + username + "' dropped successfully"};
    } else {
        return {false, "Failed to drop user '" + username + "': " + user_manager_->GetLastError()};
    }
}

ExecutionResult DCLExecutor::executeGrant(std::unique_ptr<sql_parser::GrantStatement> stmt) {
    if (!user_manager_) {
        return {false, "User manager not available"};
    }
    
    std::string grantee = stmt->getGrantee();
    std::vector<std::string> privileges = stmt->getPrivileges();
    std::string object_name = stmt->getObjectName();
    std::string object_type = stmt->getObjectType();
    
    // 如果没有指定权限，则默认为ALL PRIVILEGES
    if (privileges.empty()) {
        privileges.push_back("ALL");
    }
    
    // 授予权限
    bool success = true;
    std::string error_msg;
    
    for (const auto& privilege : privileges) {
        if (!user_manager_->GrantPrivilege(grantee, "*", object_name, privilege)) {
            success = false;
            error_msg = user_manager_->GetLastError();
            break;
        }
    }
    
    if (success) {
        // 同步到系统数据库
        if (user_manager_->GetSystemDatabase()) {
            for (const auto& privilege : privileges) {
                user_manager_->GetSystemDatabase()->GrantPrivilegeRecord("USER", grantee, "*", object_name, privilege, "admin");
            }
        }
        return {true, "Permissions granted successfully"};
    } else {
        return {false, "Failed to grant permissions: " + error_msg};
    }
}

ExecutionResult DCLExecutor::executeRevoke(std::unique_ptr<sql_parser::RevokeStatement> stmt) {
    if (!user_manager_) {
        return {false, "User manager not available"};
    }
    
    std::string grantee = stmt->getGrantee();
    std::vector<std::string> privileges = stmt->getPrivileges();
    std::string object_name = stmt->getObjectName();
    std::string object_type = stmt->getObjectType();
    
    // 如果没有指定权限，则默认为ALL PRIVILEGES
    if (privileges.empty()) {
        privileges.push_back("ALL");
    }
    
    // 撤销权限
    bool success = true;
    std::string error_msg;
    
    for (const auto& privilege : privileges) {
        if (!user_manager_->RevokePrivilege(grantee, "*", object_name, privilege)) {
            success = false;
            error_msg = user_manager_->GetLastError();
            break;
        }
    }
    
    if (success) {
        // 同步到系统数据库
        if (user_manager_->GetSystemDatabase()) {
            for (const auto& privilege : privileges) {
                user_manager_->GetSystemDatabase()->RevokePrivilegeRecord("USER", grantee, "*", object_name, privilege);
            }
        }
        return {true, "Permissions revoked successfully"};
    } else {
        return {false, "Failed to revoke permissions: " + error_msg};
    }
}

} // namespace sqlcc