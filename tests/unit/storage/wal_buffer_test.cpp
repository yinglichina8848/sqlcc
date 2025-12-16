#include "storage/wal_buffer.h"
#include "utils/config_manager.h"
#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>

namespace sqlcc {
namespace storage {
namespace test {

class WALBufferTest : public ::testing::Test {
protected:
  void SetUp() override {
    config_manager_ = std::make_unique<ConfigManager>();
    wal_buffer_ = std::make_unique<WALBuffer>(*config_manager_, 1024 * 1024); // 1MB buffer
  }

  void TearDown() override {
    wal_buffer_.reset();
  }

  std::unique_ptr<ConfigManager> config_manager_;
  std::unique_ptr<WALBuffer> wal_buffer_;
};

// 测试基本功能
TEST_F(WALBufferTest, BasicFunctionality) {
  // 测试初始状态
  EXPECT_EQ(wal_buffer_->GetCurrentSize(), 0);
  EXPECT_GT(wal_buffer_->GetUtilization(), 0.0);

  // 测试统计信息
  auto stats = wal_buffer_->GetStats();
  EXPECT_EQ(stats.total_logs.load(), 0);
  EXPECT_EQ(stats.total_flushes.load(), 0);
}

// 测试添加记录
TEST_F(WALBufferTest, AddRecord) {
  // 创建测试记录
  auto record = std::make_unique<WALBuffer::WALRecord>(
      1, 1001, "INSERT", "INSERT INTO test VALUES (1, 'test')");

  // 添加记录
  EXPECT_TRUE(wal_buffer_->AddRecord(std::move(record)));

  // 检查统计信息
  auto stats = wal_buffer_->GetStats();
  EXPECT_EQ(stats.total_logs.load(), 1);
  EXPECT_GT(wal_buffer_->GetCurrentSize(), 0);
}

// 测试强制刷新
TEST_F(WALBufferTest, ForceFlush) {
  // 添加多个记录
  for (int i = 0; i < 10; ++i) {
    auto record = std::make_unique<WALBuffer::WALRecord>(
        i + 1, 1001 + i, "INSERT",
        "INSERT INTO test VALUES (" + std::to_string(i) + ", 'test')");
    wal_buffer_->AddRecord(std::move(record));
  }

  // 检查记录已添加
  auto stats_before = wal_buffer_->GetStats();
  EXPECT_EQ(stats_before.total_logs.load(), 10);

  // 强制刷新
  EXPECT_TRUE(wal_buffer_->ForceFlush());

  // 检查统计信息更新
  auto stats_after = wal_buffer_->GetStats();
  EXPECT_GE(stats_after.total_flushes.load(), 1);
}

// 测试缓冲区大小限制
TEST_F(WALBufferTest, BufferSizeLimit) {
  // 创建大记录
  std::string large_data(100 * 1024, 'x'); // 100KB数据
  auto record = std::make_unique<WALBuffer::WALRecord>(
      1, 1001, "INSERT", large_data);

  // 添加大记录
  EXPECT_TRUE(wal_buffer_->AddRecord(std::move(record)));

  // 检查缓冲区使用率
  double utilization = wal_buffer_->GetUtilization();
  EXPECT_GT(utilization, 0.0);
  EXPECT_LE(utilization, 1.0);
}

// 测试并发访问
TEST_F(WALBufferTest, ConcurrentAccess) {
  const int num_threads = 5;
  const int records_per_thread = 20;

  // 创建多个线程并发添加记录
  std::vector<std::thread> threads;
  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([this, t, records_per_thread]() {
      for (int i = 0; i < records_per_thread; ++i) {
        auto record = std::make_unique<WALBuffer::WALRecord>(
            t * records_per_thread + i + 1,
            1001 + t * records_per_thread + i,
            "INSERT",
            "INSERT INTO test VALUES (" + std::to_string(i) + ", 'thread_" + std::to_string(t) + "')");
        wal_buffer_->AddRecord(std::move(record));
        std::this_thread::sleep_for(std::chrono::microseconds(100));
      }
    });
  }

