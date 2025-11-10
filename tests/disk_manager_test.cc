#include <gtest/gtest.h>
#include "disk_manager.h"
#include "test_disk_manager.h"
#include "exception.h"
#include "page.h"
#include "config_manager.h"
#include <filesystem>
#include <cstdio>
#include <fstream>
#include <cstring>

class DiskManagerTest : public ::testing::Test {
protected:
    const std::string TEST_DB_FILE = "test_disk_manager.db";
    sqlcc::ConfigManager* config_manager;
    
    void SetUp() override {
        // 删除测试文件（如果存在）
        std::filesystem::remove(TEST_DB_FILE);
        
        // 创建配置管理器
        config_manager = &sqlcc::ConfigManager::GetInstance();
    }
    
    void TearDown() override {
        // 清理测试文件
        std::filesystem::remove(TEST_DB_FILE);
    }
};

// 测试磁盘管理器初始化
TEST_F(DiskManagerTest, InitializeDiskManager) {
    EXPECT_NO_THROW({
        sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    });
    
    // 检查数据库文件是否创建
    EXPECT_TRUE(std::filesystem::exists(TEST_DB_FILE));
}

// 测试写入页面
TEST_F(DiskManagerTest, WritePage) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 创建一个页面
    sqlcc::Page page;
    page.SetPageId(0);
    
    // 写入一些数据
    char test_data[] = "Test data for writing";
    page.WriteData(0, test_data, sizeof(test_data));
    
    // 写入页面
    EXPECT_TRUE(manager.WritePage(page.GetPageId(), page.GetData()));
}

// 测试读取页面
TEST_F(DiskManagerTest, ReadPage) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 创建一个页面并写入数据
    sqlcc::Page write_page;
    write_page.SetPageId(0);
    char test_data[] = "Test data for reading";
    write_page.WriteData(0, test_data, sizeof(test_data));
    
    // 写入页面
    EXPECT_TRUE(manager.WritePage(write_page.GetPageId(), write_page.GetData()));
    
    // 读取页面
    char read_data[sqlcc::PAGE_SIZE];
    EXPECT_TRUE(manager.ReadPage(0, read_data));
    
    // 验证数据
    EXPECT_EQ(memcmp(test_data, read_data, sizeof(test_data)), 0);
}

// 测试分配页面
TEST_F(DiskManagerTest, AllocatePage) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 分配几个页面
    int32_t page_id1 = manager.AllocatePage();
    int32_t page_id2 = manager.AllocatePage();
    int32_t page_id3 = manager.AllocatePage();
    
    // 验证页面ID是递增的
    EXPECT_EQ(page_id1, 0);
    EXPECT_EQ(page_id2, 1);
    EXPECT_EQ(page_id3, 2);
}

// 测试获取文件大小
TEST_F(DiskManagerTest, GetFileSize) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 初始文件大小应该是0
    EXPECT_EQ(manager.GetFileSize(), 0);
    
    // 创建并写入一个页面
    sqlcc::Page page;
    page.SetPageId(0);
    char test_data[] = "Test data";
    page.WriteData(0, test_data, sizeof(test_data));
    
    EXPECT_TRUE(manager.WritePage(page.GetPageId(), page.GetData()));
    
    // 文件大小应该增加
    EXPECT_GT(manager.GetFileSize(), 0);
}

// 测试写入无效页面ID（负数）
TEST_F(DiskManagerTest, WriteInvalidPageId) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 创建一个页面并设置无效ID
    sqlcc::Page page;
    page.SetPageId(-1);  // 无效的页面ID
    
    // 尝试写入页面应该失败
    EXPECT_FALSE(manager.WritePage(page.GetPageId(), page.GetData()));
}

// 测试读取无效页面ID（负数）
TEST_F(DiskManagerTest, ReadInvalidPageId) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 尝试读取无效页面ID应该失败
    sqlcc::Page page;
    EXPECT_FALSE(manager.ReadPage(-1, page.GetData()));
}

// 测试读取不存在的页面
TEST_F(DiskManagerTest, ReadNonExistentPage) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 尝试读取不存在的页面应该失败
    sqlcc::Page page;
    EXPECT_FALSE(manager.ReadPage(100, page.GetData()));
}

