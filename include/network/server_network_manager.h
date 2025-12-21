/**
 * @file server_network_manager.h
 * @brief 服务器网络管理器头文件
 */

#ifndef SQLCC_NETWORK_SERVER_NETWORK_MANAGER_H
#define SQLCC_NETWORK_SERVER_NETWORK_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "network/network_stability_guard.h"
#include "network/network_monitor.h"
#include "network/network_exception_handler.h"

namespace sqlcc {
namespace network {

// 服务器网络管理器 - 服务器端网络管理
class ServerNetworkManager {
public:
    ServerNetworkManager();
    ~ServerNetworkManager();

    // 服务器状态
    enum ServerState {
        STOPPED = 0,
        STARTING = 1,
        RUNNING = 2,
        STOPPING = 3,
        ERROR = 4
    };

    // 初始化和配置
    bool Initialize(const std::string& host, int port);
    void SetMaxConnections(int max_connections);
    void SetConnectionTimeout(std::chrono::seconds timeout);
    void SetThreadPoolSize(int num_threads);

    // 服务器控制
    bool Start();
    bool Stop();
    bool Restart();

    // 连接管理
    void HandleNewConnection(int client_socket, const std::string& client_address);
    void HandleConnectionClosed(int client_id);
    void ForceDisconnectClient(int client_id);

    // 消息处理
    void HandleMessage(int client_id, const std::vector<uint8_t>& message);
    void SendMessage(int client_id, const std::vector<uint8_t>& message);
    void BroadcastMessage(const std::vector<uint8_t>& message);

    // 监控和统计
    ServerState GetServerState() const;
    int GetActiveConnections() const;
    std::vector<int> GetClientIds() const;
    std::string GetServerStats() const;

    // 健康检查
    bool IsHealthy() const;
    std::vector<std::string> GetHealthIssues() const;

private:
    // 服务器配置
    std::string host_;
    int port_;
    int max_connections_;
    std::chrono::seconds connection_timeout_;
    int thread_pool_size_;

    // 服务器状态
    mutable std::mutex server_mutex_;
    std::condition_variable server_cv_;
    ServerState server_state_;
    int server_socket_;
    std::vector<std::thread> worker_threads_;
    bool should_stop_;

    // 连接管理
    mutable std::mutex connections_mutex_;
    std::unordered_map<int, ClientConnection> active_connections_;
    int next_client_id_;

    // 组件依赖
    std::unique_ptr<NetworkMonitor> monitor_;
    std::unique_ptr<NetworkExceptionHandler> exception_handler_;
    std::unique_ptr<NetworkStabilityGuard> stability_guard_;

    // 消息队列
    mutable std::mutex message_queue_mutex_;
    std::condition_variable message_cv_;
    std::vector<std::pair<int, std::vector<uint8_t>>> message_queue_;

    // 辅助方法
    void AcceptConnections();
    void ProcessMessages();
    void CleanupConnections();
    int CreateServerSocket();
    void SetupSocketOptions(int socket);
    bool BindAndListen(int socket);
    bool IsValidClientId(int client_id) const;
    void LogServerEvent(const std::string& event, const std::string& details = "");
};

} // namespace network
} // namespace sqlcc

#endif // SQLCC_NETWORK_SERVER_NETWORK_MANAGER_H
