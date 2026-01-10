#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include "network/mysql_protocol.h"

int main() {
    // 创建socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    // 连接到服务器
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(3306); // MySQL默认端口
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "Connected to MySQL server" << std::endl;

    // 读取握手包
    uint8_t packet_length[3];
    uint8_t sequence_id;

    if (read(sock, packet_length, 3) != 3) {
        std::cerr << "Failed to read packet length" << std::endl;
        close(sock);
        return 1;
    }

    if (read(sock, &sequence_id, 1) != 1) {
        std::cerr << "Failed to read sequence id" << std::endl;
        close(sock);
        return 1;
    }

    uint32_t length = packet_length[0] | (packet_length[1] << 8) | (packet_length[2] << 16);
    std::vector<uint8_t> handshake_packet(length);

    if (read(sock, handshake_packet.data(), length) != static_cast<ssize_t>(length)) {
        std::cerr << "Failed to read handshake packet" << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "Received handshake packet, length: " << length << std::endl;

    // 简单验证握手包格式
    if (length >= 36) {
        uint8_t protocol_version = handshake_packet[0];
        std::cout << "Protocol version: " << static_cast<int>(protocol_version) << std::endl;

        // 查找服务器版本字符串
        size_t version_start = 1;
        size_t version_end = version_start;
        while (version_end < length && handshake_packet[version_end] != 0) {
            version_end++;
        }

        if (version_end < length) {
            std::string server_version(reinterpret_cast<char*>(&handshake_packet[version_start]),
                                     version_end - version_start);
            std::cout << "Server version: " << server_version << std::endl;
        }
    }

    // 发送简单的认证包（用户名：root，无密码）
    std::vector<uint8_t> auth_packet;
    auth_packet.push_back(0x00); // packet length (will be set later)
    auth_packet.push_back(0x00);
    auth_packet.push_back(0x00);
    auth_packet.push_back(0x01); // sequence id

    // 客户端能力标志
    uint32_t client_flags = 0x00000001; // CLIENT_PROTOCOL_41
    auth_packet.push_back(client_flags & 0xFF);
    auth_packet.push_back((client_flags >> 8) & 0xFF);
    auth_packet.push_back((client_flags >> 16) & 0xFF);
    auth_packet.push_back((client_flags >> 24) & 0xFF);

    // 最大包长度
    uint32_t max_packet_size = 0x01000000;
    auth_packet.push_back(max_packet_size & 0xFF);
    auth_packet.push_back((max_packet_size >> 8) & 0xFF);
    auth_packet.push_back((max_packet_size >> 16) & 0xFF);
    auth_packet.push_back((max_packet_size >> 24) & 0xFF);

    // 字符集
    auth_packet.push_back(33); // utf8mb4

    // 填充
    for (int i = 0; i < 23; i++) {
        auth_packet.push_back(0);
    }

    // 用户名
    const char* username = "root";
    for (size_t i = 0; i < strlen(username); i++) {
        auth_packet.push_back(username[i]);
    }
    auth_packet.push_back(0); // null terminator

    // 密码（空）
    auth_packet.push_back(0);

    // 数据库名（空）
    // auth_packet.push_back(0);

    // 设置包长度
    uint32_t packet_len = auth_packet.size() - 4;
    auth_packet[0] = packet_len & 0xFF;
    auth_packet[1] = (packet_len >> 8) & 0xFF;
    auth_packet[2] = (packet_len >> 16) & 0xFF;

    // 发送认证包
    if (write(sock, auth_packet.data(), auth_packet.size()) != static_cast<ssize_t>(auth_packet.size())) {
        std::cerr << "Failed to send auth packet" << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "Sent authentication packet" << std::endl;

    // 读取响应
    if (read(sock, packet_length, 3) == 3) {
        if (read(sock, &sequence_id, 1) == 1) {
            length = packet_length[0] | (packet_length[1] << 8) | (packet_length[2] << 16);
            std::vector<uint8_t> response(length);

            if (read(sock, response.data(), length) == static_cast<ssize_t>(length)) {
                std::cout << "Received response, length: " << length << std::endl;

                if (length > 0 && response[0] == 0x00) {
                    std::cout << "Authentication successful!" << std::endl;
                } else if (length > 0 && response[0] == 0xFF) {
                    std::cout << "Authentication failed" << std::endl;
                } else {
                    std::cout << "Unknown response type" << std::endl;
                }
            }
        }
    }

    close(sock);
    std::cout << "Test completed" << std::endl;
    return 0;
}