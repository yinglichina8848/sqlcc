/**
 * @file user_manager_test.cpp
 * @brief UserManager 高覆盖率测试套件
 *
 * 实现UserManager的全面测试，包括：
 * - 用户创建、删除、修改
 * - 用户认证和密码管理
 * - 角色创建、管理和分配
 * - 权限授予、撤销和检查
 * - 数据持久化和恢复
 * - 并发安全性和边界条件
 * - 错误处理和异常情况
 */

#include "core/user_manager.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <thread>
#include <atomic>
#include <filesystem>
#include <fstream>

using namespace sqlcc;
using namespace std::chrono_literals;

// Mock 类用于隔离外部依赖
// 注意：由于SystemDatabase定义不完整，这里使用简化的Mock
class MockSystemDatabase {
public:
    MOCK_METHOD(bool, initialize, ());
    MOCK_METHOD(bool, createUser, (const std::string&, const std::string&, const std::string&));
    MOCK_METHOD(bool, dropUser, (const std::string&));
    MOCK_METHOD(bool, grantPrivilege, (const std::string&, const std::string&, const std::string&, const std::string&));
    MOCK_METHOD(bool, revokePrivilege, (const std::string&, const std::string&, const std::string&, const std::string&));
    MOCK_METHOD(bool, checkPermission, (const std::string&, const std::string&, const std::string&, const std::string&));
};

// 测试夹具
class UserManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时测试目录
        test_data_path_ = std::filesystem::temp_directory_path() / "sqlcc_test_usermgr";
        std::filesystem::create_directories(test_data_path_);

        // 创建UserManager实例
        user_manager_ = std::make_unique<UserManager>(test_data_path_.string());
        // 注意：由于SystemDatabase定义不完整，暂时不设置Mock
        // 专注于测试UserManager的核心功能
    }

    void TearDown() override {
        // 清理资源
        user_manager_.reset();

        // 删除测试目录
        if (std::filesystem::exists(test_data_path_)) {
            std::filesystem::remove_all(test_data_path_);
        }
    }

    std::filesystem::path test_data_path_;
    std::unique_ptr<UserManager> user_manager_;
};

// 用户创建测试
TEST_F(UserManagerTest, UserCreation_Success) {
    // 测试成功创建用户
    std::string username = "testuser";
    std::string password = "testpass";
    std::string role = "USER";

    bool result = user_manager_->CreateUser(username, password, role);
    EXPECT_TRUE(result);

    // 验证用户存在
    auto users = user_manager_->ListUsers();
    ASSERT_EQ(users.size(), 2); // 包括默认超级用户
    auto it = std::find_if(users.begin(), users.end(),
                          [&username](const User& u) { return u.username == username; });
    ASSERT_NE(it, users.end());
    EXPECT_EQ(it->role, role);
    EXPECT_TRUE(it->is_active);
}

TEST_F(UserManagerTest, UserCreation_DuplicateUser) {
    // 测试创建重复用户
    std::string username = "testuser";
    std::string password = "testpass";

    // 第一次创建应该成功
    bool result1 = user_manager_->CreateUser(username, password);
    EXPECT_TRUE(result1);

    // 第二次创建应该失败
    bool result2 = user_manager_->CreateUser(username, "different_pass");
    EXPECT_FALSE(result2);

    // 检查错误信息
    EXPECT_FALSE(user_manager_->GetLastError().empty());
}

TEST_F(UserManagerTest, UserCreation_EmptyUsername) {
    // 测试创建空用户名用户
    bool result = user_manager_->CreateUser("", "password");
    EXPECT_FALSE(result);
    EXPECT_FALSE(user_manager_->GetLastError().empty());
}

TEST_F(UserManagerTest, UserCreation_EmptyPassword) {
    // 测试创建空密码用户
    bool result = user_manager_->CreateUser("testuser", "");
    EXPECT_FALSE(result);
    EXPECT_FALSE(user_manager_->GetLastError().empty());
}

TEST_F(UserManagerTest, UserCreation_InvalidRole) {
    // 测试创建无效角色用户
    bool result = user_manager_->CreateUser("testuser", "password", "INVALID_ROLE");
    EXPECT_FALSE(result);
    EXPECT_FALSE(user_manager_->GetLastError().empty());
}

