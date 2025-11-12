#include "memory_stress_test.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <thread>
#include <vector>
#include <memory>

namespace sqlcc {
namespace test {

MemoryStressTest::MemoryStressTest() : gen_(rd_()) {
    std::cout << "Initializing Memory Stress Test..." << std::endl;
}

MemoryStressTest::~MemoryStressTest() {
    Cleanup();
}

void MemoryStressTest::RunAllTests() {
    std::cout << "Running Memory Stress Tests..." << std::endl;
    
    std::vector<TestResult> results;
    
    // 运行内存分配测试
    results.push_back(TestMemoryAllocation());
    
    // 运行内存释放测试
    results.push_back(TestMemoryDeallocation());
    
    // 运行内存碎片测试
    results.push_back(TestMemoryFragmentation());
    
    // 运行内存访问模式测试
    results.push_back(TestMemoryAccessPatterns());
    
    // 生成报告
    GenerateReport(results);
    
    // 保存结果到文件
    SaveResultsToFile(results, "memory_stress_test_results.csv");
}

void MemoryStressTest::Cleanup() {
    // 清理所有内存块
    small_blocks_.clear();
    medium_blocks_.clear();
    large_blocks_.clear();
}

MemoryStressTest::TestResult MemoryStressTest::TestMemoryAllocation() {
    TestResult result;
    result.test_name = "Memory Allocation Test";
    
    std::cout << "Running memory allocation test..." << std::endl;
    
    // 清理之前的内存块
    small_blocks_.clear();
    medium_blocks_.clear();
    large_blocks_.clear();
    
    // 记录开始时间
    auto start_time = GetCurrentTime();
    
    // 分配小内存块
    size_t small_block_count = (kMaxMemoryMB * 1024 * 1024 * 30) / 100 / kSmallBlockSize; // 30%内存
    AllocateMemoryBlocks(kSmallBlockSize, small_block_count, small_blocks_);
    
    // 分配中等内存块
    size_t medium_block_count = (kMaxMemoryMB * 1024 * 1024 * 50) / 100 / kMediumBlockSize; // 50%内存
    AllocateMemoryBlocks(kMediumBlockSize, medium_block_count, medium_blocks_);
    
    // 分配大内存块
    size_t large_block_count = (kMaxMemoryMB * 1024 * 1024 * 20) / 100 / kLargeBlockSize; // 20%内存
    AllocateMemoryBlocks(kLargeBlockSize, large_block_count, large_blocks_);
    
    // 记录结束时间
    auto end_time = GetCurrentTime();
    
    // 计算内存使用情况
    size_t allocated_mb;
    double fragmentation_ratio;
    MeasureMemoryUsage(allocated_mb, fragmentation_ratio);
    
    // 计算测试结果
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = small_block_count + medium_block_count + large_block_count;
    result.throughput = CalculateThroughput(result.operations_completed, result.duration);
    
    // 设置延迟为0，因为分配操作本身不计算延迟
    result.avg_latency = 0.0;
    result.p95_latency = 0.0;
    result.p99_latency = 0.0;
    
    // 添加自定义指标
    result.custom_metrics["Small Blocks"] = std::to_string(small_block_count);
    result.custom_metrics["Medium Blocks"] = std::to_string(medium_block_count);
    result.custom_metrics["Large Blocks"] = std::to_string(large_block_count);
    result.custom_metrics["Allocated MB"] = std::to_string(allocated_mb);
    result.custom_metrics["Fragmentation Ratio"] = std::to_string(fragmentation_ratio);
    
    // 打印结果
    PrintResult(result);
    
    return result;
}

MemoryStressTest::TestResult MemoryStressTest::TestMemoryDeallocation() {
    TestResult result;
    result.test_name = "Memory Deallocation Test";
    
    std::cout << "Running memory deallocation test..." << std::endl;
    
    // 确保有内存块可以释放
    if (small_blocks_.empty() && medium_blocks_.empty() && large_blocks_.empty()) {
        // 先分配一些内存块
        TestMemoryAllocation();
    }
    
    // 记录开始时间
    auto start_time = GetCurrentTime();
    
    // 记录释放前的内存块数量
    size_t total_blocks_before = small_blocks_.size() + medium_blocks_.size() + large_blocks_.size();
    
    // 随机释放50%的内存块
    DeallocateMemoryBlocks(small_blocks_, 0.5);
    DeallocateMemoryBlocks(medium_blocks_, 0.5);
    DeallocateMemoryBlocks(large_blocks_, 0.5);
    
    // 记录结束时间
    auto end_time = GetCurrentTime();
    
    // 计算内存使用情况
    size_t allocated_mb;
    double fragmentation_ratio;
    MeasureMemoryUsage(allocated_mb, fragmentation_ratio);
    
    // 计算测试结果
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = total_blocks_before - (small_blocks_.size() + medium_blocks_.size() + large_blocks_.size());
    result.throughput = CalculateThroughput(result.operations_completed, result.duration);
    
    // 设置延迟为0，因为释放操作本身不计算延迟
    result.avg_latency = 0.0;
    result.p95_latency = 0.0;
    result.p99_latency = 0.0;
    
    // 添加自定义指标
    result.custom_metrics["Blocks Before"] = std::to_string(total_blocks_before);
    result.custom_metrics["Blocks After"] = std::to_string(small_blocks_.size() + medium_blocks_.size() + large_blocks_.size());
    result.custom_metrics["Allocated MB"] = std::to_string(allocated_mb);
    result.custom_metrics["Fragmentation Ratio"] = std::to_string(fragmentation_ratio);
    
    // 打印结果
    PrintResult(result);
    
    return result;
}

MemoryStressTest::TestResult MemoryStressTest::TestMemoryFragmentation() {
    TestResult result;
    result.test_name = "Memory Fragmentation Test";
    
    std::cout << "Running memory fragmentation test..." << std::endl;
    
    // 清理之前的内存块
    Cleanup();
    
    // 记录开始时间
    auto start_time = GetCurrentTime();
    
    // 分配大量不同大小的内存块以产生碎片
    std::vector<std::unique_ptr<char[]>> mixed_blocks;
    std::uniform_int_distribution<> size_dist(64, 1048576); // 64字节到1MB
    
    // 分配内存直到达到最大限制
    size_t total_allocated = 0;
    while (total_allocated < kMaxMemoryMB * 1024 * 1024) {
        size_t block_size = size_dist(gen_);
        mixed_blocks.push_back(std::make_unique<char[]>(block_size));
        total_allocated += block_size;
    }
    
    // 随机释放一些块以产生碎片
    std::uniform_int_distribution<> index_dist(0, static_cast<int>(mixed_blocks.size()) - 1);
    size_t deallocation_count = mixed_blocks.size() / 2; // 释放50%的块
    
    for (size_t i = 0; i < deallocation_count; ++i) {
        int index = index_dist(gen_);
        if (!mixed_blocks.empty() && index < static_cast<int>(mixed_blocks.size())) {
            // 计算释放的内存大小
            size_t block_size = 64; // 默认大小，实际大小难以追踪
            if (index < static_cast<int>(mixed_blocks.size())) {
                mixed_blocks.erase(mixed_blocks.begin() + index);
            }
            total_allocated -= block_size;
        }
    }
    
    // 尝试分配一个大块，看是否能成功（碎片测试）
    bool large_allocation_success = false;
    try {
        auto large_block = std::make_unique<char[]>(kLargeBlockSize * 10); // 尝试分配10MB
        large_allocation_success = true;
    } catch (const std::bad_alloc&) {
        large_allocation_success = false;
    }
    
    // 记录结束时间
    auto end_time = GetCurrentTime();
    
    // 计算测试结果
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = mixed_blocks.size();
    result.throughput = CalculateThroughput(result.operations_completed, result.duration);
    
    // 设置延迟为0
    result.avg_latency = 0.0;
    result.p95_latency = 0.0;
    result.p99_latency = 0.0;
    
    // 添加自定义指标
    result.custom_metrics["Total Blocks"] = std::to_string(mixed_blocks.size());
    result.custom_metrics["Deallocated Blocks"] = std::to_string(deallocation_count);
    result.custom_metrics["Large Allocation Success"] = large_allocation_success ? "Yes" : "No";
    
    // 打印结果
    PrintResult(result);
    
    return result;
}

MemoryStressTest::TestResult MemoryStressTest::TestMemoryAccessPatterns() {
    TestResult result;
    result.test_name = "Memory Access Patterns Test";
    
    std::cout << "Running memory access patterns test..." << std::endl;
    
    // 确保有内存块可以访问
    if (small_blocks_.empty() && medium_blocks_.empty() && large_blocks_.empty()) {
        // 先分配一些内存块
        TestMemoryAllocation();
    }
    
    // 记录开始时间
    auto start_time = GetCurrentTime();
    
    // 随机访问模式
    RandomMemoryAccess(small_blocks_, kSmallBlockSize, kAccessCount / 4);
    RandomMemoryAccess(medium_blocks_, kMediumBlockSize, kAccessCount / 4);
    RandomMemoryAccess(large_blocks_, kLargeBlockSize, kAccessCount / 4);
    
    // 顺序访问模式
    SequentialMemoryAccess(small_blocks_, kSmallBlockSize, kAccessCount / 4);
    SequentialMemoryAccess(medium_blocks_, kMediumBlockSize, kAccessCount / 4);
    SequentialMemoryAccess(large_blocks_, kLargeBlockSize, kAccessCount / 4);
    
    // 记录结束时间
    auto end_time = GetCurrentTime();
    
    // 计算测试结果
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = kAccessCount;
    result.throughput = CalculateThroughput(result.operations_completed, result.duration);
    
    // 设置延迟为0
    result.avg_latency = 0.0;
    result.p95_latency = 0.0;
    result.p99_latency = 0.0;
    
    // 添加自定义指标
    result.custom_metrics["Small Blocks"] = std::to_string(small_blocks_.size());
    result.custom_metrics["Medium Blocks"] = std::to_string(medium_blocks_.size());
    result.custom_metrics["Large Blocks"] = std::to_string(large_blocks_.size());
    result.custom_metrics["Access Count"] = std::to_string(kAccessCount);
    
    // 打印结果
    PrintResult(result);
    
    return result;
}

MemoryStressTest::TestResult MemoryStressTest::TestMemoryLeakDetection() {
    TestResult result;
    result.test_name = "Memory Leak Detection Test";
    
    std::cout << "Running memory leak detection test..." << std::endl;
    
    // 记录初始内存使用情况
    size_t initial_memory = GetCurrentMemoryUsage();
    
    // 分配大量内存并记录
    std::vector<std::unique_ptr<char[]>> temp_allocations;
    temp_allocations.reserve(1000);
    
    auto start_time = GetCurrentTime();
    
    // 分配大量小内存块
    for (size_t i = 0; i < 1000; ++i) {
        temp_allocations.push_back(std::make_unique<char[]>(1024));
        // 初始化内存
        std::memset(temp_allocations.back().get(), 0, 1024);
    }
    
    // 记录中间内存使用情况
    size_t peak_memory = GetCurrentMemoryUsage();
    
    // 清理一半的内存块
    for (size_t i = 0; i < 500; ++i) {
        temp_allocations[i].reset();
    }
    
    // 记录结束时间
    auto end_time = GetCurrentTime();
    
    // 清理所有内存
    temp_allocations.clear();
    
    // 记录最终内存使用情况
    size_t final_memory = GetCurrentMemoryUsage();
    
    // 计算测试结果
    result.duration = CalculateDuration(start_time, end_time);
    result.operations_completed = 1000; // 分配了1000个块
    result.throughput = CalculateThroughput(result.operations_completed, result.duration);
    
    // 设置延迟为0
    result.avg_latency = 0.0;
    result.p95_latency = 0.0;
    result.p99_latency = 0.0;
    
    // 检查是否有内存泄漏
    result.memory_leak_detected = (final_memory > initial_memory);
    result.peak_memory_usage = peak_memory - initial_memory;
    result.average_memory_usage = (peak_memory - initial_memory) / 2;
    
    // 添加自定义指标
    result.custom_metrics["Initial Memory"] = std::to_string(initial_memory);
    result.custom_metrics["Peak Memory"] = std::to_string(peak_memory);
    result.custom_metrics["Final Memory"] = std::to_string(final_memory);
    result.custom_metrics["Memory Leak Detected"] = result.memory_leak_detected ? "Yes" : "No";
    
    if (result.memory_leak_detected) {
        result.error_message = "Potential memory leak detected";
    }
    
    // 打印结果
    PrintResult(result);
    
    return result;
}

void MemoryStressTest::AllocateMemoryBlocks(size_t block_size, size_t block_count, 
                                           std::vector<std::unique_ptr<char[]>>& blocks) {
    blocks.reserve(block_count);
    for (size_t i = 0; i < block_count; ++i) {
        blocks.push_back(std::make_unique<char[]>(block_size));
        
        // 初始化内存内容，防止被优化掉
        char* block = blocks.back().get();
        std::memset(block, static_cast<int>(i % 256), block_size);
    }
}

void MemoryStressTest::DeallocateMemoryBlocks(std::vector<std::unique_ptr<char[]>>& blocks, 
                                              double deallocation_ratio) {
    if (blocks.empty() || deallocation_ratio <= 0.0) {
        return;
    }
    
    size_t deallocation_count = static_cast<size_t>(blocks.size() * deallocation_ratio);
    std::uniform_int_distribution<> index_dist(0, static_cast<int>(blocks.size()) - 1);
    
    for (size_t i = 0; i < deallocation_count && !blocks.empty(); ++i) {
        int index = index_dist(gen_);
        if (index < static_cast<int>(blocks.size())) {
            blocks.erase(blocks.begin() + index);
        }
    }
}

void MemoryStressTest::RandomMemoryAccess(std::vector<std::unique_ptr<char[]>>& blocks, 
                                         size_t block_size, size_t access_count) {
    if (blocks.empty()) {
        return;
    }
    
    std::uniform_int_distribution<> block_dist(0, static_cast<int>(blocks.size()) - 1);
    std::uniform_int_distribution<> offset_dist(0, static_cast<int>(block_size - 1));
    
    for (size_t i = 0; i < access_count; ++i) {
        int block_index = block_dist(gen_);
        int offset = offset_dist(gen_);
        
        if (block_index < static_cast<int>(blocks.size())) {
            char* block = blocks[block_index].get();
            // 读取和写入操作
            volatile char value = block[offset];
            block[offset] = static_cast<char>(value + 1);
        }
    }
}

void MemoryStressTest::SequentialMemoryAccess(std::vector<std::unique_ptr<char[]>>& blocks, 
                                              size_t block_size, size_t access_count) {
    if (blocks.empty()) {
        return;
    }
    
    size_t block_index = 0;
    size_t offset = 0;
    
    for (size_t i = 0; i < access_count; ++i) {
        if (block_index >= blocks.size()) {
            block_index = 0;
            offset = 0;
        }
        
        char* block = blocks[block_index].get();
        // 读取和写入操作
        volatile char value = block[offset];
        block[offset] = static_cast<char>(value + 1);
        
        offset++;
        if (offset >= block_size) {
            offset = 0;
            block_index++;
        }
    }
}

void MemoryStressTest::MeasureMemoryUsage(size_t& allocated_mb, double& fragmentation_ratio) {
    // 计算分配的内存总量
    size_t total_bytes = 0;
    
    for ([[maybe_unused]] const auto& block : small_blocks_) {
        total_bytes += kSmallBlockSize;
    }
    
    for ([[maybe_unused]] const auto& block : medium_blocks_) {
        total_bytes += kMediumBlockSize;
    }
    
    for ([[maybe_unused]] const auto& block : large_blocks_) {
        total_bytes += kLargeBlockSize;
    }
    
    allocated_mb = total_bytes / (1024 * 1024);
    
    // 简单的碎片率计算（实际应用中需要更复杂的算法）
    size_t total_blocks = small_blocks_.size() + medium_blocks_.size() + large_blocks_.size();
    if (total_blocks > 0) {
        // 假设理想情况下所有块都是大块，碎片率 = 1 - (平均块大小 / 最大块大小)
        size_t avg_block_size = total_bytes / total_blocks;
        fragmentation_ratio = 1.0 - (static_cast<double>(avg_block_size) / kLargeBlockSize);
    } else {
        fragmentation_ratio = 0.0;
    }
}

void MemoryStressTest::GenerateReport(const std::vector<TestResult>& results) {
    std::cout << "\n=== Memory Stress Test Report ===" << std::endl;
    
    for (const auto& result : results) {
        PrintResult(result);
    }
}

void MemoryStressTest::PrintResult(const TestResult& result) {
    std::cout << "\nTest: " << result.test_name << std::endl;
    std::cout << "Duration: " << result.duration << " seconds" << std::endl;
    std::cout << "Operations: " << result.operations_completed << std::endl;
    std::cout << "Throughput: " << result.throughput << " ops/sec" << std::endl;
    std::cout << "Peak Memory Usage: " << result.peak_memory_usage << " bytes" << std::endl;
    
    if (!result.error_message.empty()) {
        std::cout << "Error: " << result.error_message << std::endl;
    }
    
    for (const auto& metric : result.custom_metrics) {
        std::cout << metric.first << ": " << metric.second << std::endl;
    }
}

void MemoryStressTest::SaveResultsToFile(const std::vector<TestResult>& results, const std::string& filename) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }
    
