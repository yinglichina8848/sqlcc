/**
 * @file network_exception_handler.cpp
 * @brief 网络异常处理系统实现 - 异常安全保证
 *
 * 该文件实现了网络异常分类、优雅降级和恢复机制，包括：
 * - 网络异常分类和处理
 * - 优雅降级和恢复
 * - 日志记录和监控
 * - 系统稳定性保证
 */

#include <algorithm>
#include <sstream>
#include <iomanip>
#include "network/network_exception.h"
#include "network/network_exception_handler.h"
#include "network/network_monitor.h"
#include "network/network_stability_guard.h"

namespace sqlcc {
namespace network {

// 网络异常实现
NetworkException::NetworkException(NetworkExceptionType type, const std::string& message,
                                 const std::string& details, bool recoverable)
    : std::runtime_error(message), type_(type), details_(details), recoverable_(recoverable) {
}

std::string NetworkException::GetFullMessage() const {
    std::stringstream ss;
    ss << "NetworkException[" << static_cast<int>(type_) << "]: " << what();
    if (!details_.empty()) {
        ss << " (Details: " << details_ << ")";
    }
    ss << " (Recoverable: " << (recoverable_ ? "Yes" : "No") << ")";
    return ss.str();
}

// 网络异常处理器实现
NetworkExceptionHandler::NetworkExceptionHandler() {
    // 初始化所有异常类型的默认统计信息
    for (int i = 0; i <= UNKNOWN_ERROR; ++i) {
        NetworkExceptionType type = static_cast<NetworkExceptionType>(i);
        exception_stats_[type] = ExceptionStats{};
    }
}

NetworkExceptionHandler::RecoveryStrategy NetworkExceptionHandler::HandleException(
    const NetworkException& exception,
    std::chrono::milliseconds time_since_last_failure) {

    std::lock_guard<std::mutex> lock(stats_mutex_);
    NetworkExceptionType type = exception.GetType();

    auto it = exception_stats_.find(type);
    if (it == exception_stats_.end()) {
        return IMMEDIATE_RETRY; // 默认策略
    }

    ExceptionStats& stats = it->second;
    return DetermineStrategy(type, stats, time_since_last_failure);
}

void NetworkExceptionHandler::SetMaxRetries(NetworkExceptionType type, int max_retries) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    exception_stats_[type].max_retries = max_retries;
}

void NetworkExceptionHandler::SetRetryDelay(NetworkExceptionType type, std::chrono::milliseconds delay) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    exception_stats_[type].retry_delay = delay;
}

void NetworkExceptionHandler::SetCircuitBreakerThreshold(NetworkExceptionType type, int threshold) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    exception_stats_[type].circuit_breaker_threshold = threshold;
}

void NetworkExceptionHandler::SetCircuitBreakerTimeout(NetworkExceptionType type, std::chrono::milliseconds timeout) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    exception_stats_[type].circuit_breaker_timeout = timeout;
}

bool NetworkExceptionHandler::IsCircuitBreakerOpen(NetworkExceptionType type) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    auto it = exception_stats_.find(type);
    if (it == exception_stats_.end()) {
        return false;
    }
    return it->second.circuit_breaker_open;
}

std::chrono::milliseconds NetworkExceptionHandler::GetRemainingCircuitBreakerTimeout(NetworkExceptionType type) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    auto it = exception_stats_.find(type);
    if (it == exception_stats_.end() || !it->second.circuit_breaker_open) {
        return std::chrono::milliseconds(0);
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - it->second.circuit_breaker_opened_time);

    if (elapsed >= it->second.circuit_breaker_timeout) {
        return std::chrono::milliseconds(0);
    }

    return it->second.circuit_breaker_timeout - elapsed;
}

int NetworkExceptionHandler::GetExceptionCount(NetworkExceptionType type) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    auto it = exception_stats_.find(type);
    return it != exception_stats_.end() ? it->second.exception_count : 0;
}

