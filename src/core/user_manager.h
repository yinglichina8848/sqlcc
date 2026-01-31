// SQLCC Core User Manager Module Interface
// Stage 2 Migration Ready - C++20 Module preparation
// Migration Phase: Traditional Header with Module Enhancement
//
// @file user_manager.h
// @brief 定义了SQLCC数据库的用户管理器，负责用户认证、权限管理和基于角色的访问控制。
//
// @WHY
// 在多用户数据库系统中，安全性是至关重要的。用户管理和权限控制是实现数据安全的核心。
// 一个健壮的用户管理器需要解决以下问题：
// 1.  **身份认证 (Authentication)**: 验证用户的合法身份（例如，用户名和密码）。
// 2.  **授权 (Authorization)**: 根据用户的身份和角色，决定其对数据库对象（如表、数据库）的访问权限。
// 3.  **基于角色的访问控制 (RBAC)**: 通过为用户分配角色，简化权限管理。权限赋给角色，用户继承角色权限。
// 4.  **可管理性**: 能够方便地创建、删除、修改用户和角色，以及授予和撤销权限。
// 5.  **安全性**: 密码需要安全存储（哈希），权限检查必须严谨，防止越权操作。
// 6.  **可扩展性**: 随着系统功能的增加，权限模型可能需要扩展。
// 7.  **审计**: 记录权限变更和敏感操作，以便后续审计。
//
// 本UserManager旨在提供一个安全、高效且可扩展的用户和权限管理框架，支持RBAC。
//
// @WHAT
// 本文件定义了 `UserManager` 类及其辅助数据结构 (`Role`, `User`, `Permission` 等)。
// `UserManager` 是实现用户身份认证、角色管理和权限检查的核心服务。
// 它维护着用户、角色及其权限的内存状态，并提供持久化到文件（或SystemDatabase）的机制。
//
// 核心功能包括：
// -   用户生命周期管理 (创建、删除、修改密码、修改角色)。
// -   角色生命周期管理 (创建、删除、修改角色名)，并支持角色继承。
// -   权限授予和撤销 (针对用户或角色，对数据库或表进行操作)。
// -   细粒度权限检查 (CheckPermission)。
// -   用户认证 (AuthenticateUser)。
// -   权限审计和冲突检测 (AuditPermissionChanges, CheckPermissionConflict)。
// -   持久化机制，能够将用户、角色和权限信息保存到文件或 SystemDatabase。
//
// @HOW
// `UserManager` 的实现遵循以下原则和技术：
// 1.  **数据结构**: 使用 `std::unordered_map` 存储用户和角色信息，实现高效查找。
//     通过 `PermissionKey` 和 `PermissionKeyHash` 构建 `permission_matrix_`，实现 O(1) 的权限检查。
// 2.  **哈希存储密码**: 用户的密码不直接存储明文，而是存储其安全的哈希值，提高安全性。
// 3.  **基于角色的访问控制 (RBAC)**: 用户被分配一个角色，权限则直接或间接（通过角色继承）赋给角色。
//     `CheckPermission` 会检查用户的当前角色及其继承的所有角色是否拥有所需权限。
// 4.  **权限矩阵 (Permission Matrix)**: 通过将授权信息构建为内存中的哈希表，加速权限检查过程。
// 5.  **SystemDatabase 集成**: `UserManager` 可以与 `SystemDatabase` 交互，实现用户、角色和权限信息的持久化存储和同步。
// 6.  **线程安全**: 使用 `std::mutex` 保护所有对内部数据结构的操作，确保在并发环境下的正确性。
// 7.  **现代 C++ 实践**: 采用智能指针 (`std::shared_ptr`, `std::unique_ptr`) 管理内存，使用 `std::string` 等现代 C++ 数据结构。
// 8.  **前向声明**: 最小化头文件包含，使用前向声明减少编译依赖，加快编译速度。
//
// 未来增强点：
// -   异步权限检查。
// -   审计日志集成。
// -   性能指标收集。
//
// Key improvements:
// - Smart pointers for memory safety
// - Forward declarations to reduce compilation dependencies
// - Thread-safe operations with mutex protection
// - Modern C++ data structures and algorithms

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
// TODO(#UM-001): Enable when Clang 18+ modules are stable.
// export module sqlcc.core.user_manager;

