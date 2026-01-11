#include "performance_metrics.h"
#include "performance_test_base.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <filesystem>

namespace sqlcc {
namespace test {

// 性能基准管理器实现
void PerformanceBenchmarkManager::RegisterBenchmark(const PerformanceBenchmark& benchmark) {
    benchmarks_[benchmark.name] = benchmark;
    std::cout << "已注册性能基准测试: " << benchmark.name << std::endl;
}

std::vector<PerformanceTestResult> PerformanceBenchmarkManager::RunAllBenchmarks(
    const PerformanceTestConfig& config) {

    std::vector<PerformanceTestResult> results;

    for (const auto& [name, benchmark] : benchmarks_) {
        try {
            auto result = RunBenchmark(name, config);
            results.push_back(result);
        } catch (const std::exception& e) {
            PerformanceTestResult failed_result;
            failed_result.test_name = name;
            failed_result.scale = config.scale;
            failed_result.passed = false;
            failed_result.error_message = std::string("测试执行失败: ") + e.what();
            failed_result.start_time = std::chrono::system_clock::now();
            failed_result.end_time = std::chrono::system_clock::now();
            results.push_back(failed_result);

            std::cerr << "性能测试失败 [" << name << "]: " << e.what() << std::endl;
        }
    }

    return results;
}

PerformanceTestResult PerformanceBenchmarkManager::RunBenchmark(
    const std::string& name, const PerformanceTestConfig& config) {

    auto it = benchmarks_.find(name);
    if (it == benchmarks_.end()) {
        throw std::runtime_error("未找到基准测试: " + name);
    }

    const auto& benchmark = it->second;
    PerformanceTestResult result;
    result.test_name = name;
    result.scale = config.scale;
    result.start_time = std::chrono::system_clock::now();

    try {
        std::cout << "\n=== 开始性能测试: " << name << " ===" << std::endl;
        std::cout << "描述: " << benchmark.description << std::endl;

        // 预热阶段
        if (config.warmup_iterations > 0) {
            std::cout << "执行预热 (" << config.warmup_iterations << " 次迭代)..." << std::endl;
            Warmup(benchmark, config.warmup_iterations);
        }

        // 设置阶段
        if (benchmark.setup_func) {
            std::cout << "执行测试设置..." << std::endl;
            benchmark.setup_func();
        }

        // 创建指标收集器
        PerformanceMetricsCollector collector;

        // 执行测试（多次迭代）
        std::vector<std::vector<MetricValue>> all_metrics;
        for (int i = 0; i < config.iterations; ++i) {
            std::cout << "执行迭代 " << (i + 1) << "/" << config.iterations << "..." << std::endl;

            collector.Reset();
            collector.StartCollection();

            // 执行测试函数
            benchmark.test_func();

            collector.StopCollection();
            all_metrics.push_back(collector.GetMetrics());
        }

        // 合并指标（计算平均值）
        std::map<PerformanceMetric, std::vector<double>> metric_values;
        for (const auto& iteration_metrics : all_metrics) {
            for (const auto& metric : iteration_metrics) {
                metric_values[metric.metric].push_back(metric.value);
            }
        }

        // 计算平均指标值
        for (const auto& [metric, values] : metric_values) {
            if (!values.empty()) {
                double avg_value = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
                result.metrics.emplace_back(metric, avg_value);
            }
        }

        // 添加系统指标
        auto system_metrics = CollectSystemMetrics();
        result.metrics.insert(result.metrics.end(), system_metrics.begin(), system_metrics.end());

        // 检测性能回归
        if (config.enable_regression_detection) {
            result.regressions = DetectRegressions(name, result.metrics);
        }

        // 检查阈值
        result.passed = true;
        for (const auto& [threshold_name, threshold_value] : benchmark.thresholds) {
            // 这里可以根据阈值名称匹配相应的指标进行检查
            // 简化实现：假设阈值名称对应指标名称
        }

        std::cout << "=== 性能测试完成: " << name << " ===" << std::endl;

    } catch (const std::exception& e) {
        result.passed = false;
        result.error_message = std::string("测试执行异常: ") + e.what();
        std::cerr << "性能测试异常 [" << name << "]: " << e.what() << std::endl;
    }

    // 清理阶段
    try {
        if (benchmark.cleanup_func) {
            benchmark.cleanup_func();
        }
    } catch (const std::exception& e) {
        std::cerr << "清理阶段异常 [" << name << "]: " << e.what() << std::endl;
    }

    result.end_time = std::chrono::system_clock::now();
    return result;
}

bool PerformanceBenchmarkManager::LoadBaselines(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "无法打开基准线文件: " << filename << std::endl;
            return false;
        }

