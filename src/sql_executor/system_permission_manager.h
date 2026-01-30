#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../../backups/core_backup_20260121_001034/core_database_manager.h"

namespace sqlcc {

// 前向声明系统数据结构
struct SysUser;
struct SysRole;
struct SysPrivilege;

/**
 * @brief 系统权限管理器
 * 
 * 负责管理系统用户、角色和权限，包括：
 * - 用户管理（创建、删除、更新、查询）
 * - 角色管理（创建、删除、更新、查询）
 * - 权限管理（授权、撤销、查询）
 */
class SystemPermissionManager {
public:
    /**
     * @brief 构造函数
     * @param db_manager 数据库管理器指针
     */
    explicit SystemPermissionManager(std::shared_ptr<DatabaseManager> db_manager);
    
    /**
     * @brief 析构函数
     */
    ~SystemPermissionManager();
    
    // 用户管理方法
    /**
     * @brief 创建用户记录
     * @param username 用户名
     * @param password_hash 密码哈希
     * @param role 角色
     * @return 是否创建成功
     */
    bool CreateUserRecord(const std::string& username, const std::string& password_hash, const std::string& role);
    
    /**
     * @brief 删除用户记录
     * @param username 用户名
     * @return 是否删除成功
     */
    bool DropUserRecord(const std::string& username);
    
    /**
     * @brief 更新用户记录
     * @param user 用户信息
     * @return 是否更新成功
     */
    bool UpdateUserRecord(const SysUser& user);
    
    /**
     * @brief 获取用户记录
     * @param username 用户名
     * @return 用户信息
     */
    SysUser GetUserRecord(const std::string& username);
    
    /**
     * @brief 列出所有用户
     * @return 用户列表
     */
    std::vector<SysUser> ListUsers();
    
    /**
     * @brief 检查用户是否存在
     * @param username 用户名
     * @return 用户是否存在
     */
    bool UserExists(const std::string& username);
    
    // 角色管理方法
    /**
     * @brief 创建角色记录
     * @param role_name 角色名
     * @return 是否创建成功
     */
    bool CreateRoleRecord(const std::string& role_name);
    
    /**
     * @brief 删除角色记录
     * @param role_name 角色名
     * @return 是否删除成功
     */
    bool DropRoleRecord(const std::string& role_name);
    
    /**
     * @brief 获取角色记录
     * @param role_name 角色名
     * @return 角色信息
     */
    SysRole GetRoleRecord(const std::string& role_name);
    
    /**
     * @brief 列出所有角色
     * @return 角色列表
     */
    std::vector<SysRole> ListRoles();
    
    /**
     * @brief 检查角色是否存在
     * @param role_name 角色名
     * @return 角色是否存在
     */
    bool RoleExists(const std::string& role_name);
    
    // 权限管理方法
    /**
     * @brief 授权记录
     * @param grantee 接收者
     * @param object_type 对象类型
     * @param object_name 对象名
     * @param privilege_type 权限类型
     * @param grantor 授权者
     * @return 是否授权成功
     */
    bool GrantPrivilegeRecord(const std::string& grantee, const std::string& object_type, 
                             const std::string& object_name, const std::string& privilege_type,
                             const std::string& grantor);
    
    /**
     * @brief 撤销权限记录
     * @param grantee 接收者
     * @param object_type 对象类型
     * @param object_name 对象名
     * @param privilege_type 权限类型
     * @return 是否撤销成功
     */
    bool RevokePrivilegeRecord(const std::string& grantee, const std::string& object_type, 
                              const std::string& object_name, const std::string& privilege_type);
    
    /**
     * @brief 获取对象权限
     * @param object_type 对象类型
     * @param object_name 对象名
     * @return 权限列表
     */
    std::vector<SysPrivilege> GetObjectPrivileges(const std::string& object_type, const std::string& object_name);
    
    /**
     * @brief 获取用户权限
     * @param username 用户名
     * @return 权限列表
     */
    std::vector<SysPrivilege> GetUserPrivileges(const std::string& username);
    
    /**
     * @brief 检查用户是否有特定权限
     * @param username 用户名
     * @param object_type 对象类型
     * @param object_name 对象名
     * @param privilege_type 权限类型
     * @return 是否有权限
     */
    bool HasPrivilege(const std::string& username, const std::string& object_type, 
                     const std::string& object_name, const std::string& privilege_type);
    
    /**
     * @brief 获取最后一次错误信息
     * @return 错误信息
     */
    std::string GetLastError() const;

private:
    /**
     * @brief 执行SQL语句
     * @param sql SQL语句
     * @return 是否执行成功
     */
    bool ExecuteSQL(const std::string& sql);
    
    /**
     * @brief 执行查询并返回结果
     * @param sql SQL查询语句
     * @return 查询结果
     */
    std::vector<std::vector<std::string>> ExecuteSelectQuery(const std::string& sql);
    
    /**
     * @brief 设置错误信息
     * @param error 错误信息
     */
    void SetError(const std::string& error);
    
    std::shared_ptr<DatabaseManager> db_manager_;  // 数据库管理器
    std::string last_error_;                       // 最后一次错误信息
};

} // namespace sqlcc