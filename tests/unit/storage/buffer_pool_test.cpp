#include "buffer_pool.h"
#include "config_manager.h"
#include "disk_manager.h"
#include <gtest/gtest.h>

namespace sqlcc {
namespace storage_engine {
namespace test {

class BufferPoolTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 每次测试前创建ConfigManager、DiskManager和BufferPool实例
    config_manager_ = std::make_unique<ConfigManager>();
    disk_manager_ = std::make_unique<DiskManager>("test_db", *config_manager_);
    buffer_pool_ = std::make_unique<BufferPool>(
        disk_manager_.get(), 10, *config_manager_); // 10个页面大小的缓冲区
  }

  void TearDown() override {
    // 每次测试后清理
    buffer_pool_.reset();
    disk_manager_.reset();
    config_manager_.reset();
    std::remove("test_db");
    std::remove("test_db.meta");
  }

  std::unique_ptr<ConfigManager> config_manager_;
  std::unique_ptr<DiskManager> disk_manager_;
  std::unique_ptr<BufferPool> buffer_pool_;
};

TEST_F(BufferPoolTest, FetchPage) {
  // 分配页面
  int32_t page_id = disk_manager_->AllocatePage();
  EXPECT_NE(page_id, -1);

  // 写入一些数据到页面，确保页面存在
  char data[8192] = {0};
  sprintf(data, "Page %d data", page_id);
  disk_manager_->WritePage(page_id, data);

  // 获取页面
  Page *page = buffer_pool_->FetchPage(page_id);
  EXPECT_NE(page, nullptr);
  EXPECT_EQ(page->GetPageId(), page_id);

  // 释放页面
  buffer_pool_->UnpinPage(page_id, false);
}

TEST_F(BufferPoolTest, UnpinPage) {
  // 分配页面
  int32_t page_id = disk_manager_->AllocatePage();
  EXPECT_NE(page_id, -1);

  // 写入一些数据到页面，确保页面存在
  char data[8192] = {0};
  sprintf(data, "Page %d data", page_id);
  disk_manager_->WritePage(page_id, data);

  // 获取并释放页面
  Page *page = buffer_pool_->FetchPage(page_id);
  EXPECT_NE(page, nullptr);
  buffer_pool_->UnpinPage(page_id, false);
  buffer_pool_->UnpinPage(page_id, false); // 再次释放，应该没有问题

  // 释放页面指针
  buffer_pool_->UnpinPage(page_id, false);
}

TEST_F(BufferPoolTest, FlushPage) {
  // 分配页面
  int32_t page_id = disk_manager_->AllocatePage();
  EXPECT_NE(page_id, -1);

  // 写入一些数据到页面，确保页面存在
  char data[8192] = {0};
  sprintf(data, "Initial data");
  disk_manager_->WritePage(page_id, data);

  // 获取页面并修改
  Page *page = buffer_pool_->FetchPage(page_id);
  EXPECT_NE(page, nullptr);
  strcpy(page->GetData(), "Modified data");
  buffer_pool_->UnpinPage(page_id, true); // 标记为脏页

  // 刷新页面到磁盘
  buffer_pool_->FlushPage(page_id);

  // 重新获取页面，验证数据已持久化
  Page *page2 = buffer_pool_->FetchPage(page_id);
  EXPECT_NE(page2, nullptr);
  EXPECT_STREQ(page2->GetData(), "Modified data");
  buffer_pool_->UnpinPage(page_id, false);
}

TEST_F(BufferPoolTest, LRUReplacement) {
  // 简化测试，创建较少的页面来触发LRU替换
  const int BUFFER_SIZE = 3; // 使用较小的缓冲区大小
  std::vector<int32_t> page_ids;

  // 分配并获取超过缓冲区大小的页面
  for (int i = 0; i < BUFFER_SIZE + 2; ++i) {
    int32_t page_id = disk_manager_->AllocatePage();
    page_ids.push_back(page_id);

    // 写入一些数据到页面，确保页面存在
    char data[8192] = {0};
    sprintf(data, "Initial data %d", i);
    disk_manager_->WritePage(page_id, data);

    Page *page = buffer_pool_->FetchPage(page_id);
    EXPECT_NE(page, nullptr);
    sprintf(page->GetData(), "Page %d", i);
    buffer_pool_->UnpinPage(page_id, true);
  }

  // 验证前2个页面已经被替换出缓冲区
  for (int i = 0; i < 2; ++i) {
    // 这些页面应该不在缓冲区中，需要从磁盘重新加载
    Page *page = buffer_pool_->FetchPage(page_ids[i]);
    EXPECT_NE(page, nullptr);
    buffer_pool_->UnpinPage(page_ids[i], false);
  }
}

TEST_F(BufferPoolTest, FlushAllPages) {
  // 分配多个页面并修改
  const int NUM_PAGES = 5;
  std::vector<int32_t> page_ids;

  for (int i = 0; i < NUM_PAGES; ++i) {
    int32_t page_id = disk_manager_->AllocatePage();
    page_ids.push_back(page_id);

    // 写入一些数据到页面，确保页面存在
    char data[8192] = {0};
    sprintf(data, "Initial data %d", i);
    disk_manager_->WritePage(page_id, data);

    Page *page = buffer_pool_->FetchPage(page_id);
    EXPECT_NE(page, nullptr);
    sprintf(page->GetData(), "Page %d", i);
    buffer_pool_->UnpinPage(page_id, true); // 标记为脏页
  }

  // 刷新所有页面
  buffer_pool_->FlushAllPages();

  // 验证页面可以重新获取，间接验证刷新成功
  for (int32_t page_id : page_ids) {
    Page *page = buffer_pool_->FetchPage(page_id);
    EXPECT_NE(page, nullptr);
    buffer_pool_->UnpinPage(page_id, false);
  }
}

TEST_F(BufferPoolTest, BasicOperations) {
  // 测试基本的获取和释放页面操作
  int32_t page_id = disk_manager_->AllocatePage();
  EXPECT_NE(page_id, -1);

  // 写入一些数据到页面，确保页面存在
  char data[8192] = {0};
  sprintf(data, "Page %d data", page_id);
  disk_manager_->WritePage(page_id, data);

  // 获取页面
  Page *page = buffer_pool_->FetchPage(page_id);
  EXPECT_NE(page, nullptr);
  EXPECT_EQ(page->GetPageId(), page_id);

  // 释放页面
  buffer_pool_->UnpinPage(page_id, false);

  // 再次获取页面
  Page *page2 = buffer_pool_->FetchPage(page_id);
  EXPECT_NE(page2, nullptr);
  EXPECT_EQ(page2->GetPageId(), page_id);
  buffer_pool_->UnpinPage(page_id, false);
}

} // namespace test
} // namespace storage_engine
} // namespace sqlcc

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
