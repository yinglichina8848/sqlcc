/**
 * @file user_manager.h
 * @brief SQLCC用户管理器 - 身份认证与权限控制（RBAC）核心
 *
 * UserManager 是数据库安全系统的基石，负责管理用户（User）、角色（Role）
 * 以及细粒度的权限（Permission）。它实现了基于角色的访问控制（RBAC）模型，
 * 确保只有经过身份验证的用户才能在被授权的范围内访问数据库资源。
 *
 * 📚 配套教材参考：
 * - [第10章：数据库安全性](../../textbook/《数据库系统原理与开发实践》.md#第十章数据库安全性)
 * - [10.2 存取控制技术](../../textbook/《数据库系统原理与开发实践》.md#102-存取控制技术)
 * - [10.3 角色授权](../../textbook/《数据库系统原理与开发实践》.md#103-角色授权)
 *
 * WHY层 - 设计意图：
 *   1. **最小权限原则**：通过 RBAC 模型，可以将复杂的权限管理简化为角色的分配，
 *      避免直接对用户进行繁琐的权限配置，降低管理错误风险。
 *   2. **安全性**：集中管理认证逻辑，确保密码（哈希）的安全存储和验证。
 *   3. **隔离性**：配合 SchemaManager 和 ExecutionContext，实现多租户间的数据隔离。
 *   4. **审计合规**：所有权限变更操作都应被记录，满足企业级审计需求。
 *
 * WHAT层 - 功能说明：
 *   - 用户管理：创建/删除用户，密码修改，认证（Authentication）。
 *   - 角色管理：创建/删除角色，角色继承（Role Inheritance）。
 *   - 授权管理：GRANT/REVOKE 操作，支持对象级（Database/Table）权限。
 *   - 权限检查：CheckPermission 快速验证用户对特定资源的操作权限。
 *   - 持久化：将安全元数据持久化到 SystemDatabase 或文件系统。
 *
 * HOW层 - 实现机制：
 *   - **权限矩阵**：在内存中维护一个高效的哈希表（PermissionMatrix），实现 O(1) 的权限查找。
 *   - **角色继承树**：支持角色的层级结构（如 Manager 继承 Employee），权限检查时递归遍历。
 *   - **哈希存储**：密码仅存储加盐哈希值（Salted Hash），防止明文泄露。
 *   - **原子变更**：权限修改操作通过互斥锁保护，并支持事务性持久化。
 *
 * @author SQLCC技术委员会
 * @version 1.2.6
 * @date 2026-02-02
 */

#pragma once

// Standard library includes - minimized for faster compilation
#include <string>           // std::string
#include <memory>           // std::shared_ptr, std::unique_ptr
#include <vector>           // std::vector
#include <unordered_map>    // std::unordered_map
#include <mutex>            // std::mutex for thread safety

namespace sqlcc {

// Forward declarations for SystemDatabase
class SystemDatabase;

/**
 * @brief 角色数据结构 - RBAC模型的核心单元
 */
struct Role {
  std::string role_name;          ///< 角色名称（主键）
  std::string created_at;         ///< 创建时间
  std::vector<std::string> parent_roles;  ///< 父角色（继承其权限）
  std::vector<std::string> child_roles;   ///< 子角色（被其继承）
};

/**
 * @brief 用户数据结构
 */
struct User {
  std::string username;           ///< 用户名（主键）
  std::string password_hash;      ///< 密码哈希值（BCrypt/Argon2）
  std::string role;               ///< 默认角色
  std::string current_role;       ///< 当前活跃角色
  bool is_active;                 ///< 账户状态
  std::string created_at;         ///< 创建时间
};

/**
 * @brief 权限条目
 * 描述 "谁（Grantee）" 对 "什么（Resource）" 拥有 "何种权利（Privilege）"。
 */
struct Permission {
  std::string grantee; ///< 授权对象（用户或角色）
  std::string database;///< 数据库名（*表示所有）
  std::string table;   ///< 表名（*表示所有）
  std::string privilege;///< 权限类型（SELECT, INSERT...）
  bool is_role;        ///< grantee 是否为角色
};

/**
 * @brief 权限矩阵键 - 用于快速查找
 */
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

/**
 * @brief 权限矩阵哈希函数
 */
struct PermissionKeyHash {
  std::size_t operator()(const PermissionKey &key) const {
    return std::hash<std::string>{}(key.grantee) ^
           std::hash<std::string>{}(key.database) ^
           std::hash<std::string>{}(key.table) ^
           std::hash<std::string>{}(key.privilege);
  }
};

struct PermissionValue {
  bool has_permission;
  bool is_role;
};

using UserRoleMap = std::unordered_map<std::string, std::string>;

/**
 * @class UserManager
 * @brief 用户与权限管理器
 */
class UserManager {
public:
  // --- 预定义角色与权限 ---
  static inline const std::string ROLE_SUPERUSER = "SUPERUSER";
  static inline const std::string ROLE_ADMIN = "ADMIN";
  static inline const std::string ROLE_USER = "USER";

