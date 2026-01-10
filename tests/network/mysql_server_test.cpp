#include <iostream>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include "network/mysql_protocol.h"

void handle_client(int client_sock) {
    std::cout << "Client connected, handling MySQL protocol..." << std::endl;

    // 创建MySQL协议处理器
    MySQLProtocolHandler protocol_handler((sqlcc::FileDescriptor(client_sock)));

    // 发送握手
    protocol_handler.send_handshake();
    std::cout << "Handshake sent to client" << std::endl;

    // 处理客户端响应
    if (protocol_handler.handle_client_response()) {
        std::cout << "Client authentication successful" << std::endl;

        // 发送OK包
        uint8_t ok_packet[5] = {0x00, 0x00, 0x00, 0x02, 0x00}; // OK packet
        write(client_sock, ok_packet, sizeof(ok_packet));

        // 简单的主循环，等待查询
        while (true) {
            uint8_t packet_length[3];
            uint8_t sequence_id;

            if (read(client_sock, packet_length, 3) != 3) {
                break;
            }

            if (read(client_sock, &sequence_id, 1) != 1) {
                break;
            }

            uint32_t length = packet_length[0] | (packet_length[1] << 8) | (packet_length[2] << 16);
            std::vector<uint8_t> query_packet(length);

            if (read(client_sock, query_packet.data(), length) != static_cast<ssize_t>(length)) {
                break;
            }

            std::cout << "Received query packet, length: " << length << std::endl;

            // 简单的响应：发送一个基本的OK包
            uint8_t response[7] = {0x01, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00};
            write(client_sock, response, sizeof(response));
        }
    } else {
        std::cout << "Client authentication failed" << std::endl;

        // 发送错误包
        uint8_t err_packet[13] = {0x17, 0x00, 0x00, 0x02, 0xFF, 0x15, 0x04, 0x23, 0x32, 0x38, 0x30, 0x30, 0x30};
        write(client_sock, err_packet, sizeof(err_packet));
    }

    close(client_sock);
    std::cout << "Client connection closed" << std::endl;
}

int main() {
    // 创建监听socket
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        std::cerr << "Server socket creation failed" << std::endl;
        return 1;
    }

    // 设置socket选项
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定地址
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(18647); // 使用SQLCC默认端口

    if (bind(server_sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close(server_sock);
        return 1;
    }

    // 监听
    if (listen(server_sock, 5) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close(server_sock);
        return 1;
    }

    std::cout << "MySQL protocol test server listening on port 18647..." << std::endl;

    while (true) {
        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);

        int client_sock = accept(server_sock, (sockaddr*)&client_addr, &addr_len);
        if (client_sock < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        std::cout << "Accepted connection from client" << std::endl;

        // 为每个客户端创建新线程处理
        std::thread client_thread(handle_client, client_sock);
        client_thread.detach();
    }

    close(server_sock);
    return 0;
}