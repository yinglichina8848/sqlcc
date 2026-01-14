#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <functional>

namespace sqlcc {
namespace test {

/**
 * 性能指标定义
 */
enum class PerformanceMetric {
    // 延迟指标
    AVG_LATENCY_MS,      // 平均延迟（毫秒）
    P50_LATENCY_MS,      // P50延迟（毫秒）
    P95_LATENCY_MS,      // P95延迟（毫秒）
    P99_LATENCY_MS,      // P99延迟（毫秒）
    P999_LATENCY_MS,     // P99.9延迟（毫秒）
    MIN_LATENCY_MS,      // 最小延迟（毫秒）
    MAX_LATENCY_MS,      // 最大延迟（毫秒）

    // 吞吐量指标
    THROUGHPUT_OPS_SEC,  // 吞吐量（操作/秒）
    THROUGHPUT_TRANS_SEC,// 吞吐量（事务/秒）

    // 资源使用指标
    CPU_USAGE_PERCENT,   // CPU使用率（百分比）
    MEMORY_USAGE_MB,     // 内存使用量（MB）
    MEMORY_USAGE_PERCENT,// 内存使用率（百分比）
    DISK_IO_MB_SEC,      // 磁盘IO速率（MB/秒）
    NETWORK_IO_MB_SEC,   // 网络IO速率（MB/秒）

    // 并发指标
    CONCURRENT_USERS,    // 并发用户数
    ACTIVE_CONNECTIONS,  // 活跃连接数

    // 错误指标
    ERROR_RATE_PERCENT,  // 错误率（百分比）
    TIMEOUT_RATE_PERCENT,// 超时率（百分比）

    // 自定义指标
    CUSTOM_METRIC
};

/**
 * 性能指标值
 */
struct MetricValue {
    PerformanceMetric metric;
    double value;
    std::string unit;
    std::chrono::system_clock::time_point timestamp;

    MetricValue(PerformanceMetric m, double v, const std::string& u = "")
        : metric(m), value(v), unit(u), timestamp(std::chrono::system_clock::now()) {}
};

/**
 * 性能基准配置
 */
struct PerformanceBenchmark {
    std::string name;                    // 基准测试名称
    std::string description;             // 描述
    std::vector<PerformanceMetric> metrics; // 关注的指标
    std::function<void()> setup_func;   // 设置函数
    std::function<void()> test_func;    // 测试函数
    std::function<void()> cleanup_func; // 清理函数
    std::map<std::string, double> thresholds; // 阈值定义
};

/**
 * 性能测试配置
 */
struct PerformanceTestConfig {
    // 测试规模
    enum class TestScale {
        SMALL,      // 小规模：100-1000操作
        MEDIUM,     // 中规模：1000-10000操作
        LARGE,      // 大规模：10000-100000操作
        XLARGE      // 超大规模：100000+操作
    };

    TestScale scale = TestScale::MEDIUM;
    int iterations = 3;                 // 迭代次数
    int warmup_iterations = 1;          // 预热迭代次数
    std::chrono::seconds timeout = std::chrono::seconds(300); // 超时时间
    bool enable_regression_detection = true; // 启用回归检测
    double regression_threshold = 0.1; // 回归阈值（10%）

    // 资源限制
    size_t max_memory_mb = 1024;        // 最大内存使用（MB）
    double max_cpu_percent = 80.0;      // 最大CPU使用率（%）
};

/**
 * 性能回归检测结果
 */
struct RegressionResult {
    std::string test_name;
    PerformanceMetric metric;
    double baseline_value;
    double current_value;
    double change_percent;
    bool is_regression;
    std::string severity; // "low", "medium", "high", "critical"

