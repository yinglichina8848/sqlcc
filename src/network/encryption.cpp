/**
 * @file encryption.cpp
 * @brief 网络通信加密模块实现文件
 * 
 * 该文件实现了SQLCC数据库系统的网络通信加密相关类和接口
 */

#include "network/encryption.h"
#include <cstddef>

namespace sqlcc {
namespace network {

SimpleEncryptor::SimpleEncryptor(const std::string& key) : key_(key) {}

std::vector<char> SimpleEncryptor::Encrypt(const std::vector<char>& data) const {
    std::vector<char> encrypted_data = data;
    for (size_t i = 0; i < encrypted_data.size(); ++i) {
        encrypted_data[i] = data[i] ^ key_[i % key_.size()];
    }
    return encrypted_data;
}

std::vector<char> SimpleEncryptor::Decrypt(const std::vector<char>& data) const {
    // XOR解密与加密使用相同的操作
    return Encrypt(data);
}

} // namespace network
} // namespace sqlcc
