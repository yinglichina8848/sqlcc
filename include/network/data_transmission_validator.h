/**
 * @file data_transmission_validator.h
 * @brief 数据传输边界检查器头文件
 */

#ifndef SQLCC_NETWORK_DATA_TRANSMISSION_VALIDATOR_H
#define SQLCC_NETWORK_DATA_TRANSMISSION_VALIDATOR_H

#include <vector>
#include <deque>
#include <chrono>
#include <mutex>
#include <memory>

#include "network/message_types.h"

namespace sqlcc {
namespace network {

// 数据传输边界检查器 - 防止缓冲区溢出和数据包完整性问题
class DataTransmissionValidator {
public:
    DataTransmissionValidator();
    ~DataTransmissionValidator() = default;

    // 消息头验证
    bool ValidateMessageHeader(const MessageHeader& header) const;

    // 消息长度验证
    bool ValidateMessageLength(size_t declared_length, size_t actual_length) const;

    // 魔数验证
    bool ValidateMessageMagic(uint32_t magic) const;

    // 消息类型验证
    bool ValidateMessageType(uint16_t type) const;

    // 缓冲区大小验证
    bool IsBufferSizeValid(size_t buffer_size) const;

    // 消息大小限制检查
    bool IsMessageSizeWithinLimits(size_t message_size) const;

    // 分片处理
    bool ShouldFragmentMessage(size_t message_size) const;
    std::vector<std::vector<char>> FragmentMessage(const std::vector<char>& message);
    bool ValidateFragment(const std::vector<char>& fragment) const;
    std::vector<char> ReassembleFragments(const std::vector<std::vector<char>>& fragments);

    // 流量控制
    bool CanAcceptMessage(size_t message_size, std::chrono::milliseconds time_window);
    void RecordMessageSent(size_t message_size);
    void RecordMessageReceived(size_t message_size);
    double GetCurrentThroughput() const;
    bool IsRateLimited() const;

    // 配置参数
    void SetMaxMessageSize(size_t max_size);
    void SetMaxBufferSize(size_t max_size);
    void SetFragmentSize(size_t fragment_size);
    void SetRateLimit(size_t bytes_per_second);

    // 获取配置
    size_t GetMaxMessageSize() const;
    size_t GetMaxBufferSize() const;

private:
    // 配置参数
    size_t max_message_size_;
    size_t max_buffer_size_;
    size_t fragment_size_;
    size_t rate_limit_bytes_per_sec_;

    // 统计数据
    size_t total_bytes_sent_;
    size_t total_bytes_received_;

    // 流量控制数据
    mutable std::mutex traffic_mutex_;
    std::deque<std::pair<std::chrono::steady_clock::time_point, size_t>> sent_messages_;
    std::deque<std::pair<std::chrono::steady_clock::time_point, size_t>> received_messages_;

    // 辅助方法
    void CleanupOldRecords();
    size_t CalculateThroughput(const std::deque<std::pair<std::chrono::steady_clock::time_point, size_t>>& records) const;
};

} // namespace network
} // namespace sqlcc

#endif // SQLCC_NETWORK_DATA_TRANSMISSION_VALIDATOR_H
