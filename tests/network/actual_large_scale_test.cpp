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
#include <map>

// 简化的MySQL客户端用于大规模测试
class SimpleMySQLClient {
public:
    SimpleMySQLClient(const std::string& host, int port, int client_id = 0)
        : host_(host), port_(port), sock_(-1), connected_(false), client_id_(client_id),
          sequence_id_(0) {}

    ~SimpleMySQLClient() {
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
        SendHandshakeResponse();
        return true;
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
        packet.push_back(sequence_id_++ & 0xFF);
        packet.push_back(0x03);
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

    bool ReceiveResponse() {
        if (!connected_ || sock_ < 0) return false;

        uint8_t header[4];
        if (read(sock_, header, 4) != 4) return false;

        uint32_t length = header[0] | (header[1] << 8) | (header[2] << 16);
        if (length > 0) {
            std::vector<uint8_t> payload(length);
            if (read(sock_, payload.data(), length) != (ssize_t)length) {
                return false;
            }
        }
        return true;
    }

    bool ExecuteCRUD(const std::string& operation, int user_id, int age = 0) {
        std::stringstream ss;

        if (operation == "INSERT") {
            ss << "INSERT INTO users (id, name, email, age) VALUES ("
               << user_id << ", 'User" << user_id << "', 'user" << user_id << "@test.com', " << age << ")";
        } else if (operation == "SELECT") {
            ss << "SELECT * FROM users WHERE id = " << user_id;
        } else if (operation == "UPDATE") {
            ss << "UPDATE users SET age = " << age << " WHERE id = " << user_id;
        } else if (operation == "CREATE") {
            ss << "CREATE TABLE IF NOT EXISTS test_users (id INT PRIMARY KEY, name VARCHAR(50), email VARCHAR(100), age INT)";
        }

        std::string query = ss.str();
        std::cout << "Client " << client_id_ << ": Executing " << operation << " query: " << query << std::endl;

        if (!SendQuery(query)) {
            std::cerr << "Client " << client_id_ << ": Failed to send " << operation << " query" << std::endl;
            return false;
        }

        if (!ReceiveResponse()) {
            std::cerr << "Client " << client_id_ << ": Failed to receive " << operation << " response" << std::endl;
            return false;
        }

        std::cout << "Client " << client_id_ << ": " << operation << " completed successfully" << std::endl;
        return true;
    }

    int GetClientId() const { return client_id_; }

private:
    void SendHandshakeResponse() {
        std::vector<uint8_t> response = {
            0x14, 0x00, 0x00, 0x01,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x61, 0x64, 0x6d, 0x69, 0x6e, 0x00, 0x00
        };

        write(sock_, response.data(), response.size());
        std::vector<uint8_t> dummy(1024);
        read(sock_, dummy.data(), dummy.size());
    }

    std::string host_;
    int port_;
    int sock_;
    bool connected_;
    int client_id_;
    uint8_t sequence_id_;
};

struct TestResult {
    int phase_percentage;
    int operations_performed;
    double total_time_ms;
    double throughput_ops_sec;
    double avg_latency_ms;
    int successful_operations;
    double success_rate;
    long long estimated_records;

    TestResult() : phase_percentage(0), operations_performed(0), total_time_ms(0.0),
                   throughput_ops_sec(0.0), avg_latency_ms(0.0), successful_operations(0),
                   success_rate(0.0), estimated_records(0) {}
};

class LargeScalePerformanceTester {
public:
    LargeScalePerformanceTester(const std::string& host, int port, int num_threads, int operations_per_thread)
        : host_(host), port_(port), num_threads_(num_threads), operations_per_thread_(operations_per_thread) {}