// 用户删除测试
TEST_F(UserManagerTest, UserDeletion_Success) {
    // 测试成功删除用户
    std::string username = "testuser";

    // 先创建用户
    user_manager_->CreateUser(username, "password");

    // 删除用户
    bool result = user_manager_->DropUser(username);
    EXPECT_TRUE(result);

    // 验证用户已删除
    auto users = user_manager_->ListUsers();
    auto it = std::find_if(users.begin(), users.end(),
                          [&username](const User& u) { return u.username == username; });
    EXPECT_EQ(it, users.end());
}

TEST_F(UserManagerTest, UserDeletion_NonExistentUser) {
    // 测试删除不存在的用户
    bool result = user_manager_->DropUser("nonexistent");
    EXPECT_FALSE(result);
    EXPECT_FALSE(user_manager_->GetLastError().empty());
}

TEST_F(UserManagerTest, UserDeletion_Superuser) {
    // 测试删除超级用户（应该失败）
    bool result = user_manager_->DropUser("admin"); // 假设默认超级用户是admin
    EXPECT_FALSE(result);
}

// 用户认证测试
TEST_F(UserManagerTest, UserAuthentication_Success) {
    // 测试成功认证
    std::string username = "testuser";
    std::string password = "testpass";

    // 创建用户
    user_manager_->CreateUser(username, password);

    // 认证
    bool result = user_manager_->AuthenticateUser(username, password);
    EXPECT_TRUE(result);
}

TEST_F(UserManagerTest, UserAuthentication_WrongPassword) {
    // 测试错误密码认证
    std::string username = "testuser";
    std::string correct_password = "correct";
    std::string wrong_password = "wrong";

    // 创建用户
    user_manager_->CreateUser(username, correct_password);

    // 使用错误密码认证
    bool result = user_manager_->AuthenticateUser(username, wrong_password);
    EXPECT_FALSE(result);
}

TEST_F(UserManagerTest, UserAuthentication_NonExistentUser) {
    // 测试不存在用户认证
    bool result = user_manager_->AuthenticateUser("nonexistent", "password");
    EXPECT_FALSE(result);
}

// 密码修改测试
TEST_F(UserManagerTest, PasswordChange_Success) {
    // 测试成功修改密码
    std::string username = "testuser";
    std::string old_password = "oldpass";
    std::string new_password = "newpass";

    // 创建用户
    EXPECT_CALL(*mock_sys_db_, createUser(username, testing::_, "USER"))
        .WillOnce(testing::Return(true));
    user_manager_->CreateUser(username, old_password);

    // 修改密码
    bool result = user_manager_->AlterUserPassword(username, new_password);
    EXPECT_TRUE(result);

    // 验证新密码有效
    bool auth_result = user_manager_->AuthenticateUser(username, new_password);
    EXPECT_TRUE(auth_result);

    // 验证旧密码无效
    bool old_auth_result = user_manager_->AuthenticateUser(username, old_password);
    EXPECT_FALSE(old_auth_result);
}

TEST_F(UserManagerTest, PasswordChange_NonExistentUser) {
    // 测试修改不存在用户的密码
    bool result = user_manager_->AlterUserPassword("nonexistent", "newpass");
    EXPECT_FALSE(result);
}

TEST_F(UserManagerTest, PasswordChange_EmptyPassword) {
    // 测试设置空密码
    std::string username = "testuser";

    // 创建用户
    EXPECT_CALL(*mock_sys_db_, createUser(username, testing::_, "USER"))
        .WillOnce(testing::Return(true));
    user_manager_->CreateUser(username, "password");

    // 修改为空密码
    bool result = user_manager_->AlterUserPassword(username, "");
    EXPECT_FALSE(result);
}

// 角色管理测试
TEST_F(UserManagerTest, RoleCreation_Success) {
    // 测试成功创建角色
    std::string role_name = "testrole";

    bool result = user_manager_->CreateRole(role_name);
    EXPECT_TRUE(result);

    // 验证角色存在
    auto roles = user_manager_->ListRoles();
    auto it = std::find_if(roles.begin(), roles.end(),
                          [&role_name](const Role& r) { return r.role_name == role_name; });
    EXPECT_NE(it, roles.end());
}

TEST_F(UserManagerTest, RoleCreation_DuplicateRole) {
    // 测试创建重复角色
    std::string role_name = "testrole";

    // 第一次创建成功
    user_manager_->CreateRole(role_name);

    // 第二次创建失败
    bool result = user_manager_->CreateRole(role_name);
    EXPECT_FALSE(result);
}

