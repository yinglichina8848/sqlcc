/**
 * WHY: 为什么数据库系统需要磁盘I/O错误处理器？
 *
 * 数据库系统依赖磁盘存储作为最终的数据持久化介质，但磁盘I/O操作面临诸多风险：
 * 1. 硬件故障：磁盘损坏、控制器故障、连接问题
 * 2. 数据损坏：磁盘扇区错误、校验和不匹配、文件系统损坏
 * 3. 系统压力：磁盘空间不足、I/O超时、资源竞争
 * 4. 并发访问：多进程并发访问导致的数据不一致
 * 5. 网络存储：NAS/SAN存储的网络故障和延迟问题
 * 6. 系统崩溃：意外断电或系统崩溃导致的数据丢失
 *
 * 磁盘I/O错误处理器的价值体现在：
 * - 数据安全性：防止数据丢失和损坏的最后防线
 * - 系统可用性：自动检测和恢复磁盘故障，提高系统可用性
 * - 性能监控：实时监控磁盘健康状态和性能指标
 * - 错误恢复：多种错误恢复策略，最大化数据恢复成功率
 * - 用户体验：透明的错误处理，用户感知不到底层故障
 * - 运维效率：自动告警和诊断，减少人工干预需求
 *
 * WHAT: DiskErrorHandler - 磁盘I/O错误处理器
 *
 * 提供企业级数据库系统的完整磁盘I/O错误处理和恢复功能，包括错误检测、分类、恢复、监控等：
 * - 错误检测和分类：自动检测和分类各种磁盘I/O错误
 * - 数据完整性验证：通过校验和验证数据完整性
 * - 自动恢复机制：多种策略的自动错误恢复
 * - 磁盘健康监控：实时监控磁盘健康状态和性能
 * - 冗余存储支持：支持数据冗余和备份恢复
 * - 空间管理优化：智能磁盘空间管理和清理
 * - 性能监控统计：详细的I/O性能统计和分析
 *
 * 核心特性：
 * - 多错误类型支持：支持所有常见磁盘I/O错误类型
 * - 智能恢复策略：基于错误类型的自适应恢复策略
 * - 实时健康监控：持续监控磁盘健康状态
 * - 数据完整性保证：校验和和数据验证机制
 * - 冗余保护：数据冗余和故障转移支持
 * - 性能优化：I/O性能监控和优化建议
 * - 扩展性设计：支持自定义错误处理策略
 *
 * HOW: 磁盘I/O错误处理器的架构和技术实现
 *
 * 1. 错误检测和分类核心架构：
 *    - 错误捕获：拦截所有磁盘I/O系统调用和异常
 *    - 错误分类：根据错误码和上下文信息分类错误类型
 *    - 错误记录：详细记录错误发生的时间、位置和上下文
 *    - 错误统计：按类型和频率统计错误发生情况
 *
 * 2. 数据完整性验证机制：
 *    - 校验和计算：对每个数据页计算CRC32或更强的校验和
 *    - 校验和存储：将校验和与数据页一起存储
 *    - 完整性检查：在读取时验证数据完整性
 *    - 损坏检测：检测数据损坏并触发恢复流程
 *
 * 3. 自动恢复策略框架：
 *    - 重试机制：配置化的重试次数和延迟策略
 *    - 备份恢复：从冗余备份中恢复数据
 *    - 故障转移：切换到备用存储设备
 *    - 数据重建：通过冗余信息重建丢失数据
 *
 * 4. 磁盘健康监控系统：
 *    - 健康评估：基于错误率和性能指标评估磁盘健康
 *    - 趋势分析：分析磁盘性能和错误率的长期趋势
 *    - 预警机制：基于阈值的自动预警和告警
 *    - 预测维护：预测磁盘故障并建议预防性维护
 *
 * 5. 冗余存储管理：
 *    - 镜像存储：实时同步到多个存储设备
 *    - RAID支持：硬件和软件RAID配置管理
 *    - 备份策略：定期备份和增量备份管理
 *    - 恢复流程：数据丢失后的自动恢复流程
 *
 * 6. 空间管理优化：
 *    - 空间监控：实时监控磁盘空间使用情况
 *    - 自动清理：触发垃圾回收和空间整理
 *    - 空间预分配：预分配空间避免动态扩展开销
 *    - 碎片整理：优化文件布局减少碎片
 *
 * 7. 性能监控和分析：
 *    - I/O统计：读取/写入操作的次数和耗时统计
 *    - 性能指标：IOPS、吞吐量、延迟等关键指标
 *    - 异常检测：检测性能异常和潜在问题
 *    - 优化建议：基于监控数据提供优化建议
 *
 * 8. 并发安全和性能优化：
 *    - 线程安全：所有操作都是线程安全的
 *    - 异步处理：非阻塞的错误处理和恢复
 *    - 缓存优化：智能缓存减少重复I/O操作
 *    - 资源管理：高效的内存和资源使用
 *
 * 🏗️ 设计模式：策略模式 + 观察者模式 + 状态模式
 *
 * 策略模式应用：
 * - 错误恢复策略：不同错误的恢复策略
 * - 健康评估策略：不同的健康评估算法
 * - 空间管理策略：不同的空间清理策略
 * - 冗余策略：不同的数据冗余方案
 *
 * 观察者模式应用：
 * - 错误事件通知：错误发生时的观察者通知
 * - 健康状态变化：磁盘健康状态变化的通知
 * - 性能指标监控：性能指标变化的监听
 * - 空间使用告警：空间使用阈值触发的告警
 *
 * 状态模式应用：
 * - 磁盘状态管理：磁盘从健康到故障的状态转换
 * - 恢复状态跟踪：错误恢复过程的状态管理
 * - 冗余状态控制：冗余系统的主备状态切换
 * - 监控状态流转：监控系统的工作状态转换
 *
 * SOLID原则体现：
 *
 * 1. 单一职责原则(SRP)：
 *    - DiskErrorHandler只负责错误处理和恢复
 *    - DiskRedundancyManager专门管理数据冗余
 *    - DiskSpaceManager专注磁盘空间管理
 *    - DiskIOMonitor负责性能监控
 *    - 职责分离清晰，功能专注
 *
 * 2. 开闭原则(OCP)：
 *    - 支持新的错误类型和处理策略扩展
 *    - 可以通过继承添加新的恢复机制
 *    - 监控指标可以独立扩展和定制
 *    - 对扩展开放，对修改关闭
 *
 * 3. 里氏替换原则(LSP)：
 *    - 任何错误处理器实现都可以替代接口使用
 *    - 保证接口契约的一致性和行为正确性
 *    - 子类可以完全替代父类的使用场景
 *
 * 4. 接口隔离原则(ISP)：
 *    - 提供简洁的错误处理接口集合
 *    - 避免客户端依赖不需要的错误处理功能
 *    - 按需暴露错误处理的各个方面
 *
 * 5. 依赖倒置原则(DIP)：
 *    - 错误处理器依赖抽象的存储接口
 *    - 不依赖具体的磁盘管理器实现细节
 *    - 通过依赖注入提高系统的可测试性
 *
 * 磁盘I/O错误处理器的性能优化：
 * - 错误处理开销最小化：快速错误检测和分类
 * - 缓存校验和：避免重复计算校验和
 * - 批量I/O操作：合并多个小I/O为大批量操作
 * - 异步恢复：非阻塞的错误恢复流程
 * - 智能重试：基于错误类型的自适应重试策略
 * - 预读优化：基于访问模式的预读优化
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
    PageChecksum() : page_id(0), checksum(0), last_verified(std::chrono::steady_clock::now()) {}

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