namespace sqlcc {

// Forward declarations for SystemDatabase
class SystemDatabase;

/**
 * @brief 角色数据结构。
 * @details 定义了数据库中的角色信息，包括角色名称、创建时间以及与父子角色的继承关系。
 */
struct Role {
  std::string role_name;          ///< 角色名称，唯一标识符。
  std::string created_at;         ///< 角色创建时间戳。
  std::vector<std::string> parent_roles;  ///< 父角色列表，实现角色继承。
  std::vector<std::string> child_roles;   ///< 子角色列表。
};

/**
 * @brief 用户数据结构。
 * @details 定义了数据库中的用户信息，包括用户名、密码哈希、所属角色、当前激活角色等。
 */
struct User {
  std::string username;           ///< 用户名，唯一标识符。
  std::string password_hash;      ///< 密码的哈希值，用于安全认证。
  std::string role;               ///< 用户默认角色或主要角色。
  std::string current_role;       ///< 用户当前激活的角色（可通过SET ROLE改变）。
  bool is_active;                 ///< 用户是否处于活跃状态。
  std::string created_at;         ///< 用户创建时间戳。
};

/**
 * @brief 权限数据结构。
 * @details 定义了数据库中的权限信息，包括被授权者（用户或角色）、数据库、表以及具体权限。
 */
struct Permission {
  std::string grantee; ///< 被授权者名称（可以是用户名或角色名）。
  std::string database;///< 权限所属的数据库（可以是空字符串表示全局）。
  std::string table;   ///< 权限所属的表（可以是空字符串表示数据库级权限）。
  std::string privilege;///< 具体权限类型（例如 "SELECT", "INSERT", "ALL"）。
  bool is_role;        ///< 标识 `grantee` 是用户 (`false`) 还是角色 (`true`)。
};

/**
 * @brief 权限矩阵的键结构。
 * @details 用于在哈希表中唯一标识一个权限条目，由被授权者、数据库、表和权限类型组成。
 */
struct PermissionKey {
  std::string grantee;
  std::string database;
  std::string table;
  std::string privilege;

  /**
   * @brief 比较两个PermissionKey是否相等。
   * @param other 另一个PermissionKey实例。
   * @return 如果所有字段都相等则返回true，否则返回false。
   */
  bool operator==(const PermissionKey &other) const {
    return grantee == other.grantee && database == other.database &&
           table == other.table && privilege == other.privilege;
  }
};

/**
 * @brief 权限矩阵的哈希函数。
 * @details 为PermissionKey结构体提供哈希功能，使其可以作为`std::unordered_map`的键。
 */
struct PermissionKeyHash {
  /**
   * @brief 计算PermissionKey的哈希值。
   * @param key PermissionKey实例。
   * @return 计算出的哈希值。
   */
  std::size_t operator()(const PermissionKey &key) const {
    // 使用异或操作组合各个字符串字段的哈希值，以生成一个综合哈希值。
    return std::hash<std::string>{}(key.grantee) ^
           std::hash<std::string>{}(key.database) ^
           std::hash<std::string>{}(key.table) ^
           std::hash<std::string>{}(key.privilege);
  }
};

/**
 * @brief 权限矩阵的值结构。
 * @details 存储权限检查的布尔结果和是否基于角色。
 */
struct PermissionValue {
  bool has_permission; ///< 是否拥有该权限。
  bool is_role;        ///< 权限是否来源于角色。
};

/**
 * @brief 用户到当前激活角色的映射。
 * @details 存储每个用户当前正在使用的角色名称。
 */
using UserRoleMap = std::unordered_map<std::string, std::string>;

/**
 * @brief 用户管理器类。
 * @details 负责用户认证、角色管理、权限授予/撤销以及权限检查。
 * 实现了基于角色的访问控制（RBAC）模型，并与SystemDatabase进行集成以实现持久化。
 */
class UserManager {
public:
  // --- 角色常量定义 ---
  /** @brief 超级用户角色，拥有最高权限。 */
  static inline const std::string ROLE_SUPERUSER = "SUPERUSER";
  /** @brief 管理员角色，拥有管理权限。 */
  static inline const std::string ROLE_ADMIN = "ADMIN";
  /** @brief 普通用户角色。 */
  static inline const std::string ROLE_USER = "USER";

