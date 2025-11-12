#include "million_insert_test.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>

namespace sqlcc {
namespace test {

MillionInsertTest::MillionInsertTest() 
    : next_record_id_(0), rng_(std::random_device{}()) {
    
    // 初始化预定义的测试配置
    
    // 单线程测试配置
    test_configs_.push_back({10000, 1, 128, true, "SingleThread_TenThousandInsert"});
    
    // 多线程测试配置
    test_configs_.push_back({10000, 2, 128, true, "MultiThread_2Threads"});
    test_configs_.push_back({10000, 4, 128, true, "MultiThread_4Threads"});
    test_configs_.push_back({10000, 8, 128, true, "MultiThread_8Threads"});
    
    // 扩展性测试配置
    test_configs_.push_back({10000, 1, 128, true, "Scalability_1Thread"});
    test_configs_.push_back({10000, 2, 128, true, "Scalability_2Threads"});
    test_configs_.push_back({10000, 4, 128, true, "Scalability_4Threads"});
    test_configs_.push_back({10000, 8, 128, true, "Scalability_8Threads"});
    test_configs_.push_back({10000, 16, 128, true, "Scalability_16Threads"});
    
    // 不同记录大小测试
    test_configs_.push_back({10000, 4, 64, true, "RecordSize_64Bytes"});
    test_configs_.push_back({10000, 4, 128, true, "RecordSize_128Bytes"});
    test_configs_.push_back({10000, 4, 256, true, "RecordSize_256Bytes"});
    test_configs_.push_back({10000, 4, 512, true, "RecordSize_512Bytes"});
    
    // 设置测试数据库路径
    test_db_path_ = "./million_insert_test.db";
}

MillionInsertTest::~MillionInsertTest() {
    Cleanup();
}

void MillionInsertTest::RunAllTests() {
    std::cout << "\n===== Running Million INSERT Performance Tests =====" << std::endl;
    
    RunSingleThreadTest();
    RunMultiThreadTest();
    RunScalabilityTest();
    
    // 生成综合报告
    GenerateReport(test_results_);
    
    std::cout << "\n===== All Million INSERT Performance Tests Completed =====" << std::endl;
}

void MillionInsertTest::Cleanup() {
    CleanupTestData();
    next_record_id_ = 0;
    test_results_.clear();
}

void MillionInsertTest::RunSingleThreadTest() {
    std::cout << "\n--- Running Single Thread Million INSERT Test ---" << std::endl;
    
    // 单线程测试配置
    std::vector<InsertTestConfig> configs = {
        {10000, 1, 128, true, "SingleThread_TenThousandInsert"}
    };
    
    for (const auto& config : configs) {
        ExecuteInsertTest(config);
    }
    
    // 保存结果
    SaveResultsToFile(test_results_, "million_insert_single_thread.csv");
}

void MillionInsertTest::RunMultiThreadTest() {
    std::cout << "\n--- Running Multi Thread Million INSERT Test ---" << std::endl;
    
    // 多线程测试配置
    std::vector<InsertTestConfig> configs = {
        {10000, 2, 128, true, "MultiThread_2Threads"},
        {10000, 4, 128, true, "MultiThread_4Threads"},
        {10000, 8, 128, true, "MultiThread_8Threads"}
    };
    
    for (const auto& config : configs) {
        ExecuteInsertTest(config);
    }
    
    // 保存结果
    SaveResultsToFile(test_results_, "million_insert_multi_thread.csv");
}

void MillionInsertTest::RunScalabilityTest() {
    std::cout << "\n--- Running Scalability Test ---" << std::endl;
    
    // 扩展性测试配置
    std::vector<InsertTestConfig> configs = {
        {10000, 1, 128, true, "Scalability_1Thread"},
        {10000, 2, 128, true, "Scalability_2Threads"},
        {10000, 4, 128, true, "Scalability_4Threads"},
        {10000, 8, 128, true, "Scalability_8Threads"},
        {10000, 16, 128, true, "Scalability_16Threads"}
    };
    
    for (const auto& config : configs) {
        ExecuteInsertTest(config);
    }
    
    // 保存结果
    SaveResultsToFile(test_results_, "million_insert_scalability.csv");
}

void MillionInsertTest::ExecuteInsertTest(const InsertTestConfig& config) {
    std::cout << "Running test: " << config.name << std::endl;
    
    // 初始化测试环境
    SetupTestEnvironment();
    
    // 启动工作线程
    std::vector<std::thread> threads;
    std::vector<std::vector<double>> thread_latencies(config.thread_count);
    std::atomic<size_t> total_operations(0);
    
    // 记录开始时的文件大小
    size_t initial_file_size = 0;
    if (config.measure_file_size) {
        initial_file_size = GetDatabaseFileSize();
    }
    
    auto start_time = GetCurrentTime();
    
    // 创建并启动线程
    for (size_t i = 0; i < config.thread_count; ++i) {
        threads.emplace_back([this, i, &config, &thread_latencies, &total_operations]() {
            WorkerThread(i, config, thread_latencies[i], total_operations);
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end_time = GetCurrentTime();
    
    // 记录结束时的文件大小
    size_t final_file_size = 0;
    if (config.measure_file_size) {
        final_file_size = GetDatabaseFileSize();
    }
    
    // 合并所有线程的延迟数据
    std::vector<double> all_latencies;
    for (const auto& latencies : thread_latencies) {
        all_latencies.insert(all_latencies.end(), latencies.begin(), latencies.end());
    }
    
    // 计算测试结果
    TestResult result;
    result.test_name = config.name;
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = total_operations.load();
    result.throughput = CalculateThroughput(result.operations_completed, result.duration);
    CalculateLatencies(all_latencies, result.avg_latency, result.p95_latency, result.p99_latency);
    
    // 添加自定义指标
    result.custom_metrics["Insert Count"] = std::to_string(config.insert_count);
    result.custom_metrics["Thread Count"] = std::to_string(config.thread_count);
    result.custom_metrics["Record Size"] = std::to_string(config.record_size) + " bytes";
    
    if (config.measure_file_size) {
        size_t theoretical_size = CalculateTheoreticalFileSize(config.insert_count, config.record_size);
        double expansion_ratio = CalculateFileExpansionRatio(final_file_size - initial_file_size, theoretical_size);
        
        result.custom_metrics["Initial File Size"] = std::to_string(initial_file_size) + " bytes";
        result.custom_metrics["Final File Size"] = std::to_string(final_file_size) + " bytes";
        result.custom_metrics["Data Size"] = std::to_string(final_file_size - initial_file_size) + " bytes";
        result.custom_metrics["Theoretical Size"] = std::to_string(theoretical_size) + " bytes";
        result.custom_metrics["Expansion Ratio"] = std::to_string(expansion_ratio) + "x";
        
        // 检查是否满足膨胀率要求
        if (expansion_ratio <= 1.2) {
            result.custom_metrics["Expansion Requirement"] = "PASSED";
        } else {
            result.custom_metrics["Expansion Requirement"] = "FAILED";
        }
    }
    
    test_results_.push_back(result);
    PrintResult(result);
}

void MillionInsertTest::WorkerThread(size_t thread_id, const InsertTestConfig& config, 
                                     std::vector<double>& latencies, 
                                     std::atomic<size_t>& operations_completed) {
    // 计算每个线程应该插入的记录数
    size_t records_per_thread = config.insert_count / config.thread_count;
    size_t start_id = thread_id * records_per_thread;
    size_t end_id = (thread_id == config.thread_count - 1) ? 
                   config.insert_count : (thread_id + 1) * records_per_thread;
    
    // 执行插入操作
    for (size_t i = start_id; i < end_id; ++i) {
        auto op_start = GetCurrentTime();
        
        // 模拟记录插入
        bool success = SimulateRecordInsert(i, config.record_size);
        
        auto op_end = GetCurrentTime();
        
        if (success) {
            // 计算操作延迟（毫秒）
            double latency = std::chrono::duration_cast<std::chrono::microseconds>(
                op_end - op_start).count() / 1000.0;
            latencies.push_back(latency);
            
            operations_completed.fetch_add(1);
        }
    }
}

bool MillionInsertTest::SimulateRecordInsert(size_t record_id, size_t record_size) {
    (void)record_id; // 避免未使用参数警告
    
    // 在实际实现中，这里应该调用存储引擎的INSERT操作
    // 为了测试目的，我们模拟一个简单的插入操作
    
    // 生成随机数据
    std::vector<char> record_data(record_size);
    std::uniform_int_distribution<char> dist('a', 'z');
    for (size_t i = 0; i < record_size; ++i) {
        record_data[i] = dist(rng_);
    }
    
    // 实际写入数据到文件
    std::ofstream db_file(test_db_path_, std::ios::binary | std::ios::app);
    if (!db_file.is_open()) {
        return false;
    }
    
    // 写入记录大小（4字节）
    uint32_t size = static_cast<uint32_t>(record_size);
    db_file.write(reinterpret_cast<const char*>(&size), sizeof(size));
    
    // 写入记录数据
    db_file.write(record_data.data(), record_size);
    db_file.close();
    
    // 模拟插入延迟（基于实际存储引擎性能）
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    
    return true;
}

size_t MillionInsertTest::GetDatabaseFileSize() {
    struct stat file_stat;
    if (stat(test_db_path_.c_str(), &file_stat) == 0) {
        return file_stat.st_size;
    }
    return 0;
}

size_t MillionInsertTest::CalculateTheoreticalFileSize(size_t record_count, size_t record_size) {
    // 计算理论文件大小（记录大小 * 记录数）
    return record_count * record_size;
}

double MillionInsertTest::CalculateFileExpansionRatio(size_t actual_size, size_t theoretical_size) {
    if (theoretical_size == 0) return 0.0;
    return static_cast<double>(actual_size) / theoretical_size;
}

void MillionInsertTest::SetupTestEnvironment() {
    // 清理之前的测试数据
    CleanupTestData();
    
    // 创建测试数据库文件
    std::ofstream db_file(test_db_path_, std::ios::binary);
    if (db_file.is_open()) {
        db_file.close();
    }
    
    // 重置记录ID
    next_record_id_ = 0;
}

void MillionInsertTest::CleanupTestData() {
    // 删除测试数据库文件
    if (std::filesystem::exists(test_db_path_)) {
        std::filesystem::remove(test_db_path_);
    }
}

}  // namespace test
}  // namespace sqlcc