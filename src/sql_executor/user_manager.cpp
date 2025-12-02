#include "../../include/user_manager.h"
#include "../../include/system_database.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <random>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace sqlcc {

// 用户角色定义
const std::string UserManager::ROLE_SUPERUSER = "SUPERUSER";
const std::string UserManager::ROLE_ADMIN = "ADMIN";
const std::string UserManager::ROLE_USER = "USER";

// 权限定义
const std::string UserManager::PRIVILEGE_CREATE = "CREATE";
const std::string UserManager::PRIVILEGE_SELECT = "SELECT";
const std::string UserManager::PRIVILEGE_INSERT = "INSERT";
const std::string UserManager::PRIVILEGE_UPDATE = "UPDATE";
const std::string UserManager::PRIVILEGE_DELETE = "DELETE";
const std::string UserManager::PRIVILEGE_DROP = "DROP";
const std::string UserManager::PRIVILEGE_ALTER = "ALTER";
const std::string UserManager::PRIVILEGE_ALL = "ALL";

UserManager::UserManager(const std::string& data_path) : data_path_(data_path), sys_db_(nullptr) {
    // 确保数据目录存在
    fs::create_directories(data_path_);
    
    // 尝试从文件加载用户数据
    if (!LoadFromFile()) {
        // 如果加载失败，创建默认超级用户
        CreateDefaultSuperuser();
    }
}

UserManager::~UserManager() {
    // 在析构时自动保存用户数据
    SaveToFile();
}

// 设置SystemDatabase引用
void UserManager::SetSystemDatabase(SystemDatabase* sys_db) {
    std::lock_guard<std::mutex> lock(mutex_);
    sys_db_ = sys_db;
}

// 创建默认超级用户
void UserManager::CreateDefaultSuperuser() {
    // 初始化内置角色
    Role superuser_role;
    superuser_role.role_name = ROLE_SUPERUSER;
    superuser_role.created_at = GetCurrentTimeString();
    roles_[superuser_role.role_name] = superuser_role;

    Role admin_role;
    admin_role.role_name = ROLE_ADMIN;
    admin_role.created_at = GetCurrentTimeString();
    roles_[admin_role.role_name] = admin_role;

    Role user_role;
    user_role.role_name = ROLE_USER;
    user_role.created_at = GetCurrentTimeString();
    roles_[user_role.role_name] = user_role;

    // 创建默认超级用户，用户名: admin, 密码: admin
    User default_superuser;
    default_superuser.username = "admin";
    default_superuser.password_hash = HashPassword("admin"); // 默认密码哈希
    default_superuser.role = ROLE_SUPERUSER;
    default_superuser.current_role = ROLE_SUPERUSER;
    default_superuser.is_active = true;
    default_superuser.created_at = GetCurrentTimeString();
    users_[default_superuser.username] = default_superuser;
    
    // 授予超级用户所有权限
    GrantAllPrivilegesToSuperuser(default_superuser.username);
}

// 获取当前时间字符串
std::string UserManager::GetCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time_t));
    return std::string(buffer);
}

// 为超级用户授予所有权限
void UserManager::GrantAllPrivilegesToSuperuser(const std::string& username) {
    GrantPrivilege(username, "*", "*", PRIVILEGE_ALL);
}

// 移除用户的所有权限
void UserManager::RemoveUserPrivileges(const std::string& username) {
    permissions_.erase(
        std::remove_if(permissions_.begin(), permissions_.end(),
                      [&username](const Permission& p) {
                          return p.grantee == username && !p.is_role;
                      }),
        permissions_.end());
}

// 移除角色的所有权限
void UserManager::RemoveRolePrivileges(const std::string& role_name) {
    permissions_.erase(
        std::remove_if(permissions_.begin(), permissions_.end(),
                      [&role_name](const Permission& p) {
                          return p.grantee == role_name && p.is_role;
                      }),
        permissions_.end());
}