    RegressionResult(const std::string& name, PerformanceMetric m,
                    double baseline, double current)
        : test_name(name), metric(m), baseline_value(baseline),
          current_value(current), is_regression(false) {
        change_percent = ((current - baseline) / baseline) * 100.0;

        // 对于延迟指标，正向变化是回归（变慢）
        // 对于吞吐量指标，负向变化是回归（变慢）
        bool is_latency_metric = (m == PerformanceMetric::AVG_LATENCY_MS ||
                                 m == PerformanceMetric::P95_LATENCY_MS ||
                                 m == PerformanceMetric::P99_LATENCY_MS);

        if (is_latency_metric) {
            is_regression = (change_percent > 5.0); // 延迟增加5%以上
            severity = change_percent > 20.0 ? "critical" :
                      change_percent > 10.0 ? "high" : "medium";
        } else {
            is_regression = (change_percent < -5.0); // 吞吐量减少5%以上
            severity = change_percent < -20.0 ? "critical" :
                      change_percent < -10.0 ? "high" : "medium";
        }
    }
};

/**
 * 性能测试结果
 */
struct PerformanceTestResult {
    std::string test_name;
    PerformanceTestConfig::TestScale scale;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    std::vector<MetricValue> metrics;
    std::vector<RegressionResult> regressions;
    bool passed = true;
    std::string error_message;

    // 计算测试持续时间
    std::chrono::milliseconds GetDuration() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    }

    // 获取特定指标的值
    double GetMetricValue(PerformanceMetric metric) const {
        for (const auto& m : metrics) {
            if (m.metric == metric) {
                return m.value;
            }
        }
        return 0.0;
    }
};

/**
 * 性能基准管理器
 */
class PerformanceBenchmarkManager {
public:
    /**
     * 注册基准测试
     */
    void RegisterBenchmark(const PerformanceBenchmark& benchmark);

    /**
     * 运行所有基准测试
     */
    std::vector<PerformanceTestResult> RunAllBenchmarks(const PerformanceTestConfig& config);

    /**
     * 运行单个基准测试
     */
    PerformanceTestResult RunBenchmark(const std::string& name, const PerformanceTestConfig& config);

    /**
     * 加载基准线数据
     */
    bool LoadBaselines(const std::string& filename);

    /**
     * 保存基准线数据
     */
    bool SaveBaselines(const std::string& filename) const;

    /**
     * 生成性能报告
     */
    void GenerateReport(const std::vector<PerformanceTestResult>& results,
                       const std::string& output_dir) const;

    /**
     * 生成摘要报告
     */
    void GenerateSummaryReport(const std::vector<PerformanceTestResult>& results,
                              const std::string& filename) const;

    /**
     * 生成详细报告
     */
    void GenerateDetailedReport(const std::vector<PerformanceTestResult>& results,
                               const std::string& filename) const;

    /**
     * 生成回归报告
     */
    void GenerateRegressionReport(const std::vector<PerformanceTestResult>& results,
                                 const std::string& filename) const;

    /**
     * 生成趋势数据
     */
    void GenerateTrendData(const std::vector<PerformanceTestResult>& results,
                          const std::string& filename) const;

private:
    std::map<std::string, PerformanceBenchmark> benchmarks_;
    std::map<std::string, std::map<PerformanceMetric, double>> baselines_; // 基准线数据

    /**
     * 检测性能回归
     */
    std::vector<RegressionResult> DetectRegressions(
        const std::string& test_name,
        const std::vector<MetricValue>& metrics) const;

    /**
     * 收集系统指标
     */
    std::vector<MetricValue> CollectSystemMetrics() const;

    /**
     * 执行预热
     */
    void Warmup(const PerformanceBenchmark& benchmark, int iterations);
};

/**
 * 性能指标收集器
 */
class PerformanceMetricsCollector {
public:
    /**
     * 开始收集
     */
    void StartCollection();

    /**
     * 停止收集
     */
    void StopCollection();

    /**
     * 记录延迟样本
     */
    void RecordLatency(double latency_ms);

    /**
     * 记录操作完成
     */
    void RecordOperation();

    /**
     * 获取收集的指标
     */
    std::vector<MetricValue> GetMetrics() const;

    /**
     * 重置收集器
     */
    void Reset();

private:
    std::chrono::high_resolution_clock::time_point start_time_;
    bool is_collecting_ = false;
    std::vector<double> latency_samples_;
    size_t operations_completed_ = 0;
};

}  // namespace test
}  // namespace sqlcc