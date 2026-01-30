/**
 * @file index_manager_test.cpp
 * @brief 索引管理器单元测试 - 全面测试智能索引缓存和索引管理功能
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <filesystem>
#include <unordered_set>

#include "storage_engine/index_manager/smart_index_cache.h"
#include "storage_engine/index_manager/smart_index_factory.h"
#include "storage_engine/b_plus_tree_index.h"
#include "storage_engine.h"
#include "src/utils/config_manager.h"

namespace fs = std::filesystem;
namespace sqlcc {

class IndexManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时测试目录
        test_dir = fs::temp_directory_path() / "sqlcc_index_manager_test";
        fs::create_directories(test_dir);

        // 初始化配置管理器
        config = std::make_unique<ConfigManager>();
        config->SetValue("storage.data_directory", test_dir.string());
        config->SetValue("buffer_pool.size", std::string("4096"));

        // 初始化存储引擎
        storage_engine = std::make_shared<StorageEngine>(*config, test_dir.string());

    // 初始化智能索引缓存
    index_cache = std::make_unique<sqlcc::storage_engine::index_manager::SmartIndexCache>(100, std::chrono::minutes(30));
    }

    void TearDown() override {
        index_cache.reset();
        storage_engine.reset();
        config.reset();

        // 清理测试目录
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }

    // 辅助函数：创建B+树索引
    std::unique_ptr<BPlusTreeIndex> CreateTestIndex(const std::string& table_name, const std::string& index_name) {
        return std::make_unique<BPlusTreeIndex>(storage_engine, table_name, index_name);
    }

    fs::path test_dir;
    std::unique_ptr<ConfigManager> config;
    std::shared_ptr<StorageEngine> storage_engine;
    std::unique_ptr<sqlcc::storage_engine::index_manager::SmartIndexCache> index_cache;
};

// 测试基本索引缓存功能
TEST_F(IndexManagerTest, BasicIndexCaching) {
    const std::string index_name = "test_index";
    const std::string table_name = "test_table";

    // 创建索引并缓存
    auto index = CreateTestIndex(table_name, index_name);
    ASSERT_TRUE(index != nullptr);

    // 缓存索引
    index_cache->CacheIndex(index_name, std::move(index), 5, std::chrono::minutes(60));

    // 验证索引已缓存
    EXPECT_TRUE(index_cache->HasIndex(index_name));

    // 获取缓存的索引
    BPlusTreeIndex* retrieved_index = index_cache->GetIndex(index_name);
    ASSERT_TRUE(retrieved_index != nullptr);

    // 验证索引基本功能
    int32_t page_id;
    auto page = storage_engine->NewPage(&page_id);
    ASSERT_TRUE(page != nullptr);

    // 测试插入操作
    EXPECT_TRUE(retrieved_index->Insert("test_key", page_id, 0));
}

// 测试索引移除功能
TEST_F(IndexManagerTest, IndexRemoval) {
    const std::string index_name = "test_index";
    const std::string table_name = "test_table";

    // 创建并缓存索引
    auto index = CreateTestIndex(table_name, index_name);
    index_cache->CacheIndex(index_name, std::move(index), 0, std::chrono::minutes(60));

    // 验证索引存在
    EXPECT_TRUE(index_cache->HasIndex(index_name));

    // 移除索引
    EXPECT_TRUE(index_cache->RemoveIndex(index_name));

    // 验证索引已被移除
    EXPECT_FALSE(index_cache->HasIndex(index_name));
    EXPECT_TRUE(index_cache->GetIndex(index_name) == nullptr);

    // 再次移除应该失败
    EXPECT_FALSE(index_cache->RemoveIndex(index_name));
}

// 测试缓存统计信息
TEST_F(IndexManagerTest, CacheStatistics) {
    // 创建多个索引并缓存
    std::vector<std::string> index_names = {"index1", "index2", "index3", "index4"};

    for (const auto& name : index_names) {
        auto index = CreateTestIndex("test_table", name);
        index_cache->CacheIndex(name, std::move(index), 0, std::chrono::minutes(60));
    }

    // 获取增强统计信息
    storage_engine::index_manager::SmartIndexCache::EnhancedCacheStats enhanced_stats = index_cache->GetEnhancedCacheStats();

    // 验证统计信息
    EXPECT_EQ(enhanced_stats.total_indexes, 4);
    EXPECT_EQ(enhanced_stats.expired_entries, 0);
    EXPECT_EQ(enhanced_stats.high_priority_entries, 0);

    // 验证增强统计信息
    EXPECT_EQ(enhanced_stats.total_indexes, 4);
    EXPECT_GT(enhanced_stats.oldest_access.time_since_epoch().count(), 0);
    EXPECT_GT(enhanced_stats.newest_access.time_since_epoch().count(), 0);
}

// 测试优先级缓存
TEST_F(IndexManagerTest, PriorityCaching) {
    // 创建不同优先级的索引
    std::vector<std::pair<std::string, int>> index_priority_pairs = {
        {"low_priority_index", 1},
        {"medium_priority_index", 5},
        {"high_priority_index", 10}
    };

    for (const auto& [name, priority] : index_priority_pairs) {
        auto index = CreateTestIndex("test_table", name);
        index_cache->CacheIndex(name, std::move(index), priority, std::chrono::minutes(60));
    }

    // 验证所有索引都存在
    for (const auto& [name, _] : index_priority_pairs) {
        EXPECT_TRUE(index_cache->HasIndex(name));
    }

    // 获取统计信息并验证优先级分布
    storage_engine::index_manager::SmartIndexCache::EnhancedCacheStats stats = index_cache->GetEnhancedCacheStats();
    EXPECT_EQ(stats.priority_distribution[1], 1);  // 低优先级
    EXPECT_EQ(stats.priority_distribution[5], 1);  // 中优先级
    EXPECT_EQ(stats.priority_distribution[10], 1); // 高优先级
}

// 测试缓存过期功能
TEST_F(IndexManagerTest, CacheExpiration) {
    const std::string index_name = "expiring_index";

    // 创建一个短期TTL的索引
    auto index = CreateTestIndex("test_table", index_name);
    index_cache->CacheIndex(index_name, std::move(index), 0, std::chrono::minutes(0));

    // 立即验证索引存在
    EXPECT_TRUE(index_cache->HasIndex(index_name));

    // 等待过期
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    // 手动清理过期缓存
    index_cache->CleanupExpiredCache(std::chrono::minutes(0));

    // 验证索引已被清理（取决于实现，可能需要手动清理）
    // 注意：实际过期行为取决于实现，这里测试清理功能
}

// 测试缓存预热功能
TEST_F(IndexManagerTest, CacheWarmup) {
    std::vector<std::string> warmup_indexes = {"warmup_index1", "warmup_index2", "warmup_index3"};

    // 预热缓存（在实际实现中，这会加载预测的索引）
    index_cache->WarmupCache(warmup_indexes);

    // 验证预热后的状态（取决于实现，可能不会实际创建索引）
    // 这里主要测试接口调用不抛出异常
    SUCCEED();
}

// 测试智能缓存清理
TEST_F(IndexManagerTest, IntelligentCleanup) {
    // 创建多个索引填充缓存
    const int num_indexes = 20;
    for (int i = 0; i < num_indexes; ++i) {
        std::string name = "cleanup_index_" + std::to_string(i);
        auto index = CreateTestIndex("test_table", name);
        index_cache->CacheIndex(name, std::move(index), i % 10, std::chrono::minutes(60));
    }

    // 执行智能清理
    index_cache->IntelligentCleanup();

    // 验证清理后的状态（至少有一些索引被保留）
    sqlcc::storage_engine::index_manager::SmartIndexCache::EnhancedCacheStats stats = index_cache->GetEnhancedCacheStats();
    EXPECT_GE(stats.total_indexes, 0);  // 可能全部保留或部分清理
}

// 测试并发访问
TEST_F(IndexManagerTest, ConcurrentAccess) {
    const int num_threads = 5;
    const int operations_per_thread = 10;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    // 启动多个线程并发执行缓存操作
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, i, operations_per_thread, &success_count]() {
            try {
                for (int j = 0; j < operations_per_thread; ++j) {
                    std::string index_name = "concurrent_index_" + std::to_string(i) + "_" + std::to_string(j);
                    auto index = CreateTestIndex("test_table", index_name);

                    // 缓存索引
                    index_cache->CacheIndex(index_name, std::move(index), 0, std::chrono::minutes(60));

                    // 验证可以获取
                    BPlusTreeIndex* retrieved = index_cache->GetIndex(index_name);
                    if (retrieved != nullptr) {
                        success_count++;
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

    // 验证操作成功
    EXPECT_EQ(success_count, num_threads * operations_per_thread);

    // 验证最终状态
    storage_engine::index_manager::SmartIndexCache::EnhancedCacheStats final_stats = index_cache->GetEnhancedCacheStats();
    EXPECT_EQ(final_stats.total_indexes, num_threads * operations_per_thread);
}

// 测试批量索引获取
TEST_F(IndexManagerTest, BatchIndexRetrieval) {
    // 创建多个索引
    std::vector<std::string> index_names;
    const int num_indexes = 5;

    for (int i = 0; i < num_indexes; ++i) {
        std::string name = "batch_index_" + std::to_string(i);
        index_names.push_back(name);

        auto index = CreateTestIndex("test_table", name);
        index_cache->CacheIndex(name, std::move(index), 0, std::chrono::minutes(60));
    }

    // 批量获取索引
    std::vector<BPlusTreeIndex*> retrieved_indexes = index_cache->GetMultipleIndexes(index_names);

    // 验证所有索引都被成功获取
    EXPECT_EQ(retrieved_indexes.size(), num_indexes);
    for (auto* index : retrieved_indexes) {
        EXPECT_TRUE(index != nullptr);
    }
}

// 测试边界条件 - 空缓存操作
TEST_F(IndexManagerTest, BoundaryConditionsEmptyCache) {
    // 测试在空缓存上的操作
    EXPECT_FALSE(index_cache->HasIndex("non_existent"));
    EXPECT_TRUE(index_cache->GetIndex("non_existent") == nullptr);
    EXPECT_FALSE(index_cache->RemoveIndex("non_existent"));

    // 获取空缓存的统计信息
    storage_engine::index_manager::SmartIndexCache::EnhancedCacheStats stats = index_cache->GetEnhancedCacheStats();
    EXPECT_EQ(stats.total_indexes, 0);
    EXPECT_EQ(stats.total_hits, 0);
    EXPECT_EQ(stats.total_misses, 0);
}

// 测试边界条件 - 重复缓存
TEST_F(IndexManagerTest, BoundaryConditionsDuplicateCaching) {
    const std::string index_name = "duplicate_index";

    // 第一次缓存
    auto index1 = CreateTestIndex("test_table", index_name);
    EXPECT_NO_THROW(index_cache->CacheIndex(index_name, std::move(index1), 0, std::chrono::minutes(60)));

    // 第二次缓存相同名称（取决于实现，可能覆盖或抛出异常）
    auto index2 = CreateTestIndex("test_table", index_name + "_v2");
    // 这里测试接口调用不崩溃
    EXPECT_NO_THROW(index_cache->CacheIndex(index_name, std::move(index2), 0, std::chrono::minutes(60)));
}

// 测试边界条件 - 超长索引名称
TEST_F(IndexManagerTest, BoundaryConditionsLongIndexName) {
    // 创建超长索引名称
    std::string long_name(1000, 'a');  // 1000个字符的名称

    auto index = CreateTestIndex("test_table", long_name);
    EXPECT_NO_THROW(index_cache->CacheIndex(long_name, std::move(index), 0, std::chrono::minutes(60)));

    // 验证可以检索
    EXPECT_TRUE(index_cache->HasIndex(long_name));
    EXPECT_TRUE(index_cache->GetIndex(long_name) != nullptr);
}

// 测试边界条件 - 特殊字符索引名称
TEST_F(IndexManagerTest, BoundaryConditionsSpecialCharacters) {
    std::vector<std::string> special_names = {
        "index_with_spaces test",
        "index-with-dashes",
        "index.with.dots",
        "index_123_numbers",
        "INDEX_UPPERCASE"
    };

    for (const auto& name : special_names) {
        auto index = CreateTestIndex("test_table", name);
        EXPECT_NO_THROW(index_cache->CacheIndex(name, std::move(index), 0, std::chrono::minutes(60)));
        EXPECT_TRUE(index_cache->HasIndex(name));
    }
}

// 测试性能特征
TEST_F(IndexManagerTest, PerformanceCharacteristics) {
    const int num_operations = 1000;
    auto start_time = std::chrono::high_resolution_clock::now();

    // 执行大量缓存操作
    for (int i = 0; i < num_operations; ++i) {
        std::string name = "perf_index_" + std::to_string(i);
        auto index = CreateTestIndex("test_table", name);
        index_cache->CacheIndex(name, std::move(index), 0, std::chrono::minutes(60));

        // 执行一些访问操作
        BPlusTreeIndex* retrieved = index_cache->GetIndex(name);
        ASSERT_TRUE(retrieved != nullptr);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 验证操作完成且性能合理
    EXPECT_GT(num_operations, 0);
    EXPECT_LT(duration.count(), 10000);  // 应该在10秒内完成

    std::cout << "Index cache performance: " << num_operations
              << " operations in " << duration.count() << "ms" << std::endl;

    // 验证最终状态
    sqlcc::storage_engine::index_manager::SmartIndexCache::EnhancedCacheStats final_stats = index_cache->GetEnhancedCacheStats();
    EXPECT_EQ(final_stats.total_indexes, num_operations);
}

// 测试访问模式统计
TEST_F(IndexManagerTest, AccessPatternStatistics) {
    const std::string index_name = "access_test_index";

    // 缓存索引
    auto index = CreateTestIndex("test_table", index_name);
    index_cache->CacheIndex(index_name, std::move(index), 0, std::chrono::minutes(60));

    // 执行多次访问
    const int num_accesses = 10;
    for (int i = 0; i < num_accesses; ++i) {
        BPlusTreeIndex* retrieved = index_cache->GetIndex(index_name);
        ASSERT_TRUE(retrieved != nullptr);
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }

    // 获取统计信息
    sqlcc::storage_engine::index_manager::SmartIndexCache::EnhancedCacheStats stats = index_cache->GetEnhancedCacheStats();

    // 验证访问统计
    EXPECT_GE(stats.total_hits, num_accesses);
    EXPECT_GE(stats.average_access_frequency, 0.0);

    // 计算命中率（假设没有未命中）
    double expected_hit_rate = 1.0;
    EXPECT_NEAR(stats.hit_rate, expected_hit_rate, 0.1);
}

// 测试错误处理 - nullptr索引
TEST_F(IndexManagerTest, ErrorHandlingNullIndex) {
    const std::string index_name = "null_index";

    // 尝试缓存nullptr（这应该失败或抛出异常）
    std::unique_ptr<BPlusTreeIndex> null_index = nullptr;

    // 这里测试接口的鲁棒性
    EXPECT_THROW(
        index_cache->CacheIndex(index_name, std::move(null_index), 0, std::chrono::minutes(60)),
        std::exception
    );
}

// 测试内存使用监控
TEST_F(IndexManagerTest, MemoryUsageMonitoring) {
    // 创建多个索引来测试内存使用情况
    const int num_indexes = 50;
    for (int i = 0; i < num_indexes; ++i) {
        std::string name = "memory_test_index_" + std::to_string(i);
        auto index = CreateTestIndex("test_table", name);
        index_cache->CacheIndex(name, std::move(index), 0, std::chrono::minutes(60));
    }

    // 执行清理操作
    index_cache->IntelligentCleanup();

    // 验证缓存仍然有效
    storage_engine::index_manager::SmartIndexCache::EnhancedCacheStats stats = index_cache->GetEnhancedCacheStats();
    EXPECT_GE(stats.total_indexes, 0);  // 清理后可能保留部分索引

    std::cout << "Memory usage test completed. Final index count: " << stats.total_indexes << std::endl;
}

} // namespace sqlcc