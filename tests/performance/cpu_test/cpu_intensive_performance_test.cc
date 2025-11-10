#include "cpu_intensive_performance_test.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <string.h>

namespace sqlcc {
namespace test {

CpuIntensivePerformanceTest::CpuIntensivePerformanceTest() {
    GenerateTestData();
}

CpuIntensivePerformanceTest::~CpuIntensivePerformanceTest() {
    Cleanup();
}

void CpuIntensivePerformanceTest::RunAllTests() {
    std::cout << "Running CPU Intensive Performance Tests..." << std::endl;
    
    std::vector<TestResult> results;
    
    // 运行查询处理测试
    results.push_back(TestQueryProcessing());
    
    // 运行索引操作测试
    results.push_back(TestIndexOperations());
    
    // 运行事务处理测试
    results.push_back(TestTransactionProcessing());
    
    // 生成报告
    GenerateReport(results);
    
    // 保存结果到文件
    SaveResultsToFile(results, "cpu_intensive_performance_results.csv");
}

void CpuIntensivePerformanceTest::Cleanup() {
    // 清理测试数据
    int_data_.clear();
    string_data_.clear();
    tree_data_.clear();
    search_keys_.clear();
    log_data_.clear();
}

CpuIntensivePerformanceTest::TestResult CpuIntensivePerformanceTest::TestQueryProcessing() {
    TestResult result;
    result.test_name = "Query Processing Test";
    
    std::cout << "Running query processing test..." << std::endl;
    
    // 准备测试数据
    std::vector<int> sort_data = int_data_;
    
    // 记录开始时间
    auto start_time = GetCurrentTime();
    
    // 执行查询处理模拟操作
    size_t operations = 0;
    std::vector<double> latencies;
    
    // 模拟数据排序操作
    for (size_t i = 0; i < kDefaultIterations; ++i) {
        double latency = SimulateDataSorting(sort_data, 10);
        
        latencies.push_back(latency);
        operations += 10;  // 每次迭代执行10次排序操作
    }
    
    // 模拟哈希计算操作
    for (size_t i = 0; i < kDefaultIterations; ++i) {
        double latency = SimulateHashCalculation(string_data_, 100);
        
        latencies.push_back(latency);
        operations += 100;  // 每次迭代执行100次哈希计算
    }
    
    // 记录结束时间
    auto end_time = GetCurrentTime();
    
    // 计算测试结果
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = operations;
    result.throughput = CalculateThroughput(operations, result.duration);
    CalculateLatencies(latencies, result.avg_latency, result.p95_latency, result.p99_latency);
    
    // 添加自定义指标
    result.custom_metrics["Data Size"] = std::to_string(kDataSize);
    result.custom_metrics["Sort Operations"] = std::to_string(kDefaultIterations * 10);
    result.custom_metrics["Hash Operations"] = std::to_string(kDefaultIterations * 100);
    
    // 打印结果
    PrintResult(result);
    
    return result;
}

CpuIntensivePerformanceTest::TestResult CpuIntensivePerformanceTest::TestIndexOperations() {
    TestResult result;
    result.test_name = "Index Operations Test";
    
    std::cout << "Running index operations test..." << std::endl;
    
    // 记录开始时间
    auto start_time = GetCurrentTime();
    
    // 执行索引操作模拟
    size_t operations = 0;
    std::vector<double> latencies;
    
    // 模拟B+树搜索操作
    for (size_t i = 0; i < kDefaultIterations; ++i) {
        double latency = SimulateBPlusTreeSearch(tree_data_, search_keys_, 10);
        
        latencies.push_back(latency);
        operations += 10;  // 每次迭代执行10次搜索操作
    }
    
    // 记录结束时间
    auto end_time = GetCurrentTime();
    
    // 计算测试结果
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = operations;
    result.throughput = CalculateThroughput(operations, result.duration);
    CalculateLatencies(latencies, result.avg_latency, result.p95_latency, result.p99_latency);
    
    // 添加自定义指标
    result.custom_metrics["Tree Size"] = std::to_string(kTreeSize);
    result.custom_metrics["Search Keys"] = std::to_string(kSearchKeyCount);
    result.custom_metrics["Search Operations"] = std::to_string(kDefaultIterations * 10);
    
    // 打印结果
    PrintResult(result);
    
    return result;
}

CpuIntensivePerformanceTest::TestResult CpuIntensivePerformanceTest::TestTransactionProcessing() {
    TestResult result;
    result.test_name = "Transaction Processing Test";
    
    std::cout << "Running transaction processing test..." << std::endl;
    
    // 记录开始时间
    auto start_time = GetCurrentTime();
    
    // 执行事务处理模拟
    size_t operations = 0;
    std::vector<double> latencies;
    
    // 模拟事务日志操作
    for (size_t i = 0; i < kDefaultIterations; ++i) {
        double latency = SimulateTransactionLogging(log_data_, 10);
        
        latencies.push_back(latency);
        operations += 10;  // 每次迭代执行10次日志操作
    }
    
    // 模拟锁管理操作
    for (size_t i = 0; i < kDefaultIterations; ++i) {
        double latency = SimulateLockManagement(100, 10);
        
        latencies.push_back(latency);
        operations += 10;  // 每次迭代执行10次锁管理操作
    }
    
    // 记录结束时间
    auto end_time = GetCurrentTime();
    
    // 计算测试结果
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = operations;
    result.throughput = CalculateThroughput(operations, result.duration);
    CalculateLatencies(latencies, result.avg_latency, result.p95_latency, result.p99_latency);
    
    // 添加自定义指标
    result.custom_metrics["Log Entries"] = std::to_string(kLogEntryCount);
    result.custom_metrics["Log Operations"] = std::to_string(kDefaultIterations * 10);
    result.custom_metrics["Lock Operations"] = std::to_string(kDefaultIterations * 10);
    
    // 打印结果
    PrintResult(result);
    
    return result;
}

double CpuIntensivePerformanceTest::SimulateDataSorting(std::vector<int>& data, size_t iterations) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < iterations; ++i) {
        // 创建数据副本并排序
        std::vector<int> data_copy = data;
        std::sort(data_copy.begin(), data_copy.end());
        
        // 反向排序
        std::sort(data_copy.rbegin(), data_copy.rend());
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return static_cast<double>(duration.count());
}

double CpuIntensivePerformanceTest::SimulateHashCalculation(const std::vector<std::string>& data, size_t iterations) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::hash<std::string> hasher;
    std::unordered_map<size_t, int> hash_counts;
    