  // --- 权限常量定义 ---
  /** @brief 创建数据库、表等对象的权限。 */
  static inline const std::string PRIVILEGE_CREATE = "CREATE";
  /** @brief 查询（读取）数据的权限。 */
  static inline const std::string PRIVILEGE_SELECT = "SELECT";
  /** @brief 插入数据的权限。 */
  static inline const std::string PRIVILEGE_INSERT = "INSERT";
  /** @brief 更新数据的权限。 */
  static inline const std::string PRIVILEGE_UPDATE = "UPDATE";
  /** @brief 删除数据的权限。 */
  static inline const std::string PRIVILEGE_DELETE = "DELETE";
  /** @brief 删除数据库、表等对象的权限。 */
  static inline const std::string PRIVILEGE_DROP = "DROP";
  /** @brief 修改表结构的权限。 */
  static inline const std::string PRIVILEGE_ALTER = "ALTER";
  /** @brief 所有权限的通配符。 */
  static inline const std::string PRIVILEGE_ALL = "ALL";

  /**
   * @brief 构造UserManager实例。
   * @details 初始化UserManager，设置数据存储路径，并加载用户、角色和权限数据。
   * 如果文件不存在，会尝试创建默认的超级用户。
   * @param data_path 用户数据文件的存储目录。
   */
  UserManager(const std::string &data_path = "./data");
  /**
   * @brief 销毁UserManager实例。
   * @details 在销毁UserManager之前，确保所有修改都被持久化到文件。
   */
  ~UserManager();

  /**
   * @brief 设置SystemDatabase的共享指针引用。
   * @details UserManager需要SystemDatabase的引用来实现用户、角色和权限的持久化同步。
   * @param sys_db SystemDatabase的共享指针。
   */
  void SetSystemDatabase(std::shared_ptr<SystemDatabase> sys_db);
  
  /**
   * @brief 获取SystemDatabase的共享指针引用。
   * @return SystemDatabase的共享指针。
   */
  std::shared_ptr<SystemDatabase> GetSystemDatabase() const { return sys_db_; }
  // --- 用户管理方法 ---
  /**
   * @brief 创建一个新用户。
   * @param username 新用户的用户名。
   * @param password 新用户的密码（明文，内部会进行哈希）。
   * @param role 新用户所属的角色，默认为"USER"。
   * @return 创建成功返回true，否则返回false。
   */
  bool CreateUser(const std::string &username, const std::string &password,
                  const std::string &role = "USER");
  /**
   * @brief 删除一个用户。
   * @param username 待删除用户的用户名。
   * @return 删除成功返回true，否则返回false。
   */
  bool DropUser(const std::string &username);
  /**
   * @brief 修改用户密码。
   * @param username 待修改密码的用户名。
   * @param new_password 新密码（明文）。
   * @return 修改成功返回true，否则返回false。
   */
  bool AlterUserPassword(const std::string &username,
                         const std::string &new_password);
  /**
   * @brief 修改用户角色。
   * @param username 待修改角色的用户名。
   * @param new_role 新的角色名称。
   * @return 修改成功返回true，否则返回false。
   */
  bool AlterUserRole(const std::string &username, const std::string &new_role);
  /**
   * @brief 验证用户身份。
   * @param username 待验证用户的用户名。
   * @param password 待验证用户的密码（明文）。
   * @return 认证成功返回true，否则返回false。
   */
  bool AuthenticateUser(const std::string &username,
                        const std::string &password);

