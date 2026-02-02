#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <memory>

#include "message_types.h"

namespace sqlcc {
namespace network {

/**
 * @class MessageSerializer
 * @brief 通用消息序列化器 - 实现 SQLCC 内部协议的二进制封包与解包
 *
 * WHY层 - 设计意图：
 *   在分布式数据库或 C/S 架构中，复杂的内存对象（如查询结果、错误信息）无法直接在网络上传输。
 *   MessageSerializer 统一了数据的“物理表达”规范，解决了不同架构（如 x86 vs ARM）之间的
 *   字节序（Endianness）问题，并通过 CRC 校验确保了数据在不稳定的网络链路上的完整性。
 *
 * WHAT层 - 功能说明：
 *   提供结构化数据到 std::vector<char> 字节流的双向转换。
 *   处理 MessageHeader（包含类型、标志、序列号、载荷长度）的固定格式封装。
 *   实现基于 CRC32 算法的数据校验和计算与验证。
 *   支持网络字节序（Big-Endian）与主机字节序的自动转换。
 *
 * HOW层 - 实现机制：
 *   1. 协议头封装：Serialize 方法预留固定长度头部，填充长度字段和序列号。
 *   2. 字节流对齐：通过位移运算（Shift & Mask）实现跨平台的整数序列化。
 *   3. 查表法优化：InitializeCRC32Table 预计算 256 字节查找表，将校验速度提升一个数量级。
 *   4. 原子状态：crc32_table_initialized_ 保证全局查找表仅初始化一次。
 */
class MessageSerializer {
public:
    /**
     * @brief 构造函数
     */
    MessageSerializer();

    /**
     * @brief 析构函数
     */
    ~MessageSerializer();

    /**
     * @brief 序列化消息
     * @param type 消息类型
     * @param flags 消息标志
     * @param sequence_id 序列号
     * @param payload 消息负载数据
     * @return 序列化后的字节流
     */
    std::vector<char> Serialize(uint8_t type, uint8_t flags, uint32_t sequence_id,
                               const std::vector<char>& payload);

    /**
     * @brief 反序列化消息头
     * @param data 包含消息头的字节流
     * @param header 输出的消息头结构体
     * @return true表示反序列化成功，false表示失败
     */
    bool DeserializeHeader(const std::vector<char>& data, MessageHeader& header);

    /**
     * @brief 反序列化完整消息
     * @param data 完整的消息字节流
     * @param type 输出消息类型
     * @param flags 输出消息标志
     * @param sequence_id 输出序列号
     * @param payload 输出消息负载
     * @return true表示反序列化成功，false表示失败
     */
    bool DeserializeMessage(const std::vector<char>& data, uint8_t& type, uint8_t& flags,
                           uint32_t& sequence_id, std::vector<char>& payload);

    /**
     * @brief 计算CRC32校验和
     * @param data 要计算校验和的数据
     * @return CRC32校验和值
     */
    static uint32_t CalculateCRC32(const std::vector<char>& data);

    /**
     * @brief 验证CRC32校验和
     * @param data 数据
     * @param expected_crc 期望的CRC32值
     * @return true表示校验通过，false表示失败
     */
    static bool VerifyCRC32(const std::vector<char>& data, uint32_t expected_crc);

private:
    /**
     * @brief 主机字节序转网络字节序 (32位)
     * @param value 主机字节序值
     * @return 网络字节序值
     */
    static uint32_t HostToNetwork32(uint32_t value);

    /**
     * @brief 网络字节序转主机字节序 (32位)
     * @param value 网络字节序值
     * @return 主机字节序值
     */
    static uint32_t NetworkToHost32(uint32_t value);

    /**
     * @brief 初始化CRC32表
     */
    static void InitializeCRC32Table();

    static bool crc32_table_initialized_;  ///< CRC32表初始化标志
    static uint32_t crc32_table_[256];     ///< CRC32查找表
};

} // namespace network
} // namespace sqlcc