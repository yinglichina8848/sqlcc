/**
 * @file encryption.cpp
 * @brief 网络通信加密模块实现文件
 * 
 * 该文件实现了SQLCC数据库系统的网络通信加密相关类和接口
 */

#include "network/encryption.h"
#include <cstddef>
#include <cstring>
#include <random>
#include <stdexcept>

#ifdef __linux__
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#endif

namespace sqlcc {
namespace network {

// =====================
// EncryptionKey 实现
// =====================

EncryptionKey::EncryptionKey(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv)
    : key_(key), iv_(iv) {}

std::shared_ptr<EncryptionKey> EncryptionKey::GenerateRandom(size_t key_size, size_t iv_size) {
    std::vector<uint8_t> key(key_size);
    std::vector<uint8_t> iv(iv_size);
    
#ifdef __linux__
    // 使用OpenSSL的随机数生成器
    if (RAND_bytes(key.data(), key_size) != 1 || RAND_bytes(iv.data(), iv_size) != 1) {
        throw std::runtime_error("Failed to generate random bytes using OpenSSL");
    }
#else
    // 其他平台使用C++随机数生成器
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (size_t i = 0; i < key_size; ++i) {
        key[i] = static_cast<uint8_t>(dis(gen));
    }
    for (size_t i = 0; i < iv_size; ++i) {
        iv[i] = static_cast<uint8_t>(dis(gen));
    }
#endif
    
    return std::make_shared<EncryptionKey>(key, iv);
}

// =====================
// SimpleEncryptor 实现
// =====================

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

// =====================
// AESEncryptor 实现
// =====================

AESEncryptor::AESEncryptor(std::shared_ptr<EncryptionKey> encryption_key)
    : encryption_key_(encryption_key) {
    if (!encryption_key_) {
        throw std::invalid_argument("Encryption key cannot be null");
    }
}

AESEncryptor::~AESEncryptor() = default;

bool AESEncryptor::IsAvailable() {
#ifdef __linux__
    return true; // OpenSSL库可用
#else
    return false; // 其他平台不支持
#endif
}

std::vector<uint8_t> AESEncryptor::Encrypt(const std::vector<uint8_t>& data) const {
#ifdef __linux__
    if (!encryption_key_ || encryption_key_->GetKey().size() != 32) {
        throw std::runtime_error("Invalid encryption key size for AES-256");
    }
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP cipher context");
    }
    
    std::vector<uint8_t> encrypted_data(data.size() + EVP_MAX_BLOCK_LENGTH);
    int len = 0;
    int ciphertext_len = 0;
    
    try {
        // 初始化加密上下文（AES-256-CBC）
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, 
                               encryption_key_->GetKey().data(), 
                               encryption_key_->GetIV().data()) != 1) {
            throw std::runtime_error("Failed to initialize AES encryption");
        }
        
        // 加密数据
        if (EVP_EncryptUpdate(ctx, encrypted_data.data(), &len, data.data(), data.size()) != 1) {
            throw std::runtime_error("Failed to encrypt data");
        }
        ciphertext_len = len;
        
        // 处理最后的块
        if (EVP_EncryptFinal_ex(ctx, encrypted_data.data() + len, &len) != 1) {
            throw std::runtime_error("Failed to finalize encryption");
        }
        ciphertext_len += len;
        
        encrypted_data.resize(ciphertext_len);
        return encrypted_data;
    } catch (const std::exception&) {
        EVP_CIPHER_CTX_free(ctx);
        throw;
    }
#else
    throw std::runtime_error("AES encryption not supported on non-Linux platforms");
#endif
}

