/**
 * @file aes_encryption_test.cc
 * @brief AES-256-CBC加密功能测试
 * 
 * 测试AESE(Advanced Encryption Standard Extended)加密通信功能
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include "network/encryption.h"
#include "network/network.h"

using namespace sqlcc::network;

/**
 * @class EncryptionKeyTest
 * @brief 加密密钥容器的测试用例
 */
class EncryptionKeyTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试前的设置
    }
    
    void TearDown() override {
        // 测试后的清理
    }
};

/**
 * @class AESEncryptorTest
 * @brief AES加密器的测试用例
 */
class AESEncryptorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 生成密钥和IV
        encryption_key_ = EncryptionKey::GenerateRandom(32, 16);
        ASSERT_TRUE(encryption_key_);
        EXPECT_EQ(encryption_key_->GetKey().size(), 32);
        EXPECT_EQ(encryption_key_->GetIV().size(), 16);
    }
    
    void TearDown() override {
        // 清理资源
    }
    
    std::shared_ptr<EncryptionKey> encryption_key_;
};

/**
 * @test EncryptionKeyGeneration
 * @brief 测试加密密钥的生成
 */
TEST_F(EncryptionKeyTest, GenerateRandomKey) {
    // 检查AES库是否可用
    if (!AESEncryptor::IsAvailable()) {
        GTEST_SKIP() << "AES encryption not available on this platform";
    }
    
    // 生成随机密钥和IV
    auto key1 = EncryptionKey::GenerateRandom(32, 16);
    auto key2 = EncryptionKey::GenerateRandom(32, 16);
    
    // 验证密钥大小
    EXPECT_EQ(key1->GetKey().size(), 32);
    EXPECT_EQ(key1->GetIV().size(), 16);
    
    // 验证生成的密钥是随机的（不应该相同）
    EXPECT_NE(key1->GetKey(), key2->GetKey());
}

/**
 * @test AES256CBCEncryption
 * @brief 测试AES-256-CBC加密功能
 */
TEST_F(AESEncryptorTest, BasicEncryption) {
    // 检查AES库是否可用
    if (!AESEncryptor::IsAvailable()) {
        GTEST_SKIP() << "AES encryption not available on this platform";
    }
    
    // 创建AES加密器
    auto encryptor = std::make_shared<AESEncryptor>(encryption_key_);
    ASSERT_TRUE(encryptor);
    
    // 准备测试数据
    std::string plaintext = "Hello, SQLCC Database!";
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    
    // 加密数据
    auto encrypted = encryptor->Encrypt(data);
    EXPECT_FALSE(encrypted.empty());
    EXPECT_NE(encrypted, data);
    
    // 验证加密的数据大小（应该是16的倍数）
    EXPECT_EQ(encrypted.size() % 16, 0);
}

/**
 * @test AES256CBCDecryption
 * @brief 测试AES-256-CBC解密功能
 */
TEST_F(AESEncryptorTest, EncryptionDecryption) {
    // 检查AES库是否可用
    if (!AESEncryptor::IsAvailable()) {
        GTEST_SKIP() << "AES encryption not available on this platform";
    }
    
    // 创建AES加密器
    auto encryptor = std::make_shared<AESEncryptor>(encryption_key_);
    ASSERT_TRUE(encryptor);
    
    // 准备测试数据
    std::string plaintext = "Database Security Test Data";
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    
    // 加密数据
    auto encrypted = encryptor->Encrypt(data);
    EXPECT_FALSE(encrypted.empty());
    
    // 解密数据
    auto decrypted = encryptor->Decrypt(encrypted);
    
    // 验证解密结果
    EXPECT_EQ(decrypted.size(), data.size());
    EXPECT_EQ(decrypted, data);
    
    // 验证恢复的文本
    std::string recovered(decrypted.begin(), decrypted.end());
    EXPECT_EQ(recovered, plaintext);
}

/**
 * @test MultipleEncryptions
 * @brief 测试多次加密/解密
 */
