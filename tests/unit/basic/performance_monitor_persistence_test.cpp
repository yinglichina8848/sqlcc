/**
 * @file performance_monitor_persistence_test.cpp
 * @brief 性能监控持久化测试 - 测试监控数据的持久化和历史趋势分析
 */

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "storage/performance_monitor.h"

namespace fs = std::filesystem;
namespace sqlcc {

class PerformanceMonitorPersistenceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建临时测试目录
        test_dir = fs::temp_directory_path() / "sqlcc_perf_monitor_test";
        fs::create_directories(test_dir);

        monitor = std::make_unique<PerformanceMonitor>();
        monitor->setEnabled(true);
    }

    void TearDown() override {
        monitor.reset();

        // 清理测试目录
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }

    fs::path test_dir;
    std::unique_ptr<PerformanceMonitor> monitor;
};

// 测试监控数据序列化
TEST_F(PerformanceMonitorPersistenceTest, DataSerialization) {
    // 记录一些监控数据
    monitor->incrementCounter("requests_total");
    monitor->recordGauge("active_connections", 5.0);
    monitor->recordPercentile("response_time", 100.0);
    monitor->recordPercentile("response_time", 200.0);
    monitor->recordPercentile("response_time", 300.0);
    monitor->recordDistribution("memory_usage", 1024.0);
    monitor->recordDistribution("memory_usage", 2048.0);

    // 导出监控数据
    auto exported_data = monitor->exportMetrics();
    EXPECT_FALSE(exported_data.empty());

    // 验证导出数据包含必要信息
    EXPECT_NE(exported_data.find("requests_total"), std::string::npos);
    EXPECT_NE(exported_data.find("active_connections"), std::string::npos);
    EXPECT_NE(exported_data.find("response_time"), std::string::npos);
    EXPECT_NE(exported_data.find("memory_usage"), std::string::npos);
}

// 测试监控数据反序列化
TEST_F(PerformanceMonitorPersistenceTest, DataDeserialization) {
    // 创建第一个监控实例并记录数据
    auto monitor1 = std::make_unique<PerformanceMonitor>();
    monitor1->incrementCounter("test_counter", 5);
    monitor1->recordGauge("test_gauge", 42.0);
    monitor1->recordPercentile("test_percentile", 150.0);

    // 导出数据
    auto exported_data = monitor1->exportMetrics();

    // 创建第二个监控实例并导入数据
    auto monitor2 = std::make_unique<PerformanceMonitor>();
    monitor2->importMetrics(exported_data);

    // 验证数据正确导入
    EXPECT_EQ(monitor2->getCounter("test_counter"), 5);
    EXPECT_DOUBLE_EQ(monitor2->getGauge("test_gauge"), 42.0);
    EXPECT_DOUBLE_EQ(monitor2->getPercentile("test_percentile", 50.0), 150.0);
}

// 测试监控数据文件持久化
TEST_F(PerformanceMonitorPersistenceTest, FilePersistence) {
    // 记录监控数据
    monitor->incrementCounter("file_test_counter", 10);
    monitor->recordGauge("file_test_gauge", 25.5);
    monitor->recordPercentile("file_test_percentile", 75.0);

    // 导出到文件
    fs::path data_file = test_dir / "metrics_data.json";
    auto exported_data = monitor->exportMetrics();

    std::ofstream out_file(data_file);
    out_file << exported_data;
    out_file.close();

    // 验证文件创建成功
    EXPECT_TRUE(fs::exists(data_file));

    // 从文件读取数据
    std::ifstream in_file(data_file);
    std::stringstream buffer;
    buffer << in_file.rdbuf();
    std::string file_content = buffer.str();
    in_file.close();

    // 创建新监控实例并导入文件数据
    auto monitor_from_file = std::make_unique<PerformanceMonitor>();
    monitor_from_file->importMetrics(file_content);

    // 验证数据正确恢复
    EXPECT_EQ(monitor_from_file->getCounter("file_test_counter"), 10);
    EXPECT_DOUBLE_EQ(monitor_from_file->getGauge("file_test_gauge"), 25.5);
    EXPECT_DOUBLE_EQ(monitor_from_file->getPercentile("file_test_percentile", 50.0), 75.0);
}

