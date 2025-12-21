/**
 * @file encryption.h
 * @brief 网络通信加密模块头文件
 *
 * Why: 需要统一的加密接口来保护网络通信安全
 * What: 定义了网络通信加密相关类和函数的接口
 * How: 包含各种加密算法的实现，支持简单XOR和AES-256-CBC加密
 *
 * Note: 加密类已重构为独立文件，每个类一个文件。保持此头文件用于向后兼容性。
 */

// Why: 防止头文件被多次包含，避免编译错误
// What: 使用#pragma once指令确保头文件只被编译一次
// How: 在文件开头添加#pragma once预处理指令
#pragma once

// Why: 包含所有分离的加密头文件，保持向后兼容性
// What: 包含所有加密类定义，保持API不变
// How: 使用#include预处理指令包含分离的加密头文件
#include "encryption/encryption_key.h"
#include "encryption/simple_encryptor.h"
#include "encryption/aes_encryptor.h"
#include "encryption/hmac_sha256.h"
#include "encryption/pbkdf2.h"

// Why: 将所有加密类放在命名空间中，避免命名冲突
// What: 定义sqlcc::network命名空间，包含所有网络加密相关的类
// How: 使用namespace关键字定义命名空间，并使用using声明导出所有加密类
namespace sqlcc {
namespace network {

// 导出所有加密类到sqlcc::network命名空间，保持向后兼容性
using sqlcc::network::EncryptionKey;
using sqlcc::network::SimpleEncryptor;
using sqlcc::network::AESEncryptor;
using sqlcc::network::HMACSHA256;
using sqlcc::network::PBKDF2;

// 导出辅助函数
using sqlcc::network::DeriveEncryptionKeyFromPassword;

} // namespace network
} // namespace sqlcc