TEST_F(UserManagerTest, RoleCreation_EmptyName) {
    // 测试创建空名称角色
    bool result = user_manager_->CreateRole("");
    EXPECT_FALSE(result);
}

TEST_F(UserManagerTest, RoleDeletion_Success) {
    // 测试成功删除角色
    std::string role_name = "testrole";

    // 创建角色
    user_manager_->CreateRole(role_name);

    // 删除角色
    bool result = user_manager_->DropRole(role_name);
    EXPECT_TRUE(result);

    // 验证角色已删除
    auto roles = user_manager_->ListRoles();
    auto it = std::find_if(roles.begin(), roles.end(),
                          [&role_name](const Role& r) { return r.role_name == role_name; });
    EXPECT_EQ(it, roles.end());
}

TEST_F(UserManagerTest, RoleDeletion_NonExistentRole) {
    // 测试删除不存在的角色
    bool result = user_manager_->DropRole("nonexistent");
    EXPECT_FALSE(result);
}

// 用户角色分配测试
TEST_F(UserManagerTest, UserRoleAssignment_Success) {
    // 测试成功分配用户角色
    std::string username = "testuser";
    std::string role_name = "testrole";

    // 创建用户和角色
    EXPECT_CALL(*mock_sys_db_, createUser(username, testing::_, "USER"))
        .WillOnce(testing::Return(true));
    user_manager_->CreateUser(username, "password");
    user_manager_->CreateRole(role_name);

    // 分配角色
    bool result = user_manager_->AlterUserRole(username, role_name);
    EXPECT_TRUE(result);

    // 验证用户角色
    auto users = user_manager_->ListUsers();
    auto it = std::find_if(users.begin(), users.end(),
                          [&username](const User& u) { return u.username == username; });
    ASSERT_NE(it, users.end());
    EXPECT_EQ(it->role, role_name);
}

TEST_F(UserManagerTest, UserRoleAssignment_InvalidRole) {
    // 测试分配无效角色
    std::string username = "testuser";

    // 创建用户
    EXPECT_CALL(*mock_sys_db_, createUser(username, testing::_, "USER"))
        .WillOnce(testing::Return(true));
    user_manager_->CreateUser(username, "password");

    // 分配不存在的角色
    bool result = user_manager_->AlterUserRole(username, "nonexistent_role");
    EXPECT_FALSE(result);
}

// 权限管理测试
TEST_F(UserManagerTest, PrivilegeGrant_Success) {
    // 测试成功授予权限
    std::string username = "testuser";
    std::string database = "testdb";
    std::string table = "testtable";
    std::string privilege = "SELECT";

    // 创建用户
    EXPECT_CALL(*mock_sys_db_, createUser(username, testing::_, "USER"))
        .WillOnce(testing::Return(true));
    user_manager_->CreateUser(username, "password");

    // 授予权限
    EXPECT_CALL(*mock_sys_db_, grantPrivilege(username, database, table, privilege))
        .WillOnce(testing::Return(true));
    bool result = user_manager_->GrantPrivilege(username, database, table, privilege);
    EXPECT_TRUE(result);
}

TEST_F(UserManagerTest, PrivilegeGrant_NonExistentUser) {
    // 测试给不存在用户授予权限
    bool result = user_manager_->GrantPrivilege("nonexistent", "db", "table", "SELECT");
    EXPECT_FALSE(result);
}

TEST_F(UserManagerTest, PrivilegeRevoke_Success) {
    // 测试成功撤销权限
    std::string username = "testuser";
    std::string database = "testdb";
    std::string table = "testtable";
    std::string privilege = "SELECT";

    // 创建用户并授予权限
    EXPECT_CALL(*mock_sys_db_, createUser(username, testing::_, "USER"))
        .WillOnce(testing::Return(true));
    user_manager_->CreateUser(username, "password");

    EXPECT_CALL(*mock_sys_db_, grantPrivilege(username, database, table, privilege))
        .WillOnce(testing::Return(true));
    user_manager_->GrantPrivilege(username, database, table, privilege);

    // 撤销权限
    EXPECT_CALL(*mock_sys_db_, revokePrivilege(username, database, table, privilege))
        .WillOnce(testing::Return(true));
    bool result = user_manager_->RevokePrivilege(username, database, table, privilege);
    EXPECT_TRUE(result);
}

