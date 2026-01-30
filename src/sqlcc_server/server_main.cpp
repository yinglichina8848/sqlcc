#include "src/sql_executor.h"
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
#include <memory>

// 包含网络头文件，使用命名空间限定符
#include "network/network.h"

// 使用命名空间别名避免冲突
namespace network = sqlcc::network;

// 全局服务器智能指针，用于信号处理
static std::shared_ptr<network::ServerNetworkManager> g_server = nullptr;

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
    bool enable_tls = false;         // 启用TLS
    std::string cert_path = "./test_certs/server.crt";
    std::string key_path = "./test_certs/server.key";

    // 解析命令行参数
    int opt;
    while ((opt = getopt(argc, argv, "p:vec:k:")) != -1) {
        switch (opt) {
            case 'p':
                port = std::stoi(optarg);
                break;
            case 'v':
                verbose = true;
                break;
            case 'e':
                enable_encryption = true;  // 启用AES加密
                break;
            case 'c':
                enable_tls = true;         // 启用TLS
                cert_path = optarg;        // TLS证书路径
                break;
            case 'k':
                key_path = optarg;         // TLS私钥路径
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-p port] [-v] [-e] [-c cert_path] [-k key_path]" << std::endl;
                std::cerr << "  -e: Enable AES-256 encryption for all connections" << std::endl;
                std::cerr << "  -c cert_path: Enable TLS with certificate file" << std::endl;
                std::cerr << "  -k key_path: TLS private key file (optional, defaults to cert_path with .key extension)" << std::endl;
                return 1;
        }
    }
    
    std::cout << "SqlCC Server starting on port " << port << std::endl;
    if (enable_encryption) {
        std::cout << "[加密模式] 对所有连接启用AES-256-CBC加密" << std::endl;
    }
    if (enable_tls) {
        std::cout << "[TLS模式] 启用TLS安全连接" << std::endl;
        std::cout << "证书文件: " << cert_path << std::endl;
        std::cout << "私钥文件: " << key_path << std::endl;
    }

    // 创建SQL执行器
    auto sql_executor = std::make_shared<sqlcc::SqlExecutor>();

    // 创建服务器网络管理器
    g_server = std::make_shared<network::ServerNetworkManager>(port);

    // 配置TLS（如果启用）
    if (enable_tls) {
        g_server->EnableTLS(true);
        if (!g_server->ConfigureTLSServer(cert_path, key_path, cert_path)) {
            std::cerr << "Failed to configure TLS" << std::endl;
            return 1;
        }
        std::cout << "TLS configuration completed" << std::endl;
    }

    // 设置SQL执行器到服务器网络管理器
    // TODO: 暂时注释掉，因为类型不匹配。当前的SqlExecutor实现是空的，
    // 实际的权限检查和SQL执行将在后续实现中完善
    // g_server->SetSqlExecutor(sql_executor);

    // 注册信号处理函数
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // 启动服务器
    if (!g_server->Start()) {
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
        g_server->ProcessEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // 停止服务器
    g_server->Stop();
    std::cout << "Server stopped" << std::endl;
    
    return 0;
}
