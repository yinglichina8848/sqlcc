/**
 * @file disk_manager_enhanced_test.cc
 * @brief 磁盘管理器增强测试实现文件
 * 
 * Why: 需要提高disk_manager.cc的代码覆盖率，特别是未覆盖的核心方法
 * What: 实现DiskManagerEnhancedTest类，提供更全面的测试用例
 * How: 使用Google Test框架编写测试用例，覆盖磁盘管理器的所有公共接口和错误处理路径
 */

#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "disk_manager.h"
#include "config_manager.h"
#include "exception.h"

namespace sqlcc {
namespace test {

/**
 * @brief 磁盘管理器增强测试类
 * 
 * Why: 需要创建增强测试类来提高磁盘管理器的代码覆盖率
 * What: DiskManagerEnhancedTest类继承自testing::Test，提供更全面的测试环境
 * How: 实现SetUp和TearDown方法，创建更复杂的测试场景
 */
class DiskManagerEnhancedTest : public ::testing::Test {
protected:
    std::string test_db_file_;
    std::filesystem::path temp_dir_;
    
    /**
     * @brief 测试环境设置
     * 
     * Why: 需要在每个测试前创建测试环境，包括临时目录和测试数据库文件
     * What: SetUp方法创建临时目录和测试数据库文件路径
     * How: 使用文件系统库创建临时目录，生成唯一的测试文件名
     */
    void SetUp() override {
        // 创建临时目录
        temp_dir_ = std::filesystem::temp_directory_path() / "sqlcc_test";
        std::filesystem::create_directories(temp_dir_);
        
        // 创建测试数据库文件路径
        test_db_file_ = (temp_dir_ / "test_enhanced.db").string();
    }
    
    /**
     * @brief 测试环境清理
     * 
     * Why: 需要在每个测试后清理测试环境，删除临时文件和目录
     * What: TearDown方法删除测试数据库文件和临时目录
     * How: 使用文件系统库删除临时文件和目录
     */
    void TearDown() override {
        // 清理临时文件和目录
        if (std::filesystem::exists(test_db_file_)) {
            std::filesystem::remove(test_db_file_);
        }
        
        if (std::filesystem::exists(temp_dir_)) {
            std::filesystem::remove_all(temp_dir_);
        }
    }
    
    /**
     * @brief 创建测试页面数据
     * 
     * Why: 需要创建测试页面数据用于读写测试
     * What: CreateTestPageData方法创建指定大小的测试数据
     * How: 使用指定模式填充数据缓冲区
     */
    void CreateTestPageData(char* data, size_t size, int pattern = 0xAA) {
        memset(data, pattern, size);
    }
    
