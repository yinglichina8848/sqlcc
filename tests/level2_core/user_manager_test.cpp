#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <algorithm>

// 模拟UserManager类的简化版本
namespace sqlcc {

// 模拟SystemDatabase类
class SystemDatabase {
public:
    std::string name = "system_db";
    std::unordered_map<std::string, std::string> user_records;
};

// 角色数据结构
struct Role {
    std::string role_name;
    std::string created_at;
    std::vector<std::string> parent_roles;
    std::vector<std::string> child_roles;
};

// 简化的UserManager类
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
    ~UserManager() = default;

    // 设置SystemDatabase引用
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

    // 查询方法
    bool UserExists(const std::string &username) const;
    bool RoleExists(const std::string &role_name) const;
    std::vector<std::string> GetAllUsers() const;
    std::vector<std::string> GetAllRoles() const;

private:
    std::string data_path_;
    std::shared_ptr<SystemDatabase> sys_db_;
    std::unordered_map<std::string, std::string> users_;  // username -> password
    std::unordered_map<std::string, Role> roles_;        // role_name -> Role
    std::unordered_map<std::string, std::string> user_roles_;  // username -> role_name
    mutable std::mutex mutex_;
    
    // 简单的密码验证函数
    bool ValidatePassword(const std::string& password) const {
        return password.length() >= 6;  // 简化规则：至少6个字符
    }
};

UserManager::UserManager(const std::string &data_path) 
    : data_path_(data_path) {
    // 创建默认角色
    roles_[ROLE_SUPERUSER] = {ROLE_SUPERUSER, "2023-01-01", {}, {}};
    roles_[ROLE_ADMIN] = {ROLE_ADMIN, "2023-01-01", {}, {ROLE_USER}};
    roles_[ROLE_USER] = {ROLE_USER, "2023-01-01", {ROLE_ADMIN}, {}};
    
    // 创建system database
    sys_db_ = std::make_shared<SystemDatabase>();
}

void UserManager::SetSystemDatabase(std::shared_ptr<SystemDatabase> sys_db) {
    std::lock_guard<std::mutex> lock(mutex_);
    sys_db_ = sys_db;
}

bool UserManager::CreateUser(const std::string &username, const std::string &password,
                             const std::string &role) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (username.empty() || password.empty()) {
        return false;
    }
    
    if (users_.find(username) != users_.end()) {
        return false;  // 用户已存在
    }
    
    if (!ValidatePassword(password)) {
        return false;  // 密码不符合要求
    }
    
    if (roles_.find(role) == roles_.end()) {
        return false;  // 角色不存在
    }
    
    users_[username] = password;
    user_roles_[username] = role;
    
    if (sys_db_) {
        sys_db_->user_records[username] = role;
    }
    
    return true;
}

bool UserManager::DropUser(const std::string &username) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (users_.find(username) == users_.end()) {
        return false;  // 用户不存在
    }
    
    users_.erase(username);
    user_roles_.erase(username);
    
    if (sys_db_) {
        sys_db_->user_records.erase(username);
    }
    
    return true;
}

bool UserManager::AlterUserPassword(const std::string &username,
                                   const std::string &new_password) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (users_.find(username) == users_.end()) {
        return false;  // 用户不存在
    }
    
    if (!ValidatePassword(new_password)) {
        return false;  // 密码不符合要求
    }
    
    users_[username] = new_password;
    return true;
}

bool UserManager::AlterUserRole(const std::string &username, const std::string &new_role) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (users_.find(username) == users_.end()) {
        return false;  // 用户不存在
    }
    
    if (roles_.find(new_role) == roles_.end()) {
        return false;  // 角色不存在
    }
    
    user_roles_[username] = new_role;
    
    if (sys_db_) {
        sys_db_->user_records[username] = new_role;
    }
    
    return true;
}

bool UserManager::AuthenticateUser(const std::string &username,
                                   const std::string &password) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = users_.find(username);
    if (it == users_.end()) {
        return false;  // 用户不存在
    }
    
    return it->second == password;
}

bool UserManager::CreateRole(const std::string &role_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (role_name.empty()) {
        return false;
    }
    
    if (roles_.find(role_name) != roles_.end()) {
        return false;  // 角色已存在
    }
    
    roles_[role_name] = {role_name, "2023-01-01", {}, {}};
    return true;
}

