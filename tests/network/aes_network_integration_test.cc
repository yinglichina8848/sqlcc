/**
 * @file aes_network_integration_test.cc
 * @brief AES加密网络集成测试
 * 
 * 演示客户端-服务器通过AES-256-CBC加密进行通信
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <iostream>
#include "network/encryption.h"
#include "network/network.h"

using namespace sqlcc::network;

/**
 * @class AESNetworkIntegrationTest
 * @brief AES网络集成测试套件
 */
class AESNetworkIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 检查AES是否可用
        if (!AESEncryptor::IsAvailable()) {
            GTEST_SKIP() << "AES encryption not available on this platform";
        }
    }
    
    void TearDown() override {
        // 清理资源
    }
};

/**
 * @test EncryptionKeyExchange
 * @brief 测试加密密钥交换过程
 */
TEST_F(AESNetworkIntegrationTest, EncryptionKeyExchange) {
    // 服务器生成密钥
    auto server_key = EncryptionKey::GenerateRandom(32, 16);
    ASSERT_TRUE(server_key);
    
    // 客户端接收服务器的IV
    std::vector<uint8_t> received_iv = server_key->GetIV();
    
    // 客户端生成自己的密钥但使用服务器的IV
    auto client_key = EncryptionKey::GenerateRandom(32, 16);
    auto client_key_with_server_iv = std::make_shared<EncryptionKey>(
        client_key->GetKey(),
        received_iv
    );
    
    // 验证密钥已正确设置
    EXPECT_EQ(client_key_with_server_iv->GetIV(), received_iv);
    EXPECT_EQ(client_key_with_server_iv->GetKey().size(), 32);
}

/**
 * @test SessionAESEncryption
 * @brief 测试Session中的AES加密
 */
TEST_F(AESNetworkIntegrationTest, SessionAESEncryption) {
    // 创建session
    auto session = std::make_shared<Session>(1);
    ASSERT_TRUE(session);
    
    // 初始化时没有AES加密器
    EXPECT_FALSE(session->IsAESEncryptionEnabled());
    
    // 生成密钥并设置AES加密器
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16);
    auto aes_encryptor = std::make_shared<AESEncryptor>(encryption_key);
    session->SetAESEncryptor(aes_encryptor);
    
    // 验证AES加密已启用
    EXPECT_TRUE(session->IsAESEncryptionEnabled());
    EXPECT_TRUE(session->GetAESEncryptor());
}

/**
 * @test MessageEncryptionDecryption
 * @brief 测试消息的加密和解密
 */
TEST_F(AESNetworkIntegrationTest, MessageEncryptionDecryption) {
    // 生成密钥
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16);
    auto encryptor = std::make_shared<AESEncryptor>(encryption_key);
    
    // 模拟消息头和消息体
    struct MessageHeader {
        uint32_t magic;
        uint32_t length;
        uint16_t type;
        uint16_t flags;
        uint32_t sequence_id;
    };
    
    std::string query = "SELECT * FROM users WHERE id = 1;";
    std::vector<uint8_t> original_data(query.begin(), query.end());
    
    // 加密消息体
    auto encrypted_data = encryptor->Encrypt(original_data);
    EXPECT_FALSE(encrypted_data.empty());
    EXPECT_NE(encrypted_data, original_data);
    
    // 解密消息体
    auto decrypted_data = encryptor->Decrypt(encrypted_data);
    EXPECT_EQ(decrypted_data, original_data);
    
    // 验证解密的消息内容
    std::string decrypted_query(decrypted_data.begin(), decrypted_data.end());
    EXPECT_EQ(decrypted_query, query);
}

/**
 * @test SQLQueryEncryption
 * @brief 测试SQL查询的加密传输
 */
