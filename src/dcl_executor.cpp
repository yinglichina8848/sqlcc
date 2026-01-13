#include "sql_parser/ast_node.h"
#include "sql_parser/ast_dcl_statements.h"
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
    } else if (auto create_role_stmt = dynamic_cast<sql_parser::CreateRoleStatement*>(stmt.get())) {
        return executeCreateRole(std::unique_ptr<sql_parser::CreateRoleStatement>(static_cast<sql_parser::CreateRoleStatement*>(stmt.release())));
    } else if (auto drop_role_stmt = dynamic_cast<sql_parser::DropRoleStatement*>(stmt.get())) {
        return executeDropRole(std::unique_ptr<sql_parser::DropRoleStatement>(static_cast<sql_parser::DropRoleStatement*>(stmt.release())));
    } else if (auto grant_role_stmt = dynamic_cast<sql_parser::GrantRoleStatement*>(stmt.get())) {
        return executeGrantRole(std::unique_ptr<sql_parser::GrantRoleStatement>(static_cast<sql_parser::GrantRoleStatement*>(stmt.release())));
    } else if (auto revoke_role_stmt = dynamic_cast<sql_parser::RevokeRoleStatement*>(stmt.get())) {
        return executeRevokeRole(std::unique_ptr<sql_parser::RevokeRoleStatement>(static_cast<sql_parser::RevokeRoleStatement*>(stmt.release())));
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

ExecutionResult DCLExecutor::executeCreateRole(std::unique_ptr<sql_parser::CreateRoleStatement> stmt) {
    if (!user_manager_) {
        return {false, "User manager not available"};
    }

    std::string role_name = stmt->getRoleName();

    // 检查角色是否已存在
    auto roles = user_manager_->ListRoles();
    for (const auto& role : roles) {
        if (role.name == role_name) {
            return {false, "Role '" + role_name + "' already exists"};
        }
    }

    // 创建角色
    if (user_manager_->CreateRole(role_name)) {
        // 同步到系统数据库
        if (user_manager_->GetSystemDatabase()) {
            user_manager_->GetSystemDatabase()->CreateRoleRecord(role_name, "admin");
        }
        return {true, "Role '" + role_name + "' created successfully"};
    } else {
        return {false, "Failed to create role '" + role_name + "': " + user_manager_->GetLastError()};
    }
}

ExecutionResult DCLExecutor::executeDropRole(std::unique_ptr<sql_parser::DropRoleStatement> stmt) {
    if (!user_manager_) {
        return {false, "User manager not available"};
    }

    std::string role_name = stmt->getRoleName();

    // 检查角色是否存在
    auto roles = user_manager_->ListRoles();
    bool role_exists = false;
    for (const auto& role : roles) {
        if (role.name == role_name) {
            role_exists = true;
            break;
        }
    }

    if (!role_exists) {
        return {false, "Role '" + role_name + "' does not exist"};
    }

    // 删除角色
    if (user_manager_->DropRole(role_name)) {
        // 同步到系统数据库
        if (user_manager_->GetSystemDatabase()) {
            user_manager_->GetSystemDatabase()->DropRoleRecord(role_name);
        }
        return {true, "Role '" + role_name + "' dropped successfully"};
    } else {
        return {false, "Failed to drop role '" + role_name + "': " + user_manager_->GetLastError()};
    }
}

ExecutionResult DCLExecutor::executeGrantRole(std::unique_ptr<sql_parser::GrantRoleStatement> stmt) {
    if (!user_manager_) {
        return {false, "User manager not available"};
    }

    std::string role_name = stmt->getRoleName();
    std::string grantee = stmt->getGrantee();

    // 检查角色是否存在
    auto roles = user_manager_->ListRoles();
    bool role_exists = false;
    for (const auto& role : roles) {
        if (role.name == role_name) {
            role_exists = true;
            break;
        }
    }

    if (!role_exists) {
        return {false, "Role '" + role_name + "' does not exist"};
    }

    // 检查被授权者是否存在
    auto users = user_manager_->ListUsers();
    bool user_exists = false;
    for (const auto& user : users) {
        if (user.username == grantee) {
            user_exists = true;
            break;
        }
    }

    if (!user_exists) {
        return {false, "User '" + grantee + "' does not exist"};
    }

    // 授予角色
    if (user_manager_->GrantRole(grantee, role_name)) {
        // 同步到系统数据库
        if (user_manager_->GetSystemDatabase()) {
            user_manager_->GetSystemDatabase()->GrantRoleRecord(grantee, role_name, "admin");
        }
        return {true, "Role '" + role_name + "' granted to user '" + grantee + "' successfully"};
    } else {
        return {false, "Failed to grant role '" + role_name + "' to user '" + grantee + "': " + user_manager_->GetLastError()};
    }
}

ExecutionResult DCLExecutor::executeRevokeRole(std::unique_ptr<sql_parser::RevokeRoleStatement> stmt) {
    if (!user_manager_) {
        return {false, "User manager not available"};
    }

    std::string role_name = stmt->getRoleName();
    std::string grantee = stmt->getGrantee();

    // 检查角色是否存在
    auto roles = user_manager_->ListRoles();
    bool role_exists = false;
    for (const auto& role : roles) {
        if (role.name == role_name) {
            role_exists = true;
            break;
        }
    }

    if (!role_exists) {
        return {false, "Role '" + role_name + "' does not exist"};
    }

    // 检查被授权者是否存在
    auto users = user_manager_->ListUsers();
    bool user_exists = false;
    for (const auto& user : users) {
        if (user.username == grantee) {
            user_exists = true;
            break;
        }
    }

    if (!user_exists) {
        return {false, "User '" + grantee + "' does not exist"};
    }

    // 撤销角色
    if (user_manager_->RevokeRole(grantee, role_name)) {
        // 同步到系统数据库
        if (user_manager_->GetSystemDatabase()) {
            user_manager_->GetSystemDatabase()->RevokeRoleRecord(grantee, role_name);
        }
        return {true, "Role '" + role_name + "' revoked from user '" + grantee + "' successfully"};
    } else {
        return {false, "Failed to revoke role '" + role_name + "' from user '" + grantee + "': " + user_manager_->GetLastError()};
    }
}

} // namespace sqlcc