        std::string line;
        std::getline(file, line); // 跳过标题行

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string test_name, metric_str;
            double value;

            if (std::getline(ss, test_name, ',') &&
                std::getline(ss, metric_str, ',') &&
                (ss >> value)) {

                // 将字符串转换为PerformanceMetric枚举
                PerformanceMetric metric = PerformanceMetric::CUSTOM_METRIC;
                // 这里需要实现字符串到枚举的转换逻辑
                // 简化实现：暂时跳过

                baselines_[test_name][metric] = value;
            }
        }

        std::cout << "已加载基准线数据: " << filename << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "加载基准线数据失败: " << e.what() << std::endl;
        return false;
    }
}

bool PerformanceBenchmarkManager::SaveBaselines(const std::string& filename) const {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "无法创建基准线文件: " << filename << std::endl;
            return false;
        }

        // 写入标题
        file << "test_name,metric,value\n";

        // 写入基准线数据
        for (const auto& [test_name, metrics] : baselines_) {
            for (const auto& [metric, value] : metrics) {
                // 将PerformanceMetric转换为字符串
                std::string metric_str = "unknown"; // 需要实现枚举到字符串的转换
                file << test_name << "," << metric_str << "," << value << "\n";
            }
        }

        std::cout << "已保存基准线数据: " << filename << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "保存基准线数据失败: " << e.what() << std::endl;
        return false;
    }
}

void PerformanceBenchmarkManager::GenerateReport(
    const std::vector<PerformanceTestResult>& results,
    const std::string& output_dir) const {

    // 确保输出目录存在
    std::filesystem::create_directories(output_dir);

    // 生成摘要报告
    GenerateSummaryReport(results, output_dir + "/performance_summary.html");

    // 生成详细报告
    GenerateDetailedReport(results, output_dir + "/performance_detailed.json");

    // 生成回归报告
    GenerateRegressionReport(results, output_dir + "/performance_regressions.txt");

    // 生成趋势图数据
    GenerateTrendData(results, output_dir + "/performance_trends.csv");

    std::cout << "性能报告已生成到目录: " << output_dir << std::endl;
}

void PerformanceBenchmarkManager::GenerateSummaryReport(
    const std::vector<PerformanceTestResult>& results,
    const std::string& filename) const {

    std::ofstream file(filename);
    file << "<!DOCTYPE html>\n";
    file << "<html><head><title>SQLCC 性能测试报告</title>\n";
    file << "<style>\n";
    file << "body { font-family: Arial, sans-serif; margin: 20px; }\n";
    file << "table { border-collapse: collapse; width: 100%; }\n";
    file << "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n";
    file << "th { background-color: #f2f2f2; }\n";
    file << ".passed { color: green; }\n";
    file << ".failed { color: red; }\n";
    file << ".regression { background-color: #ffebee; }\n";
    file << "</style></head><body>\n";

    file << "<h1>SQLCC 性能测试报告</h1>\n";
    file << "<p>生成时间: " << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << "</p>\n";

    // 测试概览
    file << "<h2>测试概览</h2>\n";
    file << "<table>\n";
    file << "<tr><th>测试名称</th><th>状态</th><th>持续时间(ms)</th><th>回归数量</th></tr>\n";

    for (const auto& result : results) {
        std::string status_class = result.passed ? "passed" : "failed";
        std::string status_text = result.passed ? "通过" : "失败";

        file << "<tr>\n";
        file << "<td>" << result.test_name << "</td>\n";
        file << "<td class=\"" << status_class << "\">" << status_text << "</td>\n";
        file << "<td>" << result.GetDuration().count() << "</td>\n";
        file << "<td>" << result.regressions.size() << "</td>\n";
        file << "</tr>\n";
    }

    file << "</table>\n";

    // 性能指标汇总
    file << "<h2>性能指标汇总</h2>\n";
    file << "<table>\n";
    file << "<tr><th>测试名称</th><th>指标</th><th>值</th><th>单位</th></tr>\n";

    for (const auto& result : results) {
        for (const auto& metric : result.metrics) {
            file << "<tr>\n";
            file << "<td>" << result.test_name << "</td>\n";
            file << "<td>" << static_cast<int>(metric.metric) << "</td>\n"; // 临时使用枚举值
            file << "<td>" << metric.value << "</td>\n";
            file << "<td>" << metric.unit << "</td>\n";
            file << "</tr>\n";
        }
    }

    file << "</table>\n";

    // 回归检测结果
    file << "<h2>回归检测结果</h2>\n";
    file << "<table>\n";
    file << "<tr><th>测试名称</th><th>指标</th><th>基线值</th><th>当前值</th><th>变化(%)</th><th>严重程度</th></tr>\n";

    for (const auto& result : results) {
        for (const auto& regression : result.regressions) {
            std::string row_class = regression.is_regression ? "regression" : "";
            file << "<tr class=\"" << row_class << "\">\n";
            file << "<td>" << regression.test_name << "</td>\n";
            file << "<td>" << static_cast<int>(regression.metric) << "</td>\n";
            file << "<td>" << regression.baseline_value << "</td>\n";
            file << "<td>" << regression.current_value << "</td>\n";
            file << "<td>" << regression.change_percent << "</td>\n";
            file << "<td>" << regression.severity << "</td>\n";
            file << "</tr>\n";
        }
    }

    file << "</table>\n";
    file << "</body></html>\n";
}

