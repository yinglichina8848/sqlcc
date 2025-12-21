/**
 * @file client_network_manager.h
 * @brief 客户端网络管理器头文件
 */

#ifndef SQLCC_NETWORK_CLIENT_NETWORK_MANAGER_H
#define SQLCC_NETWORK_CLIENT_NETWORK_MANAGER_H

#include <string>
#include <memory>
#include <vector>

#include "network/client_connection.h"
#include "network/session_manager.h"
#include "network/encryption.h"

namespace sqlcc {
namespace network {

// 客户端网络管理器
class ClientNetworkManager {
public:
    ClientNetworkManager(const std::string& host, int port);
    ~ClientNetworkManager();

    bool Connect();
    void Disconnect();
    bool IsConnected() const;
    bool SendRequest(const std::vector<char>& request);
    std::vector<char> ReceiveResponse();
    bool ConnectAndAuthenticate(const std::string& username,
                               const std::string& password);
    bool SendAuthMessage(const std::string& username, const std::string& password);

    // AES加密支持
    bool InitiateKeyExchange();  // 启动密钥交换
    void SetAESEncryptor(std::shared_ptr<AESEncryptor> encryptor);
    std::shared_ptr<AESEncryptor> GetAESEncryptor() const;
    bool IsAESEncryptionEnabled() const;

    // AES 加密/解密（对测试公开）
    std::vector<char> EncryptMessage(const std::vector<char>& message);
    std::vector<char> DecryptMessage(const std::vector<char>& message);

    // TLS 客户端支持
    void EnableTLS(bool enabled);
#ifdef __linux__
    bool ConfigureTLSClient(const std::string& ca_cert_path);
#endif

private:
    // AES加密/解密实现（在实现文件中定义）

    std::unique_ptr<ClientConnection> connection_;
    std::shared_ptr<SessionManager> session_manager_;
    std::shared_ptr<AESEncryptor> aes_encryptor_;  // AES加密器
};

} // namespace network
} // namespace sqlcc

#endif // SQLCC_NETWORK_CLIENT_NETWORK_MANAGER_H