    std::vector<TestResult> RunDataAccumulationTest() {
        std::vector<TestResult> results;
        std::vector<int> test_phases = {10, 25, 50, 75, 100};

        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << "LARGE SCALE DATA ACCUMULATION PERFORMANCE TEST" << std::endl;
        std::cout << "Configuration: " << num_threads_ << " threads × " << operations_per_thread_ << " operations" << std::endl;
        std::cout << "Total operations: " << (num_threads_ * operations_per_thread_) << std::endl;
        std::cout << "Expected final records: " << (num_threads_ * operations_per_thread_ * 3) << std::endl;
        std::cout << std::string(80, '=') << std::endl;

        for (int phase : test_phases) {
            int phase_operations = (operations_per_thread_ * phase) / 100;
            TestResult result = RunPhaseTest(phase, phase_operations);
            results.push_back(result);

            std::cout << "\nPHASE " << phase << "% RESULTS:" << std::endl;
            std::cout << "  Operations: " << result.operations_performed << std::endl;
            std::cout << "  Time: " << std::fixed << std::setprecision(2) << result.total_time_ms << " ms" << std::endl;
            std::cout << "  Throughput: " << std::fixed << std::setprecision(2) << result.throughput_ops_sec << " ops/sec" << std::endl;
            std::cout << "  Avg Latency: " << std::fixed << std::setprecision(2) << result.avg_latency_ms << " ms" << std::endl;
            std::cout << "  Success Rate: " << std::fixed << std::setprecision(2) << result.success_rate << "%" << std::endl;
            std::cout << "  Estimated Records: " << result.estimated_records << std::endl;
        }

        return results;
    }

private:
    TestResult RunPhaseTest(int phase_percentage, int operations_this_phase) {
        TestResult result;
        result.phase_percentage = phase_percentage;

        // 创建连接
        std::vector<std::shared_ptr<SimpleMySQLClient>> clients;
        std::cout << "Phase " << phase_percentage << "%: Attempting to connect " << num_threads_ << " clients..." << std::endl;

        for (int i = 0; i < num_threads_; ++i) {
            auto client = std::make_shared<SimpleMySQLClient>(host_, port_, i);
            if (client->Connect()) {
                clients.push_back(client);
                std::cout << "  Client " << i << ": ✓ Connected" << std::endl;
            } else {
                std::cout << "  Client " << i << ": ❌ Connection failed" << std::endl;
            }
        }

        if (clients.empty()) {
            std::cerr << "❌ CRITICAL ERROR: No clients could connect to server at " << host_ << ":" << port_ << std::endl;
            std::cerr << "   Please ensure the SQLCC server is running:" << std::endl;
            std::cerr << "   bazel run //src/sqlcc_server:server_main -t 8" << std::endl;
            std::cerr << "   This is why all operations show 0 - no server connection!" << std::endl;
            return result;
        }

        std::cout << "Phase " << phase_percentage << "%: " << clients.size() << "/" << num_threads_ << " clients connected successfully" << std::endl;

        // 运行测试
        auto start_time = std::chrono::high_resolution_clock::now();

        std::vector<std::future<int>> futures;
        for (size_t i = 0; i < clients.size(); ++i) {
            futures.push_back(std::async(std::launch::async, [this, client = clients[i], i, operations_this_phase]() {
                return RunClientOperations(client, i, operations_this_phase);
            }));
        }

        // 收集结果
        int total_successful = 0;
        for (auto& future : futures) {
            total_successful += future.get();
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        result.operations_performed = operations_this_phase * clients.size();
        result.total_time_ms = duration.count();
        result.successful_operations = total_successful;

        if (result.operations_performed > 0) {
            result.success_rate = (static_cast<double>(total_successful) / (result.operations_performed * 3)) * 100.0;
            result.throughput_ops_sec = (total_successful * 1000.0) / result.total_time_ms;
            result.avg_latency_ms = result.total_time_ms / total_successful;
        }

        result.estimated_records = static_cast<long long>(total_successful);

        // 断开连接
        clients.clear();

        return result;
    }

    int RunClientOperations(std::shared_ptr<SimpleMySQLClient> client, int client_index, int operations) {
        int successful = 0;
        int base_user_id = client_index * operations_per_thread_ + (operations_per_thread_ * client_index);

        for (int i = 0; i < operations; ++i) {
            int user_id = base_user_id + i;
            int user_age = 20 + (user_id % 50);

            // 执行不删除数据的CRUD循环
            if (client->ExecuteCRUD("INSERT", user_id, user_age)) successful++;
            if (client->ExecuteCRUD("SELECT", user_id)) successful++;
            if (client->ExecuteCRUD("UPDATE", user_id, user_age + 1)) successful++;
        }

        return successful;
    }

    std::string host_;
    int port_;
    int num_threads_;
    int operations_per_thread_;
};

int main(int argc, char* argv[]) {
    std::string host = "localhost";
    int port = 18647;
    int num_threads = 8;
    int operations_per_thread = 10000;

    // 解析命令行参数
    int opt;
    while ((opt = getopt(argc, argv, "h:p:t:o:")) != -1) {
        switch (opt) {
            case 'h': host = optarg; break;
            case 'p': port = std::stoi(optarg); break;
            case 't': num_threads = std::stoi(optarg); break;
            case 'o': operations_per_thread = std::stoi(optarg); break;
        }
    }

    std::cout << "SQLCC Large Scale Data Accumulation Test" << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "Testing server performance under data accumulation" << std::endl;
    std::cout << "Host: " << host << ":" << port << std::endl;
    std::cout << "Threads: " << num_threads << std::endl;
    std::cout << "Operations per thread: " << operations_per_thread << std::endl;
    std::cout << "Total operations: " << (num_threads * operations_per_thread) << std::endl;
    std::cout << "Expected records: " << (num_threads * operations_per_thread * 3) << std::endl;

    // 检查服务器连接
    std::cout << "\nChecking server connectivity..." << std::endl;
    int test_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (test_sock >= 0) {
        sockaddr_in test_addr{};
        test_addr.sin_family = AF_INET;
        test_addr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &test_addr.sin_addr);

        if (connect(test_sock, (sockaddr*)&test_addr, sizeof(test_addr)) >= 0) {
            std::cout << "✓ Server is accessible" << std::endl;
            close(test_sock);
        } else {
            std::cout << "✗ Cannot connect to server at " << host << ":" << port << std::endl;
            std::cout << "Please start the SQLCC server first:" << std::endl;
            std::cout << "  bazel run //src/sqlcc_server:server_main -t 8" << std::endl;
            return 1;
        }
    }

