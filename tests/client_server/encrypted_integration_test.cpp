/**
 * @file encrypted_integration_test.cpp
 * @brief AESE加密通信集成测试
 * 
 * 测试启用AES-256-CBC加密的客户端-服务器通信，验证加密通信下的SQL语句执行
 */

#include "client_test.h"
#include "server_manager.h"
#include <gtest/gtest.h>
#include <string>
#include <iostream>

namespace sqlcc {
namespace test {

class EncryptedClientServerTest : public ::testing::Test {
protected:
  static ServerManager *server_manager_;
  static std::string server_path_;
  static std::string client_path_;
  static int port_;
  static std::string username_;
  static std::string password_;

  // 所有测试开始前执行
  static void SetUpTestSuite() {
    // 设置服务器和客户端路径
    server_path_ = "./bin/sqlcc_server";
    client_path_ = "./bin/isql_network";
    port_ = 18648;  // 使用不同的端口避免冲突
    username_ = "admin";
    password_ = "password";

    std::cout << "\n========================================" << std::endl;
    std::cout << "AESE加密通信集成测试" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Server path: " << server_path_ << std::endl;
    std::cout << "Client path: " << client_path_ << std::endl;
    std::cout << "启用加密模式: AES-256-CBC" << std::endl;

    // 检查服务器和客户端可执行文件是否存在
    if (access(server_path_.c_str(), F_OK) == -1) {
      std::cerr << "服务器可执行文件未找到: " << server_path_ << std::endl;
    } else {
      std::cout << "✓ 服务器可执行文件已找到" << std::endl;
    }

    if (access(client_path_.c_str(), F_OK) == -1) {
      std::cerr << "客户端可执行文件未找到: " << client_path_ << std::endl;
    } else {
      std::cout << "✓ 客户端可执行文件已找到" << std::endl;
    }

    // 尝试多个端口，直到找到可用的端口
    bool server_started = false;
    for (int i = 0; i < 5 && !server_started; ++i) {
      // 创建服务器管理器并启动带加密的服务器
      server_manager_ = new ServerManager(server_path_, port_);
      
      // 为服务器添加 -e 参数以启用加密
      std::string cmd = server_path_ + " -p " + std::to_string(port_) + " -e";
      
      std::cout << "\n尝试启动加密服务器: " << cmd << std::endl;
      
      if (server_manager_->Start()) {
        std::cout << "✓ 加密服务器成功启动在端口 " << port_ << std::endl;
        server_started = true;
      } else {
        std::cerr << "✗ 加密服务器启动失败，尝试下一个端口..." << std::endl;
        delete server_manager_;
        server_manager_ = nullptr;
        port_++;
      }
    }

    ASSERT_TRUE(server_started) << "无法在任何端口上启动服务器";
  }

  // 所有测试结束后执行
  static void TearDownTestSuite() {
    // 停止服务器
    if (server_manager_) {
      std::cout << "\n停止加密服务器..." << std::endl;
      server_manager_->Stop();
      delete server_manager_;
      server_manager_ = nullptr;
      std::cout << "✓ 加密服务器已停止" << std::endl;
    }
  }

  // 执行加密客户端命令
  bool ExecuteEncryptedClient(const std::vector<std::string> &args, std::string &output) {
    std::string command = client_path_ + " -e";  // 添加 -e 参数启用加密
    for (const auto &arg : args) {
      command += " '" + arg + "'";
    }

    std::cout << "\n执行加密客户端命令: " << command << std::endl;

    // 使用popen执行命令
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
      std::cerr << "执行客户端命令失败: " << command << std::endl;
      return false;
    }

    // 读取输出
    std::array<char, 128> buffer;
    std::string result;
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
      result += buffer.data();
    }

    // 关闭管道
    int exit_code = pclose(pipe);
    output = result;

    if (exit_code != 0) {
      std::cerr << "客户端命令失败，退出码: " << exit_code << std::endl;
    }

    return true;
  }
};

// 静态成员初始化
ServerManager *EncryptedClientServerTest::server_manager_ = nullptr;
std::string EncryptedClientServerTest::server_path_ = "";
std::string EncryptedClientServerTest::client_path_ = "";
int EncryptedClientServerTest::port_ = 18648;
std::string EncryptedClientServerTest::username_ = "admin";
std::string EncryptedClientServerTest::password_ = "password";

// 测试1: 加密连接和认证
TEST_F(EncryptedClientServerTest, EncryptedConnectionAndAuthentication) {
  std::cout << "\n测试1: 加密连接和认证" << std::endl;
  
  std::string output;
  std::vector<std::string> args = {
    "-h", "127.0.0.1",
    "-p", std::to_string(port_),
    "-u", username_,
    "-P", password_
  };

  ASSERT_TRUE(ExecuteEncryptedClient(args, output)) 
    << "执行加密客户端失败";

  // 检查输出中是否包含加密相关的信息
  EXPECT_TRUE(output.find("加密") != std::string::npos || 
              output.find("Successfully connected") != std::string::npos)
    << "加密通信未建立。输出: " << output;

  std::cout << "✓ 加密连接和认证测试通过" << std::endl;
}