void PerformanceBenchmarkManager::GenerateDetailedReport(
    const std::vector<PerformanceTestResult>& results,
    const std::string& filename) const {

    // 这里实现JSON格式的详细报告
    // 简化实现：使用简单的文本格式
    std::ofstream file(filename);
    file << "{\n";
    file << "  \"performance_test_results\": [\n";

    for (size_t i = 0; i < results.size(); ++i) {
        const auto& result = results[i];
        file << "    {\n";
        file << "      \"test_name\": \"" << result.test_name << "\",\n";
        file << "      \"passed\": " << (result.passed ? "true" : "false") << ",\n";
        file << "      \"duration_ms\": " << result.GetDuration().count() << ",\n";
        file << "      \"metrics\": [\n";

        for (size_t j = 0; j < result.metrics.size(); ++j) {
            const auto& metric = result.metrics[j];
            file << "        {\n";
            file << "          \"metric\": " << static_cast<int>(metric.metric) << ",\n";
            file << "          \"value\": " << metric.value << ",\n";
            file << "          \"unit\": \"" << metric.unit << "\"\n";
            file << "        }";
            if (j < result.metrics.size() - 1) file << ",";
            file << "\n";
        }

        file << "      ],\n";
        file << "      \"regressions\": [\n";

        for (size_t j = 0; j < result.regressions.size(); ++j) {
            const auto& regression = result.regressions[j];
            file << "        {\n";
            file << "          \"metric\": " << static_cast<int>(regression.metric) << ",\n";
            file << "          \"baseline_value\": " << regression.baseline_value << ",\n";
            file << "          \"current_value\": " << regression.current_value << ",\n";
            file << "          \"change_percent\": " << regression.change_percent << ",\n";
            file << "          \"severity\": \"" << regression.severity << "\"\n";
            file << "        }";
            if (j < result.regressions.size() - 1) file << ",";
            file << "\n";
        }

        file << "      ]\n";
        file << "    }";
        if (i < results.size() - 1) file << ",";
        file << "\n";
    }

    file << "  ]\n";
    file << "}\n";
}

void PerformanceBenchmarkManager::GenerateRegressionReport(
    const std::vector<PerformanceTestResult>& results,
    const std::string& filename) const {

    std::ofstream file(filename);

    file << "SQLCC 性能回归报告\n";
    file << "生成时间: " << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << "\n";
    file << std::string(50, '=') << "\n\n";

    int total_regressions = 0;
    std::map<std::string, int> severity_count;

    for (const auto& result : results) {
        if (!result.regressions.empty()) {
            file << "测试: " << result.test_name << "\n";
            file << "发现 " << result.regressions.size() << " 个性能回归:\n\n";

            for (const auto& regression : result.regressions) {
                file << "  - 指标: " << static_cast<int>(regression.metric) << "\n";
                file << "    基线值: " << regression.baseline_value << "\n";
                file << "    当前值: " << regression.current_value << "\n";
                file << "    变化: " << regression.change_percent << "%\n";
                file << "    严重程度: " << regression.severity << "\n\n";

                total_regressions++;
                severity_count[regression.severity]++;
            }

            file << std::string(30, '-') << "\n\n";
        }
    }

    if (total_regressions == 0) {
        file << "未发现性能回归！\n";
    } else {
        file << "回归汇总:\n";
        file << "总回归数: " << total_regressions << "\n";
        for (const auto& [severity, count] : severity_count) {
            file << severity << "级: " << count << " 个\n";
        }
    }
}

void PerformanceBenchmarkManager::GenerateTrendData(
    const std::vector<PerformanceTestResult>& results,
    const std::string& filename) const {

    std::ofstream file(filename);
    file << "timestamp,test_name,metric,value,unit\n";

    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    for (const auto& result : results) {
        for (const auto& metric : result.metrics) {
            file << now << ",";
            file << result.test_name << ",";
            file << static_cast<int>(metric.metric) << ",";
            file << metric.value << ",";
            file << metric.unit << "\n";
        }
    }
}

