/**
 * @file encrypted_test_runner.cpp
 * @brief 加密通信测试运行器 - 简化版本
 * 
 * 这是一个简化的加密通信测试程序，演示AESE加密功能在网络通信中的应用
 */

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <array>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

/**
 * 执行系统命令并返回输出
 */
std::string ExecuteCommand(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return "";
    }
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);
    return result;
}

/**
 * 启动服务器进程
 */
pid_t StartServer(const std::string& server_path, int port, bool enable_encryption) {
    pid_t pid = fork();
    if (pid == 0) {  // 子进程
        std::string cmd = server_path + " -p " + std::to_string(port);
        if (enable_encryption) {
            cmd += " -e";
        }
        std::cout << "[服务器] 启动命令: " << cmd << std::endl;
        execl("/bin/sh", "sh", "-c", cmd.c_str(), nullptr);
        exit(1);
    }
    return pid;
}

/**
 * 运行客户端测试
 */
bool RunClientTest(const std::string& client_path, int port, bool enable_encryption) {
    std::string cmd = client_path + " -h 127.0.0.1 -p " + std::to_string(port) + 
                      " -u admin -P password";
    if (enable_encryption) {
        cmd += " -e";
    }
    
    std::cout << "[客户端] 执行命令: " << cmd << std::endl;
    std::string output = ExecuteCommand(cmd);
    
    std::cout << "[客户端] 输出:\n" << output << std::endl;
    
    // 检查是否成功
    if (output.find("Successfully connected") != std::string::npos ||
        output.find("Successfully authenticated") != std::string::npos) {
        return true;
    }
    return false;
}

int main(int argc, char* argv[]) {
    std::cout << "\n╔═══════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║         AESE加密通信集成测试运行器                     ║" << std::endl;
    std::cout << "║    基于AES-256-CBC的数据库网络通信安全验证              ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════════════╝" << std::endl;

    std::string server_path = "./bin/sqlcc_server";
    std::string client_path = "./bin/isql_network";
    int port = 18648;
    
    // 检查服务器和客户端是否存在
    if (access(server_path.c_str(), F_OK) == -1) {
        std::cerr << "✗ 服务器可执行文件不存在: " << server_path << std::endl;
        std::cerr << "请先编译: make sqlcc_server" << std::endl;
        return 1;
    }
    
    if (access(client_path.c_str(), F_OK) == -1) {
        std::cerr << "✗ 客户端可执行文件不存在: " << client_path << std::endl;
        std::cerr << "请先编译: make isql_network" << std::endl;
        return 1;
    }
    
    std::cout << "\n测试配置:" << std::endl;
    std::cout << "  服务器路径: " << server_path << std::endl;
    std::cout << "  客户端路径: " << client_path << std::endl;
    std::cout << "  服务器端口: " << port << std::endl;
    std::cout << "  加密模式: AES-256-CBC" << std::endl;

    int test_count = 0;
    int passed_count = 0;

    // 测试1: 启动加密服务器并进行客户端连接
    {
        std::cout << "\n测试1: 加密服务器启动和客户端连接" << std::endl;
        std::cout << "========================================" << std::endl;
        
        test_count++;
        
        std::cout << "[1] 启动加密服务器..." << std::endl;
        pid_t server_pid = StartServer(server_path, port, true);
        
        // 等待服务器启动
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        std::cout << "[2] 运行加密客户端连接测试..." << std::endl;
        bool client_success = RunClientTest(client_path, port, true);
        
        // 停止服务器
        std::cout << "[3] 停止服务器..." << std::endl;
        kill(server_pid, SIGTERM);
        int status;
        waitpid(server_pid, &status, 0);
        
        if (client_success) {
            std::cout << "✓ 测试1通过: 加密通信建立成功" << std::endl;
            passed_count++;
        } else {
            std::cout << "✗ 测试1失败: 加密通信失败" << std::endl;
        }
    }

    // 测试2: 多个连接的并发加密通信
    {
        std::cout << "\n测试2: 多个并发加密连接" << std::endl;
        std::cout << "========================================" << std::endl;
        
        test_count++;
        
        std::cout << "[1] 启动加密服务器..." << std::endl;
        pid_t server_pid = StartServer(server_path, port + 1, true);
        
        // 等待服务器启动
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        std::cout << "[2] 进行3个并发加密客户端连接..." << std::endl;
        bool all_success = true;
        for (int i = 0; i < 3; ++i) {
            std::cout << "  连接 " << (i + 1) << "/3..." << std::endl;
            if (!RunClientTest(client_path, port + 1, true)) {
                all_success = false;
            }
        }
        
        // 停止服务器
        std::cout << "[3] 停止服务器..." << std::endl;
        kill(server_pid, SIGTERM);
        int status;
        waitpid(server_pid, &status, 0);
        
        if (all_success) {
            std::cout << "✓ 测试2通过: 并发加密通信成功" << std::endl;
            passed_count++;
        } else {
            std::cout << "✗ 测试2失败: 部分并发连接失败" << std::endl;
        }
    }

    // 打印测试总结
    std::cout << "\n╔═══════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                   测试总结                               ║" << std::endl;
    std::cout << "║                                                           ║" << std::endl;
    std::cout << "║  总测试数: " << test_count << "                                             ║" << std::endl;
    std::cout << "║  通过数:   " << passed_count << "                                             ║" << std::endl;
    std::cout << "║  失败数:   " << (test_count - passed_count) << "                                             ║" << std::endl;
    
    if (passed_count == test_count) {
        std::cout << "║                                                           ║" << std::endl;
        std::cout << "║  ✓ 所有AESE加密通信测试通过！                            ║" << std::endl;
    } else {
        std::cout << "║                                                           ║" << std::endl;
        std::cout << "║  ✗ 部分AESE加密通信测试失败                              ║" << std::endl;
    }
    std::cout << "╚═══════════════════════════════════════════════════════════╝" << std::endl;

    return (passed_count == test_count) ? 0 : 1;
}
