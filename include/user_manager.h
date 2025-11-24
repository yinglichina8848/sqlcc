#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <chrono>

namespace sqlcc {

// 用户数据结构
struct User {
    std::string username;
    std::string password_hash;
    std::string role;
    bool is_active;
    std::string created_at;
};

// 权限数据结构
struct Permission {
    std::string username;
    std::string database;
    std::string table;
    std::string privilege;
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
    
    UserManager();
    ~UserManager();
    
    // 用户管理方法
    bool CreateUser(const std::string& username, const std::string& password, const std::string& role);
    bool DropUser(const std::string& username);
    bool AlterUserPassword(const std::string& username, const std::string& new_password);
    bool AuthenticateUser(const std::string& username, const std::string& password);
    
    // 权限管理方法
    bool GrantPrivilege(const std::string& username, const std::string& database,
                       const std::string& table, const std::string& privilege);
    bool RevokePrivilege(const std::string& username, const std::string& database,
                        const std::string& table, const std::string& privilege);
    bool CheckPermission(const std::string& username, const std::string& database,
                        const std::string& table, const std::string& required_privilege);
    
    // 查询方法
    std::vector<User> ListUsers() const;
    std::vector<Permission> ListUserPermissions(const std::string& username) const;
    
    // 错误处理
    const std::string& GetLastError() const;
    
private:
    // 辅助方法
    void CreateDefaultSuperuser();
    std::string GetCurrentTimeString();
    void GrantAllPrivilegesToSuperuser(const std::string& username);
    void RemoveUserPrivileges(const std::string& username);
    
    // 成员变量
    std::unordered_map<std::string, User> users_; // 用户名 -> 用户信息
    std::vector<Permission> permissions_;         // 权限列表
    mutable std::mutex mutex_;                    // 互斥锁，保护并发访问
    std::string last_error_;                      // 最后一次错误信息
};

} // namespace sqlcc

#endif // USER_MANAGER_H
