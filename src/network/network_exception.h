/**
 * @file network_exception.h
 * @brief 网络异常类定义
 *
 * Why: 需要专门的异常类来处理网络通信中的各种错误情况
 * What: NetworkException类提供网络通信异常的分类和处理
 * How: 实现异常恢复策略和错误信息管理
 */

#pragma once

#include <stdexcept>
#include <string>

namespace sqlcc {
namespace network {

/**
 * @brief 网络异常类型枚举
 */
enum NetworkExceptionType {
    CONNECTION_LOST = 0,           ///< 连接丢失
    CONNECTION_TIMEOUT = 1,        ///< 连接超时
    AUTHENTICATION_FAILED = 2,     ///< 认证失败
    PROTOCOL_VIOLATION = 3,        ///< 协议违规
    RESOURCE_EXHAUSTED = 4,        ///< 资源耗尽
    DATA_CORRUPTION = 5,           ///< 数据损坏
    RATE_LIMIT_EXCEEDED = 6,       ///< 速率限制超限
    SYSTEM_OVERLOAD = 7,           ///< 系统过载
    NETWORK_UNAVAILABLE = 8,       ///< 网络不可用
    UNKNOWN_ERROR = 9             ///< 未知错误
};

/**
 * @brief 网络异常类
 *
 * 专门处理网络通信中的异常情况，提供详细的错误信息和恢复建议
 */
class NetworkException : public std::runtime_error {
public:
    /**
     * @brief 构造函数
     * @param type 异常类型
     * @param message 错误消息
     * @param details 详细信息
     * @param recoverable 是否可恢复
     */
    NetworkException(NetworkExceptionType type,
                    const std::string& message,
                    const std::string& details = "",
                    bool recoverable = true);

    /**
     * @brief 析构函数
     */
    ~NetworkException() override = default;

    /**
     * @brief 获取异常类型
     * @return 异常类型
     */
    NetworkExceptionType GetType() const { return type_; }

    /**
     * @brief 获取详细信息
     * @return 详细信息
     */
    const std::string& GetDetails() const { return details_; }

    /**
     * @brief 检查是否可恢复
     * @return 是否可恢复
     */
    bool IsRecoverable() const { return recoverable_; }

    /**
     * @brief 获取完整错误信息
     * @return 完整的错误信息字符串
     */
    std::string GetFullMessage() const;

private:
    NetworkExceptionType type_;    ///< 异常类型
    std::string details_;          ///< 详细信息
    bool recoverable_;             ///< 是否可恢复
};

} // namespace network
} // namespace sqlcc
