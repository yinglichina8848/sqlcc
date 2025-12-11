#include <gtest/gtest.h>
#include <filesystem>
#include "database_manager.h"
#include "storage/table_storage.h"
#include "storage/index_manager.h"

namespace fs = std::filesystem;

class SimpleIndexTest : public ::testing::Test {
protected:
  std::string test_dir = "./simple_index_test";
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
        {"age", "INTEGER"}};
    ASSERT_TRUE(db_manager->CreateTable("users", columns));
  }

  void TearDown() override {
    if (fs::exists(test_dir)) {
      fs::remove_all(test_dir);
    }
  }
};

// 测试索引创建和基本功能
TEST_F(SimpleIndexTest, BasicIndexFunctionality) {
  // 获取索引管理器
  auto index_manager = db_manager->GetIndexManager();
  ASSERT_NE(index_manager, nullptr);

  // 创建索引
  bool create_result = index_manager->CreateIndex("id_idx", "users", "id");
  EXPECT_TRUE(create_result);

  // 验证索引存在
  bool exists = index_manager->IndexExists("id_idx", "users");
  EXPECT_TRUE(exists);

  // 获取索引
  auto index = index_manager->GetIndex("id_idx", "users");
  EXPECT_NE(index, nullptr);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}