std::vector<uint8_t> AESEncryptor::Decrypt(const std::vector<uint8_t>& data) const {
#ifdef __linux__
    if (!encryption_key_ || encryption_key_->GetKey().size() != 32) {
        throw std::runtime_error("Invalid encryption key size for AES-256");
    }
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP cipher context");
    }
    
    std::vector<uint8_t> decrypted_data(data.size() + EVP_MAX_BLOCK_LENGTH);
    int len = 0;
    int plaintext_len = 0;
    
    try {
        // 初始化解密上下文（AES-256-CBC）
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, 
                               encryption_key_->GetKey().data(), 
                               encryption_key_->GetIV().data()) != 1) {
            throw std::runtime_error("Failed to initialize AES decryption");
        }
        
        // 解密数据
        if (EVP_DecryptUpdate(ctx, decrypted_data.data(), &len, data.data(), data.size()) != 1) {
            throw std::runtime_error("Failed to decrypt data");
        }
        plaintext_len = len;
        
        // 处理最后的块
        if (EVP_DecryptFinal_ex(ctx, decrypted_data.data() + len, &len) != 1) {
            throw std::runtime_error("Failed to finalize decryption");
        }
        plaintext_len += len;
        
        decrypted_data.resize(plaintext_len);
        return decrypted_data;
    } catch (const std::exception&) {
        EVP_CIPHER_CTX_free(ctx);
        throw;
    }
#else
    throw std::runtime_error("AES decryption not supported on non-Linux platforms");
#endif
}

void AESEncryptor::UpdateKey(std::shared_ptr<EncryptionKey> encryption_key) {
    if (!encryption_key) {
        throw std::invalid_argument("Encryption key cannot be null");
    }
    encryption_key_ = encryption_key;
}

bool AESEncryptor::InitializeContext() {
#ifdef __linux__
    return encryption_key_ && encryption_key_->GetKey().size() == 32;
#else
    return false;
#endif
}

// HMAC-SHA256 实现
std::vector<uint8_t> HMACSHA256::Compute(const std::vector<uint8_t>& key,
                                         const std::vector<uint8_t>& data) {
#ifdef __linux__
    unsigned int len = 0;
    std::vector<uint8_t> mac(EVP_MAX_MD_SIZE);
    HMAC(EVP_sha256(), key.data(), static_cast<int>(key.size()),
         reinterpret_cast<const unsigned char*>(data.data()), data.size(),
         mac.data(), &len);
    mac.resize(len);
    return mac;
#else
    throw std::runtime_error("HMAC-SHA256 not supported on non-Linux platforms");
#endif
}

bool HMACSHA256::Verify(const std::vector<uint8_t>& key,
                        const std::vector<uint8_t>& data,
                        const std::vector<uint8_t>& mac) {
#ifdef __linux__
    auto calc = Compute(key, data);
    if (calc.size() != mac.size()) return false;
    // 常量时间比较
    volatile uint8_t diff = 0;
    for (size_t i = 0; i < calc.size(); ++i) {
        diff |= static_cast<uint8_t>(calc[i] ^ mac[i]);
    }
    return diff == 0;
#else
    return false;
#endif
}

// PBKDF2 实现
std::vector<uint8_t> PBKDF2::Derive(const std::string& passphrase,
                                    const std::vector<uint8_t>& salt,
                                    int iterations,
                                    size_t key_len) {
#ifdef __linux__
    std::vector<uint8_t> out(key_len);
    if (PKCS5_PBKDF2_HMAC(passphrase.c_str(), static_cast<int>(passphrase.size()),
                           salt.data(), static_cast<int>(salt.size()),
                           iterations, EVP_sha256(), static_cast<int>(key_len), out.data()) != 1) {
        throw std::runtime_error("PBKDF2 derivation failed");
    }
    return out;
#else
    throw std::runtime_error("PBKDF2 not supported on non-Linux platforms");
#endif
}

// 基于PBKDF2派生AES密钥与IV
std::shared_ptr<EncryptionKey> DeriveEncryptionKeyFromPassword(
        const std::string& passphrase,
        const std::vector<uint8_t>& salt,
        int iterations,
        size_t key_len,
        size_t iv_len) {
#ifdef __linux__
    // 一次派生 key_len + iv_len 字节，前半为key，后半为iv
    std::vector<uint8_t> material = PBKDF2::Derive(passphrase, salt, iterations, key_len + iv_len);
    std::vector<uint8_t> key(material.begin(), material.begin() + key_len);
    std::vector<uint8_t> iv(material.begin() + key_len, material.end());
    return std::make_shared<EncryptionKey>(key, iv);
#else
    throw std::runtime_error("PBKDF2 not supported on non-Linux platforms");
#endif
}

} // namespace network
} // namespace sqlcc