int NetworkExceptionHandler::GetRecoveryCount(NetworkExceptionType type) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    auto it = exception_stats_.find(type);
    return it != exception_stats_.end() ? it->second.recovery_count : 0;
}

double NetworkExceptionHandler::GetExceptionRate(NetworkExceptionType type,
                                                std::chrono::milliseconds window) const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    auto it = exception_stats_.find(type);
    if (it == exception_stats_.end()) {
        return 0.0;
    }

    const ExceptionStats& stats = it->second;
    if (stats.exception_count == 0) {
        return 0.0;
    }

    // 计算时间窗口内的异常率
    auto now = std::chrono::steady_clock::now();
    auto cutoff_time = now - window;

    // 如果最后异常时间在窗口之外，返回0
    if (stats.last_exception_time < cutoff_time) {
        return 0.0;
    }

    // 计算异常间隔时间
    auto time_span = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - stats.last_exception_time + window).count();

    if (time_span == 0) {
        return static_cast<double>(stats.exception_count); // 瞬间速率
    }

    return static_cast<double>(stats.exception_count) / (time_span / 1000.0); // 异常/秒
}

void NetworkExceptionHandler::ResetStatistics(NetworkExceptionType type) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    auto it = exception_stats_.find(type);
    if (it != exception_stats_.end()) {
        it->second.exception_count = 0;
        it->second.recovery_count = 0;
        it->second.last_exception_time = std::chrono::steady_clock::time_point{};
        it->second.circuit_breaker_open = false;
    }
}

void NetworkExceptionHandler::ResetAllStatistics() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    for (auto& pair : exception_stats_) {
        pair.second.exception_count = 0;
        pair.second.recovery_count = 0;
        pair.second.last_exception_time = std::chrono::steady_clock::time_point{};
        pair.second.circuit_breaker_open = false;
    }
}

NetworkExceptionHandler::RecoveryStrategy NetworkExceptionHandler::DetermineStrategy(
    NetworkExceptionType type, const ExceptionStats& stats,
    std::chrono::milliseconds time_since_last_failure) {

    // 检查断路器状态
    if (stats.circuit_breaker_open) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - stats.circuit_breaker_opened_time);

        if (elapsed >= stats.circuit_breaker_timeout) {
            // 断路器超时，尝试关闭
            CloseCircuitBreaker(type);
            return IMMEDIATE_RETRY;
        } else {
            // 断路器仍然开启
            return CIRCUIT_BREAKER;
        }
    }

    // 根据异常类型和历史统计确定策略
    switch (type) {
        case CONNECTION_LOST:
        case CONNECTION_TIMEOUT:
            // 连接相关异常：如果重试次数未达到上限，使用延迟重试
            if (stats.exception_count < stats.max_retries) {
                return DELAYED_RETRY;
            } else {
                return GRACEFUL_DEGRADATION;
            }

        case AUTHENTICATION_FAILED:
            // 认证失败：立即重试几次，然后降级
            if (stats.exception_count < 2) {
                return IMMEDIATE_RETRY;
            } else {
                return GRACEFUL_DEGRADATION;
            }

        case PROTOCOL_VIOLATION:
        case DATA_CORRUPTION:
            // 协议或数据错误：不重试，直接降级
            return GRACEFUL_DEGRADATION;

        case RESOURCE_EXHAUSTED:
        case SYSTEM_OVERLOAD:
            // 资源耗尽：延迟重试，如果持续发生则关闭系统
            if (stats.exception_count > stats.circuit_breaker_threshold) {
                OpenCircuitBreaker(type);
                return CIRCUIT_BREAKER;
            } else {
                return DELAYED_RETRY;
            }

        case RATE_LIMIT_EXCEEDED:
            // 速率限制：延迟重试
            return DELAYED_RETRY;

        case NETWORK_UNAVAILABLE:
            // 网络不可用：延迟重试，如果持续则降级
            if (stats.exception_count > stats.max_retries * 2) {
                return GRACEFUL_DEGRADATION;
            } else {
                return DELAYED_RETRY;
            }

        case UNKNOWN_ERROR:
        default:
            // 未知错误：保守策略，延迟重试
            return DELAYED_RETRY;
    }
}

