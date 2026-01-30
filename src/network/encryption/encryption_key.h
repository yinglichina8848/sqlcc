/**
 * @file encryption_key.h
 * @brief 加密密钥容器类定义
 *
 * Why: 需要一个专门的类来管理加密密钥和初始化向量
 * What: EncryptionKey类封装加密密钥和IV的管理
 * How: 提供密钥存储、生成和管理功能
 */

#pragma once

#include <vector>
#include <memory>
#include <cstdint>

namespace sqlcc {
namespace network {

/**
 * @brief 加密密钥容器类
 *
 * 存储加密密钥和初始化向量(IV)，提供密钥管理功能
 */
class EncryptionKey {
public:
    /**
     * @brief 构造函数
     * @param key 加密密钥
     * @param iv 初始化向量
     */
    EncryptionKey(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv);

    /**
     * @brief 生成随机密钥和IV
     * @param key_size 密钥大小（字节）
     * @param iv_size IV大小（字节）
     * @return 生成的密钥对象
     */
    static std::shared_ptr<EncryptionKey> GenerateRandom(size_t key_size = 32, size_t iv_size = 16);

    /**
     * @brief 获取密钥（常量版本）
     * @return 加密密钥
     */
    const std::vector<uint8_t>& GetKey() const { return key_; }

    /**
     * @brief 获取IV（常量版本）
     * @return 初始化向量
     */
    const std::vector<uint8_t>& GetIV() const { return iv_; }

    /**
     * @brief 获取密钥（可修改版本）
     * @return 加密密钥
     */
    std::vector<uint8_t>& GetKey() { return key_; }

    /**
     * @brief 获取IV（可修改版本）
     * @return 初始化向量
     */
    std::vector<uint8_t>& GetIV() { return iv_; }

private:
    std::vector<uint8_t> key_;  ///< 加密密钥
    std::vector<uint8_t> iv_;   ///< 初始化向量
};

} // namespace network
} // namespace sqlcc
