#include "storage/b_plus_tree.h"
#include "utils/config_manager.h"
#include "storage_engine.h"
#include <gtest/gtest.h>

namespace sqlcc {
namespace storage_engine {
namespace test {

class BPlusTreeTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 每次测试前创建ConfigManager和StorageEngine实例
    config_manager_ = std::make_unique<ConfigManager>();
    storage_engine_ = std::make_unique<StorageEngine>(*config_manager_);
    // 创建BPlusTreeIndex实例
    b_plus_tree_index_ = std::make_unique<BPlusTreeIndex>(
        storage_engine_.get(), "test_table", "test_column");
    // 创建索引
    b_plus_tree_index_->Create();
  }

  void TearDown() override {
    // 每次测试后清理
    b_plus_tree_index_.reset();
    storage_engine_.reset();
    config_manager_.reset();
    std::remove("test_db");
    std::remove("test_db.meta");
  }

  std::unique_ptr<ConfigManager> config_manager_;
  std::unique_ptr<StorageEngine> storage_engine_;
  std::unique_ptr<BPlusTreeIndex> b_plus_tree_index_;
};

TEST_F(BPlusTreeTest, InsertAndSearch) {
  // 插入键值对
  IndexEntry entry1("1", 1, 0);
  IndexEntry entry2("2", 2, 0);
  IndexEntry entry3("3", 3, 0);

  EXPECT_TRUE(b_plus_tree_index_->Insert(entry1));
  EXPECT_TRUE(b_plus_tree_index_->Insert(entry2));
  EXPECT_TRUE(b_plus_tree_index_->Insert(entry3));

  // 搜索键值对
  std::vector<IndexEntry> results = b_plus_tree_index_->Search("2");
  EXPECT_EQ(results.size(), 1);
  EXPECT_EQ(results[0].key, "2");
  EXPECT_EQ(results[0].page_id, 2);

  // 搜索不存在的键
  results = b_plus_tree_index_->Search("4");
  EXPECT_EQ(results.size(), 0);
}

TEST_F(BPlusTreeTest, Delete) {
  // 插入键值对
  IndexEntry entry1("1", 1, 0);
  IndexEntry entry2("2", 2, 0);
  IndexEntry entry3("3", 3, 0);

  EXPECT_TRUE(b_plus_tree_index_->Insert(entry1));
  EXPECT_TRUE(b_plus_tree_index_->Insert(entry2));
  EXPECT_TRUE(b_plus_tree_index_->Insert(entry3));

  // 删除中间键
  EXPECT_TRUE(b_plus_tree_index_->Delete("2"));
  std::vector<IndexEntry> results = b_plus_tree_index_->Search("2");
  EXPECT_EQ(results.size(), 0);

  // 验证其他键仍然存在
  results = b_plus_tree_index_->Search("1");
  EXPECT_EQ(results.size(), 1);
  EXPECT_EQ(results[0].key, "1");

  results = b_plus_tree_index_->Search("3");
  EXPECT_EQ(results.size(), 1);
  EXPECT_EQ(results[0].key, "3");
}

TEST_F(BPlusTreeTest, MultipleInsertions) {
  // 插入多个键值对
  const int NUM_INSERTS = 10;
  for (int i = 0; i < NUM_INSERTS; ++i) {
    std::string key = std::to_string(i);
    IndexEntry entry(key, i, 0);
    EXPECT_TRUE(b_plus_tree_index_->Insert(entry));
  }

  // 验证所有键都能被找到
  for (int i = 0; i < NUM_INSERTS; ++i) {
    std::string key = std::to_string(i);
    std::vector<IndexEntry> results = b_plus_tree_index_->Search(key);
    EXPECT_EQ(results.size(), 1) << "Key " << key << " not found";
    EXPECT_EQ(results[0].key, key) << "Value mismatch for key " << key;
  }
}

TEST_F(BPlusTreeTest, RangeQuery) {
  // 插入连续键值对
  for (int i = 0; i < 10; ++i) {
    std::string key = std::to_string(i);
    IndexEntry entry(key, i, 0);
    EXPECT_TRUE(b_plus_tree_index_->Insert(entry));
  }

  // 执行范围查询 [2, 7]
  std::vector<IndexEntry> results = b_plus_tree_index_->SearchRange("2", "7");

  // 验证查询结果
  EXPECT_EQ(results.size(), 6); // 2,3,4,5,6,7
  for (size_t i = 0; i < results.size(); ++i) {
    std::string expected_key = std::to_string(2 + i);
    EXPECT_EQ(results[i].key, expected_key);
  }
}

TEST_F(BPlusTreeTest, DeleteAll) {
  // 插入键值对
  const int NUM_INSERTS = 10;
  for (int i = 0; i < NUM_INSERTS; ++i) {
    std::string key = std::to_string(i);
    IndexEntry entry(key, i, 0);
    EXPECT_TRUE(b_plus_tree_index_->Insert(entry));
  }

  // 删除所有键
  for (int i = 0; i < NUM_INSERTS; ++i) {
    std::string key = std::to_string(i);
    EXPECT_TRUE(b_plus_tree_index_->Delete(key));
  }

  // 验证所有键都已被删除
  for (int i = 0; i < NUM_INSERTS; ++i) {
    std::string key = std::to_string(i);
    std::vector<IndexEntry> results = b_plus_tree_index_->Search(key);
    EXPECT_EQ(results.size(), 0)
        << "Key " << key << " still exists after deletion";
  }
}

TEST_F(BPlusTreeTest, DuplicateInsertions) {
  // 插入相同键多次
  IndexEntry entry1("1", 1, 0);
  IndexEntry entry2("1", 1, 10); // 相同键，不同偏移量

  EXPECT_TRUE(b_plus_tree_index_->Insert(entry1));
  EXPECT_TRUE(b_plus_tree_index_->Insert(entry2));

  // 验证键被更新
  std::vector<IndexEntry> results = b_plus_tree_index_->Search("1");
  EXPECT_EQ(results.size(), 1);
  EXPECT_EQ(results[0].key, "1");
  EXPECT_EQ(results[0].offset, 10); // 应该是最新的偏移量
}

TEST_F(BPlusTreeTest, LargeKeyInsertion) {
  // 插入一个大键
  IndexEntry entry("1000000", 1, 0);
  EXPECT_TRUE(b_plus_tree_index_->Insert(entry));

  // 搜索大键
  std::vector<IndexEntry> results = b_plus_tree_index_->Search("1000000");
  EXPECT_EQ(results.size(), 1);
  EXPECT_EQ(results[0].key, "1000000");
}

} // namespace test
} // namespace storage_engine
} // namespace sqlcc

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
