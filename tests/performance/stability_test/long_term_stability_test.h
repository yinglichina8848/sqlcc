#ifndef LONG_TERM_STABILITY_TEST_H
#define LONG_TERM_STABILITY_TEST_H

#include <vector>
#include <string>
#include <chrono>
#include <atomic>
#include <thread>

namespace sqlcc {
namespace test {

class LongTermStabilityTest {
public:
    struct StabilityTestResult {
        std::string test_name;
        double duration;
        size_t operations_completed;
        double throughput;
        size_t error_count;
        size_t warning_count;
        double success_rate;
        std::string error_message;
    };

    struct TestConfig {
        std::chrono::seconds test_duration{300};  // 默认5分钟
        size_t thread_count{4};
        size_t warmup_duration_seconds{30};
        size_t sampling_interval_seconds{10};
        std::string output_file{"long_term_stability_results.csv"};
        bool enable_memory_monitoring{true};
        bool enable_cpu_monitoring{true};
        bool enable_disk_io_monitoring{true};
        std::string output_directory{"./build/performance_results"};
    };

    LongTermStabilityTest();
    ~LongTermStabilityTest();

    void SetOutputDirectory(const std::string& directory);
    void SetConfig(const TestConfig& config);
    void RunAllTests();
    void Cleanup();

private:
    static constexpr size_t kDefaultDuration = 60; // 60 seconds
    static constexpr size_t kMaxConcurrentThreads = 10;
    static constexpr size_t kMaxOperationsPerSecond = 1000;
    static constexpr double kMaxErrorRate = 0.01; // 1% error rate

    std::atomic<bool> test_running_;
    std::atomic<size_t> total_operations_;
    std::atomic<size_t> error_count_;
    std::atomic<size_t> warning_count_;
    std::vector<std::thread> worker_threads_;
    TestConfig config_;

    void Initialize();
    void RunAllStabilityTests();
    
    StabilityTestResult TestContinuousOperations();
    StabilityTestResult TestMemoryStability();
    StabilityTestResult TestResourceCleanup();
    StabilityTestResult TestErrorRecovery();
    
    void RunContinuousOperations(size_t duration_seconds);
    void RunMemoryStabilityTest(size_t duration_seconds);
    void RunResourceCleanupTest(size_t duration_seconds);
    void RunErrorRecoveryTest(size_t duration_seconds);
    
    bool SimulateDatabaseOperation(size_t operation_id);
    bool SimulateMemoryAllocation(size_t size);
    bool SimulateResourceCleanup();
    bool SimulateErrorRecovery();
    
    void WorkerThread(size_t duration_seconds);
    void MonitorThread(size_t duration_seconds);
    
    void GenerateReport(const std::vector<StabilityTestResult>& results);
    void PrintResult(const StabilityTestResult& result);
    void SaveResultsToFile(const std::vector<StabilityTestResult>& results, const std::string& filename);
    
    std::chrono::high_resolution_clock::time_point GetCurrentTime();
    double CalculateDuration(const std::chrono::high_resolution_clock::time_point& start, 
                            const std::chrono::high_resolution_clock::time_point& end);
    double CalculateThroughput(size_t operations, double duration);
    double CalculateSuccessRate(size_t operations, size_t errors);
    
    bool ShouldStopTest();
    void IncrementOperations();
    void IncrementErrors();
    void IncrementWarnings();
};

} // namespace test
} // namespace sqlcc

#endif // LONG_TERM_STABILITY_TEST_H