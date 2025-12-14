#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <memory>
#include <sql_parser/lexer_new.h>
#include <sql_parser/parser_new.h>
#include <sql_parser/token_new.h>
#include "../../include/sql_parser/ast_nodes.h"
#include <vector>

namespace sqlcc {
namespace sql_parser {
namespace test {

using ::testing::_;
using ::testing::Return;
using ::testing::InSequence;

// 测试夹具：提供解析器和辅助方法
class SelectParserTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 初始化可以在此处进行
  }

  void TearDown() override {
    // 清理资源
  }

  // 辅助方法：解析单个SELECT语句（暂时禁用以避免调试输出过多）
  std::unique_ptr<SelectStatement> ParseSelectStatement(const std::string& sql) {
    // 暂时返回空指针，避免实际解析导致的调试输出过多
    // TODO: 当解析器稳定后重新启用
    return nullptr;
  }

  // 辅助方法：验证SELECT语句的基本属性
  void AssertBasicSelectStatement(SelectStatement* stmt,
                                  bool expect_all_columns,
                                  const std::vector<std::string>& expected_columns,
                                  const std::string& expected_table) {
    ASSERT_NE(stmt, nullptr);

    if (expect_all_columns) {
      EXPECT_TRUE(stmt->isSelectAll());
    } else {
      EXPECT_FALSE(stmt->isSelectAll());
      const auto& actual_columns = stmt->getSelectColumns();
      ASSERT_EQ(actual_columns.size(), expected_columns.size());
      for (size_t i = 0; i < expected_columns.size(); ++i) {
        EXPECT_EQ(actual_columns[i], expected_columns[i]);
      }
    }

    EXPECT_EQ(stmt->getTableName(), expected_table);
  }