TEST_F(AESNetworkIntegrationTest, SQLQueryEncryption) {
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16);
    auto encryptor = std::make_shared<AESEncryptor>(encryption_key);
    
    // 模拟各种SQL查询
    std::vector<std::string> sql_queries = {
        "SELECT * FROM users;",
        "INSERT INTO users (id, name) VALUES (1, 'Alice');",
        "UPDATE users SET name = 'Bob' WHERE id = 1;",
        "DELETE FROM users WHERE id = 2;",
        "CREATE TABLE products (id INT, name VARCHAR(255), price DECIMAL(10,2));",
        "DROP TABLE IF EXISTS temp_table;",
        "ALTER TABLE users ADD COLUMN email VARCHAR(255);"
    };
    
    for (const auto& query : sql_queries) {
        std::vector<uint8_t> data(query.begin(), query.end());
        
        // 加密
        auto encrypted = encryptor->Encrypt(data);
        EXPECT_FALSE(encrypted.empty());
        
        // 解密
        auto decrypted = encryptor->Decrypt(encrypted);
        
        // 验证
        EXPECT_EQ(decrypted, data);
        std::string recovered(decrypted.begin(), decrypted.end());
        EXPECT_EQ(recovered, query);
        
        std::cout << "✓ Query encrypted/decrypted: " << query << std::endl;
    }
}

/**
 * @test ConcurrentEncryption
 * @brief 测试并发加密
 */
TEST_F(AESNetworkIntegrationTest, ConcurrentEncryption) {
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16);
    auto encryptor = std::make_shared<AESEncryptor>(encryption_key);
    
    const int num_threads = 4;
    const int iterations_per_thread = 10;
    
    std::vector<std::thread> threads;
    std::vector<bool> results(num_threads * iterations_per_thread, false);
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < iterations_per_thread; ++i) {
                std::string message = "Thread " + std::to_string(t) + " Message " + std::to_string(i);
                std::vector<uint8_t> data(message.begin(), message.end());
                
                // 加密
                auto encrypted = encryptor->Encrypt(data);
                
                // 解密
                auto decrypted = encryptor->Decrypt(encrypted);
                
                // 验证
                bool success = (decrypted == data);
                results[t * iterations_per_thread + i] = success;
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证所有操作都成功
    for (bool result : results) {
        EXPECT_TRUE(result);
    }
}

/**
 * @test PerformanceBenchmark
 * @brief 性能基准测试
 */
TEST_F(AESNetworkIntegrationTest, PerformanceBenchmark) {
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16);
    auto encryptor = std::make_shared<AESEncryptor>(encryption_key);
    
    // 创建1MB的测试数据
    const size_t data_size = 1024 * 1024;
    std::vector<uint8_t> test_data(data_size);
    for (size_t i = 0; i < data_size; ++i) {
        test_data[i] = static_cast<uint8_t>(i % 256);
    }
    
    // 测试加密速度
    auto start = std::chrono::high_resolution_clock::now();
    auto encrypted = encryptor->Encrypt(test_data);
    auto encrypt_end = std::chrono::high_resolution_clock::now();
    
    // 测试解密速度
    auto decrypt_start = std::chrono::high_resolution_clock::now();
    auto decrypted = encryptor->Decrypt(encrypted);
    auto decrypt_end = std::chrono::high_resolution_clock::now();
    
    // 计算耗时
    auto encrypt_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        encrypt_end - start).count();
    auto decrypt_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        decrypt_end - decrypt_start).count();
    
    // 输出性能数据
    std::cout << "Encryption Performance Test:" << std::endl;
    std::cout << "  Data Size: " << data_size / (1024.0 * 1024.0) << " MB" << std::endl;
    std::cout << "  Encryption Time: " << encrypt_time << " ms" << std::endl;
    std::cout << "  Decryption Time: " << decrypt_time << " ms" << std::endl;
    
    if (encrypt_time > 0) {
        double encrypt_throughput = (data_size / (1024.0 * 1024.0)) / (encrypt_time / 1000.0);
        std::cout << "  Encryption Throughput: " << encrypt_throughput << " MB/s" << std::endl;
    }
    
    if (decrypt_time > 0) {
        double decrypt_throughput = (data_size / (1024.0 * 1024.0)) / (decrypt_time / 1000.0);
        std::cout << "  Decryption Throughput: " << decrypt_throughput << " MB/s" << std::endl;
    }
    
    // 验证解密结果
    EXPECT_EQ(decrypted, test_data);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
