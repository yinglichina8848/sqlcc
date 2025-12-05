#include "database_manager.h"
#include "execution_engine.h"
#include "sql_parser/parser_new.h"
#include "unified_query_plan.h"
#include <filesystem>
#include <gtest/gtest.h>

namespace fs = std::filesystem;

class IndexSystemIntegrationTest : public ::testing::Test {
protected:
  std::string test_dir = "./index_system_test";
  std::shared_ptr<sqlcc::DatabaseManager> db_manager;

  void SetUp() override {
    // 清理旧的测试目录
    if (fs::exists(test_dir)) {
      fs::remove_all(test_dir);
    }

    // 创建数据库管理器
    db_manager = std::make_shared<sqlcc::DatabaseManager>(test_dir);

    // 创建测试数据库和表
    ASSERT_TRUE(db_manager->CreateDatabase("testdb"));
    ASSERT_TRUE(db_manager->UseDatabase("testdb"));

    // 创建测试表
    std::vector<std::pair<std::string, std::string>> columns = {
        {"id", "INTEGER"},
        {"name", "VARCHAR"},
        {"age", "INTEGER"},
        {"email", "VARCHAR"}};
    ASSERT_TRUE(db_manager->CreateTable("users", columns));

    // 创建索引
    auto index_manager = db_manager->GetIndexManager();
    ASSERT_TRUE(index_manager->CreateIndex("id_idx", "users", "id"));
    ASSERT_TRUE(index_manager->CreateIndex("name_idx", "users", "name"));
    ASSERT_TRUE(index_manager->CreateIndex("age_idx", "users", "age"));
  }

  void TearDown() override {
    if (fs::exists(test_dir)) {
      fs::remove_all(test_dir);
    }
  }

  // 辅助函数：解析SQL语句
  std::unique_ptr<sqlcc::sql_parser::Statement>
  parseSQL(const std::string &sql) {
    sqlcc::sql_parser::ParserNew parser(sql);
    auto statements = parser.parse();
    if (statements.empty()) {
      // 解析失败时返回空指针
      return nullptr;
    }

    return std::move(statements[0]);
  }

  // 辅助函数：执行SQL语句
  sqlcc::ExecutionResult executeSQL(const std::string &sql) {
    auto stmt = parseSQL(sql);
    if (!stmt) {
      return sqlcc::ExecutionResult(false, "Failed to parse SQL");
    }

    sqlcc::DMLExecutor executor(db_manager);
    return executor.execute(std::move(stmt));
  }
};

// 测试索引优化查询 - 简单等值条件
TEST_F(IndexSystemIntegrationTest, IndexOptimizedSelectWithEqualCondition) {
  // 插入测试数据
  std::string insert_sql = "INSERT INTO users (id, name, age, email) VALUES "
                           "(1, 'Alice', 25, 'alice@example.com');";
  auto insert_result = executeSQL(insert_sql);
  EXPECT_TRUE(insert_result.success);

  // 测试使用索引的SELECT查询
  std::string select_sql = "SELECT * FROM users WHERE id = 1;";
  auto select_result = executeSQL(select_sql);

  // 验证查询成功执行
  EXPECT_TRUE(select_result.success);
  EXPECT_NE(select_result.message.find("executed successfully"),
            std::string::npos);
}

// 测试索引优化查询 - 字符串等值条件
TEST_F(IndexSystemIntegrationTest,
       IndexOptimizedSelectWithStringEqualCondition) {
  // 插入测试数据
  std::string insert_sql = "INSERT INTO users (id, name, age, email) VALUES "
                           "(2, 'Bob', 30, 'bob@example.com');";
  auto insert_result = executeSQL(insert_sql);
  EXPECT_TRUE(insert_result.success);

  // 测试使用索引的SELECT查询
  std::string select_sql = "SELECT * FROM users WHERE name = 'Bob';";
  auto select_result = executeSQL(select_sql);

  // 验证查询成功执行
  EXPECT_TRUE(select_result.success);
  EXPECT_NE(select_result.message.find("executed successfully"),
            std::string::npos);
}

// 测试索引维护 - INSERT操作
TEST_F(IndexSystemIntegrationTest, IndexMaintenanceOnInsert) {
  // 插入多条测试数据
  std::vector<std::string> insert_sqls = {
      "INSERT INTO users (id, name, age, email) VALUES (3, 'Charlie', 28, "
      "'charlie@example.com');",
      "INSERT INTO users (id, name, age, email) VALUES (4, 'Diana', 32, "
      "'diana@example.com');",
      "INSERT INTO users (id, name, age, email) VALUES (5, 'Eve', 22, "
      "'eve@example.com');"};

  for (const auto &sql : insert_sqls) {
    auto result = executeSQL(sql);
    EXPECT_TRUE(result.success);
  }

  // 验证索引维护：通过索引查询插入的数据
  std::string select_sql = "SELECT * FROM users WHERE id = 4;";
  auto select_result = executeSQL(select_sql);

  EXPECT_TRUE(select_result.success);
  EXPECT_NE(select_result.message.find("executed successfully"),
            std::string::npos);
}

