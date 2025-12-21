/**
 * @file pbkdf2.h
 * @brief PBKDF2密钥派生类定义
 *
 * Why: 需要安全的密钥派生函数来从口令生成加密密钥
 * What: PBKDF2类实现PBKDF2密钥派生算法
 * How: 使用HMAC-SHA256作为伪随机函数进行密钥派生
 */

#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace sqlcc {
namespace network {

/**
 * @brief PBKDF2密钥派生类
 *
 * 提供基于PBKDF2算法的密钥派生功能，用于从口令安全地派生加密密钥。
 */
class PBKDF2 {
public:
    /**
     * @brief 从口令派生密钥
     * @param passphrase 用户口令
     * @param salt 盐值
     * @param iterations 迭代次数（推荐10000或更多）
     * @param key_len 派生密钥长度（字节）
     * @return 派生的密钥
     */
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