bool UserManager::DropRole(const std::string &role_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (roles_.find(role_name) == roles_.end()) {
        return false;  // 角色不存在
    }
    
    // 检查是否有用户使用此角色
    for (const auto& user_role : user_roles_) {
        if (user_role.second == role_name) {
            return false;  // 有用户正在使用此角色
        }
    }
    
    roles_.erase(role_name);
    return true;
}

bool UserManager::AlterRole(const std::string &role_name,
                            const std::string &new_role_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (roles_.find(role_name) == roles_.end()) {
        return false;  // 原角色不存在
    }
    
    if (roles_.find(new_role_name) != roles_.end()) {
        return false;  // 新角色名已存在
    }
    
    Role role = roles_[role_name];
    role.role_name = new_role_name;
    roles_[new_role_name] = role;
    roles_.erase(role_name);
    
    // 更新用户角色引用
    for (auto& user_role : user_roles_) {
        if (user_role.second == role_name) {
            user_role.second = new_role_name;
        }
    }
    
    return true;
}

bool UserManager::SetCurrentRole(const std::string &username,
                                 const std::string &role_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (users_.find(username) == users_.end()) {
        return false;  // 用户不存在
    }
    
    if (roles_.find(role_name) == roles_.end()) {
        return false;  // 角色不存在
    }
    
    user_roles_[username] = role_name;
    return true;
}

std::string UserManager::GetUserCurrentRole(const std::string &username) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = user_roles_.find(username);
    if (it != user_roles_.end()) {
        return it->second;
    }
    
    return "";
}

bool UserManager::GrantRoleToRole(const std::string &parent_role, const std::string &child_role) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (roles_.find(parent_role) == roles_.end() || roles_.find(child_role) == roles_.end()) {
        return false;  // 角色不存在
    }
    
    // 避免循环继承
    if (parent_role == child_role) {
        return false;
    }
    
    // 检查是否已存在继承关系
    const auto& parent = roles_[parent_role];
    if (std::find(parent.child_roles.begin(), parent.child_roles.end(), child_role) != parent.child_roles.end()) {
        return false;  // 已存在继承关系
    }
    
    // 添加继承关系
    roles_[parent_role].child_roles.push_back(child_role);
    roles_[child_role].parent_roles.push_back(parent_role);
    return true;
}

bool UserManager::RevokeRoleFromRole(const std::string &parent_role, const std::string &child_role) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (roles_.find(parent_role) == roles_.end() || roles_.find(child_role) == roles_.end()) {
        return false;  // 角色不存在
    }
    
    // 检查是否存在继承关系
    const auto& parent = roles_[parent_role];
    if (std::find(parent.child_roles.begin(), parent.child_roles.end(), child_role) == parent.child_roles.end()) {
        return false;  // 继承关系不存在
    }
    
    // 移除继承关系
    auto& parent_role_ref = roles_[parent_role];
    auto child_it = std::find(parent_role_ref.child_roles.begin(), parent_role_ref.child_roles.end(), child_role);
    if (child_it != parent_role_ref.child_roles.end()) {
        parent_role_ref.child_roles.erase(child_it);
    }
    
    auto& child_role_ref = roles_[child_role];
    auto parent_it = std::find(child_role_ref.parent_roles.begin(), child_role_ref.parent_roles.end(), parent_role);
    if (parent_it != child_role_ref.parent_roles.end()) {
        child_role_ref.parent_roles.erase(parent_it);
    }
    
    return true;
}

bool UserManager::UserExists(const std::string &username) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return users_.find(username) != users_.end();
}

bool UserManager::RoleExists(const std::string &role_name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return roles_.find(role_name) != roles_.end();
}

std::vector<std::string> UserManager::GetAllUsers() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> users;
    for (const auto& user : users_) {
        users.push_back(user.first);
    }
    return users;
}

std::vector<std::string> UserManager::GetAllRoles() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> roles;
    for (const auto& role : roles_) {
        roles.push_back(role.first);
    }
    return roles;
}

} // namespace sqlcc

namespace sqlcc {
namespace test {

class UserManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        user_manager = std::make_unique<UserManager>("./test_data");
        sys_db = std::make_shared<SystemDatabase>();
        user_manager->SetSystemDatabase(sys_db);
    }
    
    void TearDown() override {
        user_manager.reset();
        sys_db.reset();
    }
    
    std::unique_ptr<UserManager> user_manager;
    std::shared_ptr<SystemDatabase> sys_db;
};

// Test user management
TEST_F(UserManagerTest, CreateUser) {
    EXPECT_TRUE(user_manager->CreateUser("testuser", "password123"));
    EXPECT_TRUE(user_manager->UserExists("testuser"));
    EXPECT_FALSE(user_manager->CreateUser("testuser", "password"));  // 重复创建
}

