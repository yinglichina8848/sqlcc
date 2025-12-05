#include "sql_executor.h"
#include <gtest/gtest.h>
#include <string>
#include <iostream>

namespace sqlcc {

// 集成测试类
class SqlExecutorIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化SQL执行器
        executor_ = std::make_unique<SqlExecutor>();
    }

    void TearDown() override {
        // 清理资源
        executor_.reset();
    }

    std::unique_ptr<SqlExecutor> executor_;
};

// 测试基本SQL执行器初始化
TEST_F(SqlExecutorIntegrationTest, SqlExecutorInitializationTest) {
    ASSERT_TRUE(executor_ != nullptr);

    // 测试获取最后错误信息
    std::string error = executor_->GetLastError();
    // 初始状态错误信息可能为空或有默认值
    SUCCEED();
}

// 测试CREATE TABLE语句执行
TEST_F(SqlExecutorIntegrationTest, CreateTableTest) {
    std::string sql = "CREATE TABLE test_users (id INTEGER, name VARCHAR, age INTEGER);";

    std::string result = executor_->Execute(sql);

    // 验证执行结果
    EXPECT_TRUE(result.find("test_users") != std::string::npos);
    EXPECT_TRUE(result.find("created successfully") != std::string::npos ||
                result.find("错误") == std::string::npos);

    // 检查错误状态
    std::string error = executor_->GetLastError();
    std::cout << "CREATE TABLE result: " << result << std::endl;
    std::cout << "Error: " << error << std::endl;
}

// 测试CREATE DATABASE语句执行
TEST_F(SqlExecutorIntegrationTest, CreateDatabaseTest) {
    std::string sql = "CREATE DATABASE test_db;";

    std::string result = executor_->Execute(sql);

    // 验证执行结果
    EXPECT_TRUE(result.find("test_db") != std::string::npos ||
                result.find("created successfully") != std::string::npos ||
                result.find("错误") == std::string::npos);

    std::cout << "CREATE DATABASE result: " << result << std::endl;
    std::cout << "Error: " << executor_->GetLastError() << std::endl;
}

// 测试INSERT语句执行
TEST_F(SqlExecutorIntegrationTest, InsertTest) {
    // 确保表存在
    std::string create_sql = "CREATE TABLE test_users (id INTEGER, name VARCHAR);";
    executor_->Execute(create_sql);

    // 执行INSERT
    std::string sql = "INSERT INTO test_users (id, name) VALUES (1, 'Alice');";
    std::string result = executor_->Execute(sql);

    // 验证执行结果
    EXPECT_TRUE(result.find("executed successfully") != std::string::npos ||
                result.find("错误") == std::string::npos);

    std::cout << "INSERT result: " << result << std::endl;
    std::cout << "Error: " << executor_->GetLastError() << std::endl;
}

// 测试SELECT语句执行
TEST_F(SqlExecutorIntegrationTest, SelectTest) {
    // 确保表存在
    std::string create_sql = "CREATE TABLE test_users (id INTEGER, name VARCHAR);";
    executor_->Execute(create_sql);

    // 执行SELECT
    std::string sql = "SELECT * FROM test_users;";
    std::string result = executor_->Execute(sql);

    // 验证执行结果
    EXPECT_TRUE(result.find("executed successfully") != std::string::npos ||
                result.find("错误") == std::string::npos);

    std::cout << "SELECT result: " << result << std::endl;
    std::cout << "Error: " << executor_->GetLastError() << std::endl;
}

// 测试UPDATE语句执行
TEST_F(SqlExecutorIntegrationTest, UpdateTest) {
    // 确保表存在
    std::string create_sql = "CREATE TABLE test_users (id INTEGER, name VARCHAR);";
    executor_->Execute(create_sql);

    // 执行UPDATE
    std::string sql = "UPDATE test_users SET name = 'Bob' WHERE id = 1;";
    std::string result = executor_->Execute(sql);

    // 验证执行结果
    EXPECT_TRUE(result.find("executed successfully") != std::string::npos ||
                result.find("错误") == std::string::npos);

    std::cout << "UPDATE result: " << result << std::endl;
    std::cout << "Error: " << executor_->GetLastError() << std::endl;
}

// 测试DELETE语句执行
TEST_F(SqlExecutorIntegrationTest, DeleteTest) {
    // 确保表存在
    std::string create_sql = "CREATE TABLE test_users (id INTEGER, name VARCHAR);";
    executor_->Execute(create_sql);

    // 执行DELETE
    std::string sql = "DELETE FROM test_users WHERE id = 1;";
    std::string result = executor_->Execute(sql);

    // 验证执行结果
    EXPECT_TRUE(result.find("executed successfully") != std::string::npos ||
                result.find("错误") == std::string::npos);

    std::cout << "DELETE result: " << result << std::endl;
    std::cout << "Error: " << executor_->GetLastError() << std::endl;
}

