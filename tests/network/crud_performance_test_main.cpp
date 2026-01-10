#include "network/network.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <atomic>
#include <sstream>

using namespace sqlcc::network;

// 性能测试结果结构
struct PerformanceResult {
    std::string operation;
    int operations_count;
    double total_time_ms;
    double avg_time_ms;
    double ops_per_second;
    int errors_count;
};

// 性能测试器类
class CRUDPerformanceTester {
public:
    CRUDPerformanceTester(const std::string& host, int port)
        : host_(host), port_(port), client_(host, port) {}

    bool ConnectAndAuthenticate() {
        try {
            // 连接到服务器
            if (!client_.Connect()) {
                std::cerr << "Failed to connect to server" << std::endl;
                return false;
            }

            // 发送认证消息
            if (!client_.SendAuthMessage("admin", "password")) {
                std::cerr << "Authentication message send failed" << std::endl;
                return false;
            }

            // 等待认证响应
            auto response = client_.ReceiveResponse();
            if (response.empty()) {
                std::cerr << "No authentication response received" << std::endl;
                return false;
            }

            std::string auth_result(response.begin(), response.end());
            if (auth_result.find("authenticated") != std::string::npos ||
                auth_result.find("success") != std::string::npos) {
                std::cout << "Connected and authenticated successfully" << std::endl;
                return true;
            } else {
                std::cerr << "Authentication failed: " << auth_result << std::endl;
                return false;
            }
        } catch (const std::exception& e) {
            std::cerr << "Connection error: " << e.what() << std::endl;
            return false;
        }
    }

    void Disconnect() {
        client_.Disconnect();
    }

    // 执行DDL操作
    bool ExecuteDDL(const std::string& sql) {
        try {
            // 发送查询
            std::vector<char> request(sql.begin(), sql.end());
            if (!client_.SendRequest(request)) {
                return false;
            }

            // 接收响应
            auto response = client_.ReceiveResponse();
            if (response.empty()) {
                return false;
            }

            std::string result(response.begin(), response.end());
            return result.find("successfully") != std::string::npos;
        } catch (const std::exception& e) {
            std::cerr << "DDL execution error: " << e.what() << std::endl;
            return false;
        }
    }

    // 执行DML操作
    bool ExecuteDML(const std::string& sql) {
        try {
            std::vector<char> request(sql.begin(), sql.end());
            if (!client_.SendRequest(request)) {
                return false;
            }

            auto response = client_.ReceiveResponse();
            if (response.empty()) {
                return false;
            }

            std::string result(response.begin(), response.end());
            return result.find("successfully") != std::string::npos;
        } catch (const std::exception& e) {
            std::cerr << "DML execution error: " << e.what() << std::endl;
            return false;
        }
    }

    // 执行SELECT查询
    std::string ExecuteSelect(const std::string& sql) {
        try {
            std::vector<char> request(sql.begin(), sql.end());
            if (!client_.SendRequest(request)) {
                return "";
            }

            auto response = client_.ReceiveResponse();
            if (response.empty()) {
                return "";
            }

            return std::string(response.begin(), response.end());
        } catch (const std::exception& e) {
            std::cerr << "SELECT execution error: " << e.what() << std::endl;
            return "";
        }
    }

public:
    // 直接访问ClientNetworkManager的方法
    bool Connect() { return client_.Connect(); }
    bool SendAuthMessage(const std::string& username, const std::string& password) {
        return client_.SendAuthMessage(username, password);
    }
    std::vector<char> ReceiveResponse() {
        return client_.ReceiveResponse();
    }

private:
    std::string host_;
    int port_;
    ClientNetworkManager client_;
};

