#include "core/user_manager.h"
#include "core/system_database.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>

namespace sqlcc {

UserManager::UserManager(const std::string &data_path)
    : data_path_(data_path) {
    CreateDefaultSuperuser();
    InitializePermissionMatrix();
}

UserManager::~UserManager() = default;

void UserManager::SetSystemDatabase(std::shared_ptr<SystemDatabase> sys_db) {
    std::lock_guard<std::mutex> lock(mutex_);
    sys_db_ = sys_db;
}

bool UserManager::CreateUser(const std::string &username, const std::string &password,
                             const std::string &role) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (users_.find(username) != users_.end()) {
        last_error_ = "User already exists: " + username;
        return false;
    }

    if (!IsValidRole(role)) {
        last_error_ = "Invalid role: " + role;
        return false;
    }

    User user;
    user.username = username;
    user.password_hash = HashPassword(password);
    user.role = role;
    user.current_role = role;
    user.is_active = true;
    user.created_at = GetCurrentTimeString();

    users_[username] = user;
    user_current_roles_[username] = role;

    // 为超级用户授予所有权限
    if (role == ROLE_SUPERUSER) {
        GrantAllPrivilegesToSuperuser(username);
    }

    last_error_.clear();
    return SaveToFileInternal();
}

bool UserManager::DropUser(const std::string &username) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (users_.find(username) == users_.end()) {
        last_error_ = "User does not exist: " + username;
        return false;
    }

    if (username == "superuser") {
        last_error_ = "Cannot drop superuser";
        return false;
    }

    RemoveUserPrivileges(username);
    users_.erase(username);
    user_current_roles_.erase(username);

    last_error_.clear();
    return SaveToFileInternal();
}

bool UserManager::AlterUserPassword(const std::string &username,
                                    const std::string &new_password) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = users_.find(username);
    if (it == users_.end()) {
        last_error_ = "User does not exist: " + username;
        return false;
    }

    it->second.password_hash = HashPassword(new_password);
    last_error_.clear();
    return SaveToFileInternal();
}

bool UserManager::AlterUserRole(const std::string &username, const std::string &new_role) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = users_.find(username);
    if (it == users_.end()) {
        last_error_ = "User does not exist: " + username;
        return false;
    }

    if (!IsValidRole(new_role)) {
        last_error_ = "Invalid role: " + new_role;
        return false;
    }

    it->second.role = new_role;
    it->second.current_role = new_role;
    user_current_roles_[username] = new_role;

    last_error_.clear();
    return SaveToFileInternal();
}

bool UserManager::AuthenticateUser(const std::string &username,
                                   const std::string &password) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = users_.find(username);
    if (it == users_.end()) {
        last_error_ = "User does not exist: " + username;
        return false;
    }

    if (!it->second.is_active) {
        last_error_ = "User is not active: " + username;
        return false;
    }

    if (it->second.password_hash != HashPassword(password)) {
        last_error_ = "Invalid password";
        return false;
    }

    last_error_.clear();
    return true;
}

bool UserManager::CreateRole(const std::string &role_name) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (roles_.find(role_name) != roles_.end()) {
        last_error_ = "Role already exists: " + role_name;
        return false;
    }

    Role role;
    role.role_name = role_name;
    role.created_at = GetCurrentTimeString();

    roles_[role_name] = role;
    last_error_.clear();
    return SaveToFileInternal();
}

bool UserManager::DropRole(const std::string &role_name) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (roles_.find(role_name) == roles_.end()) {
        last_error_ = "Role does not exist: " + role_name;
        return false;
    }

    if (role_name == ROLE_SUPERUSER || role_name == ROLE_ADMIN || role_name == ROLE_USER) {
        last_error_ = "Cannot drop system role: " + role_name;
        return false;
    }

    RemoveRolePrivileges(role_name);
    roles_.erase(role_name);

    last_error_.clear();
    return SaveToFileInternal();
}

