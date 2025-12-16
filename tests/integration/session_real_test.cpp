/**
 * @file session_real_test.cpp
 * @brief Session类的真实单元测试
 * 
 * 测试真实的Session类（来自include/network/network.h），而不是mock版本
 * 测试认证状态管理、加密控制、AES加密器集成和边界条件
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

// 包含真实的网络模块头文件
#include "network/network.h"
#include "network/encryption.h"

using namespace sqlcc::network;

// 测试夹具
class SessionRealTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建真实的Session实例
        session_ = std::make_shared<Session>(123);
    }

    void TearDown() override {
        session_.reset();
    }

    std::shared_ptr<Session> session_;
};

// 测试Session基本构造和初始化
TEST_F(SessionRealTest, SessionCreation) {
    // 验证会话ID
    EXPECT_EQ(session_->GetSessionId(), 123);

    // 初始状态应该是未认证
    EXPECT_FALSE(session_->IsAuthenticated());
    EXPECT_EQ(session_->GetUser(), "");

    // 初始状态不应该禁用加密和认证
    EXPECT_FALSE(session_->IsEncryptionDisabled());
    EXPECT_FALSE(session_->IsAuthenticationDisabled());

    // 初始状态AES加密器为空
    EXPECT_EQ(session_->GetAESEncryptor(), nullptr);
    EXPECT_FALSE(session_->IsAESEncryptionEnabled());
}

// 测试认证状态管理
TEST_F(SessionRealTest, AuthenticationManagement) {
    // 初始未认证
    EXPECT_FALSE(session_->IsAuthenticated());
    EXPECT_EQ(session_->GetUser(), "");

    // 设置认证
    session_->SetAuthenticated("test_user");
    EXPECT_TRUE(session_->IsAuthenticated());
    EXPECT_EQ(session_->GetUser(), "test_user");

    // 测试不同用户名
    session_->SetAuthenticated("another_user");
    EXPECT_TRUE(session_->IsAuthenticated());
    EXPECT_EQ(session_->GetUser(), "another_user");
}

// 测试加密控制
TEST_F(SessionRealTest, EncryptionControl) {
    // 初始不禁用加密
    EXPECT_FALSE(session_->IsEncryptionDisabled());

    // 禁用加密
    session_->SetEncryptionDisabled(true);
    EXPECT_TRUE(session_->IsEncryptionDisabled());

    // 重新启用加密
    session_->SetEncryptionDisabled(false);
    EXPECT_FALSE(session_->IsEncryptionDisabled());
}

// 测试认证控制
TEST_F(SessionRealTest, AuthenticationControl) {
    // 初始不禁用认证
    EXPECT_FALSE(session_->IsAuthenticationDisabled());

    // 禁用认证
    session_->SetAuthenticationDisabled(true);
    EXPECT_TRUE(session_->IsAuthenticationDisabled());

    // 重新启用认证
    session_->SetAuthenticationDisabled(false);
    EXPECT_FALSE(session_->IsAuthenticationDisabled());
}

// 测试AES加密器集成
TEST_F(SessionRealTest, AESEncryptorIntegration) {
    // 初始状态AES加密器为空
    EXPECT_EQ(session_->GetAESEncryptor(), nullptr);
    EXPECT_FALSE(session_->IsAESEncryptionEnabled());

    // 创建真实的AES加密器
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16); // AES-256密钥
    auto aes_encryptor = std::make_shared<AESEncryptor>(encryption_key);

    // 设置AES加密器
    session_->SetAESEncryptor(aes_encryptor);
    EXPECT_NE(session_->GetAESEncryptor(), nullptr);
    EXPECT_TRUE(session_->IsAESEncryptionEnabled());

    // 验证返回的是同一个实例
    EXPECT_EQ(session_->GetAESEncryptor(), aes_encryptor);
}

// 测试AES加密器与加密禁用标志的交互
TEST_F(SessionRealTest, AESEncryptorWithEncryptionDisabled) {
    // 创建真实的AES加密器并设置
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16);
    auto aes_encryptor = std::make_shared<AESEncryptor>(encryption_key);
    session_->SetAESEncryptor(aes_encryptor);

    // 虽然有AES加密器，但如果禁用加密，IsAESEncryptionEnabled应该返回false
    session_->SetEncryptionDisabled(true);
    EXPECT_TRUE(session_->IsEncryptionDisabled());
    EXPECT_FALSE(session_->IsAESEncryptionEnabled());  // 加密被禁用

    // 重新启用加密
    session_->SetEncryptionDisabled(false);
    EXPECT_TRUE(session_->IsAESEncryptionEnabled());
}

// 测试AES加密功能的实际效果
TEST_F(SessionRealTest, AESEncryptionFunctionality) {
    // 创建AES加密器
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16);
    auto aes_encryptor = std::make_shared<AESEncryptor>(encryption_key);
    session_->SetAESEncryptor(aes_encryptor);

    // 验证加密器功能
    EXPECT_TRUE(session_->IsAESEncryptionEnabled());

    // 测试加密/解密功能
    std::vector<uint8_t> original_data = {'t', 'e', 's', 't', ' ', 'd', 'a', 't', 'a'};
    
    // 加密数据
    std::vector<uint8_t> encrypted_data = aes_encryptor->Encrypt(original_data);
    
    // 加密后的数据应该与原始数据不同
    EXPECT_NE(original_data, encrypted_data);
    
    // 解密数据
    std::vector<uint8_t> decrypted_data = aes_encryptor->Decrypt(encrypted_data);
    
    // 解密后的数据应该与原始数据相同
    EXPECT_EQ(original_data, decrypted_data);
}

// 测试边界条件：空用户名认证
TEST_F(SessionRealTest, EmptyUsernameAuthentication) {
    session_->SetAuthenticated("");
    EXPECT_TRUE(session_->IsAuthenticated());
    EXPECT_EQ(session_->GetUser(), "");
}

// 测试边界条件：特殊字符用户名
TEST_F(SessionRealTest, SpecialCharactersUsername) {
    std::string special_user = "user@domain.com_123!#$%^&*()";
    session_->SetAuthenticated(special_user);
    EXPECT_TRUE(session_->IsAuthenticated());
    EXPECT_EQ(session_->GetUser(), special_user);
}

// 测试边界条件：长用户名
TEST_F(SessionRealTest, LongUsername) {
    std::string long_user(1000, 'a');  // 1000个'a'字符
    session_->SetAuthenticated(long_user);
    EXPECT_TRUE(session_->IsAuthenticated());
    EXPECT_EQ(session_->GetUser(), long_user);
}

// 测试边界条件：Unicode用户名

// 测试多个状态变化的组合
TEST_F(SessionRealTest, StateTransitionCombinations) {
    // 初始状态
    EXPECT_FALSE(session_->IsAuthenticated());
    EXPECT_FALSE(session_->IsEncryptionDisabled());
    EXPECT_FALSE(session_->IsAuthenticationDisabled());
    EXPECT_FALSE(session_->IsAESEncryptionEnabled());

    // 设置认证
    session_->SetAuthenticated("user");
    EXPECT_TRUE(session_->IsAuthenticated());

    // 禁用加密
    session_->SetEncryptionDisabled(true);
    EXPECT_TRUE(session_->IsEncryptionDisabled());

    // 设置AES加密器（但加密被禁用）
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16);
    auto aes_encryptor = std::make_shared<AESEncryptor>(encryption_key);
    session_->SetAESEncryptor(aes_encryptor);
    EXPECT_FALSE(session_->IsAESEncryptionEnabled());  // 因为加密被禁用

    // 重新启用加密
    session_->SetEncryptionDisabled(false);
    EXPECT_TRUE(session_->IsAESEncryptionEnabled());

    // 禁用认证
    session_->SetAuthenticationDisabled(true);
    EXPECT_TRUE(session_->IsAuthenticationDisabled());
}

// 测试Session的独立性（多个实例）
TEST(SessionRealIndependenceTest, MultipleSessions) {
    auto session1 = std::make_shared<Session>(1);
    auto session2 = std::make_shared<Session>(2);
    auto session3 = std::make_shared<Session>(999999);

    // 设置不同的状态
    session1->SetAuthenticated("user1");
    session1->SetEncryptionDisabled(true);

    session2->SetAuthenticated("user2");
    session2->SetAuthenticationDisabled(true);

    // session3保持默认状态

    // 验证状态不互相影响
    EXPECT_TRUE(session1->IsAuthenticated());
    EXPECT_EQ(session1->GetUser(), "user1");
    EXPECT_TRUE(session1->IsEncryptionDisabled());
    EXPECT_FALSE(session1->IsAuthenticationDisabled());

    EXPECT_TRUE(session2->IsAuthenticated());
    EXPECT_EQ(session2->GetUser(), "user2");
    EXPECT_FALSE(session2->IsEncryptionDisabled());
    EXPECT_TRUE(session2->IsAuthenticationDisabled());

    EXPECT_FALSE(session3->IsAuthenticated());
    EXPECT_FALSE(session3->IsEncryptionDisabled());
    EXPECT_FALSE(session3->IsAuthenticationDisabled());
}

// 测试AES加密器更新
TEST_F(SessionRealTest, AESEncryptorUpdate) {
    // 创建第一个AES加密器
    auto key1 = EncryptionKey::GenerateRandom(32, 16);
    auto aes1 = std::make_shared<AESEncryptor>(key1);
    session_->SetAESEncryptor(aes1);
    EXPECT_EQ(session_->GetAESEncryptor(), aes1);

    // 创建第二个AES加密器
    auto key2 = EncryptionKey::GenerateRandom(32, 16);
    auto aes2 = std::make_shared<AESEncryptor>(key2);
    session_->SetAESEncryptor(aes2);
    EXPECT_EQ(session_->GetAESEncryptor(), aes2);
    EXPECT_NE(session_->GetAESEncryptor(), aes1);

    // 验证不同的加密器使用不同的密钥
    EXPECT_NE(aes1->GetKeyBytes(), aes2->GetKeyBytes());
}

// 测试大数据加密
TEST_F(SessionRealTest, LargeDataEncryption) {
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16);
    auto aes_encryptor = std::make_shared<AESEncryptor>(encryption_key);
    session_->SetAESEncryptor(aes_encryptor);

    // 创建大数据块（1MB）
    const size_t data_size = 1024 * 1024;
    std::vector<uint8_t> large_data(data_size, 0xAA);

    // 加密大数据
    std::vector<uint8_t> encrypted_data = aes_encryptor->Encrypt(large_data);
    EXPECT_NE(large_data, encrypted_data);
    EXPECT_EQ(encrypted_data.size(), data_size);

    // 解密大数据
    std::vector<uint8_t> decrypted_data = aes_encryptor->Decrypt(encrypted_data);
    EXPECT_EQ(large_data, decrypted_data);
}

// 测试空数据加密
TEST_F(SessionRealTest, EmptyDataEncryption) {
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16);
    auto aes_encryptor = std::make_shared<AESEncryptor>(encryption_key);
    session_->SetAESEncryptor(aes_encryptor);

    // 加密空数据
    std::vector<uint8_t> empty_data;
    std::vector<uint8_t> encrypted_empty = aes_encryptor->Encrypt(empty_data);
    std::vector<uint8_t> decrypted_empty = aes_encryptor->Decrypt(encrypted_empty);
    EXPECT_EQ(empty_data, decrypted_empty);
}

// 测试加密器密钥字节访问
TEST_F(SessionRealTest, EncryptorKeyBytesAccess) {
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16);
    auto aes_encryptor = std::make_shared<AESEncryptor>(encryption_key);
    session_->SetAESEncryptor(aes_encryptor);

    // 验证密钥字节访问
    auto key_bytes = aes_encryptor->GetKeyBytes();
    EXPECT_EQ(key_bytes.size(), 32U);  // AES-256密钥长度
    EXPECT_EQ(key_bytes, encryption_key->GetKey());
}

// 测试并发访问安全性（基本测试）
TEST(SessionRealConcurrencyTest, BasicConcurrency) {
    const int num_sessions = 100;
    std::vector<std::shared_ptr<Session>> sessions;
    std::vector<std::thread> threads;

    // 创建多个会话并设置不同的状态
    for (int i = 0; i < num_sessions; ++i) {
        sessions.push_back(std::make_shared<Session>(i + 1));
    }

    // 并发设置认证状态
    for (int i = 0; i < num_sessions; ++i) {
        threads.emplace_back([&sessions, i]() {
            auto& session = sessions[i];
            session->SetAuthenticated("user" + std::to_string(i));
            session->SetEncryptionDisabled(i % 2 == 0);
        });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 验证所有会话状态
    for (int i = 0; i < num_sessions; ++i) {
        EXPECT_TRUE(sessions[i]->IsAuthenticated());
        EXPECT_EQ(sessions[i]->GetUser(), "user" + std::to_string(i));
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
