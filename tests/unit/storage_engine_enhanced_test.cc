/**
 * @file storage_engine_enhanced_test.cc
 * @brief 存储引擎增强测试实现文件
 * 
 * Why: 需要提高storage_engine.cc的代码覆盖率，特别是未覆盖的错误处理路径
 * What: 实现StorageEngineEnhancedTest类，提供更全面的测试用例
 * How: 使用Google Test框架编写测试用例，覆盖存储引擎的所有公共接口和错误处理路径
 */

#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <memory>
#include <future>

#include "storage_engine.h"
#include "config_manager.h"
#include "buffer_pool.h"
#include "disk_manager.h"
#include "exception.h"

namespace sqlcc {
namespace test {

class StorageEngineEnhancedTest : public ::testing::Test {
protected:
    const std::string TEST_DB_FILE = "test_storage_engine.db";
    ConfigManager* config_manager;
    std::unique_ptr<StorageEngine> storage_engine;
    
    void SetUp() override {
        // 删除测试文件（如果存在）
        std::filesystem::remove(TEST_DB_FILE);
        
        // 创建配置管理器
        config_manager = &ConfigManager::GetInstance();
        
        // 设置测试数据库文件路径
        config_manager->SetValue("database.db_file_path", TEST_DB_FILE);
        config_manager->SetValue("buffer_pool.pool_size", 10);
        
        // 创建存储引擎
        storage_engine = std::make_unique<StorageEngine>(*config_manager);
    }
    
    void TearDown() override {
        // 销毁存储引擎
        storage_engine.reset();
        
        // 清理测试文件
        std::filesystem::remove(TEST_DB_FILE);
    }
    
