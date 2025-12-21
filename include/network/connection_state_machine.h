/**
 * @file connection_state_machine.h
 * @brief 连接状态机类定义
 *
 * Why: 需要专门的类来管理数据库连接的复杂状态转换
 * What: ConnectionStateMachine类实现严格的连接状态机控制
 * How: 提供线程安全的连接状态管理、重连机制和超时控制
 */

#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

namespace sqlcc {
namespace network {

/**
 * @brief 连接状态枚举
 *
 * 严格定义连接状态机的所有可能状态
 */
enum ConnectionState {
    DISCONNECTED = 0,       ///< 未连接
    CONNECTING = 1,         ///< 正在连接
    CONNECTED = 2,          ///< 已连接但未认证
    AUTHENTICATING = 3,     ///< 正在认证
    AUTHENTICATED = 4,      ///< 已认证可以正常通信
    KEY_EXCHANGING = 5,     ///< 正在进行密钥交换
    ENCRYPTED = 6,          ///< 已加密可以安全通信
    CLOSING = 7,           ///< 正在关闭
    CLOSED = 8,            ///< 已关闭
    CONNECTION_ERROR = 9   ///< 连接错误状态
};

/**
 * @brief 连接状态机类
 *
 * 严格验证连接状态转换安全性，提供重连、超时和保活机制
 */
class ConnectionStateMachine {
public:
    /**
     * @brief 构造函数
     */
    ConnectionStateMachine();

    /**
     * @brief 析构函数
     */
    ~ConnectionStateMachine() = default;

    /**
     * @brief 获取当前状态
     * @return 当前连接状态
     */
    ConnectionState GetCurrentState() const;

    /**
     * @brief 检查是否处于指定状态
     * @param state 要检查的状态
     * @return 是否处于该状态
     */
    bool IsInState(ConnectionState state) const;

    /**
     * @brief 获取状态名称
     * @param state 状态枚举值
     * @return 状态的字符串名称
     */
    std::string GetStateName(ConnectionState state) const;

    /**
     * @brief 转换到新状态
     * @param new_state 目标状态
     * @return 是否转换成功
     */
    bool TransitionTo(ConnectionState new_state);

    /**
     * @brief 检查是否可以转换到指定状态
     * @param new_state 目标状态
     * @return 是否可以转换
     */
    bool CanTransitionTo(ConnectionState new_state) const;

    // 便捷状态检查方法
    bool IsDisconnected() const { return state_ == DISCONNECTED; }
    bool IsConnecting() const { return state_ == CONNECTING; }
    bool IsConnected() const { return state_ == CONNECTED || state_ == AUTHENTICATED || state_ == ENCRYPTED; }
    bool IsAuthenticated() const { return state_ == AUTHENTICATED || state_ == ENCRYPTED; }
    bool IsEncrypted() const { return state_ == ENCRYPTED; }
    bool IsClosed() const { return state_ == CLOSED || state_ == CLOSING; }
    bool IsError() const { return state_ == CONNECTION_ERROR; }

    // 允许的操作检查
    bool CanConnect() const;
    bool CanAuthenticate() const;
    bool CanSendQuery() const;
    bool CanEncrypt() const;
    bool CanClose() const;

    /**
     * @brief 重置状态机
     */
    void Reset();

    /**
     * @brief 强制设置为错误状态
     */
    void ForceError();

    /**
     * @brief 设置状态变更回调
     * @param callback 状态变更回调函数
     */
    using StateChangeCallback = std::function<void(ConnectionState old_state, ConnectionState new_state)>;
    void SetStateChangeCallback(StateChangeCallback callback);

    /**
     * @brief 设置超时和重连参数
     * @param connect_timeout 连接超时时间
     * @param reconnect_delay 重连延迟时间
     * @param keep_alive_interval 保活间隔时间
     */
    void SetTimeouts(std::chrono::milliseconds connect_timeout = std::chrono::seconds(30),
                    std::chrono::milliseconds reconnect_delay = std::chrono::seconds(5),
                    std::chrono::milliseconds keep_alive_interval = std::chrono::seconds(60));

    /**
     * @brief 启动连接计时器
     */
    void StartConnectionTimer();

    /**
     * @brief 停止连接计时器
     */
    void StopConnectionTimer();

    /**
     * @brief 检查连接是否超时
     * @return 是否超时
     */
    bool IsConnectionTimedOut() const;

    /**
     * @brief 重置连接计时器
     */
    void ResetConnectionTimer();

    /**
     * @brief 启用自动重连
     * @param enabled 是否启用
     * @param max_retry_attempts 最大重试次数
     */
    void EnableAutoReconnect(bool enabled, int max_retry_attempts = 3);

    /**
     * @brief 检查是否启用自动重连
     * @return 是否启用
     */
    bool IsAutoReconnectEnabled() const;

    /**
     * @brief 获取最大重试次数
     * @return 最大重试次数
     */
    int GetMaxRetryAttempts() const;

    /**
     * @brief 获取当前重试次数
     * @return 当前重试次数
     */
    int GetCurrentRetryCount() const;

    /**
     * @brief 增加重试次数
     */
    void IncrementRetryCount();

    /**
     * @brief 重置重试次数
     */
    void ResetRetryCount();

    /**
     * @brief 检查是否应该尝试重连
     * @return 是否应该重连
     */
    bool ShouldAttemptReconnect() const;

    /**
     * @brief 获取重连延迟时间
     * @return 重连延迟时间
     */
    std::chrono::milliseconds GetReconnectDelay() const;

    /**
     * @brief 启用保活机制
     * @param enabled 是否启用
     */
    void EnableKeepAlive(bool enabled);

    /**
     * @brief 检查是否启用保活机制
     * @return 是否启用
     */
    bool IsKeepAliveEnabled() const;

    /**
     * @brief 更新最后活动时间
     */
    void UpdateLastActivity();

    /**
     * @brief 检查保活是否超时
     * @return 是否超时
     */
    bool IsKeepAliveTimeout() const;

private:
    ConnectionState state_;                           ///< 当前状态
    mutable std::mutex mutex_;                        ///< 线程安全锁
    StateChangeCallback state_change_callback_;       ///< 状态变更回调

    // 超时和重连相关成员变量
    std::chrono::milliseconds connect_timeout_;       ///< 连接超时时间
    std::chrono::milliseconds reconnect_delay_;       ///< 重连延迟时间
    std::chrono::milliseconds keep_alive_interval_;   ///< 保活间隔时间

    std::chrono::steady_clock::time_point connection_start_time_;  ///< 连接开始时间
    std::chrono::steady_clock::time_point last_activity_time_;     ///< 最后活动时间
    bool connection_timer_active_;                      ///< 连接计时器是否激活
    bool auto_reconnect_enabled_;                       ///< 是否启用自动重连
    bool keep_alive_enabled_;                           ///< 是否启用保活
    int max_retry_attempts_;                            ///< 最大重试次数
    int current_retry_count_;                           ///< 当前重试次数

    /**
     * @brief 验证状态转换是否有效
     * @param from 起始状态
     * @param to 目标状态
     * @return 是否有效
     */
    bool IsValidTransition(ConnectionState from, ConnectionState to) const;
};

} // namespace network
} // namespace sqlcc
