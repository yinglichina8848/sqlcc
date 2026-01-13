#ifndef SQLCC_NETWORK_H
#define SQLCC_NETWORK_H

#include <cstdint>

// Message types for network protocol
enum MessageType {
    CONNECT = 1,
    QUERY = 2,
    RESPONSE = 3,
    ERROR = 4,
    AUTHENTICATE = 5,
    CLOSE = 6,
    KEY_EXCHANGE = 7,
    KEY_EXCHANGE_ACK = 8
};

// Expected magic number for message validation
const uint32_t EXPECTED_MAGIC = 0x534C4343; // "SLCC" in ASCII

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
    uint16_t flags;       // 标志位

    MessageHeader() : magic(0), version(0), type(0), length(0), sequence(0), checksum(0), flags(0) {}
};

} // namespace network
} // namespace sqlcc

#endif // SQLCC_NETWORK_H