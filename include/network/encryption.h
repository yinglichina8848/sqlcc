/**
 * @file encryption.h
 * @brief 网络通信加密模块头文件
 * 
 * 该文件定义了SQLCC数据库系统的网络通信加密相关类和函数
 * 支持简单XOR加密和AES-256-CBC高级加密
 */

#ifndef SQLCC_ENCRYPTION_H
#define SQLCC_ENCRYPTION_H

#include <vector>
#include <string>
#include <memory>
#include <cstdint>

namespace sqlcc {
namespace network {

/**
 * @class EncryptionKey
 * @brief 加密密钥容器类
 * 
 * 存储加密密钥和初始化向量(IV)
 */
class EncryptionKey {
public:
    /**
     * @brief 构造函数
     * @param key 加密密钥
     * @param iv 初始化向量
     */
    EncryptionKey(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv);
    
    /**
     * @brief 生成随机密钥和IV
     * @param key_size 密钥大小（字节）
     * @param iv_size IV大小（字节）
     * @return 生成的密钥对象
     */
    static std::shared_ptr<EncryptionKey> GenerateRandom(size_t key_size = 32, size_t iv_size = 16);
    
    const std::vector<uint8_t>& GetKey() const { return key_; }
    const std::vector<uint8_t>& GetIV() const { return iv_; }
    std::vector<uint8_t>& GetKey() { return key_; }
    std::vector<uint8_t>& GetIV() { return iv_; }

private:
    std::vector<uint8_t> key_;  ///< 加密密钥
    std::vector<uint8_t> iv_;   ///< 初始化向量
};

/**
 * @class SimpleEncryptor
 * @brief 简单加密器类
 * 
 * 提供基于固定密钥的XOR加密和解密功能
 */
class SimpleEncryptor {
public:
    /**
     * @brief 构造函数
     * @param key 加密密钥
     */
    explicit SimpleEncryptor(const std::string& key);
    
    /**
     * @brief 加密数据
     * @param data 待加密的数据
     * @return 加密后的数据
     */
    std::vector<char> Encrypt(const std::vector<char>& data) const;
    
    /**
     * @brief 解密数据
     * @param data 待解密的数据
     * @return 解密后的数据
     */
    std::vector<char> Decrypt(const std::vector<char>& data) const;

private:
    std::string key_;  ///< 加密密钥
};

/**
 * @class AESEncryptor
 * @brief AES-256-CBC加密器类
 * 
 * 提供基于AES-256-CBC算法的加密和解密功能
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

    // 访问加密密钥材料（用于HMAC等用途）
    const std::vector<uint8_t>& GetKeyBytes() const { return encryption_key_->GetKey(); }

private:
    std::shared_ptr<EncryptionKey> encryption_key_;  ///< 加密密钥和IV
    
    /**
     * @brief 初始化加密上下文
     * @return 成功返回true，否则返回false
     */
    bool InitializeContext();
    
};

// HMAC-SHA256 防篡改
class HMACSHA256 {
public:
    static std::vector<uint8_t> Compute(const std::vector<uint8_t>& key,
                                        const std::vector<uint8_t>& data);
    static bool Verify(const std::vector<uint8_t>& key,
                       const std::vector<uint8_t>& data,
                       const std::vector<uint8_t>& mac);
};

// PBKDF2 密钥派生
class PBKDF2 {
public:
    static std::vector<uint8_t> Derive(const std::string& passphrase,
                                       const std::vector<uint8_t>& salt,
                                       int iterations,
                                       size_t key_len);
};

// 基于PBKDF2从口令派生AES密钥与IV
std::shared_ptr<EncryptionKey> DeriveEncryptionKeyFromPassword(
        const std::string& passphrase,
        const std::vector<uint8_t>& salt,
        int iterations,
        size_t key_len = 32,
        size_t iv_len = 16);

} // namespace network
} // namespace sqlcc

#endif // SQLCC_ENCRYPTION_H