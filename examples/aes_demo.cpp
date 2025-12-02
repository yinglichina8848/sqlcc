/**
 * @file aes_demo.cpp
 * @brief AESE加密通信演示程序
 * 
 * 演示如何使用AES-256-CBC加密进行数据库网络通信
 */

#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include "network/encryption.h"

using namespace sqlcc::network;

/**
 * 打印十六进制数据
 */
void PrintHexData(const std::vector<uint8_t>& data, const std::string& label, size_t max_len = 32) {
    std::cout << label << " (";
    if (data.size() > max_len) {
        std::cout << max_len << " of " << data.size() << " bytes): ";
    } else {
        std::cout << data.size() << " bytes): ";
    }
    
    for (size_t i = 0; i < std::min(data.size(), max_len); ++i) {
        printf("%02x ", data[i]);
    }
    if (data.size() > max_len) {
        std::cout << "... ";
    }
    std::cout << std::endl;
}

/**
 * 演示基本的AES加密
 */
void DemoBasicEncryption() {
    std::cout << "\n=====================================" << std::endl;
    std::cout << "1. 基本AES-256-CBC加密演示" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    if (!AESEncryptor::IsAvailable()) {
        std::cout << "✗ AES库不可用" << std::endl;
        return;
    }
    
    std::cout << "✓ AES-256-CBC库可用" << std::endl;
    
    // 生成密钥和IV
    std::cout << "\n[1] 生成AES-256密钥和IV..." << std::endl;
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16);
    std::cout << "✓ 密钥大小: " << encryption_key->GetKey().size() << " 字节" << std::endl;
    std::cout << "✓ IV大小: " << encryption_key->GetIV().size() << " 字节" << std::endl;
    
    PrintHexData(encryption_key->GetKey(), "  密钥");
    PrintHexData(encryption_key->GetIV(), "  初始向量(IV)");
    
    // 创建加密器
    std::cout << "\n[2] 创建AES加密器..." << std::endl;
    auto encryptor = std::make_shared<AESEncryptor>(encryption_key);
    std::cout << "✓ AES加密器创建成功" << std::endl;
    
    // 准备测试数据
    std::string plaintext = "SELECT * FROM users WHERE id = 1;";
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    
    std::cout << "\n[3] 加密SQL查询语句..." << std::endl;
    std::cout << "原始文本: \"" << plaintext << "\"" << std::endl;
    std::cout << "大小: " << data.size() << " 字节" << std::endl;
    
    // 加密
    auto encrypted = encryptor->Encrypt(data);
    std::cout << "✓ 加密完成" << std::endl;
    std::cout << "加密后大小: " << encrypted.size() << " 字节 (16的倍数: "
              << (encrypted.size() % 16 == 0 ? "是" : "否") << ")" << std::endl;
    PrintHexData(encrypted, "加密数据");
    
    // 解密
    std::cout << "\n[4] 解密数据..." << std::endl;
    auto decrypted = encryptor->Decrypt(encrypted);
    std::cout << "✓ 解密完成" << std::endl;
    std::cout << "解密大小: " << decrypted.size() << " 字节" << std::endl;
    
    std::string recovered(decrypted.begin(), decrypted.end());
    std::cout << "恢复文本: \"" << recovered << "\"" << std::endl;
    
    // 验证
    if (decrypted == data) {
        std::cout << "✓ 加密/解密验证: 通过" << std::endl;
    } else {
        std::cout << "✗ 加密/解密验证: 失败" << std::endl;
    }
}

/**
 * 演示SQL查询加密
 */
void DemoSQLQueryEncryption() {
    std::cout << "\n=====================================" << std::endl;
    std::cout << "2. SQL查询加密演示" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    if (!AESEncryptor::IsAvailable()) {
        std::cout << "✗ AES库不可用" << std::endl;
        return;
    }
    
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16);
    auto encryptor = std::make_shared<AESEncryptor>(encryption_key);
    
    std::vector<std::string> queries = {
        "CREATE TABLE users (id INT, name VARCHAR(255), email VARCHAR(255));",
        "INSERT INTO users VALUES (1, 'Alice', 'alice@example.com');",
        "UPDATE users SET email = 'newemail@example.com' WHERE id = 1;",
        "DELETE FROM users WHERE id = 1;",
        "SELECT * FROM users WHERE name LIKE 'A%';"
    };
    
    std::cout << "\n加密以下SQL查询:" << std::endl;
    for (size_t i = 0; i < queries.size(); ++i) {
        const auto& query = queries[i];
        std::cout << "\n[" << (i + 1) << "] " << query << std::endl;
        
        std::vector<uint8_t> data(query.begin(), query.end());
        auto encrypted = encryptor->Encrypt(data);
        
        std::cout << "  原始大小: " << data.size() << " 字节" << std::endl;
        std::cout << "  加密大小: " << encrypted.size() << " 字节" << std::endl;
        
        // 验证解密
        auto decrypted = encryptor->Decrypt(encrypted);
        std::string recovered(decrypted.begin(), decrypted.end());
        if (recovered == query) {
            std::cout << "  ✓ 加密/解密验证通过" << std::endl;
        } else {
            std::cout << "  ✗ 加密/解密验证失败" << std::endl;
        }
    }
}

