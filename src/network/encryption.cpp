le/**
 * @file encryption.cpp
 * @brief 网络通信加密模块实现文件
 * 
 * 该文件实现了SQLCC数据库系统的网络通信加密相关类和接口
 */

#include "../../include/network/encryption.h"
#include <cstring>
#include <iostream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

namespace sqlcc::network {

Encryptor::Encryptor(const std::string& password) : password_(password) {
    GenerateKeyIv(password);
}

Encryptor::~Encryptor() {
    // 清理密钥和IV
    std::fill(key_.begin(), key_.end(), 0);
    std::fill(iv_.begin(), iv_.end(), 0);
}

void Encryptor::GenerateKeyIv(const std::string& password) {
    // 生成256位密钥
    key_.resize(32); // AES-256需要32字节密钥
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.length(), key_.data());
    
    // 生成128位IV
    iv_.resize(16); // AES需要16字节IV
    // 在实际应用中，应该使用安全的随机数生成器
    // 这里为了简化，使用密码的MD5哈希作为IV
    unsigned char temp_iv[16];
    MD5(reinterpret_cast<const unsigned char*>(password.c_str()), password.length(), temp_iv);
    std::copy(temp_iv, temp_iv + 16, iv_.begin());
}

std::vector<char> Encryptor::Encrypt(const std::vector<char>& data) const {
    if (data.empty()) {
        return data;
    }
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return std::vector<char>(); // 返回空向量表示失败
    }
    
    // 初始化加密操作
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key_.data(), iv_.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return std::vector<char>();
    }
    
    // 分配输出缓冲区
    std::vector<char> encrypted_data(data.size() + AES_BLOCK_SIZE);
    int len;
    int ciphertext_len;
    
    // 执行加密
    if (EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(encrypted_data.data()), &len, 
                          reinterpret_cast<const unsigned char*>(data.data()), data.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return std::vector<char>();
    }
    ciphertext_len = len;
    
    // 完成加密
    if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(encrypted_data.data()) + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return std::vector<char>();
    }
    ciphertext_len += len;
    
    // 释放上下文
    EVP_CIPHER_CTX_free(ctx);
    
    // 调整输出缓冲区大小
    encrypted_data.resize(ciphertext_len);
    return encrypted_data;
}

std::vector<char> Encryptor::Decrypt(const std::vector<char>& encrypted_data) const {
    if (encrypted_data.empty()) {
        return std::vector<char>();
    }
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return std::vector<char>(); // 返回空向量表示失败
    }
    
    // 初始化解密操作
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key_.data(), iv_.data()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return std::vector<char>();
    }
    
    // 分配输出缓冲区
    std::vector<char> decrypted_data(encrypted_data.size());
    int len;
    int plaintext_len;
    
    // 执行解密
    if (EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(decrypted_data.data()), &len,
                          reinterpret_cast<const unsigned char*>(encrypted_data.data()), encrypted_data.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return std::vector<char>();
    }
    plaintext_len = len;
    
    // 完成解密
    if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(decrypted_data.data()) + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return std::vector<char>();
    }
    plaintext_len += len;
    
    // 释放上下文
    EVP_CIPHER_CTX_free(ctx);
    
    // 调整输出缓冲区大小
    decrypted_data.resize(plaintext_len);
    return decrypted_data;
}

}

SimpleEncryptor::SimpleEncryptor(const std::string& key) : key_(key) {}

std::vector<char> SimpleEncryptor::Encrypt(const std::vector<char>& data) {
    std::vector<char> encrypted_data(data.size());
    
    for (size_t i = 0; i < data.size(); ++i) {
        // 使用简单的XOR加密
        encrypted_data[i] = data[i] ^ key_[i % key_.length()];
    }
    
    return encrypted_data;
}

std::vector<char> SimpleEncryptor::Decrypt(const std::vector<char>& data) {
    // XOR解密与加密使用相同的操作
    return Encrypt(data);
}

} // namespace network
} // namespace sqlcc