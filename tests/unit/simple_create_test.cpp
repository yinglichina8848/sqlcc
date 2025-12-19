#include "storage/b_plus_tree.h"
#include "utils/config_manager.h"
#include "storage_engine.h"
#include <gtest/gtest.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace sqlcc {
namespace storage_engine {
namespace test {

class SimpleCreateTest : public ::testing::Test {
protected:
  void SetUp() override {
    test_dir_ = fs::temp_directory_path() / "sqlcc_simple_test";
    fs::create_directories(test_dir_);
    
    config_manager_ = std::make_unique<ConfigManager>();
    storage_engine_ = std::make_shared<StorageEngine>(*config_manager_, test_dir_.string());
  }

  void TearDown() override {
    storage_engine_.reset();
    config_manager_.reset();
    if (fs::exists(test_dir_)) {
      fs::remove_all(test_dir_);
    }
  }

  std::unique_ptr<ConfigManager> config_manager_;
  std::shared_ptr<StorageEngine> storage_engine_;
  fs::path test_dir_;
};

TEST_F(SimpleCreateTest, CreateIndex) {
  // 创建BPlusTreeIndex实例
  auto b_plus_tree_index = std::make_unique<BPlusTreeIndex>(
      storage_engine_, "test_table", "test_column");
  
  // 只测试Create方法
  bool result = b_plus_tree_index->Create();
  EXPECT_TRUE(result);
  
  // 检查索引是否存在
  EXPECT_TRUE(b_plus_tree_index->Exists());
}

} // namespace test
} // namespace storage_engine
} // namespace sqlcc

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
