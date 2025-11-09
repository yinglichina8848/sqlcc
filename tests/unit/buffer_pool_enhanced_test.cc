#include <gtest/gtest.h>
#include <filesystem>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include <algorithm>

#include "buffer_pool.h"
#include "disk_manager.h"
#include "config_manager.h"
#include "page.h"

namespace sqlcc {
namespace test {

class BufferPoolEnhancedTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时测试文件
        test_db_file_ = std::filesystem::temp_directory_path() / "buffer_pool_test.db";
        
        // 创建磁盘管理器
        disk_manager_ = std::make_unique<DiskManager>(test_db_file_.string());
        
        // 获取配置管理器单例实例
        config_manager_ = &ConfigManager::GetInstance();
        
        // 创建缓冲池
        buffer_pool_ = std::make_unique<BufferPool>(disk_manager_.get(), 10, *config_manager_);
        
        // 创建一些测试页面
        CreateTestPages(5);
    }
    
    void TearDown() override {
        buffer_pool_.reset();
        disk_manager_.reset();
        config_manager_ = nullptr;
        
        // 清理测试文件
        if (std::filesystem::exists(test_db_file_)) {
            std::filesystem::remove(test_db_file_);
        }
    }
    
    void CreateTestPages(int count) {
        for (int i = 0; i < count; ++i) {
            Page page(i);
            // 填充一些测试数据
            char* data = page.GetData();
            for (int j = 0; j < 100; ++j) {
                data[j] = static_cast<char>(i + j);
            }
            disk_manager_->WritePage(page);
        }
    }

    std::filesystem::path test_db_file_;
    std::unique_ptr<DiskManager> disk_manager_;
    ConfigManager* config_manager_;
    std::unique_ptr<BufferPool> buffer_pool_;
};

// 测试构造函数和析构函数
TEST_F(BufferPoolEnhancedTest, ConstructorAndDestructor) {
    // 测试构造函数是否正确初始化
    EXPECT_NE(buffer_pool_, nullptr);
    
    // 测试析构函数是否会正确刷新所有脏页
    // 通过创建新的缓冲池并修改页面，然后销毁缓冲池来验证
    {
        auto temp_buffer_pool = std::make_unique<BufferPool>(disk_manager_.get(), 5, *config_manager_);
        Page* page = temp_buffer_pool->FetchPage(0);
        ASSERT_NE(page, nullptr);
        
        // 修改页面数据
        char* data = page->GetData();
        data[0] = 'X';
        
        // 标记为脏页
        temp_buffer_pool->UnpinPage(0, true);
        
        // 销毁缓冲池，应该自动刷新脏页
    }
    
    // 重新创建缓冲池并检查页面数据是否已写回
    auto new_buffer_pool = std::make_unique<BufferPool>(disk_manager_.get(), 5, *config_manager_);
    Page* page = new_buffer_pool->FetchPage(0);
    ASSERT_NE(page, nullptr);
    EXPECT_EQ(page->GetData()[0], 'X');
}

// 测试获取页面
TEST_F(BufferPoolEnhancedTest, FetchPage) {
    // 测试获取已存在的页面
    Page* page = buffer_pool_->FetchPage(0);
    ASSERT_NE(page, nullptr);
    EXPECT_EQ(page->GetPageId(), 0);
    
    // 测试获取不存在的页面
    Page* non_existent_page = buffer_pool_->FetchPage(100);
    EXPECT_EQ(non_existent_page, nullptr);
    
    // 测试重复获取同一页面
    Page* same_page = buffer_pool_->FetchPage(0);
    EXPECT_EQ(page, same_page); // 应该是同一个对象
}

// 测试取消固定页面
TEST_F(BufferPoolEnhancedTest, UnpinPage) {
    // 获取页面
    Page* page = buffer_pool_->FetchPage(0);
    ASSERT_NE(page, nullptr);
    
    // 测试正常取消固定
    bool result = buffer_pool_->UnpinPage(0, false);
    EXPECT_TRUE(result);
    
    // 测试取消固定不存在的页面
    result = buffer_pool_->UnpinPage(100, false);
    EXPECT_FALSE(result);
    
    // 测试标记为脏页
    page = buffer_pool_->FetchPage(1);
    ASSERT_NE(page, nullptr);
    
    // 修改页面数据
    char* data = page->GetData();
    data[0] = 'Y';
    
    // 标记为脏页
    result = buffer_pool_->UnpinPage(1, true);
    EXPECT_TRUE(result);
    
    // 重新获取页面并验证数据
    page = buffer_pool_->FetchPage(1);
    ASSERT_NE(page, nullptr);
    EXPECT_EQ(page->GetData()[0], 'Y');
}

