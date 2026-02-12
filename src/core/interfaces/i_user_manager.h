/**
 * @file i_user_manager.h
 * @brief SQLCC用户管理器接口
 * @author SQLCC Team
 * @date 2026-02-11
 * @copyright Copyright (c) 2026
 *
 * 文件用途说明：
 * 本文件定义了用户管理器的抽象接口，用于解耦权限管理模块。
 */

#pragma once

#include <string>
#include <vector>
#include <memory>

namespace sqlcc {
namespace core {
namespace interfaces {

/**
 * WHY: 为什么需要用户管理器接口？
 *
 * 1. **安全解耦**: 权限验证不应该依赖具体的 UserManager 实现
 * 2. **多认证方式**: 未来可能支持多种认证方式（LDAP、OAuth等）
 * 3. **可测试性**: 便于 Mock 用户管理器进行单元测试
 * 4. **权限矩阵**: 支持不同的权限管理策略
 *
 * WHAT: 用户管理器的核心能力
 *
 * - 用户管理：创建、删除、认证用户
 * - 角色管理：创建、删除、继承角色
 * - 权限管理：授权、撤销、检查权限
 * - RBAC 支持：基于角色的访问控制
 */

// 预定义权限常量
namespace privileges {
    constexpr const char* kCreate = "CREATE";
    constexpr const char* kSelect = "SELECT";
    constexpr const char* kInsert = "INSERT";
    constexpr const char* kUpdate = "UPDATE";
    constexpr const char* kDelete = "DELETE";
    constexpr const char* kDrop = "DROP";
    constexpr const char* kAlter = "ALTER";
    constexpr const char* kAll = "ALL";
} // namespace privileges

// 预定义角色常量
namespace roles {
    constexpr const char* kSuperuser = "SUPERUSER";
    constexpr const char* kAdmin = "ADMIN";
    constexpr const char* kUser = "USER";
} // namespace roles

/**
 * @brief 用户数据结构
 */
struct UserInfo {
    std::string username;
    std::string role;
    bool is_active;
    std::string created_at;
};

/**
 * @brief 角色数据结构
 */
struct RoleInfo {
    std::string role_name;
    std::vector<std::string> parent_roles;
    std::vector<std::string> permissions;
};

/**
 * @brief 权限条目
 */
struct PermissionEntry {
    std::string grantee;    ///< 授权对象（用户或角色）
    std::string database;   ///< 数据库名
    std::string table;      ///< 表名
    std::string privilege;  ///< 权限类型
    bool is_role;          ///< 是否为角色授权
};

/**
 * @brief 用户管理器接口
 */
class IUserManager {
public:
    virtual ~IUserManager() = default;
    
    // ========================================================================
    // 用户管理
    // ========================================================================
    
    /**
     * @brief 创建用户
     * @param username 用户名
     * @param password 密码
     * @param role 默认角色，默认为 "USER"
     * @return true 创建成功，false 失败
     */
    virtual bool CreateUser(const std::string& username,
                          const std::string& password,
                          const std::string& role = roles::kUser) = 0;
    
    /**
     * @brief 删除用户
     * @param username 用户名
     * @return true 删除成功，false 失败
     */
    virtual bool DropUser(const std::string& username) = 0;
    
    /**
     * @brief 修改用户密码
     * @param username 用户名
     * @param new_password 新密码
     * @return true 修改成功，false 失败
     */
    virtual bool AlterUserPassword(const std::string& username,
                                  const std::string& new_password) = 0;
    
    /**
     * @brief 修改用户角色
     * @param username 用户名
     * @param new_role 新角色
     * @return true 修改成功，false 失败
     */
    virtual bool AlterUserRole(const std::string& username,
                              const std::string& new_role) = 0;
    
    /**
     * @brief 认证用户
     * @param username 用户名
     * @param password 密码
     * @return true 认证成功，false 失败
     */
    virtual bool AuthenticateUser(const std::string& username,
                                 const std::string& password) = 0;
    
    /**
     * @brief 检查用户是否存在
     * @param username 用户名
     * @return true 存在，false 不存在
     */
    virtual bool UserExists(const std::string& username) const = 0;
    
    /**
     * @brief 列出所有用户
     * @return 用户信息列表
     */
    virtual std::vector<UserInfo> ListUsers() const = 0;
    
