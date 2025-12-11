#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <iostream>
#include <filesystem>

#include "execution_engine.h"
#include "core/user_manager.h"
#include "core/system_database.h"
#include "database_manager.h"
#include "sql_parser/ast_nodes.h"

using namespace sqlcc;
namespace fs = std::filesystem;

class DCLExecutorTest : public ::testing::Test {
protected:
    std::string test_dir = "./dcl_executor_test_data";
    std::shared_ptr<DatabaseManager> db_manager_;
    std::shared_ptr<UserManager> user_manager_;
    std::shared_ptr<SystemDatabase> system_db_;
    
    void SetUp() override {
        // 创建测试目录
        fs::create_directories(test_dir);
        
        // 初始化数据库管理器
        db_manager_ = std::make_shared<DatabaseManager>(test_dir);
        
        // 初始化系统数据库
        system_db_ = std::make_shared<SystemDatabase>(db_manager_);
        ASSERT_TRUE(system_db_->Initialize()) << "Failed to initialize SystemDatabase: " << system_db_->GetLastError();
        
        // 初始化用户管理器
        user_manager_ = std::make_shared<UserManager>(test_dir);
        user_manager_->SetSystemDatabase(system_db_);
    }

    void TearDown() override {
        // 清理测试数据
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }
};

// 测试CREATE USER语句执行
TEST_F(DCLExecutorTest, CreateUserExecution) {
    // 创建DCL执行器
    DCLExecutor executor(db_manager_, user_manager_);
    
    // 解析CREATE USER语句
    // 修改为使用正确的AST节点创建方式
    auto stmt = std::make_unique<sql_parser::CreateUserStatement>("testuser", "password123");
    ASSERT_NE(stmt, nullptr) << "Failed to create CREATE USER statement";
    
    // 执行语句
    ExecutionResult result = executor.execute(std::move(stmt));
    EXPECT_TRUE(result.success) << "CREATE USER execution failed: " << result.message;
    
    // 验证用户是否创建成功
    EXPECT_TRUE(user_manager_->AuthenticateUser("testuser", "password123")) 
        << "User authentication failed after CREATE USER";
}

// 测试DROP USER语句执行
TEST_F(DCLExecutorTest, DropUserExecution) {
    // 首先创建用户
    ASSERT_TRUE(user_manager_->CreateUser("testuser", "password123", "USER"))
        << "Failed to create user for DROP USER test";
    
    // 创建DCL执行器
    DCLExecutor executor(db_manager_, user_manager_);
    
    // 解析DROP USER语句
    // 修改为使用正确的AST节点创建方式
    auto stmt = std::make_unique<sql_parser::DropUserStatement>("testuser");
    ASSERT_NE(stmt, nullptr) << "Failed to create DROP USER statement";
    
    // 执行语句
    ExecutionResult result = executor.execute(std::move(stmt));
    EXPECT_TRUE(result.success) << "DROP USER execution failed: " << result.message;
    
    // 验证用户是否删除成功
    EXPECT_FALSE(user_manager_->AuthenticateUser("testuser", "password123"))
        << "User authentication should fail after DROP USER";
}

// 测试GRANT语句执行
TEST_F(DCLExecutorTest, GrantExecution) {
    // 首先创建用户
    ASSERT_TRUE(user_manager_->CreateUser("testuser", "password123", "USER"))
        << "Failed to create user for GRANT test";
    
    // 创建DCL执行器
    DCLExecutor executor(db_manager_, user_manager_);
    
    // 解析GRANT语句
    // 修改为使用正确的AST节点创建和设置方式，使用具体的表名
    auto stmt = std::make_unique<sql_parser::GrantStatement>();
    stmt->addPrivilege("SELECT");
    stmt->setObjectType("*");  // 数据库通配符
    stmt->setObjectName("testtable");  // 具体表名而不是通配符
    stmt->setGrantee("testuser");
    ASSERT_NE(stmt, nullptr) << "Failed to create GRANT statement";
    
    // 执行语句
    ExecutionResult result = executor.execute(std::move(stmt));
    EXPECT_TRUE(result.success) << "GRANT execution failed: " << result.message;
    
    // 验证权限是否授予成功
    EXPECT_TRUE(user_manager_->CheckPermission("testuser", "testdb", "testtable", "SELECT"))
        << "Permission check failed after GRANT";
}

// 测试REVOKE语句执行
TEST_F(DCLExecutorTest, RevokeExecution) {
    // 首先创建用户并授予权限
    ASSERT_TRUE(user_manager_->CreateUser("testuser", "password123", "USER"))
        << "Failed to create user for REVOKE test";
    // 修改为使用具体的表名而不是通配符，确保权限可以正确匹配
    ASSERT_TRUE(user_manager_->GrantPrivilege("testuser", "*", "testtable", "SELECT"))
        << "Failed to grant privilege for REVOKE test";
    
    // 创建DCL执行器
    DCLExecutor executor(db_manager_, user_manager_);
    
    // 解析REVOKE语句
    // 修改为使用正确的AST节点创建和设置方式，使用与GRANT相同的参数
    auto stmt = std::make_unique<sql_parser::RevokeStatement>();
    stmt->addPrivilege("SELECT");
    stmt->setObjectType("*");  // 与GRANT相同
    stmt->setObjectName("testtable");  // 与GRANT相同，使用具体表名而不是通配符
    stmt->setGrantee("testuser");
    ASSERT_NE(stmt, nullptr) << "Failed to create REVOKE statement";
    
    // 执行语句
    ExecutionResult result = executor.execute(std::move(stmt));
    EXPECT_TRUE(result.success) << "REVOKE execution failed: " << result.message;
    
    // 验证权限是否撤销成功
    EXPECT_FALSE(user_manager_->CheckPermission("testuser", "testdb", "testtable", "SELECT"))
        << "Permission check should fail after REVOKE";
}