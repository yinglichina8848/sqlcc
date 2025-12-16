/**
 * @file data_transmission_validator_test.cpp
 * @brief 数据传输边界检查器测试套件
 *
 * 验证数据传输边界检查器的各项功能，包括：
 * - 数据包完整性验证
 * - 缓冲区边界检查
 * - 大数据包分片处理
 * - 流量控制和限速
 */

#include "network/network.h"
#include <gtest/gtest.h>
#include <vector>
#include <thread>
#include <atomic>

using namespace sqlcc::network;
using namespace std::chrono_literals;

// 测试夹具
class DataTransmissionValidatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        validator_ = std::make_unique<DataTransmissionValidator>();
    }

    void TearDown() override {
        validator_.reset();
    }

    std::unique_ptr<DataTransmissionValidator> validator_;

    // 辅助函数：创建有效的消息头
    MessageHeader CreateValidHeader(uint32_t length = 100, uint16_t type = QUERY) {
        MessageHeader header;
        header.magic = 0x53434C53; // 'SQLC'
        header.length = length;
        header.type = type;
        header.flags = 0;
        header.sequence_id = 1;
        return header;
    }

    // 辅助函数：创建无效的消息头
    MessageHeader CreateInvalidHeader() {
        MessageHeader header;
        header.magic = 0xDEADBEEF; // 无效魔数
        header.length = 100;
        header.type = QUERY;
        header.flags = 0;
        header.sequence_id = 1;
        return header;
    }
};

// 数据包完整性验证测试
TEST_F(DataTransmissionValidatorTest, ValidateMessageHeader_ValidHeader) {
    MessageHeader header = CreateValidHeader();
    EXPECT_TRUE(validator_->ValidateMessageHeader(header));
}

TEST_F(DataTransmissionValidatorTest, ValidateMessageHeader_InvalidMagic) {
    MessageHeader header = CreateInvalidHeader();
    EXPECT_FALSE(validator_->ValidateMessageHeader(header));
}

TEST_F(DataTransmissionValidatorTest, ValidateMessageHeader_InvalidType) {
    MessageHeader header = CreateValidHeader();
    header.type = 999; // 无效类型
    EXPECT_FALSE(validator_->ValidateMessageHeader(header));
}

TEST_F(DataTransmissionValidatorTest, ValidateMessageHeader_ZeroLength) {
    MessageHeader header = CreateValidHeader();
    header.length = 0;
    EXPECT_FALSE(validator_->ValidateMessageHeader(header));
}

TEST_F(DataTransmissionValidatorTest, ValidateMessageHeader_TooLarge) {
    MessageHeader header = CreateValidHeader();
    header.length = validator_->GetMaxMessageSize() + 1;
    EXPECT_FALSE(validator_->ValidateMessageHeader(header));
}

TEST_F(DataTransmissionValidatorTest, ValidateMessageLength_Match) {
    EXPECT_TRUE(validator_->ValidateMessageLength(100, 100));
}

TEST_F(DataTransmissionValidatorTest, ValidateMessageLength_Mismatch) {
    EXPECT_FALSE(validator_->ValidateMessageLength(100, 150));
}

TEST_F(DataTransmissionValidatorTest, ValidateMessageMagic_Valid) {
    EXPECT_TRUE(validator_->ValidateMessageMagic(0x53434C53)); // 'SQLC'
}

TEST_F(DataTransmissionValidatorTest, ValidateMessageMagic_Invalid) {
    EXPECT_FALSE(validator_->ValidateMessageMagic(0xDEADBEEF));
}

TEST_F(DataTransmissionValidatorTest, ValidateMessageType_ValidRange) {
    EXPECT_TRUE(validator_->ValidateMessageType(CONNECT));
    EXPECT_TRUE(validator_->ValidateMessageType(QUERY));
    EXPECT_TRUE(validator_->ValidateMessageType(KEY_EXCHANGE_ACK));
}

TEST_F(DataTransmissionValidatorTest, ValidateMessageType_Invalid) {
    EXPECT_FALSE(validator_->ValidateMessageType(999));
    EXPECT_FALSE(validator_->ValidateMessageType(0xFFFF));
}

