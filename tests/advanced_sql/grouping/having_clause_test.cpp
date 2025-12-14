#include "database_manager.h"
#include "sql_parser/parser.h"
#include "sql_parser/parser_new.h"
#include "system_database.h"
#include "unified_executor.h"
#include "user_manager.h"
#include <gtest/gtest.h>
#include <memory>

namespace sqlcc {
using sql_parser::Parser;

// 测试HAVING子句的功能
class HavingClauseTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 初始化数据库管理器
    db_manager_ = std::make_shared<DatabaseManager>();
    user_manager_ = std::make_shared<UserManager>();
    system_db_ = std::make_shared<SystemDatabase>(db_manager_);
    unified_executor_ = std::make_shared<UnifiedExecutor>(
        db_manager_, user_manager_, system_db_);

    // 初始化数据库
    db_manager_->Initialize();
  }

  void TearDown() override {
    // 清理资源
    unified_executor_.reset();
    system_db_.reset();
    user_manager_.reset();
    db_manager_.reset();
  }

  std::shared_ptr<DatabaseManager> db_manager_;
  std::shared_ptr<UserManager> user_manager_;
  std::shared_ptr<SystemDatabase> system_db_;
  std::shared_ptr<UnifiedExecutor> unified_executor_;
};

// 测试基本HAVING子句
TEST_F(HavingClauseTest, BasicHavingClauseTest) {
  // 创建测试表
  Parser create_table_parser(
      "CREATE TABLE test_table (id INTEGER PRIMARY KEY, name VARCHAR(50), "
      "age INTEGER, salary INTEGER, department VARCHAR(50));");
  auto create_table_stmts = create_table_parser.parse();
  if (!create_table_stmts.empty()) {
    unified_executor_->execute(std::move(create_table_stmts[0]));
  }

  // 插入测试数据
  Parser insert_parser(
      "INSERT INTO test_table (id, name, age, salary, department) VALUES (1, "
      "'Alice', 25, 50000, 'HR'), (2, 'Bob', 30, 60000, 'IT'), (3, "
      "'Charlie', 35, 70000, 'IT'), (4, 'David', 40, 80000, 'Finance'), (5, "
      "'Eve', 45, 90000, 'Finance');");
  auto insert_stmts = insert_parser.parse();
  if (!insert_stmts.empty()) {
    unified_executor_->execute(std::move(insert_stmts[0]));
  }

  // 测试基本的HAVING子句功能
  Parser select_parser("SELECT department, COUNT(*) as employee_count FROM "
                       "test_table GROUP BY department HAVING COUNT(*) > 1;");
  auto select_stmts = select_parser.parse();
  ASSERT_FALSE(select_stmts.empty());

  // 执行查询
  ExecutionResult result =
      unified_executor_->execute(std::move(select_stmts[0]));

  // 验证结果
  EXPECT_TRUE(result.success);
}

// 测试HAVING子句与GROUP BY结合
TEST_F(HavingClauseTest, HavingWithGroupByTest) {
  // 创建测试表
  Parser create_table_parser(
      "CREATE TABLE test_table2 (id INTEGER PRIMARY KEY, name VARCHAR(50), "
      "age INTEGER, salary INTEGER, department VARCHAR(50));");
  auto create_table_stmts = create_table_parser.parse();
  if (!create_table_stmts.empty()) {
    unified_executor_->execute(std::move(create_table_stmts[0]));
  }

  // 插入测试数据
  Parser insert_parser(
      "INSERT INTO test_table2 (id, name, age, salary, department) VALUES (1, "
      "'Alice', 25, 50000, 'HR'), (2, 'Bob', 30, 60000, 'IT'), (3, "
      "'Charlie', 35, 70000, 'IT'), (4, 'David', 40, 80000, 'Finance'), (5, "
      "'Eve', 45, 90000, 'Finance');");
  auto insert_stmts = insert_parser.parse();
  if (!insert_stmts.empty()) {
    unified_executor_->execute(std::move(insert_stmts[0]));
  }

  // 测试HAVING子句与GROUP BY结合使用
  Parser select_parser(
      "SELECT department, AVG(salary) as avg_salary FROM test_table2 GROUP BY "
      "department HAVING AVG(salary) > 60000;");
  auto select_stmts = select_parser.parse();
  ASSERT_FALSE(select_stmts.empty());

  // 执行查询
  ExecutionResult result =
      unified_executor_->execute(std::move(select_stmts[0]));

  // 验证结果
  EXPECT_TRUE(result.success);
}

