#include "database_manager.h"
#include "execution/join_executor.h"
#include "sql_executor.h"
#include <gtest/gtest.h>
#include <memory>

// JOIN执行器测试 - 测试各种JOIN操作

class JoinExecutorTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 创建测试数据库管理器
    db_manager_ = std::make_shared<sqlcc::DatabaseManager>();

    // 创建测试SQL执行器
    sql_executor_ = std::make_shared<sqlcc::SqlExecutor>(db_manager_);

    // 创建JOIN执行器
    join_executor_ = std::make_shared<sqlcc::JoinExecutor>(sql_executor_);
  }

  std::shared_ptr<sqlcc::DatabaseManager> db_manager_;
  std::shared_ptr<sqlcc::SqlExecutor> sql_executor_;
  std::shared_ptr<sqlcc::JoinExecutor> join_executor_;
};

// 创建测试用的左表结果
static sqlcc::ExecutionResult create_test_left_table() {
  sqlcc::ExecutionResult result;

  // 添加列元数据
  result.column_metadata.push_back({"id", "INT", false, true, false, ""});
  result.column_metadata.push_back(
      {"name", "VARCHAR(50)", false, false, false, ""});
  result.column_metadata.push_back(
      {"department_id", "INT", false, false, false, ""});

  // 添加测试数据
  // 行1: id=1, name=John, department_id=10
  sqlcc::Row row1;
  row1.values.push_back(
      sqlcc::Value(static_cast<int64_t>(1))); // 使用显式转换避免歧义
  row1.values.push_back(sqlcc::Value(std::string("John")));
  row1.values.push_back(sqlcc::Value(static_cast<int64_t>(10)));
  result.rows.push_back(row1);

  // 行2: id=2, name=Jane, department_id=20
  sqlcc::Row row2;
  row2.values.push_back(sqlcc::Value(static_cast<int64_t>(2)));
  row2.values.push_back(sqlcc::Value(std::string("Jane")));
  row2.values.push_back(sqlcc::Value(static_cast<int64_t>(20)));
  result.rows.push_back(row2);

  // 行3: id=3, name=Bob, department_id=10
  sqlcc::Row row3;
  row3.values.push_back(sqlcc::Value(static_cast<int64_t>(3)));
  row3.values.push_back(sqlcc::Value(std::string("Bob")));
  row3.values.push_back(sqlcc::Value(static_cast<int64_t>(10)));
  result.rows.push_back(row3);

  result.success = true;
  result.message = "Test left table created successfully";

  return result;
}

// 创建测试用的右表结果
static sqlcc::ExecutionResult create_test_right_table() {
  sqlcc::ExecutionResult result;

  // 添加列元数据
  result.column_metadata.push_back(
      {"department_id", "INT", false, true, false, ""});
  result.column_metadata.push_back(
      {"department_name", "VARCHAR(50)", false, false, false, ""});
  result.column_metadata.push_back(
      {"location", "VARCHAR(50)", false, false, false, ""});

  // 添加测试数据
  // 行1: department_id=10, department_name=Sales, location=New York
  sqlcc::Row row1;
  row1.values.push_back(
      sqlcc::Value(static_cast<int64_t>(10))); // 使用显式转换避免歧义
  row1.values.push_back(sqlcc::Value(std::string("Sales")));
  row1.values.push_back(sqlcc::Value(std::string("New York")));
  result.rows.push_back(row1);

  // 行2: department_id=20, department_name=HR, location=London
  sqlcc::Row row2;
  row2.values.push_back(sqlcc::Value(static_cast<int64_t>(20)));
  row2.values.push_back(sqlcc::Value(std::string("HR")));
  row2.values.push_back(sqlcc::Value(std::string("London")));
  result.rows.push_back(row2);

  // 行3: department_id=30, department_name=IT, location=Tokyo
  sqlcc::Row row3;
  row3.values.push_back(sqlcc::Value(static_cast<int64_t>(30)));
  row3.values.push_back(sqlcc::Value(std::string("IT")));
  row3.values.push_back(sqlcc::Value(std::string("Tokyo")));
  result.rows.push_back(row3);

  result.success = true;
  result.message = "Test right table created successfully";

  return result;
}

// 测试INNER JOIN操作
TEST_F(JoinExecutorTest, InnerJoinOperation) {
  // 验证JoinExecutor可以成功构造
  EXPECT_NE(join_executor_, nullptr);

  // 创建测试数据
  sqlcc::ExecutionResult left_table = create_test_left_table();
  sqlcc::ExecutionResult right_table = create_test_right_table();

  // 执行INNER JOIN
  sqlcc::ExecutionResult result = join_executor_->execute(
      left_table, right_table, sqlcc::JoinType::INNER_JOIN,
      "department_id = department_id");

  // 验证结果
  EXPECT_TRUE(result.success);
  EXPECT_EQ(result.rows.size(), 3); // 应该有3行匹配

  // 获取执行统计信息
  auto stats = join_executor_->get_stats();
  EXPECT_EQ(stats.left_rows, 3);
  EXPECT_EQ(stats.right_rows, 3);
  EXPECT_EQ(stats.rows_processed, 9); // 3*3=9行处理
  EXPECT_EQ(stats.result_rows, 3);
  EXPECT_FALSE(stats.has_error);
}

