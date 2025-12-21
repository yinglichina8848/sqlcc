/**
 * @file network_stability_guard.h
 * @brief 网络稳定性保证器头文件
 */

#ifndef SQLCC_NETWORK_STABILITY_GUARD_H
#define SQLCC_NETWORK_STABILITY_GUARD_H

#include <chrono>
#include <mutex>

#include "network/network_monitor.h"
#include "network/network_exception_handler.h"

namespace sqlcc {
namespace network {

// 网络稳定性保证器 - 系统稳定性保证
class NetworkStabilityGuard {
public:
    NetworkStabilityGuard();
    ~NetworkStabilityGuard() = default;

    // 稳定性策略
    enum StabilityAction {
        NO_ACTION = 0,              // 无操作
        REDUCE_LOAD = 1,           // 减少负载
        THROTTLE_CONNECTIONS = 2,  // 限制连接
        ENABLE_CIRCUIT_BREAKER = 3, // 启用断路器
        GRACEFUL_SHUTDOWN = 4      // 优雅关闭
    };

    // 稳定性评估
    StabilityAction AssessStability(const NetworkMonitor& monitor,
                                  const NetworkExceptionHandler& exception_handler);

    // 负载管理
    void SetMaxConnections(int max_connections);
    void SetMaxThroughput(size_t bytes_per_second);
    void SetMaxExceptionRate(double exceptions_per_minute);

    // 稳定性控制
    bool ShouldAcceptNewConnection() const;
    bool ShouldThrottleRequests() const;
    std::chrono::milliseconds GetRecommendedDelay() const;

    // 自适应调整
    void AdjustParameters(const NetworkMonitor& monitor);
    void ResetToDefaults();

    // 统计信息
    int GetCurrentLoadLevel() const; // 0-100
    std::string GetStabilityReport() const;

private:
    // 配置参数
    int max_connections_;
    size_t max_throughput_;
    double max_exception_rate_;

    // 当前状态
    mutable std::mutex stability_mutex_;
    int current_connections_;
    size_t current_throughput_;
    double current_exception_rate_;
    StabilityAction last_action_;
    std::chrono::steady_clock::time_point last_assessment_time_;

    // 自适应参数
    double load_reduction_factor_ = 0.8;
    int connection_throttle_threshold_ = 80; // 80% of max
    double exception_rate_threshold_ = 0.05; // 5% per minute

    // 辅助方法
    int CalculateLoadLevel(const NetworkMonitor& monitor) const;
    double CalculateExceptionRate(const NetworkExceptionHandler& exception_handler) const;
    StabilityAction DetermineAction(int load_level, double exception_rate, size_t throughput) const;
};

} // namespace network
} // namespace sqlcc

#endif // SQLCC_NETWORK_STABILITY_GUARD_H