void NetworkExceptionHandler::UpdateExceptionStats(NetworkExceptionType type, bool recovery_success) {
    auto it = exception_stats_.find(type);
    if (it == exception_stats_.end()) {
        return;
    }

    ExceptionStats& stats = it->second;
    stats.exception_count++;
    stats.last_exception_time = std::chrono::steady_clock::now();

    if (recovery_success) {
        stats.recovery_count++;
    }

    // 检查是否需要开启断路器
    if (stats.exception_count >= stats.circuit_breaker_threshold) {
        OpenCircuitBreaker(type);
    }
}

void NetworkExceptionHandler::OpenCircuitBreaker(NetworkExceptionType type) {
    auto it = exception_stats_.find(type);
    if (it != exception_stats_.end()) {
        it->second.circuit_breaker_open = true;
        it->second.circuit_breaker_opened_time = std::chrono::steady_clock::now();
    }
}

void NetworkExceptionHandler::CloseCircuitBreaker(NetworkExceptionType type) {
    auto it = exception_stats_.find(type);
    if (it != exception_stats_.end()) {
        it->second.circuit_breaker_open = false;
        // 重置异常计数以允许新的尝试
        it->second.exception_count = 0;
    }
}

// 网络监控和日志系统实现
NetworkMonitor::NetworkMonitor()
    : log_level_(INFO),
      max_log_entries_(1000),
      performance_monitoring_enabled_(true),
      start_time_(std::chrono::steady_clock::now()),
      active_connections_(0),
      total_messages_sent_(0),
      total_messages_received_(0),
      total_bytes_sent_(0),
      total_bytes_received_(0),
      authentication_successes_(0),
      authentication_failures_(0),
      last_message_time_(std::chrono::steady_clock::now()) {
}

void NetworkMonitor::LogEvent(MonitorLevel level, const std::string& component,
                             const std::string& event, const std::string& details) {
    if (level < log_level_) {
        return;
    }

    LogEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.level = level;
    entry.component = component;
    entry.event = event;
    entry.details = details;

    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        log_entries_.push_back(entry);

        // 清理旧日志
        CleanupOldLogs();
    }
}

void NetworkMonitor::LogException(const NetworkException& exception, const std::string& context) {
    std::stringstream ss;
    ss << "Exception in " << context << ": " << exception.GetFullMessage();
    LogEvent(ERROR, "ExceptionHandler", "NetworkException", ss.str());
}

void NetworkMonitor::LogPerformance(const std::string& metric, double value, const std::string& unit) {
    if (!performance_monitoring_enabled_) {
        return;
    }

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << value << " " << unit;
    LogEvent(DEBUG, "PerformanceMonitor", metric, ss.str());
}

void NetworkMonitor::RecordConnectionEstablished() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    active_connections_++;
    LogEvent(INFO, "ConnectionManager", "ConnectionEstablished",
             "Active connections: " + std::to_string(active_connections_));
}

void NetworkMonitor::RecordConnectionLost() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    if (active_connections_ > 0) {
        active_connections_--;
    }
    LogEvent(WARNING, "ConnectionManager", "ConnectionLost",
             "Active connections: " + std::to_string(active_connections_));
}

void NetworkMonitor::RecordMessageSent(size_t size) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    total_messages_sent_++;
    total_bytes_sent_ += size;
    last_message_time_ = std::chrono::steady_clock::now();
}

void NetworkMonitor::RecordMessageReceived(size_t size) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    total_messages_received_++;
    total_bytes_received_ += size;
    last_message_time_ = std::chrono::steady_clock::now();
}