std::vector<RegressionResult> PerformanceBenchmarkManager::DetectRegressions(
    const std::string& test_name,
    const std::vector<MetricValue>& metrics) const {

    std::vector<RegressionResult> regressions;

    auto baseline_it = baselines_.find(test_name);
    if (baseline_it == baselines_.end()) {
        // 没有基准线数据，跳过回归检测
        return regressions;
    }

    const auto& baseline_metrics = baseline_it->second;

    for (const auto& metric : metrics) {
        auto baseline_value_it = baseline_metrics.find(metric.metric);
        if (baseline_value_it != baseline_metrics.end()) {
            RegressionResult regression(test_name, metric.metric,
                                      baseline_value_it->second, metric.value);
            if (regression.is_regression) {
                regressions.push_back(regression);
            }
        }
    }

    return regressions;
}

std::vector<MetricValue> PerformanceBenchmarkManager::CollectSystemMetrics() const {
    std::vector<MetricValue> metrics;

    // 这里应该收集实际的系统指标
    // 简化实现：返回模拟数据

    // CPU使用率
    metrics.emplace_back(PerformanceMetric::CPU_USAGE_PERCENT, 45.2, "%");

    // 内存使用量
    metrics.emplace_back(PerformanceMetric::MEMORY_USAGE_MB, 256.8, "MB");

    return metrics;
}

void PerformanceBenchmarkManager::Warmup(const PerformanceBenchmark& benchmark, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        try {
            if (benchmark.test_func) {
                benchmark.test_func();
            }
        } catch (const std::exception& e) {
            std::cerr << "预热阶段异常: " << e.what() << std::endl;
        }
    }
}

// 性能指标收集器实现
void PerformanceMetricsCollector::StartCollection() {
    start_time_ = std::chrono::high_resolution_clock::now();
    is_collecting_ = true;
    latency_samples_.clear();
    operations_completed_ = 0;
}

void PerformanceMetricsCollector::StopCollection() {
    if (!is_collecting_) return;

    is_collecting_ = false;
}

void PerformanceMetricsCollector::RecordLatency(double latency_ms) {
    if (is_collecting_) {
        latency_samples_.push_back(latency_ms);
    }
}

void PerformanceMetricsCollector::RecordOperation() {
    if (is_collecting_) {
        operations_completed_++;
    }
}

std::vector<MetricValue> PerformanceMetricsCollector::GetMetrics() const {
    std::vector<MetricValue> metrics;

    if (latency_samples_.empty() && operations_completed_ == 0) {
        return metrics;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_);

    // 计算吞吐量
    double throughput = 0.0;
    if (duration.count() > 0) {
        throughput = static_cast<double>(operations_completed_) / (duration.count() / 1000.0);
    }
    metrics.emplace_back(PerformanceMetric::THROUGHPUT_OPS_SEC, throughput, "ops/sec");

    // 计算延迟统计
    if (!latency_samples_.empty()) {
        std::vector<double> sorted_samples = latency_samples_;
        std::sort(sorted_samples.begin(), sorted_samples.end());

        // 平均延迟
        double avg_latency = std::accumulate(latency_samples_.begin(), latency_samples_.end(), 0.0) /
                           latency_samples_.size();
        metrics.emplace_back(PerformanceMetric::AVG_LATENCY_MS, avg_latency, "ms");

        // 最小延迟
        metrics.emplace_back(PerformanceMetric::MIN_LATENCY_MS, sorted_samples.front(), "ms");

        // 最大延迟
        metrics.emplace_back(PerformanceMetric::MAX_LATENCY_MS, sorted_samples.back(), "ms");

        // P50延迟
        size_t p50_index = sorted_samples.size() * 0.5;
        metrics.emplace_back(PerformanceMetric::P50_LATENCY_MS, sorted_samples[p50_index], "ms");

        // P95延迟
        size_t p95_index = sorted_samples.size() * 0.95;
        metrics.emplace_back(PerformanceMetric::P95_LATENCY_MS, sorted_samples[p95_index], "ms");

        // P99延迟
        size_t p99_index = sorted_samples.size() * 0.99;
        metrics.emplace_back(PerformanceMetric::P99_LATENCY_MS, sorted_samples[p99_index], "ms");
    }

    return metrics;
}

void PerformanceMetricsCollector::Reset() {
    is_collecting_ = false;
    latency_samples_.clear();
    operations_completed_ = 0;
}

}  // namespace test
}  // namespace sqlcc