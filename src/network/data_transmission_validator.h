#ifndef SQLCC_DATA_TRANSMISSION_VALIDATOR_H
#define SQLCC_DATA_TRANSMISSION_VALIDATOR_H

#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
#include <cstdint>

namespace sqlcc {
namespace network {

/**
 * @brief 消息头结构
 */
struct MessageHeader {
    uint32_t magic;       // 魔数，用于消息识别
    uint16_t version;     // 协议版本
    uint16_t type;        // 消息类型
    uint32_t length;      // 消息长度
    uint32_t sequence;    // 序列号
    uint32_t checksum;    // 校验和

    MessageHeader() : magic(0), version(0), type(0), length(0), sequence(0), checksum(0) {}
};

/**
 * @brief 数据传输验证器
 *
 * 该类负责验证网络数据传输的完整性、安全性和性能。
 */
class DataTransmissionValidator {
public:
    DataTransmissionValidator();
    virtual ~DataTransmissionValidator() = default;

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

    // 获取最大消息大小
    size_t GetMaxMessageSize() const;

    // 获取最大缓冲区大小
    size_t GetMaxBufferSize() const;

    // 消息分片决策
    bool ShouldFragmentMessage(size_t message_size) const;

    // 消息分片
    std::vector<std::vector<char>> FragmentMessage(const std::vector<char>& message);

    // 分片验证
    bool ValidateFragment(const std::vector<char>& fragment) const;

    // 分片重组
    std::vector<char> ReassembleFragments(const std::vector<std::vector<char>>& fragments);

    // 流量控制
    bool CanAcceptMessage(size_t message_size, std::chrono::milliseconds time_window);

    // 统计记录
    void RecordMessageSent(size_t message_size);
    void RecordMessageReceived(size_t message_size);

    // 性能监控
    double GetCurrentThroughput() const;

    // 速率限制检查
    bool IsRateLimited() const;

private:
    // 内部方法
    uint32_t CalculateChecksum(const std::vector<char>& data) const;
    bool IsValidMagicNumber(uint32_t magic) const;
    bool IsValidMessageType(uint16_t type) const;

    // 成员变量
    size_t max_message_size_;           // 最大消息大小
    size_t max_buffer_size_;            // 最大缓冲区大小
    size_t fragment_size_;              // 分片大小
    uint32_t expected_magic_;           // 期望的魔数

    // 速率限制参数
    size_t max_messages_per_second_;    // 每秒最大消息数
    size_t max_bytes_per_second_;       // 每秒最大字节数

    // 统计数据（原子操作保证线程安全）
    std::atomic<size_t> messages_sent_;
    std::atomic<size_t> messages_received_;
    std::atomic<size_t> bytes_sent_;
    std::atomic<size_t> bytes_received_;

    // 时间跟踪
    std::chrono::steady_clock::time_point last_check_time_;
    std::chrono::steady_clock::time_point start_time_;

    // 互斥锁保护非原子数据
    mutable std::mutex stats_mutex_;
};

} // namespace network
} // namespace sqlcc

#endif // SQLCC_DATA_TRANSMISSION_VALIDATOR_H