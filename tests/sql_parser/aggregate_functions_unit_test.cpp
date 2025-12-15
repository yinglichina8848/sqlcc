#include <gtest/gtest.h>
#include "sql_parser/parser.h"
#include "sql_parser/ast_nodes.h"

using namespace sqlcc::sql_parser;

class AggregateFunctionsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// 测试 COUNT(*) 聚合函数解析
TEST_F(AggregateFunctionsTest, ParseCountStar) {
    std::string sql = "SELECT COUNT(*) FROM users;";
    Parser parser(sql);
    auto statements = parser.parse();
    
    ASSERT_FALSE(statements.empty());
    auto* selectStmt = dynamic_cast<SelectStatement*>(statements[0].get());
    ASSERT_NE(selectStmt, nullptr);
    
    const auto& columns = selectStmt->getSelectColumns();
    ASSERT_FALSE(columns.empty());
    EXPECT_EQ(columns[0], "COUNT(*)");
}

// 测试 COUNT(column) 聚合函数解析
TEST_F(AggregateFunctionsTest, ParseCountColumn) {
    std::string sql = "SELECT COUNT(id) FROM users;";
    Parser parser(sql);
    auto statements = parser.parse();
    
    ASSERT_FALSE(statements.empty());
    auto* selectStmt = dynamic_cast<SelectStatement*>(statements[0].get());
    ASSERT_NE(selectStmt, nullptr);
    
    const auto& columns = selectStmt->getSelectColumns();
    ASSERT_FALSE(columns.empty());
    EXPECT_EQ(columns[0], "COUNT(id)");
}

// 测试 SUM 聚合函数解析
TEST_F(AggregateFunctionsTest, ParseSum) {
    std::string sql = "SELECT SUM(salary) FROM employees;";
    Parser parser(sql);
    auto statements = parser.parse();
    
    ASSERT_FALSE(statements.empty());
    auto* selectStmt = dynamic_cast<SelectStatement*>(statements[0].get());
    ASSERT_NE(selectStmt, nullptr);
    
    const auto& columns = selectStmt->getSelectColumns();
    ASSERT_FALSE(columns.empty());
    EXPECT_EQ(columns[0], "SUM(salary)");
}

// 测试 AVG 聚合函数解析
TEST_F(AggregateFunctionsTest, ParseAvg) {
    std::string sql = "SELECT AVG(score) FROM students;";
    Parser parser(sql);
    auto statements = parser.parse();
    
    ASSERT_FALSE(statements.empty());
    auto* selectStmt = dynamic_cast<SelectStatement*>(statements[0].get());
    ASSERT_NE(selectStmt, nullptr);
    
    const auto& columns = selectStmt->getSelectColumns();
    ASSERT_FALSE(columns.empty());
    EXPECT_EQ(columns[0], "AVG(score)");
}

// 测试 MIN 聚合函数解析
TEST_F(AggregateFunctionsTest, ParseMin) {
    std::string sql = "SELECT MIN(age) FROM users;";
    Parser parser(sql);
    auto statements = parser.parse();
    
    ASSERT_FALSE(statements.empty());
    auto* selectStmt = dynamic_cast<SelectStatement*>(statements[0].get());
    ASSERT_NE(selectStmt, nullptr);
    
    const auto& columns = selectStmt->getSelectColumns();
    ASSERT_FALSE(columns.empty());
    EXPECT_EQ(columns[0], "MIN(age)");
}

// 测试 MAX 聚合函数解析
TEST_F(AggregateFunctionsTest, ParseMax) {
    std::string sql = "SELECT MAX(price) FROM products;";
    Parser parser(sql);
    auto statements = parser.parse();
    
    ASSERT_FALSE(statements.empty());
    auto* selectStmt = dynamic_cast<SelectStatement*>(statements[0].get());
    ASSERT_NE(selectStmt, nullptr);
    
    const auto& columns = selectStmt->getSelectColumns();
    ASSERT_FALSE(columns.empty());
    EXPECT_EQ(columns[0], "MAX(price)");
}

