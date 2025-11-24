/**
 * @file client_main.cpp
 * @brief SQLCC网络客户端主程序
 * 
 * 该文件实现了SQLCC网络客户端的主程序入口，用于连接数据库服务器、
 * 认证并发送测试查询。
 */

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <unistd.h>

#include "version.h"
#include "network/network.h"
#include "network/encryption.h"

#define SQLCC_VERSION "0.6.2"

using namespace sqlcc::network;

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    int port = 18647;
    std::string username = "admin";
    std::string password = "password";
    
    // 解析命令行参数
    int opt;
    while ((opt = getopt(argc, argv, "h:p:u:P:")) != -1) {
        switch (opt) {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = std::stoi(optarg);
                break;
            case 'u':
                username = optarg;
                break;
            case 'P':
                password = optarg;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-h host] [-p port] [-u username] [-P password]" << std::endl;
                return 1;
        }
    }
    
    std::cout << "SqlCC Network Client connecting to " << host << ":" << port << std::endl;
    
    // 创建客户端网络管理器
    ClientNetworkManager client(host, port);
    
    // 连接并认证
    std::cout << "Attempting to connect and authenticate..." << std::endl;
    if (!client.ConnectAndAuthenticate(username, password)) {
        std::cerr << "Failed to connect and authenticate to server" << std::endl;
        return 1;
    }
    
    std::cout << "Successfully connected and authenticated to server" << std::endl;
    
    // 发送测试查询
    std::string query = "SELECT * FROM test_table";
    std::cout << "Sending test query: " << query << std::endl;
    
    MessageHeader query_header;
    query_header.magic = 0x53514C43; // 'SQLC'
    query_header.length = query.length();
    query_header.type = QUERY;
    query_header.flags = 0;
    query_header.sequence_id = 3;
    
    std::vector<char> query_msg(sizeof(MessageHeader) + query.length());
    std::memcpy(query_msg.data(), &query_header, sizeof(MessageHeader));
    std::memcpy(query_msg.data() + sizeof(MessageHeader), query.c_str(), query.length());
    
    if (!client.SendRequest(query_msg)) {
        std::cerr << "Failed to send query to server" << std::endl;
        client.Disconnect();
        return 1;
    }
    
    std::cout << "Sent query: " << query << std::endl;
    
    // 接收响应
    std::cout << "Waiting for response..." << std::endl;
    std::vector<char> response = client.ReceiveResponse();
    if (response.size() < sizeof(MessageHeader)) {
        std::cerr << "Invalid response from server, size: " << response.size() << std::endl;
        client.Disconnect();
        return 1;
    }
    
    MessageHeader* response_header = reinterpret_cast<MessageHeader*>(response.data());
    std::cout << "Received response with type: " << response_header->type << ", length: " << response_header->length << std::endl;
    
    if (response_header->type == QUERY_RESULT) {
        std::string result(response.data() + sizeof(MessageHeader), response_header->length);
        std::cout << "Received result: " << result << std::endl;
    } else if (response_header->type == ERROR) {
        std::string error(response.data() + sizeof(MessageHeader), response_header->length);
        std::cerr << "Received error: " << error << std::endl;
    } else {
        std::cerr << "Unexpected response type: " << response_header->type << std::endl;
    }
    
    // 断开连接
    client.Disconnect();
    std::cout << "Disconnected from server" << std::endl;
    
    return 0;
}

// 在 network 命名空间内实现 ConnectAndAuthenticate 方法
namespace sqlcc::network {
    bool ClientNetworkManager::ConnectAndAuthenticate(const std::string& username, const std::string& password) {
        // 连接到服务器
        if (!this->Connect()) {
            return false;
        }

        // 发送连接请求
        std::vector<char> connect_msg(sizeof(MessageHeader));
        MessageHeader* header = reinterpret_cast<MessageHeader*>(connect_msg.data());
        header->magic = 0x53514C43; // 'SQLC'
        header->length = 0;
        header->type = CONNECT;
        header->flags = 0;
        header->sequence_id = 1;

        if (!this->SendRequest(connect_msg)) {
            this->Disconnect();
            return false;
        }

        // 接收连接确认
        std::vector<char> connect_resp = this->ReceiveResponse();
        if (connect_resp.empty()) {
            this->Disconnect();
            return false;
        }

        MessageHeader* resp_header = reinterpret_cast<MessageHeader*>(connect_resp.data());
        if (resp_header->magic != 0x53514C43 || resp_header->type != CONN_ACK) {
            this->Disconnect();
            return false;
        }

        // 发送认证请求
        size_t user_len = username.length();
        size_t pass_len = password.length();
        size_t msg_len = 2 * sizeof(uint32_t) + user_len + pass_len;

        std::vector<char> auth_msg(sizeof(MessageHeader) + msg_len);
        header = reinterpret_cast<MessageHeader*>(auth_msg.data());
        header->magic = 0x53514C43; // 'SQLC'
        header->length = msg_len;
        header->type = AUTH;
        header->flags = 0;
        header->sequence_id = 2;

        char* body = auth_msg.data() + sizeof(MessageHeader);
        *reinterpret_cast<uint32_t*>(body) = user_len;
        *reinterpret_cast<uint32_t*>(body + sizeof(uint32_t)) = pass_len;
        std::memcpy(body + 2 * sizeof(uint32_t), username.c_str(), user_len);
        std::memcpy(body + 2 * sizeof(uint32_t) + user_len, password.c_str(), pass_len);

        if (!this->SendRequest(auth_msg)) {
            this->Disconnect();
            return false;
        }

        // 接收认证响应
        std::vector<char> auth_resp = this->ReceiveResponse();
        if (auth_resp.empty()) {
            this->Disconnect();
            return false;
        }

        resp_header = reinterpret_cast<MessageHeader*>(auth_resp.data());
        if (resp_header->magic != 0x53514C43 || resp_header->type != AUTH_ACK) {
            return false;
        }

        return true;
    }
}