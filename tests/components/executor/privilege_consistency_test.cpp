#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <iostream>
#include <filesystem>

#include "system_database.h"
#include "database_manager.h"
#include "user_manager.h"
#include "sql_executor.h"

using namespace sqlcc;
namespace fs = std::filesystem;

class PrivilegeConsistencyTest : public ::testing::Test {
protected:
    std::string test_dir = "./privilege_consistency_test_data";
    
    void SetUp() override {
        // 清理之前的测试数据
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
        fs::create_directories(test_dir);
    }

    void TearDown() override {
        // 清理测试数据
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }
};

// 测试CREATE USER操作的权限元数据同步
TEST_F(PrivilegeConsistencyTest, CreateUserMetadataSync) {
    auto db_manager = std::make_shared<DatabaseManager>(test_dir);
    auto sys_db = std::make_shared<SystemDatabase>(db_manager);
    
    ASSERT_TRUE(sys_db->Initialize()) << "Failed to initialize SystemDatabase: " << sys_db->GetLastError();
    
    UserManager user_mgr(test_dir);
    user_mgr.SetSystemDatabase(sys_db.get());
    
    // 创建用户
    ASSERT_TRUE(user_mgr.CreateUser("testuser", "password123", "USER")) 
        << "Failed to create user: " << user_mgr.GetLastError();
    
    // 验证用户记录已同步到系统数据库
    SqlExecutor sql_exec(db_manager);
    std::string result = sql_exec.Execute("SELECT * FROM sys_users WHERE username = 'testuser'");
    EXPECT_NE(result.find("testuser"), std::string::npos) 
        << "User record not found in system database";
    
    // 验证用户一致性检查通过
    EXPECT_TRUE(sys_db->UserExists("testuser")) 
        << "User existence check failed";
}

// 测试DROP USER操作的权限元数据同步
TEST_F(PrivilegeConsistencyTest, DropUserMetadataSync) {
    auto db_manager = std::make_shared<DatabaseManager>(test_dir);
    auto sys_db = std::make_shared<SystemDatabase>(db_manager);
    
    ASSERT_TRUE(sys_db->Initialize()) << "Failed to initialize SystemDatabase: " << sys_db->GetLastError();
    
    UserManager user_mgr(test_dir);
    user_mgr.SetSystemDatabase(sys_db.get());
    
    // 创建用户
    ASSERT_TRUE(user_mgr.CreateUser("testuser", "password123", "USER")) 
        << "Failed to create user: " << user_mgr.GetLastError();
    
    // 删除用户
    ASSERT_TRUE(user_mgr.DropUser("testuser")) 
        << "Failed to drop user: " << user_mgr.GetLastError();
    
    // 验证用户记录已从系统数据库删除
    SqlExecutor sql_exec(db_manager);
    std::string result = sql_exec.Execute("SELECT * FROM sys_users WHERE username = 'testuser'");
    EXPECT_EQ(result.find("testuser"), std::string::npos) 
        << "User record still exists in system database after drop";
    
    // 验证用户一致性检查失败（用户不存在）
    EXPECT_FALSE(sys_db->UserExists("testuser")) 
        << "User should not exist after drop";
}

// 测试CREATE ROLE操作的权限元数据同步
TEST_F(PrivilegeConsistencyTest, CreateRoleMetadataSync) {
    auto db_manager = std::make_shared<DatabaseManager>(test_dir);
    auto sys_db = std::make_shared<SystemDatabase>(db_manager);
    
    ASSERT_TRUE(sys_db->Initialize()) << "Failed to initialize SystemDatabase: " << sys_db->GetLastError();
    
    UserManager user_mgr(test_dir);
    user_mgr.SetSystemDatabase(sys_db.get());
    
    // 创建角色
    ASSERT_TRUE(user_mgr.CreateRole("testrole")) 
        << "Failed to create role: " << user_mgr.GetLastError();
    
    // 验证角色记录已同步到系统数据库
    SqlExecutor sql_exec(db_manager);
    std::string result = sql_exec.Execute("SELECT * FROM sys_roles WHERE role_name = 'testrole'");
    EXPECT_NE(result.find("testrole"), std::string::npos) 
        << "Role record not found in system database";
    
    // 验证角色一致性检查通过
    EXPECT_TRUE(sys_db->RoleExists("testrole")) 
        << "Role existence check failed";
}

// 测试DROP ROLE操作的权限元数据同步
TEST_F(PrivilegeConsistencyTest, DropRoleMetadataSync) {
    auto db_manager = std::make_shared<DatabaseManager>(test_dir);
    auto sys_db = std::make_shared<SystemDatabase>(db_manager);
    
    ASSERT_TRUE(sys_db->Initialize()) << "Failed to initialize SystemDatabase: " << sys_db->GetLastError();
    
    UserManager user_mgr(test_dir);
    user_mgr.SetSystemDatabase(sys_db.get());
    
    // 创建角色
    ASSERT_TRUE(user_mgr.CreateRole("testrole")) 
        << "Failed to create role: " << user_mgr.GetLastError();
    
    // 删除角色
    ASSERT_TRUE(user_mgr.DropRole("testrole")) 
        << "Failed to drop role: " << user_mgr.GetLastError();
    
    // 验证角色记录已从系统数据库删除
    SqlExecutor sql_exec(db_manager);
    std::string result = sql_exec.Execute("SELECT * FROM sys_roles WHERE role_name = 'testrole'");
    EXPECT_EQ(result.find("testrole"), std::string::npos) 
        << "Role record still exists in system database after drop";
    
    // 验证角色一致性检查失败（角色不存在）
    EXPECT_FALSE(sys_db->RoleExists("testrole")) 
        << "Role should not exist after drop";
}

