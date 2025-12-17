/**
 * @file disk_error_handler.h
 * @brief 磁盘I/O错误处理器头文件 - 改进的磁盘I/O错误处理机制
 */

#ifndef SQLCC_DISK_ERROR_HANDLER_H
#define SQLCC_DISK_ERROR_HANDLER_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <atomic>
#include <chrono>
#include <vector>
#include <queue>
#include <functional>
#include <system_error>

// 包含头文件
#include "../disk_manager.h"

namespace sqlcc {

// 磁盘I/O错误类型枚举
enum DiskErrorType {
    NO_ERROR = 0,                  // 无错误
    READ_ERROR = 1,               // 读取错误
    WRITE_ERROR = 2,              // 写入错误
    SEEK_ERROR = 3,               // 定位错误
    FILE_CORRUPTION = 4,          // 文件损坏
    DISK_FULL = 5,                // 磁盘空间不足
    PERMISSION_DENIED = 6,        // 权限不足
    DEVICE_UNAVAILABLE = 7,       // 设备不可用
    TIMEOUT_ERROR = 8,            // 超时错误
    CHECKSUM_MISMATCH = 9,        // 校验和不匹配
    UNKNOWN_ERROR = 10            // 未知错误
};

// 错误恢复策略枚举
enum RecoveryStrategy {
    RETRY_IMMEDIATE = 0,          // 立即重试
    RETRY_DELAYED = 1,            // 延迟重试
    USE_BACKUP = 2,              // 使用备份
    MARK_CORRUPTED = 3,           // 标记为损坏
    FAILOVER = 4,                // 故障转移
    ABORT_OPERATION = 5          // 中止操作
};

// 磁盘健康状态枚举
enum DiskHealthStatus {
    HEALTHY = 0,                  // 健康
    DEGRADED = 1,                // 降级
    CRITICAL = 2,                // 严重
    FAILED = 3                   // 失败
};

// 磁盘I/O错误信息结构
struct DiskErrorInfo {
    DiskErrorType error_type;
    int32_t page_id;
    std::string error_message;
    std::chrono::steady_clock::time_point error_time;
    std::error_code system_error;
    std::string context_info;

    DiskErrorInfo(DiskErrorType type, int32_t pid, const std::string& msg,
                  const std::string& context = "")
        : error_type(type), page_id(pid), error_message(msg),
          error_time(std::chrono::steady_clock::now()),
          context_info(context) {}
};

// 磁盘健康统计信息
struct DiskHealthStats {
    size_t total_operations = 0;
    size_t successful_operations = 0;
    size_t failed_operations = 0;
    size_t read_errors = 0;
    size_t write_errors = 0;
    size_t corruption_errors = 0;
    double error_rate = 0.0;      // 错误率（过去1小时）
    std::chrono::steady_clock::time_point last_error_time;
    DiskHealthStatus current_status = HEALTHY;
};

// 页面校验和信息
struct PageChecksum {
    int32_t page_id;
    uint64_t checksum;
    std::chrono::steady_clock::time_point last_verified;

    // 默认构造函数
<<<<<<< Updated upstream
    PageChecksum()
        : page_id(-1), checksum(0),
          last_verified(std::chrono::steady_clock::now()) {}
=======
    PageChecksum() : page_id(0), checksum(0), last_verified(std::chrono::steady_clock::now()) {}
>>>>>>> Stashed changes

    PageChecksum(int32_t pid, uint64_t cs)
        : page_id(pid), checksum(cs),
          last_verified(std::chrono::steady_clock::now()) {}
};

// 磁盘I/O错误处理器
class DiskErrorHandler {
public:
    DiskErrorHandler(std::shared_ptr<DiskManager> disk_manager);
    ~DiskErrorHandler() = default;

    // 错误处理和恢复
    RecoveryStrategy HandleDiskError(const DiskErrorInfo& error_info);
    bool AttemptRecovery(int32_t page_id, RecoveryStrategy strategy);
    bool ValidateDataIntegrity(int32_t page_id, const char* data, size_t size);

    // 校验和管理
    bool ComputeAndStoreChecksum(int32_t page_id, const char* data, size_t size);
    bool VerifyChecksum(int32_t page_id, const char* data, size_t size);
    bool UpdateChecksum(int32_t page_id, const char* data, size_t size);

