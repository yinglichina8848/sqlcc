/**
 * @file hmac_sha256.h
 * @brief HMAC-SHA256防篡改类定义
 *
 * Why: 需要防篡改机制来确保数据完整性
 * What: HMACSHA256类提供HMAC-SHA256消息认证码功能
 * How: 使用密钥对消息进行HMAC-SHA256计算和验证
 */

#pragma once

#include <vector>
#include <string>

namespace sqlcc {
namespace network {

/**
 * @brief HMAC-SHA256防篡改类
 *
 * 提供基于HMAC-SHA256算法的消息认证码功能，用于确保数据完整性和真实性。
 */
class HMACSHA256 {
public:
    /**
     * @brief 计算HMAC-SHA256值
     * @param key 密钥
     * @param data 待认证的数据
     * @return HMAC-SHA256值
     */
    static std::vector<uint8_t> Compute(const std::vector<uint8_t>& key,
                                        const std::vector<uint8_t>& data);

    /**
     * @brief 验证HMAC-SHA256值
     * @param key 密钥
     * @param data 原始数据
     * @param mac 待验证的HMAC值
     * @return 如果验证通过返回true，否则返回false
     */
    static bool Verify(const std::vector<uint8_t>& key,
                       const std::vector<uint8_t>& data,
                       const std::vector<uint8_t>& mac);
};

} // namespace network
} // namespace sqlcc
