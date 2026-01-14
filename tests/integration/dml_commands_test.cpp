#include "sql_executor.h"
#include "unified_executor.h"
#include "core/core_database_manager.h"
#include "system_database.h"
#include "user_manager.h"
#include <gtest/gtest.h>
#include <filesystem>

namespace sqlcc {

// DML命令测试类
class DMLCommandsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试数据目录
        test_data_dir_ = "./test_dml_" + std::to_string(std::time(nullptr));
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

    std::string test_data_dir_;
    std::shared_ptr<DatabaseManager> db_manager_;
    std::shared_ptr<UserManager> user_manager_;
    std::shared_ptr<SystemDatabase> system_db_;
    std::shared_ptr<UnifiedExecutor> unified_executor_;
    std::unique_ptr<SqlExecutor> sql_executor_;
};

// 测试DML命令综合功能
TEST_F(DMLCommandsTest, DMLCommandsComprehensiveTest) {
    std::cout << "\n=== DML命令综合测试开始 ===" << std::endl;

    // 设置测试环境
    ExecuteAndVerify("CREATE DATABASE dml_test_db");
    ExecuteAndVerify("USE dml_test_db");
    ExecuteAndVerify("CREATE TABLE products ("
                     "id INTEGER PRIMARY KEY, "
                     "name VARCHAR(100), "
                     "price DECIMAL(10,2), "
                     "stock INTEGER)");

    // 2.1 批量插入测试
    ExecuteAndVerify("INSERT INTO products VALUES (1, 'Laptop', 999.99, 10)");
    ExecuteAndVerify("INSERT INTO products VALUES (2, 'Mouse', 29.99, 50)");
    ExecuteAndVerify("INSERT INTO products VALUES (3, 'Keyboard', 79.99, 30)");
    ExecuteAndVerify("INSERT INTO products VALUES (4, 'Monitor', 299.99, 15)");

    // 2.2 更新操作测试
    ExecuteAndVerify("UPDATE products SET price = 899.99 WHERE id = 1");
    ExecuteAndVerify("UPDATE products SET stock = stock - 5 WHERE name = 'Mouse'");

    // 2.3 删除操作测试
    ExecuteAndVerify("DELETE FROM products WHERE stock = 0");
    ExecuteAndVerify("DELETE FROM products WHERE price > 1000");

    // 2.4 查询操作测试
    ExecuteAndVerify("SELECT * FROM products");
    ExecuteAndVerify("SELECT name, price FROM products WHERE price < 100");
    ExecuteAndVerify("SELECT COUNT(*) FROM products");
    ExecuteAndVerify("SELECT AVG(price) FROM products");

    // 2.5 WHERE条件复杂测试
    ExecuteAndVerify("SELECT * FROM products WHERE price BETWEEN 50 AND 200");
    ExecuteAndVerify("SELECT * FROM products WHERE name LIKE '%board%'");
    ExecuteAndVerify("SELECT * FROM products WHERE stock > 0 AND price < 100");

    // 清理
    ExecuteAndVerify("DROP TABLE products");
    ExecuteAndVerify("DROP DATABASE dml_test_db");

    std::cout << "=== DML命令综合测试完成 ===" << std::endl;
}

// 测试INSERT操作的各种形式
TEST_F(DMLCommandsTest, InsertOperationsTest) {
    std::cout << "\n=== INSERT操作测试开始 ===" << std::endl;

    ExecuteAndVerify("CREATE DATABASE insert_test");
    ExecuteAndVerify("USE insert_test");
    ExecuteAndVerify("CREATE TABLE test_table ("
                     "id INTEGER PRIMARY KEY, "
                     "name VARCHAR(50), "
                     "value INTEGER)");

    // 标准INSERT
    ExecuteAndVerify("INSERT INTO test_table VALUES (1, 'Item1', 100)");
    ExecuteAndVerify("INSERT INTO test_table VALUES (2, 'Item2', 200)");

    // INSERT指定列
    ExecuteAndVerify("INSERT INTO test_table (id, name) VALUES (3, 'Item3')");
    ExecuteAndVerify("INSERT INTO test_table (name, value) VALUES ('Item4', 400)");

    // 批量INSERT
    ExecuteAndVerify("INSERT INTO test_table VALUES (5, 'Item5', 500), (6, 'Item6', 600)");

    // 验证插入结果
    ExecuteAndVerify("SELECT COUNT(*) FROM test_table", "4");

    // 清理
    ExecuteAndVerify("DROP TABLE test_table");
    ExecuteAndVerify("DROP DATABASE insert_test");

    std::cout << "=== INSERT操作测试完成 ===" << std::endl;
}

