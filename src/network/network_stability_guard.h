#ifndef SQLCC_NETWORK_STABILITY_GUARD_H
#define SQLCC_NETWORK_STABILITY_GUARD_H

#include <chrono>
#include <mutex>
#include <string>
#include "network_monitor.h"
#include "network_exception_handler.h"

namespace sqlcc {
namespace network {

/**
 * @brief 网络稳定性行动枚举
 */
enum StabilityAction {
    NO_ACTION = 0,              // 无需行动
    REDUCE_LOAD = 1,            // 减少负载
    THROTTLE_CONNECTIONS = 2,   // 限制连接
    ENABLE_CIRCUIT_BREAKER = 3, // 启用断路器
    GRACEFUL_SHUTDOWN = 4       // 优雅关闭
};

/**
 * @brief 网络稳定性守护器
 *
 * 该类负责监控网络系统的稳定性，并在检测到问题时采取相应的保护措施。
 */
class NetworkStabilityGuard {
public:
    NetworkStabilityGuard();
    virtual ~NetworkStabilityGuard() = default;

    // 稳定性评估
    StabilityAction AssessStability(const NetworkMonitor& monitor,
                                   const NetworkExceptionHandler& exception_handler);

    // 参数配置
    void SetMaxConnections(int max_connections);
    void SetMaxThroughput(size_t bytes_per_second);
    void SetMaxExceptionRate(double exceptions_per_minute);

    // 决策查询
    bool ShouldAcceptNewConnection() const;
    bool ShouldThrottleRequests() const;
    std::chrono::milliseconds GetRecommendedDelay() const;

    // 参数调整
    void AdjustParameters(const NetworkMonitor& monitor);
    void ResetToDefaults();

    // 状态查询
    int GetCurrentLoadLevel() const;
    std::string GetStabilityReport() const;

private:
    // 内部方法
    int CalculateLoadLevel(const NetworkMonitor& monitor) const;
    double CalculateExceptionRate(const NetworkExceptionHandler& exception_handler) const;
    StabilityAction DetermineAction(int load_level, double exception_rate, size_t throughput) const;

    // 成员变量
    int max_connections_;
    size_t max_throughput_;
    double max_exception_rate_;
    int current_connections_;
    size_t current_throughput_;
    double current_exception_rate_;
    StabilityAction last_action_;
    std::chrono::steady_clock::time_point last_assessment_time_;

    // 自适应参数
    double connection_throttle_threshold_;  // 连接限制阈值 (百分比)
    double load_reduction_factor_;          // 负载减少因子

    // 互斥锁保护状态
    mutable std::mutex stability_mutex_;
};

} // namespace network
} // namespace sqlcc

#endif // SQLCC_NETWORK_STABILITY_GUARD_H