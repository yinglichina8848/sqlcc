/**
 * @file connection_handler.h
 * @brief 连接处理器头文件
 */

#ifndef SQLCC_NETWORK_CONNECTION_HANDLER_H
#define SQLCC_NETWORK_CONNECTION_HANDLER_H

#include <memory>
#include <queue>
#include <vector>
#include <mutex>

#include "utils/file_descriptor.h"
#include "network/session_manager.h"
#include "sql_executor.h"

#ifdef __linux__
#include "utils/ssl_wrapper.h"
#endif

namespace sqlcc {
namespace network {

// 连接处理器
class ConnectionHandler {
public:
    ConnectionHandler(sqlcc::FileDescriptor&& fd, std::shared_ptr<SessionManager> session_manager, std::shared_ptr<sqlcc::SqlExecutor> sql_executor);
    ~ConnectionHandler();

    int GetFd() const;
    bool IsClosed() const;
    void HandleEvent(uint32_t events);
    void HandleRead();  // 设为public以便NetworkServer调用
    void ProcessMessage(const std::vector<char>& data);

#ifdef __linux__
    void SetTLS(struct ssl_st* ssl, bool enabled);
#endif

private:
    void HandleWrite();
    void SendMessage(const std::vector<char>& message);
    bool TrySendImmediately(const std::vector<char>& data);
    void Close();

    void HandleConnectMessage(const std::vector<char>& data);
    void HandleAuthMessage(const std::vector<char>& data);
    void HandleQueryMessage(const std::vector<char>& data);
    void HandleKeyExchangeMessage(const std::vector<char>& data);
    void SendErrorMessage(const std::string& error);

    // AES加密半加密/解密方法
    std::vector<char> EncryptMessage(const std::vector<char>& message);
    std::vector<char> DecryptMessage(const std::vector<char>& message);

    sqlcc::FileDescriptor fd_;  // RAII文件描述符管理
    std::shared_ptr<SessionManager> session_manager_;
    std::shared_ptr<sqlcc::SqlExecutor> sql_executor_;
    std::shared_ptr<Session> session_;
    bool closed_;
    std::queue<std::vector<char>> write_queue_;
    std::mutex write_mutex_;
#ifdef __linux__
    sqlcc::utils::SSLSocket ssl_;      // SSL RAII包装器
    bool tls_enabled_ = false;
#endif
};

} // namespace network
} // namespace sqlcc

#endif // SQLCC_NETWORK_CONNECTION_HANDLER_H
