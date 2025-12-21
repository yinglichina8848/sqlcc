/**
 * @file network_monitor.h
 * @brief 网络监控和日志系统头文件
 */

#ifndef SQLCC_NETWORK_MONITOR_H
#define SQLCC_NETWORK_MONITOR_H

#include <string>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <mutex>

#include "network/network_exception.h"

namespace sqlcc {
namespace network {

// 网络监控和日志系统 - 日志记录和监控
class NetworkMonitor {
public:
    NetworkMonitor();
    ~NetworkMonitor() = default;

    // 监控级别
    enum MonitorLevel {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3,
        CRITICAL = 4
    };

    // 日志记录
    void LogEvent(MonitorLevel level, const std::string& component,
                 const std::string& event, const std::string& details = "");

    void LogException(const NetworkException& exception, const std::string& context = "");

    void LogPerformance(const std::string& metric, double value,
                       const std::string& unit = "");

    // 监控指标
    void RecordConnectionEstablished();
    void RecordConnectionLost();
    void RecordMessageSent(size_t size);
    void RecordMessageReceived(size_t size);
    void RecordAuthenticationSuccess();
    void RecordAuthenticationFailure();
    void RecordException(NetworkExceptionType type);

    // 统计查询
    int GetActiveConnections() const;
    size_t GetTotalMessagesSent() const;
    size_t GetTotalMessagesReceived() const;
    size_t GetTotalBytesSent() const;
    size_t GetTotalBytesReceived() const;
    double GetUptime() const; // 秒
    double GetMessagesPerSecond() const;
    double GetBytesPerSecond() const;

    // 健康检查
    bool IsSystemHealthy() const;
    std::string GetHealthReport() const;
    std::vector<std::string> GetActiveAlerts() const;

    // 配置
    void SetLogLevel(MonitorLevel level);
    void SetMaxLogEntries(size_t max_entries);
    void EnablePerformanceMonitoring(bool enable);

private:
    struct LogEntry {
        std::chrono::system_clock::time_point timestamp;
        MonitorLevel level;
        std::string component;
        std::string event;
        std::string details;
    };

    MonitorLevel log_level_;
    size_t max_log_entries_;
    bool performance_monitoring_enabled_;
    std::vector<LogEntry> log_entries_;
    std::chrono::steady_clock::time_point start_time_;

    // 统计数据
    mutable std::mutex stats_mutex_;
    int active_connections_;
    size_t total_messages_sent_;
    size_t total_messages_received_;
    size_t total_bytes_sent_;
    size_t total_bytes_received_;
    int authentication_successes_;
    int authentication_failures_;
    std::unordered_map<NetworkExceptionType, int> exception_counts_;
    std::chrono::steady_clock::time_point last_message_time_;

    // 健康检查阈值
    int max_consecutive_failures_ = 10;
    double max_exception_rate_ = 0.1; // 10% per minute
    size_t min_throughput_ = 1000; // 1KB/s minimum

    // 辅助方法
    void CleanupOldLogs();
    bool IsWithinHealthThresholds() const;
    std::string FormatLogEntry(const LogEntry& entry) const;
    void CheckHealthAlerts(std::vector<std::string>& alerts) const;
};

} // namespace network
} // namespace sqlcc

#endif // SQLCC_NETWORK_MONITOR_H
