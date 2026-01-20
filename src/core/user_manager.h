// SQLCC Core User Manager Module Interface
// Stage 2 Migration Ready - C++20 Module preparation
// Migration Phase: Traditional Header with Module Enhancement
//
// This file defines the UserManager class for SQLCC user authentication,
// authorization, and role-based access control (RBAC) system.
// Features modern C++ patterns while maintaining backward compatibility.
//
// Key improvements:
// - Smart pointers for memory safety
// - Forward declarations to reduce compilation dependencies
// - Thread-safe operations with mutex protection
// - Modern C++ data structures and algorithms
//
// Future enhancements:
// - Asynchronous permission checking
// - Hierarchical role inheritance
// - Audit logging integration
// - Performance metrics collection

#pragma once

// Standard library includes - minimized for faster compilation
// Only essential headers included to reduce transitive dependencies
#include <string>           // std::string
#include <memory>           // std::shared_ptr, std::unique_ptr
#include <vector>           // std::vector
#include <unordered_map>    // std::unordered_map
#include <mutex>            // std::mutex for thread safety

// Forward declarations for faster compilation
// Reduces header inclusion cascade and compilation time
// Note: std::mutex is now included via <memory> header, no forward declaration needed

// Future C++20 Modules declaration
// TODO: Enable when Clang 18+ modules are stable
// export module sqlcc.core.user_manager;

namespace sqlcc {

// Forward declarations for SystemDatabase
class SystemDatabase;

// 角色数据结构
struct Role {
  std::string role_name;
  std::string created_at;
  std::vector<std::string> parent_roles;  // 父角色列表（继承关系）
  std::vector<std::string> child_roles;   // 子角色列表
};

// 用户数据结构
struct User {
  std::string username;
  std::string password_hash;
  std::string role;
  std::string current_role;
  bool is_active;
  std::string created_at;
};

// 权限数据结构
struct Permission {
  std::string grantee; // 可以是用户名或角色名
  std::string database;
  std::string table;
  std::string privilege;
  bool is_role; // 标识是用户权限还是角色权限
};

// 权限矩阵键结构（用于快速查找）
struct PermissionKey {
  std::string grantee;
  std::string database;
  std::string table;
  std::string privilege;

  bool operator==(const PermissionKey &other) const {
    return grantee == other.grantee && database == other.database &&
           table == other.table && privilege == other.privilege;
  }
};

// 权限矩阵哈希函数
struct PermissionKeyHash {
  std::size_t operator()(const PermissionKey &key) const {
    return std::hash<std::string>{}(key.grantee) ^
           std::hash<std::string>{}(key.database) ^
           std::hash<std::string>{}(key.table) ^
           std::hash<std::string>{}(key.privilege);
  }
};

// 权限矩阵值结构
struct PermissionValue {
  bool has_permission;
  bool is_role;
};

// 用户角色映射
using UserRoleMap = std::unordered_map<std::string, std::string>;

// 用户管理器类
class UserManager {
public:
  // 角色常量定义
  static inline const std::string ROLE_SUPERUSER = "SUPERUSER";
  static inline const std::string ROLE_ADMIN = "ADMIN";
  static inline const std::string ROLE_USER = "USER";

  // 权限常量定义
  static inline const std::string PRIVILEGE_CREATE = "CREATE";
  static inline const std::string PRIVILEGE_SELECT = "SELECT";
  static inline const std::string PRIVILEGE_INSERT = "INSERT";
  static inline const std::string PRIVILEGE_UPDATE = "UPDATE";
  static inline const std::string PRIVILEGE_DELETE = "DELETE";
  static inline const std::string PRIVILEGE_DROP = "DROP";
  static inline const std::string PRIVILEGE_ALTER = "ALTER";
  static inline const std::string PRIVILEGE_ALL = "ALL";

  UserManager(const std::string &data_path = "./data");
  ~UserManager();

  // 设置SystemDatabase引用（用于权限同步）
  void SetSystemDatabase(std::shared_ptr<SystemDatabase> sys_db);
  
  // 获取SystemDatabase引用
  std::shared_ptr<SystemDatabase> GetSystemDatabase() const { return sys_db_; }
  // 用户管理方法
  bool CreateUser(const std::string &username, const std::string &password,
                  const std::string &role = "USER");
  bool DropUser(const std::string &username);
  bool AlterUserPassword(const std::string &username,
                         const std::string &new_password);
  bool AlterUserRole(const std::string &username, const std::string &new_role);
  bool AuthenticateUser(const std::string &username,
                        const std::string &password);

