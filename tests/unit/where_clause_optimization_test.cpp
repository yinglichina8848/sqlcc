#include <gtest/gtest.h>
#include "execution_engine.h"
#include "database_manager.h"
#include "sql_parser/parser.h"
#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

class WhereClauseOptimizationTest : public ::testing::Test {
protected:
    std::string test_dir = "./where_optimization_test";
    std::shared_ptr<sqlcc::DatabaseManager> db_manager;
    
    void SetUp() override {
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
        
        db_manager = std::make_shared<sqlcc::DatabaseManager>(test_dir);
        ASSERT_TRUE(db_manager->CreateDatabase("testdb"));
        ASSERT_TRUE(db_manager->UseDatabase("testdb"));
    }
    
    void TearDown() override {
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }
};

// 测试简单的WHERE条件（已实现）
TEST_F(WhereClauseOptimizationTest, SimpleWhereCondition) {
    std::vector<std::pair<std::string, std::string>> columns = {
        {"id", "INTEGER"},
        {"name", "VARCHAR"},
        {"age", "INTEGER"}
    };
    ASSERT_TRUE(db_manager->CreateTable("users", columns));
    
    sqlcc::DMLExecutor executor(db_manager);
    
    // 插入测试数据
    std::string insert1 = "INSERT INTO users (id, name, age) VALUES (1, 'Alice', 25);";
    sqlcc::sql_parser::Parser parser1(insert1);
    auto stmts1 = parser1.parseStatements();
    executor.execute(std::move(stmts1[0]));
    
    std::string insert2 = "INSERT INTO users (id, name, age) VALUES (2, 'Bob', 30);";
    sqlcc::sql_parser::Parser parser2(insert2);
    auto stmts2 = parser2.parseStatements();
    executor.execute(std::move(stmts2[0]));
    
    // 测试简单WHERE条件
    std::string update_sql = "UPDATE users SET age = 26 WHERE id = 1;";
    sqlcc::sql_parser::Parser parser3(update_sql);
    auto stmts3 = parser3.parseStatements();
    auto result = executor.execute(std::move(stmts3[0]));
    
    EXPECT_TRUE(result.success);
}

// 测试AND条件（待实现）
TEST_F(WhereClauseOptimizationTest, AndWhereCondition) {
    std::vector<std::pair<std::string, std::string>> columns = {
        {"id", "INTEGER"},
        {"name", "VARCHAR"},
        {"age", "INTEGER"}
    };
    ASSERT_TRUE(db_manager->CreateTable("products", columns));
    
    sqlcc::DMLExecutor executor(db_manager);
    
    // 插入测试数据
    std::string insert1 = "INSERT INTO products (id, name, age) VALUES (1, 'Product1', 100);";
    sqlcc::sql_parser::Parser parser1(insert1);
    auto stmts1 = parser1.parseStatements();
    executor.execute(std::move(stmts1[0]));
    
    std::string insert2 = "INSERT INTO products (id, name, age) VALUES (2, 'Product2', 200);";
    sqlcc::sql_parser::Parser parser2(insert2);
    auto stmts2 = parser2.parseStatements();
    executor.execute(std::move(stmts2[0]));
    
    // TODO: 测试AND条件
    // std::string update_sql = "UPDATE products SET name = 'UpdatedProduct' WHERE id = 1 AND age = 100;";
    // sqlcc::sql_parser::Parser parser3(update_sql);
    // auto stmts3 = parser3.parseStatements();
    // auto result = executor.execute(std::move(stmts3[0]));
    // EXPECT_TRUE(result.success);
}

// 测试OR条件（待实现）
TEST_F(WhereClauseOptimizationTest, OrWhereCondition) {
    std::vector<std::pair<std::string, std::string>> columns = {
        {"id", "INTEGER"},
        {"status", "VARCHAR"}
    };
    ASSERT_TRUE(db_manager->CreateTable("orders", columns));
    
    sqlcc::DMLExecutor executor(db_manager);
    
    // 插入测试数据
    std::string insert1 = "INSERT INTO orders (id, status) VALUES (1, 'pending');";
    sqlcc::sql_parser::Parser parser1(insert1);
    auto stmts1 = parser1.parseStatements();
    executor.execute(std::move(stmts1[0]));
    
    std::string insert2 = "INSERT INTO orders (id, status) VALUES (2, 'completed');";
    sqlcc::sql_parser::Parser parser2(insert2);
    auto stmts2 = parser2.parseStatements();
    executor.execute(std::move(stmts2[0]));
    
    // TODO: 测试OR条件
    // std::string delete_sql = "DELETE FROM orders WHERE status = 'pending' OR status = 'cancelled';";
    // sqlcc::sql_parser::Parser parser3(delete_sql);
    // auto stmts3 = parser3.parseStatements();
    // auto result = executor.execute(std::move(stmts3[0]));
    // EXPECT_TRUE(result.success);
}

