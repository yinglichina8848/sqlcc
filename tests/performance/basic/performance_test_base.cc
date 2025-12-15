#include "../performance_test_base.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <iomanip>
#include <sstream>

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
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }
        
        file << "Performance Test Results\n";
        file << "========================\n\n";
        
        for (const auto& result : results) {
            file << "Test: " << result.test_name << "\n";
            file << "Duration: " << FormatTime(result.duration) << "\n";
            file << "Operations: " << result.operations_completed << "\n";
            file << "Throughput: " << FormatThroughput(result.throughput) << "\n";
            file << "Average Latency: " << FormatLatency(result.avg_latency) << "\n";
            file << "P95 Latency: " << FormatLatency(result.p95_latency) << "\n";
            file << "P99 Latency: " << FormatLatency(result.p99_latency) << "\n";
            
            if (!result.custom_metrics.empty()) {
                file << "Custom Metrics:\n";
                for (const auto& [key, value] : result.custom_metrics) {
                    file << "  " << key << ": " << value << "\n";
                }
            }
            file << "\n";
        }
        
        file.close();
        std::cout << "Results saved to: " << filename << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error saving results to file: " << e.what() << std::endl;
    }
}

void PerformanceTestBase::GenerateReport(const std::vector<PerformanceTestBase::TestResult>& results) const {
    if (results.empty()) {
        return; // 静默返回，不输出任何信息
    }
    
    // 生成文本报告到文件
    std::ofstream text_file("crud_performance_report.txt");
    if (text_file.is_open()) {
        text_file << "=== 性能测试报告 ===" << std::endl;
        text_file << "生成时间: " << GetCurrentTime() << std::endl;
        text_file << std::endl;
        
        // 打印汇总信息
        text_file << "汇总信息:" << std::endl;
        text_file << "测试总数: " << results.size() << std::endl;
        
        double total_duration = 0.0;
        size_t total_operations = 0;
        
        for (const auto& result : results) {
            total_duration += result.duration.count();
            total_operations += result.operations_completed;
        }
        
        text_file << "总持续时间: " << FormatTime(std::chrono::milliseconds(static_cast<long long>(total_duration))) << std::endl;
        text_file << "总操作数: " << total_operations << std::endl;
        text_file << "平均吞吐量: " << FormatThroughput(total_operations / (total_duration / 1000.0)) << std::endl;
        text_file << std::endl;
        
        // 打印详细结果
        text_file << "详细结果:" << std::endl;
        for (const auto& result : results) {
            text_file << "测试规模: " << result.test_scale << std::endl;
            text_file << "测试名称: " << result.test_name << std::endl;
            text_file << "持续时间: " << FormatTime(result.duration) << std::endl;
            text_file << "操作数: " << result.operations_completed << std::endl;
            text_file << "吞吐量: " << FormatThroughput(result.throughput) << std::endl;
            text_file << "平均延迟: " << FormatLatency(result.avg_latency) << std::endl;
            text_file << "P95延迟: " << FormatLatency(result.p95_latency) << std::endl;
            text_file << "P99延迟: " << FormatLatency(result.p99_latency) << std::endl;
            text_file << "测试时间: " << result.test_time << std::endl;
            text_file << std::endl;
        }
        
        text_file.close();
    }
    
    // 生成CSV报告
    GenerateCSVReport(results, "performance_report.csv");
}

void PerformanceTestBase::GenerateCSVReport(const std::vector<TestResult>& results, 
                                           const std::string& filename) const {
    try {
        std::ofstream csv_file(filename);
        if (!csv_file.is_open()) {
            std::cerr << "Failed to create CSV file: " << filename << std::endl;
            return;
        }
        
        // CSV头部
        csv_file << "TestScale,TestName,Duration(ms),Operations,Throughput(ops/sec),AvgLatency(ms),P95Latency(ms),P99Latency(ms),TestTime\n";
        
        // CSV数据行
        for (const auto& result : results) {
            csv_file << result.test_scale << ","
                     << result.test_name << ","
                     << result.duration.count() << ","
                     << result.operations_completed << ","
                     << std::fixed << std::setprecision(2) << result.throughput << ","
                     << std::fixed << std::setprecision(3) << result.avg_latency << ","
                     << std::fixed << std::setprecision(3) << result.p95_latency << ","
                     << std::fixed << std::setprecision(3) << result.p99_latency << ","
                     << result.test_time << "\n";
        }
        
        csv_file.close();
    } catch (const std::exception& e) {
        std::cerr << "Error generating CSV report: " << e.what() << std::endl;
    }
}

std::string PerformanceTestBase::FormatTime(std::chrono::milliseconds duration) const {
    if (duration.count() < 1000) {
        return std::to_string(duration.count()) + " ms";
    } else if (duration.count() < 60000) {
        return std::to_string(duration.count() / 1000.0) + " s";
    } else {
        return std::to_string(duration.count() / 60000.0) + " min";
    }
}

std::string PerformanceTestBase::FormatThroughput(double throughput) const {
    if (throughput >= 1000) {
        return std::to_string(throughput / 1000.0) + " Kops/sec";
    } else {
        return std::to_string(throughput) + " ops/sec";
    }
}

std::string PerformanceTestBase::FormatLatency(double latency) const {
    if (latency < 1.0) {
        return std::to_string(latency * 1000.0) + " us";
    } else {
        return std::to_string(latency) + " ms";
    }
}

}  // namespace test
}  // namespace sqlcc