// 测试历史趋势分析
TEST_F(PerformanceMonitorPersistenceTest, HistoricalTrendAnalysis) {
    // 模拟多个时间点的监控数据
    std::vector<std::string> snapshots;

    // 时间点1: 初始状态
    monitor->incrementCounter("total_requests", 100);
    monitor->recordGauge("cpu_usage", 45.0);
    snapshots.push_back(monitor->exportMetrics());

    // 时间点2: 中间状态
    monitor->incrementCounter("total_requests", 150); // 新增50个请求
    monitor->recordGauge("cpu_usage", 65.0);
    snapshots.push_back(monitor->exportMetrics());

    // 时间点3: 最终状态
    monitor->incrementCounter("total_requests", 200); // 再新增50个请求
    monitor->recordGauge("cpu_usage", 55.0);
    snapshots.push_back(monitor->exportMetrics());

    // 分析趋势
    auto trend_analysis = monitor->analyzeHistoricalTrends(snapshots);

    // 验证趋势分析结果
    EXPECT_FALSE(trend_analysis.empty());
    EXPECT_NE(trend_analysis.find("total_requests"), std::string::npos);
    EXPECT_NE(trend_analysis.find("cpu_usage"), std::string::npos);

    // 验证请求数量趋势 (应该递增)
    EXPECT_TRUE(trend_analysis.find("increasing") != std::string::npos ||
                trend_analysis.find("stable") != std::string::npos);
}

// 测试监控数据聚合
TEST_F(PerformanceMonitorPersistenceTest, DataAggregation) {
    // 记录多个时间段的数据
    monitor->incrementCounter("hourly_requests", 100);
    monitor->recordGauge("avg_response_time", 50.0);

    // 模拟数据聚合
    auto aggregated_data = monitor->aggregateMetricsByTimeWindow();

    // 验证聚合结果
    EXPECT_FALSE(aggregated_data.empty());

    // 检查聚合后的统计信息
    if (!aggregated_data.empty()) {
        // 应该包含汇总统计
        EXPECT_NE(aggregated_data.find("total"), std::string::npos);
        EXPECT_NE(aggregated_data.find("average"), std::string::npos);
    }
}

// 测试监控数据压缩和优化存储
TEST_F(PerformanceMonitorPersistenceTest, DataCompression) {
    // 生成大量监控数据
    for (int i = 0; i < 1000; ++i) {
        std::string metric_name = "metric_" + std::to_string(i % 10);
        monitor->incrementCounter(metric_name);
        monitor->recordGauge(metric_name + "_gauge", static_cast<double>(i));
        monitor->recordPercentile(metric_name + "_percentile", static_cast<double>(i % 100));
    }

    // 导出原始数据
    auto original_data = monitor->exportMetrics();
    size_t original_size = original_data.size();

    // 应用数据压缩（如果支持）
    auto compressed_data = monitor->compressMetricsData(original_data);
    size_t compressed_size = compressed_data.size();

    // 验证压缩效果（压缩数据应该更小或相等）
    EXPECT_LE(compressed_size, original_size);

    // 验证解压后数据正确性
    auto decompressed_data = monitor->decompressMetricsData(compressed_data);
    EXPECT_EQ(decompressed_data, original_data);

    // 从解压数据创建新监控实例验证
    auto monitor_from_decompressed = std::make_unique<PerformanceMonitor>();
    monitor_from_decompressed->importMetrics(decompressed_data);

    // 验证关键指标
    EXPECT_EQ(monitor_from_decompressed->getCounter("metric_0"), 100); // 每个metric被记录100次
}

// 测试监控数据版本兼容性
TEST_F(PerformanceMonitorPersistenceTest, VersionCompatibility) {
    // 模拟旧版本数据格式
    std::string old_version_data = R"(
    {
        "version": "1.0",
        "counters": {
            "legacy_counter": 42
        },
        "gauges": {
            "legacy_gauge": 3.14
        }
    })";

    // 尝试导入旧版本数据
    EXPECT_NO_THROW(monitor->importMetrics(old_version_data));

    // 验证旧数据正确迁移
    EXPECT_EQ(monitor->getCounter("legacy_counter"), 42);
    EXPECT_DOUBLE_EQ(monitor->getGauge("legacy_gauge"), 3.14);

    // 导出新版本数据
    auto new_version_data = monitor->exportMetrics();

    // 验证新版本数据格式
    EXPECT_NE(new_version_data.find("version"), std::string::npos);
    EXPECT_NE(new_version_data.find("legacy_counter"), std::string::npos);
}

