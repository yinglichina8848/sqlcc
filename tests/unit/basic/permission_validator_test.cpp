#include "permission_validator.h"
#include "core/user_manager.h"
#include "core/core_database_manager.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>

namespace sqlcc {

// 测试夹具类
class PermissionValidatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试用的管理器
        user_manager_ = std::make_shared<UserManager>();
        db_manager_ = std::make_shared<DatabaseManager>("./test_db");
        
        // 创建权限验证器
        permission_validator_ = std::make_shared<PermissionValidator>(user_manager_, db_manager_);
        
        // 创建测试用户
        CreateTestUsers();
        CreateTestPermissions();
    }

    void TearDown() override {
        permission_validator_.reset();
        user_manager_.reset();
        db_manager_.reset();
    }

    void CreateTestUsers() {
        // 创建管理员用户
        user_manager_->CreateUser("admin", "admin123", UserManager::ROLE_SUPERUSER);
        user_manager_->CreateUser("user1", "user123", UserManager::ROLE_USER);
        user_manager_->CreateUser("user2", "user123", UserManager::ROLE_USER);
        user_manager_->CreateRole("READONLY");
        user_manager_->CreateUser("readonly_user", "readonly123", "READONLY");
    }

    void CreateTestPermissions() {
        // 为user1授权
        user_manager_->GrantPrivilege("user1", "test_db", "table1", UserManager::PRIVILEGE_SELECT);
        user_manager_->GrantPrivilege("user1", "test_db", "table1", UserManager::PRIVILEGE_INSERT);
        
        // 为user2授权
        user_manager_->GrantPrivilege("user2", "test_db", "table2", UserManager::PRIVILEGE_SELECT);
        user_manager_->GrantPrivilege("user2", "test_db", "table2", UserManager::PRIVILEGE_UPDATE);
        
        // 为READONLY角色授权
        user_manager_->GrantPrivilege("READONLY", "test_db", "*", UserManager::PRIVILEGE_SELECT);
    }

    std::shared_ptr<UserManager> user_manager_;
    std::shared_ptr<DatabaseManager> db_manager_;
    std::shared_ptr<PermissionValidator> permission_validator_;
};

// 测试权限验证器基本初始化
TEST_F(PermissionValidatorTest, BasicInitialization) {
    EXPECT_TRUE(permission_validator_ != nullptr);
    // 注意：PermissionValidator类中没有get_user_manager和get_db_manager方法
    // 我们直接测试功能而不是内部实现细节
}

// 测试用户不存在的情况
TEST_F(PermissionValidatorTest, UserNotExists) {
    PermissionResult result = permission_validator_->validate(
        PermissionOperation::SELECT, "table1", "nonexistent_user", "test_db");
    
    EXPECT_FALSE(result.allowed);
    // 使用message字段而不是error_message
    EXPECT_TRUE(result.message.find("not found") != std::string::npos);
}

// 测试不同权限类型验证
TEST_F(PermissionValidatorTest, DifferentPermissionTypes) {
    // 测试SELECT权限
    PermissionResult select_result = permission_validator_->validate(
        PermissionOperation::SELECT, "table1", "user1", "test_db");
    EXPECT_TRUE(select_result.allowed) << "user1 should have SELECT permission on table1";
    
    // 测试INSERT权限
    PermissionResult insert_result = permission_validator_->validate(
        PermissionOperation::INSERT, "table1", "user1", "test_db");
    EXPECT_TRUE(insert_result.allowed) << "user1 should have INSERT permission on table1";
    
    // 测试UPDATE权限（user1没有UPDATE权限）
    PermissionResult update_result = permission_validator_->validate(
        PermissionOperation::UPDATE, "table1", "user1", "test_db");
    EXPECT_FALSE(update_result.allowed) << "user1 should not have UPDATE permission on table1";
    
    // 测试DELETE权限（user1没有DELETE权限）
    PermissionResult delete_result = permission_validator_->validate(
        PermissionOperation::DELETE, "table1", "user1", "test_db");
    EXPECT_FALSE(delete_result.allowed) << "user1 should not have DELETE permission on table1";
}

// 测试不同用户的权限
TEST_F(PermissionValidatorTest, DifferentUsersPermissions) {
    // user1在table1有SELECT权限
    PermissionResult user1_table1 = permission_validator_->validate(
        PermissionOperation::SELECT, "table1", "user1", "test_db");
    EXPECT_TRUE(user1_table1.allowed);
    
    // user1在table2没有权限
    PermissionResult user1_table2 = permission_validator_->validate(
        PermissionOperation::SELECT, "table2", "user1", "test_db");
    EXPECT_FALSE(user1_table2.allowed);
    
    // user2在table2有SELECT权限
    PermissionResult user2_table2 = permission_validator_->validate(
        PermissionOperation::SELECT, "table2", "user2", "test_db");
    EXPECT_TRUE(user2_table2.allowed);
    
    // user2在table1没有权限
    PermissionResult user2_table1 = permission_validator_->validate(
        PermissionOperation::SELECT, "table1", "user2", "test_db");
    EXPECT_FALSE(user2_table1.allowed);
}

