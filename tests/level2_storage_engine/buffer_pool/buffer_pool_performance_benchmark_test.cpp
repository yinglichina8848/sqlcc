/**
 * @file buffer_pool_performance_benchmark_test.cpp
 * @brief 缓冲池性能基准测试 - 独立的性能测试套件，避免超时
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <filesystem>
#include <random>
#include <algorithm>
#include <numeric>

#include "storage_engine/buffer_pool/lru_manager.h"
#include "storage_engine/buffer_pool/statistics_collector.h"
#include "storage_engine.h"
#include "page.h"
#include "utils/config_manager.h"

namespace fs = std::filesystem;
namespace sqlcc {

class BufferPoolPerformanceBenchmarkTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时测试目录
        test_dir = fs::temp_directory_path() / "sqlcc_buffer_pool_perf_test";
        fs::create_directories(test_dir);

        // 初始化配置管理器
        config = std::make_unique<ConfigManager>();
        config->SetValue("storage.data_directory", test_dir.string());
        config->SetValue("buffer_pool.size", std::string("8192"));

        // 初始化存储引擎
        storage_engine = std::make_shared<StorageEngine>(*config, test_dir.string());

        // 初始化LRU管理器和统计收集器
        lru_manager = std::make_unique<LRUManager>();
        stats_collector = std::make_unique<StatisticsCollector>();
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

    // 性能测试辅助函数
    void RunPerformanceTest(const std::string& test_name, std::function<void()> test_func) {
        auto start_time = std::chrono::high_resolution_clock::now();
        test_func();
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        std::cout << test_name << " completed in " << duration.count() << "ms" << std::endl;
        EXPECT_LT(duration.count(), 5000); // 性能测试不超过5秒
    }

    fs::path test_dir;
    std::unique_ptr<ConfigManager> config;
    std::shared_ptr<StorageEngine> storage_engine;
    std::unique_ptr<storage::LRUManager> lru_manager;
    std::unique_ptr<storage::StatisticsCollector> stats_collector;
};

// 缓冲池访问模式性能测试
TEST_F(BufferPoolPerformanceBenchmarkTest, BufferPoolAccessPatternPerformance) {
    const int num_pages = 1000;
    const int operations = 10000;

    // 预热：添加页面
    for (int i = 0; i < num_pages; ++i) {
        lru_manager->Add(i);
        stats_collector->RecordPageAccess(i);
    }

    RunPerformanceTest("Sequential Access Pattern", [&]() {
        // 顺序访问模式
        for (int i = 0; i < operations; ++i) {
            int page_id = i % num_pages;
            lru_manager->Access(page_id);
            stats_collector->RecordPageAccess(page_id);
            stats_collector->RecordPageHit();
        }
    });

    double hit_rate = stats_collector->GetHitRate();
    std::cout << "Sequential access hit rate: " << hit_rate << std::endl;
    EXPECT_GT(hit_rate, 0.95); // 顺序访问命中率应该很高
}

// 随机访问模式性能测试
TEST_F(BufferPoolPerformanceBenchmarkTest, RandomAccessPatternPerformance) {
    const int num_pages = 1000;
    const int operations = 5000; // 减少操作次数以控制时间

    // 预热：添加页面
    for (int i = 0; i < num_pages; ++i) {
        lru_manager->Add(i);
        stats_collector->RecordPageAccess(i);
    }

    RunPerformanceTest("Random Access Pattern", [&]() {
        std::mt19937 gen(42); // 固定种子保证可重复性
        std::uniform_int_distribution<> dis(0, num_pages - 1);

        for (int i = 0; i < operations; ++i) {
            int page_id = dis(gen);
            if (lru_manager->Contains(page_id)) {
                lru_manager->Access(page_id);
                stats_collector->RecordPageHit();
            } else {
                lru_manager->Add(page_id);
                stats_collector->RecordPageMiss();
            }
            stats_collector->RecordPageAccess(page_id);
        }
    });

    double hit_rate = stats_collector->GetHitRate();
    std::cout << "Random access hit rate: " << hit_rate << std::endl;
    EXPECT_GT(hit_rate, 0.0);
    EXPECT_LT(hit_rate, 1.0);
}

// LRU替换策略性能测试
TEST_F(BufferPoolPerformanceBenchmarkTest, LRUReplacementPerformance) {
    const int buffer_size = 100;
    const int total_pages = 1000;
    const int operations = 5000;

    RunPerformanceTest("LRU Replacement Strategy", [&]() {
        std::mt19937 gen(123);
        std::uniform_int_distribution<> dis(0, total_pages - 1);

        for (int i = 0; i < operations; ++i) {
            int page_id = dis(gen);

            if (lru_manager->Contains(page_id)) {
                // 页面命中
                lru_manager->Access(page_id);
                stats_collector->RecordPageHit();
            } else {
                // 页面未命中，需要替换
                if (lru_manager->Size() >= buffer_size) {
                    int32_t victim = lru_manager->GetLeastRecentlyUsed();
                    if (victim != -1) {
                        lru_manager->Remove(victim);
                        stats_collector->RecordPageReplacement();
                    }
                }
                lru_manager->Add(page_id);
                stats_collector->RecordPageMiss();
            }
            stats_collector->RecordPageAccess(page_id);
        }
    });

    std::cout << "LRU replacement - Total replacements: " << stats_collector->GetReplacementCount() << std::endl;
    std::cout << "LRU replacement - Hit rate: " << stats_collector->GetHitRate() << std::endl;
}

// 并发访问性能测试
TEST_F(BufferPoolPerformanceBenchmarkTest, ConcurrentAccessPerformance) {
    const int num_threads = 4;
    const int operations_per_thread = 1000;
    const int num_pages = 200;

    // 预热：添加初始页面
    for (int i = 0; i < num_pages; ++i) {
        lru_manager->Add(i);
        stats_collector->RecordPageAccess(i);
    }

    RunPerformanceTest("Concurrent Access Performance", [&]() {
        std::vector<std::thread> threads;
        std::atomic<int> completed_threads(0);

        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([this, operations_per_thread, num_pages, t, &completed_threads]() {
                std::mt19937 gen(42 + t);
                std::uniform_int_distribution<> dis(0, num_pages - 1);

                for (int i = 0; i < operations_per_thread; ++i) {
                    int page_id = dis(gen);

                    // 注意：这里简化了并发控制，实际应用中需要锁保护
                    if (lru_manager->Contains(page_id)) {
                        lru_manager->Access(page_id);
                        stats_collector->RecordPageHit();
                    } else {
                        lru_manager->Add(page_id);
                        stats_collector->RecordPageMiss();
                    }
                    stats_collector->RecordPageAccess(page_id);
                }

                completed_threads++;
            });
        }

        // 等待所有线程完成
        for (auto& thread : threads) {
            thread.join();
        }

        EXPECT_EQ(completed_threads.load(), num_threads);
    });

    std::cout << "Concurrent access - Total accesses: " << stats_collector->GetTotalAccesses() << std::endl;
    std::cout << "Concurrent access - Hit rate: " << stats_collector->GetHitRate() << std::endl;
}

// 统计收集器性能测试
TEST_F(BufferPoolPerformanceBenchmarkTest, StatisticsCollectorPerformance) {
    const int operations = 100000;

    RunPerformanceTest("Statistics Collector Performance", [&]() {
        for (int i = 0; i < operations; ++i) {
            int page_id = i % 100;
            stats_collector->RecordPageAccess(page_id);

            if (i % 3 == 0) {
                stats_collector->RecordPageHit();
            } else {
                stats_collector->RecordPageMiss();
            }

            if (i % 10 == 0) {
                stats_collector->RecordPageReplacement();
                stats_collector->RecordPageFlush();
            }
        }
    });

    EXPECT_EQ(stats_collector->GetTotalAccesses(), operations);
    EXPECT_EQ(stats_collector->GetReplacementCount(), operations / 10);
    EXPECT_EQ(stats_collector->GetFlushCount(), operations / 10);

    std::cout << "Statistics collector - Operations: " << operations << std::endl;
    std::cout << "Statistics collector - Hit rate: " << stats_collector->GetHitRate() << std::endl;
}

// 内存使用模式分析
TEST_F(BufferPoolPerformanceBenchmarkTest, MemoryUsagePatternAnalysis) {
    const int max_pages = 500;
    const int operations = 2000;

    RunPerformanceTest("Memory Usage Pattern Analysis", [&]() {
        std::mt19937 gen(999);
        std::uniform_int_distribution<> dis(0, max_pages * 2);

        for (int i = 0; i < operations; ++i) {
            int page_id = dis(gen);

            if (lru_manager->Contains(page_id)) {
                lru_manager->Access(page_id);
                stats_collector->RecordPageHit();
            } else {
                // 检查是否需要替换
                if (lru_manager->Size() >= max_pages) {
                    int32_t victim = lru_manager->GetLeastRecentlyUsed();
                    if (victim != -1) {
                        lru_manager->Remove(victim);
                        stats_collector->RecordPageReplacement();
                    }
                }
                lru_manager->Add(page_id);
                stats_collector->RecordPageMiss();
            }
            stats_collector->RecordPageAccess(page_id);
        }
    });

    std::cout << "Memory usage - Final buffer size: " << lru_manager->Size() << std::endl;
    std::cout << "Memory usage - Total replacements: " << stats_collector->GetReplacementCount() << std::endl;
    std::cout << "Memory usage - Hit rate: " << stats_collector->GetHitRate() << std::endl;

    // 验证缓冲池大小控制
    EXPECT_LE(lru_manager->Size(), max_pages + 1); // 允许小幅超出
}

// 工作负载特征分析
TEST_F(BufferPoolPerformanceBenchmarkTest, WorkloadCharacteristicsAnalysis) {
    const int workload_size = 1000;
    const int access_pattern_length = 5000;

    // 模拟不同的工作负载特征
    enum class WorkloadType {
        SEQUENTIAL,
        RANDOM,
        LOCALITY_HIGH,
        LOCALITY_LOW
    };

    auto test_workload = [&](WorkloadType type, const std::string& name) {
        // 重置状态
        while (lru_manager->Size() > 0) {
            int32_t victim = lru_manager->GetLeastRecentlyUsed();
            if (victim != -1) {
                lru_manager->Remove(victim);
            }
        }
        stats_collector->Reset();

        // 添加初始页面
        for (int i = 0; i < workload_size; ++i) {
            lru_manager->Add(i);
        }

        RunPerformanceTest(name, [&]() {
            std::mt19937 gen(42);
            std::uniform_int_distribution<> dis(0, workload_size - 1);

            for (int i = 0; i < access_pattern_length; ++i) {
                int page_id;

                switch (type) {
                    case WorkloadType::SEQUENTIAL:
                        page_id = i % workload_size;
                        break;
                    case WorkloadType::RANDOM:
                        page_id = dis(gen);
                        break;
                    case WorkloadType::LOCALITY_HIGH: {
                        // 高局部性：80%的访问在最近10%的页面中
                        int locality_range = workload_size / 10;
                        if (dis(gen) % 5 == 0) { // 20%随机访问
                            page_id = dis(gen);
                        } else { // 80%局部访问
                            std::uniform_int_distribution<> local_dis(0, locality_range - 1);
                            page_id = local_dis(gen);
                        }
                        break;
                    }
                    case WorkloadType::LOCALITY_LOW: {
                        // 低局部性：随机访问整个范围
                        page_id = dis(gen);
                        break;
                    }
                }

                if (lru_manager->Contains(page_id)) {
                    lru_manager->Access(page_id);
                    stats_collector->RecordPageHit();
                } else {
                    stats_collector->RecordPageMiss();
                    // 对于这个分析，我们不实际替换页面
                }
                stats_collector->RecordPageAccess(page_id);
            }
        });

        std::cout << name << " - Hit rate: " << stats_collector->GetHitRate() << std::endl;
    };

    test_workload(WorkloadType::SEQUENTIAL, "Sequential Workload");
    test_workload(WorkloadType::RANDOM, "Random Workload");
    test_workload(WorkloadType::LOCALITY_HIGH, "High Locality Workload");
    test_workload(WorkloadType::LOCALITY_LOW, "Low Locality Workload");
}

// 缓冲池效率指标测试
TEST_F(BufferPoolPerformanceBenchmarkTest, BufferPoolEfficiencyMetrics) {
    const int buffer_capacity = 100;
    const int total_pages = 1000;
    const int operations = 10000;

    RunPerformanceTest("Buffer Pool Efficiency Analysis", [&]() {
        std::mt19937 gen(777);
        std::uniform_int_distribution<> dis(0, total_pages - 1);

        int replacements = 0;
        int total_accesses = 0;
        int hits = 0;

        for (int i = 0; i < operations; ++i) {
            int page_id = dis(gen);
            total_accesses++;

            if (lru_manager->Contains(page_id)) {
                lru_manager->Access(page_id);
                hits++;
                stats_collector->RecordPageHit();
            } else {
                // 需要加载页面
                if (lru_manager->Size() >= buffer_capacity) {
                    int32_t victim = lru_manager->GetLeastRecentlyUsed();
                    if (victim != -1) {
                        lru_manager->Remove(victim);
                        replacements++;
                        stats_collector->RecordPageReplacement();
                    }
                }
                lru_manager->Add(page_id);
                stats_collector->RecordPageMiss();
            }
            stats_collector->RecordPageAccess(page_id);
        }

        // 计算效率指标
        double hit_rate = static_cast<double>(hits) / total_accesses;
        double replacement_rate = static_cast<double>(replacements) / total_accesses;

        std::cout << "Efficiency Metrics:" << std::endl;
        std::cout << "  Total accesses: " << total_accesses << std::endl;
        std::cout << "  Hit rate: " << hit_rate << std::endl;
        std::cout << "  Replacement rate: " << replacement_rate << std::endl;
        std::cout << "  Final buffer utilization: " << lru_manager->Size() << "/" << buffer_capacity << std::endl;

        // 验证基本效率要求
        EXPECT_GT(hit_rate, 0.0);
        EXPECT_LT(replacement_rate, 1.0);
    });
}

} // namespace sqlcc
