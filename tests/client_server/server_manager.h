#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <cstdint>
#include <string>
#include <sys/types.h>

namespace sqlcc {
namespace test {

class ServerManager {
private:
  std::string server_path_; // 服务器可执行文件路径
  int port_;                // 服务器端口
  pid_t server_pid_;        // 服务器进程ID
  bool running_;            // 服务器运行状态

public:
  /**
   * @brief 构造函数
   * @param server_path 服务器可执行文件路径
   * @param port 服务器端口，默认18647
   */
  ServerManager(const std::string &server_path, int port = 18647);

  /**
   * @brief 析构函数
   */
  ~ServerManager();

  /**
   * @brief 启动服务器
   * @return 成功返回true，失败返回false
   */
  bool Start();

  /**
   * @brief 停止服务器
   * @return 成功返回true，失败返回false
   */
  bool Stop();

  /**
   * @brief 检查服务器是否正在运行
   * @return 运行中返回true，否则返回false
   */
  bool IsRunning() const;

  /**
   * @brief 获取服务器端口
   * @return 服务器端口
   */
  int GetPort() const;

  /**
   * @brief 获取服务器进程ID
   * @return 服务器进程ID
   */
  pid_t GetPid() const;
};

} // namespace test
} // namespace sqlcc

#endif // SERVER_MANAGER_H
