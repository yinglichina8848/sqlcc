/**
 * @file simple_encryptor.h
 * @brief 简单加密器类定义（XOR加密）
 *
 * Why: 需要一个轻量级的加密器用于基础数据保护
 * What: SimpleEncryptor类实现基于固定密钥的XOR加密
 * How: 使用固定密钥进行XOR运算，提供基本的加密保护
 */

#pragma once

#include <vector>
#include <string>

namespace sqlcc {
namespace network {

/**
 * @brief 简单加密器类
 *
 * 提供基于固定密钥的XOR加密和解密功能。
 * 这是一个轻量级的加密器，适合对性能要求较高但安全等级不高的场景。
 */
class SimpleEncryptor {
public:
    /**
     * @brief 构造函数
     * @param key 加密密钥字符串
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

} // namespace network
} // namespace sqlcc