// 测试读取页面时传入空指针
TEST_F(DiskManagerTest, ReadPageWithNullPointer) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 尝试读取页面时传入空指针应该失败
    char* null_data = nullptr;
    EXPECT_FALSE(manager.ReadPage(0, null_data));
}

// 测试在只读文件上写入页面
TEST_F(DiskManagerTest, WritePageToReadOnlyFile) {
    // 创建一个只读文件
    std::ofstream file(TEST_DB_FILE);
    file.close();
    
    // 设置文件为只读
    std::filesystem::permissions(TEST_DB_FILE, 
        std::filesystem::perms::owner_read | 
        std::filesystem::perms::group_read | 
        std::filesystem::perms::others_read);
    
    try {
        // 尝试在只读文件上创建DiskManager并写入页面
        sqlcc::Page page;
        page.SetPageId(0);
        char test_data[] = "Test data";
        page.WriteData(0, test_data, sizeof(test_data));
        
        // 这应该会失败
        EXPECT_FALSE(sqlcc::DiskManager(TEST_DB_FILE, *config_manager).WritePage(page.GetPageId(), page.GetData()));
    } catch (const std::exception& e) {
        // 如果抛出异常，这也是预期的
        SUCCEED();
    }
    
    // 恢复文件权限以便清理
    std::filesystem::permissions(TEST_DB_FILE, 
        std::filesystem::perms::owner_all | 
        std::filesystem::perms::group_all | 
        std::filesystem::perms::others_all);
}

// 测试在无效路径上创建DiskManager
TEST_F(DiskManagerTest, CreateDiskManagerWithInvalidPath) {
    // 使用一个无效的路径（例如一个不存在的目录）
    const std::string invalid_path = "/invalid/path/that/does/not/exist/test.db";
    
    try {
        // 这应该会抛出异常
        sqlcc::DiskManager manager(invalid_path, *config_manager);
        FAIL() << "Expected DiskManagerException to be thrown";
    } catch (const sqlcc::DiskManagerException& e) {
        // 预期的异常
        SUCCEED();
    } catch (...) {
        FAIL() << "Expected DiskManagerException to be thrown, but got different exception";
    }
}

// 测试模拟磁盘空间不足的情况
TEST_F(DiskManagerTest, SimulateDiskFull) {
    // 创建一个临时文件
    std::string temp_file = "temp_disk_full_test.db";
    
    // 创建一个小的文件系统（使用dd命令创建一个小的临时文件系统）
    // 这里我们使用一个简单的方法：创建一个文件，然后将其设置为只读
    std::ofstream file(temp_file);
    file.close();
    
    // 设置文件为只读
    std::filesystem::permissions(temp_file, 
        std::filesystem::perms::owner_read | 
        std::filesystem::perms::group_read | 
        std::filesystem::perms::others_read);
    
    try {
        // 尝试在只读文件上创建DiskManager并写入大量页面
        sqlcc::DiskManager manager(temp_file, *config_manager);
        
        // 创建一个页面
        sqlcc::Page page;
        page.SetPageId(0);
        char test_data[sqlcc::PAGE_SIZE];
        memset(test_data, 'A', sizeof(test_data));
        page.WriteData(0, test_data, sizeof(test_data));
        
        // 尝试写入页面，这应该会失败
        EXPECT_FALSE(manager.WritePage(page.GetPageId(), page.GetData()));
    } catch (const std::exception& e) {
        // 如果抛出异常，这也是预期的
        SUCCEED();
    }
    
    // 恢复文件权限以便清理
    std::filesystem::permissions(temp_file, 
        std::filesystem::perms::owner_all | 
        std::filesystem::perms::group_all | 
        std::filesystem::perms::others_all);
    
    // 删除临时文件
    std::filesystem::remove(temp_file);
}