TEST_F(AESEncryptorTest, MultipleEncryptionsDecryptions) {
    // 检查AES库是否可用
    if (!AESEncryptor::IsAvailable()) {
        GTEST_SKIP() << "AES encryption not available on this platform";
    }
    
    auto encryptor = std::make_shared<AESEncryptor>(encryption_key_);
    
    // 测试多个不同的消息
    std::vector<std::string> messages = {
        "Short",
        "This is a longer message for testing.",
        "SELECT * FROM users WHERE id = 1;",
        "CREATE TABLE test (id INT, name VARCHAR(255));"
    };
    
    for (const auto& msg : messages) {
        std::vector<uint8_t> data(msg.begin(), msg.end());
        
        // 加密
        auto encrypted = encryptor->Encrypt(data);
        EXPECT_FALSE(encrypted.empty());
        
        // 解密
        auto decrypted = encryptor->Decrypt(encrypted);
        
        // 验证
        EXPECT_EQ(decrypted, data);
        std::string recovered(decrypted.begin(), decrypted.end());
        EXPECT_EQ(recovered, msg);
    }
}

/**
 * @test LargeDataEncryption
 * @brief 测试大数据加密
 */
TEST_F(AESEncryptorTest, LargeDataEncryption) {
    // 检查AES库是否可用
    if (!AESEncryptor::IsAvailable()) {
        GTEST_SKIP() << "AES encryption not available on this platform";
    }
    
    auto encryptor = std::make_shared<AESEncryptor>(encryption_key_);
    
    // 生成大数据块（100KB）
    std::vector<uint8_t> large_data(100 * 1024);
    for (size_t i = 0; i < large_data.size(); ++i) {
        large_data[i] = static_cast<uint8_t>(i % 256);
    }
    
    // 加密
    auto encrypted = encryptor->Encrypt(large_data);
    EXPECT_FALSE(encrypted.empty());
    EXPECT_EQ(encrypted.size() % 16, 0);
    
    // 解密
    auto decrypted = encryptor->Decrypt(encrypted);
    
    // 验证
    EXPECT_EQ(decrypted, large_data);
}

/**
 * @test KeyUpdate
 * @brief 测试密钥更新
 */
TEST_F(AESEncryptorTest, UpdateKey) {
    // 检查AES库是否可用
    if (!AESEncryptor::IsAvailable()) {
        GTEST_SKIP() << "AES encryption not available on this platform";
    }
    
    auto encryptor = std::make_shared<AESEncryptor>(encryption_key_);
    
    // 用第一个密钥加密
    std::string plaintext = "Test message";
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    auto encrypted1 = encryptor->Encrypt(data);
    
    // 生成新的密钥
    auto new_key = EncryptionKey::GenerateRandom(32, 16);
    encryptor->UpdateKey(new_key);
    
    // 用新密钥加密同样的数据
    auto encrypted2 = encryptor->Encrypt(data);
    
    // 验证两次加密结果不同（使用了不同的密钥和IV）
    EXPECT_NE(encrypted1, encrypted2);
    
    // 验证用新密钥可以解密用新密钥加密的数据
    auto decrypted = encryptor->Decrypt(encrypted2);
    EXPECT_EQ(decrypted, data);
}

/**
 * @test SimplXOREncryptor
 * @brief 测试简单的XOR加密器
 */
TEST(SimpleEncryptorTest, XOREncryption) {
    SimpleEncryptor encryptor("test_key");
    
    // 准备测试数据
    std::string plaintext = "Hello, World!";
    std::vector<char> data(plaintext.begin(), plaintext.end());
    
    // 加密
    auto encrypted = encryptor.Encrypt(data);
    EXPECT_FALSE(encrypted.empty());
    EXPECT_NE(encrypted, data);
    
    // 解密（XOR是自逆的）
    auto decrypted = encryptor.Decrypt(encrypted);
    EXPECT_EQ(decrypted, data);
}

/**
 * @test AESAvailability
 * @brief 检查AES库是否可用
 */
TEST(AESAvailabilityTest, CheckAESLibrary) {
    bool available = AESEncryptor::IsAvailable();
    std::cout << "AES Encryption Available: " << (available ? "Yes" : "No") << std::endl;
    
    // 在Linux系统上应该可用
#ifdef __linux__
    EXPECT_TRUE(available);
#endif
}

// 新增：HMAC-SHA256 与 PBKDF2 相关测试
TEST(HMACTest, ComputeAndVerify) {
#ifdef __linux__
    std::string msg = "Integrity protected message";
    std::vector<uint8_t> data(msg.begin(), msg.end());
    auto key = EncryptionKey::GenerateRandom(32, 16);
    auto mac = HMACSHA256::Compute(key->GetKey(), data);
    EXPECT_EQ(mac.size(), 32);
    EXPECT_TRUE(HMACSHA256::Verify(key->GetKey(), data, mac));
    data[0] ^= 0xFF;
    EXPECT_FALSE(HMACSHA256::Verify(key->GetKey(), data, mac));
#else
    GTEST_SKIP() << "HMAC not available on this platform";
#endif
}