/**
 * @file aes_network_integration_test.cc
 * @brief AES加密网络集成测试
 * 
 * 演示客户端-服务器通过AES-256-CBC加密进行通信
 */

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <iostream>
#include "network/encryption.h"
#include "network/network.h"

using namespace sqlcc::network;

/**
 * @class AESNetworkIntegrationTest
 * @brief AES网络集成测试套件
 */
class AESNetworkIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 检查AES是否可用
        if (!AESEncryptor::IsAvailable()) {
            GTEST_SKIP() << "AES encryption not available on this platform";
        }
    }
    
    void TearDown() override {
        // 清理资源
    }
};

/**
 * @test EncryptionKeyExchange
 * @brief 测试加密密钥交换过程
 */
TEST_F(AESNetworkIntegrationTest, EncryptionKeyExchange) {
    // 服务器生成密钥
    auto server_key = EncryptionKey::GenerateRandom(32, 16);
    ASSERT_TRUE(server_key);
    
    // 客户端接收服务器的IV
    std::vector<uint8_t> received_iv = server_key->GetIV();
    
    // 客户端生成自己的密钥但使用服务器的IV
    auto client_key = EncryptionKey::GenerateRandom(32, 16);
    auto client_key_with_server_iv = std::make_shared<EncryptionKey>(
        client_key->GetKey(),
        received_iv
    );
    
    // 验证密钥已正确设置
    EXPECT_EQ(client_key_with_server_iv->GetIV(), received_iv);
    EXPECT_EQ(client_key_with_server_iv->GetKey().size(), 32);
}

/**
 * @test SessionAESEncryption
 * @brief 测试Session中的AES加密
 */
TEST_F(AESNetworkIntegrationTest, SessionAESEncryption) {
    // 创建session
    auto session = std::make_shared<Session>(1);
    ASSERT_TRUE(session);
    
    // 初始化时没有AES加密器
    EXPECT_FALSE(session->IsAESEncryptionEnabled());
    
    // 生成密钥并设置AES加密器
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16);
    auto aes_encryptor = std::make_shared<AESEncryptor>(encryption_key);
    session->SetAESEncryptor(aes_encryptor);
    
    // 验证AES加密已启用
    EXPECT_TRUE(session->IsAESEncryptionEnabled());
    EXPECT_TRUE(session->GetAESEncryptor());
}

/**
 * @test MessageEncryptionDecryption
 * @brief 测试消息的加密和解密
 */
TEST_F(AESNetworkIntegrationTest, MessageEncryptionDecryption) {
    // 生成密钥
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16);
    auto encryptor = std::make_shared<AESEncryptor>(encryption_key);
    
    // 模拟消息头和消息体
    struct MessageHeader {
        uint32_t magic;
        uint32_t length;
        uint16_t type;
        uint16_t flags;
        uint32_t sequence_id;
    };
    
    std::string query = "SELECT * FROM users WHERE id = 1;";
    std::vector<uint8_t> original_data(query.begin(), query.end());
    
    // 加密消息体
    auto encrypted_data = encryptor->Encrypt(original_data);
    EXPECT_FALSE(encrypted_data.empty());
    EXPECT_NE(encrypted_data, original_data);
    
    // 解密消息体
    auto decrypted_data = encryptor->Decrypt(encrypted_data);
    EXPECT_EQ(decrypted_data, original_data);
    
    // 验证解密的消息内容
    std::string decrypted_query(decrypted_data.begin(), decrypted_data.end());
    EXPECT_EQ(decrypted_query, query);
}

/**
 * @test SQLQueryEncryption
 * @brief 测试SQL查询的加密传输
 */
