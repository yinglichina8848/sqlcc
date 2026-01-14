#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>

#include "storage/performance_monitor.h"

// Test advanced monitoring metrics
class AdvancedPerformanceMonitorTest : public ::testing::Test {
protected:
    void SetUp() override {
        monitor = std::make_unique<PerformanceMonitor>();
        monitor->setEnabled(true);
    }

    void TearDown() override {
        monitor->reset();
    }

    std::unique_ptr<PerformanceMonitor> monitor;
};

// Test Percentile metrics collection
TEST_F(AdvancedPerformanceMonitorTest, PercentileMetrics) {
    const std::string metric_name = "response_time";

    // Record multiple measurements
    std::vector<double> measurements = {1.0, 2.0, 3.0, 4.0, 5.0, 10.0, 15.0, 20.0};
    for (double val : measurements) {
        monitor->recordPercentile(metric_name, val);
    }

    // Verify percentile calculations
    EXPECT_DOUBLE_EQ(monitor->getPercentile(metric_name, 50.0), 3.5);  // Median
    EXPECT_DOUBLE_EQ(monitor->getPercentile(metric_name, 90.0), 15.0); // P90
    EXPECT_DOUBLE_EQ(monitor->getPercentile(metric_name, 95.0), 20.0); // P95
    EXPECT_DOUBLE_EQ(monitor->getPercentile(metric_name, 99.0), 20.0); // P99
}

// Test Distribution metrics
TEST_F(AdvancedPerformanceMonitorTest, DistributionMetrics) {
    const std::string metric_name = "latency_distribution";

    // Record distributed values
    for (int i = 0; i < 100; ++i) {
        monitor->recordDistribution(metric_name, static_cast<double>(i));
    }

    // Verify distribution statistics
    auto stats = monitor->getDistributionStats(metric_name);
    EXPECT_EQ(stats.count, 100);
    EXPECT_DOUBLE_EQ(stats.mean, 49.5);
    EXPECT_DOUBLE_EQ(stats.min, 0.0);
    EXPECT_DOUBLE_EQ(stats.max, 99.0);

    // Verify buckets
    auto histogram = monitor->getHistogram(metric_name);
    EXPECT_FALSE(histogram.empty());
    EXPECT_GT(histogram.size(), 5); // Should have multiple buckets
}

// Test concurrent monitoring (thread safety)
TEST_F(AdvancedPerformanceMonitorTest, ConcurrentMonitoring) {
    const int num_threads = 10;
    const int measurements_per_thread = 100;
    const std::string counter_name = "concurrent_counter";
    const std::string gauge_name = "concurrent_gauge";

    std::vector<std::thread> threads;

    // Launch multiple threads recording metrics concurrently
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, counter_name, gauge_name, measurements_per_thread, i]() {
            for (int j = 0; j < measurements_per_thread; ++j) {
                monitor->incrementCounter(counter_name);
                monitor->recordGauge(gauge_name, static_cast<double>(i * measurements_per_thread + j));
                std::this_thread::sleep_for(std::chrono::microseconds(1)); // Small delay for interleaving
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify concurrent results
    EXPECT_EQ(monitor->getCounter(counter_name), num_threads * measurements_per_thread);

    auto gauge_stats = monitor->getGaugeStats(gauge_name);
    EXPECT_EQ(gauge_stats.count, num_threads * measurements_per_thread);
    EXPECT_GT(gauge_stats.max, gauge_stats.min);
}

// Test monitoring data persistence and retrieval
TEST_F(AdvancedPerformanceMonitorTest, DataPersistence) {
    const std::string metric_name = "persistent_metric";

    // Record some data
    monitor->recordPercentile(metric_name, 1.0);
    monitor->recordPercentile(metric_name, 2.0);
    monitor->recordPercentile(metric_name, 3.0);

    // Verify data is retrievable
    EXPECT_DOUBLE_EQ(monitor->getPercentile(metric_name, 50.0), 2.0);

    // Test data export/import functionality
    auto exported_data = monitor->exportMetrics();
    EXPECT_FALSE(exported_data.empty());

    // Create new monitor and import data
    auto new_monitor = std::make_unique<PerformanceMonitor>();
    new_monitor->importMetrics(exported_data);

    // Verify imported data
    EXPECT_DOUBLE_EQ(new_monitor->getPercentile(metric_name, 50.0), 2.0);
}

// Test monitoring configuration
TEST_F(AdvancedPerformanceMonitorTest, MonitoringConfiguration) {
    // Test enabling/disabling monitoring
    monitor->setEnabled(false);
    monitor->incrementCounter("test_counter");
    EXPECT_EQ(monitor->getCounter("test_counter"), 0); // Should not record when disabled

    monitor->setEnabled(true);
    monitor->incrementCounter("test_counter");
    EXPECT_EQ(monitor->getCounter("test_counter"), 1); // Should record when enabled

    // Test metric limits
    const int max_metrics = 1000;
    for (int i = 0; i < max_metrics + 10; ++i) {
        std::string metric_name = "metric_" + std::to_string(i);
        monitor->incrementCounter(metric_name);
    }

    // Should handle large number of metrics gracefully
    EXPECT_GE(monitor->getActiveMetricsCount(), max_metrics - 1);
}

// Test monitoring performance (ensure monitoring itself doesn't significantly impact performance)
TEST_F(AdvancedPerformanceMonitorTest, MonitoringPerformance) {
    const int iterations = 100000;
    const std::string counter_name = "perf_counter";

    auto start_time = std::chrono::high_resolution_clock::now();

    // Perform many monitoring operations
    for (int i = 0; i < iterations; ++i) {
        monitor->incrementCounter(counter_name);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Monitoring overhead should be reasonable (< 10ms for 100k operations)
    EXPECT_LT(duration.count(), 10);
    EXPECT_EQ(monitor->getCounter(counter_name), iterations);
}

// Test monitoring cleanup and resource management
TEST_F(AdvancedPerformanceMonitorTest, ResourceManagement) {
    const int num_metrics = 100;

    // Create many metrics
    for (int i = 0; i < num_metrics; ++i) {
        std::string metric_name = "resource_metric_" + std::to_string(i);
        monitor->incrementCounter(metric_name);
        monitor->recordGauge(metric_name, static_cast<double>(i));
    }

    // Verify all metrics are recorded
    EXPECT_EQ(monitor->getActiveMetricsCount(), num_metrics * 2); // Counter + Gauge per metric

    // Test cleanup
    monitor->reset();
    EXPECT_EQ(monitor->getActiveMetricsCount(), 0);

    // Verify memory is properly cleaned up
    monitor->incrementCounter("after_reset");
    EXPECT_EQ(monitor->getCounter("after_reset"), 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