  // 等待所有线程完成
  for (auto& thread : threads) {
    thread.join();
  }

  // 检查总记录数
  auto stats = wal_buffer_->GetStats();
  EXPECT_EQ(stats.total_logs.load(), num_threads * records_per_thread);
}

// 测试统计信息重置
TEST_F(WALBufferTest, ResetStats) {
  // 添加一些记录
  auto record = std::make_unique<WALBuffer::WALRecord>(
      1, 1001, "INSERT", "INSERT INTO test VALUES (1, 'test')");
  wal_buffer_->AddRecord(std::move(record));

  // 检查统计信息
  auto stats_before = wal_buffer_->GetStats();
  EXPECT_EQ(stats_before.total_logs.load(), 1);

  // 重置统计信息
  wal_buffer_->ResetStats();

  // 检查统计信息已重置
  auto stats_after = wal_buffer_->GetStats();
  EXPECT_EQ(stats_after.total_logs.load(), 0);
  EXPECT_EQ(stats_after.total_flushes.load(), 0);
}

// 测试缓冲区刷新触发条件
TEST_F(WALBufferTest, FlushTrigger) {
  // 添加记录直到触发刷新
  for (int i = 0; i < 50; ++i) {
    auto record = std::make_unique<WALBuffer::WALRecord>(
        i + 1, 1001 + i, "INSERT",
        "INSERT INTO test VALUES (" + std::to_string(i) + ", 'test')");
    wal_buffer_->AddRecord(std::move(record));

    // 每10个记录检查一次
    if ((i + 1) % 10 == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  // 强制刷新以确保处理完成
  wal_buffer_->ForceFlush();

  // 检查刷新次数
  auto stats = wal_buffer_->GetStats();
  EXPECT_GE(stats.total_flushes.load(), 1);
}

// 测试记录格式验证
TEST_F(WALBufferTest, RecordValidation) {
  // 测试有效记录
  auto valid_record = std::make_unique<WALBuffer::WALRecord>(
      1, 1001, "INSERT", "INSERT INTO test VALUES (1, 'test')");
  EXPECT_TRUE(wal_buffer_->AddRecord(std::move(valid_record)));

  // 测试空数据记录
  auto empty_record = std::make_unique<WALBuffer::WALRecord>(
      2, 1002, "INSERT", "");
  EXPECT_TRUE(wal_buffer_->AddRecord(std::move(empty_record)));

  // 测试大记录
  std::string large_data(50 * 1024, 'x'); // 50KB
  auto large_record = std::make_unique<WALBuffer::WALRecord>(
      3, 1003, "INSERT", large_data);
  EXPECT_TRUE(wal_buffer_->AddRecord(std::move(large_record)));

  // 检查统计信息
  auto stats = wal_buffer_->GetStats();
  EXPECT_EQ(stats.total_logs.load(), 3);
}

// 测试边界条件
TEST_F(WALBufferTest, EdgeCases) {
  // 测试空缓冲区刷新
  EXPECT_TRUE(wal_buffer_->ForceFlush());

  // 测试大量小记录
  for (int i = 0; i < 1000; ++i) {
    auto record = std::make_unique<WALBuffer::WALRecord>(
        i + 1, 1001 + i, "INSERT", "INSERT INTO test VALUES (" + std::to_string(i) + ")");
    wal_buffer_->AddRecord(std::move(record));
  }

  // 检查缓冲区状态
  EXPECT_GT(wal_buffer_->GetCurrentSize(), 0);
  EXPECT_TRUE(wal_buffer_->ForceFlush());

  // 检查统计信息
  auto stats = wal_buffer_->GetStats();
  EXPECT_EQ(stats.total_logs.load(), 1000);
  EXPECT_GE(stats.total_flushes.load(), 1);
}

} // namespace test
} // namespace storage
} // namespace sqlcc