// 验证角色是否有效
bool UserManager::IsValidRole(const std::string& role_name) const {
    return roles_.find(role_name) != roles_.end();
}

// 密码哈希函数
std::string UserManager::HashPassword(const std::string& password) const {
    // 简单的哈希实现，实际应用中应使用更强的哈希算法如bcrypt
    std::hash<std::string> hasher;
    return std::to_string(hasher(password));
}

// 创建用户
bool UserManager::CreateUser(const std::string& username, const std::string& password, const std::string& role) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查用户是否已存在
    if (users_.find(username) != users_.end()) {
        last_error_ = "User '" + username + "' already exists";
        return false;
    }
    
    // 检查角色是否有效
    if (!IsValidRole(role)) {
        last_error_ = "Invalid role: " + role;
        return false;
    }
    
    // 创建新用户
    User new_user;
    new_user.username = username;
    new_user.password_hash = HashPassword(password);
    new_user.role = role;
    new_user.current_role = role;
    new_user.is_active = true;
    new_user.created_at = GetCurrentTimeString();
    users_[username] = new_user;
    
    // 保存到文件（已持有锁，调用内部版本）
    SaveToFileInternal();
    
    return true;
}

// 删除用户
bool UserManager::DropUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查用户是否存在
    auto it = users_.find(username);
    if (it == users_.end()) {
        last_error_ = "User '" + username + "' does not exist";
        return false;
    }
    
    // 移除用户权限
    RemoveUserPrivileges(username);
    
    // 删除用户
    users_.erase(it);
    
    // 保存到文件（已持有锁，调用内部版本）
    SaveToFileInternal();
    
    return true;
}

// 修改用户密码
bool UserManager::AlterUserPassword(const std::string& username, const std::string& new_password) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 查找用户
    auto it = users_.find(username);
    if (it == users_.end()) {
        last_error_ = "User '" + username + "' does not exist";
        return false;
    }
    
    // 更新密码
    it->second.password_hash = HashPassword(new_password);
    
    // 保存到文件（已持有锁，调用内部版本）
    SaveToFileInternal();
    
    return true;
}

// 修改用户角色
bool UserManager::AlterUserRole(const std::string& username, const std::string& new_role) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 查找用户
    auto it = users_.find(username);
    if (it == users_.end()) {
        last_error_ = "User '" + username + "' does not exist";
        return false;
    }
    
    // 检查角色是否有效
    if (!IsValidRole(new_role)) {
        last_error_ = "Invalid role: " + new_role;
        return false;
    }
    
    // 更新角色
    it->second.role = new_role;
    it->second.current_role = new_role;
    
    // 保存到文件（已持有锁，调用内部版本）
    SaveToFileInternal();
    
    return true;
}

// 用户认证
bool UserManager::AuthenticateUser(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 查找用户
    auto it = users_.find(username);
    if (it == users_.end()) {
        last_error_ = "User '" + username + "' does not exist";
        return false;
    }
    
    // 检查用户是否激活
    if (!it->second.is_active) {
        last_error_ = "User '" + username + "' is not active";
        return false;
    }
    
    // 验证密码
    return it->second.password_hash == HashPassword(password);
}

// 创建角色
bool UserManager::CreateRole(const std::string& role_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查角色是否已存在
    if (roles_.find(role_name) != roles_.end()) {
        last_error_ = "Role '" + role_name + "' already exists";
        return false;
    }
    
    // 创建新角色
    Role new_role;
    new_role.role_name = role_name;
    new_role.created_at = GetCurrentTimeString();
    roles_[role_name] = new_role;
    
    // 保存到文件（已持有锁，调用内部版本）
    SaveToFileInternal();
    
    return true;
}

// 删除角色
bool UserManager::DropRole(const std::string& role_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查角色是否存在
    auto it = roles_.find(role_name);
    if (it == roles_.end()) {
        last_error_ = "Role '" + role_name + "' does not exist";
        return false;
    }
    
    // 移除角色权限
    RemoveRolePrivileges(role_name);
    
    // 删除角色
    roles_.erase(it);
    
    // 保存到文件（已持有锁，调用内部版本）
    SaveToFileInternal();
    
    return true;
}

