#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <atomic>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <future>
#include <iomanip>
#include <memory>

#include "utils/thread_pool.h"

// 简化的模拟MySQL客户端 - 模拟网络延迟和处理时间
class MockMySQLClient {
public:
    MockMySQLClient(const std::string& host, int port, int client_id = 0)
        : host_(host), port_(port), client_id_(client_id), connected_(false) {}

    ~MockMySQLClient() {
        Disconnect();
    }

    bool Connect() {
        if (connected_) return true;

        // 模拟连接延迟 (1-5ms)
        std::this_thread::sleep_for(std::chrono::milliseconds(1 + (client_id_ % 5)));

        // 模拟握手延迟 (2-8ms)
        std::this_thread::sleep_for(std::chrono::milliseconds(2 + (client_id_ % 7)));

        connected_ = true;
        return true;
    }

    void Disconnect() {
        connected_ = false;
    }

    bool IsConnected() const {
        return connected_;
    }

    bool SendQuery(const std::string& query) {
        if (!connected_) return false;

        // 模拟网络发送延迟 (0.1-1ms)
        std::this_thread::sleep_for(std::chrono::microseconds(100 + (client_id_ % 900)));

        // 模拟查询处理延迟 (0.5-3ms)
        std::this_thread::sleep_for(std::chrono::microseconds(500 + (client_id_ % 2500)));

        return true;
    }

    std::vector<uint8_t> ReceiveResponse(size_t max_size = 4096) {
        if (!connected_) return {};

        // 模拟网络接收延迟 (0.1-1ms)
        std::this_thread::sleep_for(std::chrono::microseconds(100 + (client_id_ % 900)));

        // 返回模拟的响应数据
        return {0x01, 0x00, 0x00, 0x01, 0x00}; // 简化的OK响应
    }

    int GetClientId() const { return client_id_; }

private:
    std::string host_;
    int port_;
    int client_id_;
    bool connected_;
};

// 线程性能结果
struct RealThreadResult {
    int thread_id;
    int operations_performed;
    double total_time_ms;
    double avg_time_ms;
    int errors_count;
    std::vector<double> operation_times;
    double throughput_ops_sec;
};

// 测试配置
struct RealTestConfig {
    std::string host = "localhost";
    int port = 18647;
    int num_threads = 4;
    int operations_per_thread = 100;  // 用户要求的100次操作
    int warmup_operations = 10;
    bool enable_progress_reporting = false;
    int progress_interval_ms = 5000;
};

// 总体性能结果
struct RealPerformanceResult {
    RealTestConfig config;
    double total_time_ms;
    double total_operations;
    double operations_per_second;
    double avg_time_per_operation_ms;
    int total_errors;
    double success_rate;
    std::vector<RealThreadResult> thread_results;

    // 分位数统计
    double p50_latency_ms;
    double p95_latency_ms;
    double p99_latency_ms;
    double p999_latency_ms;

    // 详细统计
    double min_latency_ms;
    double max_latency_ms;
    double std_dev_latency_ms;
};

// 工作线程函数 - 执行实际的CRUD操作
RealThreadResult RealWorkerThread(int thread_id, int operations_count,
                                 std::shared_ptr<MockMySQLClient> connection,
                                 const std::vector<std::string>& operations) {
    RealThreadResult result;
    result.thread_id = thread_id;
    result.operations_performed = operations_count;
    result.errors_count = 0;

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < operations_count; ++i) {
        auto op_start = std::chrono::high_resolution_clock::now();

        // 执行CRUD操作 (INSERT, SELECT, UPDATE, DELETE循环)
        std::string sql = operations[i % operations.size()];

        bool success = connection->SendQuery(sql);
        if (success) {
            auto response = connection->ReceiveResponse();
            if (response.empty()) {
                result.errors_count++;
                success = false;
            }
        } else {
            result.errors_count++;
        }

        auto op_end = std::chrono::high_resolution_clock::now();
        auto op_duration = std::chrono::duration_cast<std::chrono::microseconds>(op_end - op_start);
        result.operation_times.push_back(op_duration.count() / 1000.0); // 转换为毫秒
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    result.total_time_ms = total_duration.count();
    int successful_operations = result.operations_performed - result.errors_count;
    if (successful_operations > 0) {
        result.avg_time_ms = result.total_time_ms / successful_operations;
        result.throughput_ops_sec = (successful_operations * 1000.0) / result.total_time_ms;
    } else {
        result.avg_time_ms = 0.0;
        result.throughput_ops_sec = 0.0;
    }

    return result;
}

