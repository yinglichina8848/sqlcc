#include "system_permission_manager.h"
#include "system_data_structures.h"
#include "system_database_new.h"  // 包含SYSTEM_DB_NAME常量定义
#include <sstream>
#include <iostream>
#include <iomanip>
#include <chrono>

namespace sqlcc {

// 系统表名称常量
const std::string SYS_TABLE_USERS = "sys_users";
const std::string SYS_TABLE_ROLES = "sys_roles";
const std::string SYS_TABLE_PRIVILEGES = "sys_privileges";

SystemPermissionManager::SystemPermissionManager(std::shared_ptr<DatabaseManager> db_manager)
    : db_manager_(db_manager) {
}

SystemPermissionManager::~SystemPermissionManager() {
}

// 用户管理方法
bool SystemPermissionManager::CreateUserRecord(const std::string& username, const std::string& password_hash, const std::string& role) {
    try {
        // 检查用户是否已存在
        if (UserExists(username)) {
            SetError("User already exists: " + username);
            return false;
        }
        
        // 检查角色是否存在
        if (!RoleExists(role)) {
            SetError("Role does not exist: " + role);
            return false;
        }
        
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        std::string current_time = ss.str();
        
        // 生成用户ID
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        
        // 构建INSERT语句
        std::stringstream sql;
        sql << "INSERT INTO " << SYS_TABLE_USERS << " (user_id, username, password_hash, role, current_role, is_active, created_at) VALUES ("
            << timestamp << ", '" << username << "', '" << password_hash << "', '"
            << role << "', '" << role << "', TRUE, '" << current_time << "')";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to create user: ") + e.what());
        return false;
    }
}

bool SystemPermissionManager::DropUserRecord(const std::string& username) {
    try {
        // 检查用户是否存在
        if (!UserExists(username)) {
            SetError("User does not exist: " + username);
            return false;
        }
        
        // 构建DELETE语句
        std::stringstream sql;
        sql << "DELETE FROM " << SYS_TABLE_USERS << " WHERE username = '" << username << "'";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to drop user: ") + e.what());
        return false;
    }
}

bool SystemPermissionManager::UpdateUserRecord(const SysUser& user) {
    try {
        // 检查用户是否存在
        if (!UserExists(user.username)) {
            SetError("User does not exist: " + user.username);
            return false;
        }
        
        // 构建UPDATE语句
        std::stringstream sql;
        sql << "UPDATE " << SYS_TABLE_USERS << " SET "
            << "password_hash = '" << user.password_hash << "', "
            << "role = '" << user.role << "', "
            << "current_role = '" << user.current_role << "', "
            << "is_active = " << (user.is_active ? "TRUE" : "FALSE")
            << " WHERE username = '" << user.username << "'";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to update user: ") + e.what());
        return false;
    }
}

SysUser SystemPermissionManager::GetUserRecord(const std::string& username) {
    SysUser user;
    
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT user_id, username, password_hash, role, current_role, is_active, created_at FROM "
            << SYS_TABLE_USERS << " WHERE username = '" << username << "'";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        if (result.empty() || result[0].size() < 7) {
            SetError("User not found: " + username);
            return user;
        }
        
        // 填充用户信息
        user.user_id = std::stoll(result[0][0]);
        user.username = result[0][1];
        user.password_hash = result[0][2];
        user.role = result[0][3];
        user.current_role = result[0][4];
        user.is_active = (result[0][5] == "TRUE" || result[0][5] == "true" || result[0][5] == "1");
        user.created_at = result[0][6];
        
        return user;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to get user: ") + e.what());
        return user;
    }
}

std::vector<SysUser> SystemPermissionManager::ListUsers() {
    std::vector<SysUser> users;
    
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT user_id, username, password_hash, role, current_role, is_active, created_at FROM "
            << SYS_TABLE_USERS << " ORDER BY username";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        // 填充用户列表
        for (const auto& row : result) {
            if (row.size() >= 7) {
                SysUser user;
                user.user_id = std::stoll(row[0]);
                user.username = row[1];
                user.password_hash = row[2];
                user.role = row[3];
                user.current_role = row[4];
                user.is_active = (row[5] == "TRUE" || row[5] == "true" || row[5] == "1");
                user.created_at = row[6];
                users.push_back(user);
            }
        }
        
        return users;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to list users: ") + e.what());
        return users;
    }
}

bool SystemPermissionManager::UserExists(const std::string& username) {
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT COUNT(*) FROM " << SYS_TABLE_USERS << " WHERE username = '" << username << "'";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        if (result.empty() || result[0].empty()) {
            return false;
        }
        
        return std::stoi(result[0][0]) > 0;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to check user existence: ") + e.what());
        return false;
    }
}

