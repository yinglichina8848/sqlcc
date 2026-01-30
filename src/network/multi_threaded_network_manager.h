#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <queue>
#include <vector>

#include "utils/config_manager.h"
#include "network/connection_state.h"
#include "execution/task_executor.h"

namespace sqlcc {

class DatabaseManager;
class SqlExecutor;

/**
 * @brief 网络请求上下文
 */
struct NetworkRequest {
  int client_id;
  std::string request_data;
  std::chrono::steady_clock::time_point received_at;
  std::shared_ptr<ConnectionState> connection;

  NetworkRequest(int id, const std::string& data, std::shared_ptr<ConnectionState> conn)
      : client_id(id), request_data(data), received_at(std::chrono::steady_clock::now()),
        connection(conn) {}
};

/**
 * @brief 网络响应上下文
 */
struct NetworkResponse {
  int client_id;
  std::string response_data;
  std::chrono::steady_clock::time_point processed_at;
  bool success;

  NetworkResponse(int id, const std::string& data, bool ok = true)
      : client_id(id), response_data(data), processed_at(std::chrono::steady_clock::now()),
        success(ok) {}
};

/**
 * @brief 多线程网络管理器
 *
 * 负责处理高并发的网络连接和请求，使用多线程架构
 */
class MultiThreadedNetworkManager {
public:
  /**
   * @brief 网络统计信息
   */
  struct NetworkStats {
    std::atomic<size_t> total_connections{0};
    std::atomic<size_t> active_connections{0};
    std::atomic<size_t> total_requests{0};
    std::atomic<size_t> processed_requests{0};
    std::atomic<size_t> failed_requests{0};
    std::atomic<size_t> queued_requests{0};
    std::chrono::microseconds avg_request_time{0};
    std::chrono::microseconds max_request_time{0};

    double requests_per_second() const {
      // 简化的QPS计算，实际应基于时间窗口
      return processed_requests.load() / 1.0; // 假设1秒
    }

    double connection_utilization() const {
      return static_cast<double>(active_connections.load()) / total_connections.load();
    }
  };

  /**
   * @brief 网络配置
   */
  struct NetworkConfig {
    int port{5432};
    int max_connections{10000};
    int network_threads{32};      // 网络I/O线程数
    int worker_threads{64};       // 工作线程数
    int max_queue_size{100000};   // 最大队列大小
    std::chrono::seconds connection_timeout{300}; // 连接超时
    bool enable_ssl{false};
    std::string ssl_cert_path;
    std::string ssl_key_path;
  };

  /**
   * @brief 构造函数
   * @param config_manager 配置管理器引用
   * @param database_manager 数据库管理器引用
   * @param task_executor 任务执行器引用
   */
  MultiThreadedNetworkManager(ConfigManager& config_manager,
                             DatabaseManager& database_manager,
                             TaskExecutor& task_executor);

  /**
   * @brief 析构函数
   */
  ~MultiThreadedNetworkManager();

  /**
   * @brief 启动网络服务
   * @return 是否成功
   */
  bool Start();

  /**
   * @brief 停止网络服务
   */
  void Stop();

  /**
   * @brief 处理客户端请求
   * @param client_id 客户端ID
   * @param request_data 请求数据
   * @param connection 连接状态
   * @return 是否成功
   */
  bool HandleRequest(int client_id, const std::string& request_data,
                    std::shared_ptr<ConnectionState> connection);

  /**
   * @brief 发送响应给客户端
   * @param client_id 客户端ID
   * @param response_data 响应数据
   * @return 是否成功
   */
  bool SendResponse(int client_id, const std::string& response_data);

  /**
   * @brief 断开客户端连接
   * @param client_id 客户端ID
   */
  void DisconnectClient(int client_id);

  /**
   * @brief 获取网络统计信息
   * @return 统计信息
   */
  NetworkStats GetStats() const;

  /**
   * @brief 重置统计信息
   */
  void ResetStats();

  /**
   * @brief 设置网络配置
   * @param config 网络配置
   */
  void SetConfig(const NetworkConfig& config);

  /**
   * @brief 获取当前网络配置
   * @return 网络配置
   */
  NetworkConfig GetConfig() const;

  /**
   * @brief 检查服务是否运行中
   * @return 是否运行中
   */
  bool IsRunning() const;

private:
  /**
   * @brief 网络监听线程函数
   */
  void NetworkListenerThread();

  /**
   * @brief 网络I/O线程函数
   */
  void NetworkIOThread();

  /**
   * @brief 请求处理线程函数
   */
  void RequestProcessorThread();

  /**
   * @brief 响应发送线程函数
   */
  void ResponseSenderThread();

  /**
   * @brief 处理单个请求
   * @param request 网络请求
   */
  void ProcessRequest(std::unique_ptr<NetworkRequest> request);

  /**
   * @brief 执行SQL查询
   * @param sql SQL语句
   * @param client_id 客户端ID
   * @return 查询结果
   */
  std::string ExecuteSQL(const std::string& sql, int client_id);

  /**
   * @brief 格式化错误响应
   * @param error_message 错误信息
   * @return 格式化的错误响应
   */
  std::string FormatErrorResponse(const std::string& error_message);

  /**
   * @brief 格式化成功响应
   * @param result 结果数据
   * @return 格式化的成功响应
   */
  std::string FormatSuccessResponse(const std::vector<std::vector<std::string>>& result);

  // 配置和依赖
  ConfigManager& config_manager_;
  DatabaseManager& database_manager_;
  TaskExecutor& task_executor_;

  // 网络配置
  mutable std::mutex config_mutex_;
  NetworkConfig config_;

  // 线程管理
  std::vector<std::thread> network_threads_;
  std::vector<std::thread> worker_threads_;
  std::vector<std::thread> response_threads_;
  std::atomic<bool> running_;

  // 请求队列
  std::queue<std::unique_ptr<NetworkRequest>> request_queue_;
  mutable std::mutex request_mutex_;
  std::condition_variable request_cv_;

  // 响应队列
  std::queue<std::unique_ptr<NetworkResponse>> response_queue_;
  mutable std::mutex response_mutex_;
  std::condition_variable response_cv_;

  // 连接管理
  std::unordered_map<int, std::shared_ptr<ConnectionState>> active_connections_;
  mutable std::mutex connection_mutex_;

  // 统计信息
  mutable std::mutex stats_mutex_;
  NetworkStats stats_;

  // 网络监听器（简化实现，实际应使用epoll/kqueue等）
  int server_socket_;
  std::atomic<int> next_client_id_;
};

} // namespace sqlcc
