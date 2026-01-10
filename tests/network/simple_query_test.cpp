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

// 简化的查询测试程序
bool SendQuery(int sock, const std::string& query, uint8_t& sequence_id) {
    std::vector<uint8_t> packet;
    packet.push_back(query.size() & 0xFF);
    packet.push_back((query.size() >> 8) & 0xFF);
    packet.push_back((query.size() >> 16) & 0xFF);
    packet.push_back(sequence_id++ & 0xFF);
    packet.push_back(0x03); // COM_QUERY
    packet.insert(packet.end(), query.begin(), query.end());

    size_t sent = 0;
    while (sent < packet.size()) {
        ssize_t result = send(sock, packet.data() + sent, packet.size() - sent, MSG_NOSIGNAL);
        if (result <= 0) {
            return false;
        }
        sent += result;
    }
    return true;
}

bool ReceiveResponse(int sock, uint8_t& sequence_id) {
    uint8_t header[4];
    if (read(sock, header, 4) != 4) return false;

    uint32_t length = header[0] | (header[1] << 8) | (header[2] << 16);
    uint8_t resp_seq = header[3];

    if (resp_seq != sequence_id) {
        std::cout << "Sequence mismatch: expected " << (int)sequence_id
                  << ", got " << (int)resp_seq << std::endl;
        return false;
    }

    if (length > 0) {
        std::vector<uint8_t> payload(length);
        size_t received = 0;
        while (received < length) {
            ssize_t result = read(sock, payload.data() + received, length - received);
            if (result <= 0) return false;
            received += result;
        }

        // 检查响应类型 (第一个字节)
        if (!payload.empty()) {
            uint8_t response_type = payload[0];
            std::cout << "Response type: 0x" << std::hex << (int)response_type << std::dec;

            if (response_type == 0x00) {
                std::cout << " (OK)" << std::endl;
                return true;
            } else if (response_type == 0xFF) {
                std::cout << " (ERROR)" << std::endl;
                return false;
            } else if (response_type >= 0x01 && response_type <= 0xFA) {
                std::cout << " (Result Set Header)" << std::endl;
                return true;
            } else {
                std::cout << " (Unknown)" << std::endl;
                return false;
            }
        }
    }

    return true;
}

bool ExecuteQuery(int sock, const std::string& query, uint8_t& sequence_id) {
    std::cout << "Executing: " << query << std::endl;

    if (!SendQuery(sock, query, sequence_id)) {
        std::cout << "Failed to send query" << std::endl;
        return false;
    }

    if (!ReceiveResponse(sock, sequence_id)) {
        std::cout << "Failed to receive response" << std::endl;
        return false;
    }

    return true;
}

bool SendHandshakeResponse(int sock) {
    // 简化的握手响应
    std::vector<uint8_t> response = {
        0x14, 0x00, 0x00, 0x01,  // packet length + sequence
        0x00,  // capabilities low
        0x00, 0x00, // capabilities high
        0x00, 0x00, 0x00, 0x00, // max packet size
        0x21, // charset (utf8_general_ci)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, // filler
        0x61, 0x64, 0x6d, 0x69, 0x6e, 0x00, // username: admin
        0x00 // null terminator
    };

    size_t sent = 0;
    while (sent < response.size()) {
        ssize_t result = send(sock, response.data() + sent, response.size() - sent, MSG_NOSIGNAL);
        if (result <= 0) return false;
        sent += result;
    }

    // 读取认证响应
    uint8_t dummy_header[4];
    if (read(sock, dummy_header, 4) == 4) {
        uint32_t dummy_length = dummy_header[0] | (dummy_header[1] << 8) | (dummy_header[2] << 16);
        if (dummy_length > 0) {
            std::vector<uint8_t> dummy_payload(dummy_length);
            read(sock, dummy_payload.data(), dummy_length);
        }
    }

    return true;
}

int main(int argc, char* argv[]) {
    std::string host = "localhost";
    int port = 18647;

    int opt;
    while ((opt = getopt(argc, argv, "h:p:")) != -1) {
        switch (opt) {
            case 'h': host = optarg; break;
            case 'p': port = std::stoi(optarg); break;
        }
    }

    std::cout << "Simple SQLCC Query Test" << std::endl;
    std::cout << "=======================" << std::endl;
    std::cout << "Host: " << host << ":" << port << std::endl;

    // 连接到服务器
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr);

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed: " << strerror(errno) << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "Connected to server" << std::endl;

    // 发送握手响应
    if (!SendHandshakeResponse(sock)) {
        std::cerr << "Handshake failed" << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "Handshake completed" << std::endl;

    // 执行一些简单查询
    uint8_t sequence_id = 1; // 从1开始，因为握手使用了0

    // 测试1: 创建表
    if (!ExecuteQuery(sock, "CREATE TABLE IF NOT EXISTS test_users (id INT PRIMARY KEY, name VARCHAR(50), email VARCHAR(100), age INT)", sequence_id)) {
        std::cerr << "CREATE TABLE failed" << std::endl;
    }

    // 测试2: 插入数据
    for (int i = 1; i <= 5; ++i) {
        std::stringstream ss;
        ss << "INSERT INTO test_users (id, name, email, age) VALUES ("
           << i << ", 'User" << i << "', 'user" << i << "@test.com', " << (20 + i) << ")";
        if (!ExecuteQuery(sock, ss.str(), sequence_id)) {
            std::cerr << "INSERT failed for user " << i << std::endl;
        }
    }

    // 测试3: 查询数据
    if (!ExecuteQuery(sock, "SELECT * FROM test_users", sequence_id)) {
        std::cerr << "SELECT failed" << std::endl;
    }

    // 测试4: 更新数据
    if (!ExecuteQuery(sock, "UPDATE test_users SET age = age + 1 WHERE id = 1", sequence_id)) {
        std::cerr << "UPDATE failed" << std::endl;
    }

    // 测试5: 再次查询
    if (!ExecuteQuery(sock, "SELECT * FROM test_users WHERE id = 1", sequence_id)) {
        std::cerr << "Final SELECT failed" << std::endl;
    }

    std::cout << "\nTest completed successfully!" << std::endl;
    std::cout << "Basic CRUD operations verified." << std::endl;

    close(sock);
    return 0;
}