    for (size_t i = 0; i < iterations; ++i) {
        for (const auto& str : data) {
            size_t hash_value = hasher(str);
            hash_counts[hash_value]++;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return static_cast<double>(duration.count());
}

double CpuIntensivePerformanceTest::SimulateBPlusTreeSearch(const std::vector<int>& tree_data, 
                                                             const std::vector<int>& search_keys, 
                                                             size_t iterations) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // 模拟B+树搜索（使用二分查找作为简化）
    std::vector<int> sorted_tree = tree_data;
    std::sort(sorted_tree.begin(), sorted_tree.end());
    
    for (size_t i = 0; i < iterations; ++i) {
        for (int key : search_keys) {
            // 模拟B+树搜索操作
            auto it = std::lower_bound(sorted_tree.begin(), sorted_tree.end(), key);
            if (it != sorted_tree.end() && *it == key) {
                // 找到键，执行一些操作
                volatile int value = *it;  // 防止编译器优化
                (void)value;  // 避免未使用变量警告
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return static_cast<double>(duration.count());
}

double CpuIntensivePerformanceTest::SimulateTransactionLogging(const std::vector<std::string>& log_data, size_t iterations) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // 模拟事务日志写入和读取
    std::vector<std::string> log_buffer;
    
    for (size_t i = 0; i < iterations; ++i) {
        // 模拟日志写入
        for (const auto& entry : log_data) {
            std::string log_entry = "TXN:" + std::to_string(i) + ":" + entry;
            log_buffer.push_back(log_entry);
        }
        
        // 模拟日志读取和处理
        for (const auto& entry : log_buffer) {
            // 模拟日志解析
            size_t pos1 = entry.find(':');
            size_t pos2 = entry.find(':', pos1 + 1);
            if (pos1 != std::string::npos && pos2 != std::string::npos) {
                std::string txn_id = entry.substr(pos1 + 1, pos2 - pos1 - 1);
                std::string operation = entry.substr(pos2 + 1);
                // 模拟日志处理
                volatile size_t id = std::stoull(txn_id);
                (void)id;  // 避免未使用变量警告
            }
        }
        
        // 清空缓冲区
        log_buffer.clear();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return static_cast<double>(duration.count());
}

double CpuIntensivePerformanceTest::SimulateLockManagement(size_t lock_count, size_t iterations) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // 模拟锁表
    std::vector<std::atomic<bool>> lock_table(lock_count);
    
    for (size_t i = 0; i < iterations; ++i) {
        // 模拟锁获取和释放
        for (size_t j = 0; j < lock_count; ++j) {
            // 尝试获取锁
            bool expected = false;
            if (lock_table[j].compare_exchange_strong(expected, true)) {
                // 模拟临界区操作
                std::this_thread::sleep_for(std::chrono::microseconds(1));
                
                // 释放锁
                lock_table[j] = false;
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return static_cast<double>(duration.count());
}

void CpuIntensivePerformanceTest::GenerateTestData() {
    // 生成整数数据
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> int_dist(1, 100000);
    
    int_data_.reserve(kDataSize);
    for (size_t i = 0; i < kDataSize; ++i) {
        int_data_.push_back(int_dist(gen));
    }
    
    // 生成字符串数据
    std::uniform_int_distribution<> str_len_dist(10, 50);
    std::uniform_int_distribution<> char_dist('a', 'z');
    
    string_data_.reserve(kDataSize);
    for (size_t i = 0; i < kDataSize; ++i) {
        int len = str_len_dist(gen);
        std::string str;
        str.reserve(len);
        for (int j = 0; j < len; ++j) {
            str.push_back(static_cast<char>(char_dist(gen)));
        }
        string_data_.push_back(str);
    }
    
    // 生成B+树数据（已排序）
    tree_data_.reserve(kTreeSize);
    for (size_t i = 0; i < kTreeSize; ++i) {
        tree_data_.push_back(static_cast<int>(i * 10));
    }
    
    // 生成搜索键
    search_keys_.reserve(kSearchKeyCount);
    for (size_t i = 0; i < kSearchKeyCount; ++i) {
        search_keys_.push_back(int_dist(gen) % (kTreeSize * 10));
    }
    
    // 生成日志数据
    log_data_.reserve(kLogEntryCount);
    for (size_t i = 0; i < kLogEntryCount; ++i) {
        log_data_.push_back("INSERT INTO table VALUES (" + std::to_string(i) + ", 'value" + std::to_string(i) + "')");
    }
}

}  // namespace test
}  // namespace sqlcc