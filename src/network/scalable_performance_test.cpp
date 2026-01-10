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

#include "utils/thread_pool.h"
#include "utils/connection_pool.h"
#include "network/mysql_protocol.h"

// 简化的MySQL客户端类（用于连接池）
class ScalableMySQLClient {
public:
    ScalableMySQLClient(const std::string& host, int port, int client_id = 0)
        : host_(host), port_(port), sock_(-1), connected_(false), client_id_(client_id) {}

    ~ScalableMySQLClient() {
        Disconnect();
    }

    bool Connect() {
        if (connected_) return true;

        sock_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_ < 0) {
            std::cerr << "Client " << client_id_ << ": Socket creation failed" << std::endl;
            return false;
        }

        // 设置TCP_NODELAY以提高性能
        int flag = 1;
        setsockopt(sock_, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_);
        inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr);

        if (connect(sock_, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(sock_);
            sock_ = -1;
            std::cerr << "Client " << client_id_ << ": Connection failed" << std::endl;
            return false;
        }

        connected_ = true;

        // 发送握手响应
        return SendHandshakeResponse();
    }

    void Disconnect() {
        if (sock_ >= 0) {
            close(sock_);
            sock_ = -1;
        }
        connected_ = false;
    }

    bool IsConnected() const {
        return connected_;
    }

    bool SendQuery(const std::string& query) {
        if (!connected_ || sock_ < 0) return false;

        std::vector<uint8_t> packet;
        packet.push_back(query.size() & 0xFF);
        packet.push_back((query.size() >> 8) & 0xFF);
        packet.push_back((query.size() >> 16) & 0xFF);
        packet.push_back(sequence_id_++ & 0xFF); // sequence
        packet.push_back(0x03); // COM_QUERY
        packet.insert(packet.end(), query.begin(), query.end());

        size_t sent = 0;
        while (sent < packet.size()) {
            ssize_t result = send(sock_, packet.data() + sent, packet.size() - sent, MSG_NOSIGNAL);
            if (result <= 0) {
                connected_ = false;
                return false;
            }
            sent += result;
        }
        return true;
    }

    std::vector<uint8_t> ReceiveResponse(size_t max_size = 4096) {
        if (!connected_ || sock_ < 0) return {};

        std::vector<uint8_t> buffer(max_size);
        ssize_t received = recv(sock_, buffer.data(), buffer.size(), 0);
        if (received <= 0) {
            connected_ = false;
            return {};
        }

        buffer.resize(received);
        return buffer;
    }

    int GetClientId() const { return client_id_; }

private:
    bool SendHandshakeResponse() {
        // 简化的握手响应包
        std::vector<uint8_t> response = {
            0x14, 0x00, 0x00, 0x01,  // packet length + sequence
            0x00,  // capabilities low
            0x00, 0x00, // capabilities high
            0x00, 0x00, 0x00, 0x00, // max packet size
            0x21, // charset
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // filler
            0x61, 0x64, 0x6d, 0x69, 0x6e, 0x00, // username: admin
            0x00, // null terminator for username
            0x00 // no auth data
        };

        size_t sent = 0;
        while (sent < response.size()) {
            ssize_t result = send(sock_, response.data() + sent, response.size() - sent, MSG_NOSIGNAL);
            if (result <= 0) return false;
            sent += result;
        }

        // 等待认证响应
        auto auth_resp = ReceiveResponse();
        return !auth_resp.empty();
    }

    std::string host_;
    int port_;
    int sock_;
    bool connected_;
    int client_id_;
    uint8_t sequence_id_ = 0;
};

// 线程性能结果
struct ScalableThreadResult {
    int thread_id;
    int operations_performed;
    double total_time_ms;
    double avg_time_ms;
    int errors_count;
    std::vector<double> operation_times;
    double throughput_ops_sec;
};

// 测试配置
struct ScalableTestConfig {
    std::string host = "localhost";
    int port = 18647;
    int num_threads = 4;
    int operations_per_thread = 1000;
    int warmup_operations = 100;
    bool enable_progress_reporting = true;
    int progress_interval_ms = 1000;
};

