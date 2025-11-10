#include "buffer_pool_performance_test.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <functional>
#include <random>

namespace sqlcc {
namespace test {

BufferPoolPerformanceTest::BufferPoolPerformanceTest() 
    : current_pool_size_(0), rng_(std::random_device{}()) {
}

BufferPoolPerformanceTest::~BufferPoolPerformanceTest() {
    Cleanup();
}

void BufferPoolPerformanceTest::RunAllTests() {
    std::cout << "\n===== Running Buffer Pool Performance Tests =====" << std::endl;
    
    RunCacheHitRateTest();
    RunLRUEfficiencyTest();
    RunAccessPatternTest();
    RunPoolSizeScalabilityTest();
    
    std::cout << "\n===== All Buffer Pool Performance Tests Completed =====" << std::endl;
}

void BufferPoolPerformanceTest::Cleanup() {
    buffer_pool_.clear();
    dirty_flags_.clear();
    access_times_.clear();
    lru_list_.clear();
    current_pool_size_ = 0;
}

void BufferPoolPerformanceTest::RunCacheHitRateTest() {
    std::cout << "\n--- Running Cache Hit Rate Test ---" << std::endl;
    
    std::vector<TestResult> results;
    
    for (size_t pool_size : pool_sizes_) {
        SetupBufferPool(pool_size);
        
        // 生成随机访问序列
        std::vector<int32_t> page_ids;
        GenerateRandomAccess(page_ids, access_count_, working_set_size_);
        
        // 执行访问并记录延迟
        std::vector<double> latencies;
        size_t hit_count = 0;
<<<<<<< Updated upstream
        
=======
>>>>>>> Stashed changes
        auto start_time = GetCurrentTime();
        ExecutePageAccesses(page_ids, latencies, hit_count);
        auto end_time = GetCurrentTime();
        
        // 计算测试结果
        TestResult result;
        result.test_name = "CacheHitRate_PoolSize" + std::to_string(pool_size);
        result.duration = CalculateDuration(start_time, end_time);
        result.operations_completed = page_ids.size();
        result.throughput = CalculateThroughput(result.operations_completed, result.duration);
        CalculateLatencies(latencies, result.avg_latency, result.p95_latency, result.p99_latency);
        
        // 添加自定义指标
        double hit_rate = CalculateHitRate(hit_count, page_ids.size());
        result.custom_metrics["Hit Rate"] = std::to_string(hit_rate * 100) + "%";
        result.custom_metrics["Pool Size"] = std::to_string(pool_size);
        
        results.push_back(result);
        PrintResult(result);
    }
    
    // 保存结果
    SaveResultsToFile(results, "buffer_pool_cache_hit_rate.csv");
}

void BufferPoolPerformanceTest::RunLRUEfficiencyTest() {
    std::cout << "\n--- Running LRU Efficiency Test ---" << std::endl;
    
    std::vector<TestResult> results;
    
    // 测试不同工作集大小下的LRU效率
    std::vector<size_t> working_set_sizes = {50, 100, 200, 500, 1000};
    size_t fixed_pool_size = 128;
    
    SetupBufferPool(fixed_pool_size);
    
    for (size_t working_set : working_set_sizes) {
        // 生成具有局部性的访问序列
        std::vector<int32_t> page_ids;
        GenerateLocalityAccess(page_ids, access_count_, working_set);
        
        // 执行访问并记录延迟
        std::vector<double> latencies;
        size_t hit_count = 0;
        
        auto start_time = GetCurrentTime();
        ExecutePageAccesses(page_ids, latencies, hit_count);
        auto end_time = GetCurrentTime();
        
        // 计算测试结果
        TestResult result;
        result.test_name = "LRUEfficiency_WorkingSet" + std::to_string(working_set);
        result.duration = CalculateDuration(start_time, end_time);
        result.operations_completed = page_ids.size();
        result.throughput = CalculateThroughput(result.operations_completed, result.duration);
        CalculateLatencies(latencies, result.avg_latency, result.p95_latency, result.p99_latency);
        
        // 添加自定义指标
        double hit_rate = CalculateHitRate(hit_count, page_ids.size());
        result.custom_metrics["Hit Rate"] = std::to_string(hit_rate * 100) + "%";
        result.custom_metrics["Working Set Size"] = std::to_string(working_set);
        
        results.push_back(result);
        PrintResult(result);
    }
    
    // 保存结果
    SaveResultsToFile(results, "buffer_pool_lru_efficiency.csv");
}

void BufferPoolPerformanceTest::RunAccessPatternTest() {
    std::cout << "\n--- Running Access Pattern Test ---" << std::endl;
    
    std::vector<TestResult> results;
    size_t fixed_pool_size = 128;
    SetupBufferPool(fixed_pool_size);
    
    // 测试不同的访问模式
    std::vector<std::pair<std::string, std::function<void(std::vector<int32_t>&, size_t)>>> access_patterns = {
        {"Sequential", [this](std::vector<int32_t>& page_ids, size_t count) { 
            GenerateSequentialAccess(page_ids, count); }},
        {"Random", [this](std::vector<int32_t>& page_ids, size_t count) { 
            GenerateRandomAccess(page_ids, count, working_set_size_); }},
        {"Locality", [this](std::vector<int32_t>& page_ids, size_t count) { 
            GenerateLocalityAccess(page_ids, count, working_set_size_ / 2); }}
    };
    
    for (const auto& pattern : access_patterns) {
        std::vector<int32_t> page_ids;
        pattern.second(page_ids, access_count_);
        
        // 执行访问并记录延迟
        std::vector<double> latencies;
        size_t hit_count = 0;
        
        auto start_time = GetCurrentTime();
        ExecutePageAccesses(page_ids, latencies, hit_count);
        auto end_time = GetCurrentTime();
        
        // 计算测试结果
        TestResult result;
        result.test_name = "AccessPattern_" + pattern.first;
        result.duration = CalculateDuration(start_time, end_time);
        result.operations_completed = page_ids.size();
        result.throughput = CalculateThroughput(result.operations_completed, result.duration);
        CalculateLatencies(latencies, result.avg_latency, result.p95_latency, result.p99_latency);
        
        // 添加自定义指标
        double hit_rate = CalculateHitRate(hit_count, page_ids.size());
        result.custom_metrics["Hit Rate"] = std::to_string(hit_rate * 100) + "%";
        result.custom_metrics["Access Pattern"] = pattern.first;
        
        results.push_back(result);
        PrintResult(result);
    }
    
    // 保存结果
    SaveResultsToFile(results, "buffer_pool_access_pattern.csv");
}

void BufferPoolPerformanceTest::RunPoolSizeScalabilityTest() {
    std::cout << "\n--- Running Pool Size Scalability Test ---" << std::endl;
    
    std::vector<TestResult> results;
    
    // 测试更大的缓冲池大小
    std::vector<size_t> large_pool_sizes = {256, 512, 1024, 2048};
    
    for (size_t pool_size : large_pool_sizes) {
        SetupBufferPool(pool_size);
        
        // 生成随机访问序列
        std::vector<int32_t> page_ids;
        GenerateRandomAccess(page_ids, access_count_ * 2, pool_size * 2);  // 增加访问次数和页面范围
        
        // 执行访问并记录延迟
        std::vector<double> latencies;
        size_t hit_count = 0;
        
        auto start_time = GetCurrentTime();
        ExecutePageAccesses(page_ids, latencies, hit_count);
        auto end_time = GetCurrentTime();
        
        // 计算测试结果
        TestResult result;
        result.test_name = "PoolSizeScalability_" + std::to_string(pool_size);
        result.duration = CalculateDuration(start_time, end_time);
        result.operations_completed = page_ids.size();
        result.throughput = CalculateThroughput(result.operations_completed, result.duration);
        CalculateLatencies(latencies, result.avg_latency, result.p95_latency, result.p99_latency);
        
        // 添加自定义指标
        double hit_rate = CalculateHitRate(hit_count, page_ids.size());
        result.custom_metrics["Hit Rate"] = std::to_string(hit_rate * 100) + "%";
        result.custom_metrics["Pool Size"] = std::to_string(pool_size);
        
        results.push_back(result);
        PrintResult(result);
    }
    
    // 保存结果
    SaveResultsToFile(results, "buffer_pool_size_scalability.csv");
}

void BufferPoolPerformanceTest::SetupBufferPool(size_t pool_size) {
    Cleanup();
    
    current_pool_size_ = pool_size;
    buffer_pool_.resize(pool_size, -1);
    dirty_flags_.resize(pool_size, false);
    access_times_.resize(pool_size, GetCurrentTime());
    lru_list_.reserve(pool_size);
    
    // 初始化LRU列表
    for (size_t i = 0; i < pool_size; ++i) {
        lru_list_.push_back(i);
    }
}

void BufferPoolPerformanceTest::GenerateSequentialAccess(std::vector<int32_t>& page_ids, size_t count) {
    page_ids.clear();
    page_ids.reserve(count);
    
    for (size_t i = 0; i < count; ++i) {
        page_ids.push_back(static_cast<int32_t>(i % working_set_size_));
    }
}

void BufferPoolPerformanceTest::GenerateRandomAccess(std::vector<int32_t>& page_ids, size_t count, size_t max_page_id) {
    page_ids.clear();
    page_ids.reserve(count);
    
    std::uniform_int_distribution<int32_t> dist(0, static_cast<int32_t>(max_page_id) - 1);
    
    for (size_t i = 0; i < count; ++i) {
        page_ids.push_back(dist(rng_));
    }
}

void BufferPoolPerformanceTest::GenerateLocalityAccess(std::vector<int32_t>& page_ids, size_t count, size_t working_set) {
    page_ids.clear();
    page_ids.reserve(count);
    
    std::uniform_int_distribution<int32_t> dist(0, static_cast<int32_t>(working_set) - 1);
    std::uniform_int_distribution<int32_t> locality_dist(-10, 10);  // 局部性范围
    
    int32_t current_page = dist(rng_);
    
    for (size_t i = 0; i < count; ++i) {
        // 80%的概率在当前页面附近访问（局部性）
        if (i % 5 != 0) {
            int32_t offset = locality_dist(rng_);
            current_page = (current_page + offset + static_cast<int32_t>(working_set)) % static_cast<int32_t>(working_set);
        } else {
            // 20%的概率随机跳转到其他页面
            current_page = dist(rng_);
        }
        
        page_ids.push_back(current_page);
    }
}

void BufferPoolPerformanceTest::ExecutePageAccesses(const std::vector<int32_t>& page_ids, 
                                                    std::vector<double>& latencies, 
                                                    size_t& hit_count) {
    latencies.clear();
    latencies.reserve(page_ids.size());
    hit_count = 0;
    
    for (int32_t page_id : page_ids) {
        auto access_start = GetCurrentTime();
        bool hit = SimulatePageAccess(page_id);
        auto access_end = GetCurrentTime();
        
        if (hit) {
            hit_count++;
        }
        
        // 记录延迟（毫秒）
        auto duration = CalculateDuration(access_start, access_end);
        latencies.push_back(static_cast<double>(duration.count()));
    }
}

double BufferPoolPerformanceTest::CalculateHitRate(size_t hit_count, size_t total_accesses) const {
    if (total_accesses == 0) {
        return 0.0;
    }
    return static_cast<double>(hit_count) / static_cast<double>(total_accesses);
}

bool BufferPoolPerformanceTest::SimulatePageAccess(int32_t page_id) {
    // 检查页面是否在缓冲池中
    auto it = std::find(buffer_pool_.begin(), buffer_pool_.end(), page_id);
    
    if (it != buffer_pool_.end()) {
        // 缓存命中
        size_t index = std::distance(buffer_pool_.begin(), it);
        
        // 更新访问时间
        access_times_[index] = GetCurrentTime();
        
        // 更新LRU列表（将访问的页面移到列表末尾）
        auto lru_it = std::find(lru_list_.begin(), lru_list_.end(), index);
        if (lru_it != lru_list_.end()) {
            lru_list_.erase(lru_it);
            lru_list_.push_back(index);
        }
        
        return true;
    } else {
        // 缓存未命中，需要替换页面
        if (current_pool_size_ == 0) {
            return false;
        }
        
        // 使用LRU策略替换页面
        size_t victim_index = lru_list_.front();
        lru_list_.erase(lru_list_.begin());
        
        // 替换页面
        buffer_pool_[victim_index] = page_id;
        dirty_flags_[victim_index] = false;
        access_times_[victim_index] = GetCurrentTime();
        
        // 将新页面添加到LRU列表末尾
        lru_list_.push_back(victim_index);
        
        return false;
    }
}

}  // namespace test
}  // namespace sqlcc