    // 运行测试
    LargeScalePerformanceTester tester(host, port, num_threads, operations_per_thread);
    auto results = tester.RunDataAccumulationTest();

    // 生成最终报告
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "FINAL PERFORMANCE ANALYSIS" << std::endl;
    std::cout << std::string(80, '=') << std::endl;

    if (!results.empty()) {
        std::cout << "Data Scale Performance Summary:" << std::endl;
        std::cout << std::setw(10) << "Phase" << std::setw(12) << "Records"
                  << std::setw(12) << "Ops/sec" << std::setw(10) << "Latency"
                  << std::setw(12) << "Success%" << std::endl;
        std::cout << std::string(56, '-') << std::endl;

        double baseline_throughput = results.empty() ? 0.0 : results[0].throughput_ops_sec;

        for (const auto& result : results) {
            double degradation = baseline_throughput > 0 ?
                               ((baseline_throughput - result.throughput_ops_sec) / baseline_throughput) * 100.0 : 0.0;

            std::cout << std::setw(8) << result.phase_percentage << "%"
                      << std::setw(12) << result.estimated_records
                      << std::setw(12) << std::fixed << std::setprecision(1) << result.throughput_ops_sec
                      << std::setw(10) << std::fixed << std::setprecision(1) << result.avg_latency_ms
                      << std::setw(12) << std::fixed << std::setprecision(1) << result.success_rate << std::endl;
        }

        const auto& final_result = results.back();
        double total_degradation = baseline_throughput > 0 ?
                                 ((baseline_throughput - final_result.throughput_ops_sec) / baseline_throughput) * 100.0 : 0.0;

        std::cout << "\nOVERALL ASSESSMENT:" << std::endl;
        std::cout << "Total data accumulation: " << (num_threads * operations_per_thread) << " operations" << std::endl;
        std::cout << "Final database records: " << final_result.estimated_records << std::endl;
        std::cout << "Performance degradation: " << std::fixed << std::setprecision(1) << total_degradation << "%" << std::endl;
        std::cout << "Final throughput: " << std::fixed << std::setprecision(1) << final_result.throughput_ops_sec << " ops/sec" << std::endl;
        std::cout << "Final latency: " << std::fixed << std::setprecision(1) << final_result.avg_latency_ms << " ms" << std::endl;

        if (total_degradation < 20.0) {
            std::cout << "Assessment: EXCELLENT - Minimal performance impact" << std::endl;
        } else if (total_degradation < 40.0) {
            std::cout << "Assessment: GOOD - Acceptable performance degradation" << std::endl;
        } else if (total_degradation < 60.0) {
            std::cout << "Assessment: FAIR - Noticeable performance impact" << std::endl;
        } else {
            std::cout << "Assessment: POOR - Severe performance degradation" << std::endl;
        }
    }

    std::cout << "\nTest completed successfully!" << std::endl;
    return 0;
}