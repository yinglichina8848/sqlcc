/**
 * WHY: 为什么数据库系统需要缓存一致性管理器？
 *
 * 数据库系统的缓存层是性能优化的核心，但也带来了复杂的一致性挑战：
 * 1. 多版本并发控制：多个事务可能同时访问同一数据的不同版本
 * 2. 脏页管理：未提交的数据修改需要特殊处理以防止数据污染
 * 3. 缓存失效：缓存数据的过期和失效需要精确控制
 * 4. 并发访问：多线程环境下的缓存访问需要同步机制
 * 5. 一致性策略：不同应用场景需要不同的一致性保证级别
 * 6. 故障恢复：系统崩溃后缓存状态的一致性恢复
 *
 * 缓存一致性管理器的价值体现在：
 * - 保证数据正确性：防止缓存导致的数据不一致和错误
 * - 提高系统性能：通过智能缓存策略优化访问性能
 * - 支持并发访问：安全的多线程缓存访问机制
 * - 灵活一致性策略：支持多种一致性模型的选择
 * - 自动故障恢复：系统异常后的自动状态恢复
 * - 监控和诊断：详细的缓存状态监控和问题诊断
 *
 * WHAT: CacheConsistencyManager - 缓存一致性管理器
 *
 * 提供企业级数据库系统的完整缓存一致性管理功能，包括页面版本控制、脏页管理、缓存失效、并发访问控制等：
 * - 页面版本控制：精确的页面版本管理和冲突检测
 * - 脏页管理：未提交修改的页面状态跟踪和处理
 * - 读写锁机制：细粒度的页面级读写锁控制
 * - 一致性检查：多种一致性策略的实现和选择
 * - 缓存失效管理：智能的缓存失效和更新传播
 * - 统计监控：详细的缓存操作统计和性能监控
 *
 * 核心特性：
 * - 多版本支持：支持页面级多版本并发控制
 * - 一致性策略：可配置的一致性保证级别
 * - 并发安全：线程安全的缓存访问和修改
 * - 自动修复：检测到不一致时的自动修复机制
 * - 性能监控：缓存命中率和操作延迟的监控
 * - 扩展接口：支持自定义一致性检查和修复策略
 *
 * HOW: 缓存一致性管理器的架构和技术实现
 *
 * 1. 页面版本管理核心架构：
 *    - 版本号管理：64位原子版本号的递增和比较
 *    - 时间戳记录：页面访问和修改的时间戳管理
 *    - 事务关联：页面版本与事务ID的关联管理
 *    - 版本历史：页面修改历史的维护和查询
 *
 * 2. 脏页处理机制：
 *    - 脏页标记：页面修改时的脏页状态设置
 *    - 写时复制：避免脏页对其他读取者的影响
 *    - 延迟写入：脏页的批量写入优化
 *    - 回滚支持：事务失败时的脏页清理
 *
 * 3. 锁管理机制：
 *    - 读写锁实现：基于shared_mutex的读写锁
 *    - 锁升级策略：读锁到写锁的智能升级
 *    - 死锁预防：锁获取顺序和超时机制
 *    - 锁粒度控制：页面级别的细粒度锁控制
 *
 * 4. 一致性策略框架：
 *    - 严格一致性：强一致性保证，性能开销较大
 *    - 最终一致性：弱一致性保证，性能较好
 *    - 因果一致性：因果关系的一致性保证
 *    - 自适应策略：根据负载动态调整一致性级别
 *
 * 5. 缓存失效和更新传播：
 *    - 主动失效：显式的缓存条目失效操作
 *    - 被动失效：基于时间或访问模式的自动失效
 *    - 更新传播：页面修改时的版本更新通知
 *    - 批量操作：多个页面的批量失效和更新
 *
 * 6. 并发控制技术：
 *    - 原子操作：版本号的原子递增和比较
 *    - 内存屏障：正确的内存序保证
 *    - 无锁优化：某些操作的无锁优化路径
 *    - 细粒度锁：减小锁竞争范围的优化
 *
 * 🏗️ 设计模式：策略模式 + 观察者模式
 *
 * 策略模式应用：
 * - 一致性策略：不同场景下的一致性保证策略
 * - 锁管理策略：不同的锁获取和释放策略
 * - 缓存失效策略：不同的缓存失效处理策略
 * - 修复策略：不同的一致性问题修复策略
 *
 * 观察者模式应用：
 * - 一致性事件监听：一致性违反事件的监听和处理
 * - 版本更新通知：页面版本更新的观察者通知
 * - 缓存失效回调：缓存失效事件的回调处理
 * - 性能监控：缓存操作性能指标的收集和报告
 *
 * SOLID原则体现：
 *
 * 1. 单一职责原则(SRP)：
 *    - CacheConsistencyManager只负责缓存一致性管理
 *    - 页面版本管理由AtomicVersionManager处理
 *    - 内存屏障管理由MemoryBarrierManager负责
 *    - 职责分离清晰，功能专注
 *
 * 2. 开闭原则(OCP)：
 *    - 支持新的缓存一致性策略扩展
 *    - 可以通过继承添加新的版本管理机制
 *    - 一致性检查算法可以独立扩展
 *    - 对扩展开放，对修改关闭
 *
 * 3. 里氏替换原则(LSP)：
 *    - 任何一致性管理器实现都可以替代接口使用
 *    - 保证接口契约的一致性和行为正确性
 *    - 子类可以完全替代父类的使用场景
 *
 * 4. 接口隔离原则(ISP)：
 *    - 提供简洁的缓存一致性管理接口
 *    - 避免客户端依赖不需要的一致性功能
 *    - 按需暴露缓存管理的各个方面
 *
 * 5. 依赖倒置原则(DIP)：
 *    - 一致性管理器依赖抽象的策略接口
 *    - 不依赖具体的一致性实现细节
 *    - 通过依赖注入提高系统的可测试性
 *
 * 缓存一致性管理的性能优化：
 * - 版本缓存：页面版本信息的缓存优化
 * - 批量操作：多个页面的批量一致性检查
 * - 异步处理：一致性检查的异步处理机制
 * - 预取优化：页面访问模式的预取和缓存
 * - 压缩存储：版本历史和统计信息的压缩存储
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
