/**
 * @file cache_consistency_manager.h
 * @brief 缓存一致性管理器头文件 - 保证数据一致性和并发安全性
 */

#ifndef SQLCC_CACHE_CONSISTENCY_MANAGER_H
#define SQLCC_CACHE_CONSISTENCY_MANAGER_H

#include <memory>
#include <unordered_map>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <vector>
#include <functional>

namespace sqlcc {

// 前向声明
class BufferPoolSharded;
class Page;

// 页面版本信息
struct PageVersion {
    uint64_t version = 0;                    // 页面版本号
    std::chrono::steady_clock::time_point last_modified; // 最后修改时间
    std::chrono::steady_clock::time_point last_accessed; // 最后访问时间
    bool is_dirty = false;                  // 是否为脏页
    int32_t writer_transaction_id = -1;     // 写入事务ID
    std::vector<int32_t> reader_transaction_ids; // 读取事务ID列表
};

// 一致性检查结果
enum ConsistencyCheckResult {
    CONSISTENT = 0,           // 一致
    VERSION_MISMATCH = 1,     // 版本不匹配
    DIRTY_PAGE_CONFLICT = 2,  // 脏页冲突
    CONCURRENT_MODIFICATION = 3, // 并发修改
    STALE_READ = 4           // 过期读取
};

// 缓存一致性策略
enum CacheConsistencyStrategy {
    STRICT_CONSISTENCY = 0,    // 严格一致性
    EVENTUAL_CONSISTENCY = 1,  // 最终一致性
    CAUSAL_CONSISTENCY = 2,    // 因果一致性
    WEAK_CONSISTENCY = 3       // 弱一致性
};

// 缓存一致性管理器
class CacheConsistencyManager {
public:
    CacheConsistencyManager(std::shared_ptr<BufferPoolSharded> buffer_pool,
                           CacheConsistencyStrategy strategy = STRICT_CONSISTENCY);
    ~CacheConsistencyManager() = default;

    // 页面访问控制
    ConsistencyCheckResult CheckReadConsistency(int32_t page_id, int32_t transaction_id);
    ConsistencyCheckResult CheckWriteConsistency(int32_t page_id, int32_t transaction_id);
    bool AcquireReadLock(int32_t page_id, int32_t transaction_id, std::chrono::milliseconds timeout);
    bool AcquireWriteLock(int32_t page_id, int32_t transaction_id, std::chrono::milliseconds timeout);
    void ReleaseReadLock(int32_t page_id, int32_t transaction_id);
    void ReleaseWriteLock(int32_t page_id, int32_t transaction_id);

    // 页面版本管理
    uint64_t GetPageVersion(int32_t page_id) const;
    bool UpdatePageVersion(int32_t page_id, int32_t transaction_id);
    bool ValidatePageVersion(int32_t page_id, uint64_t expected_version) const;

    // 脏页管理
    bool MarkPageDirty(int32_t page_id, int32_t transaction_id);
    bool IsPageDirty(int32_t page_id) const;
    std::vector<int32_t> GetDirtyPages() const;
    bool FlushDirtyPage(int32_t page_id);

    // 缓存失效和更新传播
    void InvalidatePage(int32_t page_id);
    void InvalidateAllPages();
    void PropagatePageUpdate(int32_t page_id, uint64_t new_version);
    std::vector<int32_t> GetStalePages(std::chrono::milliseconds max_age) const;

    // 一致性检查和修复
    ConsistencyCheckResult PerformConsistencyCheck(int32_t page_id);
    bool RepairConsistency(int32_t page_id);
    std::unordered_map<int32_t, ConsistencyCheckResult> CheckAllPagesConsistency();

    // 统计信息
    struct ConsistencyStats {
        size_t total_pages = 0;
        size_t dirty_pages = 0;
        size_t locked_pages = 0;
        size_t version_conflicts = 0;
        size_t consistency_repairs = 0;
        double average_lock_wait_time_ms = 0.0;
        double cache_hit_rate = 0.0;
    };

    ConsistencyStats GetConsistencyStats() const;

    // 配置管理
    void SetConsistencyStrategy(CacheConsistencyStrategy strategy);
    void SetLockTimeout(std::chrono::milliseconds timeout);
    void SetVersionCheckEnabled(bool enabled);
    void SetAutoRepairEnabled(bool enabled);

    // 回调函数类型
    using ConsistencyViolationCallback = std::function<void(int32_t page_id, ConsistencyCheckResult)>;
    using PageInvalidationCallback = std::function<void(int32_t page_id)>;
    using VersionUpdateCallback = std::function<void(int32_t page_id, uint64_t old_version, uint64_t new_version)>;