/**
 * 演示密钥更新
 */
void DemoKeyUpdate() {
    std::cout << "\n=====================================" << std::endl;
    std::cout << "3. 密钥更新演示" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    if (!AESEncryptor::IsAvailable()) {
        std::cout << "✗ AES库不可用" << std::endl;
        return;
    }
    
    std::cout << "\n[1] 创建初始密钥和加密器..." << std::endl;
    auto key1 = EncryptionKey::GenerateRandom(32, 16);
    auto encryptor = std::make_shared<AESEncryptor>(key1);
    std::cout << "✓ 初始密钥生成完成" << std::endl;
    
    std::string message = "Confidential Database Record";
    std::vector<uint8_t> data(message.begin(), message.end());
    
    std::cout << "\n[2] 用初始密钥加密..." << std::endl;
    auto encrypted1 = encryptor->Encrypt(data);
    std::cout << "✓ 加密完成，大小: " << encrypted1.size() << " 字节" << std::endl;
    PrintHexData(encrypted1, "密钥1加密结果");
    
    std::cout << "\n[3] 生成新的密钥..." << std::endl;
    auto key2 = EncryptionKey::GenerateRandom(32, 16);
    std::cout << "✓ 新密钥生成完成" << std::endl;
    
    std::cout << "\n[4] 更新加密器为新密钥..." << std::endl;
    encryptor->UpdateKey(key2);
    std::cout << "✓ 密钥更新完成" << std::endl;
    
    std::cout << "\n[5] 用新密钥加密相同数据..." << std::endl;
    auto encrypted2 = encryptor->Encrypt(data);
    std::cout << "✓ 加密完成，大小: " << encrypted2.size() << " 字节" << std::endl;
    PrintHexData(encrypted2, "密钥2加密结果");
    
    if (encrypted1 != encrypted2) {
        std::cout << "\n✓ 验证: 不同密钥产生不同的加密结果" << std::endl;
    } else {
        std::cout << "\n✗ 验证: 加密结果应该不同" << std::endl;
    }
    
    std::cout << "\n[6] 验证新密钥可以解密用新密钥加密的数据..." << std::endl;
    auto decrypted = encryptor->Decrypt(encrypted2);
    if (decrypted == data) {
        std::cout << "✓ 解密验证通过" << std::endl;
    } else {
        std::cout << "✗ 解密验证失败" << std::endl;
    }
}

/**
 * 演示简单的XOR加密（用于对比）
 */
void DemoSimpleEncryption() {
    std::cout << "\n=====================================" << std::endl;
    std::cout << "4. 简单XOR加密演示（对比）" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    std::cout << "\n[1] 创建XOR加密器..." << std::endl;
    SimpleEncryptor xor_encryptor("simple_key");
    std::cout << "✓ XOR加密器创建成功" << std::endl;
    
    std::string plaintext = "Simple encryption";
    std::vector<char> data(plaintext.begin(), plaintext.end());
    
    std::cout << "\n[2] 用XOR加密..." << std::endl;
    std::cout << "原始文本: \"" << plaintext << "\"" << std::endl;
    auto encrypted = xor_encryptor.Encrypt(data);
    std::cout << "✓ XOR加密完成" << std::endl;
    
    std::cout << "\n[3] 用XOR解密..." << std::endl;
    auto decrypted = xor_encryptor.Decrypt(encrypted);
    std::string recovered(decrypted.begin(), decrypted.end());
    std::cout << "恢复文本: \"" << recovered << "\"" << std::endl;
    
    if (decrypted == data) {
        std::cout << "✓ XOR加密/解密验证通过" << std::endl;
    } else {
        std::cout << "✗ XOR加密/解密验证失败" << std::endl;
    }
    
    std::cout << "\n注意: XOR加密强度远低于AES-256，仅用于演示目的" << std::endl;
}

/**
 * 主程序
 */
int main() {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "SQLCC 数据库 - AESE加密通信演示程序" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    std::cout << "\nASES (Advanced Encryption Standard Extended)" << std::endl;
    std::cout << "本演示展示了数据库网络通信中的加密功能" << std::endl;
    
    try {
        // 运行各个演示
        DemoBasicEncryption();
        DemoSQLQueryEncryption();
        DemoKeyUpdate();
        DemoSimpleEncryption();
        
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "演示完成!" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "\n✗ 错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
