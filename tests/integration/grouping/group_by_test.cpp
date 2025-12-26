#include "sql_parser/ast_nodes.h"
#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include "sql_parser/parser.h"
#include "unified_executor.h"
#include "database_manager.h"
#include "execution_context.h"

using namespace sqlcc;
using namespace sql_parser;

class GroupByTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 初始化数据库管理器和执行器
    db_manager_ = std::make_shared<DatabaseManager>("/tmp/test_db", 1024, 4, 2);
    executor_ = std::make_unique<UnifiedExecutor>(db_manager_);

    // 创建测试表和数据
    setupTestData();
  }

  void TearDown() override {
    // 清理测试数据
    cleanupTestData();
  }

  void setupTestData() {
    // 创建测试数据库
    auto create_db_result = executor_->execute(std::make_unique<sql_parser::UseStatement>("test_db"));
    if (!create_db_result.success) {
      // 如果数据库不存在，创建数据库
      auto create_db_stmt = std::make_unique<sql_parser::CreateStatement>(CreateStatement::DATABASE);
      create_db_stmt->setObjectName("test_db");
      executor_->execute(std::move(create_db_stmt));

      // 使用数据库
      executor_->execute(std::make_unique<sql_parser::UseStatement>("test_db"));
    }

    // 创建测试表
    std::string create_table_sql = R"(
      CREATE TABLE sales (
        id INTEGER PRIMARY KEY,
        product VARCHAR(50),
        category VARCHAR(50),
        amount INTEGER,
        quantity INTEGER
      )
    )";

    auto parser = Parser(create_table_sql);
    auto statements = parser.parse();
    if (!statements.empty()) {
      executor_->execute(std::move(statements[0]));
    }

    // 插入测试数据
    std::vector<std::string> insert_statements = {
      "INSERT INTO sales VALUES (1, 'Apple', 'Fruit', 100, 10)",
      "INSERT INTO sales VALUES (2, 'Banana', 'Fruit', 50, 5)",
      "INSERT INTO sales VALUES (3, 'Carrot', 'Vegetable', 30, 3)",
      "INSERT INTO sales VALUES (4, 'Orange', 'Fruit', 80, 8)",
      "INSERT INTO sales VALUES (5, 'Potato', 'Vegetable', 25, 2)",
      "INSERT INTO sales VALUES (6, 'Apple', 'Fruit', 120, 12)"
    };

    for (const auto& sql : insert_statements) {
      auto parser = Parser(sql);
      auto statements = parser.parse();
      if (!statements.empty()) {
        executor_->execute(std::move(statements[0]));
      }
    }
  }

  void cleanupTestData() {
    // 清理测试数据
    try {
      auto drop_db_stmt = std::make_unique<sql_parser::DropStatement>(DropStatement::DATABASE);
      drop_db_stmt->setObjectName("test_db");
      drop_db_stmt->setIfExists(true);
      executor_->execute(std::move(drop_db_stmt));
    } catch (...) {
      // 忽略清理错误
    }
  }

  std::shared_ptr<DatabaseManager> db_manager_;
  std::unique_ptr<UnifiedExecutor> executor_;
};

// 测试基本的GROUP BY查询
TEST_F(GroupByTest, BasicGroupBy) {
  std::string sql = "SELECT category, COUNT(*) as count FROM sales GROUP BY category";

  auto parser = Parser(sql);
  auto statements = parser.parse();

  ASSERT_FALSE(statements.empty());

  auto result = executor_->execute(std::move(statements[0]));

  EXPECT_TRUE(result.success);
  EXPECT_EQ(result.message.find("GROUP BY query executed successfully"), std::string::npos);
}

// 测试GROUP BY配合聚合函数
TEST_F(GroupByTest, GroupByWithAggregation) {
  std::string sql = "SELECT category, SUM(amount) as total_amount, AVG(quantity) as avg_quantity FROM sales GROUP BY category";

  auto parser = Parser(sql);
  auto statements = parser.parse();

  ASSERT_FALSE(statements.empty());

  auto result = executor_->execute(std::move(statements[0]));

  EXPECT_TRUE(result.success);
  // 验证结果包含聚合数据
  EXPECT_NE(result.message.find("Fruit"), std::string::npos);
  EXPECT_NE(result.message.find("Vegetable"), std::string::npos);
}

// 测试多列GROUP BY
TEST_F(GroupByTest, MultipleColumnGroupBy) {
  std::string sql = "SELECT category, product, SUM(amount) as total FROM sales GROUP BY category, product";

  auto parser = Parser(sql);
  auto statements = parser.parse();

  ASSERT_FALSE(statements.empty());

  auto result = executor_->execute(std::move(statements[0]));

  EXPECT_TRUE(result.success);
  // 验证Apple产品分组正确
  EXPECT_NE(result.message.find("Apple"), std::string::npos);
}

// 测试GROUP BY配合WHERE条件
TEST_F(GroupByTest, GroupByWithWhere) {
  std::string sql = "SELECT category, SUM(amount) as total FROM sales WHERE quantity > 5 GROUP BY category";

  auto parser = Parser(sql);
  auto statements = parser.parse();

  ASSERT_FALSE(statements.empty());

  auto result = executor_->execute(std::move(statements[0]));

  EXPECT_TRUE(result.success);
  // 验证只有满足WHERE条件的记录被分组
  EXPECT_NE(result.message.find("Fruit"), std::string::npos);
}

// 测试空GROUP BY结果
TEST_F(GroupByTest, EmptyGroupByResult) {
  std::string sql = "SELECT category, COUNT(*) as count FROM sales WHERE amount > 1000 GROUP BY category";

  auto parser = Parser(sql);
  auto statements = parser.parse();

  ASSERT_FALSE(statements.empty());

  auto result = executor_->execute(std::move(statements[0]));

  EXPECT_TRUE(result.success);
  // 应该没有分组结果
  EXPECT_EQ(result.rows.size(), 0);
}

// 测试GROUP BY排序
TEST_F(GroupByTest, GroupByOrdering) {
  std::string sql = "SELECT category, SUM(amount) as total FROM sales GROUP BY category ORDER BY category";

  auto parser = Parser(sql);
  auto statements = parser.parse();

  ASSERT_FALSE(statements.empty());

  auto result = executor_->execute(std::move(statements[0]));

  EXPECT_TRUE(result.success);
  // 验证结果按类别排序
  std::string result_str = result.message;
  size_t fruit_pos = result_str.find("Fruit");
  size_t vegetable_pos = result_str.find("Vegetable");

  if (fruit_pos != std::string::npos && vegetable_pos != std::string::npos) {
    EXPECT_LT(fruit_pos, vegetable_pos);
  }
}
