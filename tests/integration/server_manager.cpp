#include "server_manager.h"
#include <chrono>
#include <cstring>
#include <iostream>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

namespace sqlcc {
namespace test {

ServerManager::ServerManager(const std::string &server_path, int port)
    : server_path_(server_path), port_(port), server_pid_(0), running_(false) {}

ServerManager::~ServerManager() {
  // 析构时停止服务器
  if (running_) {
    Stop();
  }
}

bool ServerManager::Start() {
  if (running_) {
    std::cerr << "Server is already running" << std::endl;
    return false;
  }

  // 创建子进程
  pid_t pid = fork();
  if (pid < 0) {
    std::cerr << "Failed to fork server process: " << strerror(errno)
              << std::endl;
    return false;
  }

  if (pid == 0) {
    // 子进程：启动服务器
    char port_str[10];
    snprintf(port_str, sizeof(port_str), "%d", port_);

    // 执行服务器程序
    execl(server_path_.c_str(), server_path_.c_str(), "-p", port_str, nullptr);

    // 如果execl返回，说明执行失败
    std::cerr << "Failed to start server: " << strerror(errno) << std::endl;
    exit(1);
  } else {
    // 父进程：记录子进程ID，等待服务器启动
    server_pid_ = pid;
    running_ = true;

    // 等待服务器启动（最多等待5秒）
    for (int i = 0; i < 50; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      // 检查服务器是否正在运行
      int status;
      pid_t result = waitpid(pid, &status, WNOHANG);
      if (result == 0) {
        // 服务器正在运行
        std::cout << "Server started successfully on port " << port_
                  << ", PID: " << pid << std::endl;
        return true;
      } else if (result > 0) {
        // 服务器已经退出
        std::cerr << "Server exited immediately with status " << status
                  << std::endl;
        running_ = false;
        server_pid_ = 0;
        return false;
      }
    }

    std::cerr << "Server startup timed out" << std::endl;
    Stop();
    return false;
  }
}

bool ServerManager::Stop() {
  if (!running_) {
    std::cerr << "Server is not running" << std::endl;
    return false;
  }

  // 发送SIGTERM信号停止服务器
  if (kill(server_pid_, SIGTERM) != 0) {
    std::cerr << "Failed to send SIGTERM to server: " << strerror(errno)
              << std::endl;
    return false;
  }

  // 等待服务器退出（最多等待3秒）
  for (int i = 0; i < 30; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int status;
    pid_t result = waitpid(server_pid_, &status, WNOHANG);
    if (result > 0) {
      // 服务器已经退出
      std::cout << "Server stopped successfully, PID: " << server_pid_
                << std::endl;
      running_ = false;
      server_pid_ = 0;
      return true;
    } else if (result < 0) {
      // 发生错误
      std::cerr << "Error waiting for server to stop: " << strerror(errno)
                << std::endl;
      running_ = false;
      server_pid_ = 0;
      return false;
    }
  }

  // 超时，发送SIGKILL信号强制终止
  std::cerr << "Server stop timed out, sending SIGKILL" << std::endl;
  if (kill(server_pid_, SIGKILL) != 0) {
    std::cerr << "Failed to send SIGKILL to server: " << strerror(errno)
              << std::endl;
    running_ = false;
    server_pid_ = 0;
    return false;
  }

  // 等待服务器退出
  int status;
  waitpid(server_pid_, &status, 0);
  std::cout << "Server terminated with SIGKILL, PID: " << server_pid_
            << std::endl;
  running_ = false;
  server_pid_ = 0;
  return true;
}

bool ServerManager::IsRunning() const {
  if (!running_) {
    return false;
  }

  // 检查进程是否存在
  int status;
  pid_t result = waitpid(server_pid_, &status, WNOHANG);
  if (result == 0) {
    // 进程正在运行
    return true;
  } else {
    // 进程已经退出
    return false;
  }
}

int ServerManager::GetPort() const { return port_; }

pid_t ServerManager::GetPid() const { return server_pid_; }

} // namespace test
} // namespace sqlcc
