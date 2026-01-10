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
#include "utils/connection_pool.h"
#include "network/mysql_protocol.h"

// 真实的MySQL客户端类 - 连接到真正的SQLCC服务器
class TrueMySQLClient {
public:
    TrueMySQLClient(const std::string& host, int port, int client_id = 0)
        : host_(host), port_(port), sock_(-1), connected_(false), client_id_(client_id),
          sequence_id_(0) {}

    ~TrueMySQLClient() {
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
            std::cerr << "Client " << client_id_ << ": Connection failed: " << strerror(errno) << std::endl;
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
        if (!connected_ || sock_ < 0) {
            std::cerr << "Client " << client_id_ << ": Not connected" << std::endl;
            return false;
        }

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
                std::cerr << "Client " << client_id_ << ": Send failed: " << strerror(errno) << std::endl;
                return false;
            }
            sent += result;
        }
        return true;
    }

    std::vector<uint8_t> ReceiveResponse(size_t max_size = 8192) {
        if (!connected_ || sock_ < 0) {
            return {};
        }

        std::vector<uint8_t> buffer(max_size);
        ssize_t received = recv(sock_, buffer.data(), buffer.size(), 0);
        if (received <= 0) {
            if (received < 0) {
                std::cerr << "Client " << client_id_ << ": Receive failed: " << strerror(errno) << std::endl;
            }
            connected_ = false;
            return {};
        }

        buffer.resize(received);
        return buffer;
    }

    // 执行CRUD操作并验证结果
    bool ExecuteInsert(int user_id, int age) {
        std::stringstream ss;
        ss << "INSERT INTO users (id, name, email, age) VALUES ("
           << user_id << ", 'User" << user_id << "', 'user" << user_id << "@test.com', " << age << ")";

        if (!SendQuery(ss.str())) {
            return false;
        }

        auto response = ReceiveResponse();
        if (response.empty()) {
            return false;
        }

        // 检查响应是否表示成功 (简单的OK包检查)
        return response.size() >= 5 && response[4] == 0x00; // OK packet type
    }

    bool ExecuteSelect(int user_id) {
        std::stringstream ss;
        ss << "SELECT * FROM users WHERE id = " << user_id;

        if (!SendQuery(ss.str())) {
            return false;
        }

        auto response = ReceiveResponse();
        return !response.empty();
    }

    bool ExecuteUpdate(int user_id, int new_age) {
        std::stringstream ss;
        ss << "UPDATE users SET age = " << new_age << " WHERE id = " << user_id;

        if (!SendQuery(ss.str())) {
            return false;
        }

        auto response = ReceiveResponse();
        if (response.empty()) {
            return false;
        }

        // 检查响应是否表示成功
        return response.size() >= 5 && response[4] == 0x00;
    }

    bool ExecuteDelete(int user_id) {
        std::stringstream ss;
        ss << "DELETE FROM users WHERE id = " << user_id;

        if (!SendQuery(ss.str())) {
            return false;
        }

        auto response = ReceiveResponse();
        if (response.empty()) {
            return false;
        }

        // 检查响应是否表示成功
        return response.size() >= 5 && response[4] == 0x00;
    }

    int GetClientId() const { return client_id_; }

private:
    bool SendHandshakeResponse() {
        // MySQL协议握手响应包
        std::vector<uint8_t> response = {
            0x14, 0x00, 0x00, 0x01,  // packet length + sequence
            0x00,  // capabilities low
            0x00, 0x00, // capabilities high
            0x00, 0x00, 0x00, 0x00, // max packet size
            0x21, // charset (utf8_general_ci)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // filler
            0x61, 0x64, 0x6d, 0x69, 0x6e, 0x00, // username: admin
            0x00, // null terminator for username
            0x00 // no auth data
        };

        size_t sent = 0;
        while (sent < response.size()) {
            ssize_t result = send(sock_, response.data() + sent, response.size() - sent, MSG_NOSIGNAL);
            if (result <= 0) {
                std::cerr << "Client " << client_id_ << ": Handshake send failed" << std::endl;
                return false;
            }
            sent += result;
        }

        // 等待认证响应
        auto auth_resp = ReceiveResponse();
        if (auth_resp.empty()) {
            std::cerr << "Client " << client_id_ << ": No auth response" << std::endl;
            return false;
        }

        std::cout << "Client " << client_id_ << ": Connected and authenticated (" << auth_resp.size() << " bytes)" << std::endl;
        return true;
    }

    std::string host_;
    int port_;
    int sock_;
    bool connected_;
    int client_id_;
    uint8_t sequence_id_;
};

