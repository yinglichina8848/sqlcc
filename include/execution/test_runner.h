/**
 * @file test_runner.h
 * @brief 测试运行器头文件
 */

#ifndef SQLCC_EXECUTION_TEST_RUNNER_H
#define SQLCC_EXECUTION_TEST_RUNNER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <chrono>
#include <functional>

#include "execution/task_executor.h"
#include "core/execution_result.h"

namespace sqlcc {

class ExecutionContext;

// 测试结果
struct TestResult {
    std::string test_name;
    bool passed = false;
    std::string error_message;
    std::chrono::milliseconds execution_time = std::chrono::milliseconds(0);
    std::string details;
};

// 测试套件
struct TestSuite {
    std::string suite_name;
    std::vector<std::string> test_names;
    std::vector<TestResult> results;
    std::chrono::milliseconds total_execution_time = std::chrono::milliseconds(0);
    size_t passed_count = 0;
    size_t failed_count = 0;
};

// 测试运行器 - 管理测试执行
class TestRunner {
public:
    TestRunner();
    ~TestRunner() = default;

    // 测试注册
    void registerTest(const std::string& test_name,
                     std::function<bool(ExecutionContext&)> test_function,
                     const std::string& description = "");

    void registerTestSuite(const std::string& suite_name,
                          const std::vector<std::string>& test_names);

    // 测试执行
    TestResult runTest(const std::string& test_name, ExecutionContext& context);
    TestSuite runTestSuite(const std::string& suite_name, ExecutionContext& context);
    std::vector<TestSuite> runAllTests(ExecutionContext& context);

    // 并发测试执行
    TestSuite runTestSuiteParallel(const std::string& suite_name,
                                  ExecutionContext& context,
                                  size_t max_concurrent = 4);

    // 测试配置
    void setTestTimeout(std::chrono::seconds timeout);
    void setMaxConcurrentTests(size_t max_concurrent);
    void enableDetailedLogging(bool enable);

    // 结果查询
    std::vector<std::string> getRegisteredTests() const;
    std::vector<std::string> getRegisteredSuites() const;
    TestResult getLastTestResult(const std::string& test_name) const;
    std::string generateReport() const;

    // 清理
    void clearResults();
    void unregisterTest(const std::string& test_name);
    void unregisterSuite(const std::string& suite_name);

private:
    // 测试信息
    struct TestInfo {
        std::string name;
        std::string description;
        std::function<bool(ExecutionContext&)> function;
        TestResult last_result;
    };

    // 内部状态
    std::unordered_map<std::string, TestInfo> registered_tests_;
    std::unordered_map<std::string, std::vector<std::string>> test_suites_;
    mutable std::mutex test_mutex_;

    // 配置
    std::chrono::seconds test_timeout_ = std::chrono::seconds(30);
    size_t max_concurrent_tests_ = 4;
    bool detailed_logging_ = false;

    // 统计
    size_t total_tests_run_ = 0;
    size_t total_tests_passed_ = 0;
    size_t total_tests_failed_ = 0;
    std::chrono::milliseconds total_execution_time_ = std::chrono::milliseconds(0);

    // 辅助方法
    TestResult executeTestWithTimeout(const TestInfo& test_info, ExecutionContext& context);
    std::string formatTestResult(const TestResult& result) const;
    std::string formatTestSuite(const TestSuite& suite) const;
    void updateStatistics(const TestResult& result);
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_TEST_RUNNER_H