  // --- 角色管理方法 ---
  /**
   * @brief 创建一个新角色。
   * @param role_name 新角色的名称。
   * @return 创建成功返回true，否则返回false。
   */
  bool CreateRole(const std::string &role_name);
  /**
   * @brief 删除一个角色。
   * @param role_name 待删除角色的名称。
   * @return 删除成功返回true，否则返回false。
   */
  bool DropRole(const std::string &role_name);
  /**
   * @brief 修改角色名称。
   * @param role_name 待修改角色的当前名称。
   * @param new_role_name 角色的新名称。
   * @return 修改成功返回true，否则返回false。
   */
  bool AlterRole(const std::string &role_name,
                 const std::string &new_role_name);
  /**
   * @brief 为指定用户设置当前激活的角色。
   * @details 允许用户在会话期间切换其活动角色，影响其权限集合。
   * @param username 用户的用户名。
   * @param role_name 待激活的角色名称。
   * @return 设置成功返回true，否则返回false。
   */
  bool SetCurrentRole(const std::string &username,
                      const std::string &role_name);
  /**
   * @brief 获取指定用户当前激活的角色。
   * @param username 用户的用户名。
   * @return 用户当前激活的角色名称。如果用户不存在或未设置当前角色，返回空字符串。
   */
  std::string GetUserCurrentRole(const std::string &username) const;

  // --- 高级权限管理方法 ---
  /**
   * @brief 授予一个角色继承另一个角色的权限。
   * @param parent_role 父角色名称，其权限将被继承。
   * @param child_role 子角色名称，将继承父角色的权限。
   * @return 授予成功返回true，否则返回false。
   */
  bool GrantRoleToRole(const std::string &parent_role, const std::string &child_role);
  /**
   * @brief 撤销一个角色继承另一个角色的权限。
   * @param parent_role 父角色名称。
   * @param child_role 子角色名称。
   * @return 撤销成功返回true，否则返回false。
   */
  bool RevokeRoleFromRole(const std::string &parent_role, const std::string &child_role);
  /**
   * @brief 检查一个角色是否直接或间接继承自另一个角色。
   * @param role_name 待检查的角色名称。
   * @param inherited_role 期望继承的父角色名称。
   * @return 如果`role_name`继承自`inherited_role`则返回true，否则返回false。
   */
  bool CheckRoleInheritance(const std::string &role_name, const std::string &inherited_role) const;
  /**
   * @brief 获取指定角色的完整权限继承链。
   * @param role_name 待查询的角色名称。
   * @return 包含从自身到所有祖先角色的名称列表。
   */
  std::vector<std::string> GetRoleHierarchy(const std::string &role_name) const;
  /**
   * @brief 级联撤销权限。
   * @details 当一个角色或用户失去某个权限时，检查是否有其他依赖此权限的授权需要被撤销。
   * @param grantee 被授权者名称。
   * @param database 数据库名称。
   * @param table 表名称。
   * @param privilege 权限类型。
   * @return 撤销成功返回true，否则返回false。
   */
  bool RevokePrivilegeCascade(const std::string &grantee, const std::string &database,
                              const std::string &table, const std::string &privilege);
  /**
   * @brief 检查权限授予或撤销操作是否存在冲突。
   * @param grantee 被授权者名称。
   * @param database 数据库名称。
   * @param table 表名称。
   * @param privilege 权限类型。
   * @return 如果存在冲突返回true，否则返回false。
   */
  bool CheckPermissionConflict(const std::string &grantee, const std::string &database,
                               const std::string &table, const std::string &privilege) const;
  /**
   * @brief 审计权限变更操作。
   * @param operation 变更操作类型（如"GRANT", "REVOKE"）。
   * @param grantee 被授权者名称。
   * @param details 变更详情。
   * @return 审计记录成功返回true，否则返回false。
   */
  bool AuditPermissionChanges(const std::string &operation, const std::string &grantee,
                              const std::string &details);
  /**
   * @brief 获取指定用户在特定数据库和表上的所有有效权限。
   * @details 考虑用户直接授予的权限以及通过角色继承的权限。
   * @param username 用户名。
   * @param database 数据库名称。
   * @param table 表名称。
   * @return 包含所有有效权限字符串的向量。
   */
  std::vector<std::string> GetEffectivePermissions(const std::string &username,
                                                    const std::string &database,
                                                    const std::string &table) const;