  static inline const std::string PRIVILEGE_CREATE = "CREATE";
  static inline const std::string PRIVILEGE_SELECT = "SELECT";
  static inline const std::string PRIVILEGE_INSERT = "INSERT";
  static inline const std::string PRIVILEGE_UPDATE = "UPDATE";
  static inline const std::string PRIVILEGE_DELETE = "DELETE";
  static inline const std::string PRIVILEGE_DROP = "DROP";
  static inline const std::string PRIVILEGE_ALTER = "ALTER";
  static inline const std::string PRIVILEGE_ALL = "ALL";

  /**
   * @brief 构造函数
   * @param data_path 数据持久化路径
   */
  UserManager(const std::string &data_path = "./data");
  ~UserManager();

  void SetSystemDatabase(std::shared_ptr<SystemDatabase> sys_db);
  std::shared_ptr<SystemDatabase> GetSystemDatabase() const { return sys_db_; }

  // --- 用户管理 ---
  
  /**
   * @brief 创建新用户
   * @return true 成功, false 若用户已存在
   */
  bool CreateUser(const std::string &username, const std::string &password,
                  const std::string &role = "USER");
  bool DropUser(const std::string &username);
  bool AlterUserPassword(const std::string &username,
                         const std::string &new_password);
  bool AlterUserRole(const std::string &username, const std::string &new_role);
  
  /**
   * @brief 用户认证
   * 
   * HOW: 验证输入的密码与存储的哈希值是否匹配。
   */
  bool AuthenticateUser(const std::string &username,
                        const std::string &password);

  // --- 角色管理 ---
  
  bool CreateRole(const std::string &role_name);
  bool DropRole(const std::string &role_name);
  bool AlterRole(const std::string &role_name,
                 const std::string &new_role_name);
  
  /**
   * @brief 切换当前活跃角色
   * 
   * WHY: 遵循最小权限原则，用户平时可能使用低权限角色，仅在需要时切换到高权限角色。
   */
  bool SetCurrentRole(const std::string &username,
                      const std::string &role_name);
  std::string GetUserCurrentRole(const std::string &username) const;

  // --- 权限管理 ---

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
  
  /**
   * @brief 获取用户的有效权限集
   * 包含直接赋予的权限和通过角色继承的所有权限。
   */
  std::vector<std::string> GetEffectivePermissions(const std::string &username,
                                                    const std::string &database,
                                                    const std::string &table) const;

  bool GrantPrivilege(const std::string &grantee, const std::string &database,
                      const std::string &table, const std::string &privilege);
  bool RevokePrivilege(const std::string &grantee, const std::string &database,
                       const std::string &table, const std::string &privilege);
  
  /**
   * @brief 核心权限检查接口
   * 
   * HOW:
   * 1. 检查用户是否直接拥有权限。
   * 2. 检查用户当前角色是否拥有权限。
   * 3. 递归检查角色的父角色是否拥有权限。
   * 
   * @return true 若拥有权限
   */
  bool CheckPermission(const std::string &username, const std::string &database,
                       const std::string &table,
                       const std::string &required_privilege);

  // --- 查询接口 ---
  bool userExists(const std::string &username) const;
  bool isUserInRole(const std::string &username, const std::string &role_name) const;
  std::vector<User> ListUsers() const;
  std::vector<Role> ListRoles() const;
  std::vector<Permission> ListUserPermissions(const std::string &username) const;
  std::vector<Permission> ListRolePermissions(const std::string &role_name) const;

  // --- 持久化 ---
  bool SaveToFile() const;
  bool LoadFromFile();

  const std::string &GetLastError() const;

private:
  void CreateDefaultSuperuser();
  std::string GetCurrentTimeString();
  void GrantAllPrivilegesToSuperuser(const std::string &username);
  void RemoveUserPrivileges(const std::string &username);
  void RemoveRolePrivileges(const std::string &role_name);
  bool IsValidRole(const std::string &role_name) const;
  std::string HashPassword(const std::string &password) const;
  bool SaveToFileInternal() const;
  
  // 权限矩阵优化
  void InitializePermissionMatrix();
  void AddPermissionToMatrix(const Permission &permission);
  void RemovePermissionFromMatrix(const Permission &permission);
  bool CheckPermissionInMatrix(
      const std::string &username, const std::string &database,
      const std::string &table,
      const std::string &required_privilege) const;
  void UpdateUserCurrentRole(const std::string &username,
                             const std::string &role_name);

  std::unordered_map<std::string, User> users_;
  std::unordered_map<std::string, Role> roles_;
  std::vector<Permission> permissions_;
  mutable std::string last_error_;
  std::string data_path_;
  std::shared_ptr<SystemDatabase> sys_db_;
  mutable std::mutex mutex_;

  // 内存加速结构
  std::unordered_map<PermissionKey, PermissionValue, PermissionKeyHash> permission_matrix_;
  UserRoleMap user_current_roles_;
};

} // namespace sqlcc
