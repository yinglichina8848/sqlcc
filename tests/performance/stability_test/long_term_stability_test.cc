#include "long_term_stability_test.h"
#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <vector>
#include <memory>
#include <atomic>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace sqlcc {
namespace test {

LongTermStabilityTest::LongTermStabilityTest()
    : test_running_(false), total_operations_(0), error_count_(0), warning_count_(0) {
    config_ = TestConfig();
}

LongTermStabilityTest::~LongTermStabilityTest() {
    Cleanup();
}

void LongTermStabilityTest::SetOutputDirectory(const std::string& directory) {
    config_.output_directory = directory;
}

void LongTermStabilityTest::SetConfig(const TestConfig& config) {
    config_ = config;
}

void LongTermStabilityTest::RunAllTests() {
    std::cout << "Running Long Term Stability Tests..." << std::endl;
    
    Initialize();
    RunAllStabilityTests();
    Cleanup();
}

void LongTermStabilityTest::Initialize() {
    test_running_ = true;
    total_operations_ = 0;
    error_count_ = 0;
    warning_count_ = 0;
}

void LongTermStabilityTest::RunAllStabilityTests() {
    std::vector<StabilityTestResult> results;
    
    // 连续操作稳定性测试
    results.push_back(TestContinuousOperations());
    
    // 内存稳定性测试
    results.push_back(TestMemoryStability());
    
    // 资源清理测试
    results.push_back(TestResourceCleanup());
    
    // 错误恢复测试
    results.push_back(TestErrorRecovery());
    
    // 生成报告
    GenerateReport(results);
    
    // 保存结果
    SaveResultsToFile(results, config_.output_file);
}

LongTermStabilityTest::StabilityTestResult LongTermStabilityTest::TestContinuousOperations() {
    StabilityTestResult result;
    result.test_name = "Continuous Operations Test";
    
    std::cout << "Running continuous operations test..." << std::endl;
    
    auto start_time = GetCurrentTime();
    
    RunContinuousOperations(config_.test_duration.count());
    
    auto end_time = GetCurrentTime();
    
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = total_operations_;
    result.error_count = error_count_;
    result.warning_count = warning_count_;
    result.throughput = CalculateThroughput(result.operations_completed, result.duration);
    result.success_rate = CalculateSuccessRate(result.operations_completed, result.error_count);
    
    PrintResult(result);
    return result;
}

LongTermStabilityTest::StabilityTestResult LongTermStabilityTest::TestMemoryStability() {
    StabilityTestResult result;
    result.test_name = "Memory Stability Test";
    
    std::cout << "Running memory stability test..." << std::endl;
    
    auto start_time = GetCurrentTime();
    
    RunMemoryStabilityTest(30); // 30秒内存稳定性测试
    
    auto end_time = GetCurrentTime();
    
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = total_operations_;
    result.error_count = error_count_;
    result.warning_count = warning_count_;
    result.throughput = CalculateThroughput(result.operations_completed, result.duration);
    result.success_rate = CalculateSuccessRate(result.operations_completed, result.error_count);
    
    PrintResult(result);
    return result;
}

LongTermStabilityTest::StabilityTestResult LongTermStabilityTest::TestResourceCleanup() {
    StabilityTestResult result;
    result.test_name = "Resource Cleanup Test";
    
    std::cout << "Running resource cleanup test..." << std::endl;
    
    auto start_time = GetCurrentTime();
    
    RunResourceCleanupTest(30); // 30秒资源清理测试
    
    auto end_time = GetCurrentTime();
    
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = total_operations_;
    result.error_count = error_count_;
    result.warning_count = warning_count_;
    result.throughput = CalculateThroughput(result.operations_completed, result.duration);
    result.success_rate = CalculateSuccessRate(result.operations_completed, result.error_count);
    
    PrintResult(result);
    return result;
}

LongTermStabilityTest::StabilityTestResult LongTermStabilityTest::TestErrorRecovery() {
    StabilityTestResult result;
    result.test_name = "Error Recovery Test";
    
    std::cout << "Running error recovery test..." << std::endl;
    
    auto start_time = GetCurrentTime();
    
    RunErrorRecoveryTest(30); // 30秒错误恢复测试
    
    auto end_time = GetCurrentTime();
    
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = total_operations_;
    result.error_count = error_count_;
    result.warning_count = warning_count_;
    result.throughput = CalculateThroughput(result.operations_completed, result.duration);
    result.success_rate = CalculateSuccessRate(result.operations_completed, result.error_count);
    
    PrintResult(result);
    return result;
}

void LongTermStabilityTest::RunContinuousOperations(size_t duration_seconds) {
    std::vector<std::thread> threads;
    
    // 启动工作线程
    for (size_t i = 0; i < config_.thread_count; ++i) {
        threads.emplace_back(&LongTermStabilityTest::WorkerThread, this, duration_seconds);
    }
    
    // 启动监控线程
    std::thread monitor_thread(&LongTermStabilityTest::MonitorThread, this, duration_seconds);
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    monitor_thread.join();
}

void LongTermStabilityTest::RunMemoryStabilityTest(size_t duration_seconds) {
    auto start_time = GetCurrentTime();
    
    while (!ShouldStopTest() && 
           CalculateDuration(start_time, GetCurrentTime()) < duration_seconds) {
        SimulateMemoryAllocation(1024); // 分配1KB
        IncrementOperations();
    }
}

void LongTermStabilityTest::RunResourceCleanupTest(size_t duration_seconds) {
    auto start_time = GetCurrentTime();
    
    while (!ShouldStopTest() && 
           CalculateDuration(start_time, GetCurrentTime()) < duration_seconds) {
        SimulateResourceCleanup();
        IncrementOperations();
    }
}

void LongTermStabilityTest::RunErrorRecoveryTest(size_t duration_seconds) {
    auto start_time = GetCurrentTime();
    
    while (!ShouldStopTest() && 
           CalculateDuration(start_time, GetCurrentTime()) < duration_seconds) {
        SimulateErrorRecovery();
        IncrementOperations();
    }
}

bool LongTermStabilityTest::SimulateDatabaseOperation(size_t operation_id) {
    // 模拟数据库操作
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    return (operation_id % 100) != 0; // 1% 的错误率
}

bool LongTermStabilityTest::SimulateMemoryAllocation(size_t size) {
    // 模拟内存分配
    try {
        std::vector<char> buffer(size);
        std::fill(buffer.begin(), buffer.end(), 'A');
        return true;
    } catch (const std::bad_alloc&) {
        return false;
    }
}

bool LongTermStabilityTest::SimulateResourceCleanup() {
    // 模拟资源清理
    std::vector<std::unique_ptr<int>> resources;
    for (int i = 0; i < 10; ++i) {
        resources.push_back(std::make_unique<int>(i));
    }
    resources.clear();
    return true;
}

bool LongTermStabilityTest::SimulateErrorRecovery() {
    // 模拟错误恢复
    if (rand() % 10 == 0) { // 10% 的错误率
        IncrementErrors();
        return false;
    }
    return true;
}

void LongTermStabilityTest::WorkerThread(size_t duration_seconds) {
    auto start_time = GetCurrentTime();
    size_t local_operation_id = 0;
    
    while (!ShouldStopTest() && 
           CalculateDuration(start_time, GetCurrentTime()) < duration_seconds) {
        if (SimulateDatabaseOperation(local_operation_id++)) {
            IncrementOperations();
        }
        
        // 随机延迟
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

void LongTermStabilityTest::MonitorThread(size_t duration_seconds) {
    auto start_time = GetCurrentTime();
    
    while (!ShouldStopTest() && 
           CalculateDuration(start_time, GetCurrentTime()) < duration_seconds) {
        std::cout << "Operations: " << total_operations_ 
                  << ", Errors: " << error_count_ 
                  << ", Warnings: " << warning_count_ << std::endl;
        
        std::this_thread::sleep_for(std::chrono::seconds(config_.sampling_interval_seconds));
    }
}

void LongTermStabilityTest::GenerateReport(const std::vector<StabilityTestResult>& results) {
    std::cout << "\n=== Long Term Stability Test Report ===" << std::endl;
    
    for (const auto& result : results) {
        PrintResult(result);
    }
}

void LongTermStabilityTest::PrintResult(const StabilityTestResult& result) {
    std::cout << "=== " << result.test_name << " ===" << std::endl;
    std::cout << "持续时间: " << result.duration << "s" << std::endl;
    std::cout << "完成操作数: " << result.operations_completed << std::endl;
    std::cout << "吞吐量: " << result.throughput << " ops/sec" << std::endl;
    std::cout << "错误数: " << result.error_count << std::endl;
    std::cout << "警告数: " << result.warning_count << std::endl;
    std::cout << "成功率: " << (result.success_rate * 100) << "%" << std::endl;
    
    if (!result.error_message.empty()) {
        std::cout << "错误信息: " << result.error_message << std::endl;
    }
    std::cout << std::endl;
}

void LongTermStabilityTest::SaveResultsToFile(const std::vector<StabilityTestResult>& results, const std::string& filename) {
    std::ofstream outfile(config_.output_directory + "/" + filename);
    if (!outfile.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return;
    }
    
    outfile << "Test Name,Duration(s),Operations,Throughput(op/s),Errors,Warnings,Success Rate(%)\n";
    
    for (const auto& result : results) {
        outfile << result.test_name << ","
                << result.duration << ","
                << result.operations_completed << ","
                << result.throughput << ","
                << result.error_count << ","
                << result.warning_count << ","
                << (result.success_rate * 100) << "\n";
    }
    
    outfile.close();
    std::cout << "测试结果已保存到: " << config_.output_directory << "/" << filename << std::endl;
}

void LongTermStabilityTest::Cleanup() {
    test_running_ = false;
    
    // 等待所有线程完成
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    worker_threads_.clear();
}

std::chrono::high_resolution_clock::time_point LongTermStabilityTest::GetCurrentTime() {
    return std::chrono::high_resolution_clock::now();
}

double LongTermStabilityTest::CalculateDuration(const std::chrono::high_resolution_clock::time_point& start, 
                                              const std::chrono::high_resolution_clock::time_point& end) {
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return static_cast<double>(duration.count()) / 1000.0;
}

double LongTermStabilityTest::CalculateThroughput(size_t operations, double duration) {
    return (duration > 0) ? static_cast<double>(operations) / duration : 0.0;
}

double LongTermStabilityTest::CalculateSuccessRate(size_t operations, size_t errors) {
    return (operations > 0) ? static_cast<double>(operations - errors) / operations : 0.0;
}

bool LongTermStabilityTest::ShouldStopTest() {
    return !test_running_;
}

void LongTermStabilityTest::IncrementOperations() {
    total_operations_++;
}

void LongTermStabilityTest::IncrementErrors() {
    error_count_++;
}

void LongTermStabilityTest::IncrementWarnings() {
    warning_count_++;
}

} // namespace test
} // namespace sqlcc