void NetworkMonitor::RecordAuthenticationSuccess() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    authentication_successes_++;
}

void NetworkMonitor::RecordAuthenticationFailure() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    authentication_failures_++;
    LogEvent(WARNING, "AuthenticationManager", "AuthenticationFailed",
             "Total failures: " + std::to_string(authentication_failures_));
}

void NetworkMonitor::RecordException(NetworkExceptionType type) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    exception_counts_[type]++;

    std::string type_name;
    switch (type) {
        case CONNECTION_LOST: type_name = "ConnectionLost"; break;
        case CONNECTION_TIMEOUT: type_name = "ConnectionTimeout"; break;
        case AUTHENTICATION_FAILED: type_name = "AuthenticationFailed"; break;
        case PROTOCOL_VIOLATION: type_name = "ProtocolViolation"; break;
        case RESOURCE_EXHAUSTED: type_name = "ResourceExhausted"; break;
        case DATA_CORRUPTION: type_name = "DataCorruption"; break;
        case RATE_LIMIT_EXCEEDED: type_name = "RateLimitExceeded"; break;
        case SYSTEM_OVERLOAD: type_name = "SystemOverload"; break;
        case NETWORK_UNAVAILABLE: type_name = "NetworkUnavailable"; break;
        case UNKNOWN_ERROR: type_name = "UnknownError"; break;
        default: type_name = "Unknown"; break;
    }

    LogEvent(ERROR, "ExceptionMonitor", type_name + "Exception",
             "Count: " + std::to_string(exception_counts_[type]));
}

int NetworkMonitor::GetActiveConnections() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return active_connections_;
}

size_t NetworkMonitor::GetTotalMessagesSent() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return total_messages_sent_;
}

size_t NetworkMonitor::GetTotalMessagesReceived() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return total_messages_received_;
}

size_t NetworkMonitor::GetTotalBytesSent() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return total_bytes_sent_;
}

size_t NetworkMonitor::GetTotalBytesReceived() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return total_bytes_received_;
}

double NetworkMonitor::GetUptime() const {
    auto now = std::chrono::steady_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_);
    return static_cast<double>(uptime.count());
}

double NetworkMonitor::GetMessagesPerSecond() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    double uptime = GetUptime();
    if (uptime == 0.0) {
        return 0.0;
    }
    return static_cast<double>(total_messages_sent_ + total_messages_received_) / uptime;
}

double NetworkMonitor::GetBytesPerSecond() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    double uptime = GetUptime();
    if (uptime == 0.0) {
        return 0.0;
    }
    return static_cast<double>(total_bytes_sent_ + total_bytes_received_) / uptime;
}

bool NetworkMonitor::IsSystemHealthy() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return IsWithinHealthThresholds();
}

std::string NetworkMonitor::GetHealthReport() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);

    std::stringstream ss;
    ss << "Network Health Report:\n";
    ss << "- Active Connections: " << active_connections_ << "\n";
    ss << "- Total Messages: " << (total_messages_sent_ + total_messages_received_) << "\n";
    ss << "- Total Bytes: " << (total_bytes_sent_ + total_bytes_received_) << "\n";
    ss << "- Authentication Success Rate: ";
    if (authentication_successes_ + authentication_failures_ > 0) {
        double rate = static_cast<double>(authentication_successes_) /
                     (authentication_successes_ + authentication_failures_) * 100.0;
        ss << std::fixed << std::setprecision(1) << rate << "%\n";
    } else {
        ss << "N/A\n";
    }
    ss << "- Uptime: " << std::fixed << std::setprecision(1) << GetUptime() << "s\n";
    ss << "- Messages/sec: " << std::fixed << std::setprecision(1) << GetMessagesPerSecond() << "\n";
    ss << "- Bytes/sec: " << std::fixed << std::setprecision(1) << GetBytesPerSecond() << "\n";

    std::vector<std::string> alerts = GetActiveAlerts();
    if (!alerts.empty()) {
        ss << "- Active Alerts:\n";
        for (const auto& alert : alerts) {
            ss << "  * " << alert << "\n";
        }
    } else {
        ss << "- Status: Healthy\n";
    }

    return ss.str();
}

