/**
 * @file standalone_test.h
 * @brief 独立测试器头文件
 */

#ifndef SQLCC_EXECUTION_STANDALONE_TEST_H
#define SQLCC_EXECUTION_STANDALONE_TEST_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <chrono>

#include "execution/test_runner.h"
#include "core/execution_result.h"

namespace sqlcc {

class ExecutionContext;

// 测试配置
struct TestConfiguration {
    bool verbose = false;
    bool quiet = false;
    std::string output_file;
    std::chrono::seconds timeout = std::chrono::seconds(30);
    size_t max_concurrent = 4;
    std::vector<std::string> include_suites;
    std::vector<std::string> exclude_suites;
    std::vector<std::string> include_tests;
    std::vector<std::string> exclude_tests;
};

// 测试统计
struct TestStatistics {
    size_t total_suites = 0;
    size_t total_tests = 0;
    size_t passed_tests = 0;
    size_t failed_tests = 0;
    size_t skipped_tests = 0;
    std::chrono::milliseconds total_execution_time = std::chrono::milliseconds(0);
    double success_rate = 0.0;
};

// 独立测试器 - 提供命令行测试执行功能
class StandaloneTest {
public:
    StandaloneTest();
    ~StandaloneTest() = default;

    // 配置管理
    void configure(const TestConfiguration& config);
    TestConfiguration getConfiguration() const;

    // 测试发现和注册
    void registerTestSuite(const std::string& suite_name,
                          const std::vector<std::pair<std::string, std::function<bool(ExecutionContext&)>>>& tests);

    void autoDiscoverTests();
    void loadTestSuiteFromFile(const std::string& file_path);

    // 测试执行
    TestStatistics runAllTests(ExecutionContext& context);
    TestStatistics runSelectedSuites(const std::vector<std::string>& suite_names,
                                   ExecutionContext& context);
    TestStatistics runSelectedTests(const std::vector<std::string>& test_names,
                                  ExecutionContext& context);

    // 结果输出
    void printResults(const TestStatistics& stats) const;
    void saveResultsToFile(const TestStatistics& stats, const std::string& file_path) const;
    std::string generateXMLReport(const TestStatistics& stats) const;
    std::string generateJSONReport(const TestStatistics& stats) const;

    // 命令行接口
    int runFromCommandLine(int argc, char* argv[], ExecutionContext& context);
    void printUsage() const;
    void printVersion() const;

private:
    TestRunner test_runner_;
    TestConfiguration config_;
    std::vector<std::string> discovered_suites_;

    // 命令行解析
    TestConfiguration parseCommandLine(int argc, char* argv[]);
    void applyConfigurationFilters();

    // 测试发现
    void discoverTestSuites();
    void loadBuiltInTestSuites();

    // 输出格式化
    void printTestHeader() const;
    void printTestResult(const TestResult& result, bool verbose) const;
    void printTestSuiteSummary(const TestSuite& suite) const;
    void printOverallSummary(const TestStatistics& stats) const;

    // 辅助方法
    bool shouldRunSuite(const std::string& suite_name) const;
    bool shouldRunTest(const std::string& test_name) const;
    TestStatistics calculateStatistics(const std::vector<TestSuite>& suites) const;
    std::string formatDuration(std::chrono::milliseconds duration) const;
    std::string getCurrentTimestamp() const;
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_STANDALONE_TEST_H