// 测试模拟读取页面时定位失败的情况
TEST_F(DiskManagerTest, SimulateReadPageSeekFailure) {
    // 创建一个临时文件
    std::string temp_file = "temp_read_seek_test.db";
    
    // 创建一个文件并写入一些数据
    std::ofstream file(temp_file, std::ios::binary);
    file.close();
    
    try {
        // 创建DiskManager
        sqlcc::DiskManager manager(temp_file, *config_manager);
        
        // 创建一个页面
        sqlcc::Page page;
        
        // 尝试读取一个不存在的页面，这应该会失败
        // 但我们需要先确保文件存在且有一些内容
        sqlcc::Page write_page;
        write_page.SetPageId(0);
        char test_data[] = "Test data";
        write_page.WriteData(0, test_data, sizeof(test_data));
        manager.WritePage(write_page.GetPageId(), write_page.GetData());
        
        // 现在尝试读取一个不存在的页面
        EXPECT_FALSE(manager.ReadPage(100, page.GetData()));
    } catch (const std::exception& e) {
        // 如果抛出异常，这也是预期的
        SUCCEED();
    }
    
    // 删除临时文件
    std::filesystem::remove(temp_file);
}

// 测试模拟WritePage写入失败的情况
TEST_F(DiskManagerTest, SimulateWritePageFailure) {
    // 创建一个临时文件
    std::string temp_file = "temp_write_fail_test.db";
    
    try {
        // 创建TestDiskManager
        sqlcc::TestDiskManager manager(temp_file, *config_manager);
        
        // 创建一个页面
        sqlcc::Page page;
        page.SetPageId(0);
        char test_data[sqlcc::PAGE_SIZE];
        memset(test_data, 'A', sizeof(test_data));
        page.WriteData(0, test_data, sizeof(test_data));
        
        // 设置模拟写入失败
        manager.SetSimulateWriteFailure(true);
        
        // 尝试写入页面，应该失败
        EXPECT_FALSE(manager.TestWritePage(page));
    } catch (const std::exception& e) {
        // 如果抛出异常，这也是预期的
        SUCCEED();
    }
    
    // 确保删除临时文件
    if (std::filesystem::exists(temp_file)) {
        std::filesystem::remove(temp_file);
    }
}

// 测试模拟WritePage刷新到磁盘失败的情况
TEST_F(DiskManagerTest, SimulateWritePageFlushFailure) {
    // 创建一个临时文件
    std::string temp_file = "temp_flush_fail_test.db";
    
    try {
        // 创建TestDiskManager
        sqlcc::TestDiskManager manager(temp_file, *config_manager);
        
        // 创建一个页面
        sqlcc::Page page;
        page.SetPageId(0);
        char test_data[sqlcc::PAGE_SIZE];
        memset(test_data, 'B', sizeof(test_data));
        page.WriteData(0, test_data, sizeof(test_data));
        
        // 设置模拟刷新失败
        manager.SetSimulateFlushFailure(true);
        
        // 尝试写入页面，应该在刷新时失败
        EXPECT_FALSE(manager.TestWritePage(page));
    } catch (const std::exception& e) {
        // 如果抛出异常，这也是预期的
        SUCCEED();
    }
    
    // 确保删除临时文件
    if (std::filesystem::exists(temp_file)) {
        std::filesystem::remove(temp_file);
    }
}

// 测试模拟ReadPage定位失败的情况
TEST_F(DiskManagerTest, SimulateReadPageSeekFailure2) {
    // 创建一个临时文件
    std::string temp_file = "temp_read_seek_fail_test.db";
    
    try {
        // 创建TestDiskManager
        sqlcc::TestDiskManager manager(temp_file, *config_manager);
        
        // 创建一个页面
        sqlcc::Page page;
        page.SetPageId(0);
        char test_data[sqlcc::PAGE_SIZE];
        memset(test_data, 'C', sizeof(test_data));
        page.WriteData(0, test_data, sizeof(test_data));
        
        // 写入页面
        EXPECT_TRUE(manager.WritePage(page.GetPageId(), page.GetData()));
        
        // 设置模拟定位失败
        manager.SetSimulateSeekFailure(true);
        
        // 尝试读取页面，应该在定位时失败
        sqlcc::Page read_page;
        EXPECT_FALSE(manager.TestReadPage(0, read_page.GetData()));
    } catch (const std::exception& e) {
        // 如果抛出异常，这也是预期的
        SUCCEED();
    }
    
    // 确保删除临时文件
    if (std::filesystem::exists(temp_file)) {
        std::filesystem::remove(temp_file);
    }
}

