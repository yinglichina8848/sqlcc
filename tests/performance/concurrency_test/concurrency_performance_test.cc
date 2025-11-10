#include "concurrency_performance_test.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <functional>

namespace sqlcc {
namespace test {

ConcurrencyPerformanceTest::ConcurrencyPerformanceTest() 
    : test_running_(false) {
    // 创建简单的屏障替代std::barrier
    start_barrier_ = nullptr;
    GenerateTestData();
}

ConcurrencyPerformanceTest::~ConcurrencyPerformanceTest() {
    Cleanup();
}

void ConcurrencyPerformanceTest::RunAllTests() {
    std::cout << "Running Concurrency Performance Tests..." << std::endl;
    
    std::vector<TestResult> results;
    
    // 运行并发读取测试
    results.push_back(TestConcurrentReads());
    
    // 运行并发写入测试
    results.push_back(TestConcurrentWrites());
    
    // 运行混合读写测试
    results.push_back(TestMixedReadWrite());
    
    // 运行锁竞争测试
    results.push_back(TestLockContention());
    
    // 生成报告
    GenerateReport(results);
    
    // 保存结果到文件
    SaveResultsToFile(results, "concurrency_performance_results.csv");
}

void ConcurrencyPerformanceTest::Cleanup() {
    // 清理测试数据
    test_data_.clear();
    string_data_.clear();
    lock_table_.clear();
    
    // 清理屏障
    if (start_barrier_) {
        delete start_barrier_;
        start_barrier_ = nullptr;
    }
}

ConcurrencyPerformanceTest::TestResult ConcurrencyPerformanceTest::TestConcurrentReads() {
    TestResult result;
    result.test_name = "Concurrent Reads Test";
    
    std::cout << "Running concurrent reads test..." << std::endl;
    
    // 设置测试参数
    const size_t thread_count = kDefaultThreadCount;
    const size_t operations_per_thread = kOperationsPerThread;
    
    // 创建屏障
    SimpleBarrier barrier(static_cast<int>(thread_count));
    
    // 同步变量
    std::atomic<size_t> completed_ops{0};
    std::vector<std::thread> threads;
    std::vector<std::vector<double>> thread_latencies(thread_count);
    
    // 记录开始时间
    auto start_time = GetCurrentTime();
    
    // 启动工作线程
    for (size_t i = 0; i < thread_count; ++i) {
        threads.emplace_back([this, i, operations_per_thread, &completed_ops, &thread_latencies, &barrier]() {
            // 等待所有线程准备就绪
            barrier.Wait();
            
            // 执行读取操作
            ReadWorkerThread(static_cast<int>(i), operations_per_thread, completed_ops, thread_latencies[i]);
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 记录结束时间
    auto end_time = GetCurrentTime();
    
    // 合并所有线程的延迟数据
    std::vector<double> all_latencies;
    for (const auto& latencies : thread_latencies) {
        all_latencies.insert(all_latencies.end(), latencies.begin(), latencies.end());
    }
    
    // 计算测试结果
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = completed_ops.load();
    result.throughput = CalculateThroughput(result.operations_completed, result.duration);
    CalculateLatencies(all_latencies, result.avg_latency, result.p95_latency, result.p99_latency);
    
    // 添加自定义指标
    result.custom_metrics["Thread Count"] = std::to_string(thread_count);
    result.custom_metrics["Operations per Thread"] = std::to_string(operations_per_thread);
    result.custom_metrics["Data Size"] = std::to_string(kDataSize);
    
    // 打印结果
    PrintResult(result);
    
    return result;
}

ConcurrencyPerformanceTest::TestResult ConcurrencyPerformanceTest::TestConcurrentWrites() {
    TestResult result;
    result.test_name = "Concurrent Writes Test";
    
    std::cout << "Running concurrent writes test..." << std::endl;
    
    // 设置测试参数
    const size_t thread_count = kDefaultThreadCount;
    const size_t operations_per_thread = kOperationsPerThread;
    
    // 创建屏障
    SimpleBarrier barrier(static_cast<int>(thread_count));
    
    // 同步变量
    std::atomic<size_t> completed_ops{0};
    std::vector<std::thread> threads;
    std::vector<std::vector<double>> thread_latencies(thread_count);
    
    // 记录开始时间
    auto start_time = GetCurrentTime();
    
    // 启动工作线程
    for (size_t i = 0; i < thread_count; ++i) {
        threads.emplace_back([this, i, operations_per_thread, &completed_ops, &thread_latencies, &barrier]() {
            // 等待所有线程准备就绪
            barrier.Wait();
            
            // 执行写入操作
            WriteWorkerThread(static_cast<int>(i), operations_per_thread, completed_ops, thread_latencies[i]);
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 记录结束时间
    auto end_time = GetCurrentTime();
    
    // 合并所有线程的延迟数据
    std::vector<double> all_latencies;
    for (const auto& latencies : thread_latencies) {
        all_latencies.insert(all_latencies.end(), latencies.begin(), latencies.end());
    }
    
    // 计算测试结果
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = completed_ops.load();
    result.throughput = CalculateThroughput(result.operations_completed, result.duration);
    CalculateLatencies(all_latencies, result.avg_latency, result.p95_latency, result.p99_latency);
    
    // 添加自定义指标
    result.custom_metrics["Thread Count"] = std::to_string(thread_count);
    result.custom_metrics["Operations per Thread"] = std::to_string(operations_per_thread);
    result.custom_metrics["Data Size"] = std::to_string(kDataSize);
    
    // 打印结果
    PrintResult(result);
    
    return result;
}

ConcurrencyPerformanceTest::TestResult ConcurrencyPerformanceTest::TestMixedReadWrite() {
    TestResult result;
    result.test_name = "Mixed Read/Write Test";
    
    std::cout << "Running mixed read/write test..." << std::endl;
    
    // 设置测试参数
    const size_t thread_count = kDefaultThreadCount;
    const size_t operations_per_thread = kOperationsPerThread;
    const double read_ratio = 0.7;  // 70%读取，30%写入
    
    // 创建屏障
    SimpleBarrier barrier(static_cast<int>(thread_count));
    
    // 同步变量
    std::atomic<size_t> completed_ops{0};
    std::vector<std::thread> threads;
    std::vector<std::vector<double>> thread_latencies(thread_count);
    
    // 记录开始时间
    auto start_time = GetCurrentTime();
    
    // 启动工作线程
    for (size_t i = 0; i < thread_count; ++i) {
        threads.emplace_back([this, i, operations_per_thread, &completed_ops, &thread_latencies, &barrier, read_ratio]() {
            // 等待所有线程准备就绪
            barrier.Wait();
            
            // 执行混合读写操作
            MixedWorkerThread(static_cast<int>(i), operations_per_thread, completed_ops, thread_latencies[i], read_ratio);
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 记录结束时间
    auto end_time = GetCurrentTime();
    
    // 合并所有线程的延迟数据
    std::vector<double> all_latencies;
    for (const auto& latencies : thread_latencies) {
        all_latencies.insert(all_latencies.end(), latencies.begin(), latencies.end());
    }
    
    // 计算测试结果
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = completed_ops.load();
    result.throughput = CalculateThroughput(result.operations_completed, result.duration);
    CalculateLatencies(all_latencies, result.avg_latency, result.p95_latency, result.p99_latency);
    
    // 添加自定义指标
    result.custom_metrics["Thread Count"] = std::to_string(thread_count);
    result.custom_metrics["Operations per Thread"] = std::to_string(operations_per_thread);
    result.custom_metrics["Read Ratio"] = std::to_string(read_ratio);
    result.custom_metrics["Data Size"] = std::to_string(kDataSize);
    
    // 打印结果
    PrintResult(result);
    
    return result;
}

ConcurrencyPerformanceTest::TestResult ConcurrencyPerformanceTest::TestLockContention() {
    TestResult result;
    result.test_name = "Lock Contention Test";
    
    std::cout << "Running lock contention test..." << std::endl;
    
    // 设置测试参数
    const size_t thread_count = kDefaultThreadCount;
    const size_t operations_per_thread = kOperationsPerThread;
    
    // 创建屏障
    SimpleBarrier barrier(static_cast<int>(thread_count));
    
    // 共享互斥锁用于模拟锁竞争
    std::mutex shared_mutex;
    
    // 同步变量
    std::atomic<size_t> completed_ops{0};
    std::vector<std::thread> threads;
    std::vector<std::vector<double>> thread_latencies(thread_count);
    
    // 记录开始时间
    auto start_time = GetCurrentTime();
    
    // 启动工作线程
    for (size_t i = 0; i < thread_count; ++i) {
        threads.emplace_back([this, i, operations_per_thread, &completed_ops, &thread_latencies, &barrier, &shared_mutex]() {
            // 等待所有线程准备就绪
            barrier.Wait();
            
            // 执行锁竞争操作
            LockContentionWorkerThread(static_cast<int>(i), operations_per_thread, completed_ops, thread_latencies[i], shared_mutex);
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 记录结束时间
    auto end_time = GetCurrentTime();
    
    // 合并所有线程的延迟数据
    std::vector<double> all_latencies;
    for (const auto& latencies : thread_latencies) {
        all_latencies.insert(all_latencies.end(), latencies.begin(), latencies.end());
    }
    
    // 计算测试结果
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = completed_ops.load();
    result.throughput = CalculateThroughput(result.operations_completed, result.duration);
    CalculateLatencies(all_latencies, result.avg_latency, result.p95_latency, result.p99_latency);
    
    // 添加自定义指标
    result.custom_metrics["Thread Count"] = std::to_string(thread_count);
    result.custom_metrics["Operations per Thread"] = std::to_string(operations_per_thread);
    result.custom_metrics["Lock Count"] = std::to_string(kLockCount);
    
    // 打印结果
    PrintResult(result);
    
    return result;
}

void ConcurrencyPerformanceTest::ReadWorkerThread(int thread_id, size_t operations, 
                                                 std::atomic<size_t>& completed_ops,
                                                 std::vector<double>& latencies) {
    for (size_t i = 0; i < operations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        
        // 执行读取操作
        SimulateReadOperation(thread_id, static_cast<int>(i));
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        latencies.push_back(static_cast<double>(duration.count()) / 1000.0);  // 转换为毫秒
        
        completed_ops.fetch_add(1);
    }
}

void ConcurrencyPerformanceTest::WriteWorkerThread(int thread_id, size_t operations, 
                                                  std::atomic<size_t>& completed_ops,
                                                  std::vector<double>& latencies) {
    for (size_t i = 0; i < operations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        
        // 执行写入操作
        SimulateWriteOperation(thread_id, static_cast<int>(i));
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        latencies.push_back(static_cast<double>(duration.count()) / 1000.0);  // 转换为毫秒
        
        completed_ops.fetch_add(1);
    }
}

void ConcurrencyPerformanceTest::MixedWorkerThread(int thread_id, size_t operations, 
                                                  std::atomic<size_t>& completed_ops,
                                                  std::vector<double>& latencies,
                                                  double read_ratio) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    for (size_t i = 0; i < operations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        
        // 根据比例决定执行读取还是写入操作
        if (dis(gen) < read_ratio) {
            // 执行读取操作
            SimulateReadOperation(thread_id, static_cast<int>(i));
        } else {
            // 执行写入操作
            SimulateWriteOperation(thread_id, static_cast<int>(i));
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        latencies.push_back(static_cast<double>(duration.count()) / 1000.0);  // 转换为毫秒
        
        completed_ops.fetch_add(1);
    }
}

void ConcurrencyPerformanceTest::LockContentionWorkerThread(int thread_id, size_t operations, 
                                                           std::atomic<size_t>& completed_ops,
                                                           std::vector<double>& latencies,
                                                           std::mutex& mutex) {
    for (size_t i = 0; i < operations; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        
        // 执行锁竞争操作
        SimulateLockOperation(thread_id, static_cast<int>(i), mutex);
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        latencies.push_back(static_cast<double>(duration.count()) / 1000.0);  // 转换为毫秒
        
        completed_ops.fetch_add(1);
    }
}

void ConcurrencyPerformanceTest::GenerateTestData() {
    // 生成整数测试数据
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> int_dist(1, 100000);
    
    test_data_.reserve(kDataSize);
    for (size_t i = 0; i < kDataSize; ++i) {
        test_data_.push_back(int_dist(gen));
    }
    
    // 生成字符串测试数据
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
    
    // 初始化锁表
    lock_table_.clear();
    for (size_t i = 0; i < kLockCount; ++i) {
        lock_table_.push_back(std::make_unique<std::mutex>());
    }
}

void ConcurrencyPerformanceTest::SimulateReadOperation(int thread_id, int operation_id) {
    // 模拟数据库读取操作
    size_t index = (thread_id * 1000 + operation_id) % test_data_.size();
    volatile int value = test_data_[index];  // 防止编译器优化
    
    // 模拟一些计算
    for (int i = 0; i < 10; ++i) {
        value = (value * 1103515245 + 12345) & 0x7fffffff;
    }
    
    // 防止编译器优化掉整个操作
    (void)value;
}

void ConcurrencyPerformanceTest::SimulateWriteOperation(int thread_id, int operation_id) {
    // 模拟数据库写入操作
    size_t index = (thread_id * 1000 + operation_id) % test_data_.size();
    
    // 获取锁
    std::lock_guard<std::mutex> lock(*lock_table_[index % lock_table_.size()]);
    
    // 模拟写入操作
    test_data_[index] = operation_id;
    
    // 模拟一些计算
    for (int i = 0; i < 20; ++i) {
        test_data_[index] = (test_data_[index] * 1103515245 + 12345) & 0x7fffffff;
    }
}

void ConcurrencyPerformanceTest::SimulateLockOperation(int thread_id, int operation_id, std::mutex& mutex) {
    // 模拟锁竞争操作
    std::lock_guard<std::mutex> lock(mutex);
    
    // 模拟临界区操作
    int value = operation_id + thread_id;  // 使用thread_id增加一些变化
    for (int i = 0; i < 50; ++i) {
        value = (value * 1103515245 + 12345) & 0x7fffffff;
    }
    
    // 防止编译器优化
    volatile int result = value;
    (void)result;
}

}  // namespace test
}  // namespace sqlcc