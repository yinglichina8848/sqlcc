#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace sqlcc {

// 前向声明
class SystemDatabase;

// 角色数据结构
struct Role {
  std::string role_name;
  std::string created_at;
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
  static const std::string ROLE_SUPERUSER;
  static const std::string ROLE_ADMIN;
  static const std::string ROLE_USER;

  // 权限常量定义
  static const std::string PRIVILEGE_CREATE;
  static const std::string PRIVILEGE_SELECT;
  static const std::string PRIVILEGE_INSERT;
  static const std::string PRIVILEGE_UPDATE;
  static const std::string PRIVILEGE_DELETE;
  static const std::string PRIVILEGE_DROP;
  static const std::string PRIVILEGE_ALTER;
  static const std::string PRIVILEGE_ALL;

  UserManager(const std::string &data_path = "./data");
  ~UserManager();

  // 设置SystemDatabase引用（用于权限同步）
  void SetSystemDatabase(SystemDatabase *sys_db);

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
  std::string GetUserCurrentRole(const std::string &username);

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
  SystemDatabase *sys_db_;   // SystemDatabase引用（用于权限同步）
  mutable std::mutex mutex_; // 线程安全互斥锁

  // 权限矩阵相关成员变量
  std::unordered_map<PermissionKey, PermissionValue, PermissionKeyHash>
      permission_matrix_;          // 权限矩阵
  UserRoleMap user_current_roles_; // 用户当前角色映射
};

} // namespace sqlcc

#endif // USER_MANAGER_H