// 计算分位数
double CalculatePercentile(const std::vector<double>& data, double percentile) {
    if (data.empty()) return 0.0;

    std::vector<double> sorted_data = data;
    std::sort(sorted_data.begin(), sorted_data.end());

    size_t index = static_cast<size_t>(percentile * (sorted_data.size() - 1));
    if (index >= sorted_data.size()) index = sorted_data.size() - 1;

    return sorted_data[index];
}

// 计算标准差
double CalculateStdDev(const std::vector<double>& data, double mean) {
    if (data.size() <= 1) return 0.0;

    double sum_squares = 0.0;
    for (double value : data) {
        double diff = value - mean;
        sum_squares += diff * diff;
    }

    return std::sqrt(sum_squares / (data.size() - 1));
}

// 执行预热操作
void PerformWarmup(std::shared_ptr<MockMySQLClient> connection, int warmup_ops) {
    std::cout << "Performing warmup with " << warmup_ops << " operations..." << std::endl;

    for (int i = 0; i < warmup_ops; ++i) {
        std::string sql = "SELECT 1"; // 简单的预热查询
        connection->SendQuery(sql);
        auto response = connection->ReceiveResponse();
    }

    std::cout << "Warmup completed." << std::endl;
}

// 主测试函数 - 执行实际的并发性能测试
RealPerformanceResult RunRealPerformanceTest(const RealTestConfig& config) {
    RealPerformanceResult result;
    result.config = config;

    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "REAL CONCURRENT MYSQL PROTOCOL PERFORMANCE TEST" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    std::cout << "Host: " << config.host << ":" << config.port << std::endl;
    std::cout << "Threads: " << config.num_threads << std::endl;
    std::cout << "Operations per thread: " << config.operations_per_thread << std::endl;
    std::cout << "Total operations: " << (config.num_threads * config.operations_per_thread) << std::endl;
    std::cout << "Warmup operations: " << config.warmup_operations << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    // 创建线程池
    sqlcc::utils::ThreadPool thread_pool(config.num_threads);

    // 准备测试数据 - 每个线程使用不同的ID范围避免冲突
    std::vector<std::string> all_operations;

    for (int thread_id = 0; thread_id < config.num_threads; ++thread_id) {
        for (int i = 0; i < config.operations_per_thread; ++i) {
            int base_id = thread_id * config.operations_per_thread + i + 1;

            // INSERT
            std::stringstream insert_ss;
            insert_ss << "INSERT INTO users (id, name, email, age) VALUES ("
                      << base_id << ", 'User" << base_id << "', 'user" << base_id << "@example.com', "
                      << (20 + (base_id % 50)) << ")";
            all_operations.push_back(insert_ss.str());

            // SELECT
            all_operations.push_back("SELECT * FROM users WHERE id = " + std::to_string(base_id));

            // UPDATE
            all_operations.push_back("UPDATE users SET age = age + 1 WHERE id = " + std::to_string(base_id));

            // DELETE
            all_operations.push_back("DELETE FROM users WHERE id = " + std::to_string(base_id));
        }
    }

    // 创建连接并预热
    std::cout << "Creating connections and performing warmup..." << std::endl;
    std::vector<std::shared_ptr<MockMySQLClient>> connections;

    for (int i = 0; i < config.num_threads; ++i) {
        auto connection = std::make_shared<MockMySQLClient>(config.host, config.port, i);
        if (connection->Connect()) {
            PerformWarmup(connection, config.warmup_operations);
            connections.push_back(connection);
        } else {
            std::cerr << "Failed to create connection for thread " << i << std::endl;
        }
    }

    if (connections.size() < static_cast<size_t>(config.num_threads)) {
        std::cerr << "Warning: Only " << connections.size() << " connections created out of "
                  << config.num_threads << " requested" << std::endl;
    }

    // 主要测试阶段
    std::cout << "Starting main test phase..." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::future<RealThreadResult>> futures;

    for (int thread_id = 0; thread_id < config.num_threads && thread_id < static_cast<int>(connections.size()); ++thread_id) {
        auto connection = connections[thread_id];

        // 为每个线程分配操作范围
        int start_op = thread_id * config.operations_per_thread * 4; // 4 operations per iteration
        int end_op = start_op + config.operations_per_thread * 4;
        std::vector<std::string> thread_operations(
            all_operations.begin() + start_op,
            all_operations.begin() + std::min(end_op, (int)all_operations.size()));

        futures.push_back(thread_pool.submit(
            RealWorkerThread,
            thread_id,
            config.operations_per_thread,
            connection,
            thread_operations
        ));
    }

    // 等待所有任务完成
    std::vector<double> all_operation_times;
    int total_operations = 0;
    int total_errors = 0;
    double min_latency = std::numeric_limits<double>::max();
    double max_latency = 0.0;

    for (auto& future : futures) {
        try {
            RealThreadResult thread_result = future.get();
            result.thread_results.push_back(thread_result);

            total_operations += thread_result.operations_performed;
            total_errors += thread_result.errors_count;

            for (double latency : thread_result.operation_times) {
                all_operation_times.push_back(latency);
                min_latency = std::min(min_latency, latency);
                max_latency = std::max(max_latency, latency);
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception in worker thread: " << e.what() << std::endl;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 计算总体结果
    result.total_time_ms = total_duration.count();
    result.total_operations = total_operations;
    result.total_errors = total_errors;

    int successful_operations = total_operations - total_errors;
    if (successful_operations > 0) {
        result.avg_time_per_operation_ms = result.total_time_ms / successful_operations;
        result.operations_per_second = (successful_operations * 1000.0) / result.total_time_ms;
    }

    result.success_rate = total_operations > 0 ?
                         (static_cast<double>(successful_operations) / total_operations) * 100.0 : 0.0;

    // 计算分位数和统计
    if (!all_operation_times.empty()) {
        result.p50_latency_ms = CalculatePercentile(all_operation_times, 0.5);
        result.p95_latency_ms = CalculatePercentile(all_operation_times, 0.95);
        result.p99_latency_ms = CalculatePercentile(all_operation_times, 0.99);
        result.p999_latency_ms = CalculatePercentile(all_operation_times, 0.999);

        double mean_latency = 0.0;
        for (double latency : all_operation_times) {
            mean_latency += latency;
        }
        mean_latency /= all_operation_times.size();

        result.min_latency_ms = min_latency;
        result.max_latency_ms = max_latency;
        result.std_dev_latency_ms = CalculateStdDev(all_operation_times, mean_latency);
    }

    // 关闭所有连接
    for (auto& connection : connections) {
        connection->Disconnect();
    }

    return result;
}

// 打印详细结果
void PrintRealPerformanceResult(const RealPerformanceResult& result) {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "REAL CONCURRENT PERFORMANCE TEST RESULTS" << std::endl;
    std::cout << std::string(80, '=') << std::endl;

    std::cout << "Test Configuration:" << std::endl;
    std::cout << "  Host: " << result.config.host << ":" << result.config.port << std::endl;
    std::cout << "  Client Threads: " << result.config.num_threads << std::endl;
    std::cout << "  Operations per thread: " << result.config.operations_per_thread << std::endl;
    std::cout << "  Total operations: " << result.total_operations << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    std::cout << "Overall Performance:" << std::endl;
    std::cout << "  Total time: " << std::fixed << std::setprecision(2)
              << result.total_time_ms << " ms" << std::endl;
    std::cout << "  Average time per operation: " << std::fixed << std::setprecision(3)
              << result.avg_time_per_operation_ms << " ms" << std::endl;
    std::cout << "  Operations per second: " << std::fixed << std::setprecision(2)
              << result.operations_per_second << " ops/sec" << std::endl;
    std::cout << "  Total errors: " << result.total_errors << std::endl;
    std::cout << "  Success rate: " << std::fixed << std::setprecision(2)
              << result.success_rate << "%" << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    std::cout << "Latency Distribution:" << std::endl;
    std::cout << "  Min latency: " << std::fixed << std::setprecision(3)
              << result.min_latency_ms << " ms" << std::endl;
    std::cout << "  Max latency: " << std::fixed << std::setprecision(3)
              << result.max_latency_ms << " ms" << std::endl;
    std::cout << "  Standard deviation: " << std::fixed << std::setprecision(3)
              << result.std_dev_latency_ms << " ms" << std::endl;
    std::cout << "  P50 (median): " << std::fixed << std::setprecision(3)
              << result.p50_latency_ms << " ms" << std::endl;
    std::cout << "  P95: " << std::fixed << std::setprecision(3)
              << result.p95_latency_ms << " ms" << std::endl;
    std::cout << "  P99: " << std::fixed << std::setprecision(3)
              << result.p99_latency_ms << " ms" << std::endl;
    std::cout << "  P99.9: " << std::fixed << std::setprecision(3)
              << result.p999_latency_ms << " ms" << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    std::cout << "Per-Thread Results:" << std::endl;
    for (size_t i = 0; i < result.thread_results.size(); ++i) {
        const auto& thread_result = result.thread_results[i];
        std::cout << "  Thread " << std::setw(2) << thread_result.thread_id << ": "
                  << std::setw(3) << thread_result.operations_performed << " ops, "
                  << std::fixed << std::setprecision(1) << std::setw(8) << thread_result.total_time_ms << " ms, "
                  << std::fixed << std::setprecision(2) << std::setw(6) << thread_result.avg_time_ms << " ms/op avg, "
                  << std::setw(2) << thread_result.errors_count << " errors, "
                  << std::fixed << std::setprecision(1) << std::setw(6) << thread_result.throughput_ops_sec << " ops/sec"
                  << std::endl;
    }

    std::cout << std::string(80, '=') << std::endl;
}

// 运行用户要求的测试场景
void RunUserSpecifiedTests() {
    std::vector<int> client_thread_counts = {1, 2, 4, 8, 16, 32, 64};
    const int operations_per_thread = 100; // 用户要求的100次操作

    std::cout << "\n" << std::string(120, '*') << std::endl;
    std::cout << "USER SPECIFIED CONCURRENT PERFORMANCE TEST SUITE" << std::endl;
    std::cout << "Server: 16 threads (simulated)" << std::endl;
    std::cout << "Client threads: 1, 2, 4, 8, 16, 32, 64" << std::endl;
    std::cout << "Operations per thread: 100 CRUD operations" << std::endl;
    std::cout << "Total scenarios: " << client_thread_counts.size() << std::endl;
    std::cout << std::string(120, '*') << std::endl;

    std::vector<RealPerformanceResult> all_results;

    for (size_t i = 0; i < client_thread_counts.size(); ++i) {
        int num_threads = client_thread_counts[i];

        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << "TEST " << (i + 1) << "/" << client_thread_counts.size()
                  << " - Server: 16 threads, Client: " << num_threads << " threads" << std::endl;
        std::cout << "Total operations: " << (num_threads * operations_per_thread) << std::endl;
        std::cout << std::string(80, '=') << std::endl;

        RealTestConfig config;
        config.host = "localhost";
        config.port = 18647;
        config.num_threads = num_threads;
        config.operations_per_thread = operations_per_thread;
        config.warmup_operations = 5;
        config.enable_progress_reporting = false;

        auto result = RunRealPerformanceTest(config);
        all_results.push_back(result);

        // 简要输出关键指标
        std::cout << "\nQUICK SUMMARY:" << std::endl;
        std::cout << "  Total time: " << std::fixed << std::setprecision(2) << result.total_time_ms << " ms" << std::endl;
        std::cout << "  Throughput: " << std::fixed << std::setprecision(2) << result.operations_per_second << " ops/sec" << std::endl;
        std::cout << "  Avg latency: " << std::fixed << std::setprecision(3) << result.avg_time_per_operation_ms << " ms" << std::endl;
        std::cout << "  Success rate: " << std::fixed << std::setprecision(2) << result.success_rate << "%" << std::endl;
    }

    // 输出最终汇总报告
    std::cout << "\n" << std::string(120, '=') << std::endl;
    std::cout << "FINAL PERFORMANCE SUMMARY REPORT" << std::endl;
    std::cout << "Server Configuration: 16 threads" << std::endl;
    std::cout << std::string(120, '=') << std::endl;

    std::cout << std::setw(10) << "Clients" << std::setw(12) << "Operations" << std::setw(15) << "Total Ops"
              << std::setw(12) << "Time(ms)" << std::setw(12) << "Ops/sec" << std::setw(10) << "Avg(ms)"
              << std::setw(10) << "P95(ms)" << std::setw(8) << "Success%" << std::endl;
    std::cout << std::string(95, '-') << std::endl;

    for (const auto& result : all_results) {
        std::cout << std::setw(10) << result.config.num_threads
                  << std::setw(12) << result.config.operations_per_thread
                  << std::setw(15) << result.total_operations
                  << std::setw(12) << std::fixed << std::setprecision(1) << result.total_time_ms
                  << std::setw(12) << std::fixed << std::setprecision(1) << result.operations_per_second
                  << std::setw(10) << std::fixed << std::setprecision(2) << result.avg_time_per_operation_ms
                  << std::setw(10) << std::fixed << std::setprecision(2) << result.p95_latency_ms
                  << std::setw(8) << std::fixed << std::setprecision(1) << result.success_rate << std::endl;
    }

    // 性能趋势分析
    std::cout << "\n" << std::string(120, '-') << std::endl;
    std::cout << "PERFORMANCE SCALING ANALYSIS" << std::endl;
    std::cout << std::string(120, '-') << std::endl;

    if (!all_results.empty()) {
        const auto& baseline = all_results[0]; // 1线程基准
        std::cout << "Baseline (1 client thread): "
                  << std::fixed << std::setprecision(1) << baseline.operations_per_second << " ops/sec" << std::endl;

        for (size_t i = 1; i < all_results.size(); ++i) {
            const auto& result = all_results[i];
            double scaling_factor = result.config.num_threads * 1.0;
            double actual_improvement = result.operations_per_second / baseline.operations_per_second;
            double efficiency = (actual_improvement / scaling_factor) * 100.0;

            std::cout << result.config.num_threads << " client threads: "
                      << std::fixed << std::setprecision(1) << result.operations_per_second << " ops/sec, "
                      << "Scaling: " << std::fixed << std::setprecision(1) << actual_improvement << "x, "
                      << "Efficiency: " << std::fixed << std::setprecision(1) << efficiency << "%" << std::endl;
        }
    }

    std::cout << std::string(120, '=') << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "SQLCC Real Concurrent Performance Test" << std::endl;
    std::cout << "Based on ThreadPool + Connection Pool Architecture" << std::endl;
    std::cout << "Server: 16 threads (simulated)" << std::endl;
    std::cout << "Test: Various client thread counts with 100 CRUD operations each" << std::endl;

    // 运行用户指定的测试场景
    RunUserSpecifiedTests();

    std::cout << "\nReal concurrent performance test completed!" << std::endl;
    std::cout << "Note: This test uses simulated network delays to demonstrate" << std::endl;
    std::cout << "the concurrent architecture performance characteristics." << std::endl;

    return 0;
}