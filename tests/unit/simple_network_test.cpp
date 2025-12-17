#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>

// 简化的消息头结构
struct SimpleMessageHeader {
    uint32_t magic;      // 魔数 'SQL'
    uint32_t length;     // 消息体长度
    uint32_t type;       // 消息类型
    uint32_t flags;      // 标志位
};

// 消息类型枚举
enum MessageType {
    CONNECT = 1,
    CONN_ACK = 2,
    AUTH = 3,
    AUTH_ACK = 4,
    QUERY = 5,
    QUERY_RESULT = 6,
    ERROR = 7
};

// 简化的网络服务器类
class SimpleNetworkServer {
private:
    int server_fd_;
    bool running_;
    int port_;

public:
    SimpleNetworkServer(int port) : port_(port), server_fd_(-1), running_(false) {}

    ~SimpleNetworkServer() {
        Stop();
    }

    bool Start() {
        // 创建socket
        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        // 设置socket选项
        int opt = 1;
        setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        // 绑定地址
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port_);

        if (bind(server_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            std::cerr << "Failed to bind socket" << std::endl;
            close(server_fd_);
            return false;
        }

        // 开始监听
        if (listen(server_fd_, 5) < 0) {
            std::cerr << "Failed to listen" << std::endl;
            close(server_fd_);
            return false;
        }

        running_ = true;
        std::cout << "Server started on port " << port_ << std::endl;
        return true;
    }

    void Stop() {
        running_ = false;
        if (server_fd_ >= 0) {
            close(server_fd_);
            server_fd_ = -1;
        }
    }

    void Run() {
        std::cout << "Server is running, waiting for connections..." << std::endl;
        
        while (running_) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd < 0) {
                if (errno == EINTR) continue;
                std::cerr << "Failed to accept connection" << std::endl;
                continue;
            }

            std::cout << "New connection accepted" << std::endl;
            HandleClient(client_fd);
            close(client_fd);
            std::cout << "Client disconnected" << std::endl;
        }
    }

private:
    void HandleClient(int client_fd) {
        char buffer[4096];
        
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);
            
            if (bytes_read <= 0) {
                break; // 连接断开或错误
            }

            if (bytes_read < sizeof(SimpleMessageHeader)) {
                continue; // 数据不完整
            }

            SimpleMessageHeader* header = reinterpret_cast<SimpleMessageHeader*>(buffer);
            
            // 检查魔数
            if (header->magic != 0x514C5300) { // 'SQL\0'
                std::cerr << "Invalid magic number" << std::endl;
                break;
            }

            // 处理不同类型的消息
            switch (header->type) {
                case CONNECT:
                    HandleConnect(client_fd);
                    break;
                case AUTH:
                    HandleAuth(client_fd, buffer + sizeof(SimpleMessageHeader), header->length);
                    break;
                case QUERY:
                    HandleQuery(client_fd, buffer + sizeof(SimpleMessageHeader), header->length);
                    break;
                default:
                    std::cerr << "Unknown message type: " << header->type << std::endl;
                    break;
            }
        }
    }

    void HandleConnect(int client_fd) {
        SimpleMessageHeader header;
        header.magic = 0x514C5300; // 'SQL\0'
        header.length = 0;
        header.type = CONN_ACK;
        header.flags = 0;

        send(client_fd, &header, sizeof(header), 0);
        std::cout << "Sent connection acknowledgment" << std::endl;
    }

    void HandleAuth(int client_fd, const char* data, uint32_t length) {
        // 简单的认证逻辑
        bool authenticated = true; // 实际实现中应该验证用户名密码
        
        SimpleMessageHeader header;
        header.magic = 0x514C5300; // 'SQL\0'
        header.length = 0;
        header.type = AUTH_ACK;
        header.flags = authenticated ? 0 : 1;

        send(client_fd, &header, sizeof(header), 0);
        std::cout << "Sent authentication response: " << (authenticated ? "success" : "failed") << std::endl;
    }

    void HandleQuery(int client_fd, const char* data, uint32_t length) {
        std::string query(data, length);
        std::cout << "Received query: " << query << std::endl;
        
        // 简单的SQL处理逻辑
        std::string result;
        if (query.find("SELECT") != std::string::npos) {
            result = "Query executed successfully. Result: [Sample data]";
        } else if (query.find("INSERT") != std::string::npos) {
            result = "INSERT operation completed. 1 row affected.";
        } else if (query.find("UPDATE") != std::string::npos) {
            result = "UPDATE operation completed. 1 row affected.";
        } else if (query.find("DELETE") != std::string::npos) {
            result = "DELETE operation completed. 1 row affected.";
        } else {
            result = "Query processed: " + query;
        }

        // 发送查询结果
        SimpleMessageHeader header;
        header.magic = 0x514C5300; // 'SQL\0'
        header.length = result.length();
        header.type = QUERY_RESULT;
        header.flags = 0;

        std::vector<char> response(sizeof(header) + result.length());
        memcpy(response.data(), &header, sizeof(header));
        memcpy(response.data() + sizeof(header), result.c_str(), result.length());

        send(client_fd, response.data(), response.size(), 0);
        std::cout << "Sent query result: " << result << std::endl;
    }
};

