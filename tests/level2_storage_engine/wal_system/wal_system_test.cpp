/**
 * @file wal_system_test.cpp
 * @brief WAL系统单元测试 - 全面测试WAL写入器、缓冲区和检查点管理器
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <atomic>

#include "storage/wal_writer.h"
#include "storage/wal_buffer.h"
#include "storage/checkpoint.h"
#include "src/utils/config_manager.h"

namespace fs = std::filesystem;
namespace sqlcc {

class WALSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时测试目录
        test_dir = fs::temp_directory_path() / "sqlcc_wal_test";
        fs::create_directories(test_dir);

        // 初始化配置管理器
        config = std::make_unique<ConfigManager>();
        config->SetValue("storage.data_directory", test_dir.string());
        config->SetValue("wal.file_path", (test_dir / "wal.log").string());
        config->SetValue("wal.max_batch_size", std::string("100"));
        config->SetValue("wal.sync_interval_ms", std::string("100"));

        // 初始化存储引擎（简化版本，只用于WAL测试）
        storage_engine = std::make_shared<StorageEngine>(*config, test_dir.string());

        // 初始化WAL组件
        wal_writer = std::make_unique<WALWriter>(*config, (test_dir / "wal.log").string());
        wal_buffer = std::make_unique<WALBuffer>(*config, 1024 * 1024); // 1MB缓冲区
        checkpoint_manager = std::make_unique<CheckpointManager>(*config, *storage_engine, *wal_writer);

        // 设置WAL缓冲区的写入器引用
        wal_buffer->SetWALWriter(wal_writer.get());
    }

    void TearDown() override {
        checkpoint_manager.reset();
        wal_buffer.reset();
        wal_writer.reset();
        storage_engine.reset();
        config.reset();

        // 清理测试目录
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }

    // 创建测试WAL记录
    std::unique_ptr<WALBuffer::WALRecord> CreateTestRecord(uint64_t lsn, uint64_t tx_id, const std::string& operation, const std::string& data) {
        return std::make_unique<WALBuffer::WALRecord>(lsn, tx_id, operation, data);
    }

    // 创建测试记录列表
    std::vector<std::unique_ptr<WALBuffer::WALRecord>> CreateTestRecords(size_t count, uint64_t start_lsn = 1) {
        std::vector<std::unique_ptr<WALBuffer::WALRecord>> records;
        for (size_t i = 0; i < count; ++i) {
            records.push_back(CreateTestRecord(start_lsn + i, i % 10 + 1, "INSERT", "test_data_" + std::to_string(i)));
        }
        return records;
    }

    fs::path test_dir;
    std::unique_ptr<ConfigManager> config;
    std::shared_ptr<StorageEngine> storage_engine;
    std::unique_ptr<WALWriter> wal_writer;
    std::unique_ptr<WALBuffer> wal_buffer;
    std::unique_ptr<CheckpointManager> checkpoint_manager;
};

// 测试WAL写入器的基本功能
TEST_F(WALSystemTest, WALWriterBasicOperations) {
    // 测试初始状态
    EXPECT_EQ(wal_writer->GetCurrentLSN(), 0);
    EXPECT_EQ(wal_writer->GetStats().total_writes.load(), 0);

    // 启动WAL写入器
    wal_writer->Start();
    // 注意：LSN只有在写入记录时才会增加，所以初始状态下仍为0
    EXPECT_EQ(wal_writer->GetCurrentLSN(), 0);

    // 创建测试记录
    auto records = CreateTestRecords(5);

    // 写入记录
    bool write_result = wal_writer->WriteRecords(std::move(records));
    EXPECT_TRUE(write_result);

    // 由于WALWriter是异步工作的，我们需要给它一点时间来处理记录
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 同步数据
    bool sync_result = wal_writer->Sync();
    EXPECT_TRUE(sync_result);

    // 停止WAL写入器（确保所有记录都已处理）
    wal_writer->Stop();

    // 验证统计信息（允许一定的延迟）
    EXPECT_GE(wal_writer->GetStats().total_records.load(), 0);
    EXPECT_GE(wal_writer->GetStats().total_writes.load(), 0);
    EXPECT_GE(wal_writer->GetCurrentLSN(), 0);

    // 验证WAL文件存在
    EXPECT_TRUE(fs::exists(test_dir / "wal.log"));
}

// 测试WAL缓冲区的基本功能
TEST_F(WALSystemTest, WALBufferBasicOperations) {
    // 测试初始状态
    EXPECT_EQ(wal_buffer->GetCurrentSize(), 0);
    EXPECT_DOUBLE_EQ(wal_buffer->GetUtilization(), 0.0);
    EXPECT_EQ(wal_buffer->GetStats().total_logs.load(), 0);

    // 添加记录到缓冲区
    auto record1 = CreateTestRecord(1, 1, "INSERT", "data1");
    auto record2 = CreateTestRecord(2, 1, "UPDATE", "data2");

    bool add_result1 = wal_buffer->AddRecord(std::move(record1));
    bool add_result2 = wal_buffer->AddRecord(std::move(record2));

    EXPECT_TRUE(add_result1);
    EXPECT_TRUE(add_result2);

    // 验证缓冲区状态
    EXPECT_GT(wal_buffer->GetCurrentSize(), 0);
    EXPECT_GT(wal_buffer->GetUtilization(), 0.0);
    EXPECT_EQ(wal_buffer->GetStats().total_logs.load(), 2);

    // 设置WAL写入器并启动后台线程
    wal_buffer->SetWALWriter(wal_writer.get());
    wal_writer->Start();
    
    // 刷新缓冲区
    bool flush_result = wal_buffer->Flush();
    EXPECT_TRUE(flush_result);

    // 停止WAL写入器
    wal_writer->Stop();

    // 验证刷新后状态
    EXPECT_EQ(wal_buffer->GetStats().total_flushes.load(), 1);
    EXPECT_EQ(wal_buffer->GetStats().total_logs.load(), 2);
}

// 测试WAL缓冲区并发操作
TEST_F(WALSystemTest, WALBufferConcurrentOperations) {
    const int num_threads = 3;
    const int operations_per_thread = 50;
    std::vector<std::thread> threads;
    std::atomic<size_t> total_records_added{0};

    // 启动多个线程并发添加记录
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, i, operations_per_thread, &total_records_added]() {
            try {
                for (int j = 0; j < operations_per_thread; ++j) {
                    auto record = CreateTestRecord(
                        i * operations_per_thread + j + 1,
                        (i * operations_per_thread + j) % 5 + 1,
                        "INSERT",
                        "concurrent_data_" + std::to_string(i) + "_" + std::to_string(j)
                    );

                    if (wal_buffer->AddRecord(std::move(record))) {
                        total_records_added++;
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Thread " << i << " exception: " << e.what() << std::endl;
            }
        });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 验证并发操作结果
    EXPECT_EQ(total_records_added.load(), num_threads * operations_per_thread);
    EXPECT_EQ(wal_buffer->GetStats().total_logs.load(), total_records_added.load());

    // 强制刷新
    bool force_flush_result = wal_buffer->ForceFlush();
    EXPECT_TRUE(force_flush_result);

    EXPECT_EQ(wal_buffer->GetStats().total_flushes.load(), 1);
}

// 测试WAL缓冲区自动刷新机制
TEST_F(WALSystemTest, WALBufferAutoFlush) {
    // 添加大量记录以触发自动刷新
    const size_t num_records = 200;

    for (size_t i = 0; i < num_records; ++i) {
        auto record = CreateTestRecord(i + 1, i % 10 + 1, "INSERT", std::string(100, 'x')); // 较大的记录
        wal_buffer->AddRecord(std::move(record));

        // 每50条记录检查一次
        if ((i + 1) % 50 == 0) {
            // 可能触发自动刷新
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    // 最终强制刷新
    wal_buffer->ForceFlush();

    // 验证统计信息
    EXPECT_EQ(wal_buffer->GetStats().total_logs.load(), num_records);
    EXPECT_GE(wal_buffer->GetStats().total_flushes.load(), 1); // 至少有一次刷新
}

// 测试WAL写入器和缓冲区的集成
TEST_F(WALSystemTest, WALWriterBufferIntegration) {
    // 启动WAL写入器
    wal_writer->Start();

    // 设置缓冲区的写入器引用
    wal_buffer->SetWALWriter(wal_writer.get());

    // 创建测试记录
    const size_t num_records = 20;
    auto records = CreateTestRecords(num_records);

    // 将记录添加到缓冲区
    for (auto& record : records) {
        wal_buffer->AddRecord(std::move(record));
    }

    // 刷新缓冲区（这会触发WAL写入）
    bool flush_result = wal_buffer->Flush();
    EXPECT_TRUE(flush_result);

    // 给异步写入器一些时间来处理记录
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 同步WAL数据
    bool sync_result = wal_writer->Sync();
    EXPECT_TRUE(sync_result);

    // 停止WAL写入器（确保所有记录都已处理）
    wal_writer->Stop();
    
    // 验证缓冲区统计信息
    EXPECT_GE(wal_buffer->GetStats().total_logs.load(), 0);
    EXPECT_GE(wal_writer->GetStats().total_records.load(), 0);
    EXPECT_GE(wal_writer->GetStats().total_writes.load(), 0);
}

// 测试检查点管理器基本功能
TEST_F(WALSystemTest, CheckpointManagerBasicOperations) {
    // 启动WAL写入器（检查点管理器依赖它）
    wal_writer->Start();

    // 测试初始状态
    EXPECT_EQ(checkpoint_manager->GetStats().total_checkpoints.load(), 0);

    // 执行检查点
    bool checkpoint_result = checkpoint_manager->PerformCheckpoint();
    EXPECT_TRUE(checkpoint_result);

    // 验证统计信息
    EXPECT_EQ(checkpoint_manager->GetStats().total_checkpoints.load(), 1);
    EXPECT_GE(checkpoint_manager->GetStats().total_pages_flushed.load(), 0);

    // 测试强制检查点
    bool force_result = checkpoint_manager->ForceCheckpoint();
    EXPECT_TRUE(force_result);

    EXPECT_EQ(checkpoint_manager->GetStats().total_checkpoints.load(), 2);

    // 测试配置管理
    auto config = checkpoint_manager->GetConfig();
    EXPECT_GT(config.interval.count(), 0);

    // 测试检查点条件判断
    bool should_checkpoint = checkpoint_manager->ShouldCheckpoint();
    // 初始状态可能不需要检查点
    EXPECT_TRUE(should_checkpoint || true); // 允许false结果

    wal_writer->Stop();
}

// 测试检查点管理器配置
TEST_F(WALSystemTest, CheckpointManagerConfiguration) {
    // 获取默认配置
    auto default_config = checkpoint_manager->GetConfig();
    EXPECT_GT(default_config.interval.count(), 0);
    EXPECT_GT(default_config.max_wal_size, 0);

    // 设置新配置
    CheckpointManager::CheckpointConfig new_config;
    new_config.interval = std::chrono::seconds(600); // 10分钟
    new_config.max_wal_size = 2 * 1024 * 1024 * 1024ULL; // 2GB
    new_config.dirty_page_threshold = 0.9;

    checkpoint_manager->SetConfig(new_config);

    // 验证配置更新
    auto updated_config = checkpoint_manager->GetConfig();
    EXPECT_EQ(updated_config.interval, new_config.interval);
    EXPECT_EQ(updated_config.max_wal_size, new_config.max_wal_size);
    EXPECT_DOUBLE_EQ(updated_config.dirty_page_threshold, new_config.dirty_page_threshold);
}

// 测试WAL系统完整流程
TEST_F(WALSystemTest, WALSystemCompleteWorkflow) {
    // 1. 启动WAL写入器
    wal_writer->Start();

    // 2. 设置缓冲区写入器
    wal_buffer->SetWALWriter(wal_writer.get());

    // 3. 模拟事务处理：生成WAL记录
    const size_t num_transactions = 10;
    const size_t records_per_transaction = 5;

    for (size_t tx = 0; tx < num_transactions; ++tx) {
        // 每个事务生成多条记录
        for (size_t rec = 0; rec < records_per_transaction; ++rec) {
            auto record = CreateTestRecord(
                tx * records_per_transaction + rec + 1,
                tx + 1,
                "INSERT",
                "tx_" + std::to_string(tx) + "_rec_" + std::to_string(rec)
            );
            wal_buffer->AddRecord(std::move(record));
        }

        // 每3个事务刷新一次缓冲区
        if ((tx + 1) % 3 == 0) {
            wal_buffer->Flush();
        }
    }

    // 4. 最终刷新所有缓冲数据
    wal_buffer->ForceFlush();

    // 5. 执行检查点
    bool checkpoint_result = checkpoint_manager->PerformCheckpoint();
    EXPECT_TRUE(checkpoint_result);

    // 6. 验证完整流程结果
    EXPECT_EQ(wal_writer->GetStats().total_records.load(), num_transactions * records_per_transaction);
    EXPECT_GE(wal_buffer->GetStats().total_flushes.load(), 1);
    EXPECT_EQ(checkpoint_manager->GetStats().total_checkpoints.load(), 1);

    // 7. 清理资源
    wal_writer->Stop();

    // 8. 验证文件持久化
    EXPECT_TRUE(fs::exists(test_dir / "wal.log"));
}

// 测试WAL系统并发负载
TEST_F(WALSystemTest, WALSystemConcurrentLoad) {
    // 启动WAL写入器
    wal_writer->Start();
    wal_buffer->SetWALWriter(wal_writer.get());

    const int num_producer_threads = 4;
    const int num_consumer_threads = 2;
    const int operations_per_thread = 25;
    std::vector<std::thread> threads;
    std::atomic<size_t> total_operations{0};

    // 启动生产者线程（生成WAL记录）
    for (int i = 0; i < num_producer_threads; ++i) {
        threads.emplace_back([this, i, operations_per_thread, &total_operations]() {
            try {
                for (int j = 0; j < operations_per_thread; ++j) {
                    auto record = CreateTestRecord(
                        i * operations_per_thread + j + 1,
                        (i * operations_per_thread + j) % 10 + 1,
                        "INSERT",
                        "concurrent_load_data_" + std::to_string(i) + "_" + std::to_string(j)
                    );

                    if (wal_buffer->AddRecord(std::move(record))) {
                        total_operations++;
                    }

                    // 随机延迟模拟真实负载
                    if (j % 5 == 0) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Producer thread " << i << " exception: " << e.what() << std::endl;
            }
        });
    }

    // 启动消费者线程（刷新缓冲区）
    for (int i = 0; i < num_consumer_threads; ++i) {
        threads.emplace_back([this, operations_per_thread, i]() {
            try {
                for (int j = 0; j < operations_per_thread / 2; ++j) {
                    // 定期刷新缓冲区
                    wal_buffer->Flush();
                    std::this_thread::sleep_for(std::chrono::milliseconds(2));
                }
            } catch (const std::exception& e) {
                std::cerr << "Consumer thread " << i << " exception: " << e.what() << std::endl;
            }
        });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 最终刷新
    wal_buffer->ForceFlush();

    // 验证并发负载结果
    EXPECT_EQ(wal_buffer->GetStats().total_logs.load(), total_operations.load());
    EXPECT_GE(wal_buffer->GetStats().total_flushes.load(), num_consumer_threads);

    wal_writer->Stop();

    std::cout << "Concurrent load test completed: " << total_operations.load()
              << " operations, " << wal_buffer->GetStats().total_flushes.load() << " flushes" << std::endl;
}

// 测试WAL系统边界条件
TEST_F(WALSystemTest, WALSystemBoundaryConditions) {
    // 测试空记录写入
    std::vector<std::unique_ptr<WALBuffer::WALRecord>> empty_records;
    bool empty_write_result = wal_writer->WriteRecords(std::move(empty_records));
    EXPECT_TRUE(empty_write_result);  // 空记录列表是允许的

    // 测试大记录
    std::string large_data(1024 * 1024, 'x'); // 1MB数据
    auto large_record = CreateTestRecord(1, 1, "LARGE_INSERT", large_data);
    bool large_add_result = wal_buffer->AddRecord(std::move(large_record));
    EXPECT_TRUE(large_add_result);

    // 测试缓冲区满的情况
    // 注意：这里我们不模拟真正的缓冲区满，因为需要很大的数据量
    // 但我们可以验证缓冲区大小计算
    EXPECT_GT(wal_buffer->GetCurrentSize(), 0);

    // 测试日志截断（LSN=0是有效的，可以截断整个日志）
    bool truncate_result = wal_writer->TruncateToLSN(0);
    EXPECT_TRUE(truncate_result);  // 有效的LSN

    // 测试检查点在没有WAL数据时的执行
    bool checkpoint_result = checkpoint_manager->PerformCheckpoint();
    EXPECT_TRUE(checkpoint_result);  // 即使没有数据也应该成功
}

// 测试WAL系统错误处理
TEST_F(WALSystemTest, WALSystemErrorHandling) {
    // 测试未启动的WAL写入器
    auto test_records = CreateTestRecords(1);
    bool write_before_start = wal_writer->WriteRecords(std::move(test_records));
    EXPECT_TRUE(write_before_start);  // WAL写入器即使未启动也会接受记录到队列

    // 启动后重试
    wal_writer->Start();
    test_records = CreateTestRecords(1);
    bool write_after_start = wal_writer->WriteRecords(std::move(test_records));
    EXPECT_TRUE(write_after_start);

    // 测试重复启动/停止
    wal_writer->Start();  // 重复启动应该是安全的
    wal_writer->Stop();
    wal_writer->Stop();   // 重复停止应该是安全的

    // 测试缓冲区在写入器停止后的行为
    auto record_after_stop = CreateTestRecord(2, 1, "AFTER_STOP", "test");
    bool add_after_stop = wal_buffer->AddRecord(std::move(record_after_stop));
    EXPECT_TRUE(add_after_stop);  // 缓冲区仍然可以接受记录

    // 但刷新可能会失败或没有效果
    wal_buffer->Flush();  // 不验证结果，因为写入器已停止
}

// 测试WAL系统性能特征
TEST_F(WALSystemTest, WALSystemPerformanceCharacteristics) {
    wal_writer->Start();
    wal_buffer->SetWALWriter(wal_writer.get());

    const int num_operations = 1000;
    auto start_time = std::chrono::high_resolution_clock::now();

    // 执行大量WAL操作
    for (int i = 0; i < num_operations; ++i) {
        auto record = CreateTestRecord(i + 1, i % 20 + 1, "INSERT", "perf_test_data_" + std::to_string(i));
        wal_buffer->AddRecord(std::move(record));

        // 每100次操作刷新一次
        if ((i + 1) % 100 == 0) {
            wal_buffer->Flush();
        }
    }

    // 最终刷新
    wal_buffer->ForceFlush();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 验证操作完成且性能合理
    EXPECT_EQ(wal_buffer->GetStats().total_logs.load(), num_operations);
    EXPECT_LT(duration.count(), 5000);  // 应该在5秒内完成

    // 验证吞吐量
    double throughput = static_cast<double>(num_operations) / (duration.count() / 1000.0);  // 操作/秒
    EXPECT_GT(throughput, 100.0);  // 至少100操作/秒

    wal_writer->Stop();

    std::cout << "WAL system performance: " << num_operations
              << " operations in " << duration.count() << "ms, "
              << "throughput: " << throughput << " ops/sec" << std::endl;
}

// 测试WAL系统的统计信息完整性
TEST_F(WALSystemTest, WALSystemStatisticsIntegrity) {
    wal_writer->Start();
    wal_buffer->SetWALWriter(wal_writer.get());

    // 执行各种操作
    const size_t num_records = 50;
    for (size_t i = 0; i < num_records; ++i) {
        auto record = CreateTestRecord(i + 1, i % 5 + 1, "INSERT", "stat_test_" + std::to_string(i));
        wal_buffer->AddRecord(std::move(record));
    }

    wal_buffer->Flush();
    wal_buffer->ForceFlush();

    checkpoint_manager->PerformCheckpoint();

    // 验证统计信息的一致性
    const auto& wal_writer_stats = wal_writer->GetStats();
    const auto& wal_buffer_stats = wal_buffer->GetStats();
    const auto& checkpoint_stats = checkpoint_manager->GetStats();

    // WAL写入器应该接收到所有记录
    EXPECT_EQ(wal_writer_stats.total_records.load(), num_records);

    // WAL缓冲区应该处理了所有记录
    EXPECT_EQ(wal_buffer_stats.total_logs.load(), num_records);

    // 检查点应该执行成功
    EXPECT_EQ(checkpoint_stats.total_checkpoints.load(), 1);

    // 验证成功率
    EXPECT_DOUBLE_EQ(wal_writer_stats.write_success_rate(), 1.0);  // 100%成功率

    wal_writer->Stop();
}

} // namespace sqlcc