  // --- 权限管理方法 ---
  /**
   * @brief 授予用户或角色在特定数据库或表上的权限。
   * @param grantee 被授权者名称（用户名或角色名）。
   * @param database 数据库名称。
   * @param table 表名称。
   * @param privilege 权限类型（如"SELECT", "INSERT", "ALL"）。
   * @return 授予成功返回true，否则返回false。
   */
  bool GrantPrivilege(const std::string &grantee, const std::string &database,
                      const std::string &table, const std::string &privilege);
  /**
   * @brief 撤销用户或角色在特定数据库或表上的权限。
   * @param grantee 被撤销者名称（用户名或角色名）。
   * @param database 数据库名称。
   * @param table 表名称。
   * @param privilege 权限类型。
   * @return 撤销成功返回true，否则返回false。
   */
  bool RevokePrivilege(const std::string &grantee, const std::string &database,
                       const std::string &table, const std::string &privilege);
  /**
   * @brief 检查用户是否拥有对特定数据库或表的所需权限。
   * @details 该检查会考虑直接授予用户的权限以及通过角色继承的权限。
   * @param username 用户的用户名。
   * @param database 数据库名称。
   * @param table 表名称。
   * @param required_privilege 所需的权限类型。
   * @return 如果用户拥有所需权限则返回true，否则返回false。
   */
  bool CheckPermission(const std::string &username, const std::string &database,
                       const std::string &table,
                       const std::string &required_privilege);

  // --- 查询方法 ---
  /**
   * @brief 列出所有用户。
   * @return 包含所有用户User结构体的向量。
   */
  std::vector<User> ListUsers() const;
  /**
   * @brief 列出所有角色。
   * @return 包含所有角色Role结构体的向量。
   */
  std::vector<Role> ListRoles() const;
  /**
   * @brief 列出指定用户直接拥有的权限（不包括通过角色继承的权限）。
   * @param username 用户的用户名。
   * @return 包含该用户直接拥有的Permission结构体的向量。
   */
  std::vector<Permission>
  ListUserPermissions(const std::string &username) const;
  /**
   * @brief 列出指定角色拥有的权限。
   * @param role_name 角色的名称。
   * @return 包含该角色拥有的Permission结构体的向量。
   */
  std::vector<Permission>
  ListRolePermissions(const std::string &role_name) const;

  // --- 持久化方法 ---
  /**
   * @brief 将用户、角色和权限数据持久化到文件。
   * @details 线程安全地将当前UserManager的状态（用户、角色、权限等）写入磁盘文件。
   * @return 保存成功返回true，否则返回false。
   */
  bool SaveToFile() const;
  /**
   * @brief 从文件加载用户、角色和权限数据。
   * @details 从磁盘文件读取并恢复UserManager的内部状态。
   * @return 加载成功返回true，否则返回false。
   */
  bool LoadFromFile();

