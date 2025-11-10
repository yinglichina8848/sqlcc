#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <map>

namespace sqlcc {
namespace test {

/**
 * 性能测试基类，提供通用的测试工具和指标收集功能
 */
class PerformanceTestBase {
public:
    /**
     * 测试结果结构
     */
    struct TestResult {
        std::string test_name;  // 测试名称
        std::chrono::milliseconds duration;  // 测试持续时间
        size_t operations_completed;  // 完成的操作数
        double throughput;  // 吞吐量 (ops/sec)
        double avg_latency;  // 平均延迟 (ms)
        double p99_latency;  // P99延迟 (ms)
        double p95_latency;  // P95延迟 (ms)
        std::map<std::string, std::string> custom_metrics;  // 自定义指标
    };

    /**
     * 构造函数
     */
    PerformanceTestBase() = default;

    /**
     * 析构函数
     */
    virtual ~PerformanceTestBase() = default;

    /**
     * 运行所有测试
     */
    virtual void RunAllTests() = 0;

protected:
    /**
     * 获取当前时间点
     */
    std::chrono::high_resolution_clock::time_point GetCurrentTime() const;

    /**
     * 计算持续时间（毫秒）
     */
    std::chrono::milliseconds CalculateDuration(
        const std::chrono::high_resolution_clock::time_point& start,
        const std::chrono::high_resolution_clock::time_point& end) const;

    /**
     * 计算吞吐量
     */
    double CalculateThroughput(size_t operations, std::chrono::milliseconds duration) const;

    /**
     * 计算延迟统计信息
     */
    void CalculateLatencies(const std::vector<double>& latencies, 
                           double& avg, double& p95, double& p99) const;

    /**
     * 打印测试结果
     */
    void PrintResult(const TestResult& result) const;

    /**
     * 将结果保存到文件
     */
    void SaveResultsToFile(const std::vector<TestResult>& results, 
                          const std::string& filename) const;

public:
    /**
     * 设置输出目录
     */
    void SetOutputDirectory(const std::string& directory) { output_directory_ = directory; }

protected:

    /**
     * 生成测试报告
     */
    void GenerateReport(const std::vector<TestResult>& results) const;

    /**
     * 清理测试环境
     */
    virtual void Cleanup() = 0;

private:
    /**
     * 输出目录
     */
    std::string output_directory_ = ".";

    /**
     * 格式化时间
     */
    std::string FormatTime(std::chrono::milliseconds duration) const;

    /**
     * 格式化吞吐量
     */
    std::string FormatThroughput(double throughput) const;

    /**
     * 格式化延迟
     */
    std::string FormatLatency(double latency) const;
};

}  // namespace test
}  // namespace sqlcc