// 缓冲区边界检查测试
TEST_F(DataTransmissionValidatorTest, IsBufferSizeValid_ValidSize) {
    EXPECT_TRUE(validator_->IsBufferSizeValid(1024));
    EXPECT_TRUE(validator_->IsBufferSizeValid(1024 * 1024)); // 1MB
}

TEST_F(DataTransmissionValidatorTest, IsBufferSizeValid_TooLarge) {
    EXPECT_FALSE(validator_->IsBufferSizeValid(validator_->GetMaxBufferSize() + 1));
}

TEST_F(DataTransmissionValidatorTest, IsBufferSizeValid_Zero) {
    EXPECT_FALSE(validator_->IsBufferSizeValid(0));
}

TEST_F(DataTransmissionValidatorTest, IsMessageSizeWithinLimits_ValidSize) {
    EXPECT_TRUE(validator_->IsMessageSizeWithinLimits(1000));
    EXPECT_TRUE(validator_->IsMessageSizeWithinLimits(1024 * 1024)); // 1MB
}

TEST_F(DataTransmissionValidatorTest, IsMessageSizeWithinLimits_TooLarge) {
    EXPECT_FALSE(validator_->IsMessageSizeWithinLimits(validator_->GetMaxMessageSize() + 1));
}

TEST_F(DataTransmissionValidatorTest, IsMessageSizeWithinLimits_Zero) {
    EXPECT_FALSE(validator_->IsMessageSizeWithinLimits(0));
}

TEST_F(DataTransmissionValidatorTest, GetMaxMessageSize_Default) {
    EXPECT_EQ(validator_->GetMaxMessageSize(), 64 * 1024 * 1024); // 64MB
}

TEST_F(DataTransmissionValidatorTest, GetMaxBufferSize_Default) {
    EXPECT_EQ(validator_->GetMaxBufferSize(), 128 * 1024 * 1024); // 128MB
}

TEST_F(DataTransmissionValidatorTest, SetMaxMessageSize_Valid) {
    size_t new_size = 32 * 1024 * 1024; // 32MB
    validator_->SetMaxMessageSize(new_size);
    EXPECT_EQ(validator_->GetMaxMessageSize(), new_size);
}

TEST_F(DataTransmissionValidatorTest, SetMaxMessageSize_Invalid) {
    EXPECT_THROW(validator_->SetMaxMessageSize(0), std::invalid_argument);
    EXPECT_THROW(validator_->SetMaxMessageSize(validator_->GetMaxBufferSize() + 1), std::invalid_argument);
}

// 大数据包分片处理测试
TEST_F(DataTransmissionValidatorTest, ShouldFragmentMessage_SmallMessage) {
    EXPECT_FALSE(validator_->ShouldFragmentMessage(1000));
}

TEST_F(DataTransmissionValidatorTest, ShouldFragmentMessage_LargeMessage) {
    EXPECT_TRUE(validator_->ShouldFragmentMessage(2 * 1024 * 1024)); // 2MB
}

TEST_F(DataTransmissionValidatorTest, FragmentMessage_Empty) {
    std::vector<char> empty_message;
    auto fragments = validator_->FragmentMessage(empty_message);
    EXPECT_TRUE(fragments.empty());
}

TEST_F(DataTransmissionValidatorTest, FragmentMessage_Small) {
    std::vector<char> message(1000, 'A');
    auto fragments = validator_->FragmentMessage(message);

    // 小消息不应该被分片
    EXPECT_EQ(fragments.size(), 1);
    EXPECT_TRUE(validator_->ValidateFragment(fragments[0]));
}

TEST_F(DataTransmissionValidatorTest, FragmentMessage_Large) {
    size_t large_size = 3 * 1024 * 1024; // 3MB
    std::vector<char> message(large_size, 'B');
    auto fragments = validator_->FragmentMessage(message);

    // 大消息应该被分片
    EXPECT_GT(fragments.size(), 1);

    // 验证所有分片
    for (const auto& fragment : fragments) {
        EXPECT_TRUE(validator_->ValidateFragment(fragment));
    }
}

TEST_F(DataTransmissionValidatorTest, ValidateFragment_Valid) {
    std::vector<char> message(1000, 'C');
    auto fragments = validator_->FragmentMessage(message);

    ASSERT_FALSE(fragments.empty());
    EXPECT_TRUE(validator_->ValidateFragment(fragments[0]));
}

