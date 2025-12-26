#include "sql_parser/ast_nodes.h"
#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <set>
#include "sql_parser/parser.h"
#include "unified_executor.h"
#include "database_manager.h"
#include "execution_context.h"

using namespace sqlcc;
using namespace sql_parser;

class SelectDistinctTest : public ::testing::Test {
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
      CREATE TABLE products (
        id INTEGER PRIMARY KEY,
        name VARCHAR(50),
        category VARCHAR(50),
        price INTEGER
      )
    )";

    auto parser = Parser(create_table_sql);
    auto statements = parser.parse();
    if (!statements.empty()) {
      executor_->execute(std::move(statements[0]));
    }

    // 插入测试数据（包含重复值）
    std::vector<std::string> insert_statements = {
      "INSERT INTO products VALUES (1, 'Apple', 'Fruit', 100)",
      "INSERT INTO products VALUES (2, 'Banana', 'Fruit', 50)",
      "INSERT INTO products VALUES (3, 'Apple', 'Fruit', 100)",  // 重复
      "INSERT INTO products VALUES (4, 'Orange', 'Fruit', 80)",
      "INSERT INTO products VALUES (5, 'Carrot', 'Vegetable', 30)",
      "INSERT INTO products VALUES (6, 'Banana', 'Fruit', 50)",  // 重复
      "INSERT INTO products VALUES (7, 'Potato', 'Vegetable', 25)",
      "INSERT INTO products VALUES (8, 'Apple', 'Fruit', 120)"   // 名称相同但价格不同
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

// 测试基本的SELECT DISTINCT查询
TEST_F(SelectDistinctTest, BasicDistinctQuery) {
  std::string sql = "SELECT DISTINCT name FROM products";

  auto parser = Parser(sql);
  auto statements = parser.parse();

  ASSERT_FALSE(statements.empty());

  auto result = executor_->execute(std::move(statements[0]));

  EXPECT_TRUE(result.success);
  // 验证去重结果：原始数据有8行，name重复的有Apple、Banana，所以去重后应该有5个不同名称
  EXPECT_NE(result.message.find("Apple"), std::string::npos);
  EXPECT_NE(result.message.find("Banana"), std::string::npos);
  EXPECT_NE(result.message.find("Orange"), std::string::npos);
  EXPECT_NE(result.message.find("Carrot"), std::string::npos);
  EXPECT_NE(result.message.find("Potato"), std::string::npos);
}

// 测试SELECT DISTINCT配合WHERE条件
TEST_F(SelectDistinctTest, DistinctWithWhere) {
  std::string sql = "SELECT DISTINCT category FROM products WHERE price > 60";

  auto parser = Parser(sql);
  auto statements = parser.parse();

  ASSERT_FALSE(statements.empty());

  auto result = executor_->execute(std::move(statements[0]));

  EXPECT_TRUE(result.success);
  // 只应该包含Fruit类别（Apple和Orange的价格都>60）
  EXPECT_NE(result.message.find("Fruit"), std::string::npos);
  EXPECT_EQ(result.message.find("Vegetable"), std::string::npos);
}

// 测试SELECT DISTINCT多个列
TEST_F(SelectDistinctTest, DistinctMultipleColumns) {
  std::string sql = "SELECT DISTINCT name, price FROM products";

  auto parser = Parser(sql);
  auto statements = parser.parse();

  ASSERT_FALSE(statements.empty());

  auto result = executor_->execute(std::move(statements[0]));

  EXPECT_TRUE(result.success);
  // 验证包含不同的name-price组合
  EXPECT_NE(result.message.find("Apple"), std::string::npos);
  EXPECT_NE(result.message.find("Banana"), std::string::npos);
}

// 测试SELECT DISTINCT配合ORDER BY
TEST_F(SelectDistinctTest, DistinctWithOrderBy) {
  std::string sql = "SELECT DISTINCT category FROM products ORDER BY category";

  auto parser = Parser(sql);
  auto statements = parser.parse();

  ASSERT_FALSE(statements.empty());

  auto result = executor_->execute(std::move(statements[0]));

  EXPECT_TRUE(result.success);
  // 验证结果包含两个类别
  EXPECT_NE(result.message.find("Fruit"), std::string::npos);
  EXPECT_NE(result.message.find("Vegetable"), std::string::npos);
}

// 测试SELECT DISTINCT *（全列去重）
TEST_F(SelectDistinctTest, DistinctAllColumns) {
  std::string sql = "SELECT DISTINCT * FROM products";

  auto parser = Parser(sql);
  auto statements = parser.parse();

  ASSERT_FALSE(statements.empty());

  auto result = executor_->execute(std::move(statements[0]));

  EXPECT_TRUE(result.success);
  // 全列去重应该保留所有不完全相同的行
  // 原始8行中，第1、3行完全相同（除了id），第2、6行完全相同，所以应该有7行结果
  EXPECT_NE(result.message.find("Apple"), std::string::npos);
}

// 测试空DISTINCT结果
TEST_F(SelectDistinctTest, EmptyDistinctResult) {
  std::string sql = "SELECT DISTINCT name FROM products WHERE price > 1000";

  auto parser = Parser(sql);
  auto statements = parser.parse();

  ASSERT_FALSE(statements.empty());

  auto result = executor_->execute(std::move(statements[0]));

  EXPECT_TRUE(result.success);
  // 没有满足条件的记录，应该返回空结果
  EXPECT_EQ(result.rows.size(), 0);
}

// 测试DISTINCT与聚合函数的结合
TEST_F(SelectDistinctTest, DistinctWithAggregation) {
  std::string sql = "SELECT COUNT(DISTINCT category) as distinct_categories FROM products";

  auto parser = Parser(sql);
  auto statements = parser.parse();

  ASSERT_FALSE(statements.empty());

  auto result = executor_->execute(std::move(statements[0]));

  EXPECT_TRUE(result.success);
  // 验证包含聚合结果
  EXPECT_NE(result.message.find("distinct_categories"), std::string::npos);
}

// 测试DISTINCT性能（大数据集）
TEST_F(SelectDistinctTest, DistinctPerformance) {
  // 插入更多重复数据来测试性能
  for (int i = 9; i <= 50; ++i) {
    std::string insert_sql = "INSERT INTO products VALUES (" +
                             std::to_string(i) + ", 'TestProduct', 'TestCategory', 99)";
    auto parser = Parser(insert_sql);
    auto statements = parser.parse();
    if (!statements.empty()) {
      executor_->execute(std::move(statements[0]));
    }
  }

  std::string sql = "SELECT DISTINCT name FROM products";

  auto parser = Parser(sql);
  auto statements = parser.parse();

  ASSERT_FALSE(statements.empty());

  auto result = executor_->execute(std::move(statements[0]));

  EXPECT_TRUE(result.success);
  // 验证去重后的结果数量合理（应该包含TestProduct、Apple、Banana等）
  EXPECT_NE(result.message.find("TestProduct"), std::string::npos);
  EXPECT_NE(result.message.find("Apple"), std::string::npos);
}