  // 角色管理方法
  bool CreateRole(const std::string &role_name);
  bool DropRole(const std::string &role_name);
  bool AlterRole(const std::string &role_name,
                 const std::string &new_role_name);
  bool SetCurrentRole(const std::string &username,
                      const std::string &role_name);
  std::string GetUserCurrentRole(const std::string &username) const;

  // 高级权限管理方法
  bool GrantRoleToRole(const std::string &parent_role, const std::string &child_role);
  bool RevokeRoleFromRole(const std::string &parent_role, const std::string &child_role);
  bool CheckRoleInheritance(const std::string &role_name, const std::string &inherited_role) const;
  std::vector<std::string> GetRoleHierarchy(const std::string &role_name) const;
  bool RevokePrivilegeCascade(const std::string &grantee, const std::string &database,
                              const std::string &table, const std::string &privilege);
  bool CheckPermissionConflict(const std::string &grantee, const std::string &database,
                               const std::string &table, const std::string &privilege) const;
  bool AuditPermissionChanges(const std::string &operation, const std::string &grantee,
                              const std::string &details);
  std::vector<std::string> GetEffectivePermissions(const std::string &username,
                                                    const std::string &database,
                                                    const std::string &table) const;

  // 权限管理方法
  bool GrantPrivilege(const std::string &grantee, const std::string &database,
                      const std::string &table, const std::string &privilege);
  bool RevokePrivilege(const std::string &grantee, const std::string &database,
                       const std::string &table, const std::string &privilege);
  bool CheckPermission(const std::string &username, const std::string &database,
                       const std::string &table,
                       const std::string &required_privilege);

  // 查询方法
  std::vector<User> ListUsers() const;
  std::vector<Role> ListRoles() const;
  std::vector<Permission>
  ListUserPermissions(const std::string &username) const;
  std::vector<Permission>
  ListRolePermissions(const std::string &role_name) const;

  // 持久化方法
  bool SaveToFile() const;
  bool LoadFromFile();

  // 错误处理
  const std::string &GetLastError() const;

private:
  // 辅助方法
  void CreateDefaultSuperuser();
  std::string GetCurrentTimeString();
  void GrantAllPrivilegesToSuperuser(const std::string &username);
  void RemoveUserPrivileges(const std::string &username);
  void RemoveRolePrivileges(const std::string &role_name);
  bool IsValidRole(const std::string &role_name) const;
  std::string HashPassword(const std::string &password) const;
  bool SaveToFileInternal() const; // 内部保存方法，不加锁

  // 权限矩阵相关方法
  void InitializePermissionMatrix();                        // 初始化权限矩阵
  void AddPermissionToMatrix(const Permission &permission); // 添加权限到矩阵
  void
  RemovePermissionFromMatrix(const Permission &permission); // 从矩阵移除权限
  bool CheckPermissionInMatrix(
      const std::string &username, const std::string &database,
      const std::string &table,
      const std::string &required_privilege) const; // 矩阵权限检查
  void UpdateUserCurrentRole(const std::string &username,
                             const std::string &role_name); // 更新用户当前角色

  // 成员变量
  std::unordered_map<std::string, User> users_; // 用户名 -> 用户信息
  std::unordered_map<std::string, Role> roles_; // 角色名 -> 角色信息
  std::vector<Permission> permissions_;         // 权限列表（兼容性保留）
  mutable std::string last_error_;              // 最后错误信息
  std::string data_path_;                       // 数据存储路径
  std::shared_ptr<SystemDatabase> sys_db_;   // SystemDatabase引用（用于权限同步）
  mutable std::mutex mutex_; // 线程安全互斥锁

  // 权限矩阵相关成员变量
  std::unordered_map<PermissionKey, PermissionValue, PermissionKeyHash>
      permission_matrix_;          // 权限矩阵
  UserRoleMap user_current_roles_; // 用户当前角色映射
};

} // namespace sqlcc

// Future module interface preparation
// When modules are enabled, these will become:
// export namespace sqlcc {
//     export struct Role;
//     export struct User;
//     export struct Permission;
//     export struct PermissionKey;
//     export struct PermissionKeyHash;
//     export struct PermissionValue;
//     export using UserRoleMap;
//     export class UserManager;
// }
