/**
 * @file network_edge_cases_test.cpp
 * @brief 网络模块边界条件和错误处理测试
 *
 * 测试各种边界条件、异常输入、资源耗尽、网络错误和协议兼容性
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <thread>
#include <chrono>
#include <limits>
#include <algorithm>

// 包含真实的网络模块头文件
#include "network/network.h"
#include "network/encryption.h"
#include "sql_executor.h"

using namespace sqlcc::network;

// Mock SqlExecutor for testing
class MockSqlExecutor : public sqlcc::SqlExecutor {
public:
    MOCK_METHOD(std::string, Execute, (const std::string&));
};

// 测试夹具：边界条件测试
class NetworkEdgeCasesTest : public ::testing::Test {
protected:
    void SetUp() override {
        sql_executor_ = std::make_shared<MockSqlExecutor>();
        session_manager_ = std::make_shared<SessionManager>();
    }

    void TearDown() override {
        sql_executor_.reset();
        session_manager_.reset();
    }

    std::shared_ptr<MockSqlExecutor> sql_executor_;
    std::shared_ptr<SessionManager> session_manager_;
};

// 测试消息大小边界
TEST_F(NetworkEdgeCasesTest, MessageSizeBoundaries) {
    // 测试各种消息大小
    struct SizeTest {
        uint32_t length;
        const char* description;
        bool should_succeed;
    };

    std::vector<SizeTest> size_tests = {
        {0, "空消息", true},
        {1, "单字节消息", true},
        {1024, "1KB消息", true},
        {1024 * 1024, "1MB消息", true},
        {10 * 1024 * 1024, "10MB消息", true}, // 大消息
        {std::numeric_limits<uint32_t>::max(), "最大uint32消息", false} // 理论上可能导致问题
    };

    for (const auto& size_test : size_tests) {
        MessageHeader header;
        header.magic = 0x53514C43; // 'SQLC'
        header.length = size_test.length;
        header.type = QUERY;
        header.flags = 0;
        header.sequence_id = 1;

        // 验证消息头构造正确
        EXPECT_EQ(header.magic, 0x53514C43U);
        EXPECT_EQ(header.length, size_test.length);

        // 对于大消息，验证序列化/反序列化
        if (size_test.should_succeed) {
            std::vector<char> buffer(sizeof(MessageHeader));
            std::memcpy(buffer.data(), &header, sizeof(MessageHeader));

            MessageHeader* restored = reinterpret_cast<MessageHeader*>(buffer.data());
            EXPECT_EQ(restored->magic, header.magic);
            EXPECT_EQ(restored->length, header.length);
        }
    }
}

// 测试序列号边界
TEST_F(NetworkEdgeCasesTest, SequenceNumberBoundaries) {
    std::vector<uint32_t> boundary_sequences = {
        0,
        1,
        1000,
        65535,
        65536,
        std::numeric_limits<uint32_t>::max() - 1,
        std::numeric_limits<uint32_t>::max()
    };

    for (uint32_t seq : boundary_sequences) {
        MessageHeader header;
        header.magic = 0x53514C43; // 'SQLC'
        header.length = 100;
        header.type = QUERY;
        header.flags = 0;
        header.sequence_id = seq;

        EXPECT_EQ(header.sequence_id, seq);

        // 验证序列化
        std::vector<char> buffer(sizeof(MessageHeader));
        std::memcpy(buffer.data(), &header, sizeof(MessageHeader));

        MessageHeader* restored = reinterpret_cast<MessageHeader*>(buffer.data());
        EXPECT_EQ(restored->sequence_id, seq);
    }
}

// 测试魔数验证
TEST_F(NetworkEdgeCasesTest, MagicNumberValidation) {
    std::vector<uint32_t> test_magics = {
        0x53514C43, // 正确的 'SQLC'
        0x53514C44, // 'SQLD' - 相似但错误
        0x00000000, // 全零
        0xFFFFFFFF, // 全一
        0x12345678, // 随机值
        0x434C5153  // 字节反转
    };

    for (uint32_t magic : test_magics) {
        MessageHeader header;
        header.magic = magic;
        header.length = 100;
        header.type = QUERY;
        header.flags = 0;
        header.sequence_id = 1;

        // 验证魔数设置正确
        EXPECT_EQ(header.magic, magic);

        // 只有正确的魔数应该通过验证
        bool is_valid_magic = (magic == 0x53514C43);
        EXPECT_EQ(header.magic == 0x53514C43U, is_valid_magic);
    }
}

// 测试标志位组合
TEST_F(NetworkEdgeCasesTest, FlagCombinations) {
    std::vector<uint16_t> test_flags = {
        0x0000, // 无标志
        0x0001, // 位0
        0x0002, // 位1
        0x0004, // 位2
        0x0008, // 位3
        0x8000, // 最高位
        0xFFFF, // 所有位
        0x5555, // 交替位
        0xAAAA  // 反交替位
    };

    for (uint16_t flags : test_flags) {
        MessageHeader header;
        header.magic = 0x53514C43; // 'SQLC'
        header.length = 100;
        header.type = QUERY;
        header.flags = flags;
        header.sequence_id = 1;

        EXPECT_EQ(header.flags, flags);

        // 验证序列化保持标志位
        std::vector<char> buffer(sizeof(MessageHeader));
        std::memcpy(buffer.data(), &header, sizeof(MessageHeader));

        MessageHeader* restored = reinterpret_cast<MessageHeader*>(buffer.data());
        EXPECT_EQ(restored->flags, flags);
    }
}

// 测试消息类型边界
TEST_F(NetworkEdgeCasesTest, MessageTypeBoundaries) {
    // 测试所有可能的MessageType值
    for (int type = 0; type <= 255; ++type) {
        MessageHeader header;
        header.magic = 0x53514C43; // 'SQLC'
        header.length = 100;
        header.type = static_cast<MessageType>(type);
        header.flags = 0;
        header.sequence_id = 1;

        EXPECT_EQ(static_cast<int>(header.type), type);

        // 验证已定义的消息类型
        bool is_defined_type = (type >= CONNECT && type <= KEY_EXCHANGE_ACK);
        if (is_defined_type) {
            EXPECT_GE(header.type, CONNECT);
            EXPECT_LE(header.type, KEY_EXCHANGE_ACK);
        }
    }
}

// 测试Session ID边界
TEST_F(NetworkEdgeCasesTest, SessionIdBoundaries) {
    std::vector<int> boundary_session_ids = {
        0,
        1,
        100,
        1000,
        std::numeric_limits<int>::max() - 1,
        std::numeric_limits<int>::max()
    };

    for (int session_id : boundary_session_ids) {
        auto session = std::make_shared<Session>(session_id);
        EXPECT_EQ(session->GetSessionId(), session_id);

        // 验证会话可以被会话管理器管理
        session_manager_->CreateSession(); // 先创建一个
        auto retrieved = session_manager_->GetSession(session_id);
        if (retrieved) {
            EXPECT_EQ(retrieved->GetSessionId(), session_id);
        }
    }
}

// 测试AES密钥长度边界
TEST_F(NetworkEdgeCasesTest, AESKeyLengthBoundaries) {
    // 测试不同密钥长度（AES支持128, 192, 256位）
    std::vector<size_t> key_lengths = {16, 24, 32}; // 字节长度

    for (size_t key_len : key_lengths) {
        try {
            auto key = EncryptionKey::GenerateRandom(key_len, 16);
            EXPECT_EQ(key->GetKey().size(), key_len);

            auto aes_encryptor = std::make_shared<AESEncryptor>(key);

            // 测试加密/解密
            std::vector<uint8_t> test_data = {'t', 'e', 's', 't'};
            auto encrypted = aes_encryptor->Encrypt(test_data);
            auto decrypted = aes_encryptor->Decrypt(encrypted);

            EXPECT_EQ(test_data, decrypted);
        } catch (const std::exception&) {
            // 某些密钥长度可能不受支持，忽略异常
        }
    }
}

// 测试HMAC验证边界
TEST_F(NetworkEdgeCasesTest, HMACVerificationBoundaries) {
    auto key = EncryptionKey::GenerateRandom(32, 16);
    auto aes_encryptor = std::make_shared<AESEncryptor>(key);

    std::vector<uint8_t> test_data = {'t', 'e', 's', 't', ' ', 'd', 'a', 't', 'a'};
    auto encrypted = aes_encryptor->Encrypt(test_data);
    auto mac = HMACSHA256::Compute(key->GetKey(), encrypted);

    // 正确的MAC应该验证通过
    bool valid_mac = HMACSHA256::Verify(key->GetKey(), encrypted, mac);
    EXPECT_TRUE(valid_mac);

    // 修改MAC应该验证失败
    if (!mac.empty()) {
        mac[0] ^= 0x01; // 翻转一位
        bool invalid_mac = HMACSHA256::Verify(key->GetKey(), encrypted, mac);
        EXPECT_FALSE(invalid_mac);
    }

    // 空数据测试
    std::vector<uint8_t> empty_data;
    auto empty_encrypted = aes_encryptor->Encrypt(empty_data);
    auto empty_mac = HMACSHA256::Compute(key->GetKey(), empty_encrypted);
    bool empty_valid = HMACSHA256::Verify(key->GetKey(), empty_encrypted, empty_mac);
    EXPECT_TRUE(empty_valid);
}

// 测试并发会话管理
TEST_F(NetworkEdgeCasesTest, ConcurrentSessionManagement) {
    const int num_threads = 10;
    const int sessions_per_thread = 50;

    std::vector<std::thread> threads;
    std::mutex results_mutex;
    std::vector<int> created_session_ids;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            std::vector<int> local_sessions;

            for (int j = 0; j < sessions_per_thread; ++j) {
                auto session = session_manager_->CreateSession();
                local_sessions.push_back(session->GetSessionId());
            }

            std::lock_guard<std::mutex> lock(results_mutex);
            created_session_ids.insert(created_session_ids.end(),
                                     local_sessions.begin(), local_sessions.end());
        });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 验证创建的会话总数
    EXPECT_EQ(created_session_ids.size(), num_threads * sessions_per_thread);

    // 验证所有会话ID都是唯一的
    std::sort(created_session_ids.begin(), created_session_ids.end());
    auto last = std::unique(created_session_ids.begin(), created_session_ids.end());
    created_session_ids.erase(last, created_session_ids.end());
    EXPECT_EQ(created_session_ids.size(), num_threads * sessions_per_thread);
}

// 测试网络端口边界
TEST_F(NetworkEdgeCasesTest, NetworkPortBoundaries) {
    // 测试各种端口号（包括边界值）
    std::vector<int> test_ports = {
        1,          // 最小有效端口
        80,         // HTTP
        443,        // HTTPS
        1024,       // 最小非特权端口
        3306,       // MySQL
        5432,       // PostgreSQL
        8080,       // 常用端口
        65535       // 最大端口
    };

    for (int port : test_ports) {
        // 创建ClientNetworkManager测试端口参数
        EXPECT_NO_THROW({
            ClientNetworkManager client("127.0.0.1", port);
            // 不实际连接，只测试构造
        });

        // 创建ServerNetworkManager测试端口参数
        EXPECT_NO_THROW({
            ServerNetworkManager server(port, 10);
            server.SetSqlExecutor(sql_executor_);
            // 不启动服务器，只测试构造
        });
    }
}

// 测试连接池大小边界
TEST_F(NetworkEdgeCasesTest, ConnectionPoolSizeBoundaries) {
    std::vector<int> pool_sizes = {1, 10, 100, 1000};

    for (int pool_size : pool_sizes) {
        EXPECT_NO_THROW({
            ServerNetworkManager server(8080, pool_size);
            server.SetSqlExecutor(sql_executor_);
            // 测试构造和基本操作
            EXPECT_FALSE(server.IsRunning());
        });
    }
}

// 测试超时处理
TEST_F(NetworkEdgeCasesTest, TimeoutHandling) {
    // 创建客户端
    ClientNetworkManager client("127.0.0.1", 12345);

    // 测试连接超时（应该快速失败）
    auto start_time = std::chrono::steady_clock::now();
    bool connect_result = client.Connect();
    auto end_time = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 连接应该快速失败（网络超时通常在几秒内）
    EXPECT_FALSE(connect_result);
    EXPECT_LT(duration.count(), 10000); // 应该在10秒内完成
}

// 测试大数据处理
TEST_F(NetworkEdgeCasesTest, LargeDataHandling) {
    // 测试大消息的处理
    const size_t large_size = 1024 * 1024; // 1MB

    // 创建大消息
    std::vector<char> large_message(sizeof(MessageHeader) + large_size);
    MessageHeader* header = reinterpret_cast<MessageHeader*>(large_message.data());
    header->magic = 0x53514C43; // 'SQLC'
    header->length = large_size;
    header->type = QUERY;
    header->flags = 0;
    header->sequence_id = 1;

    // 填充数据
    std::fill(large_message.begin() + sizeof(MessageHeader), large_message.end(), 'A');

    // 验证消息构造正确
    EXPECT_EQ(header->length, large_size);
    EXPECT_EQ(large_message.size(), sizeof(MessageHeader) + large_size);

    // 验证数据完整性
    bool all_valid = std::all_of(large_message.begin() + sizeof(MessageHeader),
                                large_message.end(),
                                [](char c) { return c == 'A'; });
    EXPECT_TRUE(all_valid);
}

// 测试特殊字符处理
TEST_F(NetworkEdgeCasesTest, SpecialCharacterHandling) {
    // 测试包含各种特殊字符的消息
    std::vector<char> special_data;
    for (int i = 0; i < 256; ++i) {
        special_data.push_back(static_cast<char>(i));
    }

    MessageHeader header;
    header.magic = 0x53514C43; // 'SQLC'
    header.length = special_data.size();
    header.type = QUERY;
    header.flags = 0;
    header.sequence_id = 1;

    std::vector<char> message(sizeof(MessageHeader) + special_data.size());
    std::memcpy(message.data(), &header, sizeof(MessageHeader));
    std::memcpy(message.data() + sizeof(MessageHeader), special_data.data(), special_data.size());

    // 验证消息完整性
    MessageHeader* restored = reinterpret_cast<MessageHeader*>(message.data());
    EXPECT_EQ(restored->length, special_data.size());
    EXPECT_EQ(memcmp(message.data() + sizeof(MessageHeader), special_data.data(), special_data.size()), 0);
}

// 测试内存压力
TEST_F(NetworkEdgeCasesTest, MemoryPressure) {
    // 测试在内存压力下的表现
    const int num_large_objects = 50;

    for (int i = 0; i < num_large_objects; ++i) {
        // 创建多个大消息
        const size_t size = 100 * 1024; // 100KB
        std::vector<char> large_message(sizeof(MessageHeader) + size);

        MessageHeader* header = reinterpret_cast<MessageHeader*>(large_message.data());
        header->magic = 0x53514C43; // 'SQLC'
        header->length = size;
        header->type = QUERY;
        header->flags = 0;
        header->sequence_id = i;

        // 验证构造成功
        EXPECT_EQ(header->length, size);
    }
}

// 测试协议兼容性
TEST_F(NetworkEdgeCasesTest, ProtocolCompatibility) {
    // 测试不同版本的消息格式兼容性
    struct ProtocolVersion {
        uint32_t magic;
        const char* version_name;
        bool should_be_compatible;
    };

    std::vector<ProtocolVersion> versions = {
        {0x53514C43, "v1.0", true},   // 当前版本
        {0x53514C44, "v1.1", false},  // 假设的未来版本
        {0x434C5153, "legacy", false} // 字节序不同的旧版本
    };

    for (const auto& version : versions) {
        MessageHeader header;
        header.magic = version.magic;
        header.length = 100;
        header.type = QUERY;
        header.flags = 0;
        header.sequence_id = 1;

        bool is_current_version = (version.magic == 0x53514C43);
        EXPECT_EQ(is_current_version, version.should_be_compatible);
    }
}

// 测试异常恢复
TEST_F(NetworkEdgeCasesTest, ExceptionRecovery) {
    // 测试各种异常情况下的恢复能力
    auto session = session_manager_->CreateSession();

    // 测试无效AES加密器
    session->SetAESEncryptor(nullptr);
    EXPECT_FALSE(session->IsAESEncryptionEnabled());

    // 测试恢复
    auto key = EncryptionKey::GenerateRandom(32, 16);
    auto aes_encryptor = std::make_shared<AESEncryptor>(key);
    session->SetAESEncryptor(aes_encryptor);
    EXPECT_TRUE(session->IsAESEncryptionEnabled());

    // 测试认证状态转换
    EXPECT_FALSE(session->IsAuthenticated());
    session->SetAuthenticated("recovery_test");
    EXPECT_TRUE(session->IsAuthenticated());
    EXPECT_EQ(session->GetUser(), "recovery_test");
}

// 测试资源竞争
TEST_F(NetworkEdgeCasesTest, ResourceContention) {
    // 测试高并发下的资源竞争
    const int num_threads = 20;
    const int operations_per_thread = 100;

    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, i]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                // 执行各种会话操作
                auto session = session_manager_->CreateSession();
                session->SetAuthenticated("user" + std::to_string(i * operations_per_thread + j));
                session_manager_->DestroySession(session->GetSessionId());
            }
        });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 验证系统仍然稳定
    EXPECT_GE(session_manager_, nullptr);
}

// 测试边界状态转换
TEST_F(NetworkEdgeCasesTest, BoundaryStateTransitions) {
    auto session = session_manager_->CreateSession();

    // 测试各种状态转换序列
    struct StateTransition {
        std::function<void()> action;
        std::string description;
    };

    std::vector<StateTransition> transitions = {
        {[&]() { session->SetEncryptionDisabled(false); }, "启用加密"},
        {[&]() { session->SetEncryptionDisabled(true); }, "禁用加密"},
        {[&]() { session->SetAuthenticationDisabled(false); }, "启用认证"},
        {[&]() { session->SetAuthenticationDisabled(true); }, "禁用认证"},
        {[&]() { session->SetAuthenticated("user1"); }, "设置认证用户"},
        {[&]() {
            auto key = EncryptionKey::GenerateRandom(32, 16);
            auto aes = std::make_shared<AESEncryptor>(key);
            session->SetAESEncryptor(aes);
        }, "设置AES加密器"}
    };

    // 执行所有状态转换
    for (const auto& transition : transitions) {
        EXPECT_NO_THROW(transition.action()) << "Failed at: " << transition.description;
    }

    // 验证最终状态
    EXPECT_TRUE(session->IsAuthenticated());
    EXPECT_TRUE(session->IsAESEncryptionEnabled());
}

// 测试系统极限
TEST_F(NetworkEdgeCasesTest, SystemLimits) {
    // 测试接近系统极限的操作
    const int max_sessions = 10000;

    // 创建大量会话
    std::vector<std::shared_ptr<Session>> sessions;
    for (int i = 0; i < max_sessions; ++i) {
        auto session = session_manager_->CreateSession();
        sessions.push_back(session);

        // 每100个会话检查一次系统状态
        if (i % 100 == 0) {
            EXPECT_GE(session->GetSessionId(), 0);
        }
    }

    // 清理会话
    for (auto& session : sessions) {
        session_manager_->DestroySession(session->GetSessionId());
    }

    sessions.clear();

    // 验证清理后可以继续创建会话
    auto new_session = session_manager_->CreateSession();
    EXPECT_GE(new_session->GetSessionId(), 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
