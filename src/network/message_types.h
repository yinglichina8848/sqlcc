// Expected magic number for message validation
// const uint32_t EXPECTED_MAGIC = 0x534C4343; // "SLCC" in ASCII - defined in network.h

#pragma once

#include <cstdint>

namespace sqlcc {
namespace network {

// 消息头结构定义
#pragma pack(push, 1)
struct MessageHeader {
    uint32_t magic;         ///< 魔数 (0x53514C43 = 'SQLC')
    uint32_t length;        ///< 消息体长度
    uint8_t type;           ///< 消息类型
    uint8_t flags;          ///< 标志位
    uint32_t sequence_id;   ///< 序列号
};
#pragma pack(pop)

// 消息类型定义
enum MessageType {
    CONNECT = 1,            ///< 连接请求
    CONN_ACK = 2,           ///< 连接确认
    AUTH = 3,               ///< 认证请求
    AUTH_ACK = 4,           ///< 认证确认
    QUERY = 5,              ///< 查询请求
    QUERY_RESULT = 6,       ///< 查询结果
    KEY_EXCHANGE = 7,       ///< 密钥交换请求
    KEY_EXCHANGE_ACK = 8,   ///< 密钥交换确认
    ERROR = 9               ///< 错误消息
};

} // namespace network
} // namespace sqlcc