TEST_F(UserManagerTest, PermissionCheck_Granted) {
    // 测试权限检查 - 已授予
    std::string username = "testuser";
    std::string database = "testdb";
    std::string table = "testtable";
    std::string privilege = "SELECT";

    // 创建用户并授予权限
    EXPECT_CALL(*mock_sys_db_, createUser(username, testing::_, "USER"))
        .WillOnce(testing::Return(true));
    user_manager_->CreateUser(username, "password");

    EXPECT_CALL(*mock_sys_db_, grantPrivilege(username, database, table, privilege))
        .WillOnce(testing::Return(true));
    user_manager_->GrantPrivilege(username, database, table, privilege);

    // 检查权限
    EXPECT_CALL(*mock_sys_db_, checkPermission(username, database, table, privilege))
        .WillOnce(testing::Return(true));
    bool result = user_manager_->CheckPermission(username, database, table, privilege);
    EXPECT_TRUE(result);
}

TEST_F(UserManagerTest, PermissionCheck_Denied) {
    // 测试权限检查 - 未授予
    std::string username = "testuser";
    std::string database = "testdb";
    std::string table = "testtable";
    std::string privilege = "INSERT";

    // 创建用户（没有授予INSERT权限）
    EXPECT_CALL(*mock_sys_db_, createUser(username, testing::_, "USER"))
        .WillOnce(testing::Return(true));
    user_manager_->CreateUser(username, "password");

    // 检查权限
    EXPECT_CALL(*mock_sys_db_, checkPermission(username, database, table, privilege))
        .WillOnce(testing::Return(false));
    bool result = user_manager_->CheckPermission(username, database, table, privilege);
    EXPECT_FALSE(result);
}

// 数据持久化测试
TEST_F(UserManagerTest, DataPersistence_SaveAndLoad) {
    // 测试数据保存和加载
    std::string username = "persistuser";

    // 创建用户
    EXPECT_CALL(*mock_sys_db_, createUser(username, testing::_, "USER"))
        .WillOnce(testing::Return(true));
    user_manager_->CreateUser(username, "password");

    // 保存数据
    bool save_result = user_manager_->SaveToFile();
    EXPECT_TRUE(save_result);

    // 创建新的UserManager实例并加载数据
    auto new_user_manager = std::make_unique<UserManager>(test_data_path_.string());
    new_user_manager->SetSystemDatabase(mock_sys_db_);

    bool load_result = new_user_manager->LoadFromFile();
    EXPECT_TRUE(load_result);

    // 验证用户数据已恢复
    auto users = new_user_manager->ListUsers();
    auto it = std::find_if(users.begin(), users.end(),
                          [&username](const User& u) { return u.username == username; });
    EXPECT_NE(it, users.end());
}

// 查询方法测试
TEST_F(UserManagerTest, ListUsers_AllUsers) {
    // 测试列出所有用户
    std::string username1 = "user1";
    std::string username2 = "user2";

    // 创建多个用户
    EXPECT_CALL(*mock_sys_db_, createUser(username1, testing::_, "USER"))
        .WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_sys_db_, createUser(username2, testing::_, "USER"))
        .WillOnce(testing::Return(true));
    user_manager_->CreateUser(username1, "pass1");
    user_manager_->CreateUser(username2, "pass2");

    // 列出用户
    auto users = user_manager_->ListUsers();
    EXPECT_GE(users.size(), 3); // 包括默认用户

    // 验证特定用户存在
    bool found_user1 = std::any_of(users.begin(), users.end(),
                                  [&username1](const User& u) { return u.username == username1; });
    bool found_user2 = std::any_of(users.begin(), users.end(),
                                  [&username2](const User& u) { return u.username == username2; });
    EXPECT_TRUE(found_user1);
    EXPECT_TRUE(found_user2);
}

TEST_F(UserManagerTest, ListRoles_AllRoles) {
    // 测试列出所有角色
    std::string role1 = "role1";
    std::string role2 = "role2";

    // 创建多个角色
    user_manager_->CreateRole(role1);
    user_manager_->CreateRole(role2);

    // 列出角色
    auto roles = user_manager_->ListRoles();
    EXPECT_GE(roles.size(), 5); // 包括默认角色

    // 验证特定角色存在
    bool found_role1 = std::any_of(roles.begin(), roles.end(),
                                  [&role1](const Role& r) { return r.role_name == role1; });
    bool found_role2 = std::any_of(roles.begin(), roles.end(),
                                  [&role2](const Role& r) { return r.role_name == role2; });
    EXPECT_TRUE(found_role1);
    EXPECT_TRUE(found_role2);
}

