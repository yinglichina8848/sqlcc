#include <gtest/gtest.h>
#include "execution_engine.h"
#include "database_manager.h"
#include "sql_parser/parser.h"
#include <filesystem>

namespace fs = std::filesystem;

class DMLExecutorIntegrationTest : public ::testing::Test {
protected:
    std::string test_dir = "./dml_executor_test";
    std::shared_ptr<sqlcc::DatabaseManager> db_manager;
    
    void SetUp() override {
        // 清理旧的测试目录
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
        
        // 创建数据库管理器
        db_manager = std::make_shared<sqlcc::DatabaseManager>(test_dir);
        
        // 创建测试数据库和表
        ASSERT_TRUE(db_manager->CreateDatabase("testdb"));
        ASSERT_TRUE(db_manager->UseDatabase("testdb"));
        
        // 创建测试表
        std::vector<std::pair<std::string, std::string>> columns = {
            {"id", "INTEGER"},
            {"name", "VARCHAR"},
            {"age", "INTEGER"}
        };
        ASSERT_TRUE(db_manager->CreateTable("users", columns));
    }
    
    void TearDown() override {
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }
};

// 测试INSERT基础功能
TEST_F(DMLExecutorIntegrationTest, InsertBasicTest) {
    sqlcc::DMLExecutor executor(db_manager);
    
    // 创建INSERT语句
    std::string sql = "INSERT INTO users (id, name, age) VALUES (1, 'Alice', 25);";
    sqlcc::sql_parser::Parser parser(sql);
    auto stmts = parser.parseStatements();
    
    ASSERT_FALSE(stmts.empty());
    auto result = executor.execute(std::move(stmts[0]));
    
    // 验证执行结果
    EXPECT_TRUE(result.success);
    EXPECT_NE(result.message.find("executed successfully"), std::string::npos);
}

// 测试INSERT到不存在的表
TEST_F(DMLExecutorIntegrationTest, InsertToNonExistentTableTest) {
    sqlcc::DMLExecutor executor(db_manager);
    
    std::string sql = "INSERT INTO nonexistent (id, name) VALUES (1, 'Bob');";
    sqlcc::sql_parser::Parser parser(sql);
    auto stmts = parser.parseStatements();
    
    ASSERT_FALSE(stmts.empty());
    auto result = executor.execute(std::move(stmts[0]));
    
    // 验证应该失败
    EXPECT_FALSE(result.success);
    EXPECT_NE(result.message.find("does not exist"), std::string::npos);
}

// 测试UPDATE基础功能
TEST_F(DMLExecutorIntegrationTest, UpdateBasicTest) {
    sqlcc::DMLExecutor executor(db_manager);
    
    // 先插入数据
    std::string insert_sql = "INSERT INTO users (id, name, age) VALUES (1, 'Alice', 25);";
    sqlcc::sql_parser::Parser insert_parser(insert_sql);
    auto insert_stmts = insert_parser.parseStatements();
    executor.execute(std::move(insert_stmts[0]));
    
    // 更新数据
    std::string update_sql = "UPDATE users SET age = 26 WHERE id = 1;";
    sqlcc::sql_parser::Parser update_parser(update_sql);
    auto update_stmts = update_parser.parseStatements();
    
    ASSERT_FALSE(update_stmts.empty());
    auto result = executor.execute(std::move(update_stmts[0]));
    
    // 验证执行结果
    EXPECT_TRUE(result.success);
    EXPECT_NE(result.message.find("executed successfully"), std::string::npos);
}

// 测试DELETE基础功能
TEST_F(DMLExecutorIntegrationTest, DeleteBasicTest) {
    sqlcc::DMLExecutor executor(db_manager);
    
    // 先插入数据
    std::string insert_sql = "INSERT INTO users (id, name, age) VALUES (1, 'Alice', 25);";
    sqlcc::sql_parser::Parser insert_parser(insert_sql);
    auto insert_stmts = insert_parser.parseStatements();
    executor.execute(std::move(insert_stmts[0]));
    
    // 删除数据
    std::string delete_sql = "DELETE FROM users WHERE id = 1;";
    sqlcc::sql_parser::Parser delete_parser(delete_sql);
    auto delete_stmts = delete_parser.parseStatements();
    
    ASSERT_FALSE(delete_stmts.empty());
    auto result = executor.execute(std::move(delete_stmts[0]));
    
    // 验证执行结果
    EXPECT_TRUE(result.success);
    EXPECT_NE(result.message.find("executed successfully"), std::string::npos);
}

// 测试DELETE不存在的表
TEST_F(DMLExecutorIntegrationTest, DeleteFromNonExistentTableTest) {
    sqlcc::DMLExecutor executor(db_manager);
    
    std::string sql = "DELETE FROM nonexistent WHERE id = 1;";
    sqlcc::sql_parser::Parser parser(sql);
    auto stmts = parser.parseStatements();
    
    ASSERT_FALSE(stmts.empty());
    auto result = executor.execute(std::move(stmts[0]));
    
    // 验证应该失败
    EXPECT_FALSE(result.success);
    EXPECT_NE(result.message.find("does not exist"), std::string::npos);
}

// 测试在没有选择数据库的情况下执行DML
TEST_F(DMLExecutorIntegrationTest, DMLWithoutDatabaseSelectedTest) {
    // 创建新的数据库管理器，不选择任何数据库
    auto temp_db = std::make_shared<sqlcc::DatabaseManager>(test_dir + "_temp");
    sqlcc::DMLExecutor executor(temp_db);
    
    std::string sql = "INSERT INTO users (id, name) VALUES (1, 'Alice');";
    sqlcc::sql_parser::Parser parser(sql);
    auto stmts = parser.parseStatements();
    
    ASSERT_FALSE(stmts.empty());
    auto result = executor.execute(std::move(stmts[0]));
    
    // 验证应该失败
    EXPECT_FALSE(result.success);
    EXPECT_NE(result.message.find("No database selected"), std::string::npos);
    
    fs::remove_all(test_dir + "_temp");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
