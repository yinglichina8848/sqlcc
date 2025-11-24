#include "user_manager.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <random>

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

UserManager::UserManager() {
  // 创建默认超级用户
  CreateDefaultSuperuser();
}

UserManager::~UserManager() = default;

// 创建默认超级用户
void UserManager::CreateDefaultSuperuser() {
  // 创建默认超级用户，用户名: admin, 密码: admin
  User default_superuser;
  default_superuser.username = "admin";
  // 密码暂存明文，实际应用中应使用哈希存储
  default_superuser.password_hash = "admin";
  default_superuser.role = ROLE_SUPERUSER;
  default_superuser.is_active = true;
  default_superuser.created_at = GetCurrentTimeString();

  users_[default_superuser.username] = default_superuser;

  // 默认超级用户拥有所有数据库和表的所有权限
  GrantAllPrivilegesToSuperuser(default_superuser.username);
}

// 获取当前时间字符串
std::string UserManager::GetCurrentTimeString() {
  auto now = std::chrono::system_clock::now();
  auto now_time_t = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm = *std::localtime(&now_time_t);

  char buffer[20];
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &now_tm);
  return std::string(buffer);
}

// 为超级用户授予所有权限
void UserManager::GrantAllPrivilegesToSuperuser(const std::string &username) {
  // 为所有资源类型授予所有权限
  GrantPrivilege(username, "*", "*", PRIVILEGE_ALL);
}

// 创建新用户
bool UserManager::CreateUser(const std::string &username,
                             const std::string &password,
                             const std::string &role) {
  std::lock_guard<std::mutex> lock(mutex_);

  // 检查用户是否已存在
  if (users_.find(username) != users_.end()) {
    last_error_ = "User already exists: " + username;
    return false;
  }

  // 验证角色有效性
  if (role != ROLE_SUPERUSER && role != ROLE_ADMIN && role != ROLE_USER) {
    last_error_ = "Invalid role: " + role;
    return false;
  }

  // 创建用户
  User new_user;
  new_user.username = username;
  // 密码暂存明文，实际应用中应使用哈希存储
  new_user.password_hash = password;
  new_user.role = role;
  new_user.is_active = true;
  new_user.created_at = GetCurrentTimeString();

  users_[username] = new_user;

  // 默认情况下，普通用户没有权限
  if (role == ROLE_USER) {
    // 用户可以查看自己的权限信息
    GrantPrivilege(username, "*", "user_permissions", PRIVILEGE_SELECT);
  }

  // 管理员拥有管理权限但不是超级用户权限
  else if (role == ROLE_ADMIN) {
    GrantPrivilege(username, "*", "*", PRIVILEGE_SELECT);
    GrantPrivilege(username, "*", "*", PRIVILEGE_INSERT);
    GrantPrivilege(username, "*", "*", PRIVILEGE_UPDATE);
    GrantPrivilege(username, "*", "*", PRIVILEGE_DELETE);
  }

  // 超级用户拥有所有权限
  else if (role == ROLE_SUPERUSER) {
    GrantAllPrivilegesToSuperuser(username);
  }

  return true;
}

// 删除用户
bool UserManager::DropUser(const std::string &username) {
  std::lock_guard<std::mutex> lock(mutex_);

  // 检查用户是否存在
  if (users_.find(username) == users_.end()) {
    last_error_ = "User not found: " + username;
    return false;
  }

  // 不允许删除默认超级用户
  if (username == "admin") {
    last_error_ = "Cannot drop default superuser 'admin'";
    return false;
  }

  // 删除用户
  users_.erase(username);

  // 删除用户的所有权限
  RemoveUserPrivileges(username);

  return true;
}

// 更改用户密码
bool UserManager::AlterUserPassword(const std::string &username,
                                    const std::string &new_password) {
  std::lock_guard<std::mutex> lock(mutex_);

  // 检查用户是否存在
  auto it = users_.find(username);
  if (it == users_.end()) {
    last_error_ = "User not found: " + username;
    return false;
  }

  // 更新密码
  it->second.password_hash = new_password;
  return true;
}

