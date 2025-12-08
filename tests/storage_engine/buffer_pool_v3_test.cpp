#include "disk_manager.h"
#include "storage/buffer_pool_v3.h"
#include "storage/concurrency_control.h"
#include "storage/replace_strategy.h"
#include "utils/config_manager.h"

#include <chrono>
#include <gtest/gtest.h>
#include <memory>
#include <random>
#include <thread>
#include <unordered_map>
#include <vector>

namespace sqlcc {

// 模拟配置管理器
class MockConfigManager : public ConfigManager {
public:
  std::string GetString(const std::string &key,
                        const std::string &default_value = "") const {
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
      return it->second;
    }
    return default_value;
  }

  int GetInt(const std::string &key, int default_value = 0) const {
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
      try {
        return std::stoi(it->second);
      } catch (...) {
        return default_value;
      }
    }
    return default_value;
  }

  bool GetBool(const std::string &key, bool default_value = false) const {
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
      return it->second == "true" || it->second == "1";
    }
    return default_value;
  }

  double GetDouble(const std::string &key, double default_value = 0.0) const {
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
      try {
        return std::stod(it->second);
      } catch (...) {
        return default_value;
      }
    }
    return default_value;
  }

  void SetValue(const std::string &key, const std::string &value) {
    config_map_[key] = value;
  }

private:
  std::unordered_map<std::string, std::string> config_map_;
};

// 模拟磁盘管理器
class MockDiskManager : public DiskManager {
public:
  MockDiskManager()
      : DiskManager("test.db", *mock_config_manager_), next_page_id_(0) {}

  bool ReadPage(int32_t page_id, char *page_data) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = pages_.find(page_id);
    if (it == pages_.end()) {
      return false;
    }

    std::memcpy(page_data, it->second.GetData(), PAGE_SIZE);
    return true;
  }

  bool WritePage(int32_t page_id, const char *page_data) {
    std::lock_guard<std::mutex> lock(mutex_);

    Page page(page_id);
    std::memcpy(page.GetData(), page_data, PAGE_SIZE);
    pages_[page_id] = page;
    return true;
  }

  bool DeletePage(int32_t page_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = pages_.find(page_id);
    if (it == pages_.end()) {
      return false;
    }

    pages_.erase(it);
    return true;
  }

  int32_t AllocatePage() {
    std::lock_guard<std::mutex> lock(mutex_);
    return next_page_id_++;
  }

  bool DeallocatePage(int32_t page_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = pages_.find(page_id);
    if (it == pages_.end()) {
      return false;
    }

    pages_.erase(it);
    return true;
  }

  // 测试辅助方法
  size_t GetPageCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pages_.size();
  }

  bool PageExists(int32_t page_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pages_.find(page_id) != pages_.end();
  }

  static MockConfigManager *mock_config_manager_;

private:
  mutable std::mutex mutex_;
  std::unordered_map<int32_t, Page> pages_;
  int32_t next_page_id_;
};

MockConfigManager *MockDiskManager::mock_config_manager_ = nullptr;

class BufferPoolTest : public ::testing::Test {
protected:
  void SetUp() override {
    mock_config_manager_ = std::make_unique<MockConfigManager>();
    MockDiskManager::mock_config_manager_ = mock_config_manager_.get();
    mock_disk_manager_ = std::make_shared<MockDiskManager>();

    // 设置默认配置
    mock_config_manager_->SetValue("buffer.replace_strategy", "LRU");
    mock_config_manager_->SetValue("buffer.pool_size", "10");
    mock_config_manager_->SetValue("lock.timeout_ms", "1000");
    mock_config_manager_->SetValue("lock.max_retries", "3");
    mock_config_manager_->SetValue("prefetch.enabled", "true");
    mock_config_manager_->SetValue("prefetch.max_prefetch_pages", "5");
    mock_config_manager_->SetValue("prefetch.access_history_size", "100");
  }

  void TearDown() override { MockDiskManager::mock_config_manager_ = nullptr; }

  std::shared_ptr<MockDiskManager> mock_disk_manager_;
  std::unique_ptr<MockConfigManager> mock_config_manager_;
};

