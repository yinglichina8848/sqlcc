/**
 * @file encryption.h
 * @brief 网络通信加密模块头文件
 *
 * 该文件定义了SQLCC数据库系统的网络通信加密相关类和接口
 */

#ifndef SQLCC_NETWORK_ENCRYPTION_H
#define SQLCC_NETWORK_ENCRYPTION_H

#include <vector>
#include <string>

namespace sqlcc::network {

/**
 * @brief 简单加密器类
 * 
 * 提供简单的XOR加密功能，用于网络通信加密
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
     * @param data 明文数据
     * @return 加密后的数据
     */
    std::vector<char> Encrypt(const std::vector<char>& data);
    
    /**
     * @brief 解密数据
     * @param data 密文数据
     * @return 解密后的数据
     */
    std::vector<char> Decrypt(const std::vector<char>& data);
    
private:
    std::string key_;
};

} // namespace sqlcc::network

#endif // SQLCC_NETWORK_ENCRYPTION_H