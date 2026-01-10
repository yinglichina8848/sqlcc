/**
 * @file auth_test_client.cpp
 * @brief 认证测试客户端
 *
 * 用于测试client-server认证功能的简单客户端程序
 */

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>

#include "network/client_network_manager.h"
#include "network/message_serializer.h"
#include "network/message_types.h"

using namespace sqlcc::network;

void test_auth_success() {
    std::cout << "\n=== Testing Authentication Success ===" << std::endl;

    ClientNetworkManager client("127.0.0.1", 8080);

    // 连接到服务器
    std::cout << "Connecting to server..." << std::endl;
    if (!client.Connect()) {
        std::cerr << "Failed to connect to server" << std::endl;
        return;
    }
    std::cout << "Connected successfully" << std::endl;

    // 发送认证消息
    std::cout << "Sending authentication message (admin/password)..." << std::endl;
    if (!client.SendAuthMessage("admin", "password")) {
        std::cerr << "Failed to send authentication message" << std::endl;
        client.Disconnect();
        return;
    }
    std::cout << "Authentication message sent" << std::endl;

    // 接收认证响应
    std::cout << "Waiting for authentication response..." << std::endl;
    std::vector<char> response = client.ReceiveResponse();

    if (response.empty()) {
        std::cerr << "No response received or connection closed" << std::endl;
        client.Disconnect();
        return;
    }

    // 解析响应
    MessageSerializer serializer;
    uint8_t msg_type, flags;
    uint32_t sequence_id;
    std::vector<char> payload;

    if (serializer.DeserializeMessage(response, msg_type, flags, sequence_id, payload)) {
        std::cout << "Received response - Type: " << (int)msg_type
                 << ", Sequence: " << sequence_id << std::endl;

        if (msg_type == AUTH_ACK) {
            std::string result(payload.begin(), payload.end());
            std::cout << "✅ Authentication SUCCESS: " << result << std::endl;
        } else if (msg_type == ERROR) {
            std::string error(payload.begin(), payload.end());
            std::cout << "❌ Authentication FAILED: " << error << std::endl;
        } else {
            std::cout << "⚠️  Unexpected response type: " << (int)msg_type << std::endl;
        }
    } else {
        std::cerr << "Failed to deserialize response message" << std::endl;
    }

    // 断开连接
    client.Disconnect();
    std::cout << "Disconnected from server" << std::endl;
}

void test_auth_failure() {
    std::cout << "\n=== Testing Authentication Failure ===" << std::endl;

    ClientNetworkManager client("127.0.0.1", 8080);

    // 连接到服务器
    std::cout << "Connecting to server..." << std::endl;
    if (!client.Connect()) {
        std::cerr << "Failed to connect to server" << std::endl;
        return;
    }
    std::cout << "Connected successfully" << std::endl;

    // 发送错误的认证消息
    std::cout << "Sending authentication message (wronguser/wrongpass)..." << std::endl;
    if (!client.SendAuthMessage("wronguser", "wrongpass")) {
        std::cerr << "Failed to send authentication message" << std::endl;
        client.Disconnect();
        return;
    }
    std::cout << "Authentication message sent" << std::endl;

    // 接收认证响应
    std::cout << "Waiting for authentication response..." << std::endl;
    std::vector<char> response = client.ReceiveResponse();

    if (response.empty()) {
        std::cerr << "No response received or connection closed" << std::endl;
        client.Disconnect();
        return;
    }

    // 解析响应
    MessageSerializer serializer;
    uint8_t msg_type, flags;
    uint32_t sequence_id;
    std::vector<char> payload;

    if (serializer.DeserializeMessage(response, msg_type, flags, sequence_id, payload)) {
        std::cout << "Received response - Type: " << (int)msg_type
                 << ", Sequence: " << sequence_id << std::endl;

        if (msg_type == AUTH_ACK) {
            std::string result(payload.begin(), payload.end());
            std::cout << "✅ Authentication SUCCESS: " << result << std::endl;
        } else if (msg_type == ERROR) {
            std::string error(payload.begin(), payload.end());
            std::cout << "❌ Authentication FAILED: " << error << std::endl;
        } else {
            std::cout << "⚠️  Unexpected response type: " << (int)msg_type << std::endl;
        }
    } else {
        std::cerr << "Failed to deserialize response message" << std::endl;
    }

    // 断开连接
    client.Disconnect();
    std::cout << "Disconnected from server" << std::endl;
}

void test_invalid_message_format() {
    std::cout << "\n=== Testing Invalid Message Format ===" << std::endl;

    ClientNetworkManager client("127.0.0.1", 8080);

    // 连接到服务器
    std::cout << "Connecting to server..." << std::endl;
    if (!client.Connect()) {
        std::cerr << "Failed to connect to server" << std::endl;
        return;
    }
    std::cout << "Connected successfully" << std::endl;

    // 发送格式错误的认证消息 (缺少密码部分)
    std::cout << "Sending malformed authentication message..." << std::endl;
    std::string malformed_auth = "admin";  // 只包含用户名，没有分隔符
    std::vector<char> payload(malformed_auth.begin(), malformed_auth.end());

    if (!client.SendRequest(payload)) {
        std::cerr << "Failed to send malformed message" << std::endl;
        client.Disconnect();
        return;
    }
    std::cout << "Malformed message sent" << std::endl;

    // 接收响应
    std::cout << "Waiting for response..." << std::endl;
    std::vector<char> response = client.ReceiveResponse();

    if (response.empty()) {
        std::cerr << "No response received or connection closed" << std::endl;
        client.Disconnect();
        return;
    }

    // 解析响应
    MessageSerializer serializer;
    uint8_t msg_type, flags;
    uint32_t sequence_id;
    std::vector<char> response_payload;

    if (serializer.DeserializeMessage(response, msg_type, flags, sequence_id, response_payload)) {
        std::cout << "Received response - Type: " << (int)msg_type
                 << ", Sequence: " << sequence_id << std::endl;

        if (msg_type == ERROR) {
            std::string error(response_payload.begin(), response_payload.end());
            std::cout << "❌ Server rejected malformed message: " << error << std::endl;
        } else {
            std::cout << "⚠️  Unexpected response to malformed message, type: " << (int)msg_type << std::endl;
        }
    } else {
        std::cerr << "Failed to deserialize response message" << std::endl;
    }

    // 断开连接
    client.Disconnect();
    std::cout << "Disconnected from server" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "SQLCC Authentication Test Client" << std::endl;
    std::cout << "=================================" << std::endl;

    // 等待服务器启动
    std::cout << "Waiting 2 seconds for server to start..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 测试成功的认证
    test_auth_success();

    // 等待一下
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 测试失败的认证
    test_auth_failure();

    // 等待一下
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 测试无效的消息格式
    test_invalid_message_format();

    std::cout << "\n=== Authentication Tests Complete ===" << std::endl;
    std::cout << "All tests finished. Check server logs for detailed results." << std::endl;

    return 0;
}