// 测试基本功能
TEST_F(BufferPoolTest, BasicFunctionality) {
  BufferPool buffer_pool(mock_disk_manager_, *mock_config_manager_, 5);

  // 测试创建新页面
  int32_t page_id = buffer_pool.NewPage(1);
  EXPECT_GE(page_id, 0);

  // 测试获取页面
  auto page = buffer_pool.FetchPage(page_id, 1);
  ASSERT_NE(page, nullptr);
  EXPECT_EQ(page->page_id, page_id);
  EXPECT_EQ(page->ref_count, 1);

  // 测试释放页面
  EXPECT_TRUE(buffer_pool.UnpinPage(page_id, 1));

  // 测试刷新页面
  EXPECT_TRUE(buffer_pool.FlushPage(page_id));

  // 测试删除页面
  EXPECT_TRUE(buffer_pool.DeletePage(page_id));
}

// 测试LRU替换策略
TEST_F(BufferPoolTest, LRUReplacement) {
  mock_config_manager_->SetValue("buffer.replace_strategy", "LRU");
  BufferPool buffer_pool(mock_disk_manager_, *mock_config_manager_, 3);

  // 创建3个页面
  int32_t page1 = buffer_pool.NewPage(1);
  int32_t page2 = buffer_pool.NewPage(1);
  int32_t page3 = buffer_pool.NewPage(1);

  // 释放所有页面
  buffer_pool.UnpinPage(page1, 1);
  buffer_pool.UnpinPage(page2, 1);
  buffer_pool.UnpinPage(page3, 1);

  // 访问page1，使其成为最近使用的
  buffer_pool.FetchPage(page1, 1);
  buffer_pool.UnpinPage(page1, 1);

  // 创建第4个页面，应该替换page2（最近最少使用）
  int32_t page4 = buffer_pool.NewPage(1);

  // page2应该被替换，不在缓存中
  auto page2_ptr = buffer_pool.FetchPage(page2, 1);
  EXPECT_EQ(page2_ptr, nullptr);

  // page1和page3应该仍在缓存中
  auto page1_ptr = buffer_pool.FetchPage(page1, 1);
  auto page3_ptr = buffer_pool.FetchPage(page3, 1);
  EXPECT_NE(page1_ptr, nullptr);
  EXPECT_NE(page3_ptr, nullptr);
}

// 测试2Q替换策略
TEST_F(BufferPoolTest, TwoQReplacement) {
  mock_config_manager_->SetValue("buffer.replace_strategy", "2Q");
  BufferPool buffer_pool(mock_disk_manager_, *mock_config_manager_, 3);

  // 创建3个页面
  int32_t page1 = buffer_pool.NewPage(1);
  int32_t page2 = buffer_pool.NewPage(1);
  int32_t page3 = buffer_pool.NewPage(1);

  // 释放所有页面
  buffer_pool.UnpinPage(page1, 1);
  buffer_pool.UnpinPage(page2, 1);
  buffer_pool.UnpinPage(page3, 1);

  // 访问page1两次，使其进入Am队列
  buffer_pool.FetchPage(page1, 1);
  buffer_pool.UnpinPage(page1, 1);
  buffer_pool.FetchPage(page1, 1);
  buffer_pool.UnpinPage(page1, 1);

  // 创建第4个页面，应该替换page2（在A1out队列中）
  int32_t page4 = buffer_pool.NewPage(1);

  // page2应该被替换，不在缓存中
  auto page2_ptr = buffer_pool.FetchPage(page2, 1);
  EXPECT_EQ(page2_ptr, nullptr);

  // page1和page3应该仍在缓存中
  auto page1_ptr = buffer_pool.FetchPage(page1, 1);
  auto page3_ptr = buffer_pool.FetchPage(page3, 1);
  EXPECT_NE(page1_ptr, nullptr);
  EXPECT_NE(page3_ptr, nullptr);
}

// 测试并发访问
TEST_F(BufferPoolTest, ConcurrentAccess) {
  BufferPool buffer_pool(mock_disk_manager_, *mock_config_manager_, 10);

  const int num_threads = 10;
  const int operations_per_thread = 100;
  std::vector<std::thread> threads;
  std::vector<int32_t> page_ids(num_threads * operations_per_thread / 2);

  // 创建页面
  for (int i = 0; i < num_threads * operations_per_thread / 2; ++i) {
    page_ids[i] = buffer_pool.NewPage(i % 10); // 使用不同的transaction_id
  }

  // 启动多个线程进行并发访问
  std::atomic<int> success_count{0};

  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([&, i]() {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<> dis(0, page_ids.size() - 1);

      for (int j = 0; j < operations_per_thread; ++j) {
        int page_idx = dis(gen);
        int32_t page_id = page_ids[page_idx];

        // 获取页面
        auto page = buffer_pool.FetchPage(page_id, i);
        if (page) {
          // 模拟一些处理
          std::this_thread::sleep_for(std::chrono::microseconds(10));

          // 释放页面
          if (buffer_pool.UnpinPage(page_id, i)) {
            success_count++;
          }
        }
      }
    });
  }

  // 等待所有线程完成
  for (auto &thread : threads) {
    thread.join();
  }

  // 验证所有操作都成功
  EXPECT_EQ(success_count.load(), num_threads * operations_per_thread);
}

