#include "../performance_test_base.h"
#include "../../include/sql_executor.h"
#include "../../include/core/core_database_manager.h"
#include <chrono>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <random>
#include <sstream>

namespace sqlcc {
namespace test {

class DDLConcurrentPerformanceTest {
public:
    DDLConcurrentPerformanceTest();
    ~DDLConcurrentPerformanceTest();

    void RunAllTests();

private:
    void SetupTestEnvironment();
    void Cleanup();
    void RunConcurrentDDLTest(int thread_count);
    void DDLWorker(int thread_id, int thread_count, std::atomic<int>& completed_operations,
                   std::vector<double>& latencies, std::mutex& result_mutex);

    std::unique_ptr<SqlExecutor> sql_executor_;
    std::atomic<int> table_counter_;
    std::mutex sql_mutex_;
    std::mt19937 rng_;
};

DDLConcurrentPerformanceTest::DDLConcurrentPerformanceTest()
    : table_counter_(0), rng_(std::random_device{}()) {
}

DDLConcurrentPerformanceTest::~DDLConcurrentPerformanceTest() {
    Cleanup();
}

void DDLConcurrentPerformanceTest::RunAllTests() {
    std::cout << "=== SQLCC DDL Concurrent Performance Tests ===" << std::endl;

    SetupTestEnvironment();

    std::vector<int> thread_counts = {1, 2, 4, 8, 16, 32};

    for (int thread_count : thread_counts) {
        std::cout << "\n--- Testing with " << thread_count << " threads ---" << std::endl;
        RunConcurrentDDLTest(thread_count);
    }

    std::cout << "\n=== DDL Concurrent Performance Tests Complete ===" << std::endl;
}

void DDLConcurrentPerformanceTest::SetupTestEnvironment() {
    std::cout << "Setting up test environment..." << std::endl;

    // 创建数据库管理器
    auto db_manager = std::make_shared<DatabaseManager>("./test_data", 1024, 16, 64);
    db_manager->Initialize();

    // 创建SQL执行器
    sql_executor_ = std::make_unique<SqlExecutor>(db_manager);

    // 清理可能存在的测试表
    for (int i = 0; i < 100; ++i) {
        try {
            std::string drop_sql = "DROP TABLE IF EXISTS ddl_test_table_" + std::to_string(i);
            sql_executor_->Execute(drop_sql);
        } catch (...) {
            // 忽略删除失败的表
        }
    }

    std::cout << "Test environment setup complete." << std::endl;
}

void DDLConcurrentPerformanceTest::Cleanup() {
    // 清理测试表
    if (sql_executor_) {
        for (int i = 0; i < 100; ++i) {
            try {
                std::string drop_sql = "DROP TABLE IF EXISTS ddl_test_table_" + std::to_string(i);
                sql_executor_->Execute(drop_sql);
            } catch (...) {
                // 忽略清理失败
            }
        }
    }
}

void DDLConcurrentPerformanceTest::RunConcurrentDDLTest(int thread_count) {
    std::cout << "Running DDL concurrent performance test with " << thread_count << " threads..." << std::endl;

    // 重置表计数器
    table_counter_ = 0;

    // 准备线程和结果收集
    std::vector<std::thread> threads;
    std::vector<std::vector<double>> thread_latencies(thread_count);
    std::atomic<int> completed_operations(0);

    auto start_time = std::chrono::high_resolution_clock::now();

    // 启动工作线程
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(&DDLConcurrentPerformanceTest::DDLWorker, this,
                           i, thread_count, std::ref(completed_operations),
                           std::ref(thread_latencies[i]), std::ref(sql_mutex_));
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 收集所有延迟数据
    std::vector<double> all_latencies;
    for (const auto& thread_latency : thread_latencies) {
        all_latencies.insert(all_latencies.end(), thread_latency.begin(), thread_latency.end());
    }

    // 计算统计数据
    double avg_latency = 0, p95_latency = 0, p99_latency = 0, min_latency = 0, max_latency = 0;
    if (!all_latencies.empty()) {
        std::sort(all_latencies.begin(), all_latencies.end());
        avg_latency = std::accumulate(all_latencies.begin(), all_latencies.end(), 0.0) / all_latencies.size();
        min_latency = all_latencies[0];
        max_latency = all_latencies.back();
        p95_latency = all_latencies[static_cast<size_t>(all_latencies.size() * 0.95)];
        p99_latency = all_latencies[static_cast<size_t>(all_latencies.size() * 0.99)];
    }

    double throughput = (completed_operations.load() * 1000.0) / total_duration.count();

    // 输出结果
    std::cout << "Results for " << thread_count << " threads:" << std::endl;
    std::cout << "  Total operations: " << completed_operations.load() << std::endl;
    std::cout << "  Total time: " << total_duration.count() << " ms" << std::endl;
    std::cout << "  Throughput: " << throughput << " ops/sec" << std::endl;
    std::cout << "  Average latency: " << avg_latency << " ms" << std::endl;
    std::cout << "  Min latency: " << min_latency << " ms" << std::endl;
    std::cout << "  Max latency: " << max_latency << " ms" << std::endl;
    std::cout << "  95th percentile latency: " << p95_latency << " ms" << std::endl;
    std::cout << "  99th percentile latency: " << p99_latency << " ms" << std::endl;

    // 保存到文件
    std::ofstream report_file("ddl_concurrent_performance_report.txt", std::ios::app);
    report_file << "Threads: " << thread_count << "\n";
    report_file << "  Operations: " << completed_operations.load() << "\n";
    report_file << "  Duration: " << total_duration.count() << " ms\n";
    report_file << "  Throughput: " << throughput << " ops/sec\n";
    report_file << "  Avg Latency: " << avg_latency << " ms\n";
    report_file << "  Min Latency: " << min_latency << " ms\n";
    report_file << "  Max Latency: " << max_latency << " ms\n";
    report_file << "  P95 Latency: " << p95_latency << " ms\n";
    report_file << "  P99 Latency: " << p99_latency << " ms\n";
    report_file << "  ------------------------\n";
    report_file.close();
}

void DDLConcurrentPerformanceTest::DDLWorker(int thread_id, int thread_count,
                                            std::atomic<int>& completed_operations,
                                            std::vector<double>& latencies,
                                            std::mutex& result_mutex) {
    // 每个线程执行一定数量的DDL操作
    const int operations_per_thread = 50;  // 每个线程执行50个DDL操作

    for (int i = 0; i < operations_per_thread; ++i) {
        auto op_start = std::chrono::high_resolution_clock::now();

        // 获取唯一的表名
        int table_id = table_counter_.fetch_add(1);

        // 随机选择DDL操作类型
        std::uniform_int_distribution<int> op_dist(0, 2);
        int operation_type = op_dist(rng_);

        std::string sql;

        try {
            switch (operation_type) {
                case 0: { // CREATE TABLE
                    std::string table_name = "ddl_test_table_" + std::to_string(table_id);
                    sql = "CREATE TABLE " + table_name + " ("
                          "id INT PRIMARY KEY, "
                          "name VARCHAR(50), "
                          "value INT, "
                          "data TEXT)";
                    break;
                }
                case 1: { // ALTER TABLE
                    std::string table_name = "ddl_test_table_" + std::to_string(table_id % 10); // 使用已存在的表
                    sql = "ALTER TABLE " + table_name + " ADD COLUMN new_col VARCHAR(100)";
                    break;
                }
                case 2: { // DROP TABLE
                    std::string table_name = "ddl_test_table_" + std::to_string(table_id % 10); // 使用已存在的表
                    sql = "DROP TABLE IF EXISTS " + table_name;
                    break;
                }
            }

            // 执行DDL操作（需要互斥访问SQL执行器）
            {
                std::lock_guard<std::mutex> lock(result_mutex);
                sql_executor_->Execute(sql);
            }

            auto op_end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(op_end - op_start);
            double latency_ms = duration.count() / 1000.0;

            latencies.push_back(latency_ms);
            completed_operations.fetch_add(1);

        } catch (const std::exception& e) {
            // 记录失败的操作，但继续执行
            std::cerr << "DDL operation failed in thread " << thread_id << ": " << e.what() << std::endl;
            // 仍然记录延迟
            auto op_end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(op_end - op_start);
            double latency_ms = duration.count() / 1000.0;
            latencies.push_back(latency_ms);
        }

        // 小延迟以避免过于激烈的竞争
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

} // namespace test
} // namespace sqlcc

// 主函数
int main(int argc, char* argv[]) {
    sqlcc::test::DDLConcurrentPerformanceTest test;
    test.RunAllTests();
    return 0;
}
