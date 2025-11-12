#ifndef MEMORY_STRESS_TEST_H
#define MEMORY_STRESS_TEST_H

#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <map>
#include <random>

namespace sqlcc {
namespace test {

class MemoryStressTest {
public:
    struct TestResult {
        std::string test_name;
        double duration;
        size_t operations_completed;
        double throughput;
        double avg_latency;
        double p95_latency;
        double p99_latency;
        size_t peak_memory_usage;
        size_t average_memory_usage;
        bool memory_leak_detected;
        std::string error_message;
        std::map<std::string, std::string> custom_metrics;
    };

    MemoryStressTest();
    ~MemoryStressTest();

    void SetOutputDirectory(const std::string& directory);
    void RunAllTests();
    void Cleanup();

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