// 测试索引维护 - UPDATE操作
TEST_F(IndexSystemIntegrationTest, IndexMaintenanceOnUpdate) {
  // 插入测试数据
  std::string insert_sql = "INSERT INTO users (id, name, age, email) VALUES "
                           "(6, 'Frank', 35, 'frank@example.com');";
  auto insert_result = executeSQL(insert_sql);
  EXPECT_TRUE(insert_result.success);

  // 更新数据（索引列值变化）
  std::string update_sql = "UPDATE users SET age = 36 WHERE id = 6;";
  auto update_result = executeSQL(update_sql);
  EXPECT_TRUE(update_result.success);

  // 验证索引维护：通过新值查询
  std::string select_sql = "SELECT * FROM users WHERE age = 36;";
  auto select_result = executeSQL(select_sql);

  EXPECT_TRUE(select_result.success);
  EXPECT_NE(select_result.message.find("executed successfully"),
            std::string::npos);
}

// 测试索引维护 - DELETE操作
TEST_F(IndexSystemIntegrationTest, IndexMaintenanceOnDelete) {
  // 插入测试数据
  std::string insert_sql = "INSERT INTO users (id, name, age, email) VALUES "
                           "(7, 'Grace', 40, 'grace@example.com');";
  auto insert_result = executeSQL(insert_sql);
  EXPECT_TRUE(insert_result.success);

  // 删除数据
  std::string delete_sql = "DELETE FROM users WHERE id = 7;";
  auto delete_result = executeSQL(delete_sql);
  EXPECT_TRUE(delete_result.success);

  // 验证索引维护：尝试查询已删除的数据
  std::string select_sql = "SELECT * FROM users WHERE id = 7;";
  auto select_result = executeSQL(select_sql);

  // 应该找不到数据，但查询执行应该成功
  EXPECT_TRUE(select_result.success);
  EXPECT_NE(select_result.message.find("executed successfully"),
            std::string::npos);
}

// 测试全表扫描（无索引条件）
TEST_F(IndexSystemIntegrationTest, FullTableScanWithoutIndex) {
  // 插入测试数据
  std::string insert_sql = "INSERT INTO users (id, name, age, email) VALUES "
                           "(8, 'Henry', 45, 'henry@example.com');";
  auto insert_result = executeSQL(insert_sql);
  EXPECT_TRUE(insert_result.success);

  // 测试不使用索引的查询（复杂条件）
  std::string select_sql = "SELECT * FROM users WHERE age > 20 AND age < 50;";
  auto select_result = executeSQL(select_sql);

  // 验证查询成功执行（应该使用全表扫描）
  EXPECT_TRUE(select_result.success);
  EXPECT_NE(select_result.message.find("executed successfully"),
            std::string::npos);
}

// 测试复合索引场景（当前实现不支持，但验证基础功能）
TEST_F(IndexSystemIntegrationTest, BasicIndexFunctionality) {
  // 插入测试数据
  std::string insert_sql = "INSERT INTO users (id, name, age, email) VALUES "
                           "(9, 'Ivan', 50, 'ivan@example.com');";
  auto insert_result = executeSQL(insert_sql);
  EXPECT_TRUE(insert_result.success);

  // 测试各种索引查询
  std::vector<std::string> select_sqls = {
      "SELECT * FROM users WHERE id = 9;",
      "SELECT * FROM users WHERE name = 'Ivan';",
      "SELECT * FROM users WHERE age = 50;"};

  for (const auto &sql : select_sqls) {
    auto result = executeSQL(sql);
    EXPECT_TRUE(result.success);
    EXPECT_NE(result.message.find("executed successfully"), std::string::npos);
  }
}

// 测试DML查询计划集成
TEST_F(IndexSystemIntegrationTest, DMLQueryPlanIntegration) {
  // 创建DML查询计划
  auto user_manager = std::make_shared<sqlcc::UserManager>();
  auto system_db = std::make_shared<sqlcc::SystemDatabase>(db_manager);

  // 插入测试数据
  std::string insert_sql = "INSERT INTO users (id, name, age, email) VALUES "
                           "(10, 'Jack', 55, 'jack@example.com');";
  auto insert_stmt = parseSQL(insert_sql);
  ASSERT_TRUE(insert_stmt != nullptr);

  // 构建并执行插入计划
  sqlcc::DMLQueryPlan insert_plan(db_manager, user_manager, system_db);
  bool build_result = insert_plan.buildPlan(std::move(insert_stmt));
  EXPECT_TRUE(build_result);

  auto execute_result = insert_plan.executePlan();
  EXPECT_TRUE(execute_result.success);

  // 测试SELECT查询计划
  std::string select_sql = "SELECT * FROM users WHERE id = 10;";
  auto select_stmt = parseSQL(select_sql);
  ASSERT_TRUE(select_stmt != nullptr);

  sqlcc::DMLQueryPlan select_plan(db_manager, user_manager, system_db);
  build_result = select_plan.buildPlan(std::move(select_stmt));
  EXPECT_TRUE(build_result);

  execute_result = select_plan.executePlan();
  EXPECT_TRUE(execute_result.success);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}