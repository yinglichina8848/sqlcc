#include "disk_io_performance_test.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <filesystem>
#include <thread>
#include <vector>
#include <future>

namespace sqlcc {
namespace test {

DiskIOPerformanceTest::DiskIOPerformanceTest() 
    : test_file_path_("test_io_file.dat"),
      test_file_size_mb_(100),
      page_sizes_({4096, 8192, 16384}),  // 4KB, 8KB, 16KB
      page_count_(0),
      test_data_(),
      rng_(std::random_device{}()) {
    
    // 初始化测试数据
    size_t max_page_size = *std::max_element(page_sizes_.begin(), page_sizes_.end());
    test_data_.resize(max_page_size);
    
    // 填充测试数据
    for (size_t i = 0; i < max_page_size; ++i) {
        test_data_[i] = static_cast<char>(i % 256);
    }
}

DiskIOPerformanceTest::~DiskIOPerformanceTest() {
    Cleanup();
}

void DiskIOPerformanceTest::RunAllTests() {
    std::cout << "\n===== Running Disk I/O Performance Tests =====" << std::endl;
    
    RunSequentialReadWriteTest();
    RunRandomReadWriteTest();
    RunVaryingPageSizeTest();
    RunConcurrentIOTest();
    
    std::cout << "\n===== All Disk I/O Performance Tests Completed =====" << std::endl;
}

void DiskIOPerformanceTest::Cleanup() {
    CleanupTestFile();
    test_data_.clear();
}

void DiskIOPerformanceTest::RunSequentialReadWriteTest() {
    std::cout << "\n--- Running Sequential Read/Write Test ---" << std::endl;
    
    std::vector<TestResult> results;
    
    for (size_t page_size : page_sizes_) {
        // 准备测试文件
        PrepareTestFile(test_file_size_mb_);
        page_count_ = (test_file_size_mb_ * 1024 * 1024) / page_size;
        
        // 顺序读取测试
        std::vector<double> read_latencies;
        auto read_start = GetCurrentTime();
        ExecuteSequentialReads(page_count_, page_size, read_latencies);
        auto read_end = GetCurrentTime();
        
        TestResult read_result;
        read_result.test_name = "SequentialRead_PageSize" + std::to_string(page_size);
        read_result.duration = CalculateDuration(read_start, read_end);
        read_result.operations_completed = page_count_;
        read_result.throughput = CalculateThroughput(page_count_, read_result.duration);
        CalculateLatencies(read_latencies, read_result.avg_latency, read_result.p95_latency, read_result.p99_latency);
        
        // 添加自定义指标
        double read_throughput_mb = (page_count_ * page_size) / (1024.0 * 1024.0) / 
                                   (read_result.duration.count() / 1000.0);
        read_result.custom_metrics["Throughput(MB/s)"] = std::to_string(read_throughput_mb);
        read_result.custom_metrics["Page Size"] = std::to_string(page_size);
        
        results.push_back(read_result);
        PrintResult(read_result);
        
        // 顺序写入测试
        std::vector<double> write_latencies;
        auto write_start = GetCurrentTime();
        ExecuteSequentialWrites(page_count_, page_size, write_latencies);
        auto write_end = GetCurrentTime();
        
        TestResult write_result;
        write_result.test_name = "SequentialWrite_PageSize" + std::to_string(page_size);
        write_result.duration = CalculateDuration(write_start, write_end);
        write_result.operations_completed = page_count_;
        write_result.throughput = CalculateThroughput(page_count_, write_result.duration);
        CalculateLatencies(write_latencies, write_result.avg_latency, write_result.p95_latency, write_result.p99_latency);
        
        // 添加自定义指标
        double write_throughput_mb = (page_count_ * page_size) / (1024.0 * 1024.0) / 
                                    (write_result.duration.count() / 1000.0);
        write_result.custom_metrics["Throughput(MB/s)"] = std::to_string(write_throughput_mb);
        write_result.custom_metrics["Page Size"] = std::to_string(page_size);
        
        results.push_back(write_result);
        PrintResult(write_result);
        
        // 清理测试文件
        CleanupTestFile();
    }
    
    // 保存结果
    SaveResultsToFile(results, "disk_io_sequential.csv");
}

void DiskIOPerformanceTest::RunRandomReadWriteTest() {
    std::cout << "\n--- Running Random Read/Write Test ---" << std::endl;
    
    std::vector<TestResult> results;
    size_t fixed_page_size = 4096;  // 使用4KB页面大小
    
    // 准备测试文件
    PrepareTestFile(test_file_size_mb_);
    page_count_ = (test_file_size_mb_ * 1024 * 1024) / fixed_page_size;
    
    // 随机读取测试
    std::vector<double> read_latencies;
    auto read_start = GetCurrentTime();
    ExecuteRandomReads(page_count_, fixed_page_size, read_latencies);
    auto read_end = GetCurrentTime();
    
    TestResult read_result;
    read_result.test_name = "RandomRead";
    read_result.duration = CalculateDuration(read_start, read_end);
    read_result.operations_completed = page_count_;
    read_result.throughput = CalculateThroughput(page_count_, read_result.duration);
    CalculateLatencies(read_latencies, read_result.avg_latency, read_result.p95_latency, read_result.p99_latency);
    
    // 添加自定义指标
    double read_throughput_mb = (page_count_ * fixed_page_size) / (1024.0 * 1024.0) / 
                               (read_result.duration.count() / 1000.0);
    read_result.custom_metrics["Throughput(MB/s)"] = std::to_string(read_throughput_mb);
    read_result.custom_metrics["Page Size"] = std::to_string(fixed_page_size);
    
    results.push_back(read_result);
    PrintResult(read_result);
    
    // 随机写入测试
    std::vector<double> write_latencies;
    auto write_start = GetCurrentTime();
    ExecuteRandomWrites(page_count_, fixed_page_size, write_latencies);
    auto write_end = GetCurrentTime();
    
    TestResult write_result;
    write_result.test_name = "RandomWrite";
    write_result.duration = CalculateDuration(write_start, write_end);
    write_result.operations_completed = page_count_;
    write_result.throughput = CalculateThroughput(page_count_, write_result.duration);
    CalculateLatencies(write_latencies, write_result.avg_latency, write_result.p95_latency, write_result.p99_latency);
    
    // 添加自定义指标
    double write_throughput_mb = (page_count_ * fixed_page_size) / (1024.0 * 1024.0) / 
                                (write_result.duration.count() / 1000.0);
    write_result.custom_metrics["Throughput(MB/s)"] = std::to_string(write_throughput_mb);
    write_result.custom_metrics["Page Size"] = std::to_string(fixed_page_size);
    
    results.push_back(write_result);
    PrintResult(write_result);
    
    // 清理测试文件
    CleanupTestFile();
    
    // 保存结果
    SaveResultsToFile(results, "disk_io_random.csv");
}

void DiskIOPerformanceTest::RunVaryingPageSizeTest() {
    std::cout << "\n--- Running Varying Page Size Test ---" << std::endl;
    
    std::vector<TestResult> results;
    
    for (size_t page_size : page_sizes_) {
        // 准备测试文件
        PrepareTestFile(test_file_size_mb_);
        page_count_ = (test_file_size_mb_ * 1024 * 1024) / page_size;
        
        // 混合读写测试（70%读取，30%写入）
        std::vector<double> latencies;
        size_t read_count = page_count_ * 0.7;
        size_t write_count = page_count_ * 0.3;
        
        auto start_time = GetCurrentTime();
        
        // 先执行读取
        ExecuteSequentialReads(read_count, page_size, latencies);
        // 再执行写入
        ExecuteSequentialWrites(write_count, page_size, latencies);
        
        auto end_time = GetCurrentTime();
        
        TestResult result;
        result.test_name = "MixedIO_PageSize" + std::to_string(page_size);
        result.duration = CalculateDuration(start_time, end_time);
        result.operations_completed = read_count + write_count;
        result.throughput = CalculateThroughput(result.operations_completed, result.duration);
        CalculateLatencies(latencies, result.avg_latency, result.p95_latency, result.p99_latency);
        
        // 添加自定义指标
        double throughput_mb = (result.operations_completed * page_size) / (1024.0 * 1024.0) / 
                             (result.duration.count() / 1000.0);
        result.custom_metrics["Throughput(MB/s)"] = std::to_string(throughput_mb);
        result.custom_metrics["Page Size"] = std::to_string(page_size);
        result.custom_metrics["Read Ratio"] = "70%";
        result.custom_metrics["Write Ratio"] = "30%";
        
        results.push_back(result);
        PrintResult(result);
        
        // 清理测试文件
        CleanupTestFile();
    }
    
    // 保存结果
    SaveResultsToFile(results, "disk_io_varying_page_size.csv");
}

void DiskIOPerformanceTest::RunConcurrentIOTest() {
    std::cout << "\n--- Running Concurrent I/O Test ---" << std::endl;
    
    std::vector<TestResult> results;
    size_t fixed_page_size = 4096;  // 使用4KB页面大小
    
    // 准备测试文件
    PrepareTestFile(test_file_size_mb_);
    page_count_ = (test_file_size_mb_ * 1024 * 1024) / fixed_page_size;
    
    // 测试不同线程数
    std::vector<size_t> thread_counts = {1, 2, 4, 8};
    
    for (size_t thread_count : thread_counts) {
        std::vector<std::future<void>> futures;
        std::vector<std::vector<double>> thread_latencies(thread_count);
        
        auto start_time = GetCurrentTime();
        
        // 启动多个线程并发执行读取
        for (size_t i = 0; i < thread_count; ++i) {
            futures.push_back(std::async(std::launch::async, [this, i, thread_count, fixed_page_size, &thread_latencies]() {
                size_t pages_per_thread = page_count_ / thread_count;
                size_t start_page = i * pages_per_thread;
                
                // 每个线程读取自己的页面范围
                for (size_t j = 0; j < pages_per_thread; ++j) {
                    int32_t page_id = static_cast<int32_t>(start_page + j);
                    std::vector<char> buffer(fixed_page_size);
                    
                    auto read_start = GetCurrentTime();
                    SimulatePageRead(page_id, buffer.data(), fixed_page_size);
                    auto read_end = GetCurrentTime();
                    
                    auto duration = CalculateDuration(read_start, read_end);
                    thread_latencies[i].push_back(static_cast<double>(duration.count()));
                }
            }));
        }
        
        // 等待所有线程完成
        for (auto& future : futures) {
            future.wait();
        }
        
        auto end_time = GetCurrentTime();
        
        // 合并所有线程的延迟数据
        std::vector<double> all_latencies;
        for (const auto& latencies : thread_latencies) {
            all_latencies.insert(all_latencies.end(), latencies.begin(), latencies.end());
        }
        
        TestResult result;
        result.test_name = "ConcurrentIO_Threads" + std::to_string(thread_count);
        result.duration = CalculateDuration(start_time, end_time);
        result.operations_completed = page_count_;
        result.throughput = CalculateThroughput(page_count_, result.duration);
        CalculateLatencies(all_latencies, result.avg_latency, result.p95_latency, result.p99_latency);
        
        // 添加自定义指标
        double throughput_mb = (page_count_ * fixed_page_size) / (1024.0 * 1024.0) / 
                             (result.duration.count() / 1000.0);
        result.custom_metrics["Throughput(MB/s)"] = std::to_string(throughput_mb);
        result.custom_metrics["Thread Count"] = std::to_string(thread_count);
        
        results.push_back(result);
        PrintResult(result);
    }
    
    // 清理测试文件
    CleanupTestFile();
    
    // 保存结果
    SaveResultsToFile(results, "disk_io_concurrent.csv");
}

void DiskIOPerformanceTest::PrepareTestFile(size_t file_size_mb) {
    // 清理已存在的测试文件
    CleanupTestFile();
    
    // 创建测试文件
    std::ofstream file(test_file_path_, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to create test file: " << test_file_path_ << std::endl;
        return;
    }
    
    // 填充文件内容
    size_t file_size_bytes = file_size_mb * 1024 * 1024;
    size_t buffer_size = test_data_.size();
    
    for (size_t i = 0; i < file_size_bytes; i += buffer_size) {
        size_t write_size = std::min(buffer_size, file_size_bytes - i);
        file.write(test_data_.data(), write_size);
    }
    
    file.close();
}

void DiskIOPerformanceTest::CleanupTestFile() {
    if (std::filesystem::exists(test_file_path_)) {
        std::filesystem::remove(test_file_path_);
    }
}

bool DiskIOPerformanceTest::SimulatePageRead(int32_t page_id, char* buffer, size_t page_size) {
    std::ifstream file(test_file_path_, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // 计算页面偏移量
    std::streampos offset = static_cast<std::streampos>(page_id) * page_size;
    file.seekg(offset);
    
    // 读取页面数据
    file.read(buffer, page_size);
    
    file.close();
    return true;
}

bool DiskIOPerformanceTest::SimulatePageWrite(int32_t page_id, const char* buffer, size_t page_size) {
    std::fstream file(test_file_path_, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        return false;
    }
    
    // 计算页面偏移量
    std::streampos offset = static_cast<std::streampos>(page_id) * page_size;
    file.seekp(offset);
    
    // 写入页面数据
    file.write(buffer, page_size);
    
    file.close();
    return true;
}

void DiskIOPerformanceTest::ExecuteSequentialReads(size_t page_count, size_t page_size, 
                                                   std::vector<double>& latencies) {
    latencies.clear();
    latencies.reserve(page_count);
    
    std::vector<char> buffer(page_size);
    
    for (size_t i = 0; i < page_count; ++i) {
        int32_t page_id = static_cast<int32_t>(i);
        
        auto read_start = GetCurrentTime();
        SimulatePageRead(page_id, buffer.data(), page_size);
        auto read_end = GetCurrentTime();
        
        auto duration = CalculateDuration(read_start, read_end);
        latencies.push_back(static_cast<double>(duration.count()));
    }
}

void DiskIOPerformanceTest::ExecuteSequentialWrites(size_t page_count, size_t page_size, 
                                                    std::vector<double>& latencies) {
    latencies.clear();
    latencies.reserve(page_count);
    
    // 使用测试数据作为写入内容
    const char* write_data = test_data_.data();
    
    for (size_t i = 0; i < page_count; ++i) {
        int32_t page_id = static_cast<int32_t>(i);
        
        auto write_start = GetCurrentTime();
        SimulatePageWrite(page_id, write_data, page_size);
        auto write_end = GetCurrentTime();
        
        auto duration = CalculateDuration(write_start, write_end);
        latencies.push_back(static_cast<double>(duration.count()));
    }
}

void DiskIOPerformanceTest::ExecuteRandomReads(size_t page_count, size_t page_size, 
                                              std::vector<double>& latencies) {
    latencies.clear();
    latencies.reserve(page_count);
    
    std::vector<char> buffer(page_size);
    std::uniform_int_distribution<int32_t> dist(0, static_cast<int32_t>(page_count) - 1);
    
    for (size_t i = 0; i < page_count; ++i) {
        int32_t page_id = dist(rng_);
        
        auto read_start = GetCurrentTime();
        SimulatePageRead(page_id, buffer.data(), page_size);
        auto read_end = GetCurrentTime();
        
        auto duration = CalculateDuration(read_start, read_end);
        latencies.push_back(static_cast<double>(duration.count()));
    }
}

void DiskIOPerformanceTest::ExecuteRandomWrites(size_t page_count, size_t page_size, 
                                               std::vector<double>& latencies) {
    latencies.clear();
    latencies.reserve(page_count);
    
    // 使用测试数据作为写入内容
    const char* write_data = test_data_.data();
    std::uniform_int_distribution<int32_t> dist(0, static_cast<int32_t>(page_count) - 1);
    
    for (size_t i = 0; i < page_count; ++i) {
        int32_t page_id = dist(rng_);
        
        auto write_start = GetCurrentTime();
        SimulatePageWrite(page_id, write_data, page_size);
        auto write_end = GetCurrentTime();
        
        auto duration = CalculateDuration(write_start, write_end);
        latencies.push_back(static_cast<double>(duration.count()));
    }
}

}  // namespace test
}  // namespace sqlcc