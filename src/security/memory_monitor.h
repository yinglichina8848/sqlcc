#ifndef SQLCC_MEMORY_MONITOR_H
#define SQLCC_MEMORY_MONITOR_H

#include <atomic>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>

namespace sqlcc {
namespace security {

/**
 * @class MemoryMonitor
 * @brief 内存安全监控系统 - Phase 4长效机制
 * @details 实时监控内存使用、泄漏检测、性能指标和异常告警
 */
class MemoryMonitor {
public:
    // 内存监控指标
    struct MemoryMetrics {
        size_t current_usage;           // 当前内存使用量
        size_t peak_usage;              // 峰值内存使用量
        size_t allocation_count;        // 分配次数
        size_t deallocation_count;      // 释放次数
        size_t leaked_blocks;           // 泄漏块数
        size_t leaked_bytes;            // 泄漏字节数
        double allocation_rate;         // 分配速率（字节/秒）
        double fragmentation_ratio;     // 内存碎片率
        std::chrono::steady_clock::time_point last_update;
    };

    // 告警级别
    enum class AlertLevel {
        INFO,      // 信息级别
        WARNING,   // 警告级别
        ERROR,     // 错误级别
        CRITICAL   // 严重级别
    };

    // 告警回调函数类型
    using AlertCallback = std::function<void(AlertLevel, const std::string&, const MemoryMetrics&)>;

    // 单例模式获取实例
    static MemoryMonitor& getInstance() {
        static MemoryMonitor instance;
        return instance;
    }

    // 禁止拷贝和移动
    MemoryMonitor(const MemoryMonitor&) = delete;
    MemoryMonitor& operator=(const MemoryMonitor&) = delete;

    /**
     * @brief 启动内存监控
     * @param check_interval 检查间隔（毫秒）
     */
    void startMonitoring(int check_interval = 1000);

    /**
     * @brief 停止内存监控
     */
    void stopMonitoring();

    /**
     * @brief 注册告警回调
     * @param callback 回调函数
     */
    void registerAlertCallback(AlertCallback callback);

    /**
     * @brief 记录内存分配
     * @param size 分配大小
     * @param file 文件名
     * @param line 行号
     */
    void recordAllocation(size_t size, const char* file = "", int line = 0);

    /**
     * @brief 记录内存释放
     * @param size 释放大小
     */
    void recordDeallocation(size_t size);

    /**
     * @brief 获取当前内存指标
     */
    MemoryMetrics getCurrentMetrics();

    /**
     * @brief 设置内存使用阈值
     * @param threshold_bytes 阈值字节数
     */
    void setMemoryThreshold(size_t threshold_bytes);

    /**
     * @brief 设置泄漏检测灵敏度
     * @param sensitivity 灵敏度级别（1-10）
     */
    void setLeakDetectionSensitivity(int sensitivity);

    /**
     * @brief 生成内存安全报告
     */
    std::string generateSecurityReport();

    /**
     * @brief 检查是否存在内存安全问题
     */
    bool hasMemorySafetyIssues();

private:
    MemoryMonitor();
    ~MemoryMonitor();

    // 监控线程函数
    void monitoringThread();
    
    // 更新内存指标
    void updateMetrics();
    
    // 触发告警
    void triggerAlert(AlertLevel level, const std::string& message);
    
    // 检查内存泄漏
    void checkMemoryLeaks();
    
    // 检查性能问题
    void checkPerformanceIssues();

    // 分配信息结构
    struct AllocationInfo {
        size_t size;
        std::string file;
        int line;
        std::chrono::steady_clock::time_point timestamp;
    };

    // 成员变量
    std::atomic<bool> monitoring_active_{false};
    std::thread monitoring_thread_;
    std::mutex metrics_mutex_;
    std::condition_variable stop_condition_;
    
    MemoryMetrics current_metrics_;
    std::vector<AlertCallback> alert_callbacks_;
    
    size_t memory_threshold_{0};
    int leak_sensitivity_{5};
    
    // 分配跟踪
    std::unordered_map<void*, AllocationInfo> active_allocations_;
    std::mutex allocation_mutex_;
};

/**
 * @class MemorySafetyAuditor
 * @brief 内存安全审计器
 * @details 定期执行内存安全审计，生成审计报告
 */
class MemorySafetyAuditor {
public:
    MemorySafetyAuditor();
    ~MemorySafetyAuditor();

    /**
     * @brief 执行全面内存安全审计
     */
    void performComprehensiveAudit();

    /**
     * @brief 生成审计报告
     */
    std::string generateAuditReport() const;

    /**
     * @brief 设置审计频率
     * @param hours 审计间隔（小时）
     */
    void setAuditFrequency(int hours);

private:
    void auditThread();
    
    std::atomic<bool> audit_active_{false};
    std::thread audit_thread_;
    int audit_frequency_hours_{24}; // 默认每天审计一次
};

/**
 * @brief 内存安全工具函数
 */
namespace memory_utils {

/**
 * @brief 安全内存分配（带监控）
 */
template<typename T, typename... Args>
std::unique_ptr<T> make_safe_unique(Args&&... args) {
    auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
    MemoryMonitor::getInstance().recordAllocation(sizeof(T));
    return ptr;
}

template<typename T, typename... Args>
std::shared_ptr<T> make_safe_shared(Args&&... args) {
    auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
    MemoryMonitor::getInstance().recordAllocation(sizeof(T));
    return ptr;
}

/**
 * @brief 内存使用统计
 */
class MemoryUsageTracker {
public:
    MemoryUsageTracker(const std::string& context);
    ~MemoryUsageTracker();
    
    void recordUsageChange(int64_t delta);
    
private:
    std::string context_;
    int64_t start_usage_;
};

} // namespace memory_utils

} // namespace security
} // namespace sqlcc

// 内存监控宏定义
#define SQLCC_MEMORY_ALLOC(size) \
    sqlcc::security::MemoryMonitor::getInstance().recordAllocation(size, __FILE__, __LINE__)

#define SQLCC_MEMORY_FREE(size) \
    sqlcc::security::MemoryMonitor::getInstance().recordDeallocation(size)

#define SQLCC_MEMORY_TRACKER(context) \
    sqlcc::security::memory_utils::MemoryUsageTracker tracker(context)

#endif // SQLCC_MEMORY_MONITOR_H