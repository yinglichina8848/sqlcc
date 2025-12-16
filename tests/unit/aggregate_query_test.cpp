#include "database_manager.h"
#include "sql_parser/parser.h"
#include "system_database.h"
#include "unified_executor.h"
#include "user_manager.h"
#include <gtest/gtest.h>
#include <memory>

namespace sqlcc {
using sql_parser::Parser;

// 测试聚合查询功能的调试测试
class AggregateQueryDebugTest : public ::testing::Test {
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

// 测试AggregateEngine类的基本功能
TEST_F(AggregateQueryDebugTest, AggregateEngineBasicTest) {
  // 创建测试表
  Parser create_table_parser(
      "CREATE TABLE agg_test (id INTEGER PRIMARY KEY, name VARCHAR(50), "
      "salary INTEGER, department VARCHAR(50));");
  auto create_table_stmts = create_table_parser.parse();
  if (!create_table_stmts.empty()) {
    unified_executor_->execute(std::move(create_table_stmts[0]));
  }

  // 插入测试数据
  Parser insert_parser(
      "INSERT INTO agg_test (id, name, salary, department) VALUES "
      "(1, 'Alice', 50000, 'HR'), "
      "(2, 'Bob', 60000, 'IT'), "
      "(3, 'Charlie', 70000, 'IT'), "
      "(4, 'David', 80000, 'Finance'), "
      "(5, 'Eve', 90000, 'Finance');");
  auto insert_stmts = insert_parser.parse();
  if (!insert_stmts.empty()) {
    unified_executor_->execute(std::move(insert_stmts[0]));
  }

  // 测试COUNT聚合函数
  Parser count_parser("SELECT COUNT(*) FROM agg_test;");
  auto count_stmts = count_parser.parse();
  ASSERT_FALSE(count_stmts.empty());

  ExecutionResult count_result =
      unified_executor_->execute(std::move(count_stmts[0]));
  EXPECT_TRUE(count_result.success);
  std::cout << "[DEBUG] COUNT result: " << count_result.message << std::endl;

  // 测试SUM聚合函数
  Parser sum_parser("SELECT SUM(salary) FROM agg_test;");
  auto sum_stmts = sum_parser.parse();
  ASSERT_FALSE(sum_stmts.empty());

  ExecutionResult sum_result =
      unified_executor_->execute(std::move(sum_stmts[0]));
  EXPECT_TRUE(sum_result.success);
  std::cout << "[DEBUG] SUM result: " << sum_result.message << std::endl;

  // 测试AVG聚合函数
  Parser avg_parser("SELECT AVG(salary) FROM agg_test;");
  auto avg_stmts = avg_parser.parse();
  ASSERT_FALSE(avg_stmts.empty());

  ExecutionResult avg_result =
      unified_executor_->execute(std::move(avg_stmts[0]));
  EXPECT_TRUE(avg_result.success);
  std::cout << "[DEBUG] AVG result: " << avg_result.message << std::endl;

  // 测试MIN聚合函数
  Parser min_parser("SELECT MIN(salary) FROM agg_test;");
  auto min_stmts = min_parser.parse();
  ASSERT_FALSE(min_stmts.empty());

  ExecutionResult min_result =
      unified_executor_->execute(std::move(min_stmts[0]));
  EXPECT_TRUE(min_result.success);
  std::cout << "[DEBUG] MIN result: " << min_result.message << std::endl;

  // 测试MAX聚合函数
  Parser max_parser("SELECT MAX(salary) FROM agg_test;");
  auto max_stmts = max_parser.parse();
  ASSERT_FALSE(max_stmts.empty());

  ExecutionResult max_result =
      unified_executor_->execute(std::move(max_stmts[0]));
  EXPECT_TRUE(max_result.success);
  std::cout << "[DEBUG] MAX result: " << max_result.message << std::endl;
}

// 测试GROUP BY功能
TEST_F(AggregateQueryDebugTest, GroupByTest) {
  // 创建测试表
  Parser create_table_parser(
      "CREATE TABLE group_test (id INTEGER PRIMARY KEY, name VARCHAR(50), "
      "salary INTEGER, department VARCHAR(50));");
  auto create_table_stmts = create_table_parser.parse();
  if (!create_table_stmts.empty()) {
    unified_executor_->execute(std::move(create_table_stmts[0]));
  }

  // 插入测试数据
  Parser insert_parser(
      "INSERT INTO group_test (id, name, salary, department) VALUES "
      "(1, 'Alice', 50000, 'HR'), "
      "(2, 'Bob', 60000, 'IT'), "
      "(3, 'Charlie', 70000, 'IT'), "
      "(4, 'David', 80000, 'Finance'), "
      "(5, 'Eve', 90000, 'Finance');");
  auto insert_stmts = insert_parser.parse();
  if (!insert_stmts.empty()) {
    unified_executor_->execute(std::move(insert_stmts[0]));
  }

  // 测试GROUP BY查询
  Parser group_parser("SELECT department, COUNT(*) FROM group_test GROUP BY department;");
  auto group_stmts = group_parser.parse();
  ASSERT_FALSE(group_stmts.empty());

  ExecutionResult group_result =
      unified_executor_->execute(std::move(group_stmts[0]));
  EXPECT_TRUE(group_result.success);
  std::cout << "[DEBUG] GROUP BY result: " << group_result.message << std::endl;
}

// 测试HAVING子句
TEST_F(AggregateQueryDebugTest, HavingClauseTest) {
  // 创建测试表
  Parser create_table_parser(
      "CREATE TABLE having_test (id INTEGER PRIMARY KEY, name VARCHAR(50), "
      "salary INTEGER, department VARCHAR(50));");
  auto create_table_stmts = create_table_parser.parse();
  if (!create_table_stmts.empty()) {
    unified_executor_->execute(std::move(create_table_stmts[0]));
  }

  // 插入测试数据
  Parser insert_parser(
      "INSERT INTO having_test (id, name, salary, department) VALUES "
      "(1, 'Alice', 50000, 'HR'), "
      "(2, 'Bob', 60000, 'IT'), "
      "(3, 'Charlie', 70000, 'IT'), "
      "(4, 'David', 80000, 'Finance'), "
      "(5, 'Eve', 90000, 'Finance');");
  auto insert_stmts = insert_parser.parse();
  if (!insert_stmts.empty()) {
    unified_executor_->execute(std::move(insert_stmts[0]));
  }

  // 测试HAVING子句（这里只是解析测试，实际执行可能不完整）
  Parser having_parser("SELECT department, COUNT(*) FROM having_test GROUP BY department HAVING COUNT(*) > 1;");
  auto having_stmts = having_parser.parse();
  ASSERT_FALSE(having_stmts.empty());

  ExecutionResult having_result =
      unified_executor_->execute(std::move(having_stmts[0]));
  EXPECT_TRUE(having_result.success);
  std::cout << "[DEBUG] HAVING result: " << having_result.message << std::endl;
}

} // namespace sqlcc
