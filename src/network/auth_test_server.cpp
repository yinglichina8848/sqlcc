/**
 * @file auth_test_server.cpp
 * @brief 认证测试服务器
 *
 * 用于测试client-server认证功能的简单服务器程序
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <memory>

#include "network/connection_handler.h"
#include "network/session_manager.h"
#include "core/user_manager.h"
#include "utils/file_descriptor.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace sqlcc;

// 全局变量
static bool running = true;
static int epoll_fd = -1;

// 信号处理
void signal_handler(int sig) {
    std::cout << "Received signal " << sig << ", shutting down..." << std::endl;
    running = false;
}

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

    std::cout << "Starting authentication test server on port " << port << std::endl;

    // 注册信号处理
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // 创建epoll
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        std::cerr << "Failed to create epoll: " << strerror(errno) << std::endl;
        return 1;
    }

    // 创建监听socket
    int listen_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (listen_fd < 0) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        close(epoll_fd);
        return 1;
    }

    // 设置SO_REUSEADDR
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Failed to set SO_REUSEADDR: " << strerror(errno) << std::endl;
        close(listen_fd);
        close(epoll_fd);
        return 1;
    }

    // 绑定地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to bind to port " << port << ": " << strerror(errno) << std::endl;
        close(listen_fd);
        close(epoll_fd);
        return 1;
    }

    // 监听
    if (listen(listen_fd, 10) < 0) {
        std::cerr << "Failed to listen: " << strerror(errno) << std::endl;
        close(listen_fd);
        close(epoll_fd);
        return 1;
    }

    // 连接池
    std::vector<std::unique_ptr<network::ConnectionHandler>> connections;

    // 添加到epoll (使用指针区分监听socket)
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.ptr = nullptr;  // 监听socket使用nullptr标记
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event) < 0) {
        std::cerr << "Failed to add listen_fd to epoll: " << strerror(errno) << std::endl;
        close(listen_fd);
        close(epoll_fd);
        return 1;
    }

    std::cout << "Server listening on port " << port << std::endl;

    // 创建核心组件
    auto session_manager = std::make_shared<network::SessionManager>();
    auto user_manager = std::make_shared<UserManager>("./test_data");
    auto sql_executor = std::make_shared<SqlExecutor>(); // 使用默认构造函数

    // 创建默认用户用于测试
    if (!user_manager->CreateUser("admin", "password", "SUPERUSER")) {
        std::cout << "Warning: Failed to create test user, it may already exist" << std::endl;
    }

    const int MAX_EVENTS = 64;
    struct epoll_event events[MAX_EVENTS];

    while (running) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000); // 1秒超时

        for (int i = 0; i < num_events; ++i) {
            if (events[i].data.ptr == nullptr) {
                // 监听socket事件
                // 新连接
                struct sockaddr_in client_addr;
                socklen_t addr_len = sizeof(client_addr);
                int client_fd = accept4(listen_fd, (struct sockaddr*)&client_addr, &addr_len, SOCK_NONBLOCK);

                if (client_fd >= 0) {
                    std::cout << "New connection from " << inet_ntoa(client_addr.sin_addr)
                             << ":" << ntohs(client_addr.sin_port) << std::endl;

                    // 创建ConnectionHandler
                    auto connection = std::make_unique<network::ConnectionHandler>(
                        sqlcc::FileDescriptor(client_fd),
                        session_manager,
                        sql_executor,
                        user_manager
                    );

                    // 添加到epoll
                    struct epoll_event conn_event;
                    conn_event.events = EPOLLIN | EPOLLET; // 边缘触发
                    conn_event.data.ptr = connection.get();
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &conn_event) < 0) {
                        std::cerr << "Failed to add client_fd to epoll: " << strerror(errno) << std::endl;
                        continue;
                    }

                    connections.push_back(std::move(connection));
                } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    std::cerr << "Accept failed: " << strerror(errno) << std::endl;
                }
            } else {
                // 现有连接的事件
                auto* conn = static_cast<network::ConnectionHandler*>(events[i].data.ptr);
                if (conn) {
                    conn->HandleEvent(events[i].events);
                }
            }
        }

        // 清理已关闭的连接
        connections.erase(
            std::remove_if(connections.begin(), connections.end(),
                [](const std::unique_ptr<network::ConnectionHandler>& conn) {
                    if (conn->IsClosed()) {
                        std::cout << "Removing closed connection" << std::endl;
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn->GetFd(), nullptr);
                        return true;
                    }
                    return false;
                }),
            connections.end()
        );
    }

    // 清理
    close(listen_fd);
    close(epoll_fd);

    std::cout << "Server shutdown complete" << std::endl;
    return 0;
}