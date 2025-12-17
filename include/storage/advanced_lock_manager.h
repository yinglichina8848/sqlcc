/**
 * @file advanced_lock_manager.h
 * @brief 高级锁管理器头文件 - 改进的页面锁定机制
 */

#ifndef SQLCC_ADVANCED_LOCK_MANAGER_H
#define SQLCC_ADVANCED_LOCK_MANAGER_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <shared_mutex>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <vector>
#include <queue>
#include <functional>
#include <thread>

namespace sqlcc {

// 前向声明
class TransactionManager;

// 锁模式枚举 - 支持更细粒度的锁定
enum LockMode {
    SHARED = 0,           // 共享锁 (S) - 允许多个事务同时读取
    EXCLUSIVE = 1,        // 排他锁 (X) - 只允许一个事务访问
    INTENTION_SHARED = 2, // 意向共享锁 (IS) - 表示事务意图在子节点上加共享锁
    INTENTION_EXCLUSIVE = 3, // 意向排他锁 (IX) - 表示事务意图在子节点上加排他锁
    SHARED_INTENTION_EXCLUSIVE = 4 // 共享意向排他锁 (SIX) - 持有共享锁但意图在子节点加排他锁
};

// 锁请求状态
enum LockRequestStatus {
    GRANTED = 0,          // 已授予
    WAITING = 1,          // 等待中
    TIMED_OUT = 2,        // 超时
    DEADLOCK = 3,         // 死锁检测到
    ABORTED = 4           // 已中止
};

// 锁请求结构
struct LockRequest {
    int32_t transaction_id;
    LockMode mode;
    int32_t page_id;
    std::chrono::steady_clock::time_point request_time;
    std::chrono::milliseconds timeout;
    LockRequestStatus status;
    std::function<void(LockRequestStatus)> callback; // 异步回调

    LockRequest(int32_t tid, LockMode m, int32_t pid,
                std::chrono::milliseconds t = std::chrono::milliseconds(5000))
        : transaction_id(tid), mode(m), page_id(pid),
          request_time(std::chrono::steady_clock::now()),
          timeout(t), status(WAITING) {}
};

// 锁持有信息
struct LockHolder {
    int32_t transaction_id;
    LockMode mode;
    std::chrono::steady_clock::time_point acquire_time;
    int32_t reference_count; // 引用计数，支持重入

    LockHolder(int32_t tid, LockMode m)
        : transaction_id(tid), mode(m),
          acquire_time(std::chrono::steady_clock::now()),
          reference_count(1) {}
};

// 页面锁状态
struct PageLockState {
    std::vector<LockHolder> holders;
    std::deque<LockRequest> waiting_queue;  // 改为std::deque以支持迭代
    std::unordered_map<int32_t, LockMode> transaction_locks; // 事务->锁模式的映射

    // 锁计数器
    int shared_count = 0;
    int exclusive_count = 0;
    int intention_shared_count = 0;
    int intention_exclusive_count = 0;
    int shared_intention_exclusive_count = 0;
};

// 锁兼容性矩阵
class LockCompatibilityMatrix {
public:
    LockCompatibilityMatrix();
    ~LockCompatibilityMatrix() = default;

    // 检查两种锁模式是否兼容
    bool IsCompatible(LockMode existing, LockMode requested) const;

    // 检查请求的锁是否可以立即授予
    bool CanGrantImmediately(const PageLockState& state, LockMode requested) const;

private:
    // 锁兼容性矩阵 [existing][requested]
    bool compatibility_matrix_[5][5];
};

// 死锁检测器
class DeadlockDetector {
public:
    DeadlockDetector();
    ~DeadlockDetector() = default;

    // 检测死锁
    bool DetectDeadlock(const std::unordered_map<int32_t, PageLockState>& lock_table,
                       int32_t transaction_id, std::vector<int32_t>& deadlock_chain);

    // 添加等待关系
    void AddWaitFor(int32_t waiter, int32_t holder);

    // 移除等待关系
    void RemoveWaitFor(int32_t waiter);

    // 获取等待图
    const std::unordered_map<int32_t, std::unordered_set<int32_t>>& GetWaitGraph() const;

private:
    // 等待图：waiter -> set of holders
    std::unordered_map<int32_t, std::unordered_set<int32_t>> wait_graph_;
    mutable std::shared_mutex graph_mutex_;

    // 深度优先搜索检测环
    bool HasCycleDFS(int32_t start, std::unordered_set<int32_t>& visited,
                    std::unordered_set<int32_t>& recursion_stack,
                    std::vector<int32_t>& path) const;
};

// 锁升级/降级管理器
class LockUpgradeManager {
public:
    LockUpgradeManager();
    ~LockUpgradeManager() = default;

    // 锁升级策略
    enum UpgradeStrategy {
        IMMEDIATE_UPGRADE = 0,    // 立即升级
        DEFERRED_UPGRADE = 1,     // 延迟升级
        NO_UPGRADE = 2           // 不升级
    };

    // 检查锁是否可以升级
    UpgradeStrategy CanUpgrade(LockMode current, LockMode requested) const;

    // 执行锁升级
    bool PerformUpgrade(PageLockState& state, int32_t transaction_id,
                       LockMode new_mode);

    // 检查锁降级
    bool CanDowngrade(LockMode current, LockMode requested) const;