    // ========================================================================
    // 角色管理
    // ========================================================================
    
    /**
     * @brief 创建角色
     * @param role_name 角色名
     * @return true 创建成功，false 失败
     */
    virtual bool CreateRole(const std::string& role_name) = 0;
    
    /**
     * @brief 删除角色
     * @param role_name 角色名
     * @return true 删除成功，false 失败
     */
    virtual bool DropRole(const std::string& role_name) = 0;
    
    /**
     * @brief 修改角色名
     * @param role_name 原角色名
     * @param new_role_name 新角色名
     * @return true 修改成功，false 失败
     */
    virtual bool AlterRole(const std::string& role_name,
                          const std::string& new_role_name) = 0;
    
    /**
     * @brief 授予角色给角色（角色继承）
     * @param parent_role 父角色
     * @param child_role 子角色
     * @return true 成功，false 失败
     */
    virtual bool GrantRoleToRole(const std::string& parent_role,
                                const std::string& child_role) = 0;
    
    /**
     * @brief 撤销角色继承
     * @param parent_role 父角色
     * @param child_role 子角色
     * @return true 成功，false 失败
     */
    virtual bool RevokeRoleFromRole(const std::string& parent_role,
                                   const std::string& child_role) = 0;
    
    /**
     * @brief 列出所有角色
     * @return 角色信息列表
     */
    virtual std::vector<RoleInfo> ListRoles() const = 0;
    
    /**
     * @brief 检查角色继承关系
     * @param role_name 角色名
     * @param inherited_role 被继承的角色名
     * @return true 存在继承关系，false 不存在
     */
    virtual bool CheckRoleInheritance(const std::string& role_name,
                                     const std::string& inherited_role) const = 0;
    
    // ========================================================================
    // 权限管理
    // ========================================================================
    
    /**
     * @brief 授予权限
     * @param grantee 授权对象（用户或角色）
     * @param database 数据库名，* 表示所有
     * @param table 表名，* 表示所有
     * @param privilege 权限类型
     * @return true 成功，false 失败
     */
    virtual bool GrantPrivilege(const std::string& grantee,
                               const std::string& database,
                               const std::string& table,
                               const std::string& privilege) = 0;
    
    /**
     * @brief 撤销权限
     * @param grantee 授权对象
     * @param database 数据库名
     * @param table 表名
     * @param privilege 权限类型
     * @return true 成功，false 失败
     */
    virtual bool RevokePrivilege(const std::string& grantee,
                                const std::string& database,
                                const std::string& table,
                                const std::string& privilege) = 0;
    
    /**
     * @brief 检查权限
     * @param username 用户名
     * @param database 数据库名
     * @param table 表名
     * @param privilege 权限类型
     * @return true 有权限，false 无权限
     */
    virtual bool CheckPermission(const std::string& username,
                                const std::string& database,
                                const std::string& table,
                                const std::string& privilege) const = 0;
    
    /**
     * @brief 获取用户的有效权限列表
     * @param username 用户名
     * @param database 数据库名
     * @param table 表名
     * @return 权限列表
     */
    virtual std::vector<std::string> GetEffectivePermissions(const std::string& username,
                                                           const std::string& database,
                                                           const std::string& table) const = 0;
    
    /**
     * @brief 列出用户的权限
     * @param username 用户名
     * @return 权限条目列表
     */
    virtual std::vector<PermissionEntry> ListUserPermissions(const std::string& username) const = 0;
    
    /**
     * @brief 列出角色的权限
     * @param role_name 角色名
     * @return 权限条目列表
     */
    virtual std::vector<PermissionEntry> ListRolePermissions(const std::string& role_name) const = 0;
    
    // ========================================================================
    // 持久化
    // ========================================================================
    
    /**
     * @brief 保存到文件
     * @return true 成功，false 失败
     */
    virtual bool SaveToFile() const = 0;
    
    /**
     * @brief 从文件加载
     * @return true 成功，false 失败
     */
    virtual bool LoadFromFile() = 0;
    
    /**
     * @brief 获取最后错误信息
     * @return 错误信息
     */
    virtual std::string GetLastError() const = 0;
};

} // namespace interfaces
} // namespace core
} // namespace sqlcc