  // --- 错误处理 ---
  /**
   * @brief 获取UserManager最近一次操作的错误信息。
   * @return 包含错误信息的字符串引用。
   */
  const std::string &GetLastError() const;

private:
  // --- 辅助方法 ---
  /**
   * @brief 创建一个默认的超级用户（在初始化时）。
   * @details 通常在UserManager首次初始化，且没有用户数据时调用。
   */
  void CreateDefaultSuperuser();
  /**
   * @brief 获取当前时间字符串。
   * @return 格式化后的当前时间字符串。
   */
  std::string GetCurrentTimeString();
  /**
   * @brief 为指定超级用户授予所有权限。
   * @param username 超级用户的用户名。
   */
  void GrantAllPrivilegesToSuperuser(const std::string &username);
  /**
   * @brief 移除指定用户的所有权限。
   * @param username 待移除权限的用户名。
   */
  void RemoveUserPrivileges(const std::string &username);
  /**
   * @brief 移除指定角色的所有权限。
   * @param role_name 待移除权限的角色名。
   */
  void RemoveRolePrivileges(const std::string &role_name);
  /**
   * @brief 检查角色名称是否合法且存在。
   * @param role_name 待检查的角色名称。
   * @return 如果角色合法且存在返回true，否则返回false。
   */
  bool IsValidRole(const std::string &role_name) const;
  /**
   * @brief 对密码进行哈希处理。
   * @param password 原始密码。
   * @return 哈希后的密码字符串。
   */
  std::string HashPassword(const std::string &password) const;
  /**
   * @brief 内部持久化方法，不加锁。
   * @details 由SaveToFile()调用，用于实际写入数据到文件，避免在内部方法中重复加锁。
   * @return 保存成功返回true，否则返回false。
   */
  bool SaveToFileInternal() const;
  // --- 权限矩阵相关方法 ---
  /**
   * @brief 初始化权限矩阵。
   * @details 在UserManager加载数据后构建内存中的权限矩阵，以便快速进行权限检查。
   */
  void InitializePermissionMatrix();
  /**
   * @brief 将一个权限添加到内存权限矩阵中。
   * @param permission 待添加的权限实例。
   */
  void AddPermissionToMatrix(const Permission &permission);
  /**
   * @brief 从内存权限矩阵中移除一个权限。
   * @param permission 待移除的权限实例。
   */
  void
  RemovePermissionFromMatrix(const Permission &permission);
  /**
   * @brief 在内存权限矩阵中检查用户是否拥有特定权限。
   * @details 这是底层、高效的权限查找方法，不涉及角色继承或当前激活角色的复杂逻辑。
   * @param username 用户的用户名。
   * @param database 数据库名称。
   * @param table 表名称。
   * @param required_privilege 所需的权限类型。
   * @return 如果权限存在于矩阵中则返回true，否则返回false。
   */
  bool CheckPermissionInMatrix(
      const std::string &username, const std::string &database,
      const std::string &table,
      const std::string &required_privilege) const;
  /**
   * @brief 更新用户当前激活的角色。
   * @details 仅更新内存中的映射，通常由`SetCurrentRole`调用。
   * @param username 用户的用户名。
   * @param role_name 用户当前激活的角色名称。
   */
  void UpdateUserCurrentRole(const std::string &username,
                             const std::string &role_name);
  // --- 成员变量 ---
  std::unordered_map<std::string, User> users_; ///< 用户名到用户信息的映射。
  std::unordered_map<std::string, Role> roles_; ///< 角色名到角色信息的映射。
  std::vector<Permission> permissions_;         ///< 原始权限列表（兼容性保留，实际检查使用权限矩阵）。
  mutable std::string last_error_;              ///< 记录最近一次操作的错误信息。
  std::string data_path_;                       ///< 用户数据文件的存储路径。
  std::shared_ptr<SystemDatabase> sys_db_;      ///< SystemDatabase的共享指针，用于持久化和同步权限信息。
  mutable std::mutex mutex_;                    ///< 保护UserManager内部状态的互斥锁，确保线程安全。

  // --- 权限矩阵相关成员变量 ---
  std::unordered_map<PermissionKey, PermissionValue, PermissionKeyHash>
      permission_matrix_;          ///< 内存中的权限矩阵，用于快速权限查找。
  UserRoleMap user_current_roles_; ///< 用户名到其当前激活角色的映射。
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