// 性能测试函数
PerformanceResult RunPerformanceTest(CRUDPerformanceTester& tester,
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

        bool success = false;
        if (operation_name == "CREATE" || operation_name == "INSERT") {
            success = tester.ExecuteDML(sql);
        } else if (operation_name == "SELECT") {
            std::string result = tester.ExecuteSelect(sql);
            success = !result.empty() && result.find("Query executed") != std::string::npos;
        } else if (operation_name == "UPDATE" || operation_name == "DELETE") {
            success = tester.ExecuteDML(sql);
        }

        if (!success) {
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
    int iterations = 100;

    // 解析命令行参数
    if (argc >= 2) host = argv[1];
    if (argc >= 3) port = std::stoi(argv[2]);
    if (argc >= 4) iterations = std::stoi(argv[3]);

    std::cout << "=== SQLCC CRUD Performance Test ===" << std::endl;
    std::cout << "Server: " << host << ":" << port << std::endl;
    std::cout << "Iterations per test: " << iterations << std::endl;
    std::cout << std::endl;

    CRUDPerformanceTester tester(host, port);

    // 连接到服务器（不使用加密）
    if (!tester.Connect()) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }

    // 发送认证消息
    if (!tester.SendAuthMessage("admin", "password")) {
        std::cerr << "Authentication message send failed" << std::endl;
        tester.Disconnect();
        return 1;
    }

    // 等待认证响应
    auto auth_response = tester.ReceiveResponse();
    if (auth_response.empty()) {
        std::cerr << "No authentication response received" << std::endl;
        tester.Disconnect();
        return 1;
    }

    std::string auth_result(auth_response.begin(), auth_response.end());
    if (auth_result.find("OK") == std::string::npos &&
        auth_result.find("authenticated") == std::string::npos &&
        auth_result.find("success") == std::string::npos) {
        std::cerr << "Authentication failed: " << auth_result << std::endl;
        tester.Disconnect();
        return 1;
    }

    std::cout << "Connected and authenticated successfully (MySQL protocol, no encryption)" << std::endl;

    // 初始化数据库和表
    std::cout << "Initializing test database..." << std::endl;
    tester.ExecuteDDL("CREATE DATABASE perftest");
    tester.ExecuteDDL("CREATE TABLE users (id INT PRIMARY KEY, name VARCHAR(50), email VARCHAR(100), age INT)");

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
    results.push_back(RunPerformanceTest(tester, "INSERT", insert_ops, iterations));

    // SELECT性能测试
    std::cout << "Running SELECT performance test..." << std::endl;
    std::vector<std::string> select_ops;
    for (int i = 0; i < iterations; ++i) {
        int user_id = (i % iterations) + 1;
        select_ops.push_back("SELECT * FROM users WHERE id = " + std::to_string(user_id));
    }
    results.push_back(RunPerformanceTest(tester, "SELECT", select_ops, iterations));

    // UPDATE性能测试
    std::cout << "Running UPDATE performance test..." << std::endl;
    std::vector<std::string> update_ops;
    for (int i = 0; i < iterations; ++i) {
        int user_id = (i % iterations) + 1;
        update_ops.push_back("UPDATE users SET age = age + 1 WHERE id = " + std::to_string(user_id));
    }
    results.push_back(RunPerformanceTest(tester, "UPDATE", update_ops, iterations));

    // DELETE性能测试
    std::cout << "Running DELETE performance test..." << std::endl;
    std::vector<std::string> delete_ops;
    for (int i = iterations; i > 0; --i) {
        delete_ops.push_back("DELETE FROM users WHERE id = " + std::to_string(i));
    }
    results.push_back(RunPerformanceTest(tester, "DELETE", delete_ops, iterations));

    // 打印结果
    std::cout << "=== PERFORMANCE TEST RESULTS ===" << std::endl;
    for (const auto& result : results) {
        PrintPerformanceResult(result);
    }

    // 清理测试数据
    std::cout << "Cleaning up test data..." << std::endl;
    tester.ExecuteDDL("DROP TABLE users");
    tester.ExecuteDDL("DROP DATABASE perftest");

    // 断开连接
    tester.Disconnect();

    std::cout << "Performance test completed!" << std::endl;
    return 0;
}