#include "client_test.h"
#include "server_manager.h"
#include <gtest/gtest.h>
#include <string>

namespace sqlcc {
namespace test {

class ClientServerIntegrationTest : public ::testing::Test {
protected:
  static ServerManager *server_manager_;
  static ClientTest *client_test_;
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
    port_ = 18647;
    username_ = "admin";
    password_ = "password";

    std::cout << "Server path: " << server_path_ << std::endl;
    std::cout << "Client path: " << client_path_ << std::endl;

    // 检查服务器和客户端可执行文件是否存在
    if (access(server_path_.c_str(), F_OK) == -1) {
      std::cerr << "Server executable not found: " << server_path_ << std::endl;
    } else {
      std::cout << "Server executable found: " << server_path_ << std::endl;
    }

    if (access(client_path_.c_str(), F_OK) == -1) {
      std::cerr << "Client executable not found: " << client_path_ << std::endl;
    } else {
      std::cout << "Client executable found: " << client_path_ << std::endl;
    }

    // 尝试多个端口，直到找到可用的端口
    bool server_started = false;
    for (int i = 0; i < 5 && !server_started; ++i) {
      // 创建服务器管理器并启动服务器
      server_manager_ = new ServerManager(server_path_, port_);
      if (server_manager_->Start()) {
        std::cout << "Server started successfully on port " << port_
                  << std::endl;
        server_started = true;
      } else {
        std::cerr << "Failed to start server on port " << port_
                  << ", trying next port..." << std::endl;
        delete server_manager_;
        server_manager_ = nullptr;
        port_++;
      }
    }

    ASSERT_TRUE(server_started) << "Failed to start server on any port";

    // 创建客户端测试对象
    client_test_ = new ClientTest(client_path_, "127.0.0.1", port_);
  }

  // 所有测试结束后执行
  static void TearDownTestSuite() {
    // 停止服务器
    if (server_manager_) {
      std::cout << "Stopping server..." << std::endl;
      server_manager_->Stop();
      delete server_manager_;
      server_manager_ = nullptr;
      std::cout << "Server stopped" << std::endl;
    }

    // 清理客户端测试对象
    if (client_test_) {
      delete client_test_;
      client_test_ = nullptr;
    }
  }
};

// 静态成员初始化
ServerManager *ClientServerIntegrationTest::server_manager_ = nullptr;
ClientTest *ClientServerIntegrationTest::client_test_ = nullptr;
std::string ClientServerIntegrationTest::server_path_ = "";
std::string ClientServerIntegrationTest::client_path_ = "";
int ClientServerIntegrationTest::port_ = 18647;
std::string ClientServerIntegrationTest::username_ = "admin";
std::string ClientServerIntegrationTest::password_ = "password";

// 测试连接
TEST_F(ClientServerIntegrationTest, ConnectionTest) {
  ASSERT_TRUE(client_test_->TestConnection()) << "Connection test failed";
}

// 测试认证
TEST_F(ClientServerIntegrationTest, AuthenticationTest) {
  ASSERT_TRUE(client_test_->TestAuthentication(username_, password_))
      << "Authentication test failed";
  ASSERT_FALSE(client_test_->TestAuthentication(username_, "wrong_password"))
      << "Authentication should fail with wrong password";
}

// 测试基本查询
TEST_F(ClientServerIntegrationTest, BasicQueryTest) {
  ASSERT_TRUE(client_test_->TestQuery(username_, password_, "SELECT 1"))
      << "Basic query test failed";
}

// 测试表操作
TEST_F(ClientServerIntegrationTest, TableOperationsTest) {
  // 创建表
  ASSERT_TRUE(client_test_->TestQuery(
      username_, password_,
      "CREATE TABLE test_table (id INT, name VARCHAR(50))"))
      << "Create table test failed";

  // 插入数据
  ASSERT_TRUE(client_test_->TestQuery(
      username_, password_, "INSERT INTO test_table VALUES (1, 'test')"))
      << "Insert test failed";

  // 查询数据
  ASSERT_TRUE(
      client_test_->TestQuery(username_, password_, "SELECT * FROM test_table"))
      << "Select test failed";

  // 删除表
  ASSERT_TRUE(
      client_test_->TestQuery(username_, password_, "DROP TABLE test_table"))
      << "Drop table test failed";
}

// 运行完整测试
TEST_F(ClientServerIntegrationTest, FullTest) {
  ASSERT_TRUE(client_test_->RunFullTest(username_, password_))
      << "Full test failed";
}

} // namespace test
} // namespace sqlcc

// 主函数
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