// 角色管理方法
bool SystemPermissionManager::CreateRoleRecord(const std::string& role_name) {
    try {
        // 检查角色是否已存在
        if (RoleExists(role_name)) {
            SetError("Role already exists: " + role_name);
            return false;
        }
        
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        std::string current_time = ss.str();
        
        // 生成角色ID
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        
        // 构建INSERT语句
        std::stringstream sql;
        sql << "INSERT INTO " << SYS_TABLE_ROLES << " (role_id, role_name, created_at) VALUES ("
            << timestamp << ", '" << role_name << "', '" << current_time << "')";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to create role: ") + e.what());
        return false;
    }
}

bool SystemPermissionManager::DropRoleRecord(const std::string& role_name) {
    try {
        // 检查角色是否存在
        if (!RoleExists(role_name)) {
            SetError("Role does not exist: " + role_name);
            return false;
        }
        
        // 检查是否有用户使用该角色
        std::stringstream check_sql;
        check_sql << "SELECT COUNT(*) FROM " << SYS_TABLE_USERS << " WHERE role = '" << role_name << "'";
        auto result = ExecuteSelectQuery(check_sql.str());
        
        if (!result.empty() && !result[0].empty() && std::stoi(result[0][0]) > 0) {
            SetError("Cannot drop role, it is still in use: " + role_name);
            return false;
        }
        
        // 构建DELETE语句
        std::stringstream sql;
        sql << "DELETE FROM " << SYS_TABLE_ROLES << " WHERE role_name = '" << role_name << "'";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to drop role: ") + e.what());
        return false;
    }
}

SysRole SystemPermissionManager::GetRoleRecord(const std::string& role_name) {
    SysRole role;
    
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT role_id, role_name, created_at FROM "
            << SYS_TABLE_ROLES << " WHERE role_name = '" << role_name << "'";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        if (result.empty() || result[0].size() < 3) {
            SetError("Role not found: " + role_name);
            return role;
        }
        
        // 填充角色信息
        role.role_id = std::stoll(result[0][0]);
        role.role_name = result[0][1];
        role.created_at = result[0][2];
        
        return role;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to get role: ") + e.what());
        return role;
    }
}

std::vector<SysRole> SystemPermissionManager::ListRoles() {
    std::vector<SysRole> roles;
    
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT role_id, role_name, created_at FROM "
            << SYS_TABLE_ROLES << " ORDER BY role_name";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        // 填充角色列表
        for (const auto& row : result) {
            if (row.size() >= 3) {
                SysRole role;
                role.role_id = std::stoll(row[0]);
                role.role_name = row[1];
                role.created_at = row[2];
                roles.push_back(role);
            }
        }
        
        return roles;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to list roles: ") + e.what());
        return roles;
    }
}

bool SystemPermissionManager::RoleExists(const std::string& role_name) {
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT COUNT(*) FROM " << SYS_TABLE_ROLES << " WHERE role_name = '" << role_name << "'";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        if (result.empty() || result[0].empty()) {
            return false;
        }
        
        return std::stoi(result[0][0]) > 0;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to check role existence: ") + e.what());
        return false;
    }
}

// 权限管理方法
bool SystemPermissionManager::GrantPrivilegeRecord(const std::string& grantee, const std::string& object_type, 
                                                  const std::string& object_name, const std::string& privilege_type,
                                                  const std::string& grantor) {
    try {
        // 检查接收者是否存在
        if (!UserExists(grantee) && !RoleExists(grantee)) {
            SetError("Grantee does not exist: " + grantee);
            return false;
        }
        
        // 检查授权者是否存在
        if (!UserExists(grantor)) {
            SetError("Grantor does not exist: " + grantor);
            return false;
        }
        
        // 检查权限是否已存在
        if (HasPrivilege(grantee, object_type, object_name, privilege_type)) {
            SetError("Privilege already granted");
            return false;
        }
        
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        std::string current_time = ss.str();
        
        // 生成权限ID
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        
        // 构建INSERT语句
        std::stringstream sql;
        sql << "INSERT INTO " << SYS_TABLE_PRIVILEGES << " (privilege_id, grantee, object_type, object_name, "
            << "privilege_type, grantor, is_grantable, granted_at) VALUES ("
            << timestamp << ", '" << grantee << "', '" << object_type << "', '" << object_name
            << "', '" << privilege_type << "', '" << grantor << "', FALSE, '" << current_time << "')";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to grant privilege: ") + e.what());
        return false;
    }
}

bool SystemPermissionManager::RevokePrivilegeRecord(const std::string& grantee, const std::string& object_type, 
                                                   const std::string& object_name, const std::string& privilege_type) {
    try {
        // 构建DELETE语句
        std::stringstream sql;
        sql << "DELETE FROM " << SYS_TABLE_PRIVILEGES << " WHERE "
            << "grantee = '" << grantee << "' AND "
            << "object_type = '" << object_type << "' AND "
            << "object_name = '" << object_name << "' AND "
            << "privilege_type = '" << privilege_type << "'";
        
        return ExecuteSQL(sql.str());
    } catch (const std::exception& e) {
        SetError(std::string("Failed to revoke privilege: ") + e.what());
        return false;
    }
}