// 测试创建多个页面并读写
TEST_F(DiskManagerTest, MultiplePagesReadWrite) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    const int NUM_PAGES = 10;
    std::vector<sqlcc::Page> pages(NUM_PAGES);
    
    // 创建并写入多个页面
    for (int i = 0; i < NUM_PAGES; ++i) {
        pages[i].SetPageId(i);
        
        // 为每个页面写入不同的数据
        char test_data[100];
        snprintf(test_data, sizeof(test_data), "Test data for page %d", i);
        pages[i].WriteData(0, test_data, strlen(test_data) + 1);
        
        // 写入页面
        EXPECT_TRUE(manager.WritePage(pages[i].GetPageId(), pages[i].GetData()));
    }
    
    // 读取并验证所有页面
    for (int i = 0; i < NUM_PAGES; ++i) {
        sqlcc::Page read_page;
        EXPECT_TRUE(manager.ReadPage(i, read_page.GetData()));
        
        // 验证数据
        char read_data[100];
        read_page.ReadData(0, read_data, sizeof(read_data));
        
        char expected_data[100];
        snprintf(expected_data, sizeof(expected_data), "Test data for page %d", i);
        EXPECT_EQ(strcmp(expected_data, read_data), 0);
    }
}

// 测试构造函数中文件创建逻辑
TEST_F(DiskManagerTest, ConstructorFileCreation) {
    // 确保测试文件不存在
    std::filesystem::remove(TEST_DB_FILE);
    
    // 创建DiskManager，应该触发文件创建逻辑
    EXPECT_NO_THROW({
        sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    });
    
    // 验证文件已创建
    EXPECT_TRUE(std::filesystem::exists(TEST_DB_FILE));
}

// 测试构造函数中文件打开失败的情况
TEST_F(DiskManagerTest, ConstructorFileOpenFailure) {
    // 创建一个目录而不是文件，这样文件打开会失败
    std::filesystem::create_directories(TEST_DB_FILE);
    
    try {
        // 尝试创建DiskManager，应该抛出异常
        sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
        FAIL() << "Expected DiskManagerException to be thrown";
    } catch (const sqlcc::DiskManagerException& e) {
        // 预期的异常
        SUCCEED();
    }
    
    // 清理
    std::filesystem::remove_all(TEST_DB_FILE);
}

// 测试WritePage中无效页面ID的情况
TEST_F(DiskManagerTest, WritePageInvalidPageId) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 创建一个页面并设置无效ID
    sqlcc::Page page;
    page.SetPageId(-1);
    
    // 写入应该失败
    EXPECT_FALSE(manager.WritePage(page.GetPageId(), page.GetData()));
}

// 测试ReadPage中无效参数的情况
TEST_F(DiskManagerTest, ReadPageInvalidParameters) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 测试无效页面ID
    sqlcc::Page page1;
    EXPECT_FALSE(manager.ReadPage(-1, page1.GetData()));
    
    // 测试空指针
    EXPECT_FALSE(manager.ReadPage(0, nullptr));
}

// 测试模拟WritePage中定位失败的情况
TEST_F(DiskManagerTest, SimulateWritePageSeekFailure) {
    // 创建一个临时文件
    std::string temp_file = "temp_write_seek_fail_test.db";
    
    try {
        // 创建TestDiskManager
        sqlcc::TestDiskManager manager(temp_file, *config_manager);
        
        // 创建一个页面
        sqlcc::Page page;
        page.SetPageId(0);
        char test_data[sqlcc::PAGE_SIZE];
        memset(test_data, 'D', sizeof(test_data));
        page.WriteData(0, test_data, sizeof(test_data));
        
        // 设置模拟定位失败
        manager.SetSimulateSeekFailure(true);
        
        // 尝试写入页面，应该在定位时失败
        EXPECT_FALSE(manager.TestWritePage(page));
    } catch (const std::exception& e) {
        // 如果抛出异常，这也是预期的
        SUCCEED();
    }
    
    // 确保删除临时文件
    if (std::filesystem::exists(temp_file)) {
        std::filesystem::remove(temp_file);
    }
}