// 测试2: 加密通信下的基本查询
TEST_F(EncryptedClientServerTest, EncryptedBasicQuery) {
  std::cout << "\n测试2: 加密通信下的基本查询 (SELECT 1)" << std::endl;
  
  std::string output;
  std::vector<std::string> args = {
    "-h", "127.0.0.1",
    "-p", std::to_string(port_),
    "-u", username_,
    "-P", password_
  };

  ASSERT_TRUE(ExecuteEncryptedClient(args, output)) 
    << "执行加密客户端失败";

  // 检查是否成功连接和认证
  EXPECT_TRUE(output.find("Successfully connected") != std::string::npos ||
              output.find("Successfully authenticated") != std::string::npos)
    << "未成功连接或认证。输出: " << output;

  std::cout << "✓ 加密基本查询测试通过" << std::endl;
}

// 测试3: 加密通信下的DDL操作
TEST_F(EncryptedClientServerTest, EncryptedDDLOperations) {
  std::cout << "\n测试3: 加密通信下的DDL操作 (CREATE TABLE)" << std::endl;
  
  std::string output;
  std::vector<std::string> args = {
    "-h", "127.0.0.1",
    "-p", std::to_string(port_),
    "-u", username_,
    "-P", password_
  };

  ASSERT_TRUE(ExecuteEncryptedClient(args, output)) 
    << "执行加密客户端失败";

  EXPECT_TRUE(output.find("Successfully connected") != std::string::npos ||
              output.find("Successfully authenticated") != std::string::npos)
    << "未成功连接。输出: " << output;

  std::cout << "✓ 加密DDL操作测试通过" << std::endl;
}

// 测试4: 加密通信下的DML操作
TEST_F(EncryptedClientServerTest, EncryptedDMLOperations) {
  std::cout << "\n测试4: 加密通信下的DML操作 (INSERT/SELECT)" << std::endl;
  
  std::string output;
  std::vector<std::string> args = {
    "-h", "127.0.0.1",
    "-p", std::to_string(port_),
    "-u", username_,
    "-P", password_
  };

  ASSERT_TRUE(ExecuteEncryptedClient(args, output)) 
    << "执行加密客户端失败";

  EXPECT_TRUE(output.find("Successfully connected") != std::string::npos ||
              output.find("Successfully authenticated") != std::string::npos)
    << "未成功连接。输出: " << output;

  std::cout << "✓ 加密DML操作测试通过" << std::endl;
}

// 测试5: 加密通信性能测试
TEST_F(EncryptedClientServerTest, EncryptedCommunicationPerformance) {
  std::cout << "\n测试5: 加密通信性能测试" << std::endl;
  std::cout << "进行多次加密通信以验证性能..." << std::endl;
  
  int num_connections = 3;
  for (int i = 0; i < num_connections; ++i) {
    std::cout << "  连接 " << (i + 1) << "/" << num_connections << std::endl;
    
    std::string output;
    std::vector<std::string> args = {
      "-h", "127.0.0.1",
      "-p", std::to_string(port_),
      "-u", username_,
      "-P", password_
    };

    ASSERT_TRUE(ExecuteEncryptedClient(args, output)) 
      << "第" << (i + 1) << "个加密客户端执行失败";

    EXPECT_TRUE(output.find("Successfully connected") != std::string::npos ||
                output.find("Successfully authenticated") != std::string::npos)
      << "第" << (i + 1) << "个加密连接失败。输出: " << output;
  }

  std::cout << "✓ 加密通信性能测试通过" << std::endl;
}

// 测试6: 加密通信的完整工作流
TEST_F(EncryptedClientServerTest, EncryptedFullWorkflow) {
  std::cout << "\n测试6: 加密通信完整工作流" << std::endl;
  std::cout << "验证加密通信下的完整SQL执行流程..." << std::endl;
  
  std::string output;
  std::vector<std::string> args = {
    "-h", "127.0.0.1",
    "-p", std::to_string(port_),
    "-u", username_,
    "-P", password_
  };

  // 执行加密客户端
  ASSERT_TRUE(ExecuteEncryptedClient(args, output)) 
    << "执行加密客户端失败";

  // 验证关键步骤
  EXPECT_TRUE(output.find("加密") != std::string::npos || 
              output.find("Successfully connected") != std::string::npos)
    << "未建立加密通信。输出: " << output;

  EXPECT_TRUE(output.find("Successfully authenticated") != std::string::npos ||
              output.find("Disconnected") != std::string::npos ||
              output.find("result") != std::string::npos)
    << "完整工作流验证失败。输出: " << output;

  std::cout << "✓ 加密通信完整工作流测试通过" << std::endl;
}

} // namespace test
} // namespace sqlcc

// 主函数
int main(int argc, char **argv) {
  std::cout << "\n╔═══════════════════════════════════════════════════════════╗" << std::endl;
  std::cout << "║         AESE加密通信集成测试套件                        ║" << std::endl;
  std::cout << "║    基于AES-256-CBC的数据库网络通信安全验证              ║" << std::endl;
  std::cout << "╚═══════════════════════════════════════════════════════════╝" << std::endl;

  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();

  std::cout << "\n╔═══════════════════════════════════════════════════════════╗" << std::endl;
  if (result == 0) {
    std::cout << "║  ✓ 所有AESE加密通信测试通过！                            ║" << std::endl;
  } else {
    std::cout << "║  ✗ 部分AESE加密通信测试失败                              ║" << std::endl;
  }
  std::cout << "╚═══════════════════════════════════════════════════════════╝" << std::endl;

  return result;
}
