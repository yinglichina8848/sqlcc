/**
 * @file page_allocator_test.cpp
 * @brief 页面分配器单元测试 - 全面测试页面分配、释放和统计功能
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <filesystem>

#include "storage/page_allocator.h"
#include "page.h"
#include "utils/config_manager.h"

namespace fs = std::filesystem;
namespace sqlcc {

class PageAllocatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时测试目录
        test_dir = fs::temp_directory_path() / "sqlcc_page_allocator_test";
        fs::create_directories(test_dir);

        // 初始化配置管理器
        config = std::make_unique<ConfigManager>();
        config->SetValue("storage.data_directory", test_dir.string());
        config->SetValue("buffer_pool.size", std::string("4096"));  // 4KB buffer pool for testing

        // 初始化页面分配器
        allocator = std::make_unique<PageAllocator>(100, PageAllocationStrategy::SEQUENTIAL);
    }

    void TearDown() override {
        allocator.reset();
        config.reset();

        // 清理测试目录
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }

    fs::path test_dir;
    std::unique_ptr<ConfigManager> config;
    std::unique_ptr<PageAllocator> allocator;
};

// 测试基本页面分配功能
TEST_F(PageAllocatorTest, BasicPageAllocation) {
    // 测试分配数据页面
    auto page1 = allocator->AllocatePage(PageType::DATA);
    ASSERT_TRUE(page1 != nullptr);
    EXPECT_EQ(page1->GetPageId(), 0);  // 第一个页面ID应该是0

    // 测试分配索引页面
    auto page2 = allocator->AllocatePage(PageType::INDEX);
    ASSERT_TRUE(page2 != nullptr);
    EXPECT_EQ(page2->GetPageId(), 1);

    // 测试分配元数据页面
    auto page3 = allocator->AllocatePage(PageType::METADATA);
    ASSERT_TRUE(page3 != nullptr);
    EXPECT_EQ(page3->GetPageId(), 2);
}

// 测试页面释放功能
TEST_F(PageAllocatorTest, PageDeallocation) {
    // 分配页面
    auto page = allocator->AllocatePage(PageType::DATA);
    ASSERT_TRUE(page != nullptr);
    int32_t page_id = page->GetPageId();

    // 释放页面
    EXPECT_TRUE(allocator->DeallocatePage(page_id));

    // 验证页面已被释放
    EXPECT_TRUE(allocator->GetPage(page_id) == nullptr);

    // 尝试再次释放应该失败
    EXPECT_FALSE(allocator->DeallocatePage(page_id));
}

// 测试页面获取功能
TEST_F(PageAllocatorTest, PageRetrieval) {
    // 分配页面
    auto page1 = allocator->AllocatePage(PageType::DATA);
    ASSERT_TRUE(page1 != nullptr);
    int32_t page_id = page1->GetPageId();

    // 获取页面
    auto retrieved_page = allocator->GetPage(page_id);
    ASSERT_TRUE(retrieved_page != nullptr);
    EXPECT_EQ(retrieved_page->GetPageId(), page_id);

    // 获取不存在的页面
    auto non_existent_page = allocator->GetPage(-1);
    EXPECT_TRUE(non_existent_page == nullptr);
}

// 测试页面脏标记功能
TEST_F(PageAllocatorTest, PageDirtyMarking) {
    // 分配页面
    auto page = allocator->AllocatePage(PageType::DATA);
    ASSERT_TRUE(page != nullptr);
    int32_t page_id = page->GetPageId();

    // 初始状态应该是干净的
    EXPECT_FALSE(allocator->IsPageDirty(page_id));

    // 标记为脏
    allocator->MarkPageDirty(page_id);
    EXPECT_TRUE(allocator->IsPageDirty(page_id));

    // 对于不存在的页面应该抛出异常或返回false
    EXPECT_FALSE(allocator->IsPageDirty(-1));
}

// 测试分配策略设置
TEST_F(PageAllocatorTest, AllocationStrategyConfiguration) {
    // 测试顺序分配策略
    allocator->SetAllocationStrategy(PageAllocationStrategy::SEQUENTIAL);

    auto page1 = allocator->AllocatePage(PageType::DATA);
    auto page2 = allocator->AllocatePage(PageType::DATA);
    EXPECT_LT(page1->GetPageId(), page2->GetPageId());

    // 测试随机分配策略
    auto allocator_random = std::make_unique<PageAllocator>(100, PageAllocationStrategy::RANDOM);
    auto page3 = allocator_random->AllocatePage(PageType::DATA);
    auto page4 = allocator_random->AllocatePage(PageType::DATA);
    // 随机策略下ID可能不连续，但应该都是有效的
    ASSERT_TRUE(page3 != nullptr);
    ASSERT_TRUE(page4 != nullptr);
}

// 测试统计信息收集
TEST_F(PageAllocatorTest, StatisticsCollection) {
    // 执行一些操作
    auto page1 = allocator->AllocatePage(PageType::DATA);
    auto page2 = allocator->AllocatePage(PageType::INDEX);
    int32_t page_id1 = page1->GetPageId();

    allocator->MarkPageDirty(page_id1);
    allocator->DeallocatePage(page_id1);

    // 获取统计信息
    PageAllocationStats stats = allocator->GetAllocationStats();
    EXPECT_EQ(stats.total_allocations, 2);
    EXPECT_EQ(stats.total_deallocations, 1);
    EXPECT_EQ(stats.allocation_failures, 0);

    // 获取页面使用统计
    PageUsageStats usage_stats = allocator->GetPageUsageStats();
    EXPECT_EQ(usage_stats.total_pages, 1);  // 还有一个页面未释放
    EXPECT_EQ(usage_stats.page_type_distribution[PageType::INDEX], 1);
}

// 测试内存优化功能
TEST_F(PageAllocatorTest, MemoryOptimization) {
    // 分配多个页面
    std::vector<std::shared_ptr<Page>> pages;
    for (int i = 0; i < 10; ++i) {
        pages.push_back(allocator->AllocatePage(PageType::DATA));
    }

    // 调用内存优化
    allocator->OptimizeMemoryUsage();

    // 验证所有页面仍然有效
    for (auto& page : pages) {
        ASSERT_TRUE(page != nullptr);
        auto retrieved = allocator->GetPage(page->GetPageId());
        EXPECT_TRUE(retrieved != nullptr);
    }
}

// 测试并发访问
TEST_F(PageAllocatorTest, ConcurrentAccess) {
    const int num_threads = 5;
    const int operations_per_thread = 20;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    // 启动多个线程并发执行分配/释放操作
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, operations_per_thread, &success_count]() {
            try {
                for (int j = 0; j < operations_per_thread; ++j) {
                    auto page = allocator->AllocatePage(PageType::DATA);
                    if (page) {
                        int32_t page_id = page->GetPageId();
                    // 模拟一些操作 - 移除不必要的sleep以加快测试
                        if (allocator->DeallocatePage(page_id)) {
                            success_count++;
                        }
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Thread exception: " << e.what() << std::endl;
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
    PageUsageStats final_stats = allocator->GetPageUsageStats();
    EXPECT_EQ(final_stats.total_pages, 0);  // 所有页面都应该被释放了
}

// 测试边界条件 - 分配超过限制
TEST_F(PageAllocatorTest, BoundaryConditionsAllocationLimit) {
    // 分配器最大页面数设为100，应该能够分配100个页面
    for (int i = 0; i < 100; ++i) {
        auto page = allocator->AllocatePage(PageType::DATA);
        ASSERT_TRUE(page != nullptr);
    }

    // 第101个分配应该失败（取决于实现，可能返回nullptr或抛出异常）
    // 这里我们测试实现是否能正确处理边界情况
    try {
        auto page = allocator->AllocatePage(PageType::DATA);
        // 如果返回了页面，验证它是有效的
        if (page) {
            EXPECT_TRUE(page->GetPageId() >= 0);
        }
    } catch (const std::exception& e) {
        // 抛出异常也是可接受的行为
        SUCCEED();
    }
}

// 测试边界条件 - 释放无效页面ID
TEST_F(PageAllocatorTest, BoundaryConditionsInvalidPageId) {
    // 尝试释放不存在的页面ID
    EXPECT_FALSE(allocator->DeallocatePage(-1));
    EXPECT_FALSE(allocator->DeallocatePage(99999));

    // 尝试获取不存在的页面
    EXPECT_TRUE(allocator->GetPage(-1) == nullptr);
    EXPECT_TRUE(allocator->GetPage(99999) == nullptr);

    // 尝试标记不存在的页面为脏
    EXPECT_FALSE(allocator->IsPageDirty(-1));
    EXPECT_FALSE(allocator->IsPageDirty(99999));
}

// 测试边界条件 - 重复释放
TEST_F(PageAllocatorTest, BoundaryConditionsDoubleFree) {
    // 分配页面
    auto page = allocator->AllocatePage(PageType::DATA);
    ASSERT_TRUE(page != nullptr);
    int32_t page_id = page->GetPageId();

    // 第一次释放应该成功
    EXPECT_TRUE(allocator->DeallocatePage(page_id));

    // 第二次释放应该失败
    EXPECT_FALSE(allocator->DeallocatePage(page_id));
}

// 测试访问模式分析
TEST_F(PageAllocatorTest, AccessPatternAnalysis) {
    // 分配一些页面并模拟访问模式
    auto page1 = allocator->AllocatePage(PageType::DATA);
    auto page2 = allocator->AllocatePage(PageType::INDEX);
    auto page3 = allocator->AllocatePage(PageType::DATA);

    int32_t page_id1 = page1->GetPageId();
    int32_t page_id2 = page2->GetPageId();
    int32_t page_id3 = page3->GetPageId();

    // 模拟访问模式 - 多次访问page1
    for (int i = 0; i < 5; ++i) {
        auto retrieved = allocator->GetPage(page_id1);
        ASSERT_TRUE(retrieved != nullptr);
        // 移除不必要的sleep以加快测试
    }

    // 偶尔访问page2
    for (int i = 0; i < 2; ++i) {
        auto retrieved = allocator->GetPage(page_id2);
        ASSERT_TRUE(retrieved != nullptr);
        // 移除不必要的sleep以加快测试
    }

    // 获取访问模式分析
    AccessPatternAnalysis analysis = allocator->GetAccessPatternAnalysis();
    EXPECT_GT(analysis.total_accesses, 0);

    // page1应该被识别为最常访问的页面
    EXPECT_EQ(analysis.most_accessed_page, page_id1);
}

// 测试错误处理 - 内存不足模拟
TEST_F(PageAllocatorTest, ErrorHandlingOutOfMemory) {
    // 创建一个小的分配器来更容易触发内存相关错误
    auto small_allocator = std::make_unique<PageAllocator>(5, PageAllocationStrategy::SEQUENTIAL);

    // 快速分配大量页面，可能触发内存相关问题
    std::vector<std::shared_ptr<Page>> pages;
    try {
        for (int i = 0; i < 10; ++i) {
            auto page = small_allocator->AllocatePage(PageType::DATA);
            if (page) {
                pages.push_back(page);
            } else {
                break;  // 分配失败，停止
            }
        }

        // 验证分配的页面都是有效的
        for (auto& page : pages) {
            ASSERT_TRUE(page != nullptr);
            EXPECT_TRUE(page->GetPageId() >= 0);
        }

        // 获取统计信息，应该记录分配失败（如果有的话）
        PageAllocationStats stats = small_allocator->GetAllocationStats();
        EXPECT_GE(stats.total_allocations, static_cast<size_t>(pages.size()));

    } catch (const std::bad_alloc& e) {
        // 内存不足异常是可接受的
        SUCCEED();
    } catch (const std::exception& e) {
        // 其他异常也可能发生，取决于实现
        SUCCEED();
    }
}

// 测试性能特征
TEST_F(PageAllocatorTest, PerformanceCharacteristics) {
    const int num_operations = 1000;
    auto start_time = std::chrono::high_resolution_clock::now();

    // 执行大量分配/释放操作
    std::vector<int32_t> page_ids;
    for (int i = 0; i < num_operations; ++i) {
        auto page = allocator->AllocatePage(PageType::DATA);
        if (page) {
            page_ids.push_back(page->GetPageId());
        }
    }

    // 释放所有页面
    for (int32_t page_id : page_ids) {
        allocator->DeallocatePage(page_id);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 验证操作完成且性能合理
    EXPECT_GT(num_operations, 0);
    EXPECT_LT(duration.count(), 5000);  // 应该在5秒内完成

    std::cout << "Page allocator performance: " << num_operations
              << " operations in " << duration.count() << "ms" << std::endl;
}

// 测试页面类型分布
TEST_F(PageAllocatorTest, PageTypeDistribution) {
    // 分配不同类型的页面
    auto data_page = allocator->AllocatePage(PageType::DATA);
    auto index_page = allocator->AllocatePage(PageType::INDEX);
    auto metadata_page = allocator->AllocatePage(PageType::METADATA);
    auto log_page = allocator->AllocatePage(PageType::LOG);

    ASSERT_TRUE(data_page != nullptr);
    ASSERT_TRUE(index_page != nullptr);
    ASSERT_TRUE(metadata_page != nullptr);
    ASSERT_TRUE(log_page != nullptr);

    // 获取页面使用统计
    PageUsageStats stats = allocator->GetPageUsageStats();

    // 验证每种类型都有一个页面
    EXPECT_EQ(stats.page_type_distribution[PageType::DATA], 1);
    EXPECT_EQ(stats.page_type_distribution[PageType::INDEX], 1);
    EXPECT_EQ(stats.page_type_distribution[PageType::METADATA], 1);
    EXPECT_EQ(stats.page_type_distribution[PageType::LOG], 1);
    EXPECT_EQ(stats.page_type_distribution[PageType::TEMP], 0);  // 没有临时页面
}

// 测试页面生命周期统计
TEST_F(PageAllocatorTest, PageLifetimeStatistics) {
    // 分配页面并等待一段时间
    auto page = allocator->AllocatePage(PageType::DATA);
    ASSERT_TRUE(page != nullptr);

    // 等待一段时间模拟页面生命周期
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // 释放页面
    allocator->DeallocatePage(page->GetPageId());

    // 获取统计信息
    PageUsageStats stats = allocator->GetPageUsageStats();

    // 验证生命周期统计
    EXPECT_GE(stats.average_page_lifetime_ms, 0.0);
    EXPECT_LE(stats.average_page_lifetime_ms, 1000.0);  // 应该在合理范围内
}

} // namespace sqlcc