// 用户登录验证
bool UserManager::AuthenticateUser(const std::string &username,
                                   const std::string &password) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = users_.find(username);
  if (it == users_.end()) {
    last_error_ = "Invalid username or password";
    return false;
  }

  // 检查用户是否被禁用
  if (!it->second.is_active) {
    last_error_ = "User account is disabled";
    return false;
  }

  // 简单的密码验证，实际应用中应使用哈希比较
  if (it->second.password_hash != password) {
    last_error_ = "Invalid username or password";
    return false;
  }

  return true;
}

// 授权权限
bool UserManager::GrantPrivilege(const std::string &username,
                                 const std::string &database,
                                 const std::string &table,
                                 const std::string &privilege) {
  std::lock_guard<std::mutex> lock(mutex_);

  // 检查用户是否存在
  if (users_.find(username) == users_.end()) {
    last_error_ = "User not found: " + username;
    return false;
  }

  // 创建权限项
  Permission perm;
  perm.username = username;
  perm.database = database;
  perm.table = table;
  perm.privilege = privilege;

  // 添加到权限集合
  permissions_.push_back(perm);

  return true;
}

// 撤销权限
bool UserManager::RevokePrivilege(const std::string &username,
                                  const std::string &database,
                                  const std::string &table,
                                  const std::string &privilege) {
  std::lock_guard<std::mutex> lock(mutex_);

  // 不允许撤销超级用户的权限
  auto user_it = users_.find(username);
  if (user_it != users_.end() && user_it->second.role == ROLE_SUPERUSER) {
    last_error_ = "Cannot revoke privileges from superuser";
    return false;
  }

  // 查找并移除匹配的权限
  auto new_end = std::remove_if(
      permissions_.begin(), permissions_.end(), [&](const Permission &perm) {
        return perm.username == username &&
               (database == "*" || perm.database == database) &&
               (table == "*" || perm.table == table) &&
               (privilege == "ALL" || perm.privilege == privilege);
      });

  if (new_end == permissions_.end()) {
    last_error_ = "No matching privileges found";
    return false;
  }

  permissions_.erase(new_end, permissions_.end());
  return true;
}

// 检查用户是否有权限执行操作
bool UserManager::CheckPermission(const std::string &username,
                                  const std::string &database,
                                  const std::string &table,
                                  const std::string &required_privilege) {
  std::lock_guard<std::mutex> lock(mutex_);

  // 检查用户是否存在
  auto user_it = users_.find(username);
  if (user_it == users_.end()) {
    return false;
  }

  // 超级用户拥有所有权限
  if (user_it->second.role == ROLE_SUPERUSER) {
    return true;
  }

  // 检查是否有权限
  for (const auto &perm : permissions_) {
    // 用户匹配
    if (perm.username != username)
      continue;

    // 数据库匹配（精确匹配或通配符）
    if (perm.database != "*" && perm.database != database)
      continue;

    // 表匹配（精确匹配或通配符）
    if (perm.table != "*" && perm.table != table)
      continue;

    // 权限匹配：ALL权限或精确权限匹配
    if (perm.privilege == PRIVILEGE_ALL ||
        perm.privilege == required_privilege) {
      return true;
    }
  }

  return false;
}

// 列出所有用户
std::vector<User> UserManager::ListUsers() const {
  std::vector<User> user_list;

  for (const auto &pair : users_) {
    user_list.push_back(pair.second);
  }

  return user_list;
}

// 列出用户权限
std::vector<Permission>
UserManager::ListUserPermissions(const std::string &username) const {
  std::vector<Permission> user_permissions;

  for (const auto &perm : permissions_) {
    if (perm.username == username) {
      user_permissions.push_back(perm);
    }
  }

  return user_permissions;
}

// 获取最后一次错误信息
const std::string &UserManager::GetLastError() const { return last_error_; }

// 移除用户的所有权限
void UserManager::RemoveUserPrivileges(const std::string &username) {
  auto new_end = std::remove_if(
      permissions_.begin(), permissions_.end(),
      [&](const Permission &perm) { return perm.username == username; });

  permissions_.erase(new_end, permissions_.end());
}

} // namespace sqlcc
