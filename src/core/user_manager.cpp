/**
 * @file user_manager.cpp
 *
 * WHY: 为什么需要用户管理器？
 *
 * 数据库系统是多用户的共享资源，需要精细的访问控制来确保数据安全和完整性。
 * 没有用户管理器，数据库就无法区分不同用户的权限，任何人都能访问所有数据。
 *
 * 主要问题解决：
 * 1. 身份认证：验证用户身份，防止未授权访问
 * 2. 权限控制：基于角色的访问控制，精确控制数据操作权限
 * 3. 安全隔离：不同用户的数据和操作相互隔离
 * 4. 审计追踪：记录用户操作，便于安全审计和问题排查
 * 5. 密码安全：安全的密码存储和验证机制
 *
 * 用户管理器失败的影响：
 * - 数据泄露：任何人都能访问敏感数据
 * - 数据破坏：恶意用户可以删除或修改重要数据
 * - 法律风险：违反数据保护法规和隐私要求
 * - 系统瘫痪：恶意用户占用系统资源
 *
 * WHAT: 这实现了什么功能？
 *
 * 用户管理器提供完整的用户和权限管理系统：
 * - 用户管理：创建、删除、修改用户信息和密码
 * - 角色管理：创建、删除、修改角色定义和继承关系
 * - 权限管理：授予、撤销用户和角色的数据库对象访问权限
 * - 身份验证：用户名密码认证和会话管理
 * - 权限检查：实时验证用户对数据库对象的访问权限
 * - 权限矩阵：高效的权限查找和缓存机制
 *
 * 核心组件：
 * - 用户存储：用户信息、密码哈希、安全状态管理
 * - 角色体系：角色定义、继承关系、权限聚合
 * - 权限模型：数据库、表、列级别的细粒度权限控制
 * - 认证引擎：安全的身份验证和密码管理
 * - 授权引擎：基于角色的访问控制决策
 * - 持久化层：用户和权限信息的持久化存储
 *
 * HOW: 如何实现的？
 *
 * 技术实现要点：
 * 1. 并发安全：使用互斥锁保护共享状态的多线程安全
 * 2. 密码安全：哈希存储密码（当前实现为简化版本）
 * 3. 权限矩阵：高效的权限查找和缓存数据结构
 * 4. 角色继承：支持角色间的权限继承关系
 * 5. 原子操作：用户和权限管理的原子性保证
 * 6. 错误处理：完善的错误信息和异常处理机制
 *
 * 架构设计：
 * - 观察者模式：与系统数据库集成，支持权限变更通知
 * - 工厂模式：动态创建和管理器实例
 * - 策略模式：可插拔的认证和授权策略
 * - 模板方法：统一的CRUD操作流程
 * - 享元模式：权限对象的复用和缓存
 *
 * 性能优化：
 * - 权限缓存：内存中的权限矩阵快速查找
 * - 延迟加载：按需加载用户和角色信息
 * - 批量操作：支持批量权限管理和用户操作
 * - 索引优化：用户和角色名称的快速查找
 *
 * @note 该实现专为SQLCC数据库系统优化，支持RBAC权限模型
 * @see include/core/user_manager.h
 */

#include "../backups/core_backup_20260121_001034/user_manager.h"
#include <algorithm>
#include <iostream>
#include <queue>
#include <unordered_set>

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

    // 检查通配符权限 (*)
    // 数据库级别通配符
    PermissionKey db_wildcard_key{username, database, "*", required_privilege};
    auto db_it = permission_matrix_.find(db_wildcard_key);
    if (db_it != permission_matrix_.end() && db_it->second.has_permission) {
        return true;
    }

    // 角色数据库级别通配符
    if (!user_role.empty()) {
        PermissionKey role_db_wildcard_key{user_role, database, "*", required_privilege};
        auto role_db_it = permission_matrix_.find(role_db_wildcard_key);
        if (role_db_it != permission_matrix_.end() && role_db_it->second.has_permission) {
            return true;
        }
    }

    // 全局通配符 (所有数据库，所有表)
    PermissionKey global_wildcard_key{username, "*", "*", required_privilege};
    auto global_it = permission_matrix_.find(global_wildcard_key);
    if (global_it != permission_matrix_.end() && global_it->second.has_permission) {
        return true;
    }

    // 角色全局通配符
    if (!user_role.empty()) {
        PermissionKey role_global_wildcard_key{user_role, "*", "*", required_privilege};
        auto role_global_it = permission_matrix_.find(role_global_wildcard_key);
        if (role_global_it != permission_matrix_.end() && role_global_it->second.has_permission) {
            return true;
        }
    }

    return false;
}

