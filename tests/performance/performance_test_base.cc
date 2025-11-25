#include "performance_test_base.h"
#include <iostream>
#include <algorithm>

namespace sqlcc {
namespace test {

// 实现性能测试基类中的方法
std::chrono::high_resolution_clock::time_point PerformanceTestBase::GetCurrentTime() const {
    return std::chrono::high_resolution_clock::now();
}

std::chrono::milliseconds PerformanceTestBase::CalculateDuration(
    const std::chrono::high_resolution_clock::time_point& start,
    const std::chrono::high_resolution_clock::time_point& end) const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
}

double PerformanceTestBase::CalculateThroughput(size_t operations, std::chrono::milliseconds duration) const {
    if (duration.count() == 0) return 0;
    return static_cast<double>(operations) * 1000.0 / duration.count();
}

void PerformanceTestBase::CalculateLatencies(const std::vector<double>& latencies, 
                                           double& avg, double& p95, double& p99) const {
    if (latencies.empty()) {
        avg = p95 = p99 = 0.0;
        return;
    }
    
    // 计算平均值
    double sum = 0.0;
    for (double latency : latencies) {
        sum += latency;
    }
    avg = sum / latencies.size();
    
    // 计算P95和P99分位数
    std::vector<double> sorted_latencies = latencies;
    std::sort(sorted_latencies.begin(), sorted_latencies.end());
    
    size_t p95_index = static_cast<size_t>(latencies.size() * 0.95);
    size_t p99_index = static_cast<size_t>(latencies.size() * 0.99);
    
    p95 = sorted_latencies[p95_index];
    p99 = sorted_latencies[p99_index];
}

void PerformanceTestBase::PrintResult(const PerformanceTestBase::TestResult& result) const {
    std::cout << "\n==== Test Result: " << result.test_name << " ====\n";
    std::cout << "Duration: " << FormatTime(result.duration) << "\n";
    std::cout << "Operations: " << result.operations_completed << "\n";
    std::cout << "Throughput: " << FormatThroughput(result.throughput) << "\n";
    std::cout << "Average Latency: " << FormatLatency(result.avg_latency) << "\n";
    std::cout << "P95 Latency: " << FormatLatency(result.p95_latency) << "\n";
    std::cout << "P99 Latency: " << FormatLatency(result.p99_latency) << "\n";
    
    if (!result.custom_metrics.empty()) {
        std::cout << "\nCustom Metrics:\n";
        for (const auto& [key, value] : result.custom_metrics) {
            std::cout << "  " << key << ": " << value << "\n";
        }
    }
    std::cout << "========================================\n";
}

void PerformanceTestBase::SaveResultsToFile(const std::vector<PerformanceTestBase::TestResult>& results, 
                                          const std::string& filename) const {
    // 简单实现，实际应用中应该使用更健壮的文件IO
    try {
        std::cout << "Saving results to file: " << filename << std::endl;
        // 这里可以添加实际的文件保存逻辑
    } catch (const std::exception& e) {
        std::cerr << "Error saving results to file: " << e.what() << std::endl;
    }
}

void PerformanceTestBase::GenerateReport(const std::vector<PerformanceTestBase::TestResult>& results) const {
    std::cout << "\n======== Performance Test Report ========\n";
    for (const auto& result : results) {
        PrintResult(result);
    }
    std::cout << "\nReport generated.\n";
}

std::string PerformanceTestBase::FormatTime(std::chrono::milliseconds duration) const {
    return std::to_string(duration.count()) + " ms";
}

std::string PerformanceTestBase::FormatThroughput(double throughput) const {
    return std::to_string(throughput) + " ops/sec";
}

std::string PerformanceTestBase::FormatLatency(double latency) const {
    return std::to_string(latency) + " ms";
}

}  // namespace test
}  // namespace sqlcc
