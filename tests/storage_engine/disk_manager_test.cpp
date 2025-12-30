/**
 * @file disk_manager_test.cpp
 * @brief 磁盘管理器单元测试 - 测试基本的磁盘I/O操作
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

#include "disk_manager.h"
#include "utils/config_manager.h"

namespace fs = std::filesystem;
namespace sqlcc {

class DiskManagerTest : public ::testing::Test {
protected:
    static const size_t PAGE_SIZE = 8192;

    void SetUp() override {
        // 创建临时测试目录
        test_dir = fs::temp_directory_path() / "sqlcc_disk_test";
        fs::create_directories(test_dir);

        // 初始化配置管理器
        config = std::make_unique<ConfigManager>();
        config->SetValue("storage.data_directory", test_dir.string());
        config->SetValue("storage.page_size", std::to_string(PAGE_SIZE));

        // 初始化磁盘管理器
        db_file_name = (test_dir / "test_db").string();
        disk_manager = std::make_unique<DiskManager>(db_file_name, *config);

        // 设置随机数生成器
        std::random_device rd;
        random_engine = std::mt19937(rd());
    }

    void TearDown() override {
        disk_manager.reset();
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

    fs::path test_dir;
    std::string db_file_name;
    std::unique_ptr<ConfigManager> config;
    std::unique_ptr<DiskManager> disk_manager;
    std::mt19937 random_engine;
};

// 测试磁盘管理器初始化
TEST_F(DiskManagerTest, Initialization) {
    // 验证初始状态
    EXPECT_NE(disk_manager, nullptr);
    EXPECT_EQ(disk_manager->GetFileSize(), 0); // 新创建的文件大小为0
}

// 测试页面分配
TEST_F(DiskManagerTest, PageAllocation) {
    // 测试分配页面
    int32_t page_id1 = disk_manager->AllocatePage();
    EXPECT_NE(page_id1, -1);
    EXPECT_EQ(page_id1, 0); // 第一个页面ID应该是0

    // 测试连续分配页面
    int32_t page_id2 = disk_manager->AllocatePage();
    EXPECT_EQ(page_id2, 1);

    int32_t page_id3 = disk_manager->AllocatePage();
    EXPECT_EQ(page_id3, 2);

    // 验证文件大小增长
    EXPECT_GE(disk_manager->GetFileSize(), 3); // 至少有3个页面
}

// 测试页面写入和读取
TEST_F(DiskManagerTest, PageReadWrite) {
    // 创建测试数据
    auto test_data = CreateRandomData(PAGE_SIZE);
    char* test_data_ptr = reinterpret_cast<char*>(test_data.data());

    // 分配页面
    int32_t page_id = disk_manager->AllocatePage();
    EXPECT_NE(page_id, -1);

    // 写入页面
    bool write_result = disk_manager->WritePage(page_id, test_data_ptr);
    EXPECT_TRUE(write_result);

    // 读取页面
    std::vector<uint8_t> read_data(PAGE_SIZE);
    char* read_data_ptr = reinterpret_cast<char*>(read_data.data());
    bool read_result = disk_manager->ReadPage(page_id, read_data_ptr);
    EXPECT_TRUE(read_result);

    // 验证数据一致性
    EXPECT_EQ(memcmp(test_data_ptr, read_data_ptr, PAGE_SIZE), 0);
}

// 测试批量读取页面
TEST_F(DiskManagerTest, BatchReadPages) {
    // 分配多个页面并写入数据
    const int NUM_PAGES = 5;
    std::vector<int32_t> page_ids;
    std::vector<std::vector<uint8_t>> test_datas;

    for (int i = 0; i < NUM_PAGES; ++i) {
        int32_t page_id = disk_manager->AllocatePage();
        page_ids.push_back(page_id);

        auto test_data = CreateRandomData(PAGE_SIZE);
        test_datas.push_back(std::move(test_data));

        // 写入页面
        bool write_result = disk_manager->WritePage(page_id, reinterpret_cast<char*>(test_datas.back().data()));
        EXPECT_TRUE(write_result);
    }

    // 批量读取页面
    std::vector<char*> page_data_ptrs;
    for (int i = 0; i < NUM_PAGES; ++i) {
        page_data_ptrs.push_back(new char[PAGE_SIZE]);
    }

    bool batch_read_result = disk_manager->BatchReadPages(page_ids, page_data_ptrs);
    EXPECT_TRUE(batch_read_result);

    // 验证数据一致性
    for (int i = 0; i < NUM_PAGES; ++i) {
        EXPECT_EQ(memcmp(page_data_ptrs[i], test_datas[i].data(), PAGE_SIZE), 0);
        delete[] page_data_ptrs[i];
    }
}

// 测试预取页面
TEST_F(DiskManagerTest, PrefetchPage) {
    // 分配页面并写入数据
    int32_t page_id = disk_manager->AllocatePage();
    auto test_data = CreateRandomData(PAGE_SIZE);
    bool write_result = disk_manager->WritePage(page_id, reinterpret_cast<char*>(test_data.data()));
    EXPECT_TRUE(write_result);

    // 预取页面
    bool prefetch_result = disk_manager->PrefetchPage(page_id);
    EXPECT_TRUE(prefetch_result);

    // 验证页面可以正常读取
    std::vector<uint8_t> read_data(PAGE_SIZE);
    bool read_result = disk_manager->ReadPage(page_id, reinterpret_cast<char*>(read_data.data()));
    EXPECT_TRUE(read_result);
    EXPECT_EQ(memcmp(read_data.data(), test_data.data(), PAGE_SIZE), 0);
}

// 测试页面释放
TEST_F(DiskManagerTest, PageDeallocation) {
    // 分配页面
    int32_t page_id = disk_manager->AllocatePage();
    EXPECT_NE(page_id, -1);

    // 释放页面
    bool dealloc_result = disk_manager->DeallocatePage(page_id);
    EXPECT_TRUE(dealloc_result);

    // 验证页面可以重新分配
    int32_t new_page_id = disk_manager->AllocatePage();
    EXPECT_EQ(new_page_id, page_id); // 应该重新使用已释放的页面ID
}

// 测试同步操作
TEST_F(DiskManagerTest, SyncOperation) {
    // 分配并写入页面
    int32_t page_id = disk_manager->AllocatePage();
    auto test_data = CreateRandomData(PAGE_SIZE);
    bool write_result = disk_manager->WritePage(page_id, reinterpret_cast<char*>(test_data.data()));
    EXPECT_TRUE(write_result);

    // 同步到磁盘
    bool sync_result = disk_manager->Sync();
    EXPECT_TRUE(sync_result);
}

// 测试I/O统计信息已移除，因为DiskManager类中没有对应的方法实现

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
                    // 分配页面
                    int32_t page_id = disk_manager->AllocatePage();
                    EXPECT_NE(page_id, -1);

                    // 创建测试数据
                    auto test_data = CreateRandomData(PAGE_SIZE);
                    char* test_data_ptr = reinterpret_cast<char*>(test_data.data());

                    // 写入页面
                    bool write_result = disk_manager->WritePage(page_id, test_data_ptr);
                    if (write_result) {
                        // 读取验证
                        std::vector<uint8_t> read_data(PAGE_SIZE);
                        char* read_data_ptr = reinterpret_cast<char*>(read_data.data());
                        bool read_result = disk_manager->ReadPage(page_id, read_data_ptr);
                        if (read_result && memcmp(test_data_ptr, read_data_ptr, PAGE_SIZE) == 0) {
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
}

// 测试错误模拟功能
TEST_F(DiskManagerTest, ErrorSimulation) {
    // 设置模拟读取失败
    disk_manager->SetSimulateReadFailure(true);

    // 分配并写入页面
    int32_t page_id = disk_manager->AllocatePage();
    auto test_data = CreateRandomData(PAGE_SIZE);
    bool write_result = disk_manager->WritePage(page_id, reinterpret_cast<char*>(test_data.data()));
    EXPECT_TRUE(write_result);

    // 读取页面应该失败
    std::vector<uint8_t> read_data(PAGE_SIZE);
    bool read_result = disk_manager->ReadPage(page_id, reinterpret_cast<char*>(read_data.data()));
    EXPECT_FALSE(read_result);

    // 关闭模拟失败
    disk_manager->SetSimulateReadFailure(false);

    // 读取页面应该成功
    read_result = disk_manager->ReadPage(page_id, reinterpret_cast<char*>(read_data.data()));
    EXPECT_TRUE(read_result);
}

} // namespace sqlcc