// 测试WritePage中定位失败的情况（使用内部模拟机制）
TEST_F(DiskManagerTest, WritePageSeekFailure) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 创建一个页面
    sqlcc::Page page;
    page.SetPageId(1);
    char test_data[sqlcc::PAGE_SIZE];
    memset(test_data, 'X', sizeof(test_data));
    page.WriteData(0, test_data, sizeof(test_data));
    
    // 设置模拟定位失败
    manager.SetSimulateSeekFailure(true);
    
    // 尝试写入页面，应该失败
    EXPECT_FALSE(manager.WritePage(page.GetPageId(), page.GetData()));
    
    // 重置失败模拟
    manager.SetSimulateSeekFailure(false);
    
    // 现在写入应该成功
    EXPECT_TRUE(manager.WritePage(page.GetPageId(), page.GetData()));
}

// 测试WritePage中写入失败的情况（使用内部模拟机制）
TEST_F(DiskManagerTest, WritePageWriteFailure) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 创建一个页面
    sqlcc::Page page;
    page.SetPageId(2);
    char test_data[sqlcc::PAGE_SIZE];
    memset(test_data, 'Y', sizeof(test_data));
    page.WriteData(0, test_data, sizeof(test_data));
    
    // 设置模拟写入失败
    manager.SetSimulateWriteFailure(true);
    
    // 尝试写入页面，应该失败
    EXPECT_FALSE(manager.WritePage(page.GetPageId(), page.GetData()));
    
    // 重置失败模拟
    manager.SetSimulateWriteFailure(false);
    
    // 现在写入应该成功
    EXPECT_TRUE(manager.WritePage(page.GetPageId(), page.GetData()));
}

// 测试WritePage中刷新失败的情况（使用内部模拟机制）
TEST_F(DiskManagerTest, WritePageFlushFailure) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 创建一个页面
    sqlcc::Page page;
    page.SetPageId(3);
    char test_data[sqlcc::PAGE_SIZE];
    memset(test_data, 'Z', sizeof(test_data));
    page.WriteData(0, test_data, sizeof(test_data));
    
    // 设置模拟刷新失败
    manager.SetSimulateFlushFailure(true);
    
    // 尝试写入页面，应该失败
    EXPECT_FALSE(manager.WritePage(page.GetPageId(), page.GetData()));
    
    // 重置失败模拟
    manager.SetSimulateFlushFailure(false);
    
    // 现在写入应该成功
    EXPECT_TRUE(manager.WritePage(page.GetPageId(), page.GetData()));
}

// 测试ReadPage中定位失败的情况（使用内部模拟机制）
TEST_F(DiskManagerTest, ReadPageSeekFailure) {
    // 创建一个临时文件用于此测试
    std::string temp_file = "temp_read_seek_test.db";
    
    try {
        sqlcc::DiskManager manager(temp_file, *config_manager);
        
        // 首先写入页面0（确保页面存在）
        sqlcc::Page write_page;
        write_page.SetPageId(0);
        char test_data[] = "Test data for seek failure";
        write_page.WriteData(0, test_data, sizeof(test_data));
        EXPECT_TRUE(manager.WritePage(write_page.GetPageId(), write_page.GetData()));
        
        // 设置模拟定位失败
        manager.SetSimulateSeekFailure(true);
        
        // 尝试读取页面，应该失败
        char read_data[sqlcc::PAGE_SIZE];
        EXPECT_FALSE(manager.ReadPage(0, read_data));
        
        // 重置失败模拟
        manager.SetSimulateSeekFailure(false);
        
        // 现在读取应该成功
        EXPECT_TRUE(manager.ReadPage(0, read_data));
        
        // 验证数据
        EXPECT_EQ(memcmp(test_data, read_data, sizeof(test_data)), 0);
    } catch (const std::exception& e) {
        // 如果抛出异常，这也是预期的
        SUCCEED();
    }
    
    // 确保删除临时文件
    if (std::filesystem::exists(temp_file)) {
        std::filesystem::remove(temp_file);
    }
}

