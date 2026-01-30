/**
 * @file aes_encryptor.h
 * @brief AES-256-CBC加密器类定义
 *
 * Why: 需要高级加密算法来保护敏感数据传输
 * What: AESEncryptor类实现AES-256-CBC加密算法
 * How: 使用OpenSSL或其他AES库提供高级加密功能
 */

#pragma once

#include "src/network/encryption/encryption_key.h"
#include <vector>
#include <memory>
#include <cstdint>

namespace sqlcc {
namespace network {

/**
 * @brief AES-256-CBC加密器类
 *
 * 提供基于AES-256-CBC算法的高级加密和解密功能。
 * 使用加密密钥容器管理密钥和初始化向量。
 */
class AESEncryptor {
public:
    /**
     * @brief 构造函数
     * @param encryption_key 加密密钥和IV
     */
    explicit AESEncryptor(std::shared_ptr<EncryptionKey> encryption_key);

    /**
     * @brief 析构函数
     */
    ~AESEncryptor();

    /**
     * @brief 加密数据
     * @param data 待加密的数据
     * @return 加密后的数据
     */
    std::vector<uint8_t> Encrypt(const std::vector<uint8_t>& data) const;

    /**
     * @brief 解密数据
     * @param data 待解密的数据
     * @return 解密后的数据
     */
    std::vector<uint8_t> Decrypt(const std::vector<uint8_t>& data) const;

    /**
     * @brief 更新加密密钥
     * @param encryption_key 新的加密密钥和IV
     */
    void UpdateKey(std::shared_ptr<EncryptionKey> encryption_key);

    /**
     * @brief 检查AES库是否可用
     * @return 如果AES库可用返回true，否则返回false
     */
    static bool IsAvailable();

    /**
     * @brief 获取密钥字节（用于HMAC等用途）
     * @return 加密密钥的字节数据
     */
    const std::vector<uint8_t>& GetKeyBytes() const { return encryption_key_->GetKey(); }

private:
    std::shared_ptr<EncryptionKey> encryption_key_;  ///< 加密密钥和IV

    /**
     * @brief 初始化加密上下文
     * @return 成功返回true，否则返回false
     */
    bool InitializeContext();
};

} // namespace network
} // namespace sqlcc