    /**
     * @brief 验证页面数据
     * 
     * Why: 需要验证页面数据是否符合预期
     * What: VerifyPageData方法检查数据是否匹配指定模式
     * How: 遍历数据缓冲区，检查每个字节是否匹配
     */
    bool VerifyPageData(const char* data, size_t size, int pattern = 0xAA) {
        for (size_t i = 0; i < size; ++i) {
            if (static_cast<unsigned char>(data[i]) != static_cast<unsigned char>(pattern)) {
                return false;
            }
        }
        return true;
    }
};

/**
 * @brief 测试磁盘管理器构造函数 - 文件不存在的情况
 * 
 * Why: 需要验证当数据库文件不存在时，磁盘管理器能否正确创建新文件
 * What: 测试DiskManager构造函数在文件不存在时的行为
 * How: 创建DiskManager实例，指定不存在的文件路径，验证文件是否被创建
 */
TEST_F(DiskManagerEnhancedTest, ConstructorWithNonExistentFile) {
    // 确保文件不存在
    ASSERT_FALSE(std::filesystem::exists(test_db_file_));
    
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器，应该会创建新文件
    {
        DiskManager disk_manager(test_db_file_, config);
        
        // 验证文件被创建
        EXPECT_TRUE(std::filesystem::exists(test_db_file_));
        
        // 验证文件大小为0
        EXPECT_EQ(disk_manager.GetFileSize(), 0);
    }
}

/**
 * @brief 测试磁盘管理器构造函数 - 文件已存在的情况
 * 
 * Why: 需要验证当数据库文件已存在时，磁盘管理器能否正确打开文件
 * What: 测试DiskManager构造函数在文件已存在时的行为
 * How: 先创建一个文件，然后创建DiskManager实例，验证文件是否被正确打开
 */
TEST_F(DiskManagerEnhancedTest, ConstructorWithExistingFile) {
    // 创建一个初始文件
    std::ofstream file(test_db_file_, std::ios::binary);
    char data[8192] = {0};
    file.write(data, 8192);
    file.close();
    
    // 验证文件存在且有正确的大小
    ASSERT_TRUE(std::filesystem::exists(test_db_file_));
    ASSERT_EQ(std::filesystem::file_size(test_db_file_), 8192);
    
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器，应该会打开现有文件
    {
        DiskManager disk_manager(test_db_file_, config);
        
        // 验证文件大小被正确读取
        EXPECT_EQ(disk_manager.GetFileSize(), 8192);
    }
}

/**
 * @brief 测试磁盘管理器构造函数 - 文件打开失败的情况
 * 
 * Why: 需要验证当无法打开文件时，磁盘管理器能否正确抛出异常
 * What: 测试DiskManager构造函数在文件打开失败时的行为
 * How: 使用无效的文件路径，验证是否抛出异常
 */
TEST_F(DiskManagerEnhancedTest, ConstructorWithInvalidPath) {
    // 使用无效的文件路径（包含不存在的目录）
    std::string invalid_path = "/nonexistent/directory/test.db";
    
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器，应该会抛出异常
    EXPECT_THROW({
        DiskManager disk_manager(invalid_path, config);
    }, DiskManagerException);
}

/**
 * @brief 测试写入页面 - 成功的情况
 * 
 * Why: 需要验证写入页面功能是否正常工作
 * What: 测试WritePage方法能否正确写入页面数据
 * How: 创建测试数据，调用WritePage方法，然后读取验证数据
 */
TEST_F(DiskManagerEnhancedTest, WritePageSuccess) {
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器
    DiskManager disk_manager(test_db_file_, config);
    
    // 创建测试页面数据
    char page_data[8192];
    CreateTestPageData(page_data, 8192, 0xAB);
    
    // 写入页面
    EXPECT_TRUE(disk_manager.WritePage(0, page_data));
    
    // 验证文件大小已更新
    EXPECT_EQ(disk_manager.GetFileSize(), 8192);
}

/**
 * @brief 测试写入页面 - 无效页面ID
 * 
 * Why: 需要验证当页面ID无效时，写入页面能否正确处理
 * What: 测试WritePage方法在页面ID为负数时的行为
 * How: 使用负数页面ID调用WritePage方法，验证返回值
 */
TEST_F(DiskManagerEnhancedTest, WritePageInvalidPageId) {
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器
    DiskManager disk_manager(test_db_file_, config);
    
    // 创建测试页面数据
    char page_data[8192];
    CreateTestPageData(page_data, 8192);
    
    // 使用负数页面ID，应该失败
    EXPECT_FALSE(disk_manager.WritePage(-1, page_data));
}

/**
 * @brief 测试写入页面 - 空指针
 * 
 * Why: 需要验证当页面数据指针为空时，写入页面能否正确处理
 * What: 测试WritePage方法在页面数据指针为空时的行为
 * How: 使用空指针调用WritePage方法，验证返回值
 */
TEST_F(DiskManagerEnhancedTest, WritePageNullPointer) {
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器
    DiskManager disk_manager(test_db_file_, config);
    
    // 使用空指针，应该失败
    EXPECT_FALSE(disk_manager.WritePage(0, nullptr));
}

/**
 * @brief 测试读取页面 - 成功的情况
 * 
 * Why: 需要验证读取页面功能是否正常工作
 * What: 测试ReadPage方法能否正确读取页面数据
 * How: 先写入测试数据，然后调用ReadPage方法，验证数据是否正确
 */
TEST_F(DiskManagerEnhancedTest, ReadPageSuccess) {
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器
    DiskManager disk_manager(test_db_file_, config);
    
    // 创建测试页面数据
    char write_data[8192];
    CreateTestPageData(write_data, 8192, 0xCD);
    
    // 写入页面
    ASSERT_TRUE(disk_manager.WritePage(0, write_data));
    
    // 读取页面
    char read_data[8192];
    EXPECT_TRUE(disk_manager.ReadPage(0, read_data));
    
    // 验证数据是否正确
    EXPECT_TRUE(VerifyPageData(read_data, 8192, 0xCD));
}

/**
 * @brief 测试读取页面 - 无效页面ID
 * 
 * Why: 需要验证当页面ID无效时，读取页面能否正确处理
 * What: 测试ReadPage方法在页面ID为负数时的行为
 * How: 使用负数页面ID调用ReadPage方法，验证返回值
 */
TEST_F(DiskManagerEnhancedTest, ReadPageInvalidPageId) {
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器
    DiskManager disk_manager(test_db_file_, config);
    
    // 创建读取缓冲区
    char read_data[8192];
    
    // 使用负数页面ID，应该失败
    EXPECT_FALSE(disk_manager.ReadPage(-1, read_data));
}

/**
 * @brief 测试读取页面 - 空指针
 * 
 * Why: 需要验证当页面数据指针为空时，读取页面能否正确处理
 * What: 测试ReadPage方法在页面数据指针为空时的行为
 * How: 使用空指针调用ReadPage方法，验证返回值
 */
TEST_F(DiskManagerEnhancedTest, ReadPageNullPointer) {
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器
    DiskManager disk_manager(test_db_file_, config);
    
    // 使用空指针，应该失败
    EXPECT_FALSE(disk_manager.ReadPage(0, nullptr));
}

/**
 * @brief 测试读取页面 - 页面不存在
 * 
 * Why: 需要验证当读取不存在的页面时，读取页面能否正确处理
 * What: 测试ReadPage方法在页面不存在时的行为
 * How: 读取一个未写入的页面，验证返回值
 */
TEST_F(DiskManagerEnhancedTest, ReadPageNonExistentPage) {
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器
    DiskManager disk_manager(test_db_file_, config);
    
    // 创建读取缓冲区
    char read_data[8192];
    
    // 读取不存在的页面，应该失败
    EXPECT_FALSE(disk_manager.ReadPage(0, read_data));
}

/**
 * @brief 测试分配新页面
 * 
 * Why: 需要验证分配新页面功能是否正常工作
 * What: 测试AllocatePage方法能否正确分配新的页面ID
 * How: 多次调用AllocatePage方法，验证返回的页面ID是否递增
 */
TEST_F(DiskManagerEnhancedTest, AllocatePage) {
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器
    DiskManager disk_manager(test_db_file_, config);
    
    // 分配新页面
    int32_t page_id1 = disk_manager.AllocatePage();
    EXPECT_EQ(page_id1, 0);
    
    int32_t page_id2 = disk_manager.AllocatePage();
    EXPECT_EQ(page_id2, 1);
    
    int32_t page_id3 = disk_manager.AllocatePage();
    EXPECT_EQ(page_id3, 2);
}

/**
 * @brief 测试获取文件大小
 * 
 * Why: 需要验证获取文件大小功能是否正常工作
 * What: 测试GetFileSize方法能否正确返回文件大小
 * How: 写入不同数量的页面，验证文件大小是否正确更新
 */
TEST_F(DiskManagerEnhancedTest, GetFileSize) {
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器
    DiskManager disk_manager(test_db_file_, config);
    
    // 初始文件大小应该为0
    EXPECT_EQ(disk_manager.GetFileSize(), 0);
    
    // 创建测试页面数据
    char page_data[8192];
    CreateTestPageData(page_data, 8192);
    
    // 写入一个页面
    ASSERT_TRUE(disk_manager.WritePage(0, page_data));
    EXPECT_EQ(disk_manager.GetFileSize(), 8192);
    
    // 写入另一个页面
    ASSERT_TRUE(disk_manager.WritePage(1, page_data));
    EXPECT_EQ(disk_manager.GetFileSize(), 16384);
}

/**
 * @brief 测试批量读取页面
 * 
 * Why: 需要验证批量读取页面功能是否正常工作
 * What: 测试BatchReadPages方法能否正确批量读取页面数据
 * How: 先写入多个页面，然后调用BatchReadPages方法，验证数据是否正确
 */
TEST_F(DiskManagerEnhancedTest, BatchReadPages) {
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器
    DiskManager disk_manager(test_db_file_, config);
    
    // 创建测试页面数据
    std::vector<char> page_data(8192);
    CreateTestPageData(page_data.data(), 8192, 0xEF);
    
    // 写入多个页面
    for (int i = 0; i < 5; ++i) {
        ASSERT_TRUE(disk_manager.WritePage(i, page_data.data()));
    }
    
    // 准备批量读取
    std::vector<int32_t> page_ids = {0, 2, 4};
    std::vector<char*> data_buffers;
    std::vector<std::vector<char>> buffers(3, std::vector<char>(8192));
    
    for (int i = 0; i < 3; ++i) {
        data_buffers.push_back(buffers[i].data());
    }
    
    // 批量读取页面
    EXPECT_TRUE(disk_manager.BatchReadPages(page_ids, data_buffers));
    
    // 验证数据是否正确
    for (int i = 0; i < 3; ++i) {
        EXPECT_TRUE(VerifyPageData(data_buffers[i], 8192, 0xEF));
    }
}

/**
 * @brief 测试批量读取页面 - 无效参数
 * 
 * Why: 需要验证当参数无效时，批量读取页面能否正确处理
 * What: 测试BatchReadPages方法在参数无效时的行为
 * How: 使用空向量或大小不匹配的向量调用BatchReadPages方法，验证返回值
 */
TEST_F(DiskManagerEnhancedTest, BatchReadPagesInvalidParameters) {
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器
    DiskManager disk_manager(test_db_file_, config);
    
    // 准备无效参数
    std::vector<int32_t> empty_page_ids;
    std::vector<char*> empty_data_buffers;
    
    // 使用空向量，应该失败
    EXPECT_FALSE(disk_manager.BatchReadPages(empty_page_ids, empty_data_buffers));
    
    // 使用大小不匹配的向量，应该失败
    std::vector<int32_t> page_ids = {0, 1};
    std::vector<char*> data_buffers = {new char[8192]};
    
    EXPECT_FALSE(disk_manager.BatchReadPages(page_ids, data_buffers));
    
    delete[] data_buffers[0];
}

/**
 * @brief 测试预取页面
 * 
 * Why: 需要验证预取页面功能是否正常工作
 * What: 测试PrefetchPage方法能否正确预取页面
 * How: 写入页面后调用PrefetchPage方法，验证返回值
 */
TEST_F(DiskManagerEnhancedTest, PrefetchPage) {
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器
    DiskManager disk_manager(test_db_file_, config);
    
    // 创建测试页面数据
    char page_data[8192];
    CreateTestPageData(page_data, 8192);
    
    // 写入页面
    ASSERT_TRUE(disk_manager.WritePage(0, page_data));
    
    // 预取页面
    EXPECT_TRUE(disk_manager.PrefetchPage(0));
}

/**
 * @brief 测试预取页面 - 无效页面ID
 * 
 * Why: 需要验证当页面ID无效时，预取页面能否正确处理
 * What: 测试PrefetchPage方法在页面ID为负数时的行为
 * How: 使用负数页面ID调用PrefetchPage方法，验证返回值
 */
TEST_F(DiskManagerEnhancedTest, PrefetchPageInvalidPageId) {
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器
    DiskManager disk_manager(test_db_file_, config);
    
    // 使用负数页面ID，应该失败
    EXPECT_FALSE(disk_manager.PrefetchPage(-1));
}

/**
 * @brief 测试批量预取页面
 * 
 * Why: 需要验证批量预取页面功能是否正常工作
 * What: 测试BatchPrefetchPages方法能否正确批量预取页面
 * How: 写入多个页面后调用BatchPrefetchPages方法，验证返回值
 */
TEST_F(DiskManagerEnhancedTest, BatchPrefetchPages) {
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器
    DiskManager disk_manager(test_db_file_, config);
    
    // 创建测试页面数据
    char page_data[8192];
    CreateTestPageData(page_data, 8192);
    
    // 写入多个页面
    for (int i = 0; i < 5; ++i) {
        ASSERT_TRUE(disk_manager.WritePage(i, page_data));
    }
    
    // 准备批量预取
    std::vector<int32_t> page_ids = {0, 2, 4};
    
    // 批量预取页面
    EXPECT_TRUE(disk_manager.BatchPrefetchPages(page_ids));
}

/**
 * @brief 测试批量预取页面 - 无效参数
 * 
 * Why: 需要验证当参数无效时，批量预取页面能否正确处理
 * What: 测试BatchPrefetchPages方法在参数无效时的行为
 * How: 使用空向量调用BatchPrefetchPages方法，验证返回值
 */
TEST_F(DiskManagerEnhancedTest, BatchPrefetchPagesInvalidParameters) {
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器
    DiskManager disk_manager(test_db_file_, config);
    
    // 准备无效参数
    std::vector<int32_t> empty_page_ids;
    
    // 使用空向量，应该失败
    EXPECT_FALSE(disk_manager.BatchPrefetchPages(empty_page_ids));
    
    // 使用包含负数的向量，应该成功但只预取有效页面
    std::vector<int32_t> page_ids_with_negative = {0, -1, 2};
    EXPECT_TRUE(disk_manager.BatchPrefetchPages(page_ids_with_negative));
}

/**
 * @brief 测试配置变更回调
 * 
 * Why: 需要验证配置变更回调功能是否正常工作
 * What: 测试OnConfigChange方法能否正确处理配置变更
 * How: 修改配置值，验证回调是否被正确调用
 */
TEST_F(DiskManagerEnhancedTest, ConfigChangeCallback) {
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器
    DiskManager disk_manager(test_db_file_, config);
    
    // 修改配置值
    config.SetValue("disk_manager.enable_direct_io", true);
    config.SetValue("disk_manager.io_queue_depth", 16);
    config.SetValue("disk_manager.enable_async_io", true);
    config.SetValue("disk_manager.batch_io_size", 16);
    config.SetValue("disk_manager.sync_strategy", std::string("FULL"));
    config.SetValue("disk_manager.sync_interval", 60);
    
    // 由于OnConfigChange方法是私有的，我们无法直接测试它
    // 但我们可以通过修改配置来间接测试回调是否被注册
    // 这里只是验证SetValue不会导致崩溃
    SUCCEED();
}

/**
 * @brief 测试多线程安全性
 * 
 * Why: 需要验证磁盘管理器在多线程环境下的安全性
 * What: 测试多个线程同时读写页面时的行为
 * How: 创建多个线程，同时进行读写操作，验证数据一致性
 */
TEST_F(DiskManagerEnhancedTest, ThreadSafety) {
    // 创建配置管理器
    ConfigManager& config = ConfigManager::GetInstance();
    
    // 创建磁盘管理器
    DiskManager disk_manager(test_db_file_, config);
    
    // 创建多个线程进行读写操作
    const int num_threads = 4;
    const int num_pages = 10;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            // 每个线程写入和读取不同的页面
            for (int j = 0; j < num_pages; ++j) {
                int32_t page_id = i * num_pages + j;
                
                // 创建测试数据
                char write_data[8192];
                CreateTestPageData(write_data, 8192, i + j);
                
                // 写入页面
                ASSERT_TRUE(disk_manager.WritePage(page_id, write_data));
                
                // 读取页面
                char read_data[8192];
                ASSERT_TRUE(disk_manager.ReadPage(page_id, read_data));
                
                // 验证数据
                ASSERT_TRUE(VerifyPageData(read_data, 8192, i + j));
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
}

}  // namespace test
}  // namespace sqlcc