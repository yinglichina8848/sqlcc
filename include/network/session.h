/**
 * @file session.h
 * @brief 会话类定义
 *
 * Why: 需要专门的类来管理数据库会话的认证和加密状态
 * What: Session类封装用户会话的完整生命周期管理
 * How: 提供会话认证、加密控制和权限检查功能
 */

#pragma once

#include <memory>
#include <string>
#include "network/encryption.h"

namespace sqlcc {
namespace network {

/**
 * @brief 会话类
 *
 * 管理数据库用户的会话状态，包括认证、权限和加密设置
 */
class Session {
public:
    /**
     * @brief 构造函数
     * @param session_id 会话ID
     */
    explicit Session(int session_id);

    /**
     * @brief 获取会话ID
     * @return 会话ID
     */
    int GetSessionId() const { return session_id_; }

    /**
     * @brief 检查是否已认证
     * @return 是否已认证
     */
    bool IsAuthenticated() const { return authenticated_; }

    /**
     * @brief 获取用户名
     * @return 用户名
     */
    const std::string& GetUser() const { return user_; }

    /**
     * @brief 设置认证状态
     * @param user 用户名
     */
    void SetAuthenticated(const std::string& user) {
        authenticated_ = true;
        user_ = user;
    }

    /**
     * @brief 设置加密禁用状态
     * @param disabled 是否禁用加密
     */
    void SetEncryptionDisabled(bool disabled);

    /**
     * @brief 检查是否禁用加密
     * @return 是否禁用加密
     */
    bool IsEncryptionDisabled() const;

    /**
     * @brief 设置认证禁用状态
     * @param disabled 是否禁用认证
     */
    void SetAuthenticationDisabled(bool disabled);

    /**
     * @brief 检查是否禁用认证
     * @return 是否禁用认证
     */
    bool IsAuthenticationDisabled() const;

    /**
     * @brief 设置AES加密器
     * @param encryptor AES加密器指针
     */
    void SetAESEncryptor(std::shared_ptr<AESEncryptor> encryptor);

    /**
     * @brief 获取AES加密器
     * @return AES加密器指针
     */
    std::shared_ptr<AESEncryptor> GetAESEncryptor() const;

    /**
     * @brief 检查是否启用AES加密
     * @return 是否启用AES加密
     */
    bool IsAESEncryptionEnabled() const;

private:
    int session_id_;                           ///< 会话ID
    bool authenticated_;                       ///< 是否已认证
    std::string user_;                         ///< 用户名
    bool encryption_disabled_;                 ///< 是否禁用加密
    bool authentication_disabled_;             ///< 是否禁用认证
    std::shared_ptr<AESEncryptor> aes_encryptor_; ///< AES加密器
};

} // namespace network
} // namespace sqlcc
