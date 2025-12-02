#include "gtest/gtest.h"
#include "user_manager.h"
#include "system_database.h"
#include "database_manager.h"
#include <iostream>
#include <memory>
#include <filesystem>
#include <fstream>

using namespace sqlcc;
namespace fs = std::filesystem;

class RevokePersistenceTest : public ::testing::Test {
protected:
    std::string test_dir = "./test_revoke_data_gtest";
    
    void SetUp() override {
        // 清理旧测试数据
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }
    
    void TearDown() override {
        // 清理测试数据
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }
    
    int CountPrivilegesInFile(const std::string& username) {
        std::string perm_file = test_dir + "/permissions.dat";
        if (!fs::exists(perm_file)) {
            return 0;
        }
        
        std::ifstream file(perm_file);
        std::string line;
        int count = 0;
        
        while (std::getline(file, line)) {
            if (line.find(username) != std::string::npos) {
                count++;
            }
        }
        
        return count;
    }
};

TEST_F(RevokePersistenceTest, GrantAndRevokePersistence) {
    // ========== 阶段1: 创建用户并授权 ==========
    {
        auto db_manager = std::make_shared<DatabaseManager>(test_dir);
        auto sys_db = std::make_shared<SystemDatabase>(db_manager);
        
        ASSERT_TRUE(sys_db->Initialize()) << "Failed to initialize SystemDatabase";
        
        UserManager user_mgr(test_dir);
        user_mgr.SetSystemDatabase(sys_db.get());
        
        // 创建测试用户
        ASSERT_TRUE(user_mgr.CreateUser("alice", "pass123", "USER")) << "Failed to create user alice";
        
        // 授予3个权限
        ASSERT_TRUE(user_mgr.GrantPrivilege("alice", "testdb", "users", "SELECT")) << "Failed to grant SELECT";
        ASSERT_TRUE(user_mgr.GrantPrivilege("alice", "testdb", "users", "INSERT")) << "Failed to grant INSERT";
        ASSERT_TRUE(user_mgr.GrantPrivilege("alice", "testdb", "orders", "UPDATE")) << "Failed to grant UPDATE";
    }
    
    // 验证权限文件存在且有3个权限
    int count_before = CountPrivilegesInFile("alice");
    EXPECT_EQ(count_before, 3) << "Expected 3 privileges after grant";
    
    // ========== 阶段2: 重新加载并撤销1个权限 ==========
    {
        auto db_manager = std::make_shared<DatabaseManager>(test_dir);
        auto sys_db = std::make_shared<SystemDatabase>(db_manager);
        
        ASSERT_TRUE(sys_db->Initialize()) << "Failed to re-initialize SystemDatabase";
        
        UserManager user_mgr(test_dir);
        user_mgr.SetSystemDatabase(sys_db.get());
        
        // 撤销INSERT权限
        ASSERT_TRUE(user_mgr.RevokePrivilege("alice", "testdb", "users", "INSERT")) << "Failed to revoke INSERT privilege";
    }
    
    // 验证权限文件现在只有2个权限
    int count_after = CountPrivilegesInFile("alice");
    EXPECT_EQ(count_after, 2) << "Expected 2 privileges after revoke";
    
    // ========== 阶段3: 再次重新加载验证持久化 ==========
    {
        auto db_manager = std::make_shared<DatabaseManager>(test_dir);
        auto sys_db = std::make_shared<SystemDatabase>(db_manager);
        
        ASSERT_TRUE(sys_db->Initialize()) << "Failed to re-initialize SystemDatabase again";
        
        UserManager user_mgr(test_dir);
        user_mgr.SetSystemDatabase(sys_db.get());
    }
    
    // 最终验证：权限文件仍然只有2个权限
    int count_final = CountPrivilegesInFile("alice");
    EXPECT_EQ(count_final, 2) << "REVOKE did not persist! Expected 2 privileges after restart";
    
    // 验证具体权限内容
    std::string perm_file = test_dir + "/permissions.dat";
    std::ifstream file(perm_file);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    EXPECT_NE(content.find("SELECT"), std::string::npos) << "SELECT privilege should exist";
    EXPECT_NE(content.find("UPDATE"), std::string::npos) << "UPDATE privilege should exist";
    EXPECT_EQ(content.find("INSERT"), std::string::npos) << "INSERT privilege should have been revoked";
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