bool UserManager::AlterRole(const std::string &role_name,
                            const std::string &new_role_name) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (roles_.find(role_name) == roles_.end()) {
        last_error_ = "Role does not exist: " + role_name;
        return false;
    }

    if (roles_.find(new_role_name) != roles_.end()) {
        last_error_ = "Role already exists: " + new_role_name;
        return false;
    }

    Role role = roles_[role_name];
    role.role_name = new_role_name;
    roles_.erase(role_name);
    roles_[new_role_name] = role;

    last_error_.clear();
    return SaveToFileInternal();
}

bool UserManager::SetCurrentRole(const std::string &username,
                                 const std::string &role_name) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto user_it = users_.find(username);
    if (user_it == users_.end()) {
        last_error_ = "User does not exist: " + username;
        return false;
    }

    auto role_it = roles_.find(role_name);
    if (role_it == roles_.end()) {
        last_error_ = "Role does not exist: " + role_name;
        return false;
    }

    user_it->second.current_role = role_name;
    user_current_roles_[username] = role_name;

    last_error_.clear();
    return true;
}

std::string UserManager::GetUserCurrentRole(const std::string &username) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = user_current_roles_.find(username);
    if (it != user_current_roles_.end()) {
        return it->second;
    }

    auto user_it = users_.find(username);
    if (user_it != users_.end()) {
        return user_it->second.role;
    }

    return "";
}

bool UserManager::GrantPrivilege(const std::string &grantee, const std::string &database,
                                 const std::string &table, const std::string &privilege) {
    std::lock_guard<std::mutex> lock(mutex_);

    Permission permission;
    permission.grantee = grantee;
    permission.database = database;
    permission.table = table;
    permission.privilege = privilege;
    permission.is_role = (roles_.find(grantee) != roles_.end());

    permissions_.push_back(permission);
    AddPermissionToMatrix(permission);

    last_error_.clear();
    return SaveToFileInternal();
}

bool UserManager::RevokePrivilege(const std::string &grantee, const std::string &database,
                                  const std::string &table, const std::string &privilege) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = std::remove_if(permissions_.begin(), permissions_.end(),
                             [&](const Permission &p) {
                               return p.grantee == grantee && p.database == database &&
                                      p.table == table && p.privilege == privilege;
                             });
    if (it != permissions_.end()) {
        Permission permission = *it;
        permissions_.erase(it, permissions_.end());
        RemovePermissionFromMatrix(permission);
        last_error_.clear();
        return SaveToFileInternal();
    }

    last_error_ = "Privilege not found";
    return false;
}

bool UserManager::CheckPermission(const std::string &username, const std::string &database,
                                  const std::string &table,
                                  const std::string &required_privilege) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 检查用户是否存在且激活
    auto user_it = users_.find(username);
    if (user_it == users_.end() || !user_it->second.is_active) {
        return false;
    }

    // 超级用户拥有所有权限
    if (user_it->second.role == ROLE_SUPERUSER) {
        return true;
    }

    // 使用权限矩阵进行检查
    return CheckPermissionInMatrix(username, database, table, required_privilege);
}

std::vector<User> UserManager::ListUsers() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<User> result;
    for (const auto &pair : users_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<Role> UserManager::ListRoles() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Role> result;
    for (const auto &pair : roles_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<Permission>
UserManager::ListUserPermissions(const std::string &username) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Permission> result;
    for (const auto &permission : permissions_) {
        if (permission.grantee == username && !permission.is_role) {
            result.push_back(permission);
        }
    }
    return result;
}

std::vector<Permission>
UserManager::ListRolePermissions(const std::string &role_name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Permission> result;
    for (const auto &permission : permissions_) {
        if (permission.grantee == role_name && permission.is_role) {
            result.push_back(permission);
        }
    }
    return result;
}

bool UserManager::SaveToFile() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return SaveToFileInternal();
}

bool UserManager::LoadFromFile() {
    std::lock_guard<std::mutex> lock(mutex_);
    // 简化的实现
    return true;
}

