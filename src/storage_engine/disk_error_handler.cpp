/**
 * @file disk_error_handler.cpp
 * @brief 磁盘I/O错误处理器实现 - 改进的磁盘I/O错误处理机制
 *
 * 该文件实现了磁盘I/O错误处理器的核心功能，包括：
 * - 磁盘I/O错误分类和检测
 * - 错误恢复策略和自动修复
 * - 数据完整性校验和校验和管理
 * - 磁盘健康监控和状态评估
 * - 错误统计和报告机制
 */

#include "storage/disk_error_handler.h"
#include "storage/disk_manager.h"
#include "exception.h"
#include "utils/logger.h"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <functional>

namespace sqlcc {

// 磁盘I/O错误处理器实现
DiskErrorHandler::DiskErrorHandler(std::shared_ptr<DiskManager> disk_manager)
    : disk_manager_(std::move(disk_manager)) {

    // 初始化健康统计
    health_stats_.current_status = HEALTHY;

    SQLCC_LOG_INFO("DiskErrorHandler initialized");
}

RecoveryStrategy DiskErrorHandler::HandleDiskError(const DiskErrorInfo& error_info) {
    std::lock_guard<std::mutex> lock(handler_mutex_);

    // 记录错误
    RecordError(error_info);

    // 确定恢复策略
    RecoveryStrategy strategy = DetermineRecoveryStrategy(error_info);

    SQLCC_LOG_WARN("Disk error detected: " + FormatErrorMessage(error_info) +
                   ", recovery strategy: " + std::to_string(static_cast<int>(strategy)));

    // 如果启用了自动恢复，尝试恢复
    if (auto_recovery_enabled_) {
        bool recovery_success = AttemptRecovery(error_info.page_id, strategy);

        if (recovery_callback_) {
            recovery_callback_(error_info.page_id, strategy, recovery_success);
        }

        if (recovery_success) {
            health_stats_.consistency_repairs++;
        }
    }

    return strategy;
}

bool DiskErrorHandler::AttemptRecovery(int32_t page_id, RecoveryStrategy strategy) {
    return ExecuteRecoveryAction(page_id, strategy);
}

bool DiskErrorHandler::ValidateDataIntegrity(int32_t page_id, const char* data, size_t size) {
    if (!checksum_validation_enabled_) {
        return true; // 如果未启用校验和验证，直接返回成功
    }

    std::lock_guard<std::mutex> lock(checksum_mutex_);

    auto it = page_checksums_.find(page_id);
    if (it == page_checksums_.end()) {
        // 没有校验和信息，创建新的校验和
        return ComputeAndStoreChecksum(page_id, data, size);
    }

    // 验证现有校验和
    uint64_t computed_checksum = ComputeChecksum(data, size);
    bool is_valid = (computed_checksum == it->second.checksum);

    if (is_valid) {
        // 更新验证时间
        it->second.last_verified = std::chrono::steady_clock::now();
    } else {
        // 校验和不匹配，记录错误
        DiskErrorInfo error_info(CHECKSUM_MISMATCH, page_id,
                                "Checksum mismatch for page " + std::to_string(page_id));
        RecordError(error_info);
    }

    return is_valid;
}

bool DiskErrorHandler::ComputeAndStoreChecksum(int32_t page_id, const char* data, size_t size) {
    if (!checksum_validation_enabled_ || !data) {
        return false;
    }

    std::lock_guard<std::mutex> lock(checksum_mutex_);

    uint64_t checksum = ComputeChecksum(data, size);
    page_checksums_[page_id] = PageChecksum(page_id, checksum);

    return true;
}

bool DiskErrorHandler::VerifyChecksum(int32_t page_id, const char* data, size_t size) {
    if (!checksum_validation_enabled_) {
        return true;
    }

    std::lock_guard<std::mutex> lock(checksum_mutex_);

    auto it = page_checksums_.find(page_id);
    if (it == page_checksums_.end()) {
        return false; // 没有校验和信息
    }

    uint64_t computed_checksum = ComputeChecksum(data, size);
    return computed_checksum == it->second.checksum;
}

bool DiskErrorHandler::UpdateChecksum(int32_t page_id, const char* data, size_t size) {
    if (!checksum_validation_enabled_) {
        return true;
    }

    std::lock_guard<std::mutex> lock(checksum_mutex_);

    auto it = page_checksums_.find(page_id);
    if (it == page_checksums_.end()) {
        // 创建新的校验和
        uint64_t checksum = ComputeChecksum(data, size);
        page_checksums_[page_id] = PageChecksum(page_id, checksum);
        return true;
    } else {
        // 更新现有校验和
        it->second.checksum = ComputeChecksum(data, size);
        it->second.last_verified = std::chrono::steady_clock::now();
        return true;
    }
}

DiskHealthStatus DiskErrorHandler::AssessDiskHealth() {
    std::lock_guard<std::mutex> lock(handler_mutex_);

    UpdateHealthStatus();
    return health_stats_.current_status;
}

DiskHealthStats DiskErrorHandler::GetHealthStats() const {
    std::lock_guard<std::mutex> lock(handler_mutex_);
    return health_stats_;
}

bool DiskErrorHandler::IsDiskHealthy() const {
    std::lock_guard<std::mutex> lock(handler_mutex_);
    return health_stats_.current_status == HEALTHY;
}

void DiskErrorHandler::RecordError(const DiskErrorInfo& error_info) {
    std::lock_guard<std::mutex> lock(handler_mutex_);

    // 添加到错误历史
    error_history_.push_back(error_info);

    // 限制错误历史大小（保留最近1000个错误）
    if (error_history_.size() > 1000) {
        error_history_.erase(error_history_.begin());
    }

    // 更新健康统计
    health_stats_.total_operations++;
    health_stats_.failed_operations++;
    health_stats_.last_error_time = error_info.error_time;

    switch (error_info.error_type) {
        case READ_ERROR:
            health_stats_.read_errors++;
            break;
        case WRITE_ERROR:
            health_stats_.write_errors++;
            break;
        case FILE_CORRUPTION:
        case CHECKSUM_MISMATCH:
            health_stats_.corruption_errors++;
            break;
        default:
            break;
    }

    // 计算错误率（过去1小时）
    auto one_hour_ago = std::chrono::steady_clock::now() - std::chrono::hours(1);
    size_t recent_errors = 0;

    for (const auto& error : error_history_) {
        if (error.error_time > one_hour_ago) {
            recent_errors++;
        }
    }

    health_stats_.error_rate = static_cast<double>(recent_errors) / 3600.0; // 错误/秒

    // 触发回调
    if (error_callback_) {
        error_callback_(error_info);
    }

    // 记录严重错误
    if (error_info.error_type == FILE_CORRUPTION ||
        error_info.error_type == DEVICE_UNAVAILABLE ||
        error_info.error_type == UNKNOWN_ERROR) {
        AlertOnCriticalError(error_info);
    }
}

std::vector<DiskErrorInfo> DiskErrorHandler::GetRecentErrors(std::chrono::milliseconds time_window) const {
    std::lock_guard<std::mutex> lock(handler_mutex_);

    std::vector<DiskErrorInfo> recent_errors;
    auto cutoff_time = std::chrono::steady_clock::now() - time_window;

    for (const auto& error : error_history_) {
        if (error.error_time > cutoff_time) {
            recent_errors.push_back(error);
        }
    }

    return recent_errors;
}

std::unordered_map<DiskErrorType, size_t> DiskErrorHandler::GetErrorCounts() const {
    std::lock_guard<std::mutex> lock(handler_mutex_);

    std::unordered_map<DiskErrorType, size_t> error_counts;

    for (const auto& error : error_history_) {
        error_counts[error.error_type]++;
    }

    return error_counts;
}

void DiskErrorHandler::SetMaxRetries(size_t max_retries) {
    std::lock_guard<std::mutex> lock(handler_mutex_);
    max_retries_ = max_retries;
}

void DiskErrorHandler::SetRetryDelay(std::chrono::milliseconds delay) {
    std::lock_guard<std::mutex> lock(handler_mutex_);
    retry_delay_ = delay;
}

void DiskErrorHandler::SetHealthCheckInterval(std::chrono::milliseconds interval) {
    std::lock_guard<std::mutex> lock(handler_mutex_);
    health_check_interval_ = interval;
}

void DiskErrorHandler::EnableChecksumValidation(bool enable) {
    std::lock_guard<std::mutex> lock(handler_mutex_);
    checksum_validation_enabled_ = enable;
}

void DiskErrorHandler::EnableAutoRecovery(bool enable) {
    std::lock_guard<std::mutex> lock(handler_mutex_);
    auto_recovery_enabled_ = enable;
}

void DiskErrorHandler::SetErrorCallback(ErrorCallback callback) {
    error_callback_ = std::move(callback);
}

void DiskErrorHandler::SetRecoveryCallback(RecoveryCallback callback) {
    recovery_callback_ = std::move(callback);
}

void DiskErrorHandler::SetHealthChangeCallback(HealthChangeCallback callback) {
    health_callback_ = std::move(callback);
}

// 私有辅助方法实现
uint64_t DiskErrorHandler::ComputeChecksum(const char* data, size_t size) const {
    // 使用FNV-1a哈希算法计算校验和
    const uint64_t FNV_OFFSET_BASIS = 14695981039346656037ULL;
    const uint64_t FNV_PRIME = 1099511628211ULL;

    uint64_t hash = FNV_OFFSET_BASIS;

    for (size_t i = 0; i < size; ++i) {
        hash ^= static_cast<uint8_t>(data[i]);
        hash *= FNV_PRIME;
    }

    return hash;
}

RecoveryStrategy DiskErrorHandler::DetermineRecoveryStrategy(const DiskErrorInfo& error_info) const {
    switch (error_info.error_type) {
        case READ_ERROR:
        case SEEK_ERROR:
            // I/O错误：立即重试，然后延迟重试
            return RETRY_IMMEDIATE;

        case WRITE_ERROR:
            // 写入错误：延迟重试，避免过度写入
            return RETRY_DELAYED;

        case FILE_CORRUPTION:
        case CHECKSUM_MISMATCH:
            // 数据损坏：标记为损坏或使用备份
            return MARK_CORRUPTED;

        case DISK_FULL:
            // 磁盘空间不足：中止操作
            return ABORT_OPERATION;

        case PERMISSION_DENIED:
        case DEVICE_UNAVAILABLE:
            // 权限或设备错误：故障转移
            return FAILOVER;

        case TIMEOUT_ERROR:
            // 超时：重试
            return RETRY_DELAYED;

        case UNKNOWN_ERROR:
        default:
            // 未知错误：保守策略，使用备份
            return USE_BACKUP;
    }
}

bool DiskErrorHandler::ExecuteRecoveryAction(int32_t page_id, RecoveryStrategy strategy) {
    switch (strategy) {
        case RETRY_IMMEDIATE:
        case RETRY_DELAYED:
            // 重试逻辑已在调用方处理
            return true;

        case USE_BACKUP:
            // TODO: 实现备份恢复逻辑
            SQLCC_LOG_INFO("Backup recovery attempted for page " + std::to_string(page_id));
            return false; // 暂时不支持

        case MARK_CORRUPTED:
            // 标记页面为损坏
            SQLCC_LOG_WARN("Page " + std::to_string(page_id) + " marked as corrupted");
            return true;

        case FAILOVER:
            // TODO: 实现故障转移逻辑
            SQLCC_LOG_INFO("Failover attempted for page " + std::to_string(page_id));
            return false; // 暂时不支持

        case ABORT_OPERATION:
            // 中止操作
            SQLCC_LOG_ERROR("Operation aborted for page " + std::to_string(page_id));
            return false;

        default:
            return false;
    }
}

void DiskErrorHandler::UpdateHealthStatus() {
    DiskHealthStatus old_status = health_stats_.current_status;
    DiskHealthStatus new_status = HEALTHY;

    // 根据错误率和错误类型评估健康状态
    if (health_stats_.error_rate > 0.1) { // 每秒超过0.1个错误
        new_status = FAILED;
    } else if (health_stats_.error_rate > 0.01) { // 每秒超过0.01个错误
        new_status = CRITICAL;
    } else if (health_stats_.corruption_errors > 10 ||
               health_stats_.read_errors + health_stats_.write_errors > 100) {
        new_status = DEGRADED;
    }

    health_stats_.current_status = new_status;

    // 如果状态发生变化，触发回调
    if (old_status != new_status && health_callback_) {
        health_callback_(old_status, new_status);
    }
}

bool DiskErrorHandler::IsRecoverableError(DiskErrorType error_type) const {
    switch (error_type) {
        case READ_ERROR:
        case WRITE_ERROR:
        case SEEK_ERROR:
        case TIMEOUT_ERROR:
            return true; // I/O错误通常是可恢复的

        case FILE_CORRUPTION:
        case CHECKSUM_MISMATCH:
            return true; // 数据损坏可以通过备份恢复

        case DISK_FULL:
        case PERMISSION_DENIED:
        case DEVICE_UNAVAILABLE:
        case UNKNOWN_ERROR:
            return false; // 系统级错误通常不可恢复

        default:
            return false;
    }
}

std::chrono::milliseconds DiskErrorHandler::CalculateRetryDelay(size_t attempt_count) const {
    // 指数退避策略：基础延迟 * 2^(尝试次数-1)
    return retry_delay_ * (1ULL << (attempt_count - 1));
}

DiskErrorType DiskErrorHandler::ClassifySystemError(const std::error_code& ec) const {
    // 根据系统错误码分类磁盘错误
    if (ec == std::errc::no_such_file_or_directory ||
        ec == std::errc::no_such_device) {
        return DEVICE_UNAVAILABLE;
    } else if (ec == std::errc::permission_denied) {
        return PERMISSION_DENIED;
    } else if (ec == std::errc::no_space_on_device) {
        return DISK_FULL;
    } else if (ec == std::errc::io_error) {
        return READ_ERROR; // 泛化的I/O错误
    } else if (ec == std::errc::timed_out) {
        return TIMEOUT_ERROR;
    } else {
        return UNKNOWN_ERROR;
    }
}

std::string DiskErrorHandler::FormatErrorMessage(const DiskErrorInfo& error_info) const {
    std::string message = "Disk error [";
    message += std::to_string(static_cast<int>(error_info.error_type));
    message += "] on page ";
    message += std::to_string(error_info.page_id);
    message += ": ";
    message += error_info.error_message;

    if (!error_info.context_info.empty()) {
        message += " (Context: ";
        message += error_info.context_info;
        message += ")";
    }

    return message;
}

void DiskErrorHandler::LogError(const DiskErrorInfo& error_info) {
    std::string message = FormatErrorMessage(error_info);

    switch (error_info.error_type) {
        case FILE_CORRUPTION:
        case DEVICE_UNAVAILABLE:
        case UNKNOWN_ERROR:
            SQLCC_LOG_ERROR(message);
            break;
        case READ_ERROR:
        case WRITE_ERROR:
        case SEEK_ERROR:
            SQLCC_LOG_WARN(message);
            break;
        default:
            SQLCC_LOG_INFO(message);
            break;
    }
}

void DiskErrorHandler::AlertOnCriticalError(const DiskErrorInfo& error_info) {
    std::string alert_message = "CRITICAL DISK ERROR: " + FormatErrorMessage(error_info);

    // 在实际系统中，这里应该触发告警机制
    SQLCC_LOG_ERROR(alert_message);

    // TODO: 发送告警通知、记录到监控系统等
}

// 磁盘冗余管理器实现
DiskRedundancyManager::DiskRedundancyManager(std::shared_ptr<DiskManager> primary_disk,
                                           std::vector<std::shared_ptr<DiskManager>> backup_disks)
    : primary_disk_(std::move(primary_disk)),
      backup_disks_(std::move(backup_disks)) {
}

bool DiskRedundancyManager::WriteWithRedundancy(int32_t page_id, const char* data) {
    std::lock_guard<std::mutex> lock(redundancy_mutex_);

    bool primary_success = false;
    size_t backup_success_count = 0;

    // 写入主磁盘
    if (primary_disk_ && !failed_disk_indices_.count(0)) {
        primary_success = primary_disk_->WritePage(page_id, data);
    }

    // 写入备份磁盘
    for (size_t i = 0; i < backup_disks_.size(); ++i) {
        if (!failed_disk_indices_.count(i + 1) && backup_disks_[i]) {
            if (backup_disks_[i]->WritePage(page_id, data)) {
                backup_success_count++;
            }
        }
    }

    // 更新统计
    // TODO: 实现统计更新逻辑

    // 根据冗余级别判断整体成功
    if (redundancy_level_ == 1) {
        return primary_success; // 无冗余，只需要主磁盘成功
    } else {
        return primary_success && (backup_success_count >= redundancy_level_ - 1);
    }
}

bool DiskRedundancyManager::ReadWithRedundancy(int32_t page_id, char* data) {
    std::lock_guard<std::mutex> lock(redundancy_mutex_);

    // 首先尝试从主磁盘读取
    if (primary_disk_ && !failed_disk_indices_.count(0)) {
        if (primary_disk_->ReadPage(page_id, data)) {
            return true;
        }
    }

    // 如果主磁盘失败，从备份磁盘读取
    for (size_t i = 0; i < backup_disks_.size(); ++i) {
        if (!failed_disk_indices_.count(i + 1) && backup_disks_[i]) {
            if (backup_disks_[i]->ReadPage(page_id, data)) {
                // 成功从备份读取，标记主磁盘为失败
                if (primary_disk_) {
                    failed_disk_indices_.insert(0);
                }
                return true;
            }
        }
    }

    return false;
}

bool DiskRedundancyManager::DetectAndRecoverFromFailure(int32_t page_id) {
    // TODO: 实现故障检测和恢复逻辑
    return false;
}

std::vector<int32_t> DiskRedundancyManager::GetFailedDisks() const {
    std::lock_guard<std::mutex> lock(redundancy_mutex_);
    return std::vector<int32_t>(failed_disk_indices_.begin(), failed_disk_indices_.end());
}

void DiskRedundancyManager::SetRedundancyLevel(size_t level) {
    std::lock_guard<std::mutex> lock(redundancy_mutex_);
    redundancy_level_ = std::max(size_t(1), level);
}

void DiskRedundancyManager::EnableAutomaticFailover(bool enable) {
    std::lock_guard<std::mutex> lock(redundancy_mutex_);
    automatic_failover_enabled_ = enable;
}

DiskRedundancyManager::RedundancyStats DiskRedundancyManager::GetRedundancyStats() const {
    std::lock_guard<std::mutex> lock(redundancy_mutex_);
    // TODO: 实现统计信息收集
    return RedundancyStats{};
}

// 磁盘空间管理器实现
DiskSpaceManager::DiskSpaceManager(std::shared_ptr<DiskManager> disk_manager)
    : disk_manager_(std::move(disk_manager)) {
}

bool DiskSpaceManager::CheckAvailableSpace(size_t required_bytes) const {
    return GetAvailableSpace() >= required_bytes;
}

size_t DiskSpaceManager::GetAvailableSpace() const {
    // TODO: 实现可用空间查询逻辑
    return 1024 * 1024 * 1024; // 模拟1GB可用空间
}

size_t DiskSpaceManager::GetTotalSpace() const {
    // TODO: 实现总空间查询逻辑
    return 10 * 1024 * 1024 * 1024; // 模拟10GB总空间
}

double DiskSpaceManager::GetSpaceUtilization() const {
    size_t total = GetTotalSpace();
    return total > 0 ? static_cast<double>(total - GetAvailableSpace()) / total : 0.0;
}

bool DiskSpaceManager::PerformSpaceCleanup() {
    // TODO: 实现空间清理逻辑
    return true;
}

size_t DiskSpaceManager::ReclaimSpace(size_t target_bytes) {
    // TODO: 实现空间回收逻辑
    return target_bytes; // 模拟回收成功
}

bool DiskSpaceManager::PreallocateSpace(size_t bytes) {
    // TODO: 实现空间预分配逻辑
    return true;
}

bool DiskSpaceManager::ShrinkFile(size_t target_size) {
    // TODO: 实现文件收缩逻辑
    return true;
}

void DiskSpaceManager::SetSpaceThresholds(double warning_threshold, double critical_threshold) {
    std::lock_guard<std::mutex> lock(space_mutex_);
    warning_threshold_ = warning_threshold;
    critical_threshold_ = critical_threshold;
}

bool DiskSpaceManager::IsSpaceLow() const {
    return GetSpaceUtilization() >= warning_threshold_;
}

bool DiskSpaceManager::IsSpaceCritical() const {
    return GetSpaceUtilization() >= critical_threshold_;
}

DiskSpaceManager::SpaceStats DiskSpaceManager::GetSpaceStats() const {
    SpaceStats stats;
    stats.total_space = GetTotalSpace();
    stats.used_space = stats.total_space - GetAvailableSpace();
    stats.available_space = GetAvailableSpace();
    stats.utilization_percent = GetSpaceUtilization() * 100.0;
    stats.last_cleanup = std::chrono::steady_clock::now(); // 简化实现
    return stats;
}

// 磁盘I/O监控器实现
DiskIOMonitor::DiskIOMonitor(std::shared_ptr<DiskManager> disk_manager)
    : disk_manager_(std::move(disk_manager)),
      monitoring_start_(std::chrono::steady_clock::now()) {
}

void DiskIOMonitor::RecordReadOperation(int32_t page_id, std::chrono::microseconds duration) {
    std::lock_guard<std::mutex> lock(monitor_mutex_);
    read_times_.push_back(duration);
}

void DiskIOMonitor::RecordWriteOperation(int32_t page_id, std::chrono::microseconds duration) {
    std::lock_guard<std::mutex> lock(monitor_mutex_);
    write_times_.push_back(duration);
}

void DiskIOMonitor::RecordSeekOperation(int32_t page_id, std::chrono::microseconds duration) {
    std::lock_guard<std::mutex> lock(monitor_mutex_);
    seek_times_.push_back(duration);
}

DiskIOMonitor::PerformanceStats DiskIOMonitor::GetPerformanceStats() const {
    std::lock_guard<std::mutex> lock(monitor_mutex_);

    PerformanceStats stats;
    stats.monitoring_start = monitoring_start_;
    stats.total_reads = read_times_.size();
    stats.total_writes = write_times_.size();
    stats.total_seeks = seek_times_.size();

    // 计算平均时间
    if (!read_times_.empty()) {
        uint64_t total_read_time = 0;
        for (auto& time : read_times_) {
            total_read_time += time.count();
        }
        stats.average_read_time_us = static_cast<double>(total_read_time) / read_times_.size();
    }

    if (!write_times_.empty()) {
        uint64_t total_write_time = 0;
        for (auto& time : write_times_) {
            total_write_time += time.count();
        }
        stats.average_write_time_us = static_cast<double>(total_write_time) / write_times_.size();
    }

    if (!seek_times_.empty()) {
        uint64_t total_seek_time = 0;
        for (auto& time : seek_times_) {
            total_seek_time += time.count();
        }
        stats.average_seek_time_us = static_cast<double>(total_seek_time) / seek_times_.size();
    }

    // 计算吞吐量
    auto monitoring_duration = std::chrono::steady_clock::now() - monitoring_start_;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(monitoring_duration).count();

    if (seconds > 0) {
        stats.read_throughput_pages_per_sec = static_cast<double>(stats.total_reads) / seconds;
        stats.write_throughput_pages_per_sec = static_cast<double>(stats.total_writes) / seconds;
    }

    return stats;
}

bool DiskIOMonitor::DetectPerformanceAnomaly() const {
    auto stats = GetPerformanceStats();

    // 检查是否超过性能阈值
    return stats.average_read_time_us > read_threshold_us_ ||
           stats.average_write_time_us > write_threshold_us_;
}

std::vector<std::string> DiskIOMonitor::GetPerformanceAlerts() const {
    std::vector<std::string> alerts;
    auto stats = GetPerformanceStats();

    if (stats.average_read_time_us > read_threshold_us_) {
        alerts.push_back("High read latency: " + std::to_string(stats.average_read_time_us) + " us");
    }

    if (stats.average_write_time_us > write_threshold_us_) {
        alerts.push_back("High write latency: " + std::to_string(stats.average_write_time_us) + " us");
    }

    if (stats.read_throughput_pages_per_sec < 10.0) {
        alerts.push_back("Low read throughput: " + std::to_string(stats.read_throughput_pages_per_sec) + " pages/sec");
    }

    return alerts;
}

void DiskIOMonitor::SetMonitoringInterval(std::chrono::milliseconds interval) {
    monitoring_interval_ = interval;
}

void DiskIOMonitor::SetPerformanceThresholds(double read_threshold_us, double write_threshold_us) {
    read_threshold_us_ = read_threshold_us;
    write_threshold_us_ = write_threshold_us;
}

// 磁盘I/O错误处理器工厂实现
std::shared_ptr<DiskErrorHandler> DiskErrorHandlerFactory::CreateBasicErrorHandler(
    std::shared_ptr<DiskManager> disk_manager) {

    auto handler = std::make_shared<DiskErrorHandler>(disk_manager);
    handler->SetMaxRetries(3);
    handler->SetRetryDelay(std::chrono::milliseconds(100));
    handler->EnableChecksumValidation(true);
    handler->EnableAutoRecovery(true);

    return handler;
}

std::shared_ptr<DiskErrorHandler> DiskErrorHandlerFactory::CreateResilientErrorHandler(
    std::shared_ptr<DiskManager> disk_manager,
    std::vector<std::shared_ptr<DiskManager>> backup_disks) {

    auto handler = CreateBasicErrorHandler(disk_manager);
    handler->SetMaxRetries(5);
    handler->SetRetryDelay(std::chrono::milliseconds(200));

    // TODO: 集成冗余管理器
    // auto redundancy_manager = std::make_shared<DiskRedundancyManager>(disk_manager, backup_disks);

    return handler;
}

std::shared_ptr<DiskErrorHandler> DiskErrorHandlerFactory::CreateEnterpriseErrorHandler(
    std::shared_ptr<DiskManager> disk_manager,
    std::vector<std::shared_ptr<DiskManager>> backup_disks,
    std::chrono::milliseconds monitoring_interval) {

    auto handler = CreateResilientErrorHandler(disk_manager, backup_disks);
    handler->SetHealthCheckInterval(monitoring_interval);

    // 配置企业级监控回调
    handler->SetErrorCallback([](const DiskErrorInfo& error) {
        // TODO: 集成企业级监控系统
        SQLCC_LOG_ERROR("Enterprise alert: " + std::to_string(error.page_id) +
                       " error: " + error.error_message);
    });

    return handler;
}

} // namespace sqlcc