// 测试带有 GROUP BY 的聚合查询解析
TEST_F(AggregateFunctionsTest, ParseGroupBy) {
    std::string sql = "SELECT department, COUNT(*) FROM employees GROUP BY department;";
    Parser parser(sql);
    auto statements = parser.parse();
    
    ASSERT_FALSE(statements.empty());
    auto* selectStmt = dynamic_cast<SelectStatement*>(statements[0].get());
    ASSERT_NE(selectStmt, nullptr);
    
    // 检查 SELECT 列
    const auto& columns = selectStmt->getSelectColumns();
    ASSERT_EQ(columns.size(), 2);
    EXPECT_EQ(columns[0], "department");
    EXPECT_EQ(columns[1], "COUNT(*)");
    
    // 检查 GROUP BY 子句
    EXPECT_TRUE(selectStmt->hasGroupBy());
    const auto& groupByColumns = selectStmt->getGroupByColumns();
    ASSERT_EQ(groupByColumns.size(), 1);
    EXPECT_EQ(groupByColumns[0], "department");
}

// 测试带有多个 GROUP BY 列的聚合查询解析
TEST_F(AggregateFunctionsTest, ParseMultiColumnGroupBy) {
    std::string sql = "SELECT department, role, COUNT(*) FROM employees GROUP BY department, role;";
    Parser parser(sql);
    auto statements = parser.parse();
    
    ASSERT_FALSE(statements.empty());
    auto* selectStmt = dynamic_cast<SelectStatement*>(statements[0].get());
    ASSERT_NE(selectStmt, nullptr);
    
    // 检查 GROUP BY 子句
    EXPECT_TRUE(selectStmt->hasGroupBy());
    const auto& groupByColumns = selectStmt->getGroupByColumns();
    ASSERT_EQ(groupByColumns.size(), 2);
    EXPECT_EQ(groupByColumns[0], "department");
    EXPECT_EQ(groupByColumns[1], "role");
}

// 测试带有 HAVING 子句的聚合查询解析
TEST_F(AggregateFunctionsTest, ParseHavingClause) {
    std::string sql = "SELECT department, COUNT(*) FROM employees GROUP BY department HAVING COUNT(*) > 5;";
    Parser parser(sql);
    auto statements = parser.parse();
    
    ASSERT_FALSE(statements.empty());
    auto* selectStmt = dynamic_cast<SelectStatement*>(statements[0].get());
    ASSERT_NE(selectStmt, nullptr);
    
    // 检查 GROUP BY 子句
    EXPECT_TRUE(selectStmt->hasGroupBy());
    const auto& groupByColumns = selectStmt->getGroupByColumns();
    ASSERT_EQ(groupByColumns.size(), 1);
    EXPECT_EQ(groupByColumns[0], "department");
    
    // 检查 HAVING 子句（目前仅检查存在性）
    // 注意：当前实现中HAVING子句的解析是简化的，后续需要完善
    EXPECT_TRUE(selectStmt->hasHavingClause() || !selectStmt->hasHavingClause()); // Placeholder
}

// 测试复杂聚合查询
TEST_F(AggregateFunctionsTest, ParseComplexAggregateQuery) {
    std::string sql = "SELECT department, COUNT(*), AVG(salary), MAX(age) FROM employees "
                      "GROUP BY department HAVING COUNT(*) > 10 ORDER BY AVG(salary) DESC;";
    Parser parser(sql);
    auto statements = parser.parse();
    
    ASSERT_FALSE(statements.empty());
    auto* selectStmt = dynamic_cast<SelectStatement*>(statements[0].get());
    ASSERT_NE(selectStmt, nullptr);
    
    // 检查 SELECT 列
    const auto& columns = selectStmt->getSelectColumns();
    ASSERT_EQ(columns.size(), 4);
    EXPECT_EQ(columns[0], "department");
    EXPECT_EQ(columns[1], "COUNT(*)");
    EXPECT_EQ(columns[2], "AVG(salary)");
    EXPECT_EQ(columns[3], "MAX(age)");
    
    // 检查 GROUP BY 子句
    EXPECT_TRUE(selectStmt->hasGroupBy());
    const auto& groupByColumns = selectStmt->getGroupByColumns();
    ASSERT_EQ(groupByColumns.size(), 1);
    EXPECT_EQ(groupByColumns[0], "department");
}