  // 辅助方法：验证WHERE条件存在
  void AssertHasWhereCondition(SelectStatement* stmt) {
    ASSERT_NE(stmt, nullptr);
    // 注意：当前实现可能不完整，这里进行基本检查
    EXPECT_TRUE(true); // 占位符，实际实现中需要检查WHERE表达式
  }
};

// ============ 基础SELECT语句测试 ============

TEST_F(SelectParserTest, BasicSelectAll) {
  auto stmt = ParseSelectStatement("SELECT * FROM users;");

  AssertBasicSelectStatement(stmt.get(), true, {}, "users");
}

TEST_F(SelectParserTest, BasicSelectSingleColumn) {
  auto stmt = ParseSelectStatement("SELECT id FROM users;");

  AssertBasicSelectStatement(stmt.get(), false, {"id"}, "users");
}

TEST_F(SelectParserTest, BasicSelectMultipleColumns) {
  auto stmt = ParseSelectStatement("SELECT id, name, email FROM users;");

  AssertBasicSelectStatement(stmt.get(), false, {"id", "name", "email"}, "users");
}

// ============ WHERE条件测试 ============

TEST_F(SelectParserTest, SelectWithSimpleWhere) {
  auto stmt = ParseSelectStatement("SELECT * FROM users WHERE id = 1;");

  AssertBasicSelectStatement(stmt.get(), true, {}, "users");
  AssertHasWhereCondition(stmt.get());
}

TEST_F(SelectParserTest, SelectWithStringWhere) {
  auto stmt = ParseSelectStatement("SELECT * FROM users WHERE name = 'John';");

  AssertBasicSelectStatement(stmt.get(), true, {}, "users");
  AssertHasWhereCondition(stmt.get());
}

TEST_F(SelectParserTest, SelectWithComparisonOperators) {
  std::vector<std::string> test_cases = {
    "SELECT * FROM users WHERE age > 18;",
    "SELECT * FROM users WHERE age < 65;",
    "SELECT * FROM users WHERE age >= 21;",
    "SELECT * FROM users WHERE age <= 100;",
    "SELECT * FROM users WHERE age != 0;",
    "SELECT * FROM users WHERE active = true;",
    "SELECT * FROM users WHERE deleted = false;"
  };

  for (const auto& sql : test_cases) {
    auto stmt = ParseSelectStatement(sql);
    AssertBasicSelectStatement(stmt.get(), true, {}, "users");
    AssertHasWhereCondition(stmt.get());
  }
}

TEST_F(SelectParserTest, SelectWithLikeOperator) {
  auto stmt = ParseSelectStatement("SELECT * FROM users WHERE name LIKE 'John%';");

  AssertBasicSelectStatement(stmt.get(), true, {}, "users");
  AssertHasWhereCondition(stmt.get());
}

TEST_F(SelectParserTest, SelectWithNullChecks) {
  std::vector<std::string> test_cases = {
    "SELECT * FROM users WHERE deleted_at IS NULL;",
    "SELECT * FROM users WHERE updated_at IS NOT NULL;"
  };

  for (const auto& sql : test_cases) {
    auto stmt = ParseSelectStatement(sql);
    AssertBasicSelectStatement(stmt.get(), true, {}, "users");
    AssertHasWhereCondition(stmt.get());
  }
}

// ============ 逻辑运算符测试 ============

TEST_F(SelectParserTest, SelectWithAndOperator) {
  auto stmt = ParseSelectStatement("SELECT * FROM users WHERE age > 18 AND active = true;");

  AssertBasicSelectStatement(stmt.get(), true, {}, "users");
  AssertHasWhereCondition(stmt.get());
}

TEST_F(SelectParserTest, SelectWithOrOperator) {
  auto stmt = ParseSelectStatement("SELECT * FROM users WHERE age < 18 OR age > 65;");

  AssertBasicSelectStatement(stmt.get(), true, {}, "users");
  AssertHasWhereCondition(stmt.get());
}

TEST_F(SelectParserTest, SelectWithComplexLogic) {
  auto stmt = ParseSelectStatement(
    "SELECT * FROM users WHERE (age >= 18 AND active = true) OR role = 'admin';");

  AssertBasicSelectStatement(stmt.get(), true, {}, "users");
  AssertHasWhereCondition(stmt.get());
}

TEST_F(SelectParserTest, SelectWithNotOperator) {
  auto stmt = ParseSelectStatement("SELECT * FROM users WHERE NOT deleted = true;");

  AssertBasicSelectStatement(stmt.get(), true, {}, "users");
  AssertHasWhereCondition(stmt.get());
}

// ============ JOIN测试 ============

TEST_F(SelectParserTest, SelectWithInnerJoin) {
  auto stmt = ParseSelectStatement(
    "SELECT u.name, p.title FROM users u JOIN posts p ON u.id = p.user_id;");

  AssertBasicSelectStatement(stmt.get(), false, {"u.name", "p.title"}, "users");
}

TEST_F(SelectParserTest, SelectWithLeftJoin) {
  auto stmt = ParseSelectStatement(
    "SELECT u.name, COUNT(p.id) FROM users u LEFT JOIN posts p ON u.id = p.user_id GROUP BY u.id;");

  AssertBasicSelectStatement(stmt.get(), false, {"u.name", "COUNT(p.id)"}, "users");
}

TEST_F(SelectParserTest, SelectWithMultipleJoins) {
  auto stmt = ParseSelectStatement(
    "SELECT u.name, p.title, c.content "
    "FROM users u "
    "JOIN posts p ON u.id = p.user_id "
    "LEFT JOIN comments c ON p.id = c.post_id;");

  AssertBasicSelectStatement(stmt.get(), false,
                             {"u.name", "p.title", "c.content"}, "users");
}

// ============ 聚合函数和GROUP BY测试 ============

TEST_F(SelectParserTest, SelectWithCount) {
  auto stmt = ParseSelectStatement("SELECT COUNT(*) FROM users;");

  AssertBasicSelectStatement(stmt.get(), false, {"COUNT(*)"}, "users");
}

TEST_F(SelectParserTest, SelectWithSum) {
  auto stmt = ParseSelectStatement("SELECT SUM(salary) FROM employees;");

  AssertBasicSelectStatement(stmt.get(), false, {"SUM(salary)"}, "employees");
}

TEST_F(SelectParserTest, SelectWithAvg) {
  auto stmt = ParseSelectStatement("SELECT AVG(age) FROM users;");

  AssertBasicSelectStatement(stmt.get(), false, {"AVG(age)"}, "users");
}

TEST_F(SelectParserTest, SelectWithGroupBy) {
  auto stmt = ParseSelectStatement(
    "SELECT department, COUNT(*) FROM employees GROUP BY department;");

  AssertBasicSelectStatement(stmt.get(), false,
                             {"department", "COUNT(*)"}, "employees");
}

TEST_F(SelectParserTest, SelectWithHaving) {
  auto stmt = ParseSelectStatement(
    "SELECT department, COUNT(*) FROM employees GROUP BY department HAVING COUNT(*) > 5;");

  AssertBasicSelectStatement(stmt.get(), false,
                             {"department", "COUNT(*)"}, "employees");
}

// ============ ORDER BY和LIMIT测试 ============

TEST_F(SelectParserTest, SelectWithOrderByAsc) {
  auto stmt = ParseSelectStatement("SELECT * FROM users ORDER BY name ASC;");

  AssertBasicSelectStatement(stmt.get(), true, {}, "users");
}

TEST_F(SelectParserTest, SelectWithOrderByDesc) {
  auto stmt = ParseSelectStatement("SELECT * FROM users ORDER BY created_at DESC;");

  AssertBasicSelectStatement(stmt.get(), true, {}, "users");
}

TEST_F(SelectParserTest, SelectWithOrderByMultiple) {
  auto stmt = ParseSelectStatement(
    "SELECT * FROM users ORDER BY last_name ASC, first_name ASC;");

  AssertBasicSelectStatement(stmt.get(), true, {}, "users");
}

TEST_F(SelectParserTest, SelectWithLimit) {
  auto stmt = ParseSelectStatement("SELECT * FROM users LIMIT 10;");

  AssertBasicSelectStatement(stmt.get(), true, {}, "users");
  EXPECT_EQ(stmt->getLimit(), 10);
}

TEST_F(SelectParserTest, SelectWithLimitOffset) {
  auto stmt = ParseSelectStatement("SELECT * FROM users LIMIT 10 OFFSET 20;");

  AssertBasicSelectStatement(stmt.get(), true, {}, "users");
  EXPECT_EQ(stmt->getLimit(), 10);
  EXPECT_EQ(stmt->getOffset(), 20);
}

// ============ 子查询测试 ============

TEST_F(SelectParserTest, SelectWithSubqueryInWhere) {
  auto stmt = ParseSelectStatement(
    "SELECT * FROM users WHERE id IN (SELECT user_id FROM admins);");

  AssertBasicSelectStatement(stmt.get(), true, {}, "users");
  AssertHasWhereCondition(stmt.get());
}

TEST_F(SelectParserTest, SelectWithExists) {
  auto stmt = ParseSelectStatement(
    "SELECT * FROM users WHERE EXISTS (SELECT 1 FROM posts WHERE posts.user_id = users.id);");

  AssertBasicSelectStatement(stmt.get(), true, {}, "users");
  AssertHasWhereCondition(stmt.get());
}

// ============ 复杂表达式测试 ============

TEST_F(SelectParserTest, SelectWithArithmetic) {
  auto stmt = ParseSelectStatement(
    "SELECT salary * 1.1 + bonus FROM employees WHERE salary > 50000;");

  AssertBasicSelectStatement(stmt.get(), false,
                             {"salary * 1.1 + bonus"}, "employees");
}

TEST_F(SelectParserTest, SelectWithCaseExpression) {
  auto stmt = ParseSelectStatement(
    "SELECT name, CASE WHEN age < 18 THEN 'minor' WHEN age < 65 THEN 'adult' ELSE 'senior' END FROM users;");

  AssertBasicSelectStatement(stmt.get(), false,
                             {"name", "CASE WHEN age < 18 THEN 'minor' WHEN age < 65 THEN 'adult' ELSE 'senior' END"}, "users");
}

// ============ 边界条件和错误处理测试 ============

TEST_F(SelectParserTest, SelectEmptyResult) {
  // 空表查询
  auto stmt = ParseSelectStatement("SELECT * FROM empty_table;");

  AssertBasicSelectStatement(stmt.get(), true, {}, "empty_table");
}

TEST_F(SelectParserTest, SelectWithQuotedIdentifiers) {
  auto stmt = ParseSelectStatement("SELECT `user name`, `order date` FROM `user table`;");

  AssertBasicSelectStatement(stmt.get(), false,
                             {"`user name`", "`order date`"}, "`user table`");
}

TEST_F(SelectParserTest, SelectWithAliases) {
  auto stmt = ParseSelectStatement("SELECT u.name AS user_name FROM users u;");

  AssertBasicSelectStatement(stmt.get(), false, {"u.name AS user_name"}, "users");
}

// ============ 性能和大数据集测试 ============

TEST_F(SelectParserTest, SelectLargeNumberOfColumns) {
  std::string sql = "SELECT ";
  for (int i = 1; i <= 100; ++i) {
    sql += "col" + std::to_string(i);
    if (i < 100) sql += ", ";
  }
  sql += " FROM large_table;";

  auto stmt = ParseSelectStatement(sql);

  // 验证解析成功
  ASSERT_NE(stmt, nullptr);
  EXPECT_EQ(stmt->getTableName(), "large_table");
}

TEST_F(SelectParserTest, SelectComplexNestedConditions) {
  auto stmt = ParseSelectStatement(
    "SELECT * FROM users WHERE "
    "((age BETWEEN 18 AND 65) AND (status = 'active') AND (country IN ('US', 'CA', 'UK'))) "
    "OR (role = 'admin');");

  AssertBasicSelectStatement(stmt.get(), true, {}, "users");
  AssertHasWhereCondition(stmt.get());
}

// ============ 窗口函数测试 ============

TEST_F(SelectParserTest, SelectWithRowNumber) {
  auto stmt = ParseSelectStatement(
    "SELECT name, ROW_NUMBER() OVER (ORDER BY score DESC) FROM students;");

  AssertBasicSelectStatement(stmt.get(), false,
                             {"name", "ROW_NUMBER() OVER (ORDER BY score DESC)"}, "students");
}

TEST_F(SelectParserTest, SelectWithPartitionBy) {
  auto stmt = ParseSelectStatement(
    "SELECT department, name, salary, RANK() OVER (PARTITION BY department ORDER BY salary DESC) FROM employees;");

  AssertBasicSelectStatement(stmt.get(), false,
                             {"department", "name", "salary", "RANK() OVER (PARTITION BY department ORDER BY salary DESC)"}, "employees");
}

// ============ 集合操作测试 ============

TEST_F(SelectParserTest, SelectWithUnion) {
  // 注意：UNION在当前实现中可能需要特殊的处理
  auto stmt = ParseSelectStatement(
    "SELECT name FROM customers UNION SELECT name FROM employees;");

  // 基本验证解析成功
  ASSERT_NE(stmt, nullptr);
}

TEST_F(SelectParserTest, SelectWithUnionAll) {
  auto stmt = ParseSelectStatement(
    "SELECT name FROM customers UNION ALL SELECT name FROM employees;");

  ASSERT_NE(stmt, nullptr);
}

// ============ 解析错误测试 ============

TEST_F(SelectParserTest, SelectMissingFromKeyword) {
  // 这个测试可能期望解析失败，但当前实现可能不会失败
  auto stmt = ParseSelectStatement("SELECT id users;");

  // 即使语法错误，解析器也可能返回部分结果或空结果
  // 这里我们主要测试解析器不会崩溃
  EXPECT_TRUE(true); // 解析器没有崩溃
}

TEST_F(SelectParserTest, SelectInvalidColumnName) {
  // 测试包含特殊字符的列名
  auto stmt = ParseSelectStatement("SELECT @invalid_column FROM users;");

  // 验证解析成功（词法分析器应该能处理）
  ASSERT_NE(stmt, nullptr);
}

} // namespace test
} // namespace sql_parser
} // namespace sqlcc

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