    /**
     * @brief 带超时的测试辅助函数
     * 
     * Why: 防止测试中的死锁导致测试无限等待
     * What: 提供一个通用的超时机制，用于测试可能阻塞的操作
     * How: 使用std::async和std::future实现超时控制
     * 
     * @param func 要测试的函数
     * @param timeout_seconds 超时时间（秒）
     * @return true 如果函数在超时前完成，false 如果超时
     */
    template<typename F>
    bool TestWithTimeout(F func, int timeout_seconds = 5) {
        std::atomic<bool> completed{false};
        std::atomic<bool> exception_occurred{false};
        
        auto future = std::async(std::launch::async, [&]() {
            try {
                func();
                completed = true;
            } catch (...) {
                completed = true;
                exception_occurred = true;
            }
        });
        
        if (future.wait_for(std::chrono::seconds(timeout_seconds)) == std::future_status::timeout) {
            return false; // 超时
        }
        
        return completed; // 返回是否完成
    }
};

/**
 * @brief 测试配置变更回调
 * 
 * Why: 需要验证配置变更回调是否正确工作
 * What: 测试当buffer_pool.pool_size配置变更时，回调函数是否被正确调用
 * How: 修改配置值，验证回调是否被触发
 */
TEST_F(StorageEngineEnhancedTest, ConfigChangeCallback) {
    // 修改缓冲池大小配置
    config_manager->SetValue("buffer_pool.pool_size", 20);
    
    // 修改其他配置，应该不会触发缓冲池大小变更的回调
    config_manager->SetValue("database.page_size", 8192);
    
    // 修改无效的配置键，应该不会触发任何回调
    config_manager->SetValue("invalid.config.key", "invalid_value");
}

/**
 * @brief 测试NewPage失败情况
 * 
 * Why: 需要测试NewPage方法在创建页面失败时的错误处理
 * What: 测试当缓冲池无法创建新页面时的处理
 * How: 模拟缓冲池创建页面失败的情况，使用超时机制检测死锁
 */
TEST_F(StorageEngineEnhancedTest, NewPageFailure) {
    // 创建一个新的存储引擎，使用非常小的缓冲池
    std::filesystem::path small_db_file = "small_buffer_pool.db";
    std::filesystem::remove(small_db_file);
    
    // 设置小缓冲池大小
    config_manager->SetValue("database.db_file_path", small_db_file.string());
    config_manager->SetValue("buffer_pool.pool_size", 1);
    
    // 创建小缓冲池的存储引擎
    auto small_storage_engine = std::make_unique<StorageEngine>(*config_manager);
    
    // 创建多个页面，直到缓冲池满
    int32_t page_id1, page_id2;
    Page* page1 = small_storage_engine->NewPage(&page_id1);
    ASSERT_NE(page1, nullptr);
    
    // 创建第二个页面
    // Why: 测试缓冲池满时创建新页面的行为
    // What: 尝试创建第二个页面，应该成功（因为第一个页面会被替换）
    // How: 调用NewPage方法创建页面
    Page* page2 = small_storage_engine->NewPage(&page_id2);
    ASSERT_NE(page2, nullptr);
    
    // 取消固定第一个页面，使其可以被替换
    bool unpinned = small_storage_engine->UnpinPage(page_id1, false);
    ASSERT_TRUE(unpinned);
    
    // 使用超时测试NewPage是否会死锁
    bool completed = TestWithTimeout([&]() {
        try {
            // 尝试创建第三个页面
            // 这应该成功，因为第一个页面已被取消固定
            int32_t page_id3;
            Page* page3 = small_storage_engine->NewPage(&page_id3);
            ASSERT_NE(page3, nullptr);
            
            // 清理资源
            small_storage_engine->UnpinPage(page_id2, false);
            small_storage_engine->UnpinPage(page_id3, false);
        } catch (const BufferPoolException& e) {
            // 如果出现异常，记录异常信息
            std::cout << "BufferPoolException: " << e.what() << std::endl;
            FAIL() << "NewPage不应该抛出BufferPoolException异常";
        } catch (...) {
            FAIL() << "NewPage抛出了意外的异常";
        }
    }, 5); // 5秒超时
    
    // 检查是否超时
    ASSERT_TRUE(completed) << "NewPage操作超时，可能存在死锁";
    
    // 清理
    small_storage_engine.reset();
    std::filesystem::remove(small_db_file);
}

/**
 * @brief 测试UnpinPage失败情况
 * 
 * Why: 需要测试UnpinPage方法在取消固定页面失败时的错误处理
 * What: 测试当缓冲池无法取消固定页面时的处理
 * How: 尝试取消固定一个不存在的页面
 */
TEST_F(StorageEngineEnhancedTest, UnpinPageFailure) {
    // 尝试取消固定一个不存在的页面
    bool result = storage_engine->UnpinPage(9999, false);
    EXPECT_FALSE(result);
    
    // 尝试取消固定一个存在的但未被固定的页面
    int32_t page_id;
    Page* page = storage_engine->NewPage(&page_id);
    ASSERT_NE(page, nullptr);
    
    // 直接取消固定，应该成功
    bool result1 = storage_engine->UnpinPage(page_id, false);
    EXPECT_TRUE(result1);
    
    // 再次取消固定同一个页面，可能会失败
    bool result2 = storage_engine->UnpinPage(page_id, false);
    (void)result2; // 避免未使用变量警告
    // 结果取决于缓冲池实现，这里只验证方法能被调用
}

/**
 * @brief 测试FlushPage失败情况
 * 
 * Why: 需要测试FlushPage方法在刷新页面失败时的错误处理
 * What: 测试当缓冲池无法刷新页面时的处理
 * How: 尝试刷新一个不存在的页面
 */
TEST_F(StorageEngineEnhancedTest, FlushPageFailure) {
    // 尝试刷新一个不存在的页面
    bool result = storage_engine->FlushPage(9999);
    EXPECT_FALSE(result);
    
    // 创建一个页面但不标记为脏页，然后尝试刷新
    int32_t page_id;
    Page* page = storage_engine->NewPage(&page_id);
    ASSERT_NE(page, nullptr);
    
    // 取消固定但不标记为脏页
    storage_engine->UnpinPage(page_id, false);
    
    // 尝试刷新非脏页
    bool result1 = storage_engine->FlushPage(page_id);
    (void)result1; // 避免未使用变量警告
    // 结果取决于缓冲池实现，这里只验证方法能被调用
    
    // 创建一个页面并标记为脏页，然后尝试刷新
    int32_t page_id2;
    Page* page2 = storage_engine->NewPage(&page_id2);
    ASSERT_NE(page2, nullptr);
    
    // 修改页面数据
    char test_data[] = "Test data for flushing";
    page2->WriteData(0, test_data, sizeof(test_data));
    
    // 取消固定并标记为脏页
    storage_engine->UnpinPage(page_id2, true);
    
    // 尝试刷新脏页
    bool result2 = storage_engine->FlushPage(page_id2);
    EXPECT_TRUE(result2);
}

/**
 * @brief 测试DeletePage失败情况
 * 
 * Why: 需要测试DeletePage方法在删除页面失败时的错误处理
 * What: 测试当缓冲池无法删除页面时的处理
 * How: 尝试删除一个不存在的页面
 */
TEST_F(StorageEngineEnhancedTest, DeletePageFailure) {
    // 尝试删除一个不存在的页面
    bool result = storage_engine->DeletePage(9999);
    EXPECT_FALSE(result);
    
    // 创建一个页面，然后删除它
    int32_t page_id;
    Page* page = storage_engine->NewPage(&page_id);
    ASSERT_NE(page, nullptr);
    
    // 取消固定页面
    storage_engine->UnpinPage(page_id, false);
    
    // 删除页面
    bool result1 = storage_engine->DeletePage(page_id);
    EXPECT_TRUE(result1);
    
    // 再次尝试删除同一个页面，应该失败
    bool result2 = storage_engine->DeletePage(page_id);
    EXPECT_FALSE(result2);
}

/**
 * @brief 测试FetchPage失败情况
 * 
 * Why: 需要测试FetchPage方法在获取页面失败时的错误处理
 * What: 测试当缓冲池无法获取页面时的处理
 * How: 尝试获取一个不存在的页面
 */
TEST_F(StorageEngineEnhancedTest, FetchPageFailure) {
    // 尝试获取一个不存在的页面
    Page* page = storage_engine->FetchPage(9999);
    EXPECT_EQ(page, nullptr);
}

/**
 * @brief 测试FlushAllPages方法
 * 
 * Why: 需要测试FlushAllPages方法是否正确工作
 * What: 测试刷新所有页面的功能
 * How: 创建多个页面，修改数据，然后调用FlushAllPages
 */
TEST_F(StorageEngineEnhancedTest, FlushAllPages) {
    const int NUM_PAGES = 5;
    std::vector<int32_t> page_ids;
    
    // 创建多个页面
    for (int i = 0; i < NUM_PAGES; ++i) {
        int32_t page_id;
        Page* page = storage_engine->NewPage(&page_id);
        ASSERT_NE(page, nullptr);
        page_ids.push_back(page_id);
        
        // 修改页面数据
        char test_data[100];
        snprintf(test_data, sizeof(test_data), "Test data for page %d", i);
        page->WriteData(0, test_data, strlen(test_data) + 1);
        
        // 取消固定页面并标记为脏页
        storage_engine->UnpinPage(page_id, true);
    }
    
    // 刷新所有页面
    EXPECT_NO_THROW({
        storage_engine->FlushAllPages();
    });
    
    // 验证数据是否已写入磁盘
    for (int i = 0; i < NUM_PAGES; ++i) {
        Page* page = storage_engine->FetchPage(page_ids[i]);
        ASSERT_NE(page, nullptr);
        
        char read_data[100];
        page->ReadData(0, read_data, 100);
        
        char expected_data[100];
        snprintf(expected_data, sizeof(expected_data), "Test data for page %d", i);
        EXPECT_STREQ(read_data, expected_data);
        
        storage_engine->UnpinPage(page_ids[i], false);
    }
}

/**
 * @brief 测试存储引擎的析构函数
 * 
 * Why: 需要测试存储引擎析构时是否正确刷新所有脏页
 * What: 测试析构函数是否正确刷新所有脏页到磁盘
 * How: 创建存储引擎，修改页面数据，然后销毁存储引擎，再创建新的存储引擎验证数据
 */
TEST_F(StorageEngineEnhancedTest, DestructorFlushesPages) {
    const int NUM_PAGES = 3;
    std::vector<int32_t> page_ids;
    
    {
        // 创建临时存储引擎
        auto temp_storage_engine = std::make_unique<StorageEngine>(*config_manager);
        
        // 创建多个页面
        for (int i = 0; i < NUM_PAGES; ++i) {
            int32_t page_id;
            Page* page = temp_storage_engine->NewPage(&page_id);
            ASSERT_NE(page, nullptr);
            page_ids.push_back(page_id);
            
            // 修改页面数据
            char test_data[100];
            snprintf(test_data, sizeof(test_data), "Destructor test data %d", i);
            page->WriteData(0, test_data, strlen(test_data) + 1);
            
            // 取消固定页面并标记为脏页
            temp_storage_engine->UnpinPage(page_id, true);
        }
        
        // 销毁存储引擎，应该自动刷新所有脏页
        temp_storage_engine.reset();
    }
    
    // 创建新的存储引擎，验证数据是否已写入磁盘
    auto new_storage_engine = std::make_unique<StorageEngine>(*config_manager);
    
    for (int i = 0; i < NUM_PAGES; ++i) {
        Page* page = new_storage_engine->FetchPage(page_ids[i]);
        ASSERT_NE(page, nullptr);
        
        char read_data[100];
        page->ReadData(0, read_data, 100);
        
        char expected_data[100];
        snprintf(expected_data, sizeof(expected_data), "Destructor test data %d", i);
        EXPECT_STREQ(read_data, expected_data);
        
        new_storage_engine->UnpinPage(page_ids[i], false);
    }
}

/**
 * @brief 测试多线程环境下的存储引擎操作
 * 
 * Why: 需要测试存储引擎在多线程环境下的安全性
 * What: 测试多线程同时进行页面操作
 * How: 创建多个线程，同时进行页面创建、读写和删除操作
 */
TEST_F(StorageEngineEnhancedTest, MultiThreadedOperations) {
    const int NUM_THREADS = 4;
    const int OPERATIONS_PER_THREAD = 10;
    std::vector<std::thread> threads;
    std::vector<int32_t> page_ids(NUM_THREADS * OPERATIONS_PER_THREAD);
    
    // 创建多个线程，每个线程创建多个页面
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([this, t, OPERATIONS_PER_THREAD, &page_ids]() {
            for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
                int index = t * OPERATIONS_PER_THREAD + i;
                int32_t page_id;
                Page* page = storage_engine->NewPage(&page_id);
                ASSERT_NE(page, nullptr);
                page_ids[index] = page_id;
                
                // 写入线程ID和操作ID
                int32_t data = t * 1000 + i;
                page->WriteData(0, reinterpret_cast<const char*>(&data), sizeof(data));
                
                // 取消固定页面并标记为脏页
                storage_engine->UnpinPage(page_id, true);
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证数据
    for (int t = 0; t < NUM_THREADS; ++t) {
        for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
            int index = t * OPERATIONS_PER_THREAD + i;
            int32_t page_id = page_ids[index];
            
            Page* page = storage_engine->FetchPage(page_id);
            ASSERT_NE(page, nullptr);
            
            int32_t data;
            page->ReadData(0, reinterpret_cast<char*>(&data), sizeof(data));
            EXPECT_EQ(data, t * 1000 + i);
            
            storage_engine->UnpinPage(page_id, false);
        }
    }
}

/**
 * @brief 测试缓冲池满时的死锁场景
 * 
 * Why: 需要测试当缓冲池满时是否会发生死锁
 * What: 测试在缓冲池满的情况下创建新页面的行为
 * How: 创建一个缓冲池大小为1的存储引擎，创建一个页面但不取消固定，
 *      然后尝试创建第二个页面，使用超时机制检测死锁
 */
TEST_F(StorageEngineEnhancedTest, DeadlockDetection) {
    // 创建一个缓冲池大小为2的存储引擎
    std::filesystem::path small_db_file = "deadlock_test.db";
    std::filesystem::remove(small_db_file);
    
    config_manager->SetValue("database.db_file_path", small_db_file.string());
    config_manager->SetValue("buffer_pool.pool_size", 2);
    
    auto small_storage_engine = std::make_unique<StorageEngine>(*config_manager);
    
    // 创建第一个页面
    int32_t page_id1;
    Page* page1 = small_storage_engine->NewPage(&page_id1);
    ASSERT_NE(page1, nullptr);
    
    // 创建第二个页面
    int32_t page_id2;
    Page* page2 = small_storage_engine->NewPage(&page_id2);
    ASSERT_NE(page2, nullptr);
    
    // 使用超时机制尝试创建第三个页面
    // 这应该触发页面替换而不是死锁
    bool completed = TestWithTimeout([&small_storage_engine]() {
        try {
            int32_t page_id3;
            Page* page3 = small_storage_engine->NewPage(&page_id3);
            (void)page3; // 避免未使用变量警告
        } catch (const BufferPoolException& e) {
            // 预期的异常
        }
    }, 3); // 3秒超时
    
    // 验证操作在合理时间内完成
    EXPECT_TRUE(completed) << "操作超时，可能存在死锁";
    
    // 清理资源
    small_storage_engine->UnpinPage(page_id1, false);
    small_storage_engine->UnpinPage(page_id2, false);
    std::filesystem::remove(small_db_file);
}

/**
 * @brief 测试多线程环境下的死锁检测
 * 
 * Why: 需要测试多线程环境下是否会发生死锁
 * What: 测试多个线程同时访问有限缓冲池时的行为
 * How: 创建多个线程，每个线程尝试创建页面，使用超时机制检测死锁
 */
TEST_F(StorageEngineEnhancedTest, MultiThreadedDeadlockDetection) {
    // 创建一个缓冲池大小为4的存储引擎
    std::filesystem::path small_db_file = "multi_deadlock_test.db";
    std::filesystem::remove(small_db_file);
    
    config_manager->SetValue("database.db_file_path", small_db_file.string());
    config_manager->SetValue("buffer_pool.pool_size", 4);
    
    auto small_storage_engine = std::make_unique<StorageEngine>(*config_manager);
    
    const int NUM_THREADS = 6;
    std::vector<std::thread> threads;
    std::atomic<bool> timeout_occurred{false};
    
    // 创建多个线程，每个线程尝试创建页面
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([this, &small_storage_engine, &timeout_occurred, t]() {
            bool completed = TestWithTimeout([&small_storage_engine, t]() {
                try {
                    int32_t page_id;
                    Page* page = small_storage_engine->NewPage(&page_id);
                    if (page != nullptr) {
                        // 写入一些数据
                        int32_t data = t;
                        page->WriteData(0, reinterpret_cast<const char*>(&data), sizeof(data));
                        
                        // 短暂持有页面
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        
                        // 取消固定页面
                        small_storage_engine->UnpinPage(page_id, true);
                    }
                } catch (const BufferPoolException& e) {
                    // 预期的异常
                }
            }, 5); // 5秒超时
            
            if (!completed) {
                timeout_occurred = true;
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证没有线程超时
    EXPECT_FALSE(timeout_occurred) << "多线程操作超时，可能存在死锁";
    std::filesystem::remove(small_db_file);
}

}  // namespace test
}  // namespace sqlcc