TEST_F(UserManagerTest, CreateUserWithInvalidPassword) {
    EXPECT_FALSE(user_manager->CreateUser("testuser", "123"));  // 密码太短
    EXPECT_FALSE(user_manager->UserExists("testuser"));
}

TEST_F(UserManagerTest, CreateUserWithEmptyFields) {
    EXPECT_FALSE(user_manager->CreateUser("", "password"));  // 空用户名
    EXPECT_FALSE(user_manager->CreateUser("testuser", ""));  // 空密码
}

TEST_F(UserManagerTest, DropUser) {
    user_manager->CreateUser("testuser", "password123");
    EXPECT_TRUE(user_manager->DropUser("testuser"));
    EXPECT_FALSE(user_manager->UserExists("testuser"));
    EXPECT_FALSE(user_manager->DropUser("nonexistent"));  // 用户不存在
}

TEST_F(UserManagerTest, AuthenticateUser) {
    user_manager->CreateUser("testuser", "password123");
    EXPECT_TRUE(user_manager->AuthenticateUser("testuser", "password123"));
    EXPECT_FALSE(user_manager->AuthenticateUser("testuser", "wrongpassword"));
    EXPECT_FALSE(user_manager->AuthenticateUser("nonexistent", "password"));
}

TEST_F(UserManagerTest, AlterUserPassword) {
    user_manager->CreateUser("testuser", "password123");
    EXPECT_TRUE(user_manager->AlterUserPassword("testuser", "newpassword"));
    EXPECT_TRUE(user_manager->AuthenticateUser("testuser", "newpassword"));
    EXPECT_FALSE(user_manager->AuthenticateUser("testuser", "password123"));
    
    EXPECT_FALSE(user_manager->AlterUserPassword("nonexistent", "newpassword"));
    EXPECT_FALSE(user_manager->AlterUserPassword("testuser", "123"));  // 密码太短
}

TEST_F(UserManagerTest, AlterUserRole) {
    user_manager->CreateUser("testuser", "password123", "USER");
    EXPECT_EQ(user_manager->GetUserCurrentRole("testuser"), "USER");
    
    EXPECT_TRUE(user_manager->AlterUserRole("testuser", "ADMIN"));
    EXPECT_EQ(user_manager->GetUserCurrentRole("testuser"), "ADMIN");
    
    EXPECT_FALSE(user_manager->AlterUserRole("nonexistent", "USER"));
    EXPECT_FALSE(user_manager->AlterUserRole("testuser", "NONEXISTENT_ROLE"));
}

// Test role management
TEST_F(UserManagerTest, CreateRole) {
    EXPECT_TRUE(user_manager->CreateRole("TEST_ROLE"));
    EXPECT_TRUE(user_manager->RoleExists("TEST_ROLE"));
    EXPECT_FALSE(user_manager->CreateRole("TEST_ROLE"));  // 重复创建
    EXPECT_FALSE(user_manager->CreateRole(""));  // 空角色名
}

TEST_F(UserManagerTest, DropRole) {
    user_manager->CreateRole("TEST_ROLE");
    EXPECT_TRUE(user_manager->DropRole("TEST_ROLE"));
    EXPECT_FALSE(user_manager->RoleExists("TEST_ROLE"));
    EXPECT_FALSE(user_manager->DropRole("NONEXISTENT_ROLE"));
}

TEST_F(UserManagerTest, DropRoleWithUsers) {
    user_manager->CreateUser("testuser", "password123", "USER");
    EXPECT_FALSE(user_manager->DropRole("USER"));  // 有用户正在使用
}

TEST_F(UserManagerTest, AlterRole) {
    user_manager->CreateRole("OLD_ROLE");
    EXPECT_TRUE(user_manager->AlterRole("OLD_ROLE", "NEW_ROLE"));
    EXPECT_TRUE(user_manager->RoleExists("NEW_ROLE"));
    EXPECT_FALSE(user_manager->RoleExists("OLD_ROLE"));
    
    EXPECT_FALSE(user_manager->AlterRole("NONEXISTENT", "NEW_ROLE"));
    EXPECT_FALSE(user_manager->AlterRole("NEW_ROLE", "NEW_ROLE"));  // 重名
}

