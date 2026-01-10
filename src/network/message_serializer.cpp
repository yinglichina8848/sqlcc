#include "network/message_serializer.h"
#include <cstring>
#include <stdexcept>
#include <arpa/inet.h>  // for htonl, ntohl

namespace sqlcc {
namespace network {

// 静态成员初始化
bool MessageSerializer::crc32_table_initialized_ = false;
uint32_t MessageSerializer::crc32_table_[256];

MessageSerializer::MessageSerializer() {
    // 初始化CRC32表（只在第一次调用时）
    if (!crc32_table_initialized_) {
        InitializeCRC32Table();
        crc32_table_initialized_ = true;
    }
}

MessageSerializer::~MessageSerializer() {
    // 无需特殊清理
}

std::vector<char> MessageSerializer::Serialize(uint8_t type, uint8_t flags, uint32_t sequence_id,
                                              const std::vector<char>& payload) {
    // 计算负载的CRC32校验和
    uint32_t crc32 = CalculateCRC32(payload);

    // 创建消息头
    MessageHeader header;
    header.magic = HostToNetwork32(0x53514C43);  // 'SQLC'
    header.length = HostToNetwork32(static_cast<uint32_t>(payload.size()));
    header.type = type;
    header.flags = flags;
    header.sequence_id = HostToNetwork32(sequence_id);

    // 计算总大小：消息头 + 负载数据 + CRC32（4字节）
    size_t total_size = sizeof(MessageHeader) + payload.size() + sizeof(uint32_t);
    std::vector<char> result(total_size);

    // 复制消息头
    std::memcpy(result.data(), &header, sizeof(MessageHeader));

    // 复制负载数据
    if (!payload.empty()) {
        std::memcpy(result.data() + sizeof(MessageHeader), payload.data(), payload.size());
    }

    // 添加CRC32校验和（网络字节序）
    uint32_t network_crc32 = HostToNetwork32(crc32);
    std::memcpy(result.data() + sizeof(MessageHeader) + payload.size(), &network_crc32, sizeof(uint32_t));

    return result;
}

bool MessageSerializer::DeserializeHeader(const std::vector<char>& data, MessageHeader& header) {
    // 检查数据长度是否足够
    if (data.size() < sizeof(MessageHeader)) {
        return false;
    }

    // 复制消息头
    std::memcpy(&header, data.data(), sizeof(MessageHeader));

    // 转换字节序
    header.magic = NetworkToHost32(header.magic);
    header.length = NetworkToHost32(header.length);
    header.sequence_id = NetworkToHost32(header.sequence_id);

    // 验证魔数
    if (header.magic != 0x53514C43) {  // 'SQLC'
        return false;
    }

    return true;
}

bool MessageSerializer::DeserializeMessage(const std::vector<char>& data, uint8_t& type, uint8_t& flags,
                                          uint32_t& sequence_id, std::vector<char>& payload) {
    MessageHeader header;

    // 反序列化消息头
    if (!DeserializeHeader(data, header)) {
        return false;
    }

    // 检查数据长度是否足够（消息头 + 负载 + CRC32）
    size_t expected_size = sizeof(MessageHeader) + header.length + sizeof(uint32_t);
    if (data.size() != expected_size) {
        return false;
    }

    // 提取消息参数
    type = header.type;
    flags = header.flags;
    sequence_id = header.sequence_id;

    // 提取负载数据
    payload.resize(header.length);
    if (header.length > 0) {
        std::memcpy(payload.data(), data.data() + sizeof(MessageHeader), header.length);
    }

    // 提取并验证CRC32校验和
    uint32_t received_crc32;
    std::memcpy(&received_crc32, data.data() + sizeof(MessageHeader) + header.length, sizeof(uint32_t));
    received_crc32 = NetworkToHost32(received_crc32);

    if (!VerifyCRC32(payload, received_crc32)) {
        return false;  // 校验和验证失败
    }

    return true;
}

uint32_t MessageSerializer::CalculateCRC32(const std::vector<char>& data) {
    uint32_t crc = 0xFFFFFFFF;  // 初始化CRC值为全1

    for (size_t i = 0; i < data.size(); ++i) {
        uint8_t byte = static_cast<uint8_t>(data[i]);
        crc = crc32_table_[(crc ^ byte) & 0xFF] ^ (crc >> 8);
    }

    return crc ^ 0xFFFFFFFF;  // 最终XOR
}

bool MessageSerializer::VerifyCRC32(const std::vector<char>& data, uint32_t expected_crc) {
    uint32_t calculated_crc = CalculateCRC32(data);
    return calculated_crc == expected_crc;
}

uint32_t MessageSerializer::HostToNetwork32(uint32_t value) {
#ifdef __linux__
    return htonl(value);
#else
    // 对于非Linux系统，使用简化的字节序转换
    // 注意：这只是示例，实际实现应根据目标平台调整
    return ((value >> 24) & 0xFF) |
           ((value >> 8) & 0xFF00) |
           ((value << 8) & 0xFF0000) |
           ((value << 24) & 0xFF000000);
#endif
}

uint32_t MessageSerializer::NetworkToHost32(uint32_t value) {
#ifdef __linux__
    return ntohl(value);
#else
    // 对于非Linux系统，使用简化的字节序转换
    return ((value >> 24) & 0xFF) |
           ((value >> 8) & 0xFF00) |
           ((value << 8) & 0xFF0000) |
           ((value << 24) & 0xFF000000);
#endif
}

void MessageSerializer::InitializeCRC32Table() {
    // CRC32多项式：0xEDB88320
    const uint32_t polynomial = 0xEDB88320;

    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t crc = i;
        for (int j = 0; j < 8; ++j) {
            if (crc & 1) {
                crc = (crc >> 1) ^ polynomial;
            } else {
                crc >>= 1;
            }
        }
        crc32_table_[i] = crc;
    }
}

} // namespace network
} // namespace sqlcc