TEST_F(DataTransmissionValidatorTest, ValidateFragment_InvalidHeader) {
    std::vector<char> invalid_fragment = {'I', 'N', 'V', 'A', 'L', 'I', 'D'};
    EXPECT_FALSE(validator_->ValidateFragment(invalid_fragment));
}

TEST_F(DataTransmissionValidatorTest, ValidateFragment_WrongSize) {
    std::vector<char> message(1000, 'D');
    auto fragments = validator_->FragmentMessage(message);

    ASSERT_FALSE(fragments.empty());
    auto& fragment = fragments[0];

    // 修改分片大小使其无效
    fragment.resize(fragment.size() - 10);
    EXPECT_FALSE(validator_->ValidateFragment(fragment));
}

TEST_F(DataTransmissionValidatorTest, ReassembleFragments_Valid) {
    std::vector<char> original_message(5000, 'E');
    auto fragments = validator_->FragmentMessage(original_message);
    auto reassembled = validator_->ReassembleFragments(fragments);

    EXPECT_EQ(reassembled.size(), original_message.size());
    EXPECT_EQ(reassembled, original_message);
}

TEST_F(DataTransmissionValidatorTest, ReassembleFragments_InvalidFragment) {
    std::vector<std::vector<char>> invalid_fragments = {
        {'I', 'N', 'V', 'A', 'L', 'I', 'D'}
    };

    EXPECT_THROW(validator_->ReassembleFragments(invalid_fragments), std::runtime_error);
}

TEST_F(DataTransmissionValidatorTest, ReassembleFragments_Empty) {
    std::vector<std::vector<char>> empty_fragments;
    auto result = validator_->ReassembleFragments(empty_fragments);
    EXPECT_TRUE(result.empty());
}

// 流量控制测试
TEST_F(DataTransmissionValidatorTest, CanAcceptMessage_UnderLimit) {
    size_t small_message = 1000;
    EXPECT_TRUE(validator_->CanAcceptMessage(small_message, 1s));
}

TEST_F(DataTransmissionValidatorTest, CanAcceptMessage_OverLimit) {
    // 设置较低的速率限制以便测试
    validator_->SetRateLimit(1000); // 1000字节/秒

    // 发送大数据包
    size_t large_message = 2000;
    EXPECT_FALSE(validator_->CanAcceptMessage(large_message, 1s));
}

TEST_F(DataTransmissionValidatorTest, RecordMessageSent) {
    size_t message_size = 1000;
    validator_->RecordMessageSent(message_size);

    // 验证吞吐量计算
    double throughput = validator_->GetCurrentThroughput();
    EXPECT_GE(throughput, 0.0);
}

TEST_F(DataTransmissionValidatorTest, RecordMessageReceived) {
    size_t message_size = 1000;
    validator_->RecordMessageReceived(message_size);

    // 验证吞吐量计算
    double throughput = validator_->GetCurrentThroughput();
    EXPECT_GE(throughput, 0.0);
}

TEST_F(DataTransmissionValidatorTest, GetCurrentThroughput_NoData) {
    double throughput = validator_->GetCurrentThroughput();
    EXPECT_EQ(throughput, 0.0);
}

TEST_F(DataTransmissionValidatorTest, GetCurrentThroughput_WithData) {
    // 记录一些消息
    validator_->RecordMessageSent(1000);
    std::this_thread::sleep_for(100ms); // 等待一段时间
    validator_->RecordMessageSent(1000);

    double throughput = validator_->GetCurrentThroughput();
    EXPECT_GT(throughput, 0.0);
}

TEST_F(DataTransmissionValidatorTest, IsRateLimited_UnderLimit) {
    validator_->SetRateLimit(10000); // 10KB/s
    validator_->RecordMessageSent(1000); // 1KB

    EXPECT_FALSE(validator_->IsRateLimited());
}

TEST_F(DataTransmissionValidatorTest, IsRateLimited_OverLimit) {
    validator_->SetRateLimit(1000); // 1KB/s
    validator_->RecordMessageSent(2000); // 2KB

    EXPECT_TRUE(validator_->IsRateLimited());
}

// 配置参数测试
TEST_F(DataTransmissionValidatorTest, SetFragmentSize_Valid) {
    size_t new_fragment_size = 512 * 1024; // 512KB
    validator_->SetFragmentSize(new_fragment_size);

    // 验证分片行为
    std::vector<char> large_message(1024 * 1024, 'F'); // 1MB
    auto fragments = validator_->FragmentMessage(large_message);

    // 应该被分片
    EXPECT_GT(fragments.size(), 1);
}