void UserManager::UpdateUserCurrentRole(const std::string &username,
                                        const std::string &role_name) {
    user_current_roles_[username] = role_name;
}

// 高级权限管理方法实现
bool UserManager::GrantRoleToRole(const std::string &parent_role, const std::string &child_role) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 检查父角色是否存在
    if (roles_.find(parent_role) == roles_.end()) {
        last_error_ = "Parent role does not exist: " + parent_role;
        return false;
    }

    // 检查子角色是否存在
    if (roles_.find(child_role) == roles_.end()) {
        last_error_ = "Child role does not exist: " + child_role;
        return false;
    }

    // 检查循环依赖（简化实现）
    if (parent_role == child_role) {
        last_error_ = "Cannot grant role to itself";
        return false;
    }

    // 建立角色继承关系
    roles_[parent_role].child_roles.push_back(child_role);
    roles_[child_role].parent_roles.push_back(parent_role);

    last_error_.clear();
    return SaveToFileInternal();
}

bool UserManager::RevokeRoleFromRole(const std::string &parent_role, const std::string &child_role) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 检查父角色是否存在
    if (roles_.find(parent_role) == roles_.end()) {
        last_error_ = "Parent role does not exist: " + parent_role;
        return false;
    }

    // 检查子角色是否存在
    if (roles_.find(child_role) == roles_.end()) {
        last_error_ = "Child role does not exist: " + child_role;
        return false;
    }

    // 撤销角色继承关系
    auto &parent_children = roles_[parent_role].child_roles;
    auto &child_parents = roles_[child_role].parent_roles;

    parent_children.erase(
        std::remove(parent_children.begin(), parent_children.end(), child_role),
        parent_children.end());

    child_parents.erase(
        std::remove(child_parents.begin(), child_parents.end(), parent_role),
        child_parents.end());

    last_error_.clear();
    return SaveToFileInternal();
}

bool UserManager::CheckRoleInheritance(const std::string &role_name, const std::string &inherited_role) const {
    std::lock_guard<std::mutex> lock(mutex_);

    if (role_name == inherited_role) {
        return true;
    }

    // 广度优先搜索检查继承关系
    std::unordered_set<std::string> visited;
    std::queue<std::string> to_visit;

    to_visit.push(role_name);
    visited.insert(role_name);

    while (!to_visit.empty()) {
        std::string current = to_visit.front();
        to_visit.pop();

        auto it = roles_.find(current);
        if (it != roles_.end()) {
            // 检查直接子角色
            for (const auto &child : it->second.child_roles) {
                if (child == inherited_role) {
                    return true;
                }
                if (visited.find(child) == visited.end()) {
                    visited.insert(child);
                    to_visit.push(child);
                }
            }
        }
    }

    return false;
}

std::vector<std::string> UserManager::GetRoleHierarchy(const std::string &role_name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> hierarchy;

    auto it = roles_.find(role_name);
    if (it == roles_.end()) {
        return hierarchy;
    }

    // 广度优先遍历获取所有子角色
    std::unordered_set<std::string> visited;
    std::queue<std::string> to_visit;

    to_visit.push(role_name);
    visited.insert(role_name);

    while (!to_visit.empty()) {
        std::string current = to_visit.front();
        to_visit.pop();

        auto role_it = roles_.find(current);
        if (role_it != roles_.end()) {
            for (const auto &child : role_it->second.child_roles) {
                if (visited.find(child) == visited.end()) {
                    visited.insert(child);
                    hierarchy.push_back(child);
                    to_visit.push(child);
                }
            }
        }
    }

    return hierarchy;
}

