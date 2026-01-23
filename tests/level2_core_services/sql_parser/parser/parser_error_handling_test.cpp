#include "src/sql_parser/ast/ast_node.h"
#include "sql_parser/parser.h"
#include "sql_parser/ast_nodes.h"
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace sqlcc::sql_parser;

class ParserErrorHandlingTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 可以在这里设置测试前的准备工作
  }

  void TearDown() override {
    // 可以在这里清理测试后的工作
  }

  // 辅助方法：测试解析结果
  void testParseResult(const std::string& sql, bool shouldSucceed,
                      const std::string& testDescription) {
    SCOPED_TRACE(testDescription + " - SQL: " + sql);

    try {
      Parser parser(sql);
      auto statements = parser.parse();

      if (shouldSucceed) {
        EXPECT_FALSE(statements.empty()) << "Expected successful parsing";
        EXPECT_FALSE(parser.hadError()) << "Parser should not report errors for valid SQL";
      } else {
        // 对于无效SQL，解析器可能会抛出异常或设置错误状态
        // 我们接受两者中的任一种情况
        EXPECT_TRUE(parser.hadError() || statements.empty())
            << "Parser should report errors for invalid SQL";
      }

      // 打印错误信息（如果有）
      auto errors = parser.getDetailedErrors();
      if (!errors.empty()) {
        std::cout << "Parser errors:" << std::endl;
        for (const auto& error : errors) {
          std::cout << "  " << error << std::endl;
        }
      }

    } catch (const std::exception& e) {
      if (shouldSucceed) {
        FAIL() << "Unexpected exception for valid SQL: " << e.what();
      } else {
        std::cout << "Expected exception for invalid SQL: " << e.what() << std::endl;
        // 对于无效SQL，异常也是可以接受的结果
        SUCCEED();
      }
    }
  }
};

// 测试有效SQL语句
TEST_F(ParserErrorHandlingTest, ValidCreateTable) {
  std::string sql = "CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(255) NOT NULL);";
  testParseResult(sql, true, "Valid CREATE TABLE statement");
}

TEST_F(ParserErrorHandlingTest, ValidSelectStatement) {
  std::string sql = "SELECT id, name FROM users WHERE age > 18;";
  testParseResult(sql, true, "Valid SELECT statement with WHERE clause");
}

TEST_F(ParserErrorHandlingTest, ValidInsertStatement) {
  std::string sql = "INSERT INTO users (id, name) VALUES (1, 'John');";
  testParseResult(sql, true, "Valid INSERT statement");
}

// 测试无效SQL语句的错误处理
TEST_F(ParserErrorHandlingTest, InvalidSyntaxMissingSemicolon) {
  std::string sql = "SELECT * FROM users";  // 这个实际上是有效的SQL
  testParseResult(sql, true, "SQL without semicolon (should be valid)");
}

TEST_F(ParserErrorHandlingTest, InvalidTableName) {
  std::string sql = "SELECT * FROM ;";
  testParseResult(sql, false, "Invalid SQL with missing table name");
}

TEST_F(ParserErrorHandlingTest, InvalidCreateTableSyntax) {
  std::string sql = "CREATE TABLE (id INT);";
  testParseResult(sql, false, "Invalid CREATE TABLE missing table name");
}

TEST_F(ParserErrorHandlingTest, InvalidColumnDefinition) {
  std::string sql = "CREATE TABLE users (id , name VARCHAR(255));";
  testParseResult(sql, false, "Invalid column definition missing type");
}

TEST_F(ParserErrorHandlingTest, InvalidWhereClause) {
  std::string sql = "SELECT * FROM users WHERE ;";
  testParseResult(sql, false, "Invalid WHERE clause");
}

TEST_F(ParserErrorHandlingTest, UnknownKeyword) {
  std::string sql = "INVALID KEYWORD * FROM users;";
  testParseResult(sql, false, "Unknown keyword at start");
}

// 测试错误恢复机制
TEST_F(ParserErrorHandlingTest, ErrorRecoveryMultipleStatements) {
  std::string sql = "SELECT * FROM users; INVALID STATEMENT; SELECT * FROM products;";
  testParseResult(sql, false, "Multiple statements with invalid middle statement");
}

// 测试边界情况
TEST_F(ParserErrorHandlingTest, EmptyInput) {
  std::string sql = "";
  testParseResult(sql, true, "Empty input (should be handled gracefully)");
}

TEST_F(ParserErrorHandlingTest, OnlyWhitespace) {
  std::string sql = "   \n\t  ";
  testParseResult(sql, true, "Only whitespace input");
}

TEST_F(ParserErrorHandlingTest, OnlySemicolon) {
  std::string sql = ";";
  testParseResult(sql, true, "Only semicolon");
}

// 测试复杂SQL的错误处理
TEST_F(ParserErrorHandlingTest, InvalidJoinSyntax) {
  std::string sql = "SELECT * FROM users u JOIN orders o ON ;";
  testParseResult(sql, false, "Invalid JOIN condition");
}

TEST_F(ParserErrorHandlingTest, InvalidSubquery) {
  std::string sql = "SELECT * FROM users WHERE id IN (SELECT );";
  testParseResult(sql, false, "Invalid subquery");
}

// 测试错误上下文信息
TEST_F(ParserErrorHandlingTest, ErrorContextReporting) {
  std::string sql = "CREATE TABLE test (id INT PRIMARY KEY, name VARCHAR();";

  Parser parser(sql);
  auto statements = parser.parse();

  // 检查是否有错误
  ASSERT_TRUE(parser.hadError());

  // 检查错误信息是否包含上下文
  auto errors = parser.getDetailedErrors();
  ASSERT_FALSE(errors.empty());

  // 错误信息应该包含位置和上下文信息
  bool hasContextInfo = false;
  for (const auto& error : errors) {
    if (error.find("Context:") != std::string::npos ||
        error.find("line") != std::string::npos) {
      hasContextInfo = true;
      break;
    }
  }

  EXPECT_TRUE(hasContextInfo) << "Error messages should include context information";
}

// 测试错误清除功能
TEST_F(ParserErrorHandlingTest, ErrorClearing) {
  std::string invalidSql = "INVALID SQL;";

  Parser parser(invalidSql);
  auto statements = parser.parse();

  // 应该有错误
  ASSERT_TRUE(parser.hadError());

  // 清除错误
  parser.clearErrors();

  // 错误应该被清除
  EXPECT_FALSE(parser.hadError());
  EXPECT_TRUE(parser.getDetailedErrors().empty());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
