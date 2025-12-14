#include "network/network.h"
#include "network/encryption.h"
#include "sql_executor.h"
#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>
#include <csignal>
#include <unistd.h>
#include <fstream>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace sqlcc::network;

// 全局服务器指针，用于信号处理
static ServerNetworkManager* g_server = nullptr;

// 信号处理函数
void signalHandler(int signal) {
    std::cout << "Received signal " << signal << ", shutting down server..." << std::endl;
    if (g_server) {
        g_server->Stop();
    }
}

int main(int argc, char* argv[]) {
    int port = 18647; // 默认端口
    bool verbose = false;
    bool enable_encryption = false;  // 对所有连接启用加密
    
    // 解析命令行参数
    int opt;
    while ((opt = getopt(argc, argv, "p:ve")) != -1) {
        switch (opt) {
            case 'p':
                port = std::stoi(optarg);
                break;
            case 'v':
                verbose = true;
                break;
            case 'e':
                enable_encryption = true;  // 启用加密
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-p port] [-v] [-e]" << std::endl;
                std::cerr << "  -e: Enable AES-256 encryption for all connections" << std::endl;
                return 1;
        }
    }
    
    std::cout << "SqlCC Server starting on port " << port << std::endl;
    if (enable_encryption) {
        std::cout << "[加密模式] 对所有连接启用AES-256-CBC加密" << std::endl;
    }
    
    // 创建SQL执行器
    auto sql_executor = std::make_shared<sqlcc::SqlExecutor>();
    
    // 创建服务器网络管理器
    ServerNetworkManager server(port);
    g_server = &server;
    
    // 设置SQL执行器到服务器网络管理器
    server.SetSqlExecutor(sql_executor);
    
    // 注册信号处理函数
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // 启动服务器
    if (!server.Start()) {
        std::cerr << "Failed to start server on port " << port << std::endl;
        std::cerr << "Error: " << strerror(errno) << " (errno: " << errno << ")" << std::endl;
        return 1;
    }
    
    std::cout << "Server successfully started on port " << port << std::endl;
    
    // 主循环
    std::cout << "[SERVER] Entering main event loop..." << std::endl;
    int loop_count = 0;
    while (true) {
        loop_count++;
        if (loop_count % 100 == 0) {  // 每100次循环输出一次
            std::cout << "[SERVER] Event loop iteration " << loop_count << std::endl;
        }
        server.ProcessEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // 停止服务器
    server.Stop();
    std::cout << "Server stopped" << std::endl;
    
    return 0;
}
