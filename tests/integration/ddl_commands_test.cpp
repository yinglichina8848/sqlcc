#include "sql_executor.h"
#include "unified_executor.h"
#include "database_manager.h"
#include "system_database.h"
#include "user_manager.h"
#include <gtest/gtest.h>
#include <filesystem>

namespace sqlcc {

// DDL命令测试类
class DDLCommandsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试数据目录
        test_data_dir_ = "./test_ddl_" + std::to_string(std::time(nullptr));
        std::filesystem::create_directory(test_data_dir_);

        // 初始化数据库管理器
        db_manager_ = std::make_shared<DatabaseManager>(
            test_data_dir_ + "/test.db", 1024, 4, 2);

        // 初始化用户管理器
        user_manager_ = std::make_shared<UserManager>();

        // 初始化系统数据库
        system_db_ = std::make_shared<SystemDatabase>(db_manager_);

        // 初始化统一执行器
        unified_executor_ = std::make_shared<UnifiedExecutor>(
            db_manager_, user_manager_, system_db_);

        // 初始化SQL执行器
        sql_executor_ = std::make_unique<SqlExecutor>();
    }

    void TearDown() override {
        // 清理资源
        sql_executor_.reset();
        unified_executor_.reset();
        system_db_.reset();
        user_manager_.reset();
        db_manager_.reset();

        // 删除测试数据目录
        std::filesystem::remove_all(test_data_dir_);
    }

    // 执行SQL并验证结果
    void ExecuteAndVerify(const std::string& sql, const std::string& expected_keyword = "success") {
        std::string result = sql_executor_->Execute(sql);

        // 检查是否包含预期关键词
        EXPECT_TRUE(result.find(expected_keyword) != std::string::npos ||
                   result.find("错误") == std::string::npos);

        // 检查错误状态
        std::string error = sql_executor_->GetLastError();
        EXPECT_TRUE(error.empty() || error.find("错误") == std::string::npos);

        std::cout << "SQL: " << sql << std::endl;
        std::cout << "结果: " << result << std::endl;
        if (!error.empty()) {
            std::cout << "错误: " << error << std::endl;
        }
    }

    // 验证表是否存在
    void VerifyTableExists(const std::string& table_name) {
        bool exists = db_manager_->TableExists(table_name);
        EXPECT_TRUE(exists) << "表 " << table_name << " 应该存在";
    }

    std::string test_data_dir_;
    std::shared_ptr<DatabaseManager> db_manager_;
    std::shared_ptr<UserManager> user_manager_;
    std::shared_ptr<SystemDatabase> system_db_;
    std::shared_ptr<UnifiedExecutor> unified_executor_;
    std::unique_ptr<SqlExecutor> sql_executor_;
};

// 测试DDL命令综合功能
TEST_F(DDLCommandsTest, DDLCommandsComprehensiveTest) {
    std::cout << "\n=== DDL命令综合测试开始 ===" << std::endl;

    // 1.1 创建数据库
    ExecuteAndVerify("CREATE DATABASE test_db_ddl");
    ExecuteAndVerify("USE test_db_ddl");

    // 1.2 创建表结构
    ExecuteAndVerify("CREATE TABLE users ("
                     "id INTEGER PRIMARY KEY, "
                     "name VARCHAR(50) NOT NULL, "
                     "age INTEGER, "
                     "email VARCHAR(100))");

    // 验证表创建成功
    VerifyTableExists("users");

    // 1.3 修改表结构
    ExecuteAndVerify("ALTER TABLE users ADD COLUMN phone VARCHAR(20)");
    ExecuteAndVerify("ALTER TABLE users ADD COLUMN address TEXT");

    // 1.4 创建索引
    ExecuteAndVerify("CREATE INDEX idx_users_name ON users (name)");
    ExecuteAndVerify("CREATE INDEX idx_users_email ON users (email)");

    // 1.5 创建第二个表
    ExecuteAndVerify("CREATE TABLE orders ("
                     "order_id INTEGER PRIMARY KEY, "
                     "user_id INTEGER, "
                     "product_name VARCHAR(100), "
                     "amount DECIMAL(10,2), "
                     "order_date DATE)");

    // 1.6 创建外键关系
    ExecuteAndVerify("ALTER TABLE orders ADD CONSTRAINT fk_user_id "
                     "FOREIGN KEY (user_id) REFERENCES users(id)");

    // 1.7 删除表和数据库
    ExecuteAndVerify("DROP TABLE orders");
    ExecuteAndVerify("DROP TABLE users");
    ExecuteAndVerify("DROP DATABASE test_db_ddl");

    std::cout << "=== DDL命令综合测试完成 ===" << std::endl;
}

// 测试表创建的各种数据类型
TEST_F(DDLCommandsTest, TableCreationWithVariousDataTypes) {
    std::cout << "\n=== 表创建数据类型测试开始 ===" << std::endl;

    ExecuteAndVerify("CREATE DATABASE datatype_test");
    ExecuteAndVerify("USE datatype_test");

    // 创建包含各种数据类型的表
    ExecuteAndVerify("CREATE TABLE all_types_table ("
                     "id INTEGER PRIMARY KEY, "
                     "int_col INTEGER, "
                     "varchar_col VARCHAR(100), "
                     "text_col TEXT, "
                     "decimal_col DECIMAL(10,2), "
                     "date_col DATE, "
                     "time_col TIME, "
                     "timestamp_col TIMESTAMP, "
                     "boolean_col BOOLEAN)");

    VerifyTableExists("all_types_table");

    // 清理
    ExecuteAndVerify("DROP TABLE all_types_table");
    ExecuteAndVerify("DROP DATABASE datatype_test");

    std::cout << "=== 表创建数据类型测试完成 ===" << std::endl;
}

// 测试索引创建和删除
TEST_F(DDLCommandsTest, IndexCreationAndDeletion) {
    std::cout << "\n=== 索引创建和删除测试开始 ===" << std::endl;

    ExecuteAndVerify("CREATE DATABASE index_test");
    ExecuteAndVerify("USE index_test");

    // 创建表
    ExecuteAndVerify("CREATE TABLE indexed_table ("
                     "id INTEGER PRIMARY KEY, "
                     "name VARCHAR(100), "
                     "category VARCHAR(50), "
                     "value INTEGER)");

    // 创建各种索引
    ExecuteAndVerify("CREATE INDEX idx_name ON indexed_table (name)");
    ExecuteAndVerify("CREATE INDEX idx_category ON indexed_table (category)");
    ExecuteAndVerify("CREATE INDEX idx_composite ON indexed_table (category, value)");
    ExecuteAndVerify("CREATE UNIQUE INDEX idx_unique_name ON indexed_table (name)");

    // 删除索引
    ExecuteAndVerify("DROP INDEX idx_name");
    ExecuteAndVerify("DROP INDEX idx_composite");

    // 清理
    ExecuteAndVerify("DROP TABLE indexed_table");
    ExecuteAndVerify("DROP DATABASE index_test");

    std::cout << "=== 索引创建和删除测试完成 ===" << std::endl;
}

} // namespace sqlcc

// 主函数
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}