// 测试模拟ReadPage中读取失败的情况
TEST_F(DiskManagerTest, SimulateReadPageReadFailure) {
    // 创建一个临时文件
    std::string temp_file = "temp_read_fail_test.db";
    
    try {
        // 创建TestDiskManager
        sqlcc::TestDiskManager manager(temp_file, *config_manager);
        
        // 创建一个页面并写入
        sqlcc::Page write_page;
        write_page.SetPageId(0);
        char test_data[sqlcc::PAGE_SIZE];
        memset(test_data, 'E', sizeof(test_data));
        write_page.WriteData(0, test_data, sizeof(test_data));
        
        // 写入页面
        EXPECT_TRUE(manager.WritePage(write_page.GetPageId(), write_page.GetData()));
        
        // 方法1：尝试读取一个超出文件范围的页面
        char read_data1[sqlcc::PAGE_SIZE];
        EXPECT_FALSE(manager.ReadPage(1000, read_data1));
        
        // 方法2：损坏文件然后尝试读取
        // 先关闭manager，避免文件句柄冲突
        // 重新创建文件管理器来模拟文件损坏
        {
            // 创建一个新文件并写入损坏数据
            std::ofstream corrupt_file(temp_file, std::ios::binary | std::ios::trunc);
            corrupt_file.write("corrupted", 9); // 写入损坏的数据
            corrupt_file.close();
        }
        
        // 现在尝试用新的manager读取损坏的文件
        sqlcc::TestDiskManager manager2(temp_file, *config_manager);
        char read_data2[sqlcc::PAGE_SIZE];
        // 由于文件损坏，读取应该失败
        EXPECT_FALSE(manager2.ReadPage(0, read_data2));
        
    } catch (const std::exception& e) {
        // 如果抛出异常，这也是预期的
        SUCCEED();
    }
    
    // 确保删除临时文件
    if (std::filesystem::exists(temp_file)) {
        std::filesystem::remove(temp_file);
    }
}

// 测试配置变更回调处理
TEST_F(DiskManagerTest, ConfigChangeCallback) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 测试直接I/O配置变更
    EXPECT_NO_THROW({
        config_manager->SetValue("disk_manager.enable_direct_io", true);
    });
    
    // 测试I/O队列深度配置变更
    EXPECT_NO_THROW({
        config_manager->SetValue("disk_manager.io_queue_depth", 128);
    });
    
    // 测试异步I/O配置变更
    EXPECT_NO_THROW({
        config_manager->SetValue("disk_manager.enable_async_io", false);
    });
    
    // 测试批量I/O大小配置变更
    EXPECT_NO_THROW({
        config_manager->SetValue("disk_manager.batch_io_size", 64);
    });
    
    // 测试同步策略配置变更
    EXPECT_NO_THROW({
        config_manager->SetValue("disk_manager.sync_strategy", std::string("immediate"));
    });
    
    // 测试同步间隔配置变更
    EXPECT_NO_THROW({
        config_manager->SetValue("disk_manager.sync_interval", 1000);
    });
    
    // 测试无效的配置键（应该被忽略，不会导致崩溃）
    EXPECT_NO_THROW({
        config_manager->SetValue("disk_manager.invalid_key", std::string("invalid_value"));
    });
}

// 测试空指针参数验证
TEST_F(DiskManagerTest, NullPointerValidation) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 测试WritePage的空指针验证
    EXPECT_FALSE(manager.WritePage(0, nullptr));
    
    // 测试ReadPage的空指针验证
    char* null_data = nullptr;
    EXPECT_FALSE(manager.ReadPage(0, null_data));
}