    void SetConsistencyViolationCallback(ConsistencyViolationCallback callback);
    void SetPageInvalidationCallback(PageInvalidationCallback callback);
    void SetVersionUpdateCallback(VersionUpdateCallback callback);

private:
    // 页面锁管理
    struct PageLock {
        std::shared_mutex mutex;                    // 读写锁
        int32_t exclusive_owner = -1;             // 独占所有者（写锁）
        std::vector<int32_t> shared_owners;        // 共享所有者（读锁）
        std::chrono::steady_clock::time_point lock_time; // 锁获取时间
    };

    // 成员变量
    std::shared_ptr<BufferPoolSharded> buffer_pool_;
    CacheConsistencyStrategy strategy_;
    std::chrono::milliseconds lock_timeout_;
    bool version_check_enabled_;
    bool auto_repair_enabled_;

    mutable std::shared_mutex manager_mutex_; // 保护整个管理器的互斥锁

    // 页面版本和锁信息
    std::unordered_map<int32_t, PageVersion> page_versions_;
    std::unordered_map<int32_t, PageLock> page_locks_;

    // 统计信息
    mutable std::mutex stats_mutex_;
    ConsistencyStats stats_;
    std::vector<double> lock_wait_times_; // 锁等待时间记录

    // 回调函数
    ConsistencyViolationCallback violation_callback_;
    PageInvalidationCallback invalidation_callback_;
    VersionUpdateCallback version_callback_;

    // 辅助方法
    PageVersion& GetOrCreatePageVersion(int32_t page_id);
    PageLock& GetOrCreatePageLock(int32_t page_id);
    bool TryUpgradeLock(int32_t page_id, int32_t transaction_id);
    void UpdateLockWaitTime(std::chrono::milliseconds wait_time);
    ConsistencyCheckResult CheckConsistencyForStrategy(int32_t page_id, const PageVersion& version);
    bool RepairConsistencyForStrategy(int32_t page_id, ConsistencyCheckResult issue);
    void NotifyConsistencyViolation(int32_t page_id, ConsistencyCheckResult result);
    void NotifyPageInvalidation(int32_t page_id);
    void NotifyVersionUpdate(int32_t page_id, uint64_t old_version, uint64_t new_version);
};

// 原子版本管理器 - 提供无锁的版本管理
class AtomicVersionManager {
public:
    AtomicVersionManager();
    ~AtomicVersionManager() = default;

    // 版本操作
    uint64_t GetVersion(int32_t page_id) const;
    uint64_t IncrementVersion(int32_t page_id);
    bool CompareAndSetVersion(int32_t page_id, uint64_t expected, uint64_t desired);
    void ResetVersion(int32_t page_id);

    // 批量操作
    std::unordered_map<int32_t, uint64_t> GetVersions(const std::vector<int32_t>& page_ids) const;
    void UpdateVersions(const std::unordered_map<int32_t, uint64_t>& updates);

    // 统计信息
    size_t GetTotalVersions() const;
    uint64_t GetTotalVersionOperations() const;

private:
    std::unordered_map<int32_t, std::atomic<uint64_t>> page_versions_;
    mutable std::shared_mutex versions_mutex_;
    std::atomic<uint64_t> total_operations_{0};
};

// 内存屏障管理器 - 确保内存操作的顺序性
class MemoryBarrierManager {
public:
    MemoryBarrierManager();
    ~MemoryBarrierManager() = default;

    // 内存屏障操作
    void ReadBarrier();   // 读屏障 - 确保之前的读操作完成
    void WriteBarrier();  // 写屏障 - 确保之前的写操作对其他线程可见
    void FullBarrier();   // 全屏障 - 读写屏障的组合

    // 页面特定的屏障
    void PageReadBarrier(int32_t page_id);
    void PageWriteBarrier(int32_t page_id);
    void PageFullBarrier(int32_t page_id);

    // 性能监控
    struct BarrierStats {
        size_t read_barriers = 0;
        size_t write_barriers = 0;
        size_t full_barriers = 0;
        double average_barrier_time_ns = 0.0;
    };

    BarrierStats GetBarrierStats() const;

private:
    BarrierStats stats_;
    mutable std::mutex stats_mutex_;
    std::vector<double> barrier_times_; // 屏障操作时间记录
};

} // namespace sqlcc

#endif // SQLCC_CACHE_CONSISTENCY_MANAGER_H
