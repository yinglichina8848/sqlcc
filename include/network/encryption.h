/**
 * @file encryption.h
 * @brief 网络通信加密模块头文件
 * 
 * 该文件定义了SQLCC数据库系统的网络通信加密相关类和函数
 */

#ifndef SQLCC_ENCRYPTION_H
#define SQLCC_ENCRYPTION_H

#include <vector>
#include <string>

namespace sqlcc {
namespace network {

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

} // namespace network
} // namespace sqlcc

#endif // SQLCC_ENCRYPTION_H