// 测试死锁预防
TEST_F(BufferPoolTest, DeadlockPrevention) {
  mock_config_manager_->SetValue("lock.timeout_ms", "100");
  mock_config_manager_->SetValue("lock.max_retries", "3");
  BufferPool buffer_pool(mock_disk_manager_, *mock_config_manager_, 5);

  // 创建两个页面
  int32_t page1 = buffer_pool.NewPage(1);
  int32_t page2 = buffer_pool.NewPage(2);

  // 线程1获取page1，然后尝试获取page2
  std::thread t1([&]() {
    auto p1 = buffer_pool.FetchPage(page1, 1);
    ASSERT_NE(p1, nullptr);

    // 持有p1一段时间
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 尝试获取p2
    auto p2 = buffer_pool.FetchPage(page2, 1);
    if (p2) {
      buffer_pool.UnpinPage(page2, 1);
    }

    buffer_pool.UnpinPage(page1, 1);
  });

  // 线程2获取page2，然后尝试获取page1
  std::thread t2([&]() {
    auto p2 = buffer_pool.FetchPage(page2, 2);
    ASSERT_NE(p2, nullptr);

    // 持有p2一段时间
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 尝试获取p1
    auto p1 = buffer_pool.FetchPage(page1, 2);
    if (p1) {
      buffer_pool.UnpinPage(page1, 2);
    }

    buffer_pool.UnpinPage(page2, 2);
  });

  // 等待线程完成
  t1.join();
  t2.join();

  // 验证最终状态
  auto p1 = buffer_pool.FetchPage(page1, 3);
  auto p2 = buffer_pool.FetchPage(page2, 3);

  EXPECT_NE(p1, nullptr);
  EXPECT_NE(p2, nullptr);

  buffer_pool.UnpinPage(page1, 3);
  buffer_pool.UnpinPage(page2, 3);
}

