#include <gtest/gtest.h>
#include <sql_parser/parser_new.h>
#include <sql_parser/lexer_new.h>
#include <sql_parser/token_new.h>
#include <sql_parser/ast_nodes.h>
#include <iostream>
#include <memory>
#include <vector>

namespace sqlcc {
namespace sql_parser {
namespace test {

class ParserNewUnitTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每测试前初始化
    }
    
    void TearDown() override {
        // 每测试后清理
    }
};

// 测试基本语句解析
TEST_F(ParserNewUnitTest, BasicStatementParsing) {
    // 测试SELECT语句
    std::string sql = "SELECT id, name FROM users WHERE id = 1;";
    ParserNew parser(sql);
    
    auto statements = parser.parse();
    ASSERT_EQ(statements.size(), 1);
    EXPECT_NE(statements[0], nullptr);
}

// 测试DDL语句解析
TEST_F(ParserNewUnitTest, DDLStatementParsing) {
    // 测试CREATE TABLE语句
    std::string sql = "CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(50));";
    ParserNew parser(sql);
    
    auto statements = parser.parse();
    ASSERT_EQ(statements.size(), 1);
    EXPECT_NE(statements[0], nullptr);
    
    // 测试DROP TABLE语句
    sql = "DROP TABLE users;";
    ParserNew parser2(sql);
    statements = parser2.parse();
    ASSERT_EQ(statements.size(), 1);
    EXPECT_NE(statements[0], nullptr);
}

// 测试DML语句解析
TEST_F(ParserNewUnitTest, DMLStatementParsing) {
    // 测试INSERT语句
    std::string sql = "INSERT INTO users (id, name) VALUES (1, 'John');";
    ParserNew parser(sql);
    
    auto statements = parser.parse();
    ASSERT_EQ(statements.size(), 1);
    EXPECT_NE(statements[0], nullptr);
    
    // 测试UPDATE语句
    sql = "UPDATE users SET name = 'Jane' WHERE id = 1;";
    ParserNew parser2(sql);
    statements = parser2.parse();
    ASSERT_EQ(statements.size(), 1);
    EXPECT_NE(statements[0], nullptr);
    
    // 测试DELETE语句
    sql = "DELETE FROM users WHERE id = 1;";
    ParserNew parser3(sql);
    statements = parser3.parse();
    ASSERT_EQ(statements.size(), 1);
    EXPECT_NE(statements[0], nullptr);
}

// 测试错误处理
TEST_F(ParserNewUnitTest, ErrorHandling) {
    // 测试语法错误
    std::string sql = "SELECT FROM WHERE;";  // 不完整的语句
    ParserNew parser(sql);
    
    auto statements = parser.parse();
    // 应该返回空的statements，因为有语法错误
    EXPECT_TRUE(statements.empty());
}

// 测试多个语句
TEST_F(ParserNewUnitTest, MultipleStatements) {
    std::string sql = 
        "CREATE TABLE test (id INT);"
        "INSERT INTO test VALUES (1);"
        "SELECT * FROM test;";
    
    ParserNew parser(sql);
    auto statements = parser.parse();
    ASSERT_EQ(statements.size(), 3);
    
    for (const auto& stmt : statements) {
        EXPECT_NE(stmt, nullptr);
    }
}

// 测试复杂SELECT语句
TEST_F(ParserNewUnitTest, ComplexSelectStatement) {
    std::string sql = 
        "SELECT u.id, u.name, p.title "
        "FROM users u "
        "JOIN posts p ON u.id = p.user_id "
        "WHERE u.age > 18 "
        "ORDER BY u.name "
        "LIMIT 10;";
    
    ParserNew parser(sql);
    auto statements = parser.parse();
    ASSERT_EQ(statements.size(), 1);
    EXPECT_NE(statements[0], nullptr);
}

} // namespace test
} // namespace sql_parser
} // namespace sqlcc

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
