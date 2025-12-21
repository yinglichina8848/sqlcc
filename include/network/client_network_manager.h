#pragma once

#include <memory>
#include <string>
#include <vector>

namespace sqlcc {
namespace network {

class ClientConnection;
class Session;

/**
 * @brief 客户端网络管理器类
 *
 * ClientNetworkManager类负责客户端的网络通信逻辑，包括：
 * - 连接管理（连接、断开、重连）
 * - 消息收发和处理
 * - 加密通信支持
 * - 认证流程处理
 * - 密钥交换协议
 */
class ClientNetworkManager {
public:
    /**
     * @brief 构造函数
     * @param host 服务器主机地址
     * @param port 服务器端口号
     */
    ClientNetworkManager(const std::string& host, int port);

    /**
     * @brief 析构函数
     */
    ~ClientNetworkManager();

    /**
     * @brief 建立连接
     * @return 连接是否成功
     */
    bool Connect();

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
     * @brief 断开连接
     */
    void Disconnect();

    /**
     * @brief 检查连接状态
     * @return true表示已连接，false表示未连接
     */
    bool IsConnected() const;

    /**
     * @brief 发送请求消息
     * @param request 请求数据
     * @return 发送是否成功
     */
    bool SendRequest(const std::vector<char>& request);

    /**
     * @brief 接收响应消息
     * @return 接收到的响应数据
     */
    std::vector<char> ReceiveResponse();

    /**
     * @brief 发送认证消息
     * @param username 用户名
     * @param password 密码
     * @return 发送是否成功
     */
    bool SendAuthMessage(const std::string& username, const std::string& password);

    /**
     * @brief 发起密钥交换
     * @return 密钥交换是否成功
     */
    bool InitiateKeyExchange();

    /**
     * @brief 设置AES加密器
     * @param encryptor AES加密器智能指针
     */
    void SetAESEncryptor(std::shared_ptr<class AESEncryptor> encryptor);

    /**
     * @brief 获取AES加密器
     * @return AES加密器智能指针
     */
    std::shared_ptr<class AESEncryptor> GetAESEncryptor() const;

    /**
     * @brief 检查AES加密是否启用
     * @return true表示AES加密已启用，false表示未启用
     */
    bool IsAESEncryptionEnabled() const;

    /**
     * @brief 加密消息
     * @param message 明文消息
     * @return 加密后的消息
     */
    std::vector<char> EncryptMessage(const std::vector<char>& message);

    /**
     * @brief 解密消息
     * @param message 密文消息
     * @return 解密后的消息
     */
    std::vector<char> DecryptMessage(const std::vector<char>& message);

private:
    std::unique_ptr<ClientConnection> connection_;                    ///< 客户端连接对象
    std::shared_ptr<Session> session_;                               ///< 当前会话
    std::shared_ptr<class AESEncryptor> aes_encryptor_;              ///< AES加密器
};

} // namespace network
} // namespace sqlcc