const std::string &UserManager::GetLastError() const {
    return last_error_;
}

// 私有方法实现
void UserManager::CreateDefaultSuperuser() {
    if (users_.find("superuser") == users_.end()) {
        CreateUser("superuser", "superuser", ROLE_SUPERUSER);
    }

    // 创建默认角色
    if (roles_.find(ROLE_SUPERUSER) == roles_.end()) {
        Role superuser_role;
        superuser_role.role_name = ROLE_SUPERUSER;
        superuser_role.created_at = GetCurrentTimeString();
        roles_[ROLE_SUPERUSER] = superuser_role;
    }

    if (roles_.find(ROLE_ADMIN) == roles_.end()) {
        Role admin_role;
        admin_role.role_name = ROLE_ADMIN;
        admin_role.created_at = GetCurrentTimeString();
        roles_[ROLE_ADMIN] = admin_role;
    }

    if (roles_.find(ROLE_USER) == roles_.end()) {
        Role user_role;
        user_role.role_name = ROLE_USER;
        user_role.created_at = GetCurrentTimeString();
        roles_[ROLE_USER] = user_role;
    }
}

std::string UserManager::GetCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    return std::ctime(&time_t);
}

void UserManager::GrantAllPrivilegesToSuperuser(const std::string &username) {
    // 简化的实现
}

void UserManager::RemoveUserPrivileges(const std::string &username) {
    auto it = std::remove_if(permissions_.begin(), permissions_.end(),
                             [&](const Permission &p) {
                               return p.grantee == username && !p.is_role;
                             });
    permissions_.erase(it, permissions_.end());
}

void UserManager::RemoveRolePrivileges(const std::string &role_name) {
    auto it = std::remove_if(permissions_.begin(), permissions_.end(),
                             [&](const Permission &p) {
                               return p.grantee == role_name && p.is_role;
                             });
    permissions_.erase(it, permissions_.end());
}

bool UserManager::IsValidRole(const std::string &role_name) const {
    return roles_.find(role_name) != roles_.end();
}

std::string UserManager::HashPassword(const std::string &password) const {
    // 简化的哈希实现（实际应该使用更安全的哈希算法）
    return password + "_hashed";
}

bool UserManager::SaveToFileInternal() const {
    // 简化的保存实现
    return true;
}

void UserManager::InitializePermissionMatrix() {
    // 初始化权限矩阵
}

void UserManager::AddPermissionToMatrix(const Permission &permission) {
    PermissionKey key;
    key.grantee = permission.grantee;
    key.database = permission.database;
    key.table = permission.table;
    key.privilege = permission.privilege;

    PermissionValue value;
    value.has_permission = true;
    value.is_role = permission.is_role;

    permission_matrix_[key] = value;
}

void UserManager::RemovePermissionFromMatrix(const Permission &permission) {
    PermissionKey key;
    key.grantee = permission.grantee;
    key.database = permission.database;
    key.table = permission.table;
    key.privilege = permission.privilege;

    permission_matrix_.erase(key);
}

bool UserManager::CheckPermissionInMatrix(
    const std::string &username, const std::string &database,
    const std::string &table,
    const std::string &required_privilege) const {

    // 检查用户直接权限
    PermissionKey user_key{username, database, table, required_privilege};
    auto it = permission_matrix_.find(user_key);
    if (it != permission_matrix_.end() && it->second.has_permission) {
        return true;
    }

    // 检查用户角色的权限
    std::string user_role = GetUserCurrentRole(username);
    if (!user_role.empty()) {
        PermissionKey role_key{user_role, database, table, required_privilege};
        auto role_it = permission_matrix_.find(role_key);
        if (role_it != permission_matrix_.end() && role_it->second.has_permission) {
            return true;
        }
    }

    return false;
}

void UserManager::UpdateUserCurrentRole(const std::string &username,
                                        const std::string &role_name) {
    user_current_roles_[username] = role_name;
}

} // namespace sqlcc
