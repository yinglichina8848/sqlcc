#pragma once

#include <memory>
#include <string>

namespace sqlcc {
namespace network {
namespace encryption {
class AESEncryptor;
} // namespace encryption

/**
 * @brief 会话类，管理客户端连接的会话状态
 *
 * Session类负责管理单个客户端连接的会话状态，包括：
 * - 会话ID分配和管理
 * - 身份验证状态跟踪
 * - 加密设置控制
 * - AES加密器管理
 */
class Session {
public:
    /**
     * @brief 构造函数
     * @param session_id 会话ID，必须为正数
     */
    explicit Session(int session_id);

    /**
     * @brief 设置加密禁用状态
     * @param disabled true表示禁用加密，false表示启用加密
     */
    void SetEncryptionDisabled(bool disabled);

    /**
     * @brief 获取加密禁用状态
     * @return true表示加密被禁用，false表示加密启用
     */
    bool IsEncryptionDisabled() const;

    /**
     * @brief 设置认证禁用状态
     * @param disabled true表示禁用认证，false表示启用认证
     */
    void SetAuthenticationDisabled(bool disabled);

    /**
     * @brief 获取认证禁用状态
     * @return true表示认证被禁用，false表示认证启用
     */
    bool IsAuthenticationDisabled() const;

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
     * @brief 获取会话ID
     * @return 会话ID
     */
    int GetSessionId() const { return session_id_; }

    /**
     * @brief 设置认证状态
     * @param username 已认证的用户名
     */
    void SetAuthenticated(const std::string& username);

    /**
     * @brief 检查是否已认证
     * @return true表示已认证，false表示未认证
     */
    bool IsAuthenticated() const { return authenticated_; }

    /**
     * @brief 获取已认证的用户名
     * @return 用户名，如果未认证返回空字符串
     */
    const std::string& GetUsername() const { return username_; }

private:
    int session_id_;                                    ///< 会话ID
    bool authenticated_;                               ///< 认证状态
    bool encryption_disabled_;                         ///< 加密禁用标志
    bool authentication_disabled_;                     ///< 认证禁用标志
    std::string username_;                             ///< 已认证的用户名
    std::shared_ptr<class AESEncryptor> aes_encryptor_; ///< AES加密器
};

} // namespace network
} // namespace sqlcc
