#include "memory_stress_test.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cstring>

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

}  // namespace test
}  // namespace sqlcc