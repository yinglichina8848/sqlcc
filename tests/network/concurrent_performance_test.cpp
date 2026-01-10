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
#include <arpa/inet.h>
#include <fcntl.h>
#include <future>

#include "utils/thread_pool.h"
#include "utils/connection_pool.h"
#include "network/mysql_protocol.h"

// 简化的MySQL客户端类（用于连接池）
class PooledMySQLClient {
public:
    PooledMySQLClient(const std::string& host, int port)
        : host_(host), port_(port), sock_(-1), connected_(false) {}

    ~PooledMySQLClient() {
        Disconnect();
    }

    bool Connect() {
        if (connected_) return true;

        sock_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_ < 0) return false;

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_);
        inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr);

        if (connect(sock_, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(sock_);
            sock_ = -1;
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
        packet.push_back(0x00); // sequence
        packet.push_back(0x03); // COM_QUERY
        packet.insert(packet.end(), query.begin(), query.end());

        size_t sent = 0;
        while (sent < packet.size()) {
            ssize_t result = send(sock_, packet.data() + sent, packet.size() - sent, 0);
            if (result <= 0) return false;
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

private:
    bool SendHandshakeResponse() {
        // 简单的握手响应包
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
            ssize_t result = send(sock_, response.data() + sent, response.size() - sent, 0);
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
};

// 线程性能结果
struct ThreadPerformanceResult {
    int thread_id;
    int operations_performed;
    double total_time_ms;
    double avg_time_ms;
    int errors_count;
    std::vector<double> operation_times; // 每个操作的时间
};

// 总体性能结果
struct ConcurrentPerformanceResult {
    int total_threads;
    int operations_per_thread;
    double total_time_ms;
    double avg_time_per_operation_ms;
    double operations_per_second;
    int total_errors;
    double success_rate;
    std::vector<ThreadPerformanceResult> thread_results;

    // 分位数统计
    double p50_latency_ms;
    double p95_latency_ms;
    double p99_latency_ms;
    double p999_latency_ms;
};

// 连接池类型定义
using MySQLConnectionPool = sqlcc::utils::ConnectionPool<PooledMySQLClient>;

// 工作线程函数
ThreadPerformanceResult WorkerThread(int thread_id, int operations_count,
                                   std::shared_ptr<PooledMySQLClient> connection,
                                   const std::vector<std::string>& operations) {
    ThreadPerformanceResult result;
    result.thread_id = thread_id;
    result.operations_performed = operations_count;
    result.errors_count = 0;

    auto start_time = std::chrono::high_resolution_clock::now();

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
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    result.total_time_ms = total_duration.count();
    if (result.operations_performed > result.errors_count) {
        result.avg_time_ms = result.total_time_ms / (result.operations_performed - result.errors_count);
    } else {
        result.avg_time_ms = 0.0;
    }

    return result;
}

// 计算分位数
double CalculatePercentile(const std::vector<double>& data, double percentile) {
    if (data.empty()) return 0.0;

    std::vector<double> sorted_data = data;
    std::sort(sorted_data.begin(), sorted_data.end());

    size_t index = static_cast<size_t>(percentile * (sorted_data.size() - 1));
    return sorted_data[index];
}

// 并发性能测试主函数
ConcurrentPerformanceResult RunConcurrentPerformanceTest(
    const std::string& host, int port,
    int num_threads, int operations_per_thread) {

    ConcurrentPerformanceResult result;
    result.total_threads = num_threads;
    result.operations_per_thread = operations_per_thread;

    std::cout << "=== Concurrent MySQL Protocol Performance Test ===" << std::endl;
    std::cout << "Host: " << host << ":" << port << std::endl;
    std::cout << "Threads: " << num_threads << std::endl;
    std::cout << "Operations per thread: " << operations_per_thread << std::endl;
    std::cout << "Total operations: " << (num_threads * operations_per_thread) << std::endl;
    std::cout << std::endl;

    // 创建连接池
    MySQLConnectionPool::PoolConfig pool_config;
    pool_config.initial_size = num_threads;
    pool_config.max_size = num_threads * 2;
    pool_config.min_size = num_threads / 2;

    MySQLConnectionPool connection_pool(
        [&host, &port]() {
            auto client = std::make_shared<PooledMySQLClient>(host, port);
            if (client->Connect()) {
                return client;
            }
            return std::shared_ptr<PooledMySQLClient>(nullptr);
        },
        [](std::shared_ptr<PooledMySQLClient> client) {
            return client && client->IsConnected();
        },
        pool_config
    );

    connection_pool.start();

    // 创建线程池
    sqlcc::utils::ThreadPool thread_pool(num_threads);

    // 准备测试数据
    std::vector<std::string> insert_ops, select_ops, update_ops, delete_ops;

    for (int i = 0; i < operations_per_thread; ++i) {
        int base_id = i + 1;

        // INSERT
        std::stringstream insert_ss;
        insert_ss << "INSERT INTO users (id, name, email, age) VALUES ("
                  << base_id << ", 'User" << base_id << "', 'user" << base_id << "@example.com', "
                  << (20 + (base_id % 50)) << ")";
        insert_ops.push_back(insert_ss.str());

        // SELECT
        select_ops.push_back("SELECT * FROM users WHERE id = " + std::to_string(base_id));

        // UPDATE
        update_ops.push_back("UPDATE users SET age = age + 1 WHERE id = " + std::to_string(base_id));

        // DELETE
        delete_ops.push_back("DELETE FROM users WHERE id = " + std::to_string(base_id));
    }

    std::vector<std::string> all_operations;
    all_operations.insert(all_operations.end(), insert_ops.begin(), insert_ops.end());
    all_operations.insert(all_operations.end(), select_ops.begin(), select_ops.end());
    all_operations.insert(all_operations.end(), update_ops.begin(), update_ops.end());
    all_operations.insert(all_operations.end(), delete_ops.begin(), delete_ops.end());

    // 提交并发任务
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::future<ThreadPerformanceResult>> futures;

    for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
        auto connection = connection_pool.acquire();
        if (!connection) {
            std::cerr << "Failed to acquire connection for thread " << thread_id << std::endl;
            continue;
        }

        futures.push_back(thread_pool.submit(
            WorkerThread,
            thread_id,
            operations_per_thread,
            connection,
            std::cref(all_operations)
        ));

        // 注意：连接会在WorkerThread完成后自动释放，因为shared_ptr的生命周期
    }

    // 等待所有任务完成
    std::vector<double> all_operation_times;
    int total_operations = 0;
    int total_errors = 0;

    for (auto& future : futures) {
        try {
            ThreadPerformanceResult thread_result = future.get();
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

void PrintConcurrentPerformanceResult(const ConcurrentPerformanceResult& result) {
    std::cout << "=== CONCURRENT PERFORMANCE TEST RESULTS ===" << std::endl;
    std::cout << "Total Threads: " << result.total_threads << std::endl;
    std::cout << "Operations per Thread: " << result.operations_per_thread << std::endl;
    std::cout << "Total Operations: " << (result.total_threads * result.operations_per_thread) << std::endl;
    std::cout << "Total Time: " << result.total_time_ms << " ms" << std::endl;
    std::cout << "Average Time per Operation: " << result.avg_time_per_operation_ms << " ms" << std::endl;
    std::cout << "Operations per Second: " << result.operations_per_second << std::endl;
    std::cout << "Total Errors: " << result.total_errors << std::endl;
    std::cout << "Success Rate: " << result.success_rate << "%" << std::endl;
    std::cout << std::endl;

    std::cout << "=== LATENCY DISTRIBUTION ===" << std::endl;
    std::cout << "P50 (median): " << result.p50_latency_ms << " ms" << std::endl;
    std::cout << "P95: " << result.p95_latency_ms << " ms" << std::endl;
    std::cout << "P99: " << result.p99_latency_ms << " ms" << std::endl;
    std::cout << "P99.9: " << result.p999_latency_ms << " ms" << std::endl;
    std::cout << std::endl;

    std::cout << "=== PER-THREAD RESULTS ===" << std::endl;
    for (const auto& thread_result : result.thread_results) {
        std::cout << "Thread " << thread_result.thread_id << ": "
                  << thread_result.operations_performed << " ops, "
                  << thread_result.total_time_ms << " ms, "
                  << thread_result.avg_time_ms << " ms/op avg, "
                  << thread_result.errors_count << " errors" << std::endl;
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    std::string host = "localhost";
    int port = 18647;
    int num_threads = 8;
    int operations_per_thread = 100;

    // 解析命令行参数
    if (argc >= 2) host = argv[1];
    if (argc >= 3) port = std::stoi(argv[2]);
    if (argc >= 4) num_threads = std::stoi(argv[3]);
    if (argc >= 5) operations_per_thread = std::stoi(argv[4]);

    // 运行并发性能测试
    ConcurrentPerformanceResult result = RunConcurrentPerformanceTest(
        host, port, num_threads, operations_per_thread);

    // 打印结果
    PrintConcurrentPerformanceResult(result);

    std::cout << "Concurrent performance test completed!" << std::endl;
    return 0;
}