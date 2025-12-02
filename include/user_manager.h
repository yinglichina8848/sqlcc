#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <chrono>

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
    
    UserManager(const std::string& data_path = "./data");
    ~UserManager();
    
    // 设置SystemDatabase引用（用于权限同步）
    void SetSystemDatabase(SystemDatabase* sys_db);
    
    // 用户管理方法
    bool CreateUser(const std::string& username, const std::string& password, const std::string& role = "USER");
    bool DropUser(const std::string& username);
    bool AlterUserPassword(const std::string& username, const std::string& new_password);
    bool AlterUserRole(const std::string& username, const std::string& new_role);
    bool AuthenticateUser(const std::string& username, const std::string& password);
    
    // 角色管理方法
    bool CreateRole(const std::string& role_name);
    bool DropRole(const std::string& role_name);
    bool AlterRole(const std::string& role_name, const std::string& new_role_name);
    bool SetCurrentRole(const std::string& username, const std::string& role_name);
    std::string GetUserCurrentRole(const std::string& username);
    
    // 权限管理方法
    bool GrantPrivilege(const std::string& grantee, const std::string& database,
                       const std::string& table, const std::string& privilege);
    bool RevokePrivilege(const std::string& grantee, const std::string& database,
                        const std::string& table, const std::string& privilege);
    bool CheckPermission(const std::string& username, const std::string& database,
                        const std::string& table, const std::string& required_privilege);
    
    // 查询方法
    std::vector<User> ListUsers() const;
    std::vector<Role> ListRoles() const;
    std::vector<Permission> ListUserPermissions(const std::string& username) const;
    std::vector<Permission> ListRolePermissions(const std::string& role_name) const;
    
    // 持久化方法
    bool SaveToFile() const;
    bool LoadFromFile();
    
    // 错误处理
    const std::string& GetLastError() const;
    
private:
    // 辅助方法
    void CreateDefaultSuperuser();
    std::string GetCurrentTimeString();
    void GrantAllPrivilegesToSuperuser(const std::string& username);
    void RemoveUserPrivileges(const std::string& username);
    void RemoveRolePrivileges(const std::string& role_name);
    bool IsValidRole(const std::string& role_name) const;
    std::string HashPassword(const std::string& password) const;
    bool SaveToFileInternal() const;  // 内部保存方法，不加锁
    
    // 成员变量
    std::unordered_map<std::string, User> users_; // 用户名 -> 用户信息
    std::unordered_map<std::string, Role> roles_; // 角色名 -> 角色信息
    std::vector<Permission> permissions_;         // 权限列表
    mutable std::string last_error_;              // 最后错误信息
    std::string data_path_;                       // 数据存储路径
    SystemDatabase* sys_db_;                      // SystemDatabase引用（用于权限同步）
    mutable std::mutex mutex_;                    // 线程安全互斥锁
};

} // namespace sqlcc

#endif // USER_MANAGER_H