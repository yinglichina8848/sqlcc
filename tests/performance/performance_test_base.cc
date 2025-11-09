#include "performance_test_base.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <set>

namespace sqlcc {
namespace test {

std::chrono::high_resolution_clock::time_point PerformanceTestBase::GetCurrentTime() const {
    return std::chrono::high_resolution_clock::now();
}

std::chrono::milliseconds PerformanceTestBase::CalculateDuration(
    const std::chrono::high_resolution_clock::time_point& start,
    const std::chrono::high_resolution_clock::time_point& end) const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
}

double PerformanceTestBase::CalculateThroughput(size_t operations, std::chrono::milliseconds duration) const {
    if (duration.count() == 0) {
        return 0.0;
    }
    return static_cast<double>(operations) / (static_cast<double>(duration.count()) / 1000.0);
}

void PerformanceTestBase::CalculateLatencies(const std::vector<double>& latencies, 
                                            double& avg, double& p95, double& p99) const {
    if (latencies.empty()) {
        avg = p95 = p99 = 0.0;
        return;
    }

    // 计算平均延迟
    double sum = 0.0;
    for (double latency : latencies) {
        sum += latency;
    }
    avg = sum / latencies.size();

    // 计算P95和P99延迟
    std::vector<double> sorted_latencies = latencies;
    std::sort(sorted_latencies.begin(), sorted_latencies.end());

    size_t p95_index = static_cast<size_t>(sorted_latencies.size() * 0.95);
    size_t p99_index = static_cast<size_t>(sorted_latencies.size() * 0.99);

    // 确保索引在有效范围内
    p95_index = std::min(p95_index, sorted_latencies.size() - 1);
    p99_index = std::min(p99_index, sorted_latencies.size() - 1);

    p95 = sorted_latencies[p95_index];
    p99 = sorted_latencies[p99_index];
}

void PerformanceTestBase::PrintResult(const TestResult& result) const {
    std::cout << "\n===== " << result.test_name << " =====" << std::endl;
    std::cout << "Duration: " << FormatTime(result.duration) << std::endl;
    std::cout << "Operations: " << result.operations_completed << std::endl;
    std::cout << "Throughput: " << FormatThroughput(result.throughput) << std::endl;
    std::cout << "Avg Latency: " << FormatLatency(result.avg_latency) << std::endl;
    std::cout << "P95 Latency: " << FormatLatency(result.p95_latency) << std::endl;
    std::cout << "P99 Latency: " << FormatLatency(result.p99_latency) << std::endl;

    if (!result.custom_metrics.empty()) {
        std::cout << "Custom Metrics:" << std::endl;
        for (const auto& metric : result.custom_metrics) {
            std::cout << "  " << metric.first << ": " << metric.second << std::endl;
        }
    }
    std::cout << "=====================================" << std::endl;
}

void PerformanceTestBase::SaveResultsToFile(const std::vector<TestResult>& results, 
                                           const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    // 写入CSV头部
    file << "Test Name,Duration(ms),Operations,Throughput(ops/sec),"
         << "Avg Latency(ms),P95 Latency(ms),P99 Latency(ms)";
    
    // 添加自定义指标列（如果有）
    std::set<std::string> custom_metric_names;
    for (const auto& result : results) {
        for (const auto& metric : result.custom_metrics) {
            custom_metric_names.insert(metric.first);
        }
    }
    
    for (const auto& name : custom_metric_names) {
        file << "," << name;
    }
    
    file << std::endl;

    // 写入数据行
    for (const auto& result : results) {
        file << result.test_name << ","
             << result.duration.count() << ","
             << result.operations_completed << ","
             << result.throughput << ","
             << result.avg_latency << ","
             << result.p95_latency << ","
             << result.p99_latency;
        
        // 添加自定义指标值
        for (const auto& name : custom_metric_names) {
            auto it = result.custom_metrics.find(name);
            if (it != result.custom_metrics.end()) {
                file << "," << it->second;
            } else {
                file << ",";
            }
        }
        
        file << std::endl;
    }

    file.close();
    std::cout << "Results saved to: " << filename << std::endl;
}

void PerformanceTestBase::GenerateReport(const std::vector<TestResult>& results) const {
    std::cout << "\n\n===== PERFORMANCE TEST REPORT =====" << std::endl;
    std::cout << "Total Tests: " << results.size() << std::endl;
    
    // 按测试名称排序
    std::vector<TestResult> sorted_results = results;
    std::sort(sorted_results.begin(), sorted_results.end(), 
              [](const TestResult& a, const TestResult& b) {
                  return a.test_name < b.test_name;
              });
    
    // 打印摘要表格
    std::cout << "\nTest Summary:" << std::endl;
    std::cout << std::left << std::setw(25) << "Test Name" 
              << std::setw(12) << "Duration" 
              << std::setw(12) << "Throughput" 
              << std::setw(12) << "Avg Latency" 
              << std::setw(12) << "P99 Latency" << std::endl;
    std::cout << std::string(73, '-') << std::endl;
    
    for (const auto& result : sorted_results) {
        std::cout << std::left << std::setw(25) << result.test_name
                  << std::setw(12) << FormatTime(result.duration)
                  << std::setw(12) << FormatThroughput(result.throughput)
                  << std::setw(12) << FormatLatency(result.avg_latency)
                  << std::setw(12) << FormatLatency(result.p99_latency) << std::endl;
    }
    
    std::cout << "\n=====================================" << std::endl;
}

std::string PerformanceTestBase::FormatTime(std::chrono::milliseconds duration) const {
    std::ostringstream oss;
    oss << duration.count() << " ms";
    return oss.str();
}

std::string PerformanceTestBase::FormatThroughput(double throughput) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << throughput << " ops/sec";
    return oss.str();
}

std::string PerformanceTestBase::FormatLatency(double latency) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3) << latency << " ms";
    return oss.str();
}

}  // namespace test
}  // namespace sqlcc