// 测试HAVING子句与WHERE子句结合
TEST_F(HavingClauseTest, HavingWithWhereTest) {
  // 创建测试表
  Parser create_table_parser(
      "CREATE TABLE test_table3 (id INTEGER PRIMARY KEY, name VARCHAR(50), "
      "age INTEGER, salary INTEGER, department VARCHAR(50));");
  auto create_table_stmts = create_table_parser.parse();
  if (!create_table_stmts.empty()) {
    unified_executor_->execute(std::move(create_table_stmts[0]));
  }

  // 插入测试数据
  Parser insert_parser(
      "INSERT INTO test_table3 (id, name, age, salary, department) VALUES (1, "
      "'Alice', 25, 50000, 'HR'), (2, 'Bob', 30, 60000, 'IT'), (3, "
      "'Charlie', 35, 70000, 'IT'), (4, 'David', 40, 80000, 'Finance'), (5, "
      "'Eve', 45, 90000, 'Finance');");
  auto insert_stmts = insert_parser.parse();
  if (!insert_stmts.empty()) {
    unified_executor_->execute(std::move(insert_stmts[0]));
  }

  // 测试HAVING子句与WHERE子句结合使用
  Parser select_parser(
      "SELECT department, COUNT(*) as employee_count FROM test_table3 WHERE age "
      "> 30 GROUP BY department HAVING COUNT(*) > 1;");
  auto select_stmts = select_parser.parse();
  ASSERT_FALSE(select_stmts.empty());

  // 执行查询
  ExecutionResult result =
      unified_executor_->execute(std::move(select_stmts[0]));

  // 验证结果
  EXPECT_TRUE(result.success);
}

// 测试HAVING子句与聚合函数结合
TEST_F(HavingClauseTest, HavingWithAggregateFunctionsTest) {
  // 创建测试表
  Parser create_table_parser(
      "CREATE TABLE test_table4 (id INTEGER PRIMARY KEY, name VARCHAR(50), "
      "age INTEGER, salary INTEGER, department VARCHAR(50));");
  auto create_table_stmts = create_table_parser.parse();
  if (!create_table_stmts.empty()) {
    unified_executor_->execute(std::move(create_table_stmts[0]));
  }

  // 插入测试数据
  Parser insert_parser(
      "INSERT INTO test_table4 (id, name, age, salary, department) VALUES (1, "
      "'Alice', 25, 50000, 'HR'), (2, 'Bob', 30, 60000, 'IT'), (3, "
      "'Charlie', 35, 70000, 'IT'), (4, 'David', 40, 80000, 'Finance'), (5, "
      "'Eve', 45, 90000, 'Finance');");
  auto insert_stmts = insert_parser.parse();
  if (!insert_stmts.empty()) {
    unified_executor_->execute(std::move(insert_stmts[0]));
  }

  // SUM函数
  Parser sum_parser(
      "SELECT department, SUM(salary) as total_salary FROM test_table4 GROUP BY "
      "department HAVING SUM(salary) > 100000;");
  auto sum_stmts = sum_parser.parse();
  ASSERT_FALSE(sum_stmts.empty());
  ExecutionResult sum_result =
      unified_executor_->execute(std::move(sum_stmts[0]));
  EXPECT_TRUE(sum_result.success);

  // MIN函数
  Parser min_parser(
      "SELECT department, MIN(salary) as min_salary FROM test_table4 GROUP BY "
      "department HAVING MIN(salary) > 50000;");
  auto min_stmts = min_parser.parse();
  ASSERT_FALSE(min_stmts.empty());
  ExecutionResult min_result =
      unified_executor_->execute(std::move(min_stmts[0]));
  EXPECT_TRUE(min_result.success);

  // MAX函数
  Parser max_parser(
      "SELECT department, MAX(salary) as max_salary FROM test_table4 GROUP BY "
      "department HAVING MAX(salary) < 100000;");
  auto max_stmts = max_parser.parse();
  ASSERT_FALSE(max_stmts.empty());
  ExecutionResult max_result =
      unified_executor_->execute(std::move(max_stmts[0]));
  EXPECT_TRUE(max_result.success);
}

// 测试复杂条件的HAVING子句
TEST_F(HavingClauseTest, ComplexHavingConditionTest) {
  // 创建测试表
  Parser create_table_parser(
      "CREATE TABLE test_table5 (id INTEGER PRIMARY KEY, name VARCHAR(50), "
      "age INTEGER, salary INTEGER, department VARCHAR(50));");
  auto create_table_stmts = create_table_parser.parse();
  if (!create_table_stmts.empty()) {
    unified_executor_->execute(std::move(create_table_stmts[0]));
  }

  // 插入测试数据
  Parser insert_parser(
      "INSERT INTO test_table5 (id, name, age, salary, department) VALUES (1, "
      "'Alice', 25, 50000, 'HR'), (2, 'Bob', 30, 60000, 'IT'), (3, "
      "'Charlie', 35, 70000, 'IT'), (4, 'David', 40, 80000, 'Finance'), (5, "
      "'Eve', 45, 90000, 'Finance');");
  auto insert_stmts = insert_parser.parse();
  if (!insert_stmts.empty()) {
    unified_executor_->execute(std::move(insert_stmts[0]));
  }

  // 测试复杂条件的HAVING子句
  Parser complex_parser(
      "SELECT department, AVG(salary) as avg_salary, COUNT(*) as "
      "employee_count FROM test_table5 GROUP BY department HAVING AVG(salary) > "
      "60000 AND COUNT(*) > 1;");
  auto complex_stmts = complex_parser.parse();
  ASSERT_FALSE(complex_stmts.empty());

  // 执行查询
  ExecutionResult result =
      unified_executor_->execute(std::move(complex_stmts[0]));

  // 验证结果
  EXPECT_TRUE(result.success);
}

} // namespace sqlcc