// 测试GRANT权限操作的权限元数据同步
TEST_F(PrivilegeConsistencyTest, GrantPrivilegeMetadataSync) {
    auto db_manager = std::make_shared<DatabaseManager>(test_dir);
    auto sys_db = std::make_shared<SystemDatabase>(db_manager);
    
    ASSERT_TRUE(sys_db->Initialize()) << "Failed to initialize SystemDatabase: " << sys_db->GetLastError();
    
    UserManager user_mgr(test_dir);
    user_mgr.SetSystemDatabase(sys_db.get());
    
    // 创建用户
    ASSERT_TRUE(user_mgr.CreateUser("testuser", "password123", "USER")) 
        << "Failed to create user: " << user_mgr.GetLastError();
    
    // 授予权限
    ASSERT_TRUE(user_mgr.GrantPrivilege("testuser", "testdb", "testtable", "SELECT")) 
        << "Failed to grant privilege: " << user_mgr.GetLastError();
    
    // 验证权限记录已同步到系统数据库
    SqlExecutor sql_exec(db_manager);
    std::string result = sql_exec.Execute("SELECT * FROM sys_privileges WHERE grantee_name = 'testuser' AND privilege = 'SELECT'");
    EXPECT_NE(result.find("SELECT"), std::string::npos) 
        << "Privilege record not found in system database";
    
    // 验证权限一致性检查通过
    EXPECT_TRUE(sys_db->CheckPrivilegeConsistency("testuser")) 
        << "Privilege consistency check failed";
}

// 测试REVOKE权限操作的权限元数据同步
TEST_F(PrivilegeConsistencyTest, RevokePrivilegeMetadataSync) {
    auto db_manager = std::make_shared<DatabaseManager>(test_dir);
    auto sys_db = std::make_shared<SystemDatabase>(db_manager);
    
    ASSERT_TRUE(sys_db->Initialize()) << "Failed to initialize SystemDatabase: " << sys_db->GetLastError();
    
    UserManager user_mgr(test_dir);
    user_mgr.SetSystemDatabase(sys_db.get());
    
    // 创建用户
    ASSERT_TRUE(user_mgr.CreateUser("testuser", "password123", "USER")) 
        << "Failed to create user: " << user_mgr.GetLastError();
    
    // 授予权限
    ASSERT_TRUE(user_mgr.GrantPrivilege("testuser", "testdb", "testtable", "SELECT")) 
        << "Failed to grant privilege: " << user_mgr.GetLastError();
    
    // 撤销权限
    ASSERT_TRUE(user_mgr.RevokePrivilege("testuser", "testdb", "testtable", "SELECT")) 
        << "Failed to revoke privilege: " << user_mgr.GetLastError();
    
    // 验证权限记录已从系统数据库删除
    SqlExecutor sql_exec(db_manager);
    std::string result = sql_exec.Execute("SELECT * FROM sys_privileges WHERE grantee_name = 'testuser' AND privilege = 'SELECT'");
    EXPECT_EQ(result.find("SELECT"), std::string::npos) 
        << "Privilege record still exists in system database after revoke";
}

// 测试权限元数据持久化
TEST_F(PrivilegeConsistencyTest, PrivilegeMetadataPersistence) {
    // 第一阶段：创建用户和权限
    {
        auto db_manager = std::make_shared<DatabaseManager>(test_dir);
        auto sys_db = std::make_shared<SystemDatabase>(db_manager);
        
        ASSERT_TRUE(sys_db->Initialize()) << "Failed to initialize SystemDatabase: " << sys_db->GetLastError();
        
        UserManager user_mgr(test_dir);
        user_mgr.SetSystemDatabase(sys_db.get());
        
        // 创建用户
        ASSERT_TRUE(user_mgr.CreateUser("testuser", "password123", "USER")) 
            << "Failed to create user: " << user_mgr.GetLastError();
        
        // 授予权限
        ASSERT_TRUE(user_mgr.GrantPrivilege("testuser", "testdb", "testtable", "SELECT")) 
            << "Failed to grant privilege: " << user_mgr.GetLastError();
        
        ASSERT_TRUE(user_mgr.GrantPrivilege("testuser", "testdb", "testtable", "INSERT")) 
            << "Failed to grant privilege: " << user_mgr.GetLastError();
    } // UserManager和SystemDatabase在此处析构，应触发数据保存
    
    // 第二阶段：重新加载并验证持久化
    {
        auto db_manager = std::make_shared<DatabaseManager>(test_dir);
        auto sys_db = std::make_shared<SystemDatabase>(db_manager);
        
        ASSERT_TRUE(sys_db->Initialize()) << "Failed to re-initialize SystemDatabase: " << sys_db->GetLastError();
        
        UserManager user_mgr(test_dir);
        user_mgr.SetSystemDatabase(sys_db.get());
        
        // 验证用户仍然存在
        EXPECT_TRUE(sys_db->UserExists("testuser")) 
            << "User should exist after restart";
        
        // 验证权限仍然存在
        SqlExecutor sql_exec(db_manager);
        std::string result = sql_exec.Execute("SELECT COUNT(*) FROM sys_privileges WHERE grantee_name = 'testuser'");
        EXPECT_NE(result.find("2"), std::string::npos) 
            << "Should have 2 privileges after restart";
        
        // 验证权限一致性检查通过
        EXPECT_TRUE(sys_db->CheckPrivilegeConsistency("testuser")) 
            << "Privilege consistency check failed after restart";
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}