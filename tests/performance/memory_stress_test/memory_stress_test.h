#ifndef MEMORY_STRESS_TEST_H
#define MEMORY_STRESS_TEST_H

#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <map>
#include <random>
#include "sql_executor.h"
#include "performance_test_base.h"

namespace sqlcc {
namespace test {

class MemoryStressTest : public PerformanceTestBase {
public:
    // 使用基类的TestResult结构体
    
    MemoryStressTest();
    ~MemoryStressTest();

    void SetUp();
    void TearDown();
    void RunAllTests() override;
    void Cleanup() override;
    
    // 内存相关指标
    // 注意：这些指标将存储在TestResult中

private:
    static constexpr size_t kDefaultIterations = 1000;
    static constexpr size_t kMaxMemoryMB = 512; // 最大内存限制
    static constexpr size_t kMaxMemoryAllocation = 1024 * 1024 * 100; // 100MB
    static constexpr size_t kSmallAllocationSize = 1024; // 1KB
    static constexpr size_t kMediumAllocationSize = 1024 * 10; // 10KB
    static constexpr size_t kLargeAllocationSize = 1024 * 100; // 100KB
    
    static constexpr size_t kSmallBlockSize = 1024; // 1KB
    static constexpr size_t kMediumBlockSize = 1024 * 10; // 10KB  
    static constexpr size_t kLargeBlockSize = 1024 * 100; // 100KB
    static constexpr size_t kAccessCount = 10000;
    
    // SQL执行器
    SqlExecutor* sql_executor_;

    std::vector<std::unique_ptr<char[]>> allocated_memory_;
    std::vector<std::unique_ptr<char[]>> small_blocks_;
    std::vector<std::unique_ptr<char[]>> medium_blocks_;
    std::vector<std::unique_ptr<char[]>> large_blocks_;
    size_t current_memory_usage_;
    size_t peak_memory_usage_;
    std::random_device rd_;
    std::mt19937 gen_;
    std::uniform_int_distribution<> dist_;

    void Initialize();
    void RunAllStressTests();
    
    TestResult TestMemoryAllocationStress();
    TestResult TestMemoryDeallocationStress();
    TestResult TestMemoryFragmentationStress();
    
    double SimulateMemoryAllocation(size_t iterations, size_t allocation_size);
    double SimulateMemoryDeallocation(size_t iterations);
    double SimulateMemoryFragmentation(size_t iterations);
    bool SimulateMemoryLeakDetection(size_t iterations);
    
    TestResult TestMemoryAllocation();
    TestResult TestMemoryDeallocation();
    TestResult TestMemoryFragmentation();
    TestResult TestMemoryLeakDetection();
    TestResult TestMemoryAccessPatterns();
    
    void GenerateReport(const std::vector<TestResult>& results);
    void PrintResult(const TestResult& result);
    void SaveResultsToFile(const std::vector<TestResult>& results, const std::string& filename);
    
    std::chrono::high_resolution_clock::time_point GetCurrentTime();
    double CalculateDuration(const std::chrono::high_resolution_clock::time_point& start, 
                            const std::chrono::high_resolution_clock::time_point& end);
    double CalculateThroughput(size_t operations, double duration);
    size_t GetCurrentMemoryUsage();
    size_t GetPeakMemoryUsage();
    void UpdateMemoryUsage();
    
    void AllocateMemoryBlocks(size_t block_size, size_t block_count, 
                             std::vector<std::unique_ptr<char[]>>& blocks);
    void DeallocateMemoryBlocks(std::vector<std::unique_ptr<char[]>>& blocks, 
                                double deallocation_ratio);
    void RandomMemoryAccess(std::vector<std::unique_ptr<char[]>>& blocks, 
                           size_t block_size, size_t access_count);
    void SequentialMemoryAccess(std::vector<std::unique_ptr<char[]>>& blocks, 
                               size_t block_size, size_t access_count);
    void MeasureMemoryUsage(size_t& allocated_mb, double& fragmentation_ratio);
};


class MemoryStressTestRunner {
public:
    MemoryStressTestRunner();
    ~MemoryStressTestRunner();
    
    void RunStressTest(size_t duration_seconds, size_t thread_count);
    
private:
    void RunSingleThreadedStressTest(size_t duration_seconds);
    void RunMultiThreadedStressTest(size_t duration_seconds, size_t thread_count);
    void MonitorMemoryUsage();
    void GenerateReport();
};

} // namespace test
} // namespace sqlcc

#endif // MEMORY_STRESS_TEST_H