// 测试监控数据清理和归档
TEST_F(PerformanceMonitorPersistenceTest, DataCleanupAndArchiving) {
    // 记录大量历史数据
    for (int i = 0; i < 100; ++i) {
        std::string timestamp = "2025-12-" + std::to_string(i % 31 + 1);
        monitor->recordHistoricalMetric("daily_requests", timestamp, static_cast<double>(i * 10));
    }

    // 执行数据清理（移除旧数据）
    monitor->cleanupOldMetrics(std::chrono::hours(24 * 30)); // 保留30天内的数据

    // 验证清理后的状态
    auto archived_data = monitor->getArchivedMetrics();
    EXPECT_FALSE(archived_data.empty());

    // 验证归档文件创建
    fs::path archive_file = test_dir / "metrics_archive.json";
    monitor->saveArchivedMetrics(archive_file);

    EXPECT_TRUE(fs::exists(archive_file));

    // 验证归档文件内容
    std::ifstream archive_stream(archive_file);
    std::string archive_content((std::istreambuf_iterator<char>(archive_stream)),
                               std::istreambuf_iterator<char>());

    EXPECT_FALSE(archive_content.empty());
    EXPECT_NE(archive_content.find("daily_requests"), std::string::npos);
}

// 测试监控数据导出格式 (JSON, CSV, XML)
TEST_F(PerformanceMonitorPersistenceTest, ExportFormatSupport) {
    // 记录测试数据
    monitor->incrementCounter("test_counter", 123);
    monitor->recordGauge("test_gauge", 456.789);
    monitor->recordPercentile("test_percentile", 50.0);
    monitor->recordDistribution("test_distribution", 100.0);

    // 测试JSON格式导出
    auto json_data = monitor->exportMetrics("json");
    EXPECT_FALSE(json_data.empty());
    EXPECT_NE(json_data.find("{"), std::string::npos);

    // 测试CSV格式导出
    auto csv_data = monitor->exportMetrics("csv");
    EXPECT_FALSE(csv_data.empty());
    // CSV应该包含逗号分隔的值
    EXPECT_NE(csv_data.find(","), std::string::npos);

    // 测试XML格式导出
    auto xml_data = monitor->exportMetrics("xml");
    EXPECT_FALSE(xml_data.empty());
    EXPECT_NE(xml_data.find("<"), std::string::npos);
    EXPECT_NE(xml_data.find(">"), std::string::npos);

    // 验证所有格式都能被正确导入
    auto monitor_json = std::make_unique<PerformanceMonitor>();
    auto monitor_csv = std::make_unique<PerformanceMonitor>();
    auto monitor_xml = std::make_unique<PerformanceMonitor>();

    EXPECT_NO_THROW(monitor_json->importMetrics(json_data));
    EXPECT_NO_THROW(monitor_csv->importMetrics(csv_data));
    EXPECT_NO_THROW(monitor_xml->importMetrics(xml_data));

    // 验证关键数据在所有格式中保持一致
    EXPECT_EQ(monitor_json->getCounter("test_counter"), 123);
    EXPECT_EQ(monitor_csv->getCounter("test_counter"), 123);
    EXPECT_EQ(monitor_xml->getCounter("test_counter"), 123);
}

// 测试并发数据持久化
TEST_F(PerformanceMonitorPersistenceTest, ConcurrentPersistence) {
    const int num_threads = 5;
    const int operations_per_thread = 100;

    std::vector<std::thread> threads;
    std::atomic<int> completed_threads(0);

    // 启动多个线程并发记录和持久化数据
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, operations_per_thread, t, &completed_threads]() {
            std::string thread_prefix = "thread_" + std::to_string(t) + "_";

            // 每个线程记录自己的数据
            for (int i = 0; i < operations_per_thread; ++i) {
                monitor->incrementCounter(thread_prefix + "counter");
                monitor->recordGauge(thread_prefix + "gauge", static_cast<double>(i));
            }

            // 每个线程独立导出数据
            auto thread_data = monitor->exportMetrics();
            EXPECT_FALSE(thread_data.empty());

            completed_threads++;
        });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(completed_threads.load(), num_threads);

    // 验证最终聚合数据
    for (int t = 0; t < num_threads; ++t) {
        std::string thread_prefix = "thread_" + std::to_string(t) + "_";
        EXPECT_EQ(monitor->getCounter(thread_prefix + "counter"), operations_per_thread);
    }

    // 测试整体数据持久化
    fs::path final_data_file = test_dir / "concurrent_metrics.json";
    auto final_data = monitor->exportMetrics();

    std::ofstream final_file(final_data_file);
    final_file << final_data;
    final_file.close();

    EXPECT_TRUE(fs::exists(final_data_file));
}

} // namespace sqlcc
