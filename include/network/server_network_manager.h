#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace sqlcc {
namespace network {

class SessionManager;
class SqlExecutor;
class ConnectionHandler;

/**
 * @brief 服务端网络管理器类
 *
 * ServerNetworkManager类负责服务端的网络监听和管理，包括：
 * - TCP监听socket的管理
 * - 客户端连接的接受和处理
 * - epoll事件循环的管理
 * - TLS/SSL连接的支持
 * - 连接池的管理
 */
class ServerNetworkManager {
public:
    /**
     * @brief 构造函数
     * @param port 监听端口
     * @param max_connections 最大连接数
     */
    ServerNetworkManager(int port, int max_connections);

    /**
     * @brief 析构函数
     */
    ~ServerNetworkManager();

    /**
     * @brief 启动服务器
     * @return 启动是否成功
     */
    bool Start();

    /**
     * @brief 停止服务器
     */
    void Stop();

    /**
     * @brief 处理网络事件
     */
    void ProcessEvents();

    /**
     * @brief 设置SQL执行器
     * @param sql_executor SQL执行器智能指针
     */
    void SetSqlExecutor(std::shared_ptr<sqlcc::SqlExecutor> sql_executor);

    /**
     * @brief 启用TLS
     * @param enabled 是否启用TLS
     */
    void EnableTLS(bool enabled);

    /**
     * @brief 配置TLS服务端证书
     * @param cert_path 证书文件路径
     * @param key_path 私钥文件路径
     * @param ca_cert_path CA证书文件路径（可选）
     * @return 配置是否成功
     */
#ifdef __linux__
    bool ConfigureTLSServer(const std::string& cert_path,
                           const std::string& key_path,
                           const std::string& ca_cert_path = "");
#endif

private:
    /**
     * @brief 接受新连接
     */
    void AcceptConnection();

    int port_;                                         ///< 监听端口
    int max_connections_;                              ///< 最大连接数
    bool running_;                                     ///< 运行状态
    std::shared_ptr<SessionManager> session_manager_;  ///< 会话管理器
    std::shared_ptr<sqlcc::SqlExecutor> sql_executor_; ///< SQL执行器

    // Linux specific members
#ifdef __linux__
    int listen_fd_;                                    ///< 监听socket文件描述符
    int epoll_fd_;                                     ///< epoll文件描述符
    bool tls_enabled_;                                 ///< TLS启用标志
    struct ssl_ctx_st* ssl_ctx_;                       ///< SSL上下文
#endif

    // Connection management
    std::unordered_map<int, std::unique_ptr<ConnectionHandler>> connections_;  ///< 连接处理器映射
    std::mutex connections_mutex_;                      ///< 连接映射互斥锁
};

} // namespace network
} // namespace sqlcc