// 简化的网络客户端类
class SimpleNetworkClient {
private:
    int client_fd_;
    std::string host_;
    int port_;

public:
    SimpleNetworkClient(const std::string& host, int port) 
        : host_(host), port_(port), client_fd_(-1) {}

    ~SimpleNetworkClient() {
        Disconnect();
    }

    bool Connect() {
        client_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (client_fd_ < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_);

        if (inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << "Invalid address" << std::endl;
            close(client_fd_);
            return false;
        }

        if (connect(client_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Connection failed" << std::endl;
            close(client_fd_);
            return false;
        }

        std::cout << "Connected to server at " << host_ << ":" << port_ << std::endl;
        return true;
    }

    void Disconnect() {
        if (client_fd_ >= 0) {
            close(client_fd_);
            client_fd_ = -1;
        }
    }

    bool SendQuery(const std::string& query) {
        // 发送连接消息
        SimpleMessageHeader connect_header;
        connect_header.magic = 0x514C5300; // 'SQL\0'
        connect_header.length = 0;
        connect_header.type = CONNECT;
        connect_header.flags = 0;

        if (send(client_fd_, &connect_header, sizeof(connect_header), 0) < 0) {
            std::cerr << "Failed to send connect message" << std::endl;
            return false;
        }

        // 等待连接确认
        char buffer[4096];
        if (recv(client_fd_, buffer, sizeof(buffer), 0) <= 0) {
            std::cerr << "Failed to receive connection acknowledgment" << std::endl;
            return false;
        }

        // 发送认证消息
        SimpleMessageHeader auth_header;
        auth_header.magic = 0x514C5300; // 'SQL\0'
        auth_header.length = 0;
        auth_header.type = AUTH;
        auth_header.flags = 0;

        if (send(client_fd_, &auth_header, sizeof(auth_header), 0) < 0) {
            std::cerr << "Failed to send auth message" << std::endl;
            return false;
        }

        // 等待认证确认
        if (recv(client_fd_, buffer, sizeof(buffer), 0) <= 0) {
            std::cerr << "Failed to receive auth acknowledgment" << std::endl;
            return false;
        }

        // 发送查询
        SimpleMessageHeader query_header;
        query_header.magic = 0x514C5300; // 'SQL\0'
        query_header.length = query.length();
        query_header.type = QUERY;
        query_header.flags = 0;

        std::vector<char> message(sizeof(query_header) + query.length());
        memcpy(message.data(), &query_header, sizeof(query_header));
        memcpy(message.data() + sizeof(query_header), query.c_str(), query.length());

        if (send(client_fd_, message.data(), message.size(), 0) < 0) {
            std::cerr << "Failed to send query" << std::endl;
            return false;
        }

        // 接收查询结果
        ssize_t bytes_received = recv(client_fd_, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cerr << "Failed to receive query result" << std::endl;
            return false;
        }

        if (bytes_received < sizeof(SimpleMessageHeader)) {
            std::cerr << "Invalid response header" << std::endl;
            return false;
        }

        SimpleMessageHeader* result_header = reinterpret_cast<SimpleMessageHeader*>(buffer);
        if (result_header->type == QUERY_RESULT) {
            std::string result(buffer + sizeof(SimpleMessageHeader), result_header->length);
            std::cout << "Query result: " << result << std::endl;
            return true;
        } else {
            std::cerr << "Unexpected response type: " << result_header->type << std::endl;
            return false;
        }
    }
};

#include <gtest/gtest.h>

TEST(SimpleNetworkTest, BasicTest) {
    // 这是一个简单的占位测试
    EXPECT_TRUE(true);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
