#ifndef CLIENT_TEST_H
#define CLIENT_TEST_H

#include <string>
#include <vector>

namespace sqlcc {
namespace test {

class ClientTest {
private:
  std::string client_path_; // 客户端可执行文件路径
  std::string host_;        // 服务器主机
  int port_;                // 服务器端口

public:
  /**
   * @brief 构造函数
   * @param client_path 客户端可执行文件路径
   * @param host 服务器主机，默认127.0.0.1
   * @param port 服务器端口，默认18647
   */
  ClientTest(const std::string &client_path,
             const std::string &host = "127.0.0.1", int port = 18647);

  /**
   * @brief 测试连接到服务器
   * @return 成功返回true，失败返回false
   */
  bool TestConnection();

  /**
   * @brief 测试认证功能
   * @param username 用户名
   * @param password 密码
   * @return 成功返回true，失败返回false
   */
  bool TestAuthentication(const std::string &username,
                          const std::string &password);

  /**
   * @brief 测试执行查询
   * @param username 用户名
   * @param password 密码
   * @param query SQL查询语句
   * @return 成功返回true，失败返回false
   */
  bool TestQuery(const std::string &username, const std::string &password,
                 const std::string &query);

  /**
   * @brief 运行完整的客户机-服务器测试
   * @param username 用户名
   * @param password 密码
   * @return 成功返回true，失败返回false
   */
  bool RunFullTest(const std::string &username, const std::string &password);

private:
  /**
   * @brief 执行客户端命令并返回结果
   * @param args 命令行参数
   * @param output 输出结果
   * @return 成功返回true，失败返回false
   */
  bool ExecuteClient(const std::vector<std::string> &args, std::string &output);
};

} // namespace test
} // namespace sqlcc

#endif // CLIENT_TEST_H