// 并发安全测试
TEST_F(UserManagerTest, ConcurrentAccess_Safe) {
    // 测试并发访问的安全性
    std::atomic<bool> test_passed{true};
    std::vector<std::thread> threads;

    // 启动多个线程同时创建用户
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([this, &test_passed, i]() {
            try {
                std::string username = "user" + std::to_string(i);
                std::string password = "pass" + std::to_string(i);

                // 模拟SystemDatabase调用
                EXPECT_CALL(*mock_sys_db_, createUser(username, testing::_, "USER"))
                    .WillOnce(testing::Return(true));

                bool result = user_manager_->CreateUser(username, password);
                if (!result) {
                    test_passed = false;
                }
            } catch (...) {
                test_passed = false;
            }
        });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_TRUE(test_passed);

    // 验证所有用户都已创建
    auto users = user_manager_->ListUsers();
    EXPECT_GE(users.size(), 11); // 10个新用户 + 默认用户
}

// 边界条件测试
TEST_F(UserManagerTest, BoundaryConditions_LongUsername) {
    // 测试长用户名
    std::string long_username(256, 'a'); // 256字符用户名
    std::string password = "password";

    EXPECT_CALL(*mock_sys_db_, createUser(long_username, testing::_, "USER"))
        .WillOnce(testing::Return(true));

    bool result = user_manager_->CreateUser(long_username, password);
    EXPECT_TRUE(result);
}

TEST_F(UserManagerTest, BoundaryConditions_SpecialCharacters) {
    // 测试特殊字符
    std::string special_username = "user@#$%^&*()";
    std::string special_password = "pass!@#$%^&*()";

    EXPECT_CALL(*mock_sys_db_, createUser(special_username, testing::_, "USER"))
        .WillOnce(testing::Return(true));

    bool result = user_manager_->CreateUser(special_username, special_password);
    EXPECT_TRUE(result);
}

// 错误处理测试
TEST_F(UserManagerTest, ErrorHandling_LastError) {
    // 测试错误信息获取
    // 尝试创建重复用户
    std::string username = "testuser";

    EXPECT_CALL(*mock_sys_db_, createUser(username, testing::_, "USER"))
        .WillOnce(testing::Return(true));
    user_manager_->CreateUser(username, "password");

    // 再次创建应该失败
    user_manager_->CreateUser(username, "different_password");

    // 检查错误信息
    std::string error = user_manager_->GetLastError();
    EXPECT_FALSE(error.empty());
    EXPECT_NE(error.find("already exists"), std::string::npos);
}

// 性能测试
TEST_F(UserManagerTest, Performance_BulkUserCreation) {
    // 测试批量用户创建性能
    auto start = std::chrono::high_resolution_clock::now();

    // 创建100个用户
    for (int i = 0; i < 100; ++i) {
        std::string username = "perfuser" + std::to_string(i);

        EXPECT_CALL(*mock_sys_db_, createUser(username, testing::_, "USER"))
            .WillOnce(testing::Return(true));

        user_manager_->CreateUser(username, "password");
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 性能断言：100个用户创建应该在合理时间内完成
    EXPECT_LT(duration.count(), 1000); // 少于1秒
}

// 压力测试
TEST_F(UserManagerTest, StressTest_RapidOperations) {
    // 快速操作压力测试
    std::string base_username = "stressuser";

    // 快速创建、修改、删除用户
    for (int i = 0; i < 50; ++i) {
        std::string username = base_username + std::to_string(i);

        // 创建
        EXPECT_CALL(*mock_sys_db_, createUser(username, testing::_, "USER"))
            .WillOnce(testing::Return(true));
        user_manager_->CreateUser(username, "password");

        // 修改密码
        user_manager_->AlterUserPassword(username, "newpassword");

        // 删除
        EXPECT_CALL(*mock_sys_db_, dropUser(username))
            .WillOnce(testing::Return(true));
        user_manager_->DropUser(username);
    }

    // 验证最终状态
    auto users = user_manager_->ListUsers();
    // 应该只有默认用户保留
    EXPECT_EQ(users.size(), 1); // 只有默认超级用户
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
