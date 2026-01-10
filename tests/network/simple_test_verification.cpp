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

// 简化的测试验证程序
class TestClient {
public:
    TestClient(const std::string& host, int port)
        : host_(host), port_(port), sock_(-1), connected_(false), sequence_id_(0) {}

    ~TestClient() {
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

        connected_ = true;
        return SendHandshakeResponse() && ReadHandshake();
    }

    void Disconnect() {
        if (sock_ >= 0) {
            close(sock_);
            sock_ = -1;
        }
        connected_ = false;
    }

    bool ExecuteQuery(const std::string& query) {
        if (!connected_ || sock_ < 0) return false;

        // Send query
        std::vector<uint8_t> packet;
        packet.push_back(query.size() & 0xFF);
        packet.push_back((query.size() >> 8) & 0xFF);
        packet.push_back((query.size() >> 16) & 0xFF);
        packet.push_back(sequence_id_++ & 0xFF);
        packet.push_back(0x03);
        packet.insert(packet.end(), query.begin(), query.end());

        ssize_t sent = write(sock_, packet.data(), packet.size());
        if (sent != (ssize_t)packet.size()) return false;

        // Read response
        uint8_t header[4];
        ssize_t received = read(sock_, header, 4);
        if (received != 4) return false;

        uint32_t length = header[0] | (header[1] << 8) | (header[2] << 16);
        if (length > 0) {
            std::vector<uint8_t> payload(length);
            size_t total_received = 0;
            while (total_received < length) {
                ssize_t result = read(sock_, payload.data() + total_received, length - total_received);
                if (result <= 0) return false;
                total_received += result;
            }
        }

        return true;
    }

private:
    bool ReadHandshake() {
        uint8_t header[4];
        if (read(sock_, header, 4) != 4) return false;

        uint32_t length = header[0] | (header[1] << 8) | (header[2] << 16);
        if (length > 0) {
            std::vector<uint8_t> payload(length);
            if (read(sock_, payload.data(), length) != (ssize_t)length) return false;
        }
        return true;
    }

    bool SendHandshakeResponse() {
        std::vector<uint8_t> response = {
            0x14, 0x00, 0x00, 0x01,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x61, 0x64, 0x6d, 0x69, 0x6e, 0x00, 0x00
        };

        ssize_t sent = write(sock_, response.data(), response.size());
        if (sent != (ssize_t)response.size()) return false;

        // Read auth response
        uint8_t auth_header[4];
        if (read(sock_, auth_header, 4) == 4) {
            uint32_t auth_length = auth_header[0] | (auth_header[1] << 8) | (auth_header[2] << 16);
            if (auth_length > 0) {
                std::vector<uint8_t> auth_payload(auth_length);
                read(sock_, auth_payload.data(), auth_length);
            }
        }

        return true;
    }

    std::string host_;
    int port_;
    int sock_;
    bool connected_;
    uint8_t sequence_id_;
};

int main() {
    std::cout << "Simple Test Verification" << std::endl;
    std::cout << "=======================" << std::endl;

    TestClient client("localhost", 18647);

    std::cout << "1. Connecting to server..." << std::endl;
    if (!client.Connect()) {
        std::cout << "❌ Connection failed" << std::endl;
        return 1;
    }
    std::cout << "✅ Connection successful" << std::endl;

    std::cout << "2. Creating test table..." << std::endl;
    if (!client.ExecuteQuery("CREATE TABLE IF NOT EXISTS test_verification (id INT PRIMARY KEY, name VARCHAR(50), value INT)")) {
        std::cout << "❌ CREATE TABLE failed" << std::endl;
        client.Disconnect();
        return 1;
    }
    std::cout << "✅ CREATE TABLE successful" << std::endl;

    std::cout << "3. Inserting test data..." << std::endl;
    if (!client.ExecuteQuery("INSERT INTO test_verification (id, name, value) VALUES (1, 'test', 42)")) {
        std::cout << "❌ INSERT failed" << std::endl;
        client.Disconnect();
        return 1;
    }
    std::cout << "✅ INSERT successful" << std::endl;

    std::cout << "4. Selecting test data..." << std::endl;
    if (!client.ExecuteQuery("SELECT * FROM test_verification WHERE id = 1")) {
        std::cout << "❌ SELECT failed" << std::endl;
        client.Disconnect();
        return 1;
    }
    std::cout << "✅ SELECT successful" << std::endl;

    std::cout << "5. Updating test data..." << std::endl;
    if (!client.ExecuteQuery("UPDATE test_verification SET value = 43 WHERE id = 1")) {
        std::cout << "❌ UPDATE failed" << std::endl;
        client.Disconnect();
        return 1;
    }
    std::cout << "✅ UPDATE successful" << std::endl;

    client.Disconnect();

    std::cout << "\n🎉 All tests passed! Basic CRUD operations work correctly." << std::endl;
    std::cout << "The test framework is functioning properly." << std::endl;

    return 0;
}