// 总体性能结果
struct ScalablePerformanceResult {
    ScalableTestConfig config;
    double total_time_ms;
    double total_operations;
    double operations_per_second;
    double avg_time_per_operation_ms;
    int total_errors;
    double success_rate;
    std::vector<ScalableThreadResult> thread_results;

    // 分位数统计
    double p50_latency_ms;
    double p95_latency_ms;
    double p99_latency_ms;
    double p999_latency_ms;

    // 资源统计
    double cpu_utilization_percent;
    size_t memory_usage_mb;
    double network_bandwidth_mbps;
};

// 连接池类型定义
using ScalableMySQLConnectionPool = sqlcc::utils::ConnectionPool<ScalableMySQLClient>;

// 工作线程函数
ScalableThreadResult ScalableWorkerThread(int thread_id, int operations_count,
                                         std::shared_ptr<ScalableMySQLClient> connection,
                                         const std::vector<std::string>& operations,
                                         bool enable_progress, int progress_interval) {
    ScalableThreadResult result;
    result.thread_id = thread_id;
    result.operations_performed = operations_count;
    result.errors_count = 0;

    auto start_time = std::chrono::high_resolution_clock::now();
    auto last_progress_time = start_time;

    for (int i = 0; i < operations_count; ++i) {
        auto op_start = std::chrono::high_resolution_clock::now();

        // 随机选择一个操作
        std::string sql = operations[i % operations.size()];

        bool success = connection->SendQuery(sql);
        if (success) {
            // 等待响应
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

        // 进度报告
        if (enable_progress && i % 100 == 0) {
            auto current_time = std::chrono::high_resolution_clock::now();
            auto time_since_last_progress = std::chrono::duration_cast<std::chrono::milliseconds>(
                current_time - last_progress_time).count();

            if (time_since_last_progress >= progress_interval) {
                double progress = (i * 100.0) / operations_count;
                std::cout << "Thread " << thread_id << ": " << std::fixed << std::setprecision(1)
                         << progress << "% complete (" << i << "/" << operations_count << ")" << std::endl;
                last_progress_time = current_time;
            }
        }
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

// 执行预热操作
void PerformWarmup(std::shared_ptr<ScalableMySQLClient> connection, int warmup_ops) {
    std::cout << "Performing warmup with " << warmup_ops << " operations..." << std::endl;

    for (int i = 0; i < warmup_ops; ++i) {
        std::string sql = "SELECT 1"; // 简单的预热查询
        connection->SendQuery(sql);
        auto response = connection->ReceiveResponse();
        // 不检查结果，只为预热
    }

    std::cout << "Warmup completed." << std::endl;
}

// 主测试函数
ScalablePerformanceResult RunScalablePerformanceTest(const ScalableTestConfig& config) {
    ScalablePerformanceResult result;
    result.config = config;

    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "SCALABLE MYSQL PROTOCOL PERFORMANCE TEST" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    std::cout << "Host: " << config.host << ":" << config.port << std::endl;
    std::cout << "Threads: " << config.num_threads << std::endl;
    std::cout << "Operations per thread: " << config.operations_per_thread << std::endl;
    std::cout << "Total operations: " << (config.num_threads * config.operations_per_thread) << std::endl;
    std::cout << "Warmup operations: " << config.warmup_operations << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    // 创建连接池
    ScalableMySQLConnectionPool::PoolConfig pool_config;
    pool_config.initial_size = config.num_threads;
    pool_config.max_size = config.num_threads * 2;
    pool_config.min_size = std::max(1, config.num_threads / 4);

    std::atomic<int> client_counter{0};
    ScalableMySQLConnectionPool connection_pool(
        [&config, &client_counter]() {
            int client_id = client_counter++;
            auto client = std::make_shared<ScalableMySQLClient>(
                config.host, config.port, client_id);
            if (client->Connect()) {
                return client;
            }
            return std::shared_ptr<ScalableMySQLClient>(nullptr);
        },
        [](std::shared_ptr<ScalableMySQLClient> client) {
            return client && client->IsConnected();
        },
        pool_config
    );

    connection_pool.start();

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

    // 预热阶段
    std::cout << "Starting warmup phase..." << std::endl;
    auto warmup_connection = connection_pool.acquire();
    if (warmup_connection) {
        PerformWarmup(warmup_connection, config.warmup_operations);
        connection_pool.release(warmup_connection);
    }

    // 主要测试阶段
    std::cout << "Starting main test phase..." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::future<ScalableThreadResult>> futures;

    for (int thread_id = 0; thread_id < config.num_threads; ++thread_id) {
        auto connection = connection_pool.acquire();
        if (!connection) {
            std::cerr << "Failed to acquire connection for thread " << thread_id << std::endl;
            continue;
        }

        // 为每个线程分配操作范围
        int start_op = thread_id * config.operations_per_thread * 4; // 4 operations per iteration
        int end_op = start_op + config.operations_per_thread * 4;
        std::vector<std::string> thread_operations(
            all_operations.begin() + start_op,
            all_operations.begin() + std::min(end_op, (int)all_operations.size()));

        futures.push_back(thread_pool.submit(
            ScalableWorkerThread,
            thread_id,
            config.operations_per_thread,
            connection,
            thread_operations,
            config.enable_progress_reporting,
            config.progress_interval_ms
        ));
    }

    // 等待所有任务完成
    std::vector<double> all_operation_times;
    int total_operations = 0;
    int total_errors = 0;

    for (auto& future : futures) {
        try {
            ScalableThreadResult thread_result = future.get();
            result.thread_results.push_back(thread_result);

            total_operations += thread_result.operations_performed;
            total_errors += thread_result.errors_count;
            all_operation_times.insert(all_operation_times.end(),
                                     thread_result.operation_times.begin(),
                                     thread_result.operation_times.end());
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

    // 计算分位数
    if (!all_operation_times.empty()) {
        result.p50_latency_ms = CalculatePercentile(all_operation_times, 0.5);
        result.p95_latency_ms = CalculatePercentile(all_operation_times, 0.95);
        result.p99_latency_ms = CalculatePercentile(all_operation_times, 0.99);
        result.p999_latency_ms = CalculatePercentile(all_operation_times, 0.999);
    }

    // 停止连接池
    connection_pool.shutdown();

    return result;
}

// 打印详细结果
void PrintScalablePerformanceResult(const ScalablePerformanceResult& result) {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "SCALABLE PERFORMANCE TEST RESULTS" << std::endl;
    std::cout << std::string(80, '=') << std::endl;

    std::cout << "Test Configuration:" << std::endl;
    std::cout << "  Host: " << result.config.host << ":" << result.config.port << std::endl;
    std::cout << "  Threads: " << result.config.num_threads << std::endl;
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
                  << std::setw(4) << thread_result.operations_performed << " ops, "
                  << std::fixed << std::setprecision(1) << std::setw(8) << thread_result.total_time_ms << " ms, "
                  << std::fixed << std::setprecision(2) << std::setw(6) << thread_result.avg_time_ms << " ms/op avg, "
                  << std::setw(3) << thread_result.errors_count << " errors, "
                  << std::fixed << std::setprecision(1) << std::setw(6) << thread_result.throughput_ops_sec << " ops/sec"
                  << std::endl;
    }

    std::cout << std::string(80, '=') << std::endl;
}

// 运行多轮测试的函数
void RunMultiScaleTests(const std::string& host, int port) {
    std::vector<std::pair<int, int>> test_configs = {
        {1, 1000},   // 1线程，1000操作
        {2, 1000},   // 2线程，1000操作
        {4, 1000},   // 4线程，1000操作
        {8, 1000},   // 8线程，1000操作
        {4, 5000},   // 4线程，5000操作
        {4, 10000},  // 4线程，10000操作
        {8, 5000},   // 8线程，5000操作
        {8, 10000},  // 8线程，10000操作
        {16, 5000},  // 16线程，5000操作
        {16, 10000}, // 16线程，10000操作
    };

    std::cout << "\n" << std::string(100, '*') << std::endl;
    std::cout << "MULTI-SCALE CONCURRENT PERFORMANCE TEST SUITE" << std::endl;
    std::cout << "Host: " << host << ":" << port << std::endl;
    std::cout << "Testing various thread counts and operation scales" << std::endl;
    std::cout << std::string(100, '*') << std::endl;

    std::vector<ScalablePerformanceResult> all_results;

    for (size_t i = 0; i < test_configs.size(); ++i) {
        const auto& config_pair = test_configs[i];
        int num_threads = config_pair.first;
        int ops_per_thread = config_pair.second;

        std::cout << "\nTest " << (i + 1) << "/" << test_configs.size()
                  << " - Threads: " << num_threads
                  << ", Operations per thread: " << ops_per_thread
                  << ", Total operations: " << (num_threads * ops_per_thread) << std::endl;

        ScalableTestConfig config;
        config.host = host;
        config.port = port;
        config.num_threads = num_threads;
        config.operations_per_thread = ops_per_thread;
        config.warmup_operations = 50;
        config.enable_progress_reporting = false; // 禁用详细进度以加快测试

        auto result = RunScalablePerformanceTest(config);
        all_results.push_back(result);

        // 简要输出关键指标
        std::cout << "Result: " << std::fixed << std::setprecision(2)
                  << result.operations_per_second << " ops/sec, "
                  << result.avg_time_per_operation_ms << " ms/op avg, "
                  << result.success_rate << "% success rate" << std::endl;
    }

    // 输出汇总报告
    std::cout << "\n" << std::string(100, '=') << std::endl;
    std::cout << "MULTI-SCALE TEST SUMMARY" << std::endl;
    std::cout << std::string(100, '=') << std::endl;
    std::cout << std::setw(8) << "Threads" << std::setw(12) << "Operations" << std::setw(15) << "Total Ops"
              << std::setw(12) << "Ops/sec" << std::setw(10) << "Avg(ms)" << std::setw(8) << "Success%" << std::endl;
    std::cout << std::string(75, '-') << std::endl;

    for (const auto& result : all_results) {
        std::cout << std::setw(8) << result.config.num_threads
                  << std::setw(12) << result.config.operations_per_thread
                  << std::setw(15) << result.total_operations
                  << std::setw(12) << std::fixed << std::setprecision(1) << result.operations_per_second
                  << std::setw(10) << std::fixed << std::setprecision(2) << result.avg_time_per_operation_ms
                  << std::setw(8) << std::fixed << std::setprecision(1) << result.success_rate << std::endl;
    }

    std::cout << std::string(100, '=') << std::endl;
}

int main(int argc, char* argv[]) {
    std::string host = "localhost";
    int port = 18647;
    int num_threads = 4;
    int operations_per_thread = 1000;
    bool run_multi_scale = false;

    // 解析命令行参数
    if (argc >= 2) host = argv[1];
    if (argc >= 3) port = std::stoi(argv[2]);
    if (argc >= 4) num_threads = std::stoi(argv[3]);
    if (argc >= 5) operations_per_thread = std::stoi(argv[4]);
    if (argc >= 6) run_multi_scale = (std::string(argv[5]) == "multi");

    if (run_multi_scale) {
        RunMultiScaleTests(host, port);
    } else {
        ScalableTestConfig config;
        config.host = host;
        config.port = port;
        config.num_threads = num_threads;
        config.operations_per_thread = operations_per_thread;
        config.warmup_operations = 100;
        config.enable_progress_reporting = true;

        ScalablePerformanceResult result = RunScalablePerformanceTest(config);
        PrintScalablePerformanceResult(result);
    }

    std::cout << "\nScalable performance test suite completed!" << std::endl;
    return 0;
}