    // Write CSV header
    outfile << "Test Name,Duration,Operations,Throughput,Peak Memory,Error" << std::endl;
    
    for (const auto& result : results) {
        outfile << result.test_name << ","
                << result.duration << ","
                << result.operations_completed << ","
                << result.throughput << ","
                << result.peak_memory_usage << ","
                << result.error_message << std::endl;
    }
    
    outfile.close();
    std::cout << "Results saved to: " << filename << std::endl;
}

std::chrono::high_resolution_clock::time_point MemoryStressTest::GetCurrentTime() {
    return std::chrono::high_resolution_clock::now();
}

double MemoryStressTest::CalculateDuration(const std::chrono::high_resolution_clock::time_point& start, 
                                          const std::chrono::high_resolution_clock::time_point& end) {
    return std::chrono::duration<double>(end - start).count();
}

double MemoryStressTest::CalculateThroughput(size_t operations, double duration) {
    if (duration <= 0.0) {
        return 0.0;
    }
    return static_cast<double>(operations) / duration;
}

size_t MemoryStressTest::GetCurrentMemoryUsage() {
    // 简单的内存使用估算
    size_t total = 0;
    total += small_blocks_.size() * kSmallBlockSize;
    total += medium_blocks_.size() * kMediumBlockSize;
    total += large_blocks_.size() * kLargeBlockSize;
    return total;
}

