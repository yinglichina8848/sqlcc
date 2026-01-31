// Expected magic number for message validation
// const uint32_t EXPECTED_MAGIC = 0x534C4343; // "SLCC" in ASCII - defined in network.h

#pragma once

#include <cstdint>

namespace sqlcc {
namespace network {

/**
 * @struct MessageHeader
 * @brief 通用消息头结构 - 定义 SQLCC 内部协议的帧结构
 *
 * WHY层 - 设计意图：
 *   网络传输的原始字节流是无界的（Stream-oriented）。为了能够从 TCP 流中准确拆分出完整的消息包，
 *   必须定义一个固定格式的头部。魔数（Magic Number）用于过滤非法连接，长度字段用于界定包边界，
 *   序列号则用于请求-响应配对及丢包检测。
 *
 * WHAT层 - 功能说明：
 *   magic: 协议识别码，验证数据源是否来自 SQLCC 客户端。
 *   length: 指明紧随其后的 Payload 数据长度（不含头部本身）。
 *   type: 消息功能分类（见 MessageType 枚举）。
 *   flags: 位掩码，支持压缩、加密、分片等扩展特性。
 *
 * HOW层 - 实现机制：
 *   1. 内存布局：使用 #pragma pack(push, 1) 强制 1 字节对齐，防止编译器插入 Padding 导致跨平台不兼容。
 *   2. 校验逻辑：服务端读取首 4 字节并与常量匹配，若不符则立即断开连接以防御探测。
 */
#pragma pack(push, 1)
struct MessageHeader {
    uint32_t magic;         ///< 魔数 (0x53514C43 = 'SQLC')
    uint32_t length;        ///< 消息体长度
    uint8_t type;           ///< 消息类型
    uint8_t flags;          ///< 标志位
    uint32_t sequence_id;   ///< 序列号
};
#pragma pack(pop)

/**
 * @enum MessageType
 * @brief 内部协议消息类型枚举 - 定义 C/S 交互的状态语意
 *
 * WHY层 - 设计意图：
 *   SQLCC 协议是一个多阶段的有状态协议。通过定义明确的 Type，处理器（ConnectionHandler）
 *   可以根据当前会话状态（如 AUTH_PENDING）判断收到的包是否合法（如是否提前发送了 QUERY）。
 *
 * WHAT层 - 类型说明：
 *   CONNECT/CONN_ACK: 建立逻辑会话。
 *   AUTH/AUTH_ACK: 交换凭据并验证身份。
 *   QUERY/QUERY_RESULT: 核心业务逻辑，传输 SQL 文本和行数据。
 *   KEY_EXCHANGE: 密钥协商（如 DH 算法），为会话加密提供基础。
 *
 * HOW层 - 应用机制：
 *   1. 状态映射：类型值直接对应处理器内部的状态转移矩阵。
 *   2. 分发逻辑：ProcessMessage 方法通过 switch(type) 实现业务逻辑的快速派发。
 */
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
