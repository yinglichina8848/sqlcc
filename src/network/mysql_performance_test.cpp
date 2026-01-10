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

#include "network/mysql_protocol.h"

// 性能测试结果结构
struct PerformanceResult {
    std::string operation;
    int operations_count;
    double total_time_ms;
    double avg_time_ms;
    double ops_per_second;
    int errors_count;
};

// 简化的MySQL客户端类
class SimpleMySQLClient {
public:
    SimpleMySQLClient(const std::string& host, int port)
        : host_(host), port_(port), sock_(-1) {}

    ~SimpleMySQLClient() {
        Disconnect();
    }

    bool Connect() {
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

        return true;
    }

    void Disconnect() {
        if (sock_ >= 0) {
            close(sock_);
            sock_ = -1;
        }
    }

    bool SendData(const std::vector<uint8_t>& data) {
        if (sock_ < 0) return false;

        size_t sent = 0;
        while (sent < data.size()) {
            ssize_t result = send(sock_, data.data() + sent, data.size() - sent, 0);
            if (result <= 0) return false;
            sent += result;
        }
        return true;
    }

    std::vector<uint8_t> ReceiveData(size_t max_size = 4096) {
        if (sock_ < 0) return {};

        std::vector<uint8_t> buffer(max_size);
        ssize_t received = recv(sock_, buffer.data(), buffer.size(), 0);
        if (received <= 0) return {};

        buffer.resize(received);
        return buffer;
    }

    // 发送MySQL握手响应
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

        return SendData(response);
    }

    // 发送查询
    bool SendQuery(const std::string& query) {
        std::vector<uint8_t> packet;
        packet.push_back(query.size() & 0xFF);
        packet.push_back((query.size() >> 8) & 0xFF);
        packet.push_back((query.size() >> 16) & 0xFF);
        packet.push_back(0x00); // sequence
        packet.push_back(0x03); // COM_QUERY
        packet.insert(packet.end(), query.begin(), query.end());

        return SendData(packet);
    }

private:
    std::string host_;
    int port_;
    int sock_;
};

// 性能测试函数
PerformanceResult RunPerformanceTest(SimpleMySQLClient& client,
                                   const std::string& operation_name,
                                   const std::vector<std::string>& operations,
                                   int iterations) {
    PerformanceResult result;
    result.operation = operation_name;
    result.operations_count = iterations;
    result.errors_count = 0;

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        // 随机选择一个操作
        std::string sql = operations[i % operations.size()];

        bool success = client.SendQuery(sql);
        if (success) {
            // 等待响应
            auto response = client.ReceiveData();
            if (response.empty()) {
                result.errors_count++;
            }
        } else {
            result.errors_count++;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    result.total_time_ms = duration.count();
    result.avg_time_ms = result.total_time_ms / iterations;
    result.ops_per_second = (iterations * 1000.0) / result.total_time_ms;

    return result;
}

void PrintPerformanceResult(const PerformanceResult& result) {
    std::cout << "=== " << result.operation << " Performance Test ===" << std::endl;
    std::cout << "Operations: " << result.operations_count << std::endl;
    std::cout << "Total Time: " << result.total_time_ms << " ms" << std::endl;
    std::cout << "Average Time: " << result.avg_time_ms << " ms/op" << std::endl;
    std::cout << "Operations/sec: " << result.ops_per_second << std::endl;
    std::cout << "Errors: " << result.errors_count << std::endl;
    std::cout << "Success Rate: " << ((result.operations_count - result.errors_count) * 100.0 / result.operations_count) << "%" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    std::string host = "localhost";
    int port = 18647;
    int iterations = 50;

    // 解析命令行参数
    if (argc >= 2) host = argv[1];
    if (argc >= 3) port = std::stoi(argv[2]);
    if (argc >= 4) iterations = std::stoi(argv[3]);

    std::cout << "=== SQLCC MySQL Protocol Performance Test ===" << std::endl;
    std::cout << "Server: " << host << ":" << port << std::endl;
    std::cout << "Iterations per test: " << iterations << std::endl;
    std::cout << std::endl;

    SimpleMySQLClient client(host, port);

    // 连接到服务器
    if (!client.Connect()) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }

    std::cout << "Connected to MySQL protocol server" << std::endl;

    // 等待握手包
    auto handshake = client.ReceiveData();
    if (handshake.empty()) {
        std::cerr << "No handshake received" << std::endl;
        client.Disconnect();
        return 1;
    }

    std::cout << "Received handshake (" << handshake.size() << " bytes)" << std::endl;

    // 发送握手响应
    if (!client.SendHandshakeResponse()) {
        std::cerr << "Failed to send handshake response" << std::endl;
        client.Disconnect();
        return 1;
    }

    std::cout << "Handshake response sent" << std::endl;

    // 等待认证结果
    auto auth_response = client.ReceiveData();
    if (!auth_response.empty()) {
        std::cout << "Authentication response received (" << auth_response.size() << " bytes)" << std::endl;
    }

    std::vector<PerformanceResult> results;

    // INSERT性能测试
    std::cout << "Running INSERT performance test..." << std::endl;
    std::vector<std::string> insert_ops;
    for (int i = 0; i < iterations; ++i) {
        std::stringstream ss;
        ss << "INSERT INTO users (id, name, email, age) VALUES ("
           << (i + 1) << ", 'User" << (i + 1) << "', 'user" << (i + 1) << "@example.com', " << (20 + (i % 50)) << ")";
        insert_ops.push_back(ss.str());
    }
    results.push_back(RunPerformanceTest(client, "INSERT", insert_ops, iterations));

    // SELECT性能测试
    std::cout << "Running SELECT performance test..." << std::endl;
    std::vector<std::string> select_ops;
    for (int i = 0; i < iterations; ++i) {
        int user_id = (i % iterations) + 1;
        select_ops.push_back("SELECT * FROM users WHERE id = " + std::to_string(user_id));
    }
    results.push_back(RunPerformanceTest(client, "SELECT", select_ops, iterations));

    // UPDATE性能测试
    std::cout << "Running UPDATE performance test..." << std::endl;
    std::vector<std::string> update_ops;
    for (int i = 0; i < iterations; ++i) {
        int user_id = (i % iterations) + 1;
        update_ops.push_back("UPDATE users SET age = age + 1 WHERE id = " + std::to_string(user_id));
    }
    results.push_back(RunPerformanceTest(client, "UPDATE", update_ops, iterations));

    // DELETE性能测试
    std::cout << "Running DELETE performance test..." << std::endl;
    std::vector<std::string> delete_ops;
    for (int i = iterations; i > 0; --i) {
        delete_ops.push_back("DELETE FROM users WHERE id = " + std::to_string(i));
    }
    results.push_back(RunPerformanceTest(client, "DELETE", delete_ops, iterations));

    // 打印结果
    std::cout << "=== MYSQL PROTOCOL PERFORMANCE TEST RESULTS ===" << std::endl;
    for (const auto& result : results) {
        PrintPerformanceResult(result);
    }

    // 断开连接
    client.Disconnect();

    std::cout << "MySQL protocol performance test completed!" << std::endl;
    return 0;
}