// 测试刷新页面
TEST_F(BufferPoolEnhancedTest, FlushPage) {
    // 获取页面并修改
    Page* page = buffer_pool_->FetchPage(0);
    ASSERT_NE(page, nullptr);
    
    char* data = page->GetData();
    data[0] = 'Z';
    
    // 标记为脏页
    buffer_pool_->UnpinPage(0, true);
    
    // 刷新页面
    bool result = buffer_pool_->FlushPage(0);
    EXPECT_TRUE(result);
    
    // 刷新不存在的页面
    result = buffer_pool_->FlushPage(100);
    EXPECT_FALSE(result);
    
    // 验证数据已写回磁盘
    // 创建新的缓冲池来验证
    auto new_buffer_pool = std::make_unique<BufferPool>(disk_manager_.get(), 5, *config_manager_);
    page = new_buffer_pool->FetchPage(0);
    ASSERT_NE(page, nullptr);
    EXPECT_EQ(page->GetData()[0], 'Z');
}

// 测试刷新所有页面
TEST_F(BufferPoolEnhancedTest, FlushAllPages) {
    // 获取并修改多个页面
    for (int i = 0; i < 3; ++i) {
        Page* page = buffer_pool_->FetchPage(i);
        ASSERT_NE(page, nullptr);
        
        char* data = page->GetData();
        data[0] = 'A' + i;
        
        buffer_pool_->UnpinPage(i, true);
    }
    
    // 刷新所有页面
    buffer_pool_->FlushAllPages();
    
    // 验证数据已写回磁盘
    auto new_buffer_pool = std::make_unique<BufferPool>(disk_manager_.get(), 5, *config_manager_);
    for (int i = 0; i < 3; ++i) {
        Page* page = new_buffer_pool->FetchPage(i);
        ASSERT_NE(page, nullptr);
        EXPECT_EQ(page->GetData()[0], 'A' + i);
    }
}

// 测试创建新页面
TEST_F(BufferPoolEnhancedTest, NewPage) {
    int32_t new_page_id;
    Page* new_page = buffer_pool_->NewPage(&new_page_id);
    
    ASSERT_NE(new_page, nullptr);
    EXPECT_EQ(new_page->GetPageId(), new_page_id);
    EXPECT_GE(new_page_id, 0);
    
    // 取消固定页面
    buffer_pool_->UnpinPage(new_page_id, false);
    
    // 重新获取页面
    Page* fetched_page = buffer_pool_->FetchPage(new_page_id);
    EXPECT_EQ(new_page, fetched_page);
}

// 测试删除页面
TEST_F(BufferPoolEnhancedTest, DeletePage) {
    // 创建新页面
    int32_t new_page_id;
    Page* new_page = buffer_pool_->NewPage(&new_page_id);
    ASSERT_NE(new_page, nullptr);
    buffer_pool_->UnpinPage(new_page_id, false);
    
    // 删除页面
    bool result = buffer_pool_->DeletePage(new_page_id);
    EXPECT_TRUE(result);
    
    // 尝试获取已删除的页面
    Page* deleted_page = buffer_pool_->FetchPage(new_page_id);
    EXPECT_EQ(deleted_page, nullptr);
    
    // 删除不存在的页面
    result = buffer_pool_->DeletePage(100);
    EXPECT_FALSE(result);
    
    // 尝试删除正在使用的页面
    Page* page = buffer_pool_->FetchPage(0);
    ASSERT_NE(page, nullptr);
    result = buffer_pool_->DeletePage(0);
    EXPECT_FALSE(result); // 应该失败，因为页面正在使用
}

// 测试缓冲池满时的页面替换
TEST_F(BufferPoolEnhancedTest, BufferPoolReplacement) {
    // 创建一个小的缓冲池
    auto small_buffer_pool = std::make_unique<BufferPool>(disk_manager_.get(), 2, *config_manager_);
    
    // 填满缓冲池
    Page* page0 = small_buffer_pool->FetchPage(0);
    Page* page1 = small_buffer_pool->FetchPage(1);
    
    ASSERT_NE(page0, nullptr);
    ASSERT_NE(page1, nullptr);
    
    // 取消固定页面
    small_buffer_pool->UnpinPage(0, false);
    small_buffer_pool->UnpinPage(1, false);
    
    // 获取新页面，应该触发替换
    Page* page2 = small_buffer_pool->FetchPage(2);
    ASSERT_NE(page2, nullptr);
    
    // 再次获取页面0，应该已经不在缓冲池中
    Page* page0_again = small_buffer_pool->FetchPage(0);
    ASSERT_NE(page0_again, nullptr);
    EXPECT_NE(page0, page0_again); // 应该是不同的对象
}