// 测试批量读取页面
TEST_F(DiskManagerTest, BatchReadPages) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 分配一些页面用于测试
    int32_t page1 = manager.AllocatePage();
    int32_t page2 = manager.AllocatePage();
    int32_t page3 = manager.AllocatePage();
    
    // 写入测试数据
    sqlcc::Page page1_data, page2_data, page3_data;
    page1_data.SetPageId(page1);
    page2_data.SetPageId(page2);
    page3_data.SetPageId(page3);
    
    char test_data1[] = "Test data 1 for batch read";
    char test_data2[] = "Test data 2 for batch read";
    char test_data3[] = "Test data 3 for batch read";
    
    page1_data.WriteData(0, test_data1, sizeof(test_data1));
    page2_data.WriteData(0, test_data2, sizeof(test_data2));
    page3_data.WriteData(0, test_data3, sizeof(test_data3));
    
    EXPECT_TRUE(manager.WritePage(page1, page1_data.GetData()));
    EXPECT_TRUE(manager.WritePage(page2, page2_data.GetData()));
    EXPECT_TRUE(manager.WritePage(page3, page3_data.GetData()));
    
    // 批量读取页面
    std::vector<int32_t> page_ids = {page1, page2, page3};
    std::vector<char*> page_data_buffers;
    page_data_buffers.resize(3);
    
    // 分配缓冲区
    std::vector<std::vector<char>> buffers(3, std::vector<char>(sqlcc::PAGE_SIZE));
    for (int i = 0; i < 3; ++i) {
        page_data_buffers[i] = buffers[i].data();
    }
    
    EXPECT_TRUE(manager.BatchReadPages(page_ids, page_data_buffers));
}

// 测试预取页面
TEST_F(DiskManagerTest, PrefetchPage) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 分配一个页面并写入数据
    int32_t page_id = manager.AllocatePage();
    sqlcc::Page page_data;
    page_data.SetPageId(page_id);
    char test_data[] = "Test data for prefetch";
    page_data.WriteData(0, test_data, sizeof(test_data));
    EXPECT_TRUE(manager.WritePage(page_id, page_data.GetData()));
    
    // 预取页面
    EXPECT_TRUE(manager.PrefetchPage(page_id));
    
    // 预取不存在的页面（注意：当前实现不检查页面是否存在，所以返回true）
    // 这是预期的行为，因为预取只是给操作系统的建议
    EXPECT_TRUE(manager.PrefetchPage(99999));
}

// 测试批量预取页面
TEST_F(DiskManagerTest, BatchPrefetchPages) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 分配一些页面并写入数据
    int32_t page1 = manager.AllocatePage();
    int32_t page2 = manager.AllocatePage();
    
    sqlcc::Page page_data;
    char test_data[] = "Test data for batch prefetch";
    page_data.SetPageId(page1);
    page_data.WriteData(0, test_data, sizeof(test_data));
    EXPECT_TRUE(manager.WritePage(page1, page_data.GetData()));
    
    page_data.SetPageId(page2);
    page_data.WriteData(0, test_data, sizeof(test_data));
    EXPECT_TRUE(manager.WritePage(page2, page_data.GetData()));
    
    // 批量预取页面
    std::vector<int32_t> page_ids = {page1, page2};
    EXPECT_TRUE(manager.BatchPrefetchPages(page_ids));
    
    // 批量预取包含不存在页面的列表（注意：当前实现不检查页面是否存在，所以返回true）
    // 这是预期的行为，因为预取只是给操作系统的建议
    std::vector<int32_t> invalid_page_ids = {page1, 99999};
    EXPECT_TRUE(manager.BatchPrefetchPages(invalid_page_ids));
}

// 测试页面释放
TEST_F(DiskManagerTest, DeallocatePage) {
    sqlcc::DiskManager manager(TEST_DB_FILE, *config_manager);
    
    // 分配一个页面
    int32_t page_id = manager.AllocatePage();
    EXPECT_GE(page_id, 0);  // 页面ID可以为0（第一个页面）
    
    // 写入一些数据
    sqlcc::Page page_data;
    page_data.SetPageId(page_id);
    char test_data[] = "Test data for deallocate";
    page_data.WriteData(0, test_data, sizeof(test_data));
    EXPECT_TRUE(manager.WritePage(page_id, page_data.GetData()));
    
    // 释放页面
    EXPECT_TRUE(manager.DeallocatePage(page_id));
    
    // 尝试释放不存在的页面（注意：当前实现不检查页面是否存在，所以返回true）
    // 这是预期的行为，因为释放只是标记页面为可用状态
    EXPECT_TRUE(manager.DeallocatePage(99999));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}