    // 磁盘健康监控
    DiskHealthStatus AssessDiskHealth();
    DiskHealthStats GetHealthStats() const;
    bool IsDiskHealthy() const;

    // 错误统计和报告
    void RecordError(const DiskErrorInfo& error_info);
    std::vector<DiskErrorInfo> GetRecentErrors(std::chrono::milliseconds time_window) const;
    std::unordered_map<DiskErrorType, size_t> GetErrorCounts() const;

    // 自动恢复配置
    void SetMaxRetries(size_t max_retries);
    void SetRetryDelay(std::chrono::milliseconds delay);
    void SetHealthCheckInterval(std::chrono::milliseconds interval);
    void EnableChecksumValidation(bool enable);
    void EnableAutoRecovery(bool enable);

    // 回调函数类型
    using ErrorCallback = std::function<void(const DiskErrorInfo&)>;
    using RecoveryCallback = std::function<void(int32_t page_id, RecoveryStrategy, bool success)>;
    using HealthChangeCallback = std::function<void(DiskHealthStatus old_status, DiskHealthStatus new_status)>;

    void SetErrorCallback(ErrorCallback callback);
    void SetRecoveryCallback(RecoveryCallback callback);
    void SetHealthChangeCallback(HealthChangeCallback callback);

private:
    // 核心组件
    std::shared_ptr<DiskManager> disk_manager_;

    // 配置参数
    size_t max_retries_ = 3;
    std::chrono::milliseconds retry_delay_ = std::chrono::milliseconds(100);
    std::chrono::milliseconds health_check_interval_ = std::chrono::minutes(5);
    bool checksum_validation_enabled_ = true;
    bool auto_recovery_enabled_ = true;

    // 状态数据
    std::unordered_map<int32_t, PageChecksum> page_checksums_;
    std::vector<DiskErrorInfo> error_history_;
    DiskHealthStats health_stats_;

    // 线程安全
    mutable std::mutex handler_mutex_;
    mutable std::mutex checksum_mutex_;

    // 回调函数
    ErrorCallback error_callback_;
    RecoveryCallback recovery_callback_;
    HealthChangeCallback health_callback_;

    // 辅助方法
    uint64_t ComputeChecksum(const char* data, size_t size) const;
    RecoveryStrategy DetermineRecoveryStrategy(const DiskErrorInfo& error_info) const;
    bool ExecuteRecoveryAction(int32_t page_id, RecoveryStrategy strategy);
    void UpdateHealthStatus();
    bool IsRecoverableError(DiskErrorType error_type) const;
    std::chrono::milliseconds CalculateRetryDelay(size_t attempt_count) const;

    // 错误分类和处理
    DiskErrorType ClassifySystemError(const std::error_code& ec) const;
    std::string FormatErrorMessage(const DiskErrorInfo& error_info) const;
    void LogError(const DiskErrorInfo& error_info);
    void AlertOnCriticalError(const DiskErrorInfo& error_info);
};

// 磁盘冗余管理器 - 支持数据冗余和备份
class DiskRedundancyManager {
public:
    DiskRedundancyManager(std::shared_ptr<DiskManager> primary_disk,
                         std::vector<std::shared_ptr<DiskManager>> backup_disks);
    ~DiskRedundancyManager() = default;

    // 冗余写入
    bool WriteWithRedundancy(int32_t page_id, const char* data);
    bool ReadWithRedundancy(int32_t page_id, char* data);

    // 故障检测和恢复
    bool DetectAndRecoverFromFailure(int32_t page_id);
    std::vector<int32_t> GetFailedDisks() const;

    // 冗余配置
    void SetRedundancyLevel(size_t level); // 1=无冗余, 2=双副本, 3=三副本
    void EnableAutomaticFailover(bool enable);

    // 统计信息
    struct RedundancyStats {
        size_t total_writes = 0;
        size_t successful_writes = 0;
        size_t failed_writes = 0;
        size_t failover_events = 0;
        size_t recovered_pages = 0;
        double redundancy_efficiency = 0.0;
    };

    RedundancyStats GetRedundancyStats() const;

private:
    std::shared_ptr<DiskManager> primary_disk_;
    std::vector<std::shared_ptr<DiskManager>> backup_disks_;
    size_t redundancy_level_ = 1;
    bool automatic_failover_enabled_ = true;