// 测试IN操作符（待实现）
TEST_F(WhereClauseOptimizationTest, InClause) {
    std::vector<std::pair<std::string, std::string>> columns = {
        {"id", "INTEGER"},
        {"category", "VARCHAR"}
    };
    ASSERT_TRUE(db_manager->CreateTable("items", columns));
    
    sqlcc::DMLExecutor executor(db_manager);
    
    // 插入测试数据
    for (int i = 1; i <= 5; i++) {
        std::string sql = "INSERT INTO items (id, category) VALUES (" + std::to_string(i) + ", 'cat" + std::to_string(i % 2) + "');";
        sqlcc::sql_parser::Parser parser(sql);
        auto stmts = parser.parseStatements();
        executor.execute(std::move(stmts[0]));
    }
    
    // TODO: 测试IN操作符
    // std::string delete_sql = "DELETE FROM items WHERE id IN (1, 2, 3);";
    // sqlcc::sql_parser::Parser parser(delete_sql);
    // auto stmts = parser.parseStatements();
    // auto result = executor.execute(std::move(stmts[0]));
    // EXPECT_TRUE(result.success);
}

// 测试BETWEEN操作符（待实现）
TEST_F(WhereClauseOptimizationTest, BetweenClause) {
    std::vector<std::pair<std::string, std::string>> columns = {
        {"id", "INTEGER"},
        {"amount", "INTEGER"}
    };
    ASSERT_TRUE(db_manager->CreateTable("transactions", columns));
    
    sqlcc::DMLExecutor executor(db_manager);
    
    // 插入测试数据
    for (int i = 1; i <= 10; i++) {
        std::string sql = "INSERT INTO transactions (id, amount) VALUES (" + std::to_string(i) + ", " + std::to_string(i * 100) + ");";
        sqlcc::sql_parser::Parser parser(sql);
        auto stmts = parser.parseStatements();
        executor.execute(std::move(stmts[0]));
    }
    
    // TODO: 测试BETWEEN操作符
    // std::string select_sql = "SELECT * FROM transactions WHERE amount BETWEEN 300 AND 700;";
    // sqlcc::sql_parser::Parser parser(select_sql);
    // auto stmts = parser.parseStatements();
    // auto result = executor.execute(std::move(stmts[0]));
    // EXPECT_TRUE(result.success);
}

// 测试LIKE操作符（待实现）
TEST_F(WhereClauseOptimizationTest, LikeClause) {
    std::vector<std::pair<std::string, std::string>> columns = {
        {"id", "INTEGER"},
        {"email", "VARCHAR"}
    };
    ASSERT_TRUE(db_manager->CreateTable("users", columns));
    
    sqlcc::DMLExecutor executor(db_manager);
    
    // 插入测试数据
    std::string insert1 = "INSERT INTO users (id, email) VALUES (1, 'alice@example.com');";
    sqlcc::sql_parser::Parser parser1(insert1);
    auto stmts1 = parser1.parseStatements();
    executor.execute(std::move(stmts1[0]));
    
    std::string insert2 = "INSERT INTO users (id, email) VALUES (2, 'bob@example.com');";
    sqlcc::sql_parser::Parser parser2(insert2);
    auto stmts2 = parser2.parseStatements();
    executor.execute(std::move(stmts2[0]));
    
    // TODO: 测试LIKE操作符
    // std::string select_sql = "SELECT * FROM users WHERE email LIKE '%@example.com%';";
    // sqlcc::sql_parser::Parser parser3(select_sql);
    // auto stmts3 = parser3.parseStatements();
    // auto result = executor.execute(std::move(stmts3[0]));
    // EXPECT_TRUE(result.success);
}

// 测试复杂的嵌套条件（待实现）
TEST_F(WhereClauseOptimizationTest, ComplexNestedConditions) {
    std::vector<std::pair<std::string, std::string>> columns = {
        {"id", "INTEGER"},
        {"name", "VARCHAR"},
        {"age", "INTEGER"},
        {"department", "VARCHAR"}
    };
    ASSERT_TRUE(db_manager->CreateTable("employees", columns));
    
    sqlcc::DMLExecutor executor(db_manager);
    
    // 插入测试数据
    std::string insert1 = "INSERT INTO employees (id, name, age, department) VALUES (1, 'Alice', 25, 'IT');";
    sqlcc::sql_parser::Parser parser1(insert1);
    auto stmts1 = parser1.parseStatements();
    executor.execute(std::move(stmts1[0]));
    
    std::string insert2 = "INSERT INTO employees (id, name, age, department) VALUES (2, 'Bob', 35, 'HR');";
    sqlcc::sql_parser::Parser parser2(insert2);
    auto stmts2 = parser2.parseStatements();
    executor.execute(std::move(stmts2[0]));
    
    // TODO: 测试复杂条件
    // std::string select_sql = "SELECT * FROM employees WHERE (age >= 25 AND department = 'IT') OR (age > 30 AND department = 'HR');";
    // sqlcc::sql_parser::Parser parser3(select_sql);
    // auto stmts3 = parser3.parseStatements();
    // auto result = executor.execute(std::move(stmts3[0]));
    // EXPECT_TRUE(result.success);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
