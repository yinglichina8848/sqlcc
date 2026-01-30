/**
 * @file network_exception_handler.h
 * @brief 网络异常处理器头文件
 */

#ifndef SQLCC_NETWORK_EXCEPTION_HANDLER_H
#define SQLCC_NETWORK_EXCEPTION_HANDLER_H

#include <unordered_map>
#include <chrono>
#include <deque>
#include <mutex>

#include "network/network_exception.h"

namespace sqlcc {
namespace network {

// 网络异常处理器 - 优雅降级和恢复
class NetworkExceptionHandler {
public:
    NetworkExceptionHandler();
    ~NetworkExceptionHandler() = default;

    // 异常处理策略
    enum RecoveryStrategy {
        IMMEDIATE_RETRY = 0,        // 立即重试
        DELAYED_RETRY = 1,          // 延迟重试
        GRACEFUL_DEGRADATION = 2,   // 优雅降级
        CIRCUIT_BREAKER = 3,        // 断路器
        SYSTEM_SHUTDOWN = 4         // 系统关闭
    };

    // 处理异常
    RecoveryStrategy HandleException(const NetworkException& exception,
                                   std::chrono::milliseconds time_since_last_failure);

    // 恢复策略管理
    void SetMaxRetries(NetworkExceptionType type, int max_retries);
    void SetRetryDelay(NetworkExceptionType type, std::chrono::milliseconds delay);
    void SetCircuitBreakerThreshold(NetworkExceptionType type, int threshold);
    void SetCircuitBreakerTimeout(NetworkExceptionType type, std::chrono::milliseconds timeout);

    // 断路器状态查询
    bool IsCircuitBreakerOpen(NetworkExceptionType type) const;
    std::chrono::milliseconds GetRemainingCircuitBreakerTimeout(NetworkExceptionType type) const;

    // 统计信息
    int GetExceptionCount(NetworkExceptionType type) const;
    int GetRecoveryCount(NetworkExceptionType type) const;
    double GetExceptionRate(NetworkExceptionType type, std::chrono::milliseconds window) const;

    // 重置统计
    void ResetStatistics(NetworkExceptionType type);
    void ResetAllStatistics();

private:
    struct ExceptionStats {
        int exception_count = 0;
        int recovery_count = 0;
        std::chrono::steady_clock::time_point last_exception_time;
        std::chrono::steady_clock::time_point circuit_breaker_opened_time;
        bool circuit_breaker_open = false;
        int max_retries = 3;
        std::chrono::milliseconds retry_delay = std::chrono::seconds(1);
        int circuit_breaker_threshold = 5;
        std::chrono::milliseconds circuit_breaker_timeout = std::chrono::minutes(1);
    };

    std::unordered_map<NetworkExceptionType, ExceptionStats> exception_stats_;
    mutable std::mutex stats_mutex_;

    // 辅助方法
    RecoveryStrategy DetermineStrategy(NetworkExceptionType type, const ExceptionStats& stats,
                                     std::chrono::milliseconds time_since_last_failure);
    void UpdateExceptionStats(NetworkExceptionType type, bool recovery_success);
    void OpenCircuitBreaker(NetworkExceptionType type);
    void CloseCircuitBreaker(NetworkExceptionType type);
};

} // namespace network
} // namespace sqlcc

#endif // SQLCC_NETWORK_EXCEPTION_HANDLER_H