    std::unordered_set<size_t> failed_disk_indices_;
    mutable std::mutex redundancy_mutex_;
};

// 磁盘空间管理器 - 智能磁盘空间管理
class DiskSpaceManager {
public:
    DiskSpaceManager(std::shared_ptr<DiskManager> disk_manager);
    ~DiskSpaceManager() = default;

    // 空间监控
    bool CheckAvailableSpace(size_t required_bytes) const;
    size_t GetAvailableSpace() const;
    size_t GetTotalSpace() const;
    double GetSpaceUtilization() const;

    // 空间清理
    bool PerformSpaceCleanup();
    size_t ReclaimSpace(size_t target_bytes);

    // 空间预分配
    bool PreallocateSpace(size_t bytes);
    bool ShrinkFile(size_t target_size);

    // 空间预警
    void SetSpaceThresholds(double warning_threshold, double critical_threshold);
    bool IsSpaceLow() const;
    bool IsSpaceCritical() const;

    // 统计信息
    struct SpaceStats {
        size_t total_space = 0;
        size_t used_space = 0;
        size_t available_space = 0;
        size_t fragmented_space = 0;
        double utilization_percent = 0.0;
        std::chrono::steady_clock::time_point last_cleanup;
    };

    SpaceStats GetSpaceStats() const;

private:
    std::shared_ptr<DiskManager> disk_manager_;
    double warning_threshold_ = 0.8;   // 80% 使用率警告
    double critical_threshold_ = 0.95; // 95% 使用率临界

    mutable std::mutex space_mutex_;
};

// 磁盘I/O监控器 - 性能监控和异常检测
class DiskIOMonitor {
public:
    DiskIOMonitor(std::shared_ptr<DiskManager> disk_manager);
    ~DiskIOMonitor() = default;

    // 性能监控
    void RecordReadOperation(int32_t page_id, std::chrono::microseconds duration);
    void RecordWriteOperation(int32_t page_id, std::chrono::microseconds duration);
    void RecordSeekOperation(int32_t page_id, std::chrono::microseconds duration);

    // 性能统计
    struct PerformanceStats {
        size_t total_reads = 0;
        size_t total_writes = 0;
        size_t total_seeks = 0;
        double average_read_time_us = 0.0;
        double average_write_time_us = 0.0;
        double average_seek_time_us = 0.0;
        double read_throughput_pages_per_sec = 0.0;
        double write_throughput_pages_per_sec = 0.0;
        std::chrono::steady_clock::time_point monitoring_start;
    };

    PerformanceStats GetPerformanceStats() const;

    // 异常检测
    bool DetectPerformanceAnomaly() const;
    std::vector<std::string> GetPerformanceAlerts() const;

    // 配置
    void SetMonitoringInterval(std::chrono::milliseconds interval);
    void SetPerformanceThresholds(double read_threshold_us, double write_threshold_us);

private:
    std::shared_ptr<DiskManager> disk_manager_;
    std::chrono::milliseconds monitoring_interval_ = std::chrono::seconds(60);
    double read_threshold_us_ = 10000.0;  // 10ms
    double write_threshold_us_ = 50000.0; // 50ms

    // 性能数据
    std::vector<std::chrono::microseconds> read_times_;
    std::vector<std::chrono::microseconds> write_times_;
    std::vector<std::chrono::microseconds> seek_times_;
    std::chrono::steady_clock::time_point monitoring_start_;

    mutable std::mutex monitor_mutex_;
};

// 磁盘I/O错误处理器工厂
class DiskErrorHandlerFactory {
public:
    static std::shared_ptr<DiskErrorHandler> CreateBasicErrorHandler(
        std::shared_ptr<DiskManager> disk_manager);

    static std::shared_ptr<DiskErrorHandler> CreateResilientErrorHandler(
        std::shared_ptr<DiskManager> disk_manager,
        std::vector<std::shared_ptr<DiskManager>> backup_disks);

    static std::shared_ptr<DiskErrorHandler> CreateEnterpriseErrorHandler(
        std::shared_ptr<DiskManager> disk_manager,
        std::vector<std::shared_ptr<DiskManager>> backup_disks,
        std::chrono::milliseconds monitoring_interval);
};

} // namespace sqlcc

#endif // SQLCC_DISK_ERROR_HANDLER_H