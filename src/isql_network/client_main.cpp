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
#include <fstream>
#include <sstream>

// 移除不存在的头文件引用
// #include "version.h"

#include "network/network.h"
#include "network/encryption.h"

#define SQLCC_VERSION "0.6.2"

using namespace sqlcc::network;

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    int port = 18647;
    std::string username = "admin";
    std::string password = "password";
    bool enable_encryption = false;  // 加密开关

    // 解析命令行参数
    std::string sql_command;
    std::string sql_file;
    bool execute_mode = false;
    bool file_mode = false;
    int opt;
    while ((opt = getopt(argc, argv, "h:p:u:P:eE:f:")) != -1) {
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
            case 'e':
                enable_encryption = true;  // 启用加密
                break;
            case 'E':
                execute_mode = true;
                sql_command = optarg;  // 直接执行SQL命令
                break;
            case 'f':
                file_mode = true;
                sql_file = optarg;  // 执行SQL脚本文件
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-h host] [-p port] [-u username] [-P password] [-e] [-E sql_command] [-f sql_file]" << std::endl;
                std::cerr << "  -e: Enable AES-256 encryption for all connections" << std::endl;
                std::cerr << "  -E sql_command: Execute SQL command directly" << std::endl;
                std::cerr << "  -f sql_file: Execute SQL commands from file" << std::endl;
                return 1;
        }
    }

    std::cout << "[DEBUG] Starting ISQL client with parameters:" << std::endl;
    std::cout << "[DEBUG]   Host: " << host << std::endl;
    std::cout << "[DEBUG]   Port: " << port << std::endl;
    std::cout << "[DEBUG]   Username: " << username << std::endl;
    std::cout << "[DEBUG]   Password: " << password << std::endl;
    std::cout << "[DEBUG]   Encryption: " << (enable_encryption ? "enabled" : "disabled") << std::endl;
    std::cout << "[DEBUG]   Execute mode: " << (execute_mode ? "yes" : "no") << std::endl;
    std::cout << "[DEBUG]   File mode: " << (file_mode ? "yes" : "no") << std::endl;
    if (execute_mode) std::cout << "[DEBUG]   SQL command: " << sql_command << std::endl;
    if (file_mode) std::cout << "[DEBUG]   SQL file: " << sql_file << std::endl;

    std::cout << "SqlCC Network Client connecting to " << host << ":" << port << std::endl;
    if (enable_encryption) {
        std::cout << "[加密模式] 启用AES-256-CBC加密通信" << std::endl;
    }
    
    // 创建客户端网络管理器
    ClientNetworkManager client(host, port);
    
    // 连接并认证
    std::cout << "Attempting to connect and authenticate..." << std::endl;
    if (!client.ConnectAndAuthenticate(username, password)) {
        std::cerr << "Failed to connect and authenticate to server" << std::endl;
        return 1;
    }
    
    std::cout << "Successfully connected and authenticated to server" << std::endl;
    
    // 如果启用加密，则启动密钥交换
    if (enable_encryption) {
        std::cout << "[加密] 发起密钥交换..." << std::endl;
        if (!client.InitiateKeyExchange()) {
            std::cerr << "[加密] 密钥交换失败" << std::endl;
            client.Disconnect();
            return 1;
        }
        std::cout << "[加密] 密钥交换成功，已启用AES-256-CBC加密" << std::endl;
    }
    
    // 准备SQL命令列表
    std::vector<std::string> sql_commands;

    if (file_mode && !sql_file.empty()) {
        // 从文件读取SQL命令
        std::ifstream file(sql_file);
        if (!file.is_open()) {
            std::cerr << "Failed to open SQL file: " << sql_file << std::endl;
            client.Disconnect();
            return 1;
        }

        std::string line;
        std::string current_command;
        while (std::getline(file, line)) {
            // 移除行首尾空白字符
            line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
            line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), line.end());

            // 跳过空行和注释行
            if (line.empty() || line.substr(0, 2) == "--") {
                continue;
            }

            current_command += line + " ";

            // 检查是否是完整SQL语句（以分号结束）
            if (!line.empty() && line.back() == ';') {
                // 移除末尾的分号和多余空格
                current_command = current_command.substr(0, current_command.find_last_of(';'));
                current_command.erase(current_command.begin(),
                    std::find_if(current_command.begin(), current_command.end(),
                        [](unsigned char ch) { return !std::isspace(ch); }));
                current_command.erase(std::find_if(current_command.rbegin(), current_command.rend(),
                    [](unsigned char ch) { return !std::isspace(ch); }).base(),
                    current_command.end());

                if (!current_command.empty()) {
                    sql_commands.push_back(current_command);
                }
                current_command.clear();
            }
        }

        // 处理最后一个没有分号的命令
        if (!current_command.empty()) {
            current_command.erase(current_command.begin(),
                std::find_if(current_command.begin(), current_command.end(),
                    [](unsigned char ch) { return !std::isspace(ch); }));
            current_command.erase(std::find_if(current_command.rbegin(), current_command.rend(),
                [](unsigned char ch) { return !std::isspace(ch); }).base(),
                current_command.end());

            if (!current_command.empty()) {
                sql_commands.push_back(current_command);
            }
        }

        file.close();
        std::cout << "Loaded " << sql_commands.size() << " SQL commands from file: " << sql_file << std::endl;

    } else if (execute_mode && !sql_command.empty()) {
        // 单个SQL命令
        sql_commands.push_back(sql_command);
        std::cout << "Executing SQL command: " << sql_command << std::endl;
    } else {
        // 默认测试查询
        sql_commands.push_back("SELECT 1");
        std::cout << "Sending test query: SELECT 1" << std::endl;
    }

    // 执行所有SQL命令
    uint32_t sequence_id = 3;
    bool has_error = false;

    for (size_t i = 0; i < sql_commands.size(); ++i) {
        const std::string& query = sql_commands[i];

        if (sql_commands.size() > 1) {
            std::cout << "[" << (i + 1) << "/" << sql_commands.size() << "] Executing: " << query << std::endl;
        }

        // 构建查询消息
        MessageHeader query_header;
        query_header.magic = 0x53514C43; // 'SQLC'
        query_header.length = query.length();
        query_header.type = QUERY;
        query_header.flags = 0;
        query_header.sequence_id = sequence_id++;

        std::vector<char> query_msg(sizeof(MessageHeader) + query.length());
        std::memcpy(query_msg.data(), &query_header, sizeof(MessageHeader));
        std::memcpy(query_msg.data() + sizeof(MessageHeader), query.c_str(), query.length());

        // 发送查询
        if (!client.SendRequest(query_msg)) {
            std::cerr << "Failed to send query to server" << std::endl;
            has_error = true;
            break;
        }

        // 接收响应
        std::vector<char> response = client.ReceiveResponse();
        if (response.size() < sizeof(MessageHeader)) {
            std::cerr << "Invalid response from server, size: " << response.size() << std::endl;
            has_error = true;
            break;
        }

        MessageHeader* response_header = reinterpret_cast<MessageHeader*>(response.data());

        if (response_header->type == QUERY_RESULT) {
            std::string result(response.data() + sizeof(MessageHeader), response_header->length);
            if (sql_commands.size() > 1) {
                std::cout << "✓ Result: " << result << std::endl;
            } else {
                std::cout << "Received result: " << result << std::endl;
            }
        } else if (response_header->type == ERROR) {
            std::string error(response.data() + sizeof(MessageHeader), response_header->length);
            std::cerr << "✗ Error: " << error << std::endl;
            has_error = true;
        } else {
            std::cerr << "✗ Unexpected response type: " << response_header->type << std::endl;
            has_error = true;
        }

        // 如果是文件模式且遇到错误，继续执行其他命令
        if (file_mode && has_error) {
            std::cout << "Continuing with remaining commands despite error..." << std::endl;
            has_error = false; // 重置错误标志，继续执行
        }
    }

    if (has_error && !file_mode) {
        client.Disconnect();
        return 1;
    }
    
    // 断开连接
    client.Disconnect();
    std::cout << "Disconnected from server" << std::endl;
    
    return 0;
}

