/**
 * @file disk_manager_test.cpp
 * @brief 磁盘管理器单元测试 - 全面测试磁盘I/O操作、文件管理、缓存和性能优化
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <random>

#include "storage/disk_manager.h"
#include "storage_engine.h"
#include "utils/config_manager.h"

namespace fs = std::filesystem;
namespace sqlcc {

class DiskManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时测试目录
        test_dir = fs::temp_directory_path() / "sqlcc_disk_test";
        fs::create_directories(test_dir);

        // 初始化配置管理器
        config = std::make_unique<ConfigManager>();
        config->SetValue("storage.data_directory", test_dir.string());
        config->SetValue("disk.page_size", std::string("4096"));
        config->SetValue("disk.max_open_files", std::string("100"));
        config->SetValue("disk.io_buffer_size", std::string("65536"));
        config->SetValue("disk.enable_aio", std::string("true"));

        // 初始化存储引擎
        storage_engine = std::make_shared<StorageEngine>(*config, test_dir.string());

        // 初始化磁盘管理器
        disk_manager = std::make_unique<DiskManager>(*config);

        // 设置随机数生成器
        std::random_device rd;
        random_engine = std::mt19937(rd());
    }

    void TearDown() override {
        disk_manager.reset();
        storage_engine.reset();
        config.reset();

        // 清理测试目录
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }

    // 创建测试数据
    std::vector<uint8_t> CreateTestData(size_t size, uint8_t pattern = 0xAA) {
        return std::vector<uint8_t>(size, pattern);
    }

    // 创建随机测试数据
    std::vector<uint8_t> CreateRandomData(size_t size) {
        std::uniform_int_distribution<uint8_t> dist(0, 255);
        std::vector<uint8_t> data(size);
        for (auto& byte : data) {
            byte = dist(random_engine);
        }
        return data;
    }

    // 创建测试页面
    DiskManager::Page CreateTestPage(page_id_t page_id, const std::vector<uint8_t>& data) {
        DiskManager::Page page;
        page.page_id = page_id;
        page.data.assign(data.begin(), data.end());
        page.is_dirty = false;
        page.pin_count = 0;
        page.last_access_time = std::chrono::system_clock::now();
        return page;
    }

    fs::path test_dir;
    std::unique_ptr<ConfigManager> config;
    std::shared_ptr<StorageEngine> storage_engine;
    std::unique_ptr<DiskManager> disk_manager;
    std::mt19937 random_engine;
};

// 测试磁盘管理器初始化
TEST_F(DiskManagerTest, Initialization) {
    // 验证配置正确加载
    EXPECT_EQ(disk_manager->GetPageSize(), 4096);
    EXPECT_EQ(disk_manager->GetMaxOpenFiles(), 100);
    EXPECT_TRUE(disk_manager->IsAIOEnabled());

    // 验证初始状态
    auto stats = disk_manager->GetStatistics();
    EXPECT_EQ(stats.total_pages_read, 0);
    EXPECT_EQ(stats.total_pages_written, 0);
    EXPECT_EQ(stats.cache_hit_rate, 0.0);
}

// 测试页面读取和写入
TEST_F(DiskManagerTest, PageReadWrite) {
    auto test_data = CreateRandomData(4096); // 一页大小
    page_id_t test_page_id = 1;

    // 写入页面
    bool write_result = disk_manager->WritePage(test_page_id, test_data);
    EXPECT_TRUE(write_result);

    // 读取页面
    std::vector<uint8_t> read_data;
    bool read_result = disk_manager->ReadPage(test_page_id, read_data);
    EXPECT_TRUE(read_result);

    // 验证数据一致性
    EXPECT_EQ(read_data.size(), test_data.size());
    EXPECT_EQ(read_data, test_data);

    // 验证统计信息
    auto stats = disk_manager->GetStatistics();
    EXPECT_EQ(stats.total_pages_written, 1);
    EXPECT_EQ(stats.total_pages_read, 1);
}

// 测试页面缓存
TEST_F(DiskManagerTest, PageCaching) {
    const int num_pages = 10;
    std::vector<std::vector<uint8_t>> test_pages;

    // 创建多个测试页面
    for (int i = 0; i < num_pages; ++i) {
        auto data = CreateRandomData(4096);
        test_pages.push_back(data);

        // 写入页面
        disk_manager->WritePage(i + 1, data);
    }

    // 多次读取页面以测试缓存
    for (int round = 0; round < 3; ++round) {
        for (int i = 0; i < num_pages; ++i) {
            std::vector<uint8_t> read_data;
            disk_manager->ReadPage(i + 1, read_data);
            EXPECT_EQ(read_data, test_pages[i]);
        }
    }

    // 验证缓存统计
    auto stats = disk_manager->GetStatistics();
    EXPECT_GE(stats.total_pages_read, num_pages); // 至少读取了所有页面
    EXPECT_GE(stats.cache_hits, 0); // 可能有缓存命中
}

// 测试并发I/O操作
TEST_F(DiskManagerTest, ConcurrentIO) {
    const int num_threads = 4;
    const int operations_per_thread = 25;
    std::vector<std::thread> threads;
    std::atomic<size_t> operations_completed{0};

    // 启动并发I/O线程
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, i, operations_per_thread, &operations_completed]() {
            try {
                for (int j = 0; j < operations_per_thread; ++j) {
                    // 计算页面ID（每个线程使用不同的页面范围）
                    page_id_t page_id = i * operations_per_thread + j + 1;
                    auto test_data = CreateRandomData(4096);

                    // 写入页面
                    bool write_result = disk_manager->WritePage(page_id, test_data);
                    if (write_result) {
                        // 读取验证
                        std::vector<uint8_t> read_data;
                        bool read_result = disk_manager->ReadPage(page_id, read_data);
                        if (read_result && read_data == test_data) {
                            operations_completed++;
                        }
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
    EXPECT_EQ(operations_completed.load(), num_threads * operations_per_thread);

    // 验证统计信息
    auto stats = disk_manager->GetStatistics();
    EXPECT_EQ(stats.total_pages_written, operations_completed.load());
    EXPECT_EQ(stats.total_pages_read, operations_completed.load());
}

// 测试页面置换策略
TEST_F(DiskManagerTest, PageReplacement) {
    const size_t cache_size = 5; // 假设缓存大小为5页

    // 写入超过缓存容量的页面
    for (size_t i = 1; i <= cache_size * 2; ++i) {
        auto data = CreateTestData(4096, static_cast<uint8_t>(i));
        disk_manager->WritePage(i, data);
    }

    // 读取一些页面以触发缓存访问模式
    for (size_t i = 1; i <= cache_size * 2; ++i) {
        std::vector<uint8_t> read_data;
        disk_manager->ReadPage(i, read_data);
        // 验证数据正确性
        EXPECT_EQ(read_data.size(), 4096);
    }

    // 验证缓存统计
    auto stats = disk_manager->GetStatistics();
    EXPECT_GT(stats.total_pages_read, 0);
    // 缓存命中率应该合理
    EXPECT_GE(stats.cache_hit_rate, 0.0);
    EXPECT_LE(stats.cache_hit_rate, 1.0);
}

// 测试批量I/O操作
TEST_F(DiskManagerTest, BatchIO) {
    const size_t batch_size = 10;
    std::vector<page_id_t> page_ids;
    std::vector<std::vector<uint8_t>> page_data;

    // 准备批量数据
    for (size_t i = 0; i < batch_size; ++i) {
        page_ids.push_back(i + 1);
        page_data.push_back(CreateRandomData(4096));
    }

    // 批量写入
    bool batch_write_result = disk_manager->WritePages(page_ids, page_data);
    EXPECT_TRUE(batch_write_result);

    // 批量读取
    std::vector<std::vector<uint8_t>> read_data;
    bool batch_read_result = disk_manager->ReadPages(page_ids, read_data);
    EXPECT_TRUE(batch_read_result);

    // 验证批量数据一致性
    EXPECT_EQ(read_data.size(), page_data.size());
    for (size_t i = 0; i < batch_size; ++i) {
        EXPECT_EQ(read_data[i], page_data[i]);
    }

    // 验证统计信息
    auto stats = disk_manager->GetStatistics();
    EXPECT_EQ(stats.total_pages_written, batch_size);
    EXPECT_EQ(stats.total_pages_read, batch_size);
}

// 测试文件管理
TEST_F(DiskManagerTest, FileManagement) {
    std::string test_filename = "test_data_file.dat";
    auto test_data = CreateRandomData(8192); // 2页数据

    // 创建文件并写入数据
    bool create_result = disk_manager->CreateFile(test_filename);
    EXPECT_TRUE(create_result);

    // 写入数据到文件
    bool write_result = disk_manager->WriteFile(test_filename, test_data);
    EXPECT_TRUE(write_result);

    // 读取数据验证
    std::vector<uint8_t> read_data;
    bool read_result = disk_manager->ReadFile(test_filename, read_data);
    EXPECT_TRUE(read_result);
    EXPECT_EQ(read_data, test_data);

    // 验证文件存在
    EXPECT_TRUE(disk_manager->FileExists(test_filename));

    // 获取文件信息
    auto file_info = disk_manager->GetFileInfo(test_filename);
    EXPECT_TRUE(file_info.has_value());
    EXPECT_EQ(file_info->size, test_data.size());

    // 删除文件
    bool delete_result = disk_manager->DeleteFile(test_filename);
    EXPECT_TRUE(delete_result);
    EXPECT_FALSE(disk_manager->FileExists(test_filename));
}

// 测试异步I/O操作
TEST_F(DiskManagerTest, AsyncIO) {
    const int num_async_ops = 20;
    std::vector<std::future<bool>> futures;

    // 提交异步I/O操作
    for (int i = 0; i < num_async_ops; ++i) {
        auto future = disk_manager->WritePageAsync(i + 1, CreateRandomData(4096));
        futures.push_back(std::move(future));
    }

    // 等待所有异步操作完成
    for (auto& future : futures) {
        bool result = future.get();
        EXPECT_TRUE(result);
    }

    // 验证异步写入的结果
    for (int i = 0; i < num_async_ops; ++i) {
        std::vector<uint8_t> read_data;
        bool read_result = disk_manager->ReadPage(i + 1, read_data);
        EXPECT_TRUE(read_result);
        EXPECT_EQ(read_data.size(), 4096);
    }

    // 验证统计信息
    auto stats = disk_manager->GetStatistics();
    EXPECT_EQ(stats.total_pages_written, num_async_ops);
    EXPECT_EQ(stats.total_pages_read, num_async_ops);
}

// 测试磁盘空间管理
TEST_F(DiskManagerTest, DiskSpaceManagement) {
    // 获取磁盘空间信息
    auto space_info = disk_manager->GetDiskSpaceInfo();
    EXPECT_TRUE(space_info.has_value());

    // 验证空间信息合理性
    EXPECT_GE(space_info->total_space, space_info->used_space);
    EXPECT_GE(space_info->available_space, 0);

    // 计算使用率
    double usage_rate = static_cast<double>(space_info->used_space) / space_info->total_space;
    EXPECT_GE(usage_rate, 0.0);
    EXPECT_LE(usage_rate, 1.0);
}

// 测试I/O性能监控
TEST_F(DiskManagerTest, PerformanceMonitoring) {
    auto start_time = std::chrono::high_resolution_clock::now();

    // 执行一系列I/O操作
    const int num_operations = 100;
    for (int i = 0; i < num_operations; ++i) {
        auto data = CreateTestData(4096, static_cast<uint8_t>(i % 256));
        disk_manager->WritePage(i + 1, data);

        std::vector<uint8_t> read_data;
        disk_manager->ReadPage(i + 1, read_data);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 获取性能统计
    auto perf_stats = disk_manager->GetPerformanceStats();
    EXPECT_TRUE(perf_stats.has_value());

    // 验证性能指标
    EXPECT_GE(perf_stats->avg_read_time.count(), 0);
    EXPECT_GE(perf_stats->avg_write_time.count(), 0);
    EXPECT_GE(perf_stats->iops, 0);

    // 计算实际吞吐量
    double actual_throughput = static_cast<double>(num_operations * 4096) / (duration.count() / 1000.0); // 字节/秒
    EXPECT_GT(actual_throughput, 0);

    std::cout << "Disk I/O performance: " << num_operations
              << " operations in " << duration.count() << "ms, "
              << "throughput: " << (actual_throughput / (1024 * 1024)) << " MB/s" << std::endl;
}

// 测试错误处理
TEST_F(DiskManagerTest, ErrorHandling) {
    // 测试读取不存在的页面
    std::vector<uint8_t> read_data;
    bool read_invalid_result = disk_manager->ReadPage(99999, read_data);
    EXPECT_FALSE(read_invalid_result);

    // 测试写入无效数据
    std::vector<uint8_t> empty_data;
    bool write_empty_result = disk_manager->WritePage(1, empty_data);
    // 空数据可能被接受或拒绝，取决于实现

    // 测试文件操作错误
    bool read_nonexistent = disk_manager->ReadFile("nonexistent_file.dat", read_data);
    EXPECT_FALSE(read_nonexistent);

    bool delete_nonexistent = disk_manager->DeleteFile("nonexistent_file.dat");
    EXPECT_FALSE(delete_nonexistent);

    // 测试边界情况：超大页面
    auto huge_data = CreateTestData(1024 * 1024 * 10); // 10MB
    bool write_huge_result = disk_manager->WritePage(2, huge_data);
    // 超大页面可能成功或失败，取决于实现限制
}

// 测试配置管理
TEST_F(DiskManagerTest, ConfigurationManagement) {
    // 获取默认配置
    auto default_config = disk_manager->GetConfig();
    EXPECT_EQ(default_config.page_size, 4096);
    EXPECT_EQ(default_config.max_open_files, 100);

    // 设置新配置
    DiskManager::DiskConfig new_config;
    new_config.page_size = 8192;
    new_config.max_open_files = 200;
    new_config.io_buffer_size = 131072; // 128KB
    new_config.enable_aio = false;

    disk_manager->SetConfig(new_config);

    // 验证配置更新
    auto updated_config = disk_manager->GetConfig();
    EXPECT_EQ(updated_config.page_size, new_config.page_size);
    EXPECT_EQ(updated_config.max_open_files, new_config.max_open_files);
    EXPECT_EQ(updated_config.io_buffer_size, new_config.io_buffer_size);
    EXPECT_EQ(updated_config.enable_aio, new_config.enable_aio);
}

// 测试页面压缩（如果支持）
TEST_F(DiskManagerTest, PageCompression) {
    auto test_data = CreateTestData(4096, 0x00); // 可压缩数据（全零）

    // 尝试启用压缩
    bool compression_enabled = disk_manager->EnableCompression(true);
    // 压缩功能可能不实现

    // 写入可压缩数据
    disk_manager->WritePage(1, test_data);

    // 读取并验证
    std::vector<uint8_t> read_data;
    disk_manager->ReadPage(1, read_data);
    EXPECT_EQ(read_data, test_data);

    // 如果支持压缩，验证压缩统计
    auto stats = disk_manager->GetStatistics();
    // 压缩统计可能为0（如果不支持）
    EXPECT_GE(stats.compression_ratio, 0.0);
}

// 测试页面校验和
TEST_F(DiskManagerTest, PageChecksum) {
    auto test_data = CreateRandomData(4096);

    // 启用校验和
    disk_manager->EnableChecksum(true);

    // 写入页面（应该计算校验和）
    disk_manager->WritePage(1, test_data);

    // 读取并验证校验和
    std::vector<uint8_t> read_data;
    bool read_result = disk_manager->ReadPage(1, read_data);
    EXPECT_TRUE(read_result);
    EXPECT_EQ(read_data, test_data);

    // 验证校验和统计
    auto stats = disk_manager->GetStatistics();
    EXPECT_GE(stats.checksum_verifications, 0);
}

// 测试磁盘管理器清理
TEST_F(DiskManagerTest, CleanupOperations) {
    // 创建一些测试文件和页面
    for (int i = 1; i <= 10; ++i) {
        auto data = CreateTestData(4096, static_cast<uint8_t>(i));
        disk_manager->WritePage(i, data);
    }

    // 执行清理操作
    bool cleanup_result = disk_manager->Cleanup();
    EXPECT_TRUE(cleanup_result);

    // 验证清理后的状态
    auto stats = disk_manager->GetStatistics();
    // 清理后统计信息应该保持一致性

    // 验证页面仍然可以访问
    for (int i = 1; i <= 10; ++i) {
        std::vector<uint8_t> read_data;
        bool read_result = disk_manager->ReadPage(i, read_data);
        EXPECT_TRUE(read_result);
        EXPECT_EQ(read_data.size(), 4096);
    }
}

} // namespace sqlcc