std::vector<std::string> NetworkMonitor::GetActiveAlerts() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    std::vector<std::string> alerts;
    CheckHealthAlerts(alerts);
    return alerts;
}

void NetworkMonitor::SetLogLevel(MonitorLevel level) {
    log_level_ = level;
}

void NetworkMonitor::SetMaxLogEntries(size_t max_entries) {
    max_log_entries_ = max_entries;
    CleanupOldLogs();
}

void NetworkMonitor::EnablePerformanceMonitoring(bool enable) {
    performance_monitoring_enabled_ = enable;
}

void NetworkMonitor::CleanupOldLogs() {
    while (log_entries_.size() > max_log_entries_) {
        log_entries_.erase(log_entries_.begin());
    }
}

bool NetworkMonitor::IsWithinHealthThresholds() const {
    // 检查各种健康指标
    if (authentication_failures_ > max_consecutive_failures_) {
        return false;
    }

    // 检查异常率
    double total_exceptions = 0;
    for (const auto& pair : exception_counts_) {
        total_exceptions += pair.second;
    }

    double uptime_minutes = GetUptime() / 60.0;
    if (uptime_minutes > 0) {
        double exception_rate = total_exceptions / uptime_minutes;
        if (exception_rate > max_exception_rate_) {
            return false;
        }
    }

    // 检查吞吐量
    double throughput = GetBytesPerSecond();
    if (throughput < min_throughput_ && GetUptime() > 60.0) { // 运行超过1分钟才检查
        return false;
    }

    return true;
}

std::string NetworkMonitor::FormatLogEntry(const LogEntry& entry) const {
    auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << " [" << entry.level << "] " << entry.component << ": " << entry.event;
    if (!entry.details.empty()) {
        ss << " - " << entry.details;
    }
    return ss.str();
}

void NetworkMonitor::CheckHealthAlerts(std::vector<std::string>& alerts) const {
    if (authentication_failures_ > max_consecutive_failures_) {
        alerts.push_back("High authentication failure rate: " +
                        std::to_string(authentication_failures_) + " failures");
    }

    double total_exceptions = 0;
    for (const auto& pair : exception_counts_) {
        total_exceptions += pair.second;
    }

    double uptime_minutes = GetUptime() / 60.0;
    if (uptime_minutes > 0) {
        double exception_rate = total_exceptions / uptime_minutes;
        if (exception_rate > max_exception_rate_) {
            alerts.push_back("High exception rate: " +
                            std::to_string(exception_rate) + " exceptions/minute");
        }
    }

    double throughput = GetBytesPerSecond();
    if (throughput < min_throughput_ && GetUptime() > 60.0) {
        alerts.push_back("Low throughput: " +
                        std::to_string(throughput) + " bytes/second");
    }

    if (active_connections_ == 0 && GetUptime() > 30.0) {
        alerts.push_back("No active connections for extended period");
    }
}

// 网络稳定性保证器实现
NetworkStabilityGuard::NetworkStabilityGuard()
    : max_connections_(1000),
      max_throughput_(100 * 1024 * 1024), // 100MB/s
      max_exception_rate_(1.0), // 1 exception per minute
      current_connections_(0),
      current_throughput_(0),
      current_exception_rate_(0.0),
      last_action_(NO_ACTION),
      last_assessment_time_(std::chrono::steady_clock::now()) {
}

