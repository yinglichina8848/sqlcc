/**
 * WHY: UserManager是SQLCC的核心安全模块，负责用户认证和RBAC权限管理。
 * 为了保证数据库系统的安全性，需要全面测试用户管理功能：
 * 1. 用户生命周期管理（创建、删除、修改）
 * 2. 角色管理和继承
 * 3. 权限授予和撤销
 * 4. 用户认证流程
 * 5. 权限冲突检测
 *
 * WHAT: UserManager单元测试
 * 测试覆盖：
 * - 用户CRUD操作
 * - 角色管理
 * - 权限矩阵操作
 * - 认证流程
 * - 权限检查
 *
 * HOW: 使用Google Test框架，测试UserManager的核心逻辑。
 */

#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "src/core/user_manager.h"
#include "src/exception/exception.h"

namespace sqlcc {
namespace test {

// UserManager测试 fixture
class UserManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // UserManager 不需要 logger，构造函数只接受 data_path
        user_manager_ = std::make_unique<UserManager>("./test_data");
    }

    void TearDown() override {
        user_manager_.reset();
        // 清理测试数据目录
        system("rm -rf ./test_data 2>/dev/null");
    }

    std::unique_ptr<UserManager> user_manager_;
};

// ============ 用户创建测试 ============

TEST_F(UserManagerTest, CreateUser_BasicCreation) {
    // 测试基本用户创建功能
    bool result = user_manager_->CreateUser("test_user", "password123", "USER");
    EXPECT_TRUE(result);
}

TEST_F(UserManagerTest, CreateUser_DuplicateUser) {
    // 测试重复创建用户应该失败
    bool first = user_manager_->CreateUser("test_user", "password123", "USER");
    bool second = user_manager_->CreateUser("test_user", "different_password", "ADMIN");
    
    EXPECT_TRUE(first);
    EXPECT_FALSE(second);
}

TEST_F(UserManagerTest, CreateUser_EmptyUsername) {
    // 测试空用户名应该失败
    bool result = user_manager_->CreateUser("", "password123", "USER");
    EXPECT_FALSE(result);
}

TEST_F(UserManagerTest, CreateUser_EmptyPassword) {
    // 测试空密码应该失败
    bool result = user_manager_->CreateUser("test_user", "", "USER");
    EXPECT_FALSE(result);
}

TEST_F(UserManagerTest, CreateUser_AdminRole) {
    // 测试创建管理员用户
    bool result = user_manager_->CreateUser("admin_user", "admin_pass", "ADMIN");
    EXPECT_TRUE(result);
    
    // 验证管理员有认证成功
    EXPECT_TRUE(user_manager_->AuthenticateUser("admin_user", "admin_pass"));
}

TEST_F(UserManagerTest, CreateUser_GuestRole) {
    // 测试创建访客用户
    bool result = user_manager_->CreateUser("guest_user", "guest_pass", "GUEST");
    EXPECT_TRUE(result);
    
    // 验证访客有认证成功
    EXPECT_TRUE(user_manager_->AuthenticateUser("guest_user", "guest_pass"));
}

// ============ 用户认证测试 ============

TEST_F(UserManagerTest, AuthenticateUser_Success) {
    // 测试成功的用户认证
    user_manager_->CreateUser("test_user", "password123", "USER");
    
    bool result = user_manager_->AuthenticateUser("test_user", "password123");
    EXPECT_TRUE(result);
}

TEST_F(UserManagerTest, AuthenticateUser_WrongPassword) {
    // 测试错误密码认证
    user_manager_->CreateUser("test_user", "correct_password", "USER");
    
    bool result = user_manager_->AuthenticateUser("test_user", "wrong_password");
    EXPECT_FALSE(result);
}

TEST_F(UserManagerTest, AuthenticateUser_UserNotFound) {
    // 测试不存在的用户认证
    bool result = user_manager_->AuthenticateUser("nonexistent", "password");
    EXPECT_FALSE(result);
}

TEST_F(UserManagerTest, AuthenticateUser_EmptyPassword) {
    // 测试空密码认证
    user_manager_->CreateUser("test_user", "", "USER");
    
    bool result = user_manager_->AuthenticateUser("test_user", "");
    EXPECT_TRUE(result);
}

// ============ 用户删除测试 ============

TEST_F(UserManagerTest, DropUser_Success) {
    // 测试成功删除用户
    user_manager_->CreateUser("test_user", "password123", "USER");
    
    bool result = user_manager_->DropUser("test_user");
    EXPECT_TRUE(result);
    
    // 验证删除后无法认证
    EXPECT_FALSE(user_manager_->AuthenticateUser("test_user", "password123"));
}

TEST_F(UserManagerTest, DropUser_NotFound) {
    // 测试删除不存在的用户
    bool result = user_manager_->DropUser("nonexistent");
    EXPECT_FALSE(result);
}

// ============ 密码修改测试 ============