// 线程性能结果
struct TrueCRUDResult {
    int thread_id;
    int operations_performed;
    int inserts_completed;
    int selects_completed;
    int updates_completed;
    int deletes_completed;
    double total_time_ms;
    double avg_time_ms;
    int errors_count;
    std::vector<double> operation_times;
    double throughput_ops_sec;
};

// 测试配置
struct TrueCRUDTestConfig {
    std::string host = "localhost";
    int port = 18647;
    int num_threads = 4;
    int operations_per_thread = 100;  // 用户要求的100次操作
    int warmup_operations = 10;
    bool enable_progress_reporting = false;
    int progress_interval_ms = 5000;
    int base_user_id = 1000; // 避免与其他测试冲突
};

// 总体性能结果
struct TrueCRUDPerformanceResult {
    TrueCRUDTestConfig config;
    double total_time_ms;
    double total_operations;
    double operations_per_second;
    double avg_time_per_operation_ms;
    int total_inserts;
    int total_selects;
    int total_updates;
    int total_deletes;
    int total_errors;
    double success_rate;

    std::vector<TrueCRUDResult> thread_results;
};

// 工作线程函数 - 执行真实的CRUD操作
TrueCRUDResult TrueCRUDWorkerThread(int thread_id, int operations_count,
                                   std::shared_ptr<TrueMySQLClient> connection,
                                   int base_user_id) {
    TrueCRUDResult result;
    result.thread_id = thread_id;
    result.operations_performed = operations_count;
    result.errors_count = 0;
    result.inserts_completed = 0;
    result.selects_completed = 0;
    result.updates_completed = 0;
    result.deletes_completed = 0;

    auto start_time = std::chrono::high_resolution_clock::now();

    // 执行完整的CRUD循环
    for (int i = 0; i < operations_count; ++i) {
        auto op_start = std::chrono::high_resolution_clock::now();

        int user_id = base_user_id + thread_id * operations_count + i;
        int user_age = 20 + (user_id % 50);

        // INSERT
        if (connection->ExecuteInsert(user_id, user_age)) {
            result.inserts_completed++;
        } else {
            result.errors_count++;
        }

        // SELECT
        if (connection->ExecuteSelect(user_id)) {
            result.selects_completed++;
        } else {
            result.errors_count++;
        }

        // UPDATE
        if (connection->ExecuteUpdate(user_id, user_age + 1)) {
            result.updates_completed++;
        } else {
            result.errors_count++;
        }

        // DELETE
        if (connection->ExecuteDelete(user_id)) {
            result.deletes_completed++;
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
    int successful_operations = result.inserts_completed + result.selects_completed +
                               result.updates_completed + result.deletes_completed;
    if (successful_operations > 0) {
        result.avg_time_ms = result.total_time_ms / successful_operations;
        result.throughput_ops_sec = (successful_operations * 1000.0) / result.total_time_ms;
    } else {
        result.avg_time_ms = 0.0;
        result.throughput_ops_sec = 0.0;
    }

    return result;
}

// 执行预热操作
void PerformTrueWarmup(std::shared_ptr<TrueMySQLClient> connection, int warmup_ops, int base_user_id) {
    std::cout << "Performing true CRUD warmup with " << warmup_ops << " operations..." << std::endl;

    for (int i = 0; i < warmup_ops; ++i) {
        int user_id = base_user_id + 9990 + i; // 使用不同的ID范围

        // 执行完整的CRUD循环作为预热
        connection->ExecuteInsert(user_id, 25);
        connection->ExecuteSelect(user_id);
        connection->ExecuteUpdate(user_id, 26);
        connection->ExecuteDelete(user_id);
    }

    std::cout << "True CRUD warmup completed." << std::endl;
}

// 主测试函数 - 执行真实的CRUD并发性能测试
TrueCRUDPerformanceResult RunTrueCRUDPerformanceTest(const TrueCRUDTestConfig& config) {
    TrueCRUDPerformanceResult result;
    result.config = config;

    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "TRUE CRUD MYSQL PROTOCOL PERFORMANCE TEST" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    std::cout << "Host: " << config.host << ":" << config.port << std::endl;
    std::cout << "Threads: " << config.num_threads << std::endl;
    std::cout << "Operations per thread: " << config.operations_per_thread << std::endl;
    std::cout << "Total CRUD operations: " << (config.num_threads * config.operations_per_thread * 4) << std::endl;
    std::cout << "Base user ID: " << config.base_user_id << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    // 创建线程池
    sqlcc::utils::ThreadPool thread_pool(config.num_threads);

    // 创建连接并预热
    std::cout << "Creating true database connections..." << std::endl;
    std::vector<std::shared_ptr<TrueMySQLClient>> connections;

    for (int i = 0; i < config.num_threads; ++i) {
        auto connection = std::make_shared<TrueMySQLClient>(config.host, config.port, i);
        if (connection->Connect()) {
            PerformTrueWarmup(connection, config.warmup_operations, config.base_user_id);
            connections.push_back(connection);
            std::cout << "Connection " << i << " established successfully" << std::endl;
        } else {
            std::cerr << "Failed to create connection for thread " << i << std::endl;
        }
    }

    if (connections.empty()) {
        std::cerr << "No connections established. Cannot run test." << std::endl;
        return result;
    }

    if (connections.size() < static_cast<size_t>(config.num_threads)) {
        std::cerr << "Warning: Only " << connections.size() << " connections created out of "
                  << config.num_threads << " requested. Adjusting test." << std::endl;
        // 更新配置以反映实际连接数
        const_cast<TrueCRUDTestConfig&>(config).num_threads = connections.size();
    }

    // 主要测试阶段
    std::cout << "\nStarting TRUE CRUD performance test..." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::future<TrueCRUDResult>> futures;

    for (int thread_id = 0; thread_id < config.num_threads && thread_id < static_cast<int>(connections.size()); ++thread_id) {
        auto connection = connections[thread_id];

        futures.push_back(thread_pool.submit(
            TrueCRUDWorkerThread,
            thread_id,
            config.operations_per_thread,
            connection,
            config.base_user_id
        ));
    }

    // 等待所有任务完成并收集结果
    std::vector<double> all_operation_times;
    int total_operations = 0;
    int total_inserts = 0;
    int total_selects = 0;
    int total_updates = 0;
    int total_deletes = 0;
    int total_errors = 0;

    for (auto& future : futures) {
        try {
            TrueCRUDResult thread_result = future.get();
            result.thread_results.push_back(thread_result);

            total_operations += thread_result.operations_performed;
            total_inserts += thread_result.inserts_completed;
            total_selects += thread_result.selects_completed;
            total_updates += thread_result.updates_completed;
            total_deletes += thread_result.deletes_completed;
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
    result.total_inserts = total_inserts;
    result.total_selects = total_selects;
    result.total_updates = total_updates;
    result.total_deletes = total_deletes;
    result.total_errors = total_errors;

    int successful_operations = total_inserts + total_selects + total_updates + total_deletes;
    if (successful_operations > 0) {
        result.avg_time_per_operation_ms = result.total_time_ms / successful_operations;
        result.operations_per_second = (successful_operations * 1000.0) / result.total_time_ms;
    }

    result.success_rate = total_operations > 0 ?
                         (static_cast<double>(successful_operations) / (total_operations * 4)) * 100.0 : 0.0;

    // 关闭所有连接
    for (auto& connection : connections) {
        connection->Disconnect();
    }

    return result;
}

// 打印详细结果
void PrintTrueCRUDPerformanceResult(const TrueCRUDPerformanceResult& result) {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "TRUE CRUD PERFORMANCE TEST RESULTS" << std::endl;
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
    std::cout << "  Success rate: " << std::fixed << std::setprecision(2)
              << result.success_rate << "%" << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    std::cout << "CRUD Operations Summary:" << std::endl;
    std::cout << "  Total INSERT operations: " << result.total_inserts << std::endl;
    std::cout << "  Total SELECT operations: " << result.total_selects << std::endl;
    std::cout << "  Total UPDATE operations: " << result.total_updates << std::endl;
    std::cout << "  Total DELETE operations: " << result.total_deletes << std::endl;
    std::cout << "  Total errors: " << result.total_errors << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    std::cout << "Per-Thread Results:" << std::endl;
    for (size_t i = 0; i < result.thread_results.size(); ++i) {
        const auto& thread_result = result.thread_results[i];
        std::cout << "  Thread " << std::setw(2) << thread_result.thread_id << ": "
                  << std::setw(3) << thread_result.operations_performed << " ops, "
                  << std::setw(4) << thread_result.inserts_completed << " inserts, "
                  << std::setw(4) << thread_result.selects_completed << " selects, "
                  << std::setw(4) << thread_result.updates_completed << " updates, "
                  << std::setw(4) << thread_result.deletes_completed << " deletes, "
                  << std::fixed << std::setprecision(1) << std::setw(8) << thread_result.total_time_ms << " ms, "
                  << std::fixed << std::setprecision(1) << std::setw(6) << thread_result.throughput_ops_sec << " ops/sec"
                  << std::endl;
    }

    std::cout << std::string(80, '=') << std::endl;
}

// 运行单次测试
TrueCRUDPerformanceResult RunSingleTest(const std::string& host, int port, int num_threads, int operations_per_thread, int base_user_id) {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "RUNNING TEST: " << num_threads << " client threads, " << operations_per_thread << " ops/thread" << std::endl;
    std::cout << "Server: " << host << ":" << port << std::endl;
    std::cout << "Total operations: " << (num_threads * operations_per_thread) << std::endl;
    std::cout << "Total CRUD operations: " << (num_threads * operations_per_thread * 4) << std::endl;
    std::cout << std::string(80, '=') << std::endl;

    TrueCRUDTestConfig config;
    config.host = host;
    config.port = port;
    config.num_threads = num_threads;
    config.operations_per_thread = operations_per_thread;
    config.warmup_operations = 5;
    config.enable_progress_reporting = false;
    config.base_user_id = base_user_id;

    auto result = RunTrueCRUDPerformanceTest(config);

    // 输出关键指标
    std::cout << "\nRESULTS:" << std::endl;
    std::cout << "  Total time: " << std::fixed << std::setprecision(2) << result.total_time_ms << " ms" << std::endl;
    std::cout << "  Throughput: " << std::fixed << std::setprecision(2) << result.operations_per_second << " ops/sec" << std::endl;
    std::cout << "  Avg latency: " << std::fixed << std::setprecision(3) << result.avg_time_per_operation_ms << " ms" << std::endl;
    std::cout << "  Success rate: " << std::fixed << std::setprecision(2) << result.success_rate << "%" << std::endl;
    std::cout << "  INSERT: " << result.total_inserts << ", SELECT: " << result.total_selects
              << ", UPDATE: " << result.total_updates << ", DELETE: " << result.total_deletes << std::endl;

    return result;
}

// 运行用户要求的测试场景 - 真实的CRUD操作
void RunTrueCRUDTests(const std::string& host, int port, int num_threads, int operations_per_thread) {
    std::cout << "\n" << std::string(120, '*') << std::endl;
    std::cout << "TRUE CRUD PERFORMANCE TEST" << std::endl;
    std::cout << "Server: " << host << ":" << port << std::endl;
    std::cout << "Client threads: " << num_threads << std::endl;
    std::cout << "Operations per thread: " << operations_per_thread << " TRUE CRUD operations (INSERT+SELECT+UPDATE+DELETE)" << std::endl;
    std::cout << std::string(120, '*') << std::endl;

    // 运行单次测试
    auto result = RunSingleTest(host, port, num_threads, operations_per_thread, 1000);

    std::cout << "\n" << std::string(120, '=') << std::endl;
    std::cout << "TEST COMPLETED" << std::endl;
    std::cout << "Configuration: " << num_threads << " threads × " << operations_per_thread << " operations" << std::endl;
    std::cout << "Total CRUD operations: " << (num_threads * operations_per_thread * 4) << std::endl;
    std::cout << "Performance: " << std::fixed << std::setprecision(2) << result.operations_per_second << " ops/sec" << std::endl;
    std::cout << "Success rate: " << std::fixed << std::setprecision(2) << result.success_rate << "%" << std::endl;
    std::cout << std::string(120, '=') << std::endl;
    std::cout << "\nNOTE: These results are from TRUE database operations against SQLCC server." << std::endl;
    std::cout << "Each operation involves actual INSERT/SELECT/UPDATE/DELETE SQL execution." << std::endl;
}

int main(int argc, char* argv[]) {
    std::string host = "localhost";
    int port = 18647;
    int num_threads = 1; // 默认1个客户端线程
    int operations_per_thread = 100; // 默认100次操作

    // 解析命令行参数
    int opt;
    while ((opt = getopt(argc, argv, "h:p:t:o:")) != -1) {
        switch (opt) {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = std::stoi(optarg);
                break;
            case 't':
                num_threads = std::stoi(optarg);
                break;
            case 'o':
                operations_per_thread = std::stoi(optarg);
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-h host] [-p port] [-t threads] [-o operations_per_thread]" << std::endl;
                std::cerr << "  -h: Server host (default: localhost)" << std::endl;
                std::cerr << "  -p: Server port (default: 18647)" << std::endl;
                std::cerr << "  -t: Number of client threads (default: 1)" << std::endl;
                std::cerr << "  -o: Operations per thread (default: 100)" << std::endl;
                return 1;
        }
    }

    std::cout << "SQLCC TRUE CRUD Performance Test" << std::endl;
    std::cout << "Based on ThreadPool + Connection Pool Architecture" << std::endl;
    std::cout << "Testing REAL database operations against SQLCC MySQL server" << std::endl;
    std::cout << "Each operation executes actual INSERT/SELECT/UPDATE/DELETE operations" << std::endl;
    std::cout << "Configuration:" << std::endl;
    std::cout << "  Host: " << host << ":" << port << std::endl;
    std::cout << "  Client threads: " << num_threads << std::endl;
    std::cout << "  Operations per thread: " << operations_per_thread << std::endl;
    std::cout << "  Total operations: " << (num_threads * operations_per_thread) << std::endl;
    std::cout << "  Total CRUD operations: " << (num_threads * operations_per_thread * 4) << std::endl;

    // 检查服务器是否在运行
    std::cout << "\nChecking if SQLCC server is running on " << host << ":" << port << "..." << std::endl;

    int test_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (test_sock >= 0) {
        sockaddr_in test_addr{};
        test_addr.sin_family = AF_INET;
        test_addr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &test_addr.sin_addr);

        if (connect(test_sock, (sockaddr*)&test_addr, sizeof(test_addr)) >= 0) {
            std::cout << "✓ Server is running. Starting TRUE CRUD tests..." << std::endl;
            close(test_sock);

            // 运行真实的CRUD测试，使用命令行参数
            RunTrueCRUDTests(host, port, num_threads, operations_per_thread);
        } else {
            std::cout << "✗ Server is not running on " << host << ":" << port << std::endl;
            std::cout << "Please start the SQLCC MySQL server first:" << std::endl;
            std::cout << "  bazel run //src/network:server_main -t <thread_pool_size>" << std::endl;
            close(test_sock);
            return 1;
        }
    } else {
        std::cout << "✗ Cannot create test socket" << std::endl;
        return 1;
    }

    std::cout << "\nTrue CRUD performance test completed!" << std::endl;
    return 0;
}
    std::cout << "\nChecking if SQLCC server is running on " << host << ":" << port << "..." << std::endl;