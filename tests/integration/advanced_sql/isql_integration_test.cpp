#include <array>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <signal.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace sqlcc {
namespace test {

class IsqlIntegrationTest : public ::testing::Test {
protected:
  static std::string server_path_;
  static std::string isql_path_;
  static std::string sql_script_path_;
  static int port_;
  static pid_t server_pid_;

  // 所有测试开始前执行
  static void SetUpTestSuite() {
    // 设置服务器、isql和SQL脚本路径
    server_path_ = "./bin/sqlcc_server";
    isql_path_ = "./bin/isql";
    sql_script_path_ = "../../scripts/sql/integration_test.sql";
    port_ = 18647;
    server_pid_ = 0;

    std::cout << "Server path: " << server_path_ << std::endl;
    std::cout << "Isql path: " << isql_path_ << std::endl;
    std::cout << "SQL script path: " << sql_script_path_ << std::endl;

    // 检查服务器和isql可执行文件是否存在
    if (access(server_path_.c_str(), F_OK) == -1) {
      std::cerr << "Server executable not found: " << server_path_ << std::endl;
      GTEST_SKIP();
    }

    if (access(isql_path_.c_str(), F_OK) == -1) {
      std::cerr << "Isql executable not found: " << isql_path_ << std::endl;
      GTEST_SKIP();
    }

    // 检查SQL脚本文件是否存在
    if (access(sql_script_path_.c_str(), F_OK) == -1) {
      std::cerr << "SQL script not found: " << sql_script_path_ << std::endl;
      GTEST_SKIP();
    }

    // 启动服务器
    server_pid_ = fork();
    if (server_pid_ < 0) {
      std::cerr << "Failed to fork server process" << std::endl;
      GTEST_SKIP();
    } else if (server_pid_ == 0) {
      // 子进程：启动服务器
      char port_str[10];
      snprintf(port_str, sizeof(port_str), "%d", port_);
      execl(server_path_.c_str(), server_path_.c_str(), "-p", port_str,
            nullptr);
      std::cerr << "Failed to start server" << std::endl;
      exit(1);
    } else {
      // 父进程：等待服务器启动
      std::cout << "Server started with PID: " << server_pid_ << std::endl;
      // 等待服务器启动完成
      sleep(2);
    }
  }

  // 所有测试结束后执行
  static void TearDownTestSuite() {
    // 停止服务器
    if (server_pid_ > 0) {
      std::cout << "Stopping server with PID: " << server_pid_ << std::endl;

      // 发送SIGTERM信号停止服务器
      if (kill(server_pid_, SIGTERM) != 0) {
        std::cerr << "Failed to send SIGTERM to server: " << strerror(errno)
                  << std::endl;
      } else {
        // 等待服务器退出（最多等待3秒）
        bool server_exited = false;
        for (int i = 0; i < 30 && !server_exited; ++i) {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));

          int status;
          pid_t result = waitpid(server_pid_, &status, WNOHANG);
          if (result > 0) {
            // 服务器已经退出
            server_exited = true;
            std::cout << "Server stopped successfully, PID: " << server_pid_
                      << std::endl;
          } else if (result < 0) {
            // 发生错误
            std::cerr << "Error waiting for server to stop: " << strerror(errno)
                      << std::endl;
            server_exited = true;
          }
        }

        // 如果服务器仍未退出，发送SIGKILL信号强制终止
        if (!server_exited) {
          std::cerr << "Server stop timed out, sending SIGKILL" << std::endl;
          if (kill(server_pid_, SIGKILL) != 0) {
            std::cerr << "Failed to send SIGKILL to server: " << strerror(errno)
                      << std::endl;
          } else {
            // 等待服务器退出
            int status;
            waitpid(server_pid_, &status, 0);
            std::cout << "Server terminated with SIGKILL, PID: " << server_pid_
                      << std::endl;
          }
        }
      }

      server_pid_ = 0;
    }
  }

  // 执行命令并返回输出
  static std::string ExecuteCommand(const std::string &command) {
    std::array<char, 128> buffer;
    std::string result;

    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
      std::cerr << "Failed to execute command: " << command << std::endl;
      return "";
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
      result += buffer.data();
    }

    pclose(pipe);
    return result;
  }
};

// 静态成员初始化
std::string IsqlIntegrationTest::server_path_ = "";
std::string IsqlIntegrationTest::isql_path_ = "";
std::string IsqlIntegrationTest::sql_script_path_ = "";
int IsqlIntegrationTest::port_ = 18647;
pid_t IsqlIntegrationTest::server_pid_ = 0;

// 测试使用isql执行SQL脚本
TEST_F(IsqlIntegrationTest, ExecuteSqlScriptTest) {
  // 构建isql命令
  std::string command = isql_path_ + " -f " + sql_script_path_;
  std::cout << "Executing command: " << command << std::endl;

  // 执行命令
  std::string output = ExecuteCommand(command);
  std::cout << "Command output: " << output << std::endl;

  // 验证执行结果
  // 检查输出中是否包含成功执行的信息
  ASSERT_TRUE(output.find("Query OK") != std::string::npos ||
              output.find("Success") != std::string::npos ||
              output.find("OK") != std::string::npos)
      << "SQL script execution failed. Output: " << output;
}

// 测试基本连接
TEST_F(IsqlIntegrationTest, BasicConnectionTest) {
  // 创建一个简单的SQL脚本，只包含SELECT 1
  std::string simple_script = "SELECT 1;";
  std::string script_file = "/tmp/simple_test.sql";

  // 写入脚本文件
  std::ofstream outfile(script_file);
  outfile << simple_script;
  outfile.close();

  // 构建isql命令
  std::string command = isql_path_ + " -f " + script_file;
  std::cout << "Executing command: " << command << std::endl;

  // 执行命令
  std::string output = ExecuteCommand(command);
  std::cout << "Command output: " << output << std::endl;

  // 验证执行结果
  ASSERT_TRUE(output.find("1") != std::string::npos)
      << "Basic connection test failed. Output: " << output;

  // 删除临时脚本文件
  remove(script_file.c_str());
}

// 测试表操作
TEST_F(IsqlIntegrationTest, TableOperationsTest) {
  // 创建一个包含表操作的SQL脚本
  std::string table_script =
      "CREATE TABLE test_table (id INT, name VARCHAR(50));\n"
      "INSERT INTO test_table VALUES (1, 'test');\n"
      "SELECT * FROM test_table;\n"
      "DROP TABLE test_table;\n";
  std::string script_file = "/tmp/table_test.sql";

  // 写入脚本文件
  std::ofstream outfile(script_file);
  outfile << table_script;
  outfile.close();

  // 构建isql命令
  std::string command = isql_path_ + " -f " + script_file;
  std::cout << "Executing command: " << command << std::endl;

  // 执行命令
  std::string output = ExecuteCommand(command);
  std::cout << "Command output: " << output << std::endl;

  // 验证执行结果
  ASSERT_TRUE(output.find("test") != std::string::npos)
      << "Table operations test failed. Output: " << output;

  // 删除临时脚本文件
  remove(script_file.c_str());
}

} // namespace test
} // namespace sqlcc

// 主函数
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}