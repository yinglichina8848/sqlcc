#ifndef SQLCC_NETWORK_MONITOR_H
#define SQLCC_NETWORK_MONITOR_H

#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <map>
#include "network/network_exception.h"

namespace sqlcc {
namespace network {

/**
 * @brief 监控级别枚举
 */
enum MonitorLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

/**
 * @brief 日志条目结构
 */
struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    MonitorLevel level;
    std::string component;
    std::string event;
    std::string details;
};

/**
 * @brief 网络监控和日志系统
 *
 * 该类提供网络系统的监控、日志记录和健康检查功能。
 */
class NetworkMonitor {
public:
    NetworkMonitor();
    virtual ~NetworkMonitor() = default;

    // 日志记录方法
    void LogEvent(MonitorLevel level, const std::string& component,
                 const std::string& event, const std::string& details = "");

    void LogException(const NetworkException& exception, const std::string& context);

    void LogPerformance(const std::string& metric, double value, const std::string& unit);

    // 连接管理监控
    void RecordConnectionEstablished();
    void RecordConnectionLost();

    // 消息统计
    void RecordMessageSent(size_t size);
    void RecordMessageReceived(size_t size);

    // 认证统计
    void RecordAuthenticationSuccess();
    void RecordAuthenticationFailure();

    // 异常统计
    void RecordException(NetworkExceptionType type);

    // 获取统计信息
    int GetActiveConnections() const;
    size_t GetTotalMessagesSent() const;
    size_t GetTotalMessagesReceived() const;
    size_t GetTotalBytesSent() const;
    size_t GetTotalBytesReceived() const;

    // 性能指标
    double GetUptime() const;
    double GetMessagesPerSecond() const;
    double GetBytesPerSecond() const;

    // 健康检查
    bool IsSystemHealthy() const;
    std::string GetHealthReport() const;
    std::vector<std::string> GetActiveAlerts() const;

    // 配置方法
    void SetLogLevel(MonitorLevel level);
    void SetMaxLogEntries(size_t max_entries);
    void EnablePerformanceMonitoring(bool enable);

private:
    // 内部方法
    void CleanupOldLogs();
    bool IsWithinHealthThresholds() const;
    std::string FormatLogEntry(const LogEntry& entry) const;
    void CheckHealthAlerts(std::vector<std::string>& alerts) const;

    // 成员变量
    MonitorLevel log_level_;
    size_t max_log_entries_;
    bool performance_monitoring_enabled_;
    std::chrono::steady_clock::time_point start_time_;

    // 统计数据（原子操作保证线程安全）
    std::atomic<int> active_connections_;
    std::atomic<size_t> total_messages_sent_;
    std::atomic<size_t> total_messages_received_;
    std::atomic<size_t> total_bytes_sent_;
    std::atomic<size_t> total_bytes_received_;
    std::atomic<size_t> authentication_successes_;
    std::atomic<size_t> authentication_failures_;

    // 异常计数
    std::map<NetworkExceptionType, size_t> exception_counts_;

    // 日志和最后活动时间
    std::vector<LogEntry> log_entries_;
    std::chrono::steady_clock::time_point last_message_time_;

    // 健康阈值
    size_t max_consecutive_failures_ = 10;
    double max_exception_rate_ = 5.0; // 每分钟异常数
    double min_throughput_ = 1000.0;  // 字节/秒

    // 互斥锁保护非原子数据
    mutable std::mutex stats_mutex_;
};

} // namespace network
} // namespace sqlcc

#endif // SQLCC_NETWORK_MONITOR_H