TEST(HMACTest, TamperDetectionWithAES) {
#ifdef __linux__
    auto key = EncryptionKey::GenerateRandom(32, 16);
    AESEncryptor aes(key);
    std::string msg = "Sensitive payload";
    std::vector<uint8_t> data(msg.begin(), msg.end());
    auto ct = aes.Encrypt(data);
    auto mac = HMACSHA256::Compute(key->GetKey(), ct);
    std::vector<uint8_t> packet = ct;
    packet.insert(packet.end(), mac.begin(), mac.end());
    packet[0] ^= 0x1;
    std::vector<uint8_t> tampered_ct(packet.begin(), packet.end() - 32);
    std::vector<uint8_t> recv_mac(packet.end() - 32, packet.end());
    EXPECT_FALSE(HMACSHA256::Verify(key->GetKey(), tampered_ct, recv_mac));
#else
    GTEST_SKIP() << "HMAC not available on this platform";
#endif
}

TEST(PBKDF2Test, DeriveBasic) {
#ifdef __linux__
    std::string passphrase = "password";
    std::vector<uint8_t> salt = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
    int iterations = 10000;
    auto out1 = PBKDF2::Derive(passphrase, salt, iterations, 32);
    EXPECT_EQ(out1.size(), 32);
    std::vector<uint8_t> salt2 = {0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10};
    auto out2 = PBKDF2::Derive(passphrase, salt2, iterations, 32);
    EXPECT_NE(out1, out2);
#else
    GTEST_SKIP() << "PBKDF2 not available on this platform";
#endif
}

TEST(PBKDF2Test, DeriveEncryptionKeyAndUseAES) {
#ifdef __linux__
    std::string passphrase = "S3cureP@ss";
    std::vector<uint8_t> salt(16, 0x11);
    int iterations = 20000;
    auto enc_key = DeriveEncryptionKeyFromPassword(passphrase, salt, iterations, 32, 16);
    ASSERT_TRUE(enc_key);
    EXPECT_EQ(enc_key->GetKey().size(), 32);
    EXPECT_EQ(enc_key->GetIV().size(), 16);
    AESEncryptor aes(enc_key);
    std::string plaintext = "PBKDF2 derived AES key works";
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    auto ct = aes.Encrypt(data);
    auto pt = aes.Decrypt(ct);
    EXPECT_EQ(pt, data);
#else
    GTEST_SKIP() << "PBKDF2 not available on this platform";
#endif
}

TEST(HMACTest, TruncatedMACFails) {
#ifdef __linux__
    auto key = EncryptionKey::GenerateRandom(32, 16);
    std::string msg = "Truncated MAC should fail";
    std::vector<uint8_t> data(msg.begin(), msg.end());
    auto mac = HMACSHA256::Compute(key->GetKey(), data);
    ASSERT_EQ(mac.size(), 32);
    // 截断到16字节
    std::vector<uint8_t> mac16(mac.begin(), mac.begin() + 16);
    EXPECT_FALSE(HMACSHA256::Verify(key->GetKey(), data, mac16));
#else
    GTEST_SKIP() << "HMAC not available on this platform";
#endif
}

TEST(PBKDF2Test, IterationsImpact) {
#ifdef __linux__
    std::string passphrase = "password";
    std::vector<uint8_t> salt = {0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28};
    auto k1 = PBKDF2::Derive(passphrase, salt, 1000, 32);
    auto k2 = PBKDF2::Derive(passphrase, salt, 20000, 32);
    EXPECT_EQ(k1.size(), 32);
    EXPECT_EQ(k2.size(), 32);
    EXPECT_NE(k1, k2);
#else
    GTEST_SKIP() << "PBKDF2 not available on this platform";
#endif
}

TEST(HMACTest, AppendedMacLength) {
#ifdef __linux__
    auto key = EncryptionKey::GenerateRandom(32, 16);
    AESEncryptor aes(key);
    std::string msg = "MAC length should be 32";
    std::vector<uint8_t> data(msg.begin(), msg.end());
    auto ct = aes.Encrypt(data);
    auto mac = HMACSHA256::Compute(key->GetKey(), ct);
    ASSERT_EQ(mac.size(), 32);
    EXPECT_EQ(ct.size() + mac.size(), ct.size() + 32);
#else
    GTEST_SKIP() << "HMAC not available on this platform";
#endif
}

