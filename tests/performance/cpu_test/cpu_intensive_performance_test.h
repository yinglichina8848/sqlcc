#ifndef CPU_INTENSIVE_PERFORMANCE_TEST_H
#define CPU_INTENSIVE_PERFORMANCE_TEST_H

#include <vector>
#include <string>
#include <map>
#include <chrono>
#include "performance_test_base.h"
#include "sql_executor.h"

namespace sqlcc {
namespace test {

class CpuIntensivePerformanceTest : public PerformanceTestBase {
public:
    struct TestResult {
        std::string test_name;
        double duration;
        size_t operations_completed;
        double throughput;
        double avg_latency;
        double p95_latency;
        double p99_latency;
        std::map<std::string, std::string> custom_metrics;
    };

    CpuIntensivePerformanceTest();
    ~CpuIntensivePerformanceTest();

    void SetOutputDirectory(const std::string& directory);
    void RunAllTests();
    
    // 测试设置和清理方法
    void SetUp();
    void TearDown();

private:
    static constexpr size_t kDataSize = 10000;
    static constexpr size_t kTreeSize = 1000;
    static constexpr size_t kSearchKeyCount = 100;
    static constexpr size_t kLogEntryCount = 1000;
    static constexpr size_t kDefaultIterations = 100;

    std::vector<int> int_data_;
    std::vector<std::string> string_data_;
    std::vector<std::pair<int, std::string>> tree_data_;
    std::vector<int> search_keys_;
    std::vector<std::string> log_data_;
    std::string output_directory_;
    
    // SQL执行器
    SqlExecutor* sql_executor_;

    void GenerateTestData();
    void Cleanup();
    
    TestResult TestQueryProcessing();
    TestResult TestIndexOperations();
    TestResult TestTransactionProcessing();
    
    double SimulateDataSorting(std::vector<int>& data, size_t iterations);
    double SimulateHashCalculation(const std::vector<std::string>& data, size_t iterations);
    double SimulateBPlusTreeSearch(const std::vector<std::pair<int, std::string>>& tree_data, 
                                   const std::vector<int>& search_keys, size_t iterations);
    double SimulateTransactionLogging(const std::vector<std::string>& log_data, size_t iterations);
    double SimulateLockManagement(size_t lock_count, size_t iterations);
    
    void GenerateReport(const std::vector<TestResult>& results) const;
    void PrintResult(const TestResult& result) const;
    void SaveResultsToFile(const std::vector<TestResult>& results, const std::string& filename) const;
    
    std::chrono::high_resolution_clock::time_point GetCurrentTime();
    double CalculateDuration(const std::chrono::high_resolution_clock::time_point& start, 
                            const std::chrono::high_resolution_clock::time_point& end);
    double CalculateThroughput(size_t operations, double duration);
    void CalculateLatencies(const std::vector<double>& latencies, double& avg, double& p95, double& p99);
};

} // namespace test
} // namespace sqlcc

#endif // CPU_INTENSIVE_PERFORMANCE_TEST_H