NetworkStabilityGuard::StabilityAction NetworkStabilityGuard::AssessStability(
    const NetworkMonitor& monitor,
    const NetworkExceptionHandler& exception_handler) {

    std::lock_guard<std::mutex> lock(stability_mutex_);

    // 更新当前状态
    current_connections_ = monitor.GetActiveConnections();
    current_throughput_ = monitor.GetBytesPerSecond();

    // 计算异常率（每分钟异常数）
    double total_exceptions = 0;
    for (int i = 0; i <= UNKNOWN_ERROR; ++i) {
        NetworkExceptionType type = static_cast<NetworkExceptionType>(i);
        total_exceptions += exception_handler.GetExceptionCount(type);
    }

    double uptime_minutes = monitor.GetUptime() / 60.0;
    current_exception_rate_ = uptime_minutes > 0 ? total_exceptions / uptime_minutes : 0.0;

    last_assessment_time_ = std::chrono::steady_clock::now();

    // 计算负载水平 (0-100)
    int load_level = CalculateLoadLevel(monitor);

    // 确定稳定性行动
    StabilityAction action = DetermineAction(load_level, current_exception_rate_, current_throughput_);

    last_action_ = action;
    return action;
}

void NetworkStabilityGuard::SetMaxConnections(int max_connections) {
    std::lock_guard<std::mutex> lock(stability_mutex_);
    max_connections_ = max_connections;
}

void NetworkStabilityGuard::SetMaxThroughput(size_t bytes_per_second) {
    std::lock_guard<std::mutex> lock(stability_mutex_);
    max_throughput_ = bytes_per_second;
}

void NetworkStabilityGuard::SetMaxExceptionRate(double exceptions_per_minute) {
    std::lock_guard<std::mutex> lock(stability_mutex_);
    max_exception_rate_ = exceptions_per_minute;
}

bool NetworkStabilityGuard::ShouldAcceptNewConnection() const {
    std::lock_guard<std::mutex> lock(stability_mutex_);
    return current_connections_ < max_connections_ * connection_throttle_threshold_ / 100;
}

bool NetworkStabilityGuard::ShouldThrottleRequests() const {
    std::lock_guard<std::mutex> lock(stability_mutex_);
    return current_throughput_ > max_throughput_ * 0.9; // 90%阈值
}

std::chrono::milliseconds NetworkStabilityGuard::GetRecommendedDelay() const {
    std::lock_guard<std::mutex> lock(stability_mutex_);

    if (current_connections_ > max_connections_ * 0.8) {
        // 高负载时增加延迟
        return std::chrono::milliseconds(100);
    } else if (current_exception_rate_ > max_exception_rate_ * 0.5) {
        // 高异常率时增加延迟
        return std::chrono::milliseconds(50);
    }

    return std::chrono::milliseconds(0);
}

void NetworkStabilityGuard::AdjustParameters(const NetworkMonitor& monitor) {
    // 自适应调整参数基于当前负载
    int load_level = CalculateLoadLevel(monitor);

    if (load_level > 80) {
        // 高负载：减少最大连接数和吞吐量
        max_connections_ = static_cast<int>(max_connections_ * load_reduction_factor_);
        max_throughput_ = static_cast<size_t>(max_throughput_ * load_reduction_factor_);
    } else if (load_level < 30) {
        // 低负载：可以增加一些限制
        max_connections_ = std::min(10000, static_cast<int>(max_connections_ * 1.1));
        max_throughput_ = std::min(static_cast<size_t>(1000 * 1024 * 1024),
                                  static_cast<size_t>(max_throughput_ * 1.1));
    }
}

void NetworkStabilityGuard::ResetToDefaults() {
    std::lock_guard<std::mutex> lock(stability_mutex_);
    max_connections_ = 1000;
    max_throughput_ = 100 * 1024 * 1024;
    max_exception_rate_ = 1.0;
    last_action_ = NO_ACTION;
}

int NetworkStabilityGuard::GetCurrentLoadLevel() const {
    std::lock_guard<std::mutex> lock(stability_mutex_);
    // 简化的负载计算，不依赖NetworkMonitor
    int connection_load = max_connections_ > 0 ? (current_connections_ * 100) / max_connections_ : 0;
    int throughput_load = max_throughput_ > 0 ? static_cast<int>((current_throughput_ * 100) / max_throughput_) : 0;
    int exception_load = max_exception_rate_ > 0 ? static_cast<int>((current_exception_rate_ * 100) / max_exception_rate_) : 0;
    return std::max({connection_load, throughput_load, exception_load});
}