// 修改角色
bool UserManager::AlterRole(const std::string& role_name, const std::string& new_role_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 查找角色
    auto it = roles_.find(role_name);
    if (it == roles_.end()) {
        last_error_ = "Role '" + role_name + "' does not exist";
        return false;
    }
    
    // 检查新角色名是否已存在
    if (roles_.find(new_role_name) != roles_.end()) {
        last_error_ = "Role '" + new_role_name + "' already exists";
        return false;
    }
    
    // 更新角色名
    Role role = it->second;
    role.role_name = new_role_name;
    roles_.erase(it);
    roles_[new_role_name] = role;
    
    // 更新权限中的角色名
    for (auto& permission : permissions_) {
        if (permission.grantee == role_name && permission.is_role) {
            permission.grantee = new_role_name;
        }
    }
    
    // 更新用户的角色
    for (auto& user_pair : users_) {
        User& user = user_pair.second;
        if (user.role == role_name) {
            user.role = new_role_name;
        }
        if (user.current_role == role_name) {
            user.current_role = new_role_name;
        }
    }
    
    // 保存到文件（已持有锁，调用内部版本）
    SaveToFileInternal();
    
    return true;
}

// 设置用户当前角色
bool UserManager::SetCurrentRole(const std::string& username, const std::string& role_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 查找用户
    auto it = users_.find(username);
    if (it == users_.end()) {
        last_error_ = "User '" + username + "' does not exist";
        return false;
    }
    
    // 检查角色是否有效
    if (!IsValidRole(role_name)) {
        last_error_ = "Invalid role: " + role_name;
        return false;
    }
    
    // 更新当前角色
    it->second.current_role = role_name;
    
    // 保存到文件（已持有锁，调用内部版本）
    SaveToFileInternal();
    
    return true;
}

// 获取用户当前角色
std::string UserManager::GetUserCurrentRole(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 查找用户
    auto it = users_.find(username);
    if (it == users_.end()) {
        last_error_ = "User '" + username + "' does not exist";
        return "";
    }
    
    return it->second.current_role;
}

// 授予权限
bool UserManager::GrantPrivilege(const std::string& grantee, const std::string& database,
                                const std::string& table, const std::string& privilege) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查被授权者是否存在
    bool is_role = false;
    if (users_.find(grantee) == users_.end()) {
        if (roles_.find(grantee) == roles_.end()) {
            last_error_ = "Grantee '" + grantee + "' does not exist";
            return false;
        }
        is_role = true;
    }
    
    // 创建权限对象
    Permission permission;
    permission.grantee = grantee;
    permission.database = database;
    permission.table = table;
    permission.privilege = privilege;
    permission.is_role = is_role;
    
    // 添加权限到内存
    permissions_.push_back(permission);
    
    // 同步到SystemDatabase
    if (sys_db_ != nullptr) {
        std::string grantee_type = is_role ? "ROLE" : "USER";
        if (!sys_db_->GrantPrivilegeRecord(grantee_type, grantee, database, table, privilege, "admin")) {
            // 记录错误但不回滚内存操作（保持兼容性）
            // TODO: 考虑是否需要事务性回滚
        }
    }
    
    // 保存到文件（已持有锁，调用内部版本）
    SaveToFileInternal();
    
    return true;
}