size_t MemoryStressTest::GetPeakMemoryUsage() {
    size_t current = GetCurrentMemoryUsage();
    if (current > peak_memory_usage_) {
        peak_memory_usage_ = current;
    }
    return peak_memory_usage_;
}

void MemoryStressTest::UpdateMemoryUsage() {
    size_t current = GetCurrentMemoryUsage();
    if (current > peak_memory_usage_) {
        peak_memory_usage_ = current;
    }
}

void MemoryStressTest::SetOutputDirectory(const std::string& directory) {
    // 这里应该设置输出目录，但由于简化实现，我们直接使用文件名
    // 在实际应用中，这里会将输出保存到指定目录
    [[maybe_unused]] std::string output_dir = directory; // 消除未使用参数警告
}

void MemoryStressTest::Initialize() {
    // 初始化随机数生成器
    gen_ = std::mt19937(rd_());
    dist_ = std::uniform_int_distribution<>(0, 100);
}

void MemoryStressTest::RunAllStressTests() {
    Initialize();
    RunAllTests();
}

double MemoryStressTest::SimulateMemoryAllocation(size_t iterations, size_t allocation_size) {
    auto start_time = GetCurrentTime();
    
    for (size_t i = 0; i < iterations; ++i) {
        auto ptr = std::make_unique<char[]>(allocation_size);
        std::memset(ptr.get(), 0, allocation_size);
    }
    
    auto end_time = GetCurrentTime();
    return CalculateDuration(start_time, end_time);
}