// 测试预取功能
TEST_F(BufferPoolTest, Prefetching) {
  mock_config_manager_->SetValue("prefetch.enabled", "true");
  mock_config_manager_->SetValue("prefetch.max_prefetch_pages", "3");
  BufferPool buffer_pool(mock_disk_manager_, *mock_config_manager_, 5);

  // 创建5个页面
  std::vector<int32_t> page_ids;
  for (int i = 0; i < 5; ++i) {
    page_ids.push_back(buffer_pool.NewPage(1));
    buffer_pool.UnpinPage(page_ids.back(), 1);
  }

  // 顺序访问页面，触发预取
  for (int i = 0; i < 5; ++i) {
    auto page = buffer_pool.FetchPage(page_ids[i], 1);
    ASSERT_NE(page, nullptr);
    buffer_pool.UnpinPage(page_ids[i], 1);

    // 短暂延迟，让预取器有时间工作
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // 检查统计信息
  auto stats = buffer_pool.GetStats();
  EXPECT_GT(stats.prefetch_stats.total_prefetches, 0);
}

// 测试统计信息
TEST_F(BufferPoolTest, Statistics) {
  BufferPool buffer_pool(mock_disk_manager_, *mock_config_manager_, 5);

  // 创建页面
  int32_t page1 = buffer_pool.NewPage(1);
  int32_t page2 = buffer_pool.NewPage(1);

  // 释放页面
  buffer_pool.UnpinPage(page1, 1);
  buffer_pool.UnpinPage(page2, 1);

  // 获取页面（缓存命中）
  buffer_pool.FetchPage(page1, 1);
  buffer_pool.UnpinPage(page1, 1);

  // 获取不存在的页面（缓存未命中）
  buffer_pool.FetchPage(999, 1);

  // 检查统计信息
  auto stats = buffer_pool.GetStats();
  EXPECT_EQ(stats.pool_size, 5);
  EXPECT_EQ(stats.allocated_pages, 2);
  EXPECT_GT(stats.total_requests, 0);
  EXPECT_GT(stats.cache_hits, 0);
  EXPECT_GT(stats.hit_ratio, 0.0);

  // 测试替换策略统计
  EXPECT_EQ(stats.strategy_type, ReplaceStrategyFactory::StrategyType::LRU);
  EXPECT_EQ(stats.strategy_stats.total_accesses.load(), 3);
  EXPECT_EQ(stats.strategy_stats.strategy_hits.load(), 3);
  EXPECT_EQ(stats.prefetch_stats.total_prefetches, 0);

  // 重置统计信息
  buffer_pool.ResetStats();

  // 检查重置后的统计信息
  stats = buffer_pool.GetStats();
  EXPECT_EQ(stats.total_requests, 0);
  EXPECT_EQ(stats.cache_hits, 0);
  EXPECT_EQ(stats.hit_ratio, 0.0);
}

// 测试动态调整大小
TEST_F(BufferPoolTest, Resize) {
  BufferPool buffer_pool(mock_disk_manager_, *mock_config_manager_, 5);

  // 创建5个页面
  std::vector<int32_t> page_ids;
  for (int i = 0; i < 5; ++i) {
    page_ids.push_back(buffer_pool.NewPage(1));
    buffer_pool.UnpinPage(page_ids.back(), 1);
  }

  // 缩小缓冲池到3
  EXPECT_TRUE(buffer_pool.Resize(3));

  // 检查统计信息
  auto stats = buffer_pool.GetStats();
  EXPECT_EQ(stats.pool_size, 3);
  EXPECT_LE(stats.allocated_pages, 3);

  // 扩大缓冲池到7
  EXPECT_TRUE(buffer_pool.Resize(7));

  // 检查统计信息
  stats = buffer_pool.GetStats();
  EXPECT_EQ(stats.pool_size, 7);
}

// 测试动态切换替换策略
TEST_F(BufferPoolTest, ChangeReplaceStrategy) {
  BufferPool buffer_pool(mock_disk_manager_, *mock_config_manager_, 5);

  // 创建5个页面
  std::vector<int32_t> page_ids;
  for (int i = 0; i < 5; ++i) {
    page_ids.push_back(buffer_pool.NewPage(1));
    buffer_pool.UnpinPage(page_ids.back(), 1);
  }

  // 切换到2Q策略
  EXPECT_TRUE(buffer_pool.ChangeReplaceStrategy(
      ReplaceStrategyFactory::StrategyType::TWO_Q));

  // 检查统计信息
  auto stats = buffer_pool.GetStats();
  EXPECT_EQ(stats.strategy_type, ReplaceStrategyFactory::StrategyType::TWO_Q);

  // 切换回LRU策略
  EXPECT_TRUE(buffer_pool.ChangeReplaceStrategy(
      ReplaceStrategyFactory::StrategyType::LRU));

  // 检查统计信息
  stats = buffer_pool.GetStats();
  EXPECT_EQ(stats.strategy_type, ReplaceStrategyFactory::StrategyType::LRU);
}

// 测试脏页刷新
TEST_F(BufferPoolTest, DirtyPageFlush) {
  BufferPool buffer_pool(mock_disk_manager_, *mock_config_manager_, 5);

  // 创建页面
  int32_t page_id = buffer_pool.NewPage(1);

  // 获取页面并修改（使其变脏）
  auto page = buffer_pool.FetchPage(page_id, 1);
  ASSERT_NE(page, nullptr);

  // 修改页面数据
  page->data.GetData()[0] = 'X';

  // 释放页面
  buffer_pool.UnpinPage(page_id, 1);

  // 刷新页面
  EXPECT_TRUE(buffer_pool.FlushPage(page_id));

  // 检查页面是否已写入磁盘
  EXPECT_TRUE(mock_disk_manager_->PageExists(page_id));
}

// 测试刷新所有页面
TEST_F(BufferPoolTest, FlushAllPages) {
  BufferPool buffer_pool(mock_disk_manager_, *mock_config_manager_, 5);

  // 创建多个页面
  std::vector<int32_t> page_ids;
  for (int i = 0; i < 5; ++i) {
    page_ids.push_back(buffer_pool.NewPage(1));

    // 获取页面并修改（使其变脏）
    auto page = buffer_pool.FetchPage(page_ids.back(), 1);
    ASSERT_NE(page, nullptr);
    page->data.GetData()[0] = 'X' + i;
    buffer_pool.UnpinPage(page_ids.back(), 1);
  }

  // 刷新所有页面
  EXPECT_TRUE(buffer_pool.FlushAllPages());

  // 检查所有页面是否已写入磁盘
  for (int32_t page_id : page_ids) {
    EXPECT_TRUE(mock_disk_manager_->PageExists(page_id));
  }
}

} // namespace sqlcc