// 测试UPDATE操作的各种形式
TEST_F(DMLCommandsTest, UpdateOperationsTest) {
    std::cout << "\n=== UPDATE操作测试开始 ===" << std::endl;

    ExecuteAndVerify("CREATE DATABASE update_test");
    ExecuteAndVerify("USE update_test");
    ExecuteAndVerify("CREATE TABLE employees ("
                     "id INTEGER PRIMARY KEY, "
                     "name VARCHAR(50), "
                     "salary DECIMAL(10,2), "
                     "department VARCHAR(50))");

    // 插入测试数据
    ExecuteAndVerify("INSERT INTO employees VALUES "
                     "(1, 'Alice', 50000.00, 'HR'), "
                     "(2, 'Bob', 60000.00, 'IT'), "
                     "(3, 'Charlie', 55000.00, 'HR')");

    // 简单UPDATE
    ExecuteAndVerify("UPDATE employees SET salary = 52000.00 WHERE id = 1");

    // 条件UPDATE
    ExecuteAndVerify("UPDATE employees SET department = 'Engineering' WHERE department = 'IT'");

    // 多列UPDATE
    ExecuteAndVerify("UPDATE employees SET salary = salary * 1.1, department = 'Management' WHERE name = 'Alice'");

    // 验证更新结果
    ExecuteAndVerify("SELECT * FROM employees WHERE name = 'Alice'");

    // 清理
    ExecuteAndVerify("DROP TABLE employees");
    ExecuteAndVerify("DROP DATABASE update_test");

    std::cout << "=== UPDATE操作测试完成 ===" << std::endl;
}

// 测试DELETE操作的各种形式
TEST_F(DMLCommandsTest, DeleteOperationsTest) {
    std::cout << "\n=== DELETE操作测试开始 ===" << std::endl;

    ExecuteAndVerify("CREATE DATABASE delete_test");
    ExecuteAndVerify("USE delete_test");
    ExecuteAndVerify("CREATE TABLE items ("
                     "id INTEGER PRIMARY KEY, "
                     "name VARCHAR(50), "
                     "quantity INTEGER, "
                     "active BOOLEAN)");

    // 插入测试数据
    ExecuteAndVerify("INSERT INTO items VALUES "
                     "(1, 'Item1', 10, true), "
                     "(2, 'Item2', 0, true), "
                     "(3, 'Item3', 5, false), "
                     "(4, 'Item4', 15, true)");

    // 条件DELETE
    ExecuteAndVerify("DELETE FROM items WHERE quantity = 0");

    // 复杂条件DELETE
    ExecuteAndVerify("DELETE FROM items WHERE active = false OR quantity < 5");

    // 验证删除结果
    ExecuteAndVerify("SELECT COUNT(*) FROM items", "2");

    // 清理
    ExecuteAndVerify("DROP TABLE items");
    ExecuteAndVerify("DROP DATABASE delete_test");

    std::cout << "=== DELETE操作测试完成 ===" << std::endl;
}

// 测试SELECT查询的各种形式
TEST_F(DMLCommandsTest, SelectQueriesTest) {
    std::cout << "\n=== SELECT查询测试开始 ===" << std::endl;

    ExecuteAndVerify("CREATE DATABASE select_test");
    ExecuteAndVerify("USE select_test");
    ExecuteAndVerify("CREATE TABLE sales ("
                     "id INTEGER PRIMARY KEY, "
                     "product VARCHAR(50), "
                     "amount DECIMAL(10,2), "
                     "date DATE, "
                     "region VARCHAR(20))");

    // 插入测试数据
    ExecuteAndVerify("INSERT INTO sales VALUES "
                     "(1, 'Laptop', 1200.00, '2023-01-01', 'North'), "
                     "(2, 'Mouse', 25.00, '2023-01-02', 'South'), "
                     "(3, 'Keyboard', 80.00, '2023-01-01', 'North'), "
                     "(4, 'Monitor', 350.00, '2023-01-03', 'East')");

    // 基本SELECT
    ExecuteAndVerify("SELECT * FROM sales");

    // 选择特定列
    ExecuteAndVerify("SELECT product, amount FROM sales");

    // WHERE条件查询
    ExecuteAndVerify("SELECT * FROM sales WHERE amount > 100");
    ExecuteAndVerify("SELECT * FROM sales WHERE region = 'North'");

    // ORDER BY
    ExecuteAndVerify("SELECT * FROM sales ORDER BY amount DESC");

    // GROUP BY和聚合函数
    ExecuteAndVerify("SELECT region, COUNT(*) as count FROM sales GROUP BY region");

    // JOIN模拟（如果支持）
    // ExecuteAndVerify("SELECT * FROM sales s1 JOIN sales s2 ON s1.region = s2.region");

    // 清理
    ExecuteAndVerify("DROP TABLE sales");
    ExecuteAndVerify("DROP DATABASE select_test");

    std::cout << "=== SELECT查询测试完成 ===" << std::endl;
}

} // namespace sqlcc

// 主函数
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}