// Test role inheritance
TEST_F(UserManagerTest, GrantRoleToRole) {
    user_manager->CreateRole("PARENT");
    user_manager->CreateRole("CHILD");
    
    // 第一次添加应该成功
    EXPECT_TRUE(user_manager->GrantRoleToRole("PARENT", "CHILD"));
    
    // 第二次添加应该失败（重复添加）
    EXPECT_FALSE(user_manager->GrantRoleToRole("PARENT", "CHILD"));
    EXPECT_FALSE(user_manager->GrantRoleToRole("PARENT", "PARENT"));  // 自继承
    EXPECT_FALSE(user_manager->GrantRoleToRole("NONEXISTENT", "CHILD"));
    EXPECT_FALSE(user_manager->GrantRoleToRole("PARENT", "NONEXISTENT"));
}

TEST_F(UserManagerTest, RevokeRoleFromRole) {
    user_manager->CreateRole("PARENT");
    user_manager->CreateRole("CHILD");
    
    // 先添加关系
    EXPECT_TRUE(user_manager->GrantRoleToRole("PARENT", "CHILD"));
    
    // 撤销关系应该成功
    EXPECT_TRUE(user_manager->RevokeRoleFromRole("PARENT", "CHILD"));
    
    // 关系不存在时应该失败
    EXPECT_FALSE(user_manager->RevokeRoleFromRole("PARENT", "CHILD"));
    EXPECT_FALSE(user_manager->RevokeRoleFromRole("NONEXISTENT", "CHILD"));
}

// Test default roles
TEST_F(UserManagerTest, DefaultRolesExist) {
    EXPECT_TRUE(user_manager->RoleExists(UserManager::ROLE_SUPERUSER));
    EXPECT_TRUE(user_manager->RoleExists(UserManager::ROLE_ADMIN));
    EXPECT_TRUE(user_manager->RoleExists(UserManager::ROLE_USER));
}

TEST_F(UserManagerTest, DefaultRolesConstants) {
    EXPECT_EQ(UserManager::ROLE_SUPERUSER, "SUPERUSER");
    EXPECT_EQ(UserManager::ROLE_ADMIN, "ADMIN");
    EXPECT_EQ(UserManager::ROLE_USER, "USER");
}

// Test query methods
TEST_F(UserManagerTest, GetAllUsers) {
    EXPECT_TRUE(user_manager->GetAllUsers().empty());  // 初始状态应为空
    
    user_manager->CreateUser("user1", "password1");
    user_manager->CreateUser("user2", "password2");
    
    auto users = user_manager->GetAllUsers();
    EXPECT_EQ(users.size(), 2);
    EXPECT_NE(std::find(users.begin(), users.end(), "user1"), users.end());
    EXPECT_NE(std::find(users.begin(), users.end(), "user2"), users.end());
}

TEST_F(UserManagerTest, GetAllRoles) {
    auto roles = user_manager->GetAllRoles();
    EXPECT_GE(roles.size(), 3);  // 至少包含3个默认角色
    
    EXPECT_NE(std::find(roles.begin(), roles.end(), UserManager::ROLE_SUPERUSER), roles.end());
    EXPECT_NE(std::find(roles.begin(), roles.end(), UserManager::ROLE_ADMIN), roles.end());
    EXPECT_NE(std::find(roles.begin(), roles.end(), UserManager::ROLE_USER), roles.end());
}

// Test SystemDatabase integration
TEST_F(UserManagerTest, SystemDatabaseIntegration) {
    EXPECT_EQ(user_manager->GetSystemDatabase(), sys_db);
    
    user_manager->CreateUser("testuser", "password123", "USER");
    EXPECT_EQ(sys_db->user_records["testuser"], "USER");
    
    user_manager->DropUser("testuser");
    EXPECT_TRUE(sys_db->user_records.find("testuser") == sys_db->user_records.end());
}

// Test Constants
TEST_F(UserManagerTest, PrivilegeConstants) {
    EXPECT_EQ(UserManager::PRIVILEGE_CREATE, "CREATE");
    EXPECT_EQ(UserManager::PRIVILEGE_SELECT, "SELECT");
    EXPECT_EQ(UserManager::PRIVILEGE_INSERT, "INSERT");
    EXPECT_EQ(UserManager::PRIVILEGE_UPDATE, "UPDATE");
    EXPECT_EQ(UserManager::PRIVILEGE_DELETE, "DELETE");
    EXPECT_EQ(UserManager::PRIVILEGE_DROP, "DROP");
    EXPECT_EQ(UserManager::PRIVILEGE_ALTER, "ALTER");
    EXPECT_EQ(UserManager::PRIVILEGE_ALL, "ALL");
}

} // namespace test
} // namespace sqlcc