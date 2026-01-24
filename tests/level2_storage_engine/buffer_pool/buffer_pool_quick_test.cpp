/**
 * @file buffer_pool_quick_test.cpp
 * @brief 缓冲池快速单元测试 - 分离出快速的基础功能测试，避免超时
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <filesystem>

#include "storage_engine/buffer_pool/lru_manager.h"
#include "storage_engine/buffer_pool/statistics_collector.h"


#include "utils/config_manager.h"

using sqlcc::LRUManager;
using sqlcc::StatisticsCollector;

namespace fs = std::filesystem;
namespace sqlcc {

class BufferPoolQuickTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时测试目录
        test_dir = fs::temp_directory_path() / "sqlcc_buffer_pool_quick_test";
        fs::create_directories(test_dir);

        // 初始化配置管理器（使用单例模式）
        config = &ConfigManager::GetInstance();
        config->SetValue("storage.data_directory", test_dir.string());
        config->SetValue("buffer_pool.size", std::string("4096"));

        // 初始化存储引擎
        storage_engine = std::make_shared<StorageEngine>(*config, test_dir.string());

        // 初始化LRU管理器和统计收集器
        lru_manager = std::make_unique<LRUManager>(1000);  // 容量1000
        stats_collector = std::make_unique<StatisticsCollector>("buffer_pool_test");
    }

    void TearDown() override {
        stats_collector.reset();
        lru_manager.reset();
        storage_engine.reset();
        config.reset();

        // 清理测试目录
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }

    // 辅助函数：创建测试页面
    std::shared_ptr<Page> CreateTestPage(int32_t page_id) {
        return storage_engine->NewPage(&page_id);
    }

    fs::path test_dir;
    ConfigManager* config;
    std::shared_ptr<StorageEngine> storage_engine;
    std::unique_ptr<LRUManager> lru_manager;
    std::unique_ptr<StatisticsCollector> stats_collector;
};

// 测试LRU管理器的基本功能
TEST_F(BufferPoolQuickTest, LRUBasicOperations) {
    // 测试添加页面到LRU管理器
    lru_manager->Add(1);
    EXPECT_TRUE(lru_manager->Contains(1));
    EXPECT_EQ(lru_manager->Size(), 1);

    lru_manager->Add(2);
    EXPECT_TRUE(lru_manager->Contains(2));
    EXPECT_EQ(lru_manager->Size(), 2);

    // 测试访问页面（移到头部）
    lru_manager->Access(1);
    EXPECT_TRUE(lru_manager->Contains(1));

    // 测试移除页面
    lru_manager->Remove(1);
    EXPECT_FALSE(lru_manager->Contains(1));
    EXPECT_EQ(lru_manager->Size(), 1);
}

// 测试LRU边界条件
TEST_F(BufferPoolQuickTest, LRUBoundaryConditions) {
    // 测试空LRU管理器
    EXPECT_EQ(lru_manager->Size(), 0);
    EXPECT_EQ(lru_manager->GetLeastRecentlyUsed(), -1);
    EXPECT_FALSE(lru_manager->Contains(0));
    lru_manager->Remove(0);

    // 添加页面后移除
    lru_manager->Add(1);
    lru_manager->Remove(1);
    EXPECT_EQ(lru_manager->Size(), 0);
    EXPECT_EQ(lru_manager->GetLeastRecentlyUsed(), -1);

    // 测试重复添加
    lru_manager->Add(1);
    lru_manager->Add(1);  // 重复添加应该不影响
    EXPECT_EQ(lru_manager->Size(), 1);

    // 测试访问不存在的页面
    lru_manager->Access(999);  // 不应该崩溃
    EXPECT_EQ(lru_manager->Size(), 1);  // 大小不变

    // 清理
    lru_manager->Remove(1);
    EXPECT_EQ(lru_manager->Size(), 0);
}

// 测试统计收集器基本功能
TEST_F(BufferPoolQuickTest, StatisticsCollectorBasic) {
    // 测试初始状态
    EXPECT_EQ(stats_collector->GetTotalAccesses(), 0);
    EXPECT_EQ(stats_collector->GetTotalHits(), 0);
    EXPECT_DOUBLE_EQ(stats_collector->GetHitRate(), 0.0);

    // 记录页面访问
    stats_collector->RecordPageAccess(1);
    stats_collector->RecordPageAccess(2);
    stats_collector->RecordPageAccess(1);  // 重复访问页面1

    // 记录命中和未命中
    stats_collector->RecordPageHit();
    stats_collector->RecordPageHit();
    stats_collector->RecordPageMiss();

    // 记录替换和刷新
    stats_collector->RecordPageReplacement();
    stats_collector->RecordPageFlush();

    // 验证统计信息
    EXPECT_EQ(stats_collector->GetTotalAccesses(), 3);
    EXPECT_EQ(stats_collector->GetTotalHits(), 2);
    EXPECT_DOUBLE_EQ(stats_collector->GetHitRate(), 2.0/3.0);
    EXPECT_EQ(stats_collector->GetReplacementCount(), 1);
    EXPECT_EQ(stats_collector->GetFlushCount(), 1);

    // 验证访问频率
    auto access_freq = stats_collector->GetAccessFrequency();
    EXPECT_EQ(access_freq[1], 2);  // 页面1被访问2次
    EXPECT_EQ(access_freq[2], 1);  // 页面2被访问1次
}

// 测试统计收集器重置功能
TEST_F(BufferPoolQuickTest, StatisticsCollectorReset) {
    // 记录一些统计信息
    stats_collector->RecordPageHit();
    stats_collector->RecordPageMiss();
    stats_collector->RecordPageReplacement();

    EXPECT_GT(stats_collector->GetTotalAccesses(), 0);

    // 重置统计信息
    stats_collector->Reset();

    // 验证重置后状态
    EXPECT_EQ(stats_collector->GetTotalAccesses(), 0);
    EXPECT_EQ(stats_collector->GetTotalHits(), 0);
    EXPECT_DOUBLE_EQ(stats_collector->GetHitRate(), 0.0);
    EXPECT_EQ(stats_collector->GetReplacementCount(), 0);
    EXPECT_EQ(stats_collector->GetFlushCount(), 0);
}

// 测试LRU和统计收集器的集成
TEST_F(BufferPoolQuickTest, LRUAndStatisticsIntegration) {
    const int num_pages = 10;

    // 模拟缓冲池操作
    for (int i = 0; i < num_pages; ++i) {
        lru_manager->Add(i);
        stats_collector->RecordPageAccess(i);

        // 模拟一些命中和未命中
        if (i % 3 == 0) {
            stats_collector->RecordPageHit();
        } else {
            stats_collector->RecordPageMiss();
        }
    }

    // 访问一些页面，更新LRU顺序
    for (int i = 0; i < 5; ++i) {
        lru_manager->Access(i);
        stats_collector->RecordPageAccess(i);
        stats_collector->RecordPageHit();  // 访问现有页面应该是命中
    }

    // 验证集成状态
    EXPECT_EQ(lru_manager->Size(), num_pages);
    EXPECT_GT(stats_collector->GetTotalAccesses(), num_pages);
    EXPECT_GT(stats_collector->GetHitRate(), 0.0);

    // 移除最少使用的页面
    int32_t lru_page = lru_manager->GetLeastRecentlyUsed();
    if (lru_page != -1) {
        lru_manager->Remove(lru_page);
        stats_collector->RecordPageReplacement();
        EXPECT_FALSE(lru_manager->Contains(lru_page));
        EXPECT_EQ(stats_collector->GetReplacementCount(), 1);
    }
}

// 测试缓冲池错误处理
TEST_F(BufferPoolQuickTest, BufferPoolErrorHandling) {
    // 测试LRU管理器的错误处理
    lru_manager->Remove(-1);  // 无效页面ID
    EXPECT_FALSE(lru_manager->Contains(-1));

    lru_manager->Access(-1);  // 访问无效页面，不应该崩溃
    EXPECT_EQ(lru_manager->Size(), 0);  // 大小不变

    // 测试统计收集器的鲁棒性
    stats_collector->RecordPageAccess(-1);  // 无效页面ID，不应该崩溃
    EXPECT_GT(stats_collector->GetTotalAccesses(), 0);

    // 测试空状态下的操作
    EXPECT_EQ(lru_manager->GetLeastRecentlyUsed(), -1);
    EXPECT_EQ(stats_collector->GetHitRate(), 0.0);  // 没有访问时的命中率
}

// 测试缓冲池清理和重置
TEST_F(BufferPoolQuickTest, BufferPoolCleanupAndReset) {
    // 添加一些数据
    for (int i = 0; i < 5; ++i) {
        lru_manager->Add(i);
        stats_collector->RecordPageAccess(i);
        stats_collector->RecordPageHit();
    }

    EXPECT_GT(lru_manager->Size(), 0);
    EXPECT_GT(stats_collector->GetTotalAccesses(), 0);

    // 注意：LRUManager没有显式的Clear方法，这里我们逐个移除
    for (int i = 0; i < 5; ++i) {
        lru_manager->Remove(i);
    }

    // 重置统计信息
    stats_collector->Reset();

    // 验证清理后的状态
    EXPECT_EQ(lru_manager->Size(), 0);
    EXPECT_EQ(stats_collector->GetTotalAccesses(), 0);
    EXPECT_EQ(stats_collector->GetTotalHits(), 0);
    EXPECT_DOUBLE_EQ(stats_collector->GetHitRate(), 0.0);
}

} // namespace sqlcc