    // 执行锁降级
    bool PerformDowngrade(PageLockState& state, int32_t transaction_id,
                         LockMode new_mode);

private:
    // 锁升级兼容性矩阵
    bool upgrade_matrix_[5][5];
};

// 高级锁管理器
class AdvancedLockManager {
public:
    AdvancedLockManager(size_t max_locks = 10000,
                       std::chrono::milliseconds default_timeout = std::chrono::milliseconds(5000));
    ~AdvancedLockManager();

    // 同步锁操作
    LockRequestStatus AcquireLock(int32_t page_id, LockMode mode, int32_t transaction_id,
                                 std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));

    LockRequestStatus ReleaseLock(int32_t page_id, int32_t transaction_id);

    // 批量锁操作
    std::vector<LockRequestStatus> AcquireLocks(const std::vector<std::pair<int32_t, LockMode>>& requests,
                                               int32_t transaction_id,
                                               std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));

    size_t ReleaseAllLocks(int32_t transaction_id);

    // 异步锁操作
    bool AcquireLockAsync(int32_t page_id, LockMode mode, int32_t transaction_id,
                         std::function<void(LockRequestStatus)> callback,
                         std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));

    // 锁查询
    bool HasLock(int32_t page_id, int32_t transaction_id, LockMode min_mode = SHARED) const;
    LockMode GetLockMode(int32_t page_id, int32_t transaction_id) const;
    std::vector<int32_t> GetLockedPages(int32_t transaction_id) const;
    std::vector<int32_t> GetLockHolders(int32_t page_id) const;

    // 锁升级/降级
    LockRequestStatus UpgradeLock(int32_t page_id, int32_t transaction_id, LockMode new_mode);
    LockRequestStatus DowngradeLock(int32_t page_id, int32_t transaction_id, LockMode new_mode);

    // 死锁检测和处理
    bool DetectAndResolveDeadlock(std::vector<int32_t>& victims);
    void SetDeadlockResolutionStrategy(std::function<int32_t(const std::vector<int32_t>&)> strategy);

    // 锁超时管理
    void CleanupExpiredLocks();
    void SetLockTimeout(std::chrono::milliseconds timeout);

    // 统计信息
    struct LockManagerStats {
        size_t total_locks = 0;
        size_t waiting_requests = 0;
        size_t deadlocks_detected = 0;
        size_t deadlocks_resolved = 0;
        size_t lock_timeouts = 0;
        size_t lock_upgrades = 0;
        size_t lock_downgrades = 0;
        double average_lock_wait_time_ms = 0.0;
        double lock_hit_rate = 0.0;
    };

    LockManagerStats GetStats() const;

    // 配置管理
    void SetMaxLocks(size_t max_locks);
    void EnableDeadlockDetection(bool enable);
    void SetDeadlockCheckInterval(std::chrono::milliseconds interval);

private:
    // 核心数据结构
    std::unordered_map<int32_t, PageLockState> lock_table_; // page_id -> lock state
    mutable std::shared_mutex lock_table_mutex_;

    // 组件
    LockCompatibilityMatrix compatibility_matrix_;
    DeadlockDetector deadlock_detector_;
    LockUpgradeManager upgrade_manager_;

    // 配置参数
    size_t max_locks_;
    std::chrono::milliseconds default_timeout_;
    bool deadlock_detection_enabled_;
    std::chrono::milliseconds deadlock_check_interval_;

    // 统计信息
    mutable std::mutex stats_mutex_;
    LockManagerStats stats_;
    std::vector<double> lock_wait_times_;

    // 死锁处理策略
    std::function<int32_t(const std::vector<int32_t>&)> deadlock_resolution_strategy_;

    // 异步处理
    std::mutex async_mutex_;
    std::condition_variable async_cv_;
    std::queue<LockRequest> async_requests_;
    std::atomic<bool> async_worker_running_;
    std::thread async_worker_thread_;

    // 辅助方法
    PageLockState& GetOrCreatePageLockState(int32_t page_id);
    LockRequestStatus TryAcquireLock(int32_t page_id, LockMode mode, int32_t transaction_id);
    void GrantWaitingRequests(int32_t page_id);
    void UpdateLockWaitTime(std::chrono::milliseconds wait_time);
    void ProcessAsyncRequests();
    void StartAsyncWorker();
    void StopAsyncWorker();

    // 锁兼容性检查
    bool IsLockCompatible(const PageLockState& state, LockMode requested) const;
    bool CanUpgradeLock(const PageLockState& state, int32_t transaction_id, LockMode new_mode) const;
};

// 锁管理器工厂
class LockManagerFactory {
public:
    static std::shared_ptr<AdvancedLockManager> CreateBasicLockManager(
        std::chrono::milliseconds default_timeout = std::chrono::milliseconds(5000));

    static std::shared_ptr<AdvancedLockManager> CreateHighConcurrencyLockManager(
        size_t max_locks = 50000,
        std::chrono::milliseconds default_timeout = std::chrono::milliseconds(2000));

    static std::shared_ptr<AdvancedLockManager> CreateStrictLockManager(
        std::chrono::milliseconds default_timeout = std::chrono::milliseconds(10000));
};

} // namespace sqlcc

#endif // SQLCC_ADVANCED_LOCK_MANAGER_H
