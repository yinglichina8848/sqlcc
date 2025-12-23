#pragma once

#include <string>
#include <vector>
#include <memory>
#include "utils/ssl_wrapper.h"
#include "utils/file_descriptor.h"

namespace sqlcc {
namespace network {

/**
 * @brief 客户端连接类，管理TCP连接和TLS握手
 *
 * ClientConnection类负责建立和管理与服务器的TCP连接，
 * 支持TLS加密通信，包括证书验证和安全握手。
 */
class ClientConnection {
public:
    /**
     * @brief 构造函数
     * @param host 服务器主机地址
     * @param port 服务器端口号
     */
    ClientConnection(const std::string& host, int port);

    /**
     * @brief 析构函数，确保连接正确关闭
     */
    ~ClientConnection();

    /**
     * @brief 启用TLS加密
     * @param enabled 是否启用TLS
     */
    void EnableTLS(bool enabled);

    /**
     * @brief 配置TLS客户端证书验证
     * @param ca_cert_path CA证书文件路径
     * @return 配置是否成功
     */
#ifdef __linux__
    bool ConfigureTLSClient(const std::string& ca_cert_path);
#endif

    /**
     * @brief 建立连接
     * @return 连接是否成功
     */
    bool Connect();

    /**
     * @brief 断开连接
     */
    void Disconnect();

    /**
     * @brief 检查连接状态
     * @return true表示已连接，false表示未连接
     */
    bool IsConnected() const;

    /**
     * @brief 发送数据
     * @param data 要发送的数据
     * @return 发送是否成功
     */
    bool SendData(const std::vector<char>& data);

    /**
     * @brief 接收数据
     * @return 接收到的数据，如果失败返回空向量
     */
    std::vector<char> ReceiveData();

private:
    std::string host_;                    ///< 服务器主机地址
    int port_;                           ///< 服务器端口号
    bool connected_;                     ///< 连接状态
    bool tls_enabled_;                   ///< TLS启用状态
    std::string ca_cert_path_;           ///< CA证书路径

    // Linux specific members
#ifdef __linux__
    sqlcc::FileDescriptor socket_fd_;  ///< Socket文件描述符
    sqlcc::utils::SSLSocket ssl_;             ///< SSL连接对象
    sqlcc::utils::SSLContext ssl_ctx_;        ///< SSL上下文对象
#endif
};

} // namespace network
} // namespace sqlcc