// 在 network 命名空间内实现 ConnectAndAuthenticate 方法
namespace sqlcc::network {
    bool ClientNetworkManager::ConnectAndAuthenticate(const std::string& username, const std::string& password) {
        std::cout << "[DEBUG] Attempting to connect to server..." << std::endl;

        // 连接到服务器
        if (!this->Connect()) {
            std::cout << "[DEBUG] Connection failed" << std::endl;
            return false;
        }

        std::cout << "[DEBUG] Connected successfully, sending CONNECT message..." << std::endl;

        // 发送连接请求
        std::vector<char> connect_msg(sizeof(MessageHeader));
        MessageHeader* header = reinterpret_cast<MessageHeader*>(connect_msg.data());
        header->magic = 0x53514C43; // 'SQLC'
        header->length = 0;
        header->type = CONNECT;
        header->flags = 0x02; // Disable authentication for testing
        header->sequence_id = 1;

        std::cout << "[DEBUG] Sending CONNECT message (magic=0x" << std::hex << header->magic
                  << ", type=" << std::dec << (int)header->type
                  << ", flags=0x" << std::hex << (int)header->flags
                  << ", seq=" << std::dec << header->sequence_id << ")" << std::endl;

        if (!this->SendRequest(connect_msg)) {
            std::cout << "[DEBUG] Failed to send CONNECT message" << std::endl;
            this->Disconnect();
            return false;
        }

        std::cout << "[DEBUG] CONNECT message sent, waiting for CONN_ACK..." << std::endl;

        // 接收连接确认
        std::vector<char> connect_resp = this->ReceiveResponse();
        if (connect_resp.empty()) {
            std::cout << "[DEBUG] No response received for CONNECT" << std::endl;
            this->Disconnect();
            return false;
        }

        std::cout << "[DEBUG] Received response of " << connect_resp.size() << " bytes" << std::endl;

        MessageHeader* resp_header = reinterpret_cast<MessageHeader*>(connect_resp.data());
        std::cout << "[DEBUG] Response header: magic=0x" << std::hex << resp_header->magic
                  << ", type=" << std::dec << (int)resp_header->type
                  << ", length=" << resp_header->length
                  << ", flags=0x" << std::hex << (int)resp_header->flags
                  << ", seq=" << std::dec << resp_header->sequence_id << std::endl;

        if (resp_header->magic != 0x53514C43) {
            std::cout << "[DEBUG] Invalid magic number in response" << std::endl;
            this->Disconnect();
            return false;
        }

        if (resp_header->type != CONN_ACK) {
            std::cout << "[DEBUG] Expected CONN_ACK (" << CONN_ACK << "), got " << (int)resp_header->type << std::endl;
            this->Disconnect();
            return false;
        }

        std::cout << "[DEBUG] CONN_ACK received successfully" << std::endl;

        // 发送认证请求
        size_t user_len = username.length();
        size_t pass_len = password.length();
        size_t msg_len = 2 * sizeof(uint32_t) + user_len + pass_len;

        std::cout << "[DEBUG] Sending AUTH message (user='" << username << "', pass='" << password << "')..." << std::endl;

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

        std::cout << "[DEBUG] AUTH message: user_len=" << user_len << ", pass_len=" << pass_len << ", total_len=" << msg_len << std::endl;

        if (!this->SendRequest(auth_msg)) {
            std::cout << "[DEBUG] Failed to send AUTH message" << std::endl;
            this->Disconnect();
            return false;
        }

        std::cout << "[DEBUG] AUTH message sent, waiting for AUTH_ACK..." << std::endl;

        // 接收认证响应
        std::vector<char> auth_resp = this->ReceiveResponse();
        if (auth_resp.empty()) {
            std::cout << "[DEBUG] No response received for AUTH" << std::endl;
            this->Disconnect();
            return false;
        }

        std::cout << "[DEBUG] Received AUTH response of " << auth_resp.size() << " bytes" << std::endl;

        resp_header = reinterpret_cast<MessageHeader*>(auth_resp.data());
        std::cout << "[DEBUG] AUTH response header: magic=0x" << std::hex << resp_header->magic
                  << ", type=" << std::dec << (int)resp_header->type
                  << ", length=" << resp_header->length
                  << ", flags=0x" << std::hex << (int)resp_header->flags
                  << ", seq=" << std::dec << resp_header->sequence_id << std::endl;

        if (resp_header->magic != 0x53514C43) {
            std::cout << "[DEBUG] Invalid magic number in AUTH response" << std::endl;
            return false;
        }

        if (resp_header->type != AUTH_ACK) {
            std::cout << "[DEBUG] Expected AUTH_ACK (" << AUTH_ACK << "), got " << (int)resp_header->type << std::endl;
            return false;
        }

        // 检查认证结果标志位：0表示成功，1表示失败
        if (resp_header->flags != 0) {
            std::cout << "[DEBUG] Authentication failed (flags=" << (int)resp_header->flags << ")" << std::endl;
            return false;
        }

        std::cout << "[DEBUG] Authentication successful" << std::endl;
        return true;
    }
}
