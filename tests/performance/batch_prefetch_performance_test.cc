#include "batch_prefetch_performance_test.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <filesystem>

namespace sqlcc {
namespace test {

BatchPrefetchPerformanceTest::BatchPrefetchPerformanceTest() 
    : rng_(std::random_device{}()) {
}

BatchPrefetchPerformanceTest::~BatchPrefetchPerformanceTest() {
    Cleanup();
}

void BatchPrefetchPerformanceTest::RunAllTests() {
    std::cout << "\n=====================================" << std::endl;
    std::cout << "Running Batch & Prefetch Performance Tests" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    // 设置测试环境
    SetupTestEnvironment();
    
    // 运行各种测试
    RunSinglePageReadTest();
    RunBatchPageReadTest();
    RunSinglePagePrefetchTest();
    RunBatchPrefetchTest();
    RunMixedAccessPatternTest();
    RunVaryingBatchSizeTest();
    
    std::cout << "\n=====================================" << std::endl;
    std::cout << "Batch & Prefetch Performance Tests Completed" << std::endl;
    std::cout << "=====================================" << std::endl;
}

void BatchPrefetchPerformanceTest::Cleanup() {
    // 清理测试数据库文件
    if (std::filesystem::exists(test_db_file_)) {
        std::filesystem::remove(test_db_file_);
    }
}

void BatchPrefetchPerformanceTest::SetupTestEnvironment() {
    std::cout << "Setting up test environment..." << std::endl;
    
    // 创建磁盘管理器
    disk_manager_ = std::make_unique<sqlcc::DiskManager>(test_db_file_);
    
    // 创建缓冲池
    buffer_pool_ = std::make_unique<sqlcc::BufferPool>(disk_manager_.get(), pool_size_);
    
    // 初始化一些页面到磁盘
    for (int32_t i = 0; i < working_set_size_; ++i) {
        sqlcc::Page page(i);
        disk_manager_->WritePage(page);
    }
    
    std::cout << "Test environment setup completed." << std::endl;
}

void BatchPrefetchPerformanceTest::RunSinglePageReadTest() {
    std::cout << "\nRunning Single Page Read Test..." << std::endl;
    
    std::vector<TestResult> results;
    
    // 生成随机访问序列
    std::vector<int32_t> page_ids;
    GenerateRandomAccess(page_ids, access_count_, working_set_size_);
    
    // 执行测试
    std::vector<double> latencies;
    auto start_time = GetCurrentTime();
    ExecuteSinglePageAccesses(page_ids, latencies);
    auto end_time = GetCurrentTime();
    
    // 计算结果
    TestResult result;
    result.test_name = "Single Page Read";
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = access_count_;
    result.throughput = CalculateThroughput(access_count_, result.duration);
    
    double avg, p95, p99;
    CalculateLatencies(latencies, avg, p95, p99);
    result.avg_latency = avg;
    result.p95_latency = p95;
    result.p99_latency = p99;
    
    // 打印和保存结果
    PrintResult(result);
    results.push_back(result);
    SaveResultsToFile(results, "single_page_read.csv");
}

void BatchPrefetchPerformanceTest::RunBatchPageReadTest() {
    std::cout << "\nRunning Batch Page Read Test..." << std::endl;
    
    std::vector<TestResult> results;
    
    // 测试不同的批量大小
    for (size_t batch_size : batch_sizes_) {
        if (batch_size == 1) continue; // 跳过单个页面，因为已经在单页测试中测试过
        
        // 生成随机访问序列
        std::vector<int32_t> page_ids;
        GenerateRandomAccess(page_ids, access_count_, working_set_size_);
        
        // 执行测试
        std::vector<double> latencies;
        auto start_time = GetCurrentTime();
        ExecuteBatchPageAccesses(page_ids, latencies, batch_size);
        auto end_time = GetCurrentTime();
        
        // 计算结果
        TestResult result;
        result.test_name = "Batch Page Read (size=" + std::to_string(batch_size) + ")";
        result.duration = CalculateDuration(start_time, end_time);
        result.operations_completed = access_count_;
        result.throughput = CalculateThroughput(access_count_, result.duration);
        
        double avg, p95, p99;
        CalculateLatencies(latencies, avg, p95, p99);
        result.avg_latency = avg;
        result.p95_latency = p95;
        result.p99_latency = p99;
        
        result.custom_metrics["batch_size"] = std::to_string(batch_size);
        
        // 打印结果
        PrintResult(result);
        results.push_back(result);
    }
    
    // 保存结果
    SaveResultsToFile(results, "batch_page_read.csv");
}

void BatchPrefetchPerformanceTest::RunSinglePagePrefetchTest() {
    std::cout << "\nRunning Single Page Prefetch Test..." << std::endl;
    
    std::vector<TestResult> results;
    
    // 生成顺序访问序列（适合预取）
    std::vector<int32_t> page_ids;
    GenerateSequentialAccess(page_ids, access_count_);
    
    // 执行测试
    std::vector<double> latencies;
    auto start_time = GetCurrentTime();
    ExecutePrefetchOperations(page_ids, latencies);
    auto end_time = GetCurrentTime();
    
    // 计算结果
    TestResult result;
    result.test_name = "Single Page Prefetch";
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = access_count_;
    result.throughput = CalculateThroughput(access_count_, result.duration);
    
    double avg, p95, p99;
    CalculateLatencies(latencies, avg, p95, p99);
    result.avg_latency = avg;
    result.p95_latency = p95;
    result.p99_latency = p99;
    
    // 打印和保存结果
    PrintResult(result);
    results.push_back(result);
    SaveResultsToFile(results, "single_page_prefetch.csv");
}

void BatchPrefetchPerformanceTest::RunBatchPrefetchTest() {
    std::cout << "\nRunning Batch Prefetch Test..." << std::endl;
    
    std::vector<TestResult> results;
    
    // 测试不同的批量大小
    for (size_t batch_size : batch_sizes_) {
        if (batch_size == 1) continue; // 跳过单个页面，因为已经在单页测试中测试过
        
        // 生成顺序访问序列（适合预取）
        std::vector<int32_t> page_ids;
        GenerateSequentialAccess(page_ids, access_count_);
        
        // 执行测试
        std::vector<double> latencies;
        auto start_time = GetCurrentTime();
        ExecuteBatchPrefetchOperations(page_ids, latencies, batch_size);
        auto end_time = GetCurrentTime();
        
        // 计算结果
        TestResult result;
        result.test_name = "Batch Prefetch (size=" + std::to_string(batch_size) + ")";
        result.duration = CalculateDuration(start_time, end_time);
        result.operations_completed = access_count_;
        result.throughput = CalculateThroughput(access_count_, result.duration);
        
        double avg, p95, p99;
        CalculateLatencies(latencies, avg, p95, p99);
        result.avg_latency = avg;
        result.p95_latency = p95;
        result.p99_latency = p99;
        
        result.custom_metrics["batch_size"] = std::to_string(batch_size);
        
        // 打印结果
        PrintResult(result);
        results.push_back(result);
    }
    
    // 保存结果
    SaveResultsToFile(results, "batch_prefetch.csv");
}

void BatchPrefetchPerformanceTest::RunMixedAccessPatternTest() {
    std::cout << "\nRunning Mixed Access Pattern Test..." << std::endl;
    
    std::vector<TestResult> results;
    
    // 生成具有局部性的访问序列
    std::vector<int32_t> page_ids;
    GenerateLocalityAccess(page_ids, access_count_, working_set_size_ / 4);
    
    // 测试不同的方法
    // 1. 单个页面读取
    std::vector<double> latencies1;
    auto start_time1 = GetCurrentTime();
    ExecuteSinglePageAccesses(page_ids, latencies1);
    auto end_time1 = GetCurrentTime();
    
    TestResult result1;
    result1.test_name = "Mixed Pattern - Single Page Read";
    result1.duration = CalculateDuration(start_time1, end_time1);
    result1.operations_completed = access_count_;
    result1.throughput = CalculateThroughput(access_count_, result1.duration);
    
    double avg1, p95_1, p99_1;
    CalculateLatencies(latencies1, avg1, p95_1, p99_1);
    result1.avg_latency = avg1;
    result1.p95_latency = p95_1;
    result1.p99_latency = p99_1;
    
    PrintResult(result1);
    results.push_back(result1);
    
    // 2. 批量页面读取（批量大小为8）
    std::vector<double> latencies2;
    auto start_time2 = GetCurrentTime();
    ExecuteBatchPageAccesses(page_ids, latencies2, 8);
    auto end_time2 = GetCurrentTime();
    
    TestResult result2;
    result2.test_name = "Mixed Pattern - Batch Read (size=8)";
    result2.duration = CalculateDuration(start_time2, end_time2);
    result2.operations_completed = access_count_;
    result2.throughput = CalculateThroughput(access_count_, result2.duration);
    
    double avg2, p95_2, p99_2;
    CalculateLatencies(latencies2, avg2, p95_2, p99_2);
    result2.avg_latency = avg2;
    result2.p95_latency = p95_2;
    result2.p99_latency = p99_2;
    
    result2.custom_metrics["batch_size"] = "8";
    
    PrintResult(result2);
    results.push_back(result2);
    
    // 3. 预取操作
    std::vector<double> latencies3;
    auto start_time3 = GetCurrentTime();
    ExecutePrefetchOperations(page_ids, latencies3);
    auto end_time3 = GetCurrentTime();
    
    TestResult result3;
    result3.test_name = "Mixed Pattern - Prefetch";
    result3.duration = CalculateDuration(start_time3, end_time3);
    result3.operations_completed = access_count_;
    result3.throughput = CalculateThroughput(access_count_, result3.duration);
    
    double avg3, p95_3, p99_3;
    CalculateLatencies(latencies3, avg3, p95_3, p99_3);
    result3.avg_latency = avg3;
    result3.p95_latency = p95_3;
    result3.p99_latency = p99_3;
    
    PrintResult(result3);
    results.push_back(result3);
    
    // 保存结果
    SaveResultsToFile(results, "mixed_access_pattern.csv");
}

void BatchPrefetchPerformanceTest::RunVaryingBatchSizeTest() {
    std::cout << "\nRunning Varying Batch Size Test..." << std::endl;
    
    std::vector<TestResult> results;
    
    // 生成随机访问序列
    std::vector<int32_t> page_ids;
    GenerateRandomAccess(page_ids, access_count_, working_set_size_);
    
    // 测试不同的批量大小
    for (size_t batch_size : batch_sizes_) {
        // 执行测试
        std::vector<double> latencies;
        auto start_time = GetCurrentTime();
        ExecuteBatchPageAccesses(page_ids, latencies, batch_size);
        auto end_time = GetCurrentTime();
        
        // 计算结果
        TestResult result;
        result.test_name = "Varying Batch Size";
        result.duration = CalculateDuration(start_time, end_time);
        result.operations_completed = access_count_;
        result.throughput = CalculateThroughput(access_count_, result.duration);
        
        double avg, p95, p99;
        CalculateLatencies(latencies, avg, p95, p99);
        result.avg_latency = avg;
        result.p95_latency = p95;
        result.p99_latency = p99;
        
        result.custom_metrics["batch_size"] = std::to_string(batch_size);
        
        // 打印结果
        PrintResult(result);
        results.push_back(result);
    }
    
    // 保存结果
    SaveResultsToFile(results, "varying_batch_size.csv");
}

void BatchPrefetchPerformanceTest::GenerateSequentialAccess(std::vector<int32_t>& page_ids, size_t count) {
    page_ids.clear();
    page_ids.reserve(count);
    
    for (size_t i = 0; i < count; ++i) {
        page_ids.push_back(i % working_set_size_);
    }
}

void BatchPrefetchPerformanceTest::GenerateRandomAccess(std::vector<int32_t>& page_ids, size_t count, size_t max_page_id) {
    page_ids.clear();
    page_ids.reserve(count);
    
    std::uniform_int_distribution<int32_t> dist(0, max_page_id - 1);
    
    for (size_t i = 0; i < count; ++i) {
        page_ids.push_back(dist(rng_));
    }
}

void BatchPrefetchPerformanceTest::GenerateLocalityAccess(std::vector<int32_t>& page_ids, size_t count, size_t working_set) {
    page_ids.clear();
    page_ids.reserve(count);
    
    std::uniform_int_distribution<int32_t> dist(0, working_set - 1);
    std::uniform_int_distribution<int32_t> region_dist(0, (working_set_size_ / working_set) - 1);
    
    for (size_t i = 0; i < count; ++i) {
        int32_t region = region_dist(rng_);
        int32_t offset = dist(rng_);
        page_ids.push_back(region * working_set + offset);
    }
}

void BatchPrefetchPerformanceTest::ExecuteSinglePageAccesses(const std::vector<int32_t>& page_ids, 
                                                             std::vector<double>& latencies) {
    latencies.clear();
    latencies.reserve(page_ids.size());
    
    for (int32_t page_id : page_ids) {
        auto start = GetCurrentTime();
        auto page = buffer_pool_->FetchPage(page_id);
        auto end = GetCurrentTime();
        
        // 计算延迟（毫秒）
        auto duration = CalculateDuration(start, end);
        latencies.push_back(duration.count());
        
        // 模拟页面使用
        buffer_pool_->UnpinPage(page_id, false);
    }
}

void BatchPrefetchPerformanceTest::ExecuteBatchPageAccesses(const std::vector<int32_t>& page_ids, 
                                                            std::vector<double>& latencies, 
                                                            size_t batch_size) {
    latencies.clear();
    latencies.reserve(page_ids.size());
    
    for (size_t i = 0; i < page_ids.size(); i += batch_size) {
        // 确定当前批次的页面ID
        std::vector<int32_t> batch_ids;
        for (size_t j = 0; j < batch_size && (i + j) < page_ids.size(); ++j) {
            batch_ids.push_back(page_ids[i + j]);
        }
        
        // 执行批量读取
        auto start = GetCurrentTime();
        auto pages = buffer_pool_->BatchFetchPages(batch_ids);
        auto end = GetCurrentTime();
        
        // 计算延迟（毫秒）
        auto duration = CalculateDuration(start, end);
        double avg_latency = static_cast<double>(duration.count()) / batch_ids.size();
        
        // 为每个页面记录相同的延迟
        for (size_t j = 0; j < batch_ids.size(); ++j) {
            latencies.push_back(avg_latency);
        }
        
        // 模拟页面使用
        for (int32_t page_id : batch_ids) {
            buffer_pool_->UnpinPage(page_id, false);
        }
    }
}

void BatchPrefetchPerformanceTest::ExecutePrefetchOperations(const std::vector<int32_t>& page_ids, 
                                                            std::vector<double>& latencies) {
    latencies.clear();
    latencies.reserve(page_ids.size());
    
    for (size_t i = 0; i < page_ids.size(); ++i) {
        // 预取下一个页面
        if (i + 1 < page_ids.size()) {
            buffer_pool_->PrefetchPage(page_ids[i + 1]);
        }
        
        // 读取当前页面
        auto start = GetCurrentTime();
        auto page = buffer_pool_->FetchPage(page_ids[i]);
        auto end = GetCurrentTime();
        
        // 计算延迟（毫秒）
        auto duration = CalculateDuration(start, end);
        latencies.push_back(duration.count());
        
        // 模拟页面使用
        buffer_pool_->UnpinPage(page_ids[i], false);
    }
}

void BatchPrefetchPerformanceTest::ExecuteBatchPrefetchOperations(const std::vector<int32_t>& page_ids, 
                                                                 std::vector<double>& latencies, 
                                                                 size_t batch_size) {
    latencies.clear();
    latencies.reserve(page_ids.size());
    
    for (size_t i = 0; i < page_ids.size(); i += batch_size) {
        // 确定当前批次的页面ID
        std::vector<int32_t> batch_ids;
        for (size_t j = 0; j < batch_size && (i + j) < page_ids.size(); ++j) {
            batch_ids.push_back(page_ids[i + j]);
        }
        
        // 批量预取
        buffer_pool_->BatchPrefetchPages(batch_ids);
        
        // 读取当前批次的页面
        for (int32_t page_id : batch_ids) {
            auto start = GetCurrentTime();
            auto page = buffer_pool_->FetchPage(page_id);
            auto end = GetCurrentTime();
            
            // 计算延迟（毫秒）
            auto duration = CalculateDuration(start, end);
            latencies.push_back(duration.count());
            
            // 模拟页面使用
            buffer_pool_->UnpinPage(page_id, false);
        }
    }
}

}  // namespace test
}  // namespace sqlcc