std::string NetworkStabilityGuard::GetStabilityReport() const {
    std::lock_guard<std::mutex> lock(stability_mutex_);

    std::stringstream ss;
    ss << "Network Stability Report:\n";
    ss << "- Max Connections: " << max_connections_ << "\n";
    ss << "- Current Connections: " << current_connections_ << "\n";
    ss << "- Max Throughput: " << max_throughput_ << " bytes/s\n";
    ss << "- Current Throughput: " << current_throughput_ << " bytes/s\n";
    ss << "- Max Exception Rate: " << max_exception_rate_ << " exceptions/min\n";
    ss << "- Current Exception Rate: " << current_exception_rate_ << " exceptions/min\n";
    ss << "- Load Level: " << GetCurrentLoadLevel() << "%\n";
    ss << "- Last Action: ";

    switch (last_action_) {
        case NO_ACTION: ss << "No Action"; break;
        case REDUCE_LOAD: ss << "Reduce Load"; break;
        case THROTTLE_CONNECTIONS: ss << "Throttle Connections"; break;
        case ENABLE_CIRCUIT_BREAKER: ss << "Enable Circuit Breaker"; break;
        case GRACEFUL_SHUTDOWN: ss << "Graceful Shutdown"; break;
        default: ss << "Unknown"; break;
    }
    ss << "\n";

    return ss.str();
}

int NetworkStabilityGuard::CalculateLoadLevel(const NetworkMonitor& monitor) const {
    // 基于连接数、吞吐量和异常率计算负载水平
    int connection_load = (current_connections_ * 100) / max_connections_;
    int throughput_load = max_throughput_ > 0 ?
        static_cast<int>((current_throughput_ * 100) / max_throughput_) : 0;
    int exception_load = max_exception_rate_ > 0 ?
        static_cast<int>((current_exception_rate_ * 100) / max_exception_rate_) : 0;

    // 返回最大负载值
    return std::max({connection_load, throughput_load, exception_load});
}

double NetworkStabilityGuard::CalculateExceptionRate(const NetworkExceptionHandler& exception_handler) const {
    double total_exceptions = 0;
    for (int i = 0; i <= UNKNOWN_ERROR; ++i) {
        NetworkExceptionType type = static_cast<NetworkExceptionType>(i);
        total_exceptions += exception_handler.GetExceptionCount(type);
    }

    // 计算运行时间（秒）
    auto now = std::chrono::steady_clock::now();
    auto uptime_seconds = std::chrono::duration_cast<std::chrono::seconds>(now - last_assessment_time_).count();

    if (uptime_seconds == 0) {
        return 0.0;
    }

    return total_exceptions / (uptime_seconds / 60.0); // 每分钟异常数
}

NetworkStabilityGuard::StabilityAction NetworkStabilityGuard::DetermineAction(
    int load_level, double exception_rate, size_t throughput) const {

    // 紧急情况：立即关闭系统
    if (load_level >= 95 || exception_rate >= max_exception_rate_ * 2) {
        return GRACEFUL_SHUTDOWN;
    }

    // 高负载：启用断路器
    if (load_level >= 90 || exception_rate >= max_exception_rate_) {
        return ENABLE_CIRCUIT_BREAKER;
    }

    // 中等负载：限制连接
    if (load_level >= 80) {
        return THROTTLE_CONNECTIONS;
    }

    // 轻微负载：减少负载
    if (load_level >= 70 || throughput >= max_throughput_ * 0.9) {
        return REDUCE_LOAD;
    }

    // 正常负载
    return NO_ACTION;
}

} // namespace network
} // namespace sqlcc