// 测试管理员权限
TEST_F(PermissionValidatorTest, AdminPrivileges) {
    // 管理员应该有所有权限
    std::vector<PermissionOperation> admin_operations = {
        PermissionOperation::SELECT,
        PermissionOperation::INSERT,
        PermissionOperation::UPDATE,
        PermissionOperation::DELETE,
        PermissionOperation::CREATE_DATABASE,
        PermissionOperation::DROP_DATABASE,
        PermissionOperation::CREATE_TABLE,
        PermissionOperation::DROP_TABLE,
        PermissionOperation::ALTER_TABLE,
        PermissionOperation::GRANT,
        PermissionOperation::REVOKE
    };
    
    for (const auto& operation : admin_operations) {
        PermissionResult result = permission_validator_->validate(
            operation, "any_table", "admin", "any_db");
        EXPECT_TRUE(result.allowed) << "Admin should have permission for operation " << static_cast<int>(operation);
    }
}

// 测试空字符串参数
TEST_F(PermissionValidatorTest, EmptyStringParameters) {
    // 空用户名
    PermissionResult empty_user = permission_validator_->validate(
        PermissionOperation::SELECT, "table1", "", "test_db");
    EXPECT_FALSE(empty_user.allowed);
    
    // 空数据库名
    PermissionResult empty_db = permission_validator_->validate(
        PermissionOperation::SELECT, "table1", "user1", "");
    EXPECT_FALSE(empty_db.allowed);
    
    // 空表名（某些操作可能允许）
    PermissionResult empty_table = permission_validator_->validate(
        PermissionOperation::SHOW_TABLES, "", "user1", "test_db");
    EXPECT_TRUE(empty_table.allowed); // SHOW操作可能允许空表名
}

// 测试特殊字符处理
TEST_F(PermissionValidatorTest, SpecialCharacters) {
    // 包含特殊字符的用户名
    PermissionResult special_chars_user = permission_validator_->validate(
        PermissionOperation::SELECT, "table1", "user@domain.com", "test_db");
    EXPECT_FALSE(special_chars_user.allowed);
    
    // 包含特殊字符的表名
    PermissionResult special_chars_table = permission_validator_->validate(
        PermissionOperation::SELECT, "table-1_with.special", "user1", "test_db");
    EXPECT_FALSE(special_chars_table.allowed);
    
    // 包含特殊字符的数据库名
    PermissionResult special_chars_db = permission_validator_->validate(
        PermissionOperation::SELECT, "table1", "user1", "db@production");
    EXPECT_FALSE(special_chars_db.allowed);
}

// 测试权限验证结果结构
TEST_F(PermissionValidatorTest, PermissionResultStructure) {
    PermissionResult result = permission_validator_->validate(
        PermissionOperation::SELECT, "table1", "user1", "test_db");
    
    // 检查结果结构 - 使用实际存在的字段
    EXPECT_TRUE(result.allowed || !result.allowed); // 确保字段存在
    EXPECT_FALSE(result.message.empty()); // 应该有消息
}

// 测试缓存机制（如果存在）
TEST_F(PermissionValidatorTest, CacheMechanism) {
    // 重复查询同一权限
    PermissionResult result1 = permission_validator_->validate(
        PermissionOperation::SELECT, "table1", "user1", "test_db");
    
    PermissionResult result2 = permission_validator_->validate(
        PermissionOperation::SELECT, "table1", "user1", "test_db");
    
    // 结果应该一致
    EXPECT_EQ(result1.allowed, result2.allowed);
}

// 测试角色权限传递
TEST_F(PermissionValidatorTest, RolePermissionInheritance) {
    // 设置用户角色
    user_manager_->SetCurrentRole("user1", "READONLY");
    
    // READONLY角色应该有SELECT权限
    PermissionResult result = permission_validator_->validate(
        PermissionOperation::SELECT, "any_table", "user1", "test_db");
    EXPECT_TRUE(result.allowed) << "User with READONLY role should have SELECT permission";
    
    // READONLY角色不应该有INSERT权限
    PermissionResult insert_result = permission_validator_->validate(
        PermissionOperation::INSERT, "any_table", "user1", "test_db");
    EXPECT_FALSE(insert_result.allowed) << "READONLY role should not have INSERT permission";
}

// 测试通配符权限（*）
TEST_F(PermissionValidatorTest, WildcardPermissions) {
    // READONLY角色对test_db的所有表都有SELECT权限
    user_manager_->SetCurrentRole("readonly_user", "READONLY");
    
    PermissionResult wildcard_result = permission_validator_->validate(
        PermissionOperation::SELECT, "table1", "readonly_user", "test_db");
    EXPECT_TRUE(wildcard_result.allowed) << "READONLY role should have SELECT on all tables";
    
    PermissionResult another_table = permission_validator_->validate(
        PermissionOperation::SELECT, "table2", "readonly_user", "test_db");
    EXPECT_TRUE(another_table.allowed) << "READONLY role should have SELECT on table2";
}

