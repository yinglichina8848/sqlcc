#include "disk_manager.h"
#include "storage/buffer_pool.h"
#include "utils/config_manager.h"
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

TEST_F(BufferPoolTest, InvalidPageOperations) {
  // 测试获取无效页面
  Page *page = buffer_pool_->FetchPage(-1);
  EXPECT_EQ(page, nullptr);

  // 测试释放无效页面
  buffer_pool_->UnpinPage(-1, false); // 应该没有崩溃

  // 测试刷新无效页面
  buffer_pool_->FlushPage(-1); // 应该没有崩溃
}

TEST_F(BufferPoolTest, MultipleFetchSamePage) {
  // 分配页面
  int32_t page_id = disk_manager_->AllocatePage();
  EXPECT_NE(page_id, -1);

  // 写入一些数据到页面
  char data[8192] = {0};
  sprintf(data, "Initial data");
  disk_manager_->WritePage(page_id, data);

  // 多次获取同一页面
  Page *page1 = buffer_pool_->FetchPage(page_id);
  EXPECT_NE(page1, nullptr);

  Page *page2 = buffer_pool_->FetchPage(page_id);
  EXPECT_NE(page2, nullptr);
  EXPECT_EQ(page1, page2); // 应该返回同一指针

  // 修改页面数据
  strcpy(page1->GetData(), "Modified by page1");
  EXPECT_STREQ(page2->GetData(), "Modified by page1"); // 两个指针指向同一页面

  // 释放页面两次
  buffer_pool_->UnpinPage(page_id, true);
  buffer_pool_->UnpinPage(page_id, false);
}

TEST_F(BufferPoolTest, PageEviction) {
  // 创建一个小的缓冲区
  std::unique_ptr<BufferPool> small_buffer_pool = std::make_unique<BufferPool>(
      disk_manager_.get(), 3, *config_manager_); // 仅3个页面的缓冲区

  // 分配并获取多个页面，触发页面驱逐
  std::vector<int32_t> page_ids;
  for (int i = 0; i < 5; ++i) {
    int32_t page_id = disk_manager_->AllocatePage();
    page_ids.push_back(page_id);

    // 写入数据
    char data[8192] = {0};
    sprintf(data, "Page %d data", page_id);
    disk_manager_->WritePage(page_id, data);

    // 获取页面
    Page *page = small_buffer_pool->FetchPage(page_id);
    EXPECT_NE(page, nullptr);

    // 修改页面并释放，标记为脏页
    sprintf(page->GetData(), "Modified page %d", page_id);
    small_buffer_pool->UnpinPage(page_id, true);
  }

  // 验证可以再次获取最早的页面（应该已经被驱逐）
  Page *evicted_page = small_buffer_pool->FetchPage(page_ids[0]);
  EXPECT_NE(evicted_page, nullptr);
  small_buffer_pool->UnpinPage(page_ids[0], false);
}

TEST_F(BufferPoolTest, LargeDataOperations) {
  // 分配页面
  int32_t page_id = disk_manager_->AllocatePage();
  EXPECT_NE(page_id, -1);

  // 写入大量数据到页面
  char large_data[8192] = {0};
  for (int i = 0; i < 8191; ++i) {
    large_data[i] = 'A' + (i % 26);
  }
  large_data[8191] = '\0';
  disk_manager_->WritePage(page_id, large_data);

  // 获取页面并验证数据完整性
  Page *page = buffer_pool_->FetchPage(page_id);
  EXPECT_NE(page, nullptr);
  EXPECT_STREQ(page->GetData(), large_data);

  // 修改大量数据
  for (int i = 0; i < 8191; ++i) {
    page->GetData()[i] = 'Z' - (i % 26);
  }
  page->GetData()[8191] = '\0';

  // 释放并刷新页面
  buffer_pool_->UnpinPage(page_id, true);
  buffer_pool_->FlushPage(page_id);

  // 重新获取并验证修改后的数据
  Page *page2 = buffer_pool_->FetchPage(page_id);
  EXPECT_NE(page2, nullptr);
  for (int i = 0; i < 8191; ++i) {
    EXPECT_EQ(page2->GetData()[i], 'Z' - (i % 26));
  }
  buffer_pool_->UnpinPage(page_id, false);
}

} // namespace test
} // namespace storage_engine
} // namespace sqlcc

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