// 测试LEFT JOIN操作
TEST_F(JoinExecutorTest, LeftJoinOperation) {
  // 验证JoinExecutor可以成功构造
  EXPECT_NE(join_executor_, nullptr);

  // 创建测试数据
  sqlcc::ExecutionResult left_table = create_test_left_table();
  sqlcc::ExecutionResult right_table = create_test_right_table();

  // 执行LEFT JOIN
  sqlcc::ExecutionResult result = join_executor_->execute(
      left_table, right_table, sqlcc::JoinType::LEFT_JOIN,
      "department_id = department_id");

  // 验证结果
  EXPECT_TRUE(result.success);
  EXPECT_EQ(result.rows.size(), 3); // 左表有3行，都应该出现在结果中

  // 获取执行统计信息
  auto stats = join_executor_->get_stats();
  EXPECT_EQ(stats.left_rows, 3);
  EXPECT_EQ(stats.right_rows, 3);
  EXPECT_EQ(stats.rows_processed, 9); // 3*3=9行处理
  EXPECT_EQ(stats.result_rows, 3);
  EXPECT_FALSE(stats.has_error);
}

// 测试RIGHT JOIN操作
TEST_F(JoinExecutorTest, RightJoinOperation) {
  // 验证JoinExecutor可以成功构造
  EXPECT_NE(join_executor_, nullptr);

  // 创建测试数据
  sqlcc::ExecutionResult left_table = create_test_left_table();
  sqlcc::ExecutionResult right_table = create_test_right_table();

  // 执行RIGHT JOIN
  sqlcc::ExecutionResult result = join_executor_->execute(
      left_table, right_table, sqlcc::JoinType::RIGHT_JOIN,
      "department_id = department_id");

  // 验证结果
  EXPECT_TRUE(result.success);
  EXPECT_EQ(result.rows.size(),
            4); // 右表有3行，其中1行没有匹配，所以应该有4行结果

  // 获取执行统计信息
  auto stats = join_executor_->get_stats();
  EXPECT_EQ(stats.left_rows, 3);
  EXPECT_EQ(stats.right_rows, 3);
  EXPECT_EQ(stats.rows_processed, 9); // 3*3=9行处理
  EXPECT_EQ(stats.result_rows, 4);
  EXPECT_FALSE(stats.has_error);
}

// 测试CROSS JOIN操作
TEST_F(JoinExecutorTest, CrossJoinOperation) {
  // 验证JoinExecutor可以成功构造
  EXPECT_NE(join_executor_, nullptr);

  // 创建测试数据
  sqlcc::ExecutionResult left_table = create_test_left_table();
  sqlcc::ExecutionResult right_table = create_test_right_table();

  // 执行CROSS JOIN（空条件）
  sqlcc::ExecutionResult result = join_executor_->execute(
      left_table, right_table, sqlcc::JoinType::CROSS_JOIN, "");

  // 验证结果
  EXPECT_TRUE(result.success);
  EXPECT_EQ(result.rows.size(), 9); // 3*3=9行交叉连接

  // 获取执行统计信息
  auto stats = join_executor_->get_stats();
  EXPECT_EQ(stats.left_rows, 3);
  EXPECT_EQ(stats.right_rows, 3);
  EXPECT_EQ(stats.rows_processed, 9); // 3*3=9行处理
  EXPECT_EQ(stats.result_rows, 9);
  EXPECT_FALSE(stats.has_error);
}

// 简单的JOIN执行器测试
TEST(JoinExecutorSimpleTest, BasicFunctionality) {
  // 测试JoinExecutor的基本功能
  // 验证JoinType枚举和JoinExecutionStats结构体
  EXPECT_EQ(static_cast<int>(sqlcc::JoinType::INNER_JOIN), 0);
  EXPECT_EQ(static_cast<int>(sqlcc::JoinType::LEFT_JOIN), 1);
  EXPECT_EQ(static_cast<int>(sqlcc::JoinType::RIGHT_JOIN), 2);
  EXPECT_EQ(static_cast<int>(sqlcc::JoinType::FULL_JOIN), 3);
  EXPECT_EQ(static_cast<int>(sqlcc::JoinType::CROSS_JOIN), 4);
  EXPECT_EQ(static_cast<int>(sqlcc::JoinType::NATURAL_JOIN), 5);

  // 测试JoinExecutionStats初始化
  sqlcc::JoinExecutionStats stats;
  EXPECT_EQ(stats.left_rows, 0);
  EXPECT_EQ(stats.right_rows, 0);
  EXPECT_EQ(stats.rows_processed, 0);
  EXPECT_EQ(stats.result_rows, 0);
  EXPECT_FALSE(stats.has_error);
  EXPECT_FALSE(stats.error_message.has_value());
}