TEST(PBKDF2Test, DeterministicWithSameParams) {
#ifdef __linux__
    std::string passphrase = "password";
    std::vector<uint8_t> salt = {0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38};
    int iterations = 15000;
    auto k1 = PBKDF2::Derive(passphrase, salt, iterations, 32);
    auto k2 = PBKDF2::Derive(passphrase, salt, iterations, 32);
    EXPECT_EQ(k1, k2);
#else
    GTEST_SKIP() << "PBKDF2 not available on this platform";
#endif
}

TEST_F(AESEncryptorTest, DifferentIVDifferentCiphertext) {
#ifdef __linux__
    // 相同密钥，不同IV，加密结果不同
    auto base = EncryptionKey::GenerateRandom(32, 16);
    auto iv2 = EncryptionKey::GenerateRandom(32, 16)->GetIV();
    auto key1 = base;
    auto key2 = std::make_shared<EncryptionKey>(base->GetKey(), iv2);
    AESEncryptor aes1(key1);
    AESEncryptor aes2(key2);
    std::string plaintext = "IV impact test";
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    auto c1 = aes1.Encrypt(data);
    auto c2 = aes2.Encrypt(data);
    EXPECT_NE(c1, c2);
#else
    GTEST_SKIP() << "AES not available on this platform";
#endif
}

TEST(KeyRotationPolicyTest, SimpleInterval) {
    sqlcc::network::KeyRotationPolicy policy(5);
    EXPECT_FALSE(policy.ShouldRotate(1));
    EXPECT_TRUE(policy.ShouldRotate(5));
    EXPECT_TRUE(policy.ShouldRotate(10));
}

TEST_F(AESEncryptorTest, DecryptWithWrongKeyThrows) {
#ifdef __linux__
    auto key1 = EncryptionKey::GenerateRandom(32, 16);
    auto key2 = EncryptionKey::GenerateRandom(32, 16);
    AESEncryptor aes1(key1);
    AESEncryptor aes2(key2);
    std::string plaintext = "Wrong key should fail";
    std::vector<uint8_t> data(plaintext.begin(), plaintext.end());
    auto ct = aes1.Encrypt(data);
    EXPECT_THROW(aes2.Decrypt(ct), std::runtime_error);
#else
    GTEST_SKIP() << "AES not available on this platform";
#endif
}

TEST_F(AESEncryptorTest, UpdateKeyNullThrows) {
#ifdef __linux__
    auto key = EncryptionKey::GenerateRandom(32, 16);
    auto encryptor = std::make_shared<AESEncryptor>(key);
    EXPECT_THROW(encryptor->UpdateKey(nullptr), std::invalid_argument);
#else
    GTEST_SKIP() << "AES not available on this platform";
#endif
}

TEST_F(EncryptionKeyTest, CustomKeyIvSizes) {
    auto key = EncryptionKey::GenerateRandom(16, 12);
    EXPECT_EQ(key->GetKey().size(), 16);
    EXPECT_EQ(key->GetIV().size(), 12);
}

TEST(HMACTest, VerifyWithWrongKeyFails) {
#ifdef __linux__
    auto key1 = EncryptionKey::GenerateRandom(32, 16);
    auto key2 = EncryptionKey::GenerateRandom(32, 16);
    std::string msg = "Wrong key MAC should fail";
    std::vector<uint8_t> data(msg.begin(), msg.end());
    auto mac = HMACSHA256::Compute(key1->GetKey(), data);
    EXPECT_FALSE(HMACSHA256::Verify(key2->GetKey(), data, mac));
#else
    GTEST_SKIP() << "HMAC not available on this platform";
#endif
}

TEST(PBKDF2Test, InvalidIterationsThrows) {
#ifdef __linux__
    std::string passphrase = "password";
    std::vector<uint8_t> salt = {0x01,0x02,0x03,0x04};
    EXPECT_THROW(PBKDF2::Derive(passphrase, salt, 0, 32), std::runtime_error);
#else
    GTEST_SKIP() << "PBKDF2 not available on this platform";
#endif
}

TEST(PBKDF2Test, VariableOutputLength) {
#ifdef __linux__
    std::string passphrase = "password";
    std::vector<uint8_t> salt = {0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18};
    auto k64 = PBKDF2::Derive(passphrase, salt, 5000, 64);
    EXPECT_EQ(k64.size(), 64);
#else
    GTEST_SKIP() << "PBKDF2 not available on this platform";
#endif
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