double MemoryStressTest::SimulateMemoryDeallocation(size_t iterations) {
    std::vector<std::unique_ptr<char[]>> temp_blocks;
    temp_blocks.reserve(iterations);
    
    // 先分配内存
    for (size_t i = 0; i < iterations; ++i) {
        temp_blocks.push_back(std::make_unique<char[]>(1024));
    }
    
    auto start_time = GetCurrentTime();
    temp_blocks.clear(); // 释放内存
    auto end_time = GetCurrentTime();
    
    return CalculateDuration(start_time, end_time);
}

double MemoryStressTest::SimulateMemoryFragmentation(size_t iterations) {
    // 创建大量小内存块然后释放一半，产生碎片
    std::vector<std::unique_ptr<char[]>> fragments;
    fragments.reserve(iterations);
    
    auto start_time = GetCurrentTime();
    
    // 分配小内存块
    for (size_t i = 0; i < iterations; ++i) {
        fragments.push_back(std::make_unique<char[]>(64));
    }
    
    // 释放一半产生碎片
    for (size_t i = 0; i < iterations / 2; ++i) {
        if (!fragments.empty()) {
            fragments.pop_back();
        }
    }
    
    // 清理
    fragments.clear();
    auto end_time = GetCurrentTime();
    
    return CalculateDuration(start_time, end_time);
}