TEST_F(AESNetworkIntegrationTest, SQLQueryEncryption) {
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16);
    auto encryptor = std::make_shared<AESEncryptor>(encryption_key);
    
    // 模拟各种SQL查询
    std::vector<std::string> sql_queries = {
        "SELECT * FROM users;",
        "INSERT INTO users (id, name) VALUES (1, 'Alice');",
        "UPDATE users SET name = 'Bob' WHERE id = 1;",
        "DELETE FROM users WHERE id = 2;",
        "CREATE TABLE products (id INT, name VARCHAR(255), price DECIMAL(10,2));",
        "DROP TABLE IF EXISTS temp_table;",
        "ALTER TABLE users ADD COLUMN email VARCHAR(255);"
    };
    
    for (const auto& query : sql_queries) {
        std::vector<uint8_t> data(query.begin(), query.end());
        
        // 加密
        auto encrypted = encryptor->Encrypt(data);
        EXPECT_FALSE(encrypted.empty());
        
        // 解密
        auto decrypted = encryptor->Decrypt(encrypted);
        
        // 验证
        EXPECT_EQ(decrypted, data);
        std::string recovered(decrypted.begin(), decrypted.end());
        EXPECT_EQ(recovered, query);
        
        std::cout << "✓ Query encrypted/decrypted: " << query << std::endl;
    }
}

/**
 * @test ConcurrentEncryption
 * @brief 测试并发加密
 */
TEST_F(AESNetworkIntegrationTest, ConcurrentEncryption) {
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16);
    auto encryptor = std::make_shared<AESEncryptor>(encryption_key);
    
    const int num_threads = 4;
    const int iterations_per_thread = 10;
    
    std::vector<std::thread> threads;
    std::vector<bool> results(num_threads * iterations_per_thread, false);
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < iterations_per_thread; ++i) {
                std::string message = "Thread " + std::to_string(t) + " Message " + std::to_string(i);
                std::vector<uint8_t> data(message.begin(), message.end());
                
                // 加密
                auto encrypted = encryptor->Encrypt(data);
                
                // 解密
                auto decrypted = encryptor->Decrypt(encrypted);
                
                // 验证
                bool success = (decrypted == data);
                results[t * iterations_per_thread + i] = success;
            }
        });
    }
    
    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }
    
    // 验证所有操作都成功
    for (bool result : results) {
        EXPECT_TRUE(result);
    }
}

/**
 * @test PerformanceBenchmark
 * @brief 性能基准测试
 */
TEST_F(AESNetworkIntegrationTest, PerformanceBenchmark) {
    auto encryption_key = EncryptionKey::GenerateRandom(32, 16);
    auto encryptor = std::make_shared<AESEncryptor>(encryption_key);
    
    // 创建1MB的测试数据
    const size_t data_size = 1024 * 1024;
    std::vector<uint8_t> test_data(data_size);
    for (size_t i = 0; i < data_size; ++i) {
        test_data[i] = static_cast<uint8_t>(i % 256);
    }
    
    // 测试加密速度
    auto start = std::chrono::high_resolution_clock::now();
    auto encrypted = encryptor->Encrypt(test_data);
    auto encrypt_end = std::chrono::high_resolution_clock::now();
    
    // 测试解密速度
    auto decrypt_start = std::chrono::high_resolution_clock::now();
    auto decrypted = encryptor->Decrypt(encrypted);
    auto decrypt_end = std::chrono::high_resolution_clock::now();
    
    // 计算耗时
    auto encrypt_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        encrypt_end - start).count();
    auto decrypt_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        decrypt_end - decrypt_start).count();
    
    // 输出性能数据
    std::cout << "Encryption Performance Test:" << std::endl;
    std::cout << "  Data Size: " << data_size / (1024.0 * 1024.0) << " MB" << std::endl;
    std::cout << "  Encryption Time: " << encrypt_time << " ms" << std::endl;
    std::cout << "  Decryption Time: " << decrypt_time << " ms" << std::endl;
    
    if (encrypt_time > 0) {
        double encrypt_throughput = (data_size / (1024.0 * 1024.0)) / (encrypt_time / 1000.0);
        std::cout << "  Encryption Throughput: " << encrypt_throughput << " MB/s" << std::endl;
    }
    
    if (decrypt_time > 0) {
        double decrypt_throughput = (data_size / (1024.0 * 1024.0)) / (decrypt_time / 1000.0);
        std::cout << "  Decryption Throughput: " << decrypt_throughput << " MB/s" << std::endl;
    }
    
    // 验证解密结果
    EXPECT_EQ(decrypted, test_data);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