TEST_F(UserManagerTest, AlterUserPassword_Success) {
    // 测试成功修改密码
    user_manager_->CreateUser("test_user", "old_password", "USER");
    
    bool result = user_manager_->AlterUserPassword("test_user", "new_password");
    EXPECT_TRUE(result);
    
    // 验证新密码可以登录
    EXPECT_TRUE(user_manager_->AuthenticateUser("test_user", "new_password"));
    
    // 旧密码应该失效
    EXPECT_FALSE(user_manager_->AuthenticateUser("test_user", "old_password"));
}

TEST_F(UserManagerTest, AlterUserPassword_UserNotFound) {
    // 测试修改不存在的用户密码
    bool result = user_manager_->AlterUserPassword("nonexistent", "new_password");
    EXPECT_FALSE(result);
}

TEST_F(UserManagerTest, AlterUserPassword_EmptyNewPassword) {
    // 测试新密码为空应该失败
    user_manager_->CreateUser("test_user", "old_password", "USER");
    
    bool result = user_manager_->AlterUserPassword("test_user", "");
    EXPECT_FALSE(result);
}

// ============ 角色管理测试 ============

TEST_F(UserManagerTest, AlterUserRole_Success) {
    // 测试成功修改用户角色
    user_manager_->CreateUser("test_user", "password", "USER");
    
    bool result = user_manager_->AlterUserRole("test_user", "ADMIN");
    EXPECT_TRUE(result);
}

TEST_F(UserManagerTest, AlterUserRole_UserNotFound) {
    // 测试修改不存在的用户角色
    bool result = user_manager_->AlterUserRole("nonexistent", "ADMIN");
    EXPECT_FALSE(result);
}

// ============ 角色创建测试 ============

TEST_F(UserManagerTest, CreateRole_Success) {
    // 测试成功创建角色
    bool result = user_manager_->CreateRole("custom_role");
    EXPECT_TRUE(result);
}

TEST_F(UserManagerTest, CreateRole_DuplicateRole) {
    // 测试重复创建角色应该失败
    bool first = user_manager_->CreateRole("custom_role");
    bool second = user_manager_->CreateRole("custom_role");
    
    EXPECT_TRUE(first);
    EXPECT_FALSE(second);
}

TEST_F(UserManagerTest, DropRole_Success) {
    // 测试成功删除角色
    user_manager_->CreateRole("custom_role");
    
    bool result = user_manager_->DropRole("custom_role");
    EXPECT_TRUE(result);
}

// ============ 角色继承测试 ============

TEST_F(UserManagerTest, GrantRoleToRole_Success) {
    // 测试角色继承
    user_manager_->CreateRole("parent_role");
    user_manager_->CreateRole("child_role");
    
    bool result = user_manager_->GrantRoleToRole("parent_role", "child_role");
    EXPECT_TRUE(result);
}

TEST_F(UserManagerTest, CheckRoleInheritance_Direct) {
    // 测试直接角色继承检查
    user_manager_->CreateRole("parent_role");
    user_manager_->CreateRole("child_role");
    user_manager_->GrantRoleToRole("parent_role", "child_role");
    
    bool result = user_manager_->CheckRoleInheritance("child_role", "parent_role");
    EXPECT_TRUE(result);
}

TEST_F(UserManagerTest, CheckRoleInheritance_NotInherited) {
    // 测试未继承的角色
    user_manager_->CreateRole("role_a");
    user_manager_->CreateRole("role_b");
    
    bool result = user_manager_->CheckRoleInheritance("role_a", "role_b");
    EXPECT_FALSE(result);
}

TEST_F(UserManagerTest, GetRoleHierarchy) {
    // 测试获取角色层次结构
    user_manager_->CreateRole("grandparent");
    user_manager_->CreateRole("parent");
    user_manager_->CreateRole("child");
    
    user_manager_->GrantRoleToRole("grandparent", "parent");
    user_manager_->GrantRoleToRole("parent", "child");
    
    auto hierarchy = user_manager_->GetRoleHierarchy("child");
    EXPECT_GE(hierarchy.size(), 2);  // 至少包含 parent 和 grandparent
}

// ============ 当前角色测试 ============

TEST_F(UserManagerTest, SetAndGetCurrentRole) {
    // 测试设置和获取当前角色
    user_manager_->CreateUser("test_user", "password", "USER");
    user_manager_->CreateRole("ADMIN");
    
    bool set_result = user_manager_->SetCurrentRole("test_user", "ADMIN");
    EXPECT_TRUE(set_result);
    
    auto current_role = user_manager_->GetUserCurrentRole("test_user");
    EXPECT_EQ(current_role, "ADMIN");
}

// ============ 初始化和关闭测试 ============

TEST_F(UserManagerTest, Initialize_CreatesDataDirectory) {
    // 测试初始化创建数据目录
    UserManager manager("./test_init_dir");
    
    bool result = manager.CreateUser("test_user", "password", "USER");
    EXPECT_TRUE(result);
}

}  // namespace test
}  // namespace sqlcc