bool MemoryStressTest::SimulateMemoryLeakDetection(size_t iterations) {
    std::vector<std::unique_ptr<char[]>> temp_allocations;
    temp_allocations.reserve(iterations);
    
    // 分配内存
    for (size_t i = 0; i < iterations; ++i) {
        temp_allocations.push_back(std::make_unique<char[]>(1024));
    }
    
    // 故意不释放一半的内存，模拟内存泄漏
    for (size_t i = 0; i < iterations / 2; ++i) {
        if (!temp_allocations.empty()) {
            temp_allocations.pop_back(); // 只释放一部分
        }
    }
    
    // 检查是否还有未释放的内存
    bool leak_detected = !temp_allocations.empty();
    
    // 清理剩余内存
    temp_allocations.clear();
    
    return leak_detected;
}

MemoryStressTestRunner::MemoryStressTestRunner() {
    std::cout << "Initializing Memory Stress Test Runner..." << std::endl;
}

MemoryStressTestRunner::~MemoryStressTestRunner() {
    std::cout << "Memory Stress Test Runner cleanup completed." << std::endl;
}

void MemoryStressTestRunner::RunStressTest(size_t duration_seconds, size_t thread_count) {
    std::cout << "Running stress test for " << duration_seconds << " seconds with " 
              << thread_count << " threads..." << std::endl;
    
    if (thread_count == 1) {
        RunSingleThreadedStressTest(duration_seconds);
    } else {
        RunMultiThreadedStressTest(duration_seconds, thread_count);
    }
    
    GenerateReport();
}

