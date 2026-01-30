#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <memory>

#include "src/network/message_types.h"

namespace sqlcc {
namespace network {

/**
 * @brief 消息序列化器类
 *
 * MessageSerializer类负责消息的序列化和反序列化，包括：
 * - 消息头的序列化/反序列化
 * - 消息体的序列化/反序列化
 * - 字节序转换
 * - 数据校验和验证
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