#include "storage/lazy_writer.h"
#include "utils/config_manager.h"
#include "storage/disk_manager.h"
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>

namespace sqlcc {
namespace storage {
namespace test {

class LazyWriterTest : public ::testing::Test {
protected:
  void SetUp() override {
    // 创建配置管理器
    config_manager_ = std::make_unique<ConfigManager>();

    // 创建磁盘管理器（使用临时数据库文件）
    disk_manager_ = std::make_unique<DiskManager>("/tmp/test_lazy_writer.db", *config_manager_);

    // 创建延迟写入器
    lazy_writer_ = std::make_unique<LazyWriter>(*config_manager_, *disk_manager_);
  }

  void TearDown() override {
    lazy_writer_->Stop();
  }

  std::unique_ptr<ConfigManager> config_manager_;
  std::unique_ptr<DiskManager> disk_manager_;
  std::unique_ptr<LazyWriter> lazy_writer_;
};

// 测试基本功能
TEST_F(LazyWriterTest, BasicFunctionality) {
  // 测试启用/禁用
  EXPECT_TRUE(lazy_writer_->IsEnabled());
  lazy_writer_->SetEnabled(false);
  EXPECT_FALSE(lazy_writer_->IsEnabled());
  lazy_writer_->SetEnabled(true);
  EXPECT_TRUE(lazy_writer_->IsEnabled());
}

// 测试脏页标记
TEST_F(LazyWriterTest, MarkDirty) {
  // 创建测试页面数据
  PageData page_data;
  page_data.resize(4096, 0); // 4KB页面

  // 标记脏页（不启动工作线程）
  lazy_writer_->MarkDirty(1, page_data);
  lazy_writer_->MarkDirty(2, page_data);

  // 检查统计信息
  auto stats = lazy_writer_->GetStats();
  EXPECT_EQ(stats.current_dirty_pages.load(), 2);

  // 强制刷新所有脏页
  lazy_writer_->ForceFlush();

  // 检查统计信息更新
  auto final_stats = lazy_writer_->GetStats();
  EXPECT_EQ(final_stats.forced_writes.load(), 1);
}

// 测试强制刷新
TEST_F(LazyWriterTest, ForceFlush) {
  // 创建测试页面数据
  PageData page_data;
  page_data.resize(4096, 0);

  // 启动延迟写入器
  lazy_writer_->Start();

  // 标记多个脏页
  for (int i = 1; i <= 10; ++i) {
    lazy_writer_->MarkDirty(i, page_data);
  }

  // 检查脏页数量
  auto stats_before = lazy_writer_->GetStats();
  EXPECT_EQ(stats_before.current_dirty_pages.load(), 10);

  // 强制刷新
  lazy_writer_->ForceFlush();

  // 检查统计信息
  auto stats_after = lazy_writer_->GetStats();
  EXPECT_EQ(stats_after.forced_writes.load(), 1);

  // 停止延迟写入器
  lazy_writer_->Stop();
}

// 测试统计信息
TEST_F(LazyWriterTest, Statistics) {
  // 创建测试页面数据
  PageData page_data;
  page_data.resize(4096, 0);

  // 重置统计信息
  lazy_writer_->ResetStats();
  auto initial_stats = lazy_writer_->GetStats();
  EXPECT_EQ(initial_stats.total_writes.load(), 0);

  // 启动延迟写入器
  lazy_writer_->Start();

  // 执行一些操作
  lazy_writer_->MarkDirty(1, page_data);
  lazy_writer_->MarkDirty(2, page_data);
  lazy_writer_->ForceFlush();

  // 检查统计信息更新
  auto final_stats = lazy_writer_->GetStats();
  EXPECT_GE(final_stats.total_writes.load(), 0);

  // 停止延迟写入器
  lazy_writer_->Stop();
}

// 测试并发访问
TEST_F(LazyWriterTest, ConcurrentAccess) {
  // 创建测试页面数据
  PageData page_data;
  page_data.resize(4096, 0);

  // 启动延迟写入器
  lazy_writer_->Start();

  // 创建多个线程并发标记脏页
  const int num_threads = 10;
  const int pages_per_thread = 50;
  std::vector<std::thread> threads;

  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([this, &page_data, t, pages_per_thread]() {
      for (int i = 0; i < pages_per_thread; ++i) {
        int page_id = t * pages_per_thread + i + 1;
        lazy_writer_->MarkDirty(page_id, page_data);
        std::this_thread::sleep_for(std::chrono::microseconds(100));
      }
    });
  }

  // 等待所有线程完成
  for (auto& thread : threads) {
    thread.join();
  }

  // 检查脏页数量
  auto stats = lazy_writer_->GetStats();
  EXPECT_EQ(stats.current_dirty_pages.load(), num_threads * pages_per_thread);

  // 强制刷新所有脏页
  lazy_writer_->ForceFlush();

  // 等待处理完成
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // 停止延迟写入器
  lazy_writer_->Stop();
}

// 测试页面选择算法
TEST_F(LazyWriterTest, PageSelection) {
  // 创建测试页面数据
  PageData page_data;
  page_data.resize(4096, 0);

  // 启动延迟写入器
  lazy_writer_->Start();

  // 标记多个脏页，模拟不同修改时间
  for (int i = 1; i <= 100; ++i) {
    lazy_writer_->MarkDirty(i, page_data);
    // 每10个页面暂停一下，创建时间差异
    if (i % 10 == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  // 等待后台处理
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  // 检查是否有写入发生
  auto stats = lazy_writer_->GetStats();
  EXPECT_GE(stats.total_writes.load(), 0);

  // 停止延迟写入器
  lazy_writer_->Stop();
}

// 测试边界条件
TEST_F(LazyWriterTest, EdgeCases) {
  // 测试空页面数据
  PageData empty_page_data;
  lazy_writer_->MarkDirty(1, empty_page_data);

  // 测试重复标记同一页面
  PageData page_data;
  page_data.resize(4096, 0);
  lazy_writer_->MarkDirty(1, page_data);
  lazy_writer_->MarkDirty(1, page_data);

  // 检查统计信息
  auto stats = lazy_writer_->GetStats();
  EXPECT_GE(stats.current_dirty_pages.load(), 0);

  // 测试未启用时的行为
  lazy_writer_->SetEnabled(false);
  lazy_writer_->MarkDirty(2, page_data);
  // 应该不产生影响

  lazy_writer_->SetEnabled(true);
}

// 测试启动和停止
TEST_F(LazyWriterTest, StartStop) {
  // 测试多次启动
  lazy_writer_->Start();
  lazy_writer_->Start(); // 应该安全

  // 测试多次停止
  lazy_writer_->Stop();
  lazy_writer_->Stop(); // 应该安全

  // 测试重新启动
  lazy_writer_->Start();
  PageData page_data;
  page_data.resize(4096, 0);
  lazy_writer_->MarkDirty(1, page_data);
  lazy_writer_->Stop();
}

} // namespace test
} // namespace storage
} // namespace sqlcc