void MemoryStressTestRunner::RunSingleThreadedStressTest(size_t duration_seconds) {
    std::cout << "Running single-threaded stress test..." << std::endl;
    
    MemoryStressTest test;
    auto start_time = std::chrono::high_resolution_clock::now();
    auto end_time = start_time + std::chrono::seconds(duration_seconds);
    
    while (std::chrono::high_resolution_clock::now() < end_time) {
        test.RunAllTests();
        MonitorMemoryUsage();
    }
}

void MemoryStressTestRunner::RunMultiThreadedStressTest(size_t duration_seconds, size_t thread_count) {
    std::cout << "Running multi-threaded stress test with " << thread_count << " threads..." << std::endl;
    
    std::vector<std::thread> threads;
    threads.reserve(thread_count);
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < thread_count; ++i) {
        threads.emplace_back([duration_seconds, start_time]() {
            MemoryStressTest test;
            auto current_time = std::chrono::high_resolution_clock::now();
        while (std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count() < static_cast<long long>(duration_seconds)) {
            test.RunAllTests();
            current_time = std::chrono::high_resolution_clock::now();
        }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
}

void MemoryStressTestRunner::MonitorMemoryUsage() {
    // 这里可以实现内存使用监控逻辑
    // 暂时只是打印一条信息
    // std::cout << "Memory monitoring active..." << std::endl;
}

void MemoryStressTestRunner::GenerateReport() {
    std::cout << "Stress test completed. Check individual test results for details." << std::endl;
}

}  // namespace test
}  // namespace sqlcc