TEST_F(DataTransmissionValidatorTest, SetFragmentSize_Invalid) {
    EXPECT_THROW(validator_->SetFragmentSize(0), std::invalid_argument);
    EXPECT_THROW(validator_->SetFragmentSize(validator_->GetMaxMessageSize() + 1), std::invalid_argument);
}

TEST_F(DataTransmissionValidatorTest, SetRateLimit) {
    size_t new_limit = 50 * 1024 * 1024; // 50MB/s
    validator_->SetRateLimit(new_limit);

    // 验证新限制生效
    EXPECT_TRUE(validator_->CanAcceptMessage(1000, 1s));
}

// 边界条件测试
TEST_F(DataTransmissionValidatorTest, BoundaryConditions_MaxMessageSize) {
    size_t max_size = validator_->GetMaxMessageSize();
    std::vector<char> max_message(max_size, 'G');

    // 最大大小的消息应该有效
    EXPECT_TRUE(validator_->IsMessageSizeWithinLimits(max_size));
    EXPECT_FALSE(validator_->ShouldFragmentMessage(max_size));

    // 超过最大大小应该无效
    EXPECT_FALSE(validator_->IsMessageSizeWithinLimits(max_size + 1));
}

TEST_F(DataTransmissionValidatorTest, BoundaryConditions_BufferSize) {
    size_t max_buffer = validator_->GetMaxBufferSize();

    // 最大缓冲区大小应该有效
    EXPECT_TRUE(validator_->IsBufferSizeValid(max_buffer));

    // 超过最大缓冲区大小应该无效
    EXPECT_FALSE(validator_->IsBufferSizeValid(max_buffer + 1));
}

TEST_F(DataTransmissionValidatorTest, BoundaryConditions_EmptyBuffers) {
    // 空消息处理
    std::vector<char> empty_message;
    auto fragments = validator_->FragmentMessage(empty_message);
    EXPECT_TRUE(fragments.empty());

    // 空分片列表重组
    std::vector<std::vector<char>> empty_fragments;
    auto result = validator_->ReassembleFragments(empty_fragments);
    EXPECT_TRUE(result.empty());
}

// 并发访问测试
TEST_F(DataTransmissionValidatorTest, ConcurrentAccess_Safe) {
    std::atomic<bool> test_passed{true};
    std::vector<std::thread> threads;

    // 启动多个线程并发访问验证器
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([this, &test_passed, i]() {
            try {
                // 每个线程执行一系列操作
                for (int j = 0; j < 50; ++j) {
                    // 记录消息发送/接收
                    validator_->RecordMessageSent(100 * (i + 1));
                    validator_->RecordMessageReceived(100 * (i + 1));

                    // 检查各种验证
                    MessageHeader header = CreateValidHeader();
                    validator_->ValidateMessageHeader(header);
                    validator_->GetCurrentThroughput();
                    validator_->IsRateLimited();
                }
            } catch (...) {
                test_passed = false;
            }
        });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_TRUE(test_passed);
}

// 性能测试
TEST_F(DataTransmissionValidatorTest, Performance_ValidationSpeed) {
    MessageHeader header = CreateValidHeader();

    auto start = std::chrono::high_resolution_clock::now();

    // 执行大量验证操作
    for (int i = 0; i < 10000; ++i) {
        validator_->ValidateMessageHeader(header);
        validator_->IsBufferSizeValid(1024);
        validator_->IsMessageSizeWithinLimits(1000);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 验证性能合理（应该在几毫秒内完成）
    EXPECT_LT(duration.count(), 100); // 少于100ms
}

TEST_F(DataTransmissionValidatorTest, Performance_FragmentationSpeed) {
    std::vector<char> large_message(2 * 1024 * 1024, 'H'); // 2MB

    auto start = std::chrono::high_resolution_clock::now();

    // 执行分片操作
    for (int i = 0; i < 100; ++i) {
        auto fragments = validator_->FragmentMessage(large_message);
        // 不进行重组以专注于分片性能
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 验证分片性能合理
    EXPECT_LT(duration.count(), 500); // 少于500ms for 100 iterations
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