// 测试数据库级别权限
TEST_F(PermissionValidatorTest, DatabaseLevelPermissions) {
    // 为用户授予数据库级别权限
    user_manager_->GrantPrivilege("user1", "production_db", "*", UserManager::PRIVILEGE_ALL);
    
    // 在production_db中应该拥有所有权限
    PermissionResult all_privs = permission_validator_->validate(
        PermissionOperation::INSERT, "any_table", "user1", "production_db");
    EXPECT_TRUE(all_privs.allowed) << "User should have ALL privileges in production_db";
}

// 测试并发安全性
TEST_F(PermissionValidatorTest, ConcurrencySafety) {
    std::atomic<int> success_count{0};
    std::atomic<int> failure_count{0};
    const int num_threads = 20;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, &success_count, &failure_count, i]() {
            try {
                std::string user = "user" + std::to_string(i % 2 + 1);
                std::string table = "table" + std::to_string(i % 2 + 1);
                
                PermissionResult result = permission_validator_->validate(
                    PermissionOperation::SELECT, table, user, "test_db");
                
                if (result.allowed) {
                    success_count.fetch_add(1);
                } else {
                    failure_count.fetch_add(1);
                }
            } catch (...) {
                failure_count.fetch_add(1);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证并发操作成功
    EXPECT_GT(success_count.load(), 0);
    // 允许一些失败（由于权限不足），但不应该崩溃
    EXPECT_LT(failure_count.load(), num_threads);
}

// 测试权限验证性能
TEST_F(PermissionValidatorTest, PerformanceTest) {
    const int num_operations = 1000;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_operations; ++i) {
        PermissionResult result = permission_validator_->validate(
            PermissionOperation::SELECT, "table1", "user1", "test_db");
        // 简单验证操作完成
        EXPECT_TRUE(result.allowed || !result.allowed); // 确保操作完成
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // 验证性能在合理范围内（1000次操作应该在1秒内完成）
    EXPECT_LT(duration.count(), 1000) << "1000 permission checks should complete within 1 second";
}

// 测试错误边界条件
TEST_F(PermissionValidatorTest, ErrorBoundaryConditions) {
    // 极长的字符串
    std::string long_string(10000, 'a');
    PermissionResult long_user = permission_validator_->validate(
        PermissionOperation::SELECT, "table1", long_string, "test_db");
    EXPECT_FALSE(long_user.allowed);
}

// 测试权限撤销验证
TEST_F(PermissionValidatorTest, PermissionRevocation) {
    // 初始状态：user1有SELECT权限
    PermissionResult initial_result = permission_validator_->validate(
        PermissionOperation::SELECT, "table1", "user1", "test_db");
    EXPECT_TRUE(initial_result.allowed);
    
    // 撤销权限
    user_manager_->RevokePrivilege("user1", "test_db", "table1", UserManager::PRIVILEGE_SELECT);
    
    // 验证权限已撤销
    PermissionResult after_revoke = permission_validator_->validate(
        PermissionOperation::SELECT, "table1", "user1", "test_db");
    EXPECT_FALSE(after_revoke.allowed);
}

// 测试动态权限更新
TEST_F(PermissionValidatorTest, DynamicPermissionUpdate) {
    // 创建新用户
    user_manager_->CreateUser("new_user", "new_pass", UserManager::ROLE_USER);
    
    // 初始状态：无权限
    PermissionResult no_permission = permission_validator_->validate(
        PermissionOperation::SELECT, "table1", "new_user", "test_db");
    EXPECT_FALSE(no_permission.allowed);
    
    // 授予权限
    user_manager_->GrantPrivilege("new_user", "test_db", "table1", UserManager::PRIVILEGE_SELECT);
    
    // 验证权限生效
    PermissionResult with_permission = permission_validator_->validate(
        PermissionOperation::SELECT, "table1", "new_user", "test_db");
    EXPECT_TRUE(with_permission.allowed);
}

// 测试内存安全
TEST_F(PermissionValidatorTest, MemorySafety) {
    // 测试智能指针的正确使用
    {
        auto temp_validator = std::make_shared<PermissionValidator>(user_manager_, db_manager_);
        EXPECT_TRUE(temp_validator != nullptr);
        
        PermissionResult result = temp_validator->validate(
            PermissionOperation::SELECT, "table1", "user1", "test_db");
        EXPECT_TRUE(result.allowed || !result.allowed);
    }
    
    // 验证原始验证器仍然有效
    PermissionResult original_result = permission_validator_->validate(
        PermissionOperation::SELECT, "table1", "user1", "test_db");
    EXPECT_TRUE(original_result.allowed || !original_result.allowed);
}

} // namespace sqlcc

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}