std::vector<SysPrivilege> SystemPermissionManager::GetObjectPrivileges(const std::string& object_type, const std::string& object_name) {
    std::vector<SysPrivilege> privileges;
    
    try {
        // 构建SELECT语句
        std::stringstream sql;
        sql << "SELECT privilege_id, grantee, object_type, object_name, privilege_type, "
            << "grantor, is_grantable, granted_at FROM " << SYS_TABLE_PRIVILEGES
            << " WHERE object_type = '" << object_type << "' AND object_name = '" << object_name << "'";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        // 填充权限列表
        for (const auto& row : result) {
            if (row.size() >= 8) {
                SysPrivilege privilege;
                privilege.privilege_id = std::stoll(row[0]);
                privilege.grantee = row[1];
                privilege.object_type = row[2];
                privilege.object_name = row[3];
                privilege.privilege_type = row[4];
                privilege.grantor = row[5];
                privilege.is_grantable = (row[6] == "TRUE" || row[6] == "true" || row[6] == "1");
                privilege.granted_at = row[7];
                privileges.push_back(privilege);
            }
        }
        
        return privileges;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to get object privileges: ") + e.what());
        return privileges;
    }
}

std::vector<SysPrivilege> SystemPermissionManager::GetUserPrivileges(const std::string& username) {
    std::vector<SysPrivilege> privileges;
    
    try {
        // 获取用户角色
        SysUser user = GetUserRecord(username);
        if (user.username.empty()) {
            SetError("User not found: " + username);
            return privileges;
        }
        
        // 构建SELECT语句，获取用户和角色的权限
        std::stringstream sql;
        sql << "SELECT privilege_id, grantee, object_type, object_name, privilege_type, "
            << "grantor, is_grantable, granted_at FROM " << SYS_TABLE_PRIVILEGES
            << " WHERE grantee = '" << username << "' OR grantee = '" << user.role << "'";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        // 填充权限列表
        for (const auto& row : result) {
            if (row.size() >= 8) {
                SysPrivilege privilege;
                privilege.privilege_id = std::stoll(row[0]);
                privilege.grantee = row[1];
                privilege.object_type = row[2];
                privilege.object_name = row[3];
                privilege.privilege_type = row[4];
                privilege.grantor = row[5];
                privilege.is_grantable = (row[6] == "TRUE" || row[6] == "true" || row[6] == "1");
                privilege.granted_at = row[7];
                privileges.push_back(privilege);
            }
        }
        
        return privileges;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to get user privileges: ") + e.what());
        return privileges;
    }
}

bool SystemPermissionManager::HasPrivilege(const std::string& username, const std::string& object_type, 
                                          const std::string& object_name, const std::string& privilege_type) {
    try {
        // 获取用户角色
        SysUser user = GetUserRecord(username);
        if (user.username.empty()) {
            return false;
        }
        
        // 构建SELECT语句，检查用户和角色的权限
        std::stringstream sql;
        sql << "SELECT COUNT(*) FROM " << SYS_TABLE_PRIVILEGES
            << " WHERE (grantee = '" << username << "' OR grantee = '" << user.role << "') AND "
            << "object_type = '" << object_type << "' AND "
            << "(object_name = '" << object_name << "' OR object_name = '*') AND "
            << "(privilege_type = '" << privilege_type << "' OR privilege_type = 'ALL PRIVILEGES')";
        
        // 执行查询
        auto result = ExecuteSelectQuery(sql.str());
        
        if (result.empty() || result[0].empty()) {
            return false;
        }
        
        return std::stoi(result[0][0]) > 0;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to check privilege: ") + e.what());
        return false;
    }
}

std::string SystemPermissionManager::GetLastError() const {
    return last_error_;
}

bool SystemPermissionManager::ExecuteSQL(const std::string& sql) {
    try {
        // 切换到系统数据库
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to use system database");
            return false;
        }
        
        // 执行SQL语句
        if (!db_manager_->ExecuteSQL(sql)) {
            SetError("Failed to execute SQL: " + sql);
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to execute SQL: ") + e.what());
        return false;
    }
}

std::vector<std::vector<std::string>> SystemPermissionManager::ExecuteSelectQuery(const std::string& sql) {
    std::vector<std::vector<std::string>> result;
    
    try {
        // 切换到系统数据库
        if (!db_manager_->UseDatabase(SYSTEM_DB_NAME)) {
            SetError("Failed to use system database");
            return result;
        }
        
        // 执行查询
        // 注意：这里假设DatabaseManager有ExecuteQuery方法，返回查询结果
        // 实际实现可能需要根据DatabaseManager的API进行调整
        result = db_manager_->ExecuteQuery(sql);
        
        return result;
    } catch (const std::exception& e) {
        SetError(std::string("Failed to execute query: ") + e.what());
        return result;
    }
}

void SystemPermissionManager::SetError(const std::string& error) {
    last_error_ = error;
}

} // namespace sqlcc