// 撤销权限
bool UserManager::RevokePrivilege(const std::string& grantee, const std::string& database,
                                 const std::string& table, const std::string& privilege) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 查找并移除权限
    auto it = std::remove_if(permissions_.begin(), permissions_.end(),
                           [&grantee, &database, &table, &privilege](const Permission& p) {
                               return p.grantee == grantee &&
                                      p.database == database &&
                                      p.table == table &&
                                      p.privilege == privilege;
                           });
    
    if (it == permissions_.end()) {
        last_error_ = "Permission not found";
        return false;
    }
    
    // 获取被移除的权限信息（用于同步到SystemDatabase）
    bool is_role = it->is_role;
    
    // 从内存中移除权限
    permissions_.erase(it, permissions_.end());
    
    // 同步到SystemDatabase
    if (sys_db_ != nullptr) {
        std::string grantee_type = is_role ? "ROLE" : "USER";
        if (!sys_db_->RevokePrivilegeRecord(grantee_type, grantee, database, table, privilege)) {
            // 记录错误但不回滚内存操作（保持兼容性）
            // TODO: 考虑是否需要事务性回滚
        }
    }
    
    // 保存到文件（已持有锁，调用内部版本）
    SaveToFileInternal();
    
    return true;
}

// 检查权限
bool UserManager::CheckPermission(const std::string& username, const std::string& database,
                                const std::string& table, const std::string& required_privilege) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 查找用户
    auto user_it = users_.find(username);
    if (user_it == users_.end()) {
        last_error_ = "User '" + username + "' does not exist";
        return false;
    }
    
    const User& user = user_it->second;
    
    // 超级用户拥有所有权限
    if (user.role == ROLE_SUPERUSER) {
        return true;
    }
    
    // 检查用户权限
    for (const auto& permission : permissions_) {
        if (permission.grantee == username && !permission.is_role) {
            if ((permission.database == "*" || permission.database == database) &&
                (permission.table == "*" || permission.table == table) &&
                (permission.privilege == PRIVILEGE_ALL || permission.privilege == required_privilege)) {
                return true;
            }
        }
    }
    
    // 检查角色权限
    for (const auto& permission : permissions_) {
        if (permission.grantee == user.current_role && permission.is_role) {
            if ((permission.database == "*" || permission.database == database) &&
                (permission.table == "*" || permission.table == table) &&
                (permission.privilege == PRIVILEGE_ALL || permission.privilege == required_privilege)) {
                return true;
            }
        }
    }
    
    return false;
}

// 列出所有用户
std::vector<User> UserManager::ListUsers() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<User> result;
    for (const auto& pair : users_) {
        result.push_back(pair.second);
    }
    return result;
}

// 列出所有角色
std::vector<Role> UserManager::ListRoles() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<Role> result;
    for (const auto& pair : roles_) {
        result.push_back(pair.second);
    }
    return result;
}

// 列出用户权限
std::vector<Permission> UserManager::ListUserPermissions(const std::string& username) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<Permission> result;
    for (const auto& permission : permissions_) {
        if (permission.grantee == username && !permission.is_role) {
            result.push_back(permission);
        }
    }
    return result;
}

// 列出角色权限
std::vector<Permission> UserManager::ListRolePermissions(const std::string& role_name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<Permission> result;
    for (const auto& permission : permissions_) {
        if (permission.grantee == role_name && permission.is_role) {
            result.push_back(permission);
        }
    }
    return result;
}

// 保存到文件（内部版本，不加锁）
bool UserManager::SaveToFileInternal() const {
    try {
        // 创建用户数据文件
        std::ofstream user_file(data_path_ + "/users.dat");
        if (!user_file.is_open()) {
            last_error_ = "Failed to open user data file for writing";
            return false;
        }
        
        // 保存用户数据
        user_file << users_.size() << std::endl;
        for (const auto& pair : users_) {
            const User& user = pair.second;
            user_file << user.username << std::endl;
            user_file << user.password_hash << std::endl;
            user_file << user.role << std::endl;
            user_file << user.current_role << std::endl;
            user_file << (user.is_active ? "1" : "0") << std::endl;
            user_file << user.created_at << std::endl;
        }
        user_file.close();
        
        // 创建角色数据文件
        std::ofstream role_file(data_path_ + "/roles.dat");
        if (!role_file.is_open()) {
            last_error_ = "Failed to open role data file for writing";
            return false;
        }
        
        // 保存角色数据
        role_file << roles_.size() << std::endl;
        for (const auto& pair : roles_) {
            const Role& role = pair.second;
            role_file << role.role_name << std::endl;
            role_file << role.created_at << std::endl;
        }
        role_file.close();
        
        // 创建权限数据文件
        std::ofstream perm_file(data_path_ + "/permissions.dat");
        if (!perm_file.is_open()) {
            last_error_ = "Failed to open permission data file for writing";
            return false;
        }
        
        // 保存权限数据
        perm_file << permissions_.size() << std::endl;
        for (const auto& permission : permissions_) {
            perm_file << permission.grantee << std::endl;
            perm_file << permission.database << std::endl;
            perm_file << permission.table << std::endl;
            perm_file << permission.privilege << std::endl;
            perm_file << (permission.is_role ? "1" : "0") << std::endl;
        }
        perm_file.close();
        
        return true;
    } catch (const std::exception& e) {
        last_error_ = "Failed to save user data: " + std::string(e.what());
        return false;
    }
}