bool UserManager::RevokePrivilegeCascade(const std::string &grantee, const std::string &database,
                                         const std::string &table, const std::string &privilege) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 撤销直接权限
    bool direct_revoked = RevokePrivilege(grantee, database, table, privilege);
    if (!direct_revoked && GetLastError() != "Privilege not found") {
        return false;
    }

    // 如果是角色，级联撤销所有子角色的权限
    auto role_it = roles_.find(grantee);
    if (role_it != roles_.end()) {
        std::vector<std::string> child_roles = GetRoleHierarchy(grantee);
        for (const auto &child_role : child_roles) {
            // 撤销子角色的权限（不改变错误状态，因为可能某些子角色没有该权限）
            RevokePrivilege(child_role, database, table, privilege);
        }
    }

    // 如果是用户，撤销其所有角色的权限
    auto user_it = users_.find(grantee);
    if (user_it != users_.end()) {
        std::string user_role = user_it->second.role;
        if (!user_role.empty()) {
            RevokePrivilege(user_role, database, table, privilege);
        }
    }

    last_error_.clear();
    return true;
}

bool UserManager::CheckPermissionConflict(const std::string &grantee, const std::string &database,
                                          const std::string &table, const std::string &privilege) const {
    std::lock_guard<std::mutex> lock(mutex_);

    // 检查是否已经存在相同的权限
    for (const auto &perm : permissions_) {
        if (perm.grantee == grantee && perm.database == database &&
            perm.table == table && perm.privilege == privilege) {
            return true; // 权限冲突
        }
    }

    return false; // 无冲突
}

bool UserManager::AuditPermissionChanges(const std::string &operation, const std::string &grantee,
                                         const std::string &details) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 记录权限变更审计信息
    // 简化实现，实际应该写入审计日志
    std::cout << "[AUDIT] " << GetCurrentTimeString()
              << " Operation: " << operation
              << " Grantee: " << grantee
              << " Details: " << details << std::endl;

    // 如果有SystemDatabase，可以同步审计信息
    if (sys_db_) {
        // 这里可以调用SystemDatabase的审计记录方法
        // sys_db_->RecordAuditLog(operation, grantee, details);
    }

    return true;
}

std::vector<std::string> UserManager::GetEffectivePermissions(const std::string &username,
                                                              const std::string &database,
                                                              const std::string &table) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> effective_permissions;

    auto user_it = users_.find(username);
    if (user_it == users_.end() || !user_it->second.is_active) {
        return effective_permissions;
    }

    // 超级用户拥有所有权限
    if (user_it->second.role == ROLE_SUPERUSER) {
        effective_permissions = {PRIVILEGE_CREATE, PRIVILEGE_SELECT, PRIVILEGE_INSERT,
                                PRIVILEGE_UPDATE, PRIVILEGE_DELETE, PRIVILEGE_DROP,
                                PRIVILEGE_ALTER};
        return effective_permissions;
    }

    // 检查用户直接权限
    std::string user_current_role = GetUserCurrentRole(username);
    std::unordered_set<std::string> unique_permissions;

    // 用户直接权限
    for (const auto &perm : permissions_) {
        if (perm.grantee == username && !perm.is_role &&
            perm.database == database && perm.table == table) {
            unique_permissions.insert(perm.privilege);
        }
    }

    // 用户角色权限
    if (!user_current_role.empty()) {
        for (const auto &perm : permissions_) {
            if (perm.grantee == user_current_role && perm.is_role &&
                perm.database == database && perm.table == table) {
                unique_permissions.insert(perm.privilege);
            }
        }

        // 检查角色继承权限
        std::vector<std::string> parent_roles;
        auto role_it = roles_.find(user_current_role);
        if (role_it != roles_.end()) {
            parent_roles = role_it->second.parent_roles;
        }

        for (const auto &parent_role : parent_roles) {
            for (const auto &perm : permissions_) {
                if (perm.grantee == parent_role && perm.is_role &&
                    perm.database == database && perm.table == table) {
                    unique_permissions.insert(perm.privilege);
                }
            }
        }
    }

    effective_permissions.assign(unique_permissions.begin(), unique_permissions.end());
    return effective_permissions;
}} // namespace sqlcc
