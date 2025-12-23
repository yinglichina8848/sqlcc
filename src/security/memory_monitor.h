#ifndef SQLCC_SECURITY_MEMORY_MONITOR_H
#define SQLCC_SECURITY_MEMORY_MONITOR_H

#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <functional>

namespace sqlcc {
namespace security {

// 前向声明
class MemorySafetyAuditor;

// 内存告警等级
enum class AlertLevel {
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

// 内存指标结构体
struct MemoryMetrics {
    size_t current_usage = 0;           // 当前内存使用量
    size_t peak_usage = 0;              // 峰值内存使用量
    size_t allocation_count = 0;        // 分配次数
    size_t deallocation_count = 0;      // 释放次数
    size_t leaked_blocks = 0;           // 泄漏块数
    size_t leaked_bytes = 0;            // 泄漏字节数
    double allocation_rate = 0.0;       // 分配速率 (bytes/sec)
    double fragmentation_ratio = 0.0;   // 碎片率
    std::chrono::steady_clock::time_point last_update; // 最后更新时间
};

// 告警回调类型
using AlertCallback = std::function<void(AlertLevel, const std::string&, const MemoryMetrics&)>;

// 内存监控类
class MemoryMonitor {
public:
    static MemoryMonitor& getInstance();

    MemoryMonitor();
    virtual ~MemoryMonitor();

    // 监控控制
    void startMonitoring(int check_interval_ms = 1000);
    void stopMonitoring();

    // 内存跟踪
    void recordAllocation(size_t size, const char* file = nullptr, int line = 0);
    void recordDeallocation(size_t size);

    // 告警配置
    void registerAlertCallback(AlertCallback callback);
    void setMemoryThreshold(size_t threshold_bytes);
    void setLeakDetectionSensitivity(int sensitivity);

    // 监控信息
    MemoryMetrics getCurrentMetrics();
    std::string generateSecurityReport();
    bool hasMemorySafetyIssues();

private:
    MemoryMonitor(const MemoryMonitor&) = delete;
    MemoryMonitor& operator=(const MemoryMonitor&) = delete;

    // 内部实现
    void monitoringThread();
    void updateMetrics();
    void triggerAlert(AlertLevel level, const std::string& message);
    void checkMemoryLeaks();
    void checkPerformanceIssues();

    // 成员变量
    std::atomic<bool> monitoring_active_;
    std::thread monitoring_thread_;
    std::mutex metrics_mutex_;
    std::condition_variable stop_condition_;

    MemoryMetrics current_metrics_;
    size_t memory_threshold_;
    int leak_sensitivity_;

    std::vector<AlertCallback> alert_callbacks_;

    // 分配跟踪（简化实现）
    struct AllocationInfo {
        size_t size;
        std::string file;
        int line;
        std::chrono::steady_clock::time_point timestamp;
    };
    std::mutex allocation_mutex_;
    std::vector<AllocationInfo> active_allocations_;
};

// 内存安全审计器类
class MemorySafetyAuditor {
public:
    MemorySafetyAuditor();
    virtual ~MemorySafetyAuditor();

    void performComprehensiveAudit();
    std::string generateAuditReport() const;

    void setAuditFrequency(int hours);

private:
    void auditThread();

    int audit_frequency_hours_;
    std::atomic<bool> audit_active_;
    std::thread audit_thread_;
};

// 内存使用跟踪器类
namespace memory_utils {

class MemoryUsageTracker {
public:
    MemoryUsageTracker(const std::string& context);
    ~MemoryUsageTracker();

    void recordUsageChange(int64_t delta);

private:
    std::string context_;
    size_t start_usage_;
};

} // namespace memory_utils

} // namespace security
} // namespace sqlcc

#endif // SQLCC_SECURITY_MEMORY_MONITOR_H