// 测试批量获取页面
TEST_F(BufferPoolEnhancedTest, BatchFetchPages) {
    std::vector<int32_t> page_ids = {0, 1, 2, 10}; // 包含一个不存在的页面
    std::vector<Page*> pages = buffer_pool_->BatchFetchPages(page_ids);
    
    ASSERT_EQ(pages.size(), 4);
    EXPECT_NE(pages[0], nullptr); // 页面0存在
    EXPECT_NE(pages[1], nullptr); // 页面1存在
    EXPECT_NE(pages[2], nullptr); // 页面2存在
    EXPECT_EQ(pages[3], nullptr); // 页面10不存在
    
    // 测试无效页面ID
    std::vector<int32_t> invalid_page_ids = {-1, 0, 1};
    std::vector<Page*> pages_with_invalid = buffer_pool_->BatchFetchPages(invalid_page_ids);
    
    ASSERT_EQ(pages_with_invalid.size(), 3);
    EXPECT_EQ(pages_with_invalid[0], nullptr); // 无效ID
    EXPECT_NE(pages_with_invalid[1], nullptr); // 页面0存在
    EXPECT_NE(pages_with_invalid[2], nullptr); // 页面1存在
}

// 测试预取页面
TEST_F(BufferPoolEnhancedTest, PrefetchPage) {
    // 预取存在的页面
    bool result = buffer_pool_->PrefetchPage(3);
    EXPECT_TRUE(result);
    
    // 预取不存在的页面
    result = buffer_pool_->PrefetchPage(100);
    EXPECT_FALSE(result);
    
    // 预取无效页面ID
    result = buffer_pool_->PrefetchPage(-1);
    EXPECT_FALSE(result);
    
    // 预取已在缓冲池中的页面
    Page* page = buffer_pool_->FetchPage(0);
    ASSERT_NE(page, nullptr);
    buffer_pool_->UnpinPage(0, false);
    
    result = buffer_pool_->PrefetchPage(0);
    EXPECT_TRUE(result); // 已在缓冲池中，应该返回true
}

// 测试批量预取页面
TEST_F(BufferPoolEnhancedTest, BatchPrefetchPages) {
    std::vector<int32_t> page_ids = {0, 1, 2, 10}; // 包含一个不存在的页面
    
    // 批量预取
    bool result = buffer_pool_->BatchPrefetchPages(page_ids);
    EXPECT_TRUE(result);
    
    // 验证页面已在缓冲池中
    Page* page0 = buffer_pool_->FetchPage(0);
    Page* page1 = buffer_pool_->FetchPage(1);
    Page* page2 = buffer_pool_->FetchPage(2);
    
    EXPECT_NE(page0, nullptr);
    EXPECT_NE(page1, nullptr);
    EXPECT_NE(page2, nullptr);
    
    // 测试包含无效ID的批量预取
    std::vector<int32_t> invalid_page_ids = {-1, 0, 1};
    result = buffer_pool_->BatchPrefetchPages(invalid_page_ids);
    EXPECT_TRUE(result); // 应该忽略无效ID并返回true
}

// 测试配置变更回调
TEST_F(BufferPoolEnhancedTest, ConfigChangeCallback) {
    // 测试缓冲池大小变更
    ConfigValue pool_size_value(20);
    buffer_pool_->OnConfigChange("buffer_pool.pool_size", pool_size_value);
    
    // 测试预取开关变更
    ConfigValue prefetch_value(true);
    buffer_pool_->OnConfigChange("buffer_pool.enable_prefetch", prefetch_value);
    
    // 测试预取策略变更
    ConfigValue strategy_value("SEQUENTIAL");
    buffer_pool_->OnConfigChange("buffer_pool.prefetch_strategy", strategy_value);
    
    // 测试预取窗口大小变更
    ConfigValue window_value(8);
    buffer_pool_->OnConfigChange("buffer_pool.prefetch_window", window_value);
    
    // 测试无效的配置键
    ConfigValue invalid_value(100);
    buffer_pool_->OnConfigChange("invalid.config.key", invalid_value);
}

// 测试线程安全性
TEST_F(BufferPoolEnhancedTest, ThreadSafety) {
    const int num_threads = 5;
    const int operations_per_thread = 10;
    std::vector<std::thread> threads;
    
    // 启动多个线程同时访问缓冲池
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, i, operations_per_thread]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                int32_t page_id = (i * operations_per_thread + j) % 5; // 使用0-4的页面ID
                Page* page = buffer_pool_->FetchPage(page_id);
                if (page) {
                    // 修改页面数据
                    char* data = page->GetData();
                    data[0] = static_cast<char>('A' + i);
                    
                    // 取消固定页面
                    buffer_pool_->UnpinPage(page_id, true);
                    
                    // 短暂休眠，增加线程交错的可能性
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证所有页面都存在且数据一致
    for (int i = 0; i < 5; ++i) {
        Page* page = buffer_pool_->FetchPage(i);
        ASSERT_NE(page, nullptr);
        // 由于多个线程可能修改了同一页面，我们只验证页面存在
        buffer_pool_->UnpinPage(i, false);
    }
}

} // namespace test
} // namespace sqlcc