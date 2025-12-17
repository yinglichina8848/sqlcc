/**
 * @file data_transmission_validator.cpp
 * @brief 数据传输边界检查器实现 - 防止缓冲区溢出和数据包完整性问题
 *
 * 该文件实现了数据传输边界检查器的核心功能，包括：
 * - 数据包完整性验证
 * - 缓冲区边界检查
 * - 大数据包分片处理
 * - 流量控制和限速
 */

#include "network/network.h"
#include <algorithm>
#include <stdexcept>
#include <cstring>

namespace sqlcc {
namespace network {

// 数据传输边界检查器实现
DataTransmissionValidator::DataTransmissionValidator()
    : max_message_size_(64 * 1024 * 1024),      // 64MB
      max_buffer_size_(128 * 1024 * 1024),     // 128MB
      fragment_size_(1024 * 1024),             // 1MB
      rate_limit_bytes_per_sec_(100 * 1024 * 1024), // 100MB/s
      total_bytes_sent_(0),
      total_bytes_received_(0) {
}

bool DataTransmissionValidator::ValidateMessageHeader(const MessageHeader& header) const {
    // 验证魔数
    if (!ValidateMessageMagic(header.magic)) {
        return false;
    }

    // 验证消息类型
    if (!ValidateMessageType(header.type)) {
        return false;
    }

    // 验证消息长度
    if (header.length > max_message_size_) {
        return false;
    }

    // 验证长度不为0（消息头本身不算在长度内）
    if (header.length == 0) {
        return false;
    }

    return true;
}

bool DataTransmissionValidator::ValidateMessageLength(size_t declared_length, size_t actual_length) const {
    // 精确匹配长度
    return declared_length == actual_length;
}

bool DataTransmissionValidator::ValidateMessageMagic(uint32_t magic) const {
    return magic == EXPECTED_MAGIC;
}

bool DataTransmissionValidator::ValidateMessageType(uint16_t type) const {
    // 验证消息类型在有效范围内
    return type >= CONNECT && type <= KEY_EXCHANGE_ACK;
}

bool DataTransmissionValidator::IsBufferSizeValid(size_t buffer_size) const {
    return buffer_size <= max_buffer_size_ && buffer_size > 0;
}

bool DataTransmissionValidator::IsMessageSizeWithinLimits(size_t message_size) const {
    return message_size <= max_message_size_ && message_size > 0;
}

size_t DataTransmissionValidator::GetMaxMessageSize() const {
    return max_message_size_;
}

size_t DataTransmissionValidator::GetMaxBufferSize() const {
    return max_buffer_size_;
}

bool DataTransmissionValidator::ShouldFragmentMessage(size_t message_size) const {
    return message_size > fragment_size_;
}

std::vector<std::vector<char>> DataTransmissionValidator::FragmentMessage(const std::vector<char>& message) {
    std::vector<std::vector<char>> fragments;

    if (message.empty()) {
        return fragments;
    }

    size_t total_size = message.size();
    size_t offset = 0;

    while (offset < total_size) {
        size_t remaining = total_size - offset;
        size_t fragment_data_size = std::min(remaining, fragment_size_);

        // 创建分片：消息头 + 数据
        std::vector<char> fragment;
        fragment.reserve(sizeof(MessageHeader) + fragment_data_size);

        // 构建分片消息头
        MessageHeader header;
        header.magic = EXPECTED_MAGIC;
        header.length = static_cast<uint32_t>(fragment_data_size);
        header.type = QUERY; // 假设是查询消息的分片
        header.flags = 0x0001; // 设置分片标志
        header.sequence_id = 0; // 分片序号可以后续设置

        // 添加消息头
        fragment.insert(fragment.end(),
                       reinterpret_cast<char*>(&header),
                       reinterpret_cast<char*>(&header) + sizeof(MessageHeader));

        // 添加数据
        fragment.insert(fragment.end(),
                       message.begin() + offset,
                       message.begin() + offset + fragment_data_size);

        fragments.push_back(std::move(fragment));
        offset += fragment_data_size;
    }

    return fragments;
}

bool DataTransmissionValidator::ValidateFragment(const std::vector<char>& fragment) const {
    // 分片至少要包含消息头
    if (fragment.size() < sizeof(MessageHeader)) {
        return false;
    }

    // 使用智能指针管理消息头，避免裸指针
    auto header = std::make_shared<MessageHeader>();
    std::memcpy(header.get(), fragment.data(), sizeof(MessageHeader));
    if (!ValidateMessageHeader(*header)) {
        return false;
    }

    // 验证分片大小
    size_t expected_size = sizeof(MessageHeader) + header->length;
    if (fragment.size() != expected_size) {
        return false;
    }

    return true;
}

std::vector<char> DataTransmissionValidator::ReassembleFragments(const std::vector<std::vector<char>>& fragments) {
    if (fragments.empty()) {
        return {};
    }

    // 计算总数据大小
    size_t total_data_size = 0;
    for (const auto& fragment : fragments) {
        if (!ValidateFragment(fragment)) {
            throw std::runtime_error("Invalid fragment detected during reassembly");
        }

        // 使用智能指针管理消息头，避免裸指针
        auto header = std::make_shared<MessageHeader>();
        std::memcpy(header.get(), fragment.data(), sizeof(MessageHeader));
        total_data_size += header->length;
    }

    // 重组数据
    std::vector<char> reassembled_data;
    reassembled_data.reserve(total_data_size);

    for (const auto& fragment : fragments) {
        // 使用智能指针管理消息头，避免裸指针
        auto header = std::make_shared<MessageHeader>();
        std::memcpy(header.get(), fragment.data(), sizeof(MessageHeader));
        const char* data_start = fragment.data() + sizeof(MessageHeader);

        reassembled_data.insert(reassembled_data.end(),
                               data_start,
                               data_start + header->length);
    }

    return reassembled_data;
}

bool DataTransmissionValidator::CanAcceptMessage(size_t message_size, std::chrono::milliseconds time_window) {
    std::lock_guard<std::mutex> lock(traffic_mutex_);

    // 清理过期记录
    CleanupOldRecords();

    // 计算当前时间窗口内的流量
    auto now = std::chrono::steady_clock::now();
    auto window_start = now - time_window;

    size_t bytes_in_window = 0;
    for (const auto& record : sent_messages_) {
        if (record.first >= window_start) {
            bytes_in_window += record.second;
        }
    }

    // 检查是否超过速率限制
    size_t max_bytes_in_window = (rate_limit_bytes_per_sec_ * time_window.count()) / 1000;
    return (bytes_in_window + message_size) <= max_bytes_in_window;
}

void DataTransmissionValidator::RecordMessageSent(size_t message_size) {
    std::lock_guard<std::mutex> lock(traffic_mutex_);
    auto now = std::chrono::steady_clock::now();
    sent_messages_.emplace_back(now, message_size);
    total_bytes_sent_ += message_size;

    // 清理过期记录
    CleanupOldRecords();
}

void DataTransmissionValidator::RecordMessageReceived(size_t message_size) {
    std::lock_guard<std::mutex> lock(traffic_mutex_);
    auto now = std::chrono::steady_clock::now();
    received_messages_.emplace_back(now, message_size);
    total_bytes_received_ += message_size;

    // 清理过期记录
    CleanupOldRecords();
}

double DataTransmissionValidator::GetCurrentThroughput() const {
    std::lock_guard<std::mutex> lock(traffic_mutex_);

    // 计算最近60秒的吞吐量
    auto now = std::chrono::steady_clock::now();
    auto window_start = now - std::chrono::seconds(60);

    size_t bytes_in_window = 0;
    for (const auto& record : sent_messages_) {
        if (record.first >= window_start) {
            bytes_in_window += record.second;
        }
    }

    // 转换为字节/秒
    auto window_seconds = std::chrono::duration_cast<std::chrono::seconds>(now - window_start).count();
    return window_seconds > 0 ? static_cast<double>(bytes_in_window) / window_seconds : 0.0;
}

bool DataTransmissionValidator::IsRateLimited() const {
    std::lock_guard<std::mutex> lock(traffic_mutex_);

    // 检查最近1秒的流量
    auto now = std::chrono::steady_clock::now();
    auto window_start = now - std::chrono::seconds(1);

    size_t bytes_in_last_second = 0;
    for (const auto& record : sent_messages_) {
        if (record.first >= window_start) {
            bytes_in_last_second += record.second;
        }
    }

    return bytes_in_last_second >= rate_limit_bytes_per_sec_;
}

void DataTransmissionValidator::SetMaxMessageSize(size_t max_size) {
    if (max_size == 0 || max_size > max_buffer_size_) {
        throw std::invalid_argument("Invalid max message size");
    }
    max_message_size_ = max_size;
}

void DataTransmissionValidator::SetMaxBufferSize(size_t max_size) {
    if (max_size == 0 || max_size < max_message_size_) {
        throw std::invalid_argument("Invalid max buffer size");
    }
    max_buffer_size_ = max_size;
}

void DataTransmissionValidator::SetFragmentSize(size_t fragment_size) {
    if (fragment_size == 0 || fragment_size > max_message_size_) {
        throw std::invalid_argument("Invalid fragment size");
    }
    fragment_size_ = fragment_size;
}

void DataTransmissionValidator::SetRateLimit(size_t bytes_per_second) {
    rate_limit_bytes_per_sec_ = bytes_per_second;
}

void DataTransmissionValidator::CleanupOldRecords() {
    auto now = std::chrono::steady_clock::now();
    auto cutoff_time = now - std::chrono::minutes(5); // 保留5分钟的记录

    // 清理发送记录
    while (!sent_messages_.empty() && sent_messages_.front().first < cutoff_time) {
        sent_messages_.pop_front();
    }

    // 清理接收记录
    while (!received_messages_.empty() && received_messages_.front().first < cutoff_time) {
        received_messages_.pop_front();
    }
}

size_t DataTransmissionValidator::CalculateThroughput(const std::deque<std::pair<std::chrono::steady_clock::time_point, size_t>>& records) const {
    if (records.empty()) {
        return 0;
    }

    auto now = std::chrono::steady_clock::now();
    auto oldest_time = records.front().first;
    auto time_span = std::chrono::duration_cast<std::chrono::seconds>(now - oldest_time).count();

    if (time_span == 0) {
        return records.back().second; // 瞬间速率
    }

    size_t total_bytes = 0;
    for (const auto& record : records) {
        total_bytes += record.second;
    }

    return total_bytes / time_span; // 字节/秒
}

} // namespace network
} // namespace sqlcc