// 保存到文件（公共版本，加锁）
bool UserManager::SaveToFile() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return SaveToFileInternal();
}

// 从文件加载
bool UserManager::LoadFromFile() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        // 检查用户文件是否存在
        std::string user_file_path = data_path_ + "/users.dat";
        if (!fs::exists(user_file_path)) {
            return false; // 文件不存在，返回false表示需要初始化
        }
        
        // 读取用户数据
        std::ifstream user_file(user_file_path);
        if (!user_file.is_open()) {
            last_error_ = "Failed to open user data file for reading";
            return false;
        }
        
        size_t user_count;
        user_file >> user_count;
        user_file.ignore(); // 忽略换行符
        
        for (size_t i = 0; i < user_count; ++i) {
            User user;
            std::getline(user_file, user.username);
            std::getline(user_file, user.password_hash);
            std::getline(user_file, user.role);
            std::getline(user_file, user.current_role);
            
            std::string active_str;
            std::getline(user_file, active_str);
            user.is_active = (active_str == "1");
            
            std::getline(user_file, user.created_at);
            
            users_[user.username] = user;
        }
        user_file.close();
        
        // 读取角色数据
        std::string role_file_path = data_path_ + "/roles.dat";
        if (!fs::exists(role_file_path)) {
            return false;
        }
        
        std::ifstream role_file(role_file_path);
        if (!role_file.is_open()) {
            last_error_ = "Failed to open role data file for reading";
            return false;
        }
        
        size_t role_count;
        role_file >> role_count;
        role_file.ignore(); // 忽略换行符
        
        for (size_t i = 0; i < role_count; ++i) {
            Role role;
            std::getline(role_file, role.role_name);
            std::getline(role_file, role.created_at);
            
            roles_[role.role_name] = role;
        }
        role_file.close();
        
        // 读取权限数据
        std::string perm_file_path = data_path_ + "/permissions.dat";
        if (!fs::exists(perm_file_path)) {
            return false;
        }
        
        std::ifstream perm_file(perm_file_path);
        if (!perm_file.is_open()) {
            last_error_ = "Failed to open permission data file for reading";
            return false;
        }
        
        size_t perm_count;
        perm_file >> perm_count;
        perm_file.ignore(); // 忽略换行符
        
        for (size_t i = 0; i < perm_count; ++i) {
            Permission permission;
            std::getline(perm_file, permission.grantee);
            std::getline(perm_file, permission.database);
            std::getline(perm_file, permission.table);
            std::getline(perm_file, permission.privilege);
            
            std::string role_str;
            std::getline(perm_file, role_str);
            permission.is_role = (role_str == "1");
            
            permissions_.push_back(permission);
        }
        perm_file.close();
        
        return true;
    } catch (const std::exception& e) {
        last_error_ = "Failed to load user data: " + std::string(e.what());
        return false;
    }
}

// 获取最后错误信息
const std::string& UserManager::GetLastError() const {
    return last_error_;
}

} // namespace sqlcc