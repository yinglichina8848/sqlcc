/**
 * @file client_connection.h
 * @brief 客户端连接类头文件
 */

#ifndef SQLCC_NETWORK_CLIENT_CONNECTION_H
#define SQLCC_NETWORK_CLIENT_CONNECTION_H

#include <string>
#include <memory>
#include <vector>

#include "sql_executor.h"
#include "network/encryption.h"
#include "utils/file_descriptor.h"
#include "utils/ssl_wrapper.h"

#ifdef __linux__
#include <openssl/ssl.h>
#endif

namespace sqlcc {
namespace network {

// 客户端连接类
class ClientConnection {
public:
    ClientConnection(const std::string& host, int port);
    ~ClientConnection();

    bool Connect();
    void Disconnect();
    bool IsConnected() const;
    bool SendData(const std::vector<char>& data);
    std::vector<char> ReceiveData();
    // Test helper: return associated session id (0 if none)
    int GetSessionId() const { return 0; }

    // TLS/SSL 支持
    void EnableTLS(bool enabled);
#ifdef __linux__
    bool ConfigureTLSClient(const std::string& ca_cert_path);
#endif

private:
    std::string host_;
    int port_;
    bool connected_;
    sqlcc::FileDescriptor socket_fd_;
#ifdef __linux__
    bool tls_enabled_ = false;
    std::string ca_cert_path_;
    sqlcc::utils::SSLContext ssl_ctx_; // SSL_CTX RAII包装器
    sqlcc::utils::SSLSocket ssl_;      // SSL RAII包装器
#endif
};

} // namespace network
} // namespace sqlcc

#endif // SQLCC_NETWORK_CLIENT_CONNECTION_H