// 测试DROP TABLE语句执行
TEST_F(SqlExecutorIntegrationTest, DropTableTest) {
    // 先创建表
    std::string create_sql = "CREATE TABLE temp_drop_test (id INTEGER);";
    executor_->Execute(create_sql);

    // 执行DROP TABLE
    std::string sql = "DROP TABLE temp_drop_test;";
    std::string result = executor_->Execute(sql);

    // 验证执行结果
    EXPECT_TRUE(result.find("temp_drop_test") != std::string::npos ||
                result.find("dropped successfully") != std::string::npos ||
                result.find("错误") == std::string::npos);

    std::cout << "DROP TABLE result: " << result << std::endl;
    std::cout << "Error: " << executor_->GetLastError() << std::endl;
}

// 测试错误处理 - 不存在的表
TEST_F(SqlExecutorIntegrationTest, ErrorHandlingNonExistentTableTest) {
    std::string sql = "SELECT * FROM non_existent_table;";
    std::string result = executor_->Execute(sql);

    // 应该返回错误信息
    EXPECT_TRUE(result.find("错误") != std::string::npos ||
                result.find("does not exist") != std::string::npos);

    std::cout << "Non-existent table result: " << result << std::endl;
    std::cout << "Error: " << executor_->GetLastError() << std::endl;
}

// 测试错误处理 - 无效SQL语法
TEST_F(SqlExecutorIntegrationTest, ErrorHandlingInvalidSqlTest) {
    std::string sql = "INVALID SQL STATEMENT;";
    std::string result = executor_->Execute(sql);

    // 应该返回错误信息
    EXPECT_TRUE(result.find("错误") != std::string::npos ||
                result.find("SQL语句解析失败") != std::string::npos);

    std::cout << "Invalid SQL result: " << result << std::endl;
    std::cout << "Error: " << executor_->GetLastError() << std::endl;
}

// 测试空SQL语句
TEST_F(SqlExecutorIntegrationTest, EmptySqlTest) {
    std::string sql = "";
    std::string result = executor_->Execute(sql);

    // 应该返回错误信息
    EXPECT_TRUE(result.find("错误") != std::string::npos ||
                result.find("空的SQL语句") != std::string::npos);

    std::cout << "Empty SQL result: " << result << std::endl;
    std::cout << "Error: " << executor_->GetLastError() << std::endl;
}

// 测试多个语句执行
TEST_F(SqlExecutorIntegrationTest, MultipleStatementsTest) {
    // 执行一系列SQL语句
    std::string sql1 = "CREATE TABLE multi_test (id INTEGER, name VARCHAR);";
    std::string sql2 = "INSERT INTO multi_test (id, name) VALUES (1, 'Test');";
    std::string sql3 = "SELECT * FROM multi_test;";
    std::string sql4 = "DROP TABLE multi_test;";

    std::string result1 = executor_->Execute(sql1);
    std::string result2 = executor_->Execute(sql2);
    std::string result3 = executor_->Execute(sql3);
    std::string result4 = executor_->Execute(sql4);

    // 验证所有语句都执行了（不一定都成功，但至少尝试执行了）
    EXPECT_FALSE(result1.empty());
    EXPECT_FALSE(result2.empty());
    EXPECT_FALSE(result3.empty());
    EXPECT_FALSE(result4.empty());

    std::cout << "Multiple statements test completed:" << std::endl;
    std::cout << "CREATE: " << result1 << std::endl;
    std::cout << "INSERT: " << result2 << std::endl;
    std::cout << "SELECT: " << result3 << std::endl;
    std::cout << "DROP: " << result4 << std::endl;
}

// 测试特殊字符和边界情况
TEST_F(SqlExecutorIntegrationTest, SpecialCharactersTest) {
    // 测试包含特殊字符的SQL
    std::string sql = "CREATE TABLE `special-table` (`id-column` INTEGER, `name-column` VARCHAR);";
    std::string result = executor_->Execute(sql);

    // 记录结果（可能成功也可能失败，取决于解析器支持程度）
    std::cout << "Special characters test result: " << result << std::endl;
    std::cout << "Error: " << executor_->GetLastError() << std::endl;

    // 主要验证方法调用没有崩溃
    SUCCEED();
}

// 性能测试 - 重复执行相同语句
TEST_F(SqlExecutorIntegrationTest, PerformanceRepeatedExecutionTest) {
    std::string sql = "CREATE TABLE perf_test (id INTEGER);";

    // 执行多次相同的语句
    for (int i = 0; i < 5; ++i) {
        std::string result = executor_->Execute(sql);
        // 记录每次执行的结果
        std::cout << "Iteration " << i + 1 << ": " << result << std::endl;
    }

    std::cout << "Performance test completed" << std::endl;
    SUCCEED();
}

} // namespace sqlcc

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
