/**
 * WHY: 为什么数据库系统需要高级锁管理器？
 *
 * 数据库系统作为多用户并发访问的核心组件，锁管理直接影响系统的并发性能和数据一致性：
 * 1. 并发控制复杂性：现代数据库需要支持高并发的多事务访问
 * 2. 死锁预防：复杂的业务逻辑可能导致死锁，需要自动检测和解决
 * 3. 锁粒度管理：不同场景需要不同粒度的锁（行级、页级、表级）
 * 4. 锁升级策略：锁的动态升级和降级以优化性能
 * 5. 异步处理：支持异步锁请求以提高响应性
 * 6. 性能监控：需要详细的锁统计信息进行性能调优
 *
 * 高级锁管理器的价值体现在：
 * - 提高并发性能：减少锁竞争，提高系统吞吐量
 * - 确保数据一致性：防止并发访问导致的数据不一致
 * - 自动死锁解决：检测并自动解决死锁情况
 * - 灵活的锁策略：支持多种锁模式和升级策略
 * - 性能监控：提供详细的锁统计和性能指标
 * - 可扩展性：支持大规模并发访问的扩展
 *
 * WHAT: AdvancedLockManager - 高级锁管理器
 *
 * 提供企业级数据库系统的完整锁管理功能，包括同步/异步锁操作、死锁检测与解决、锁升级/降级等：
 * - 锁生命周期：完整的锁获取、持有、释放生命周期管理
 * - 死锁处理：自动死锁检测和基于策略的死锁解决
 * - 锁升级管理：智能的锁升级和降级策略
 * - 异步操作：支持异步锁请求以提高系统响应性
 * - 统计监控：详细的锁操作统计和性能监控
 * - 配置管理：可配置的锁参数和超时策略
 *
 * 核心特性：
 * - 多粒度锁：支持行级、页级、表级等多粒度锁管理
 * - 死锁检测：基于等待图的死锁检测算法
 * - 锁兼容性：完整的锁模式兼容性矩阵
 * - 异步处理：异步锁请求队列和处理机制
 * - 统计信息：详细的锁操作统计和性能指标
 * - 扩展性：支持自定义死锁解决策略和锁管理策略
 *
 * HOW: 高级锁管理器的架构和技术实现
 *
 * 1. 锁管理核心架构：
 *    - 锁表管理：page_id到锁状态的映射表
 *    - 等待队列：每个页面的锁请求等待队列
 *    - 锁持有信息：记录锁的持有者和模式信息
 *    - 兼容性检查：锁模式间的兼容性判断矩阵
 *
 * 2. 死锁检测机制：
 *    - 等待图构建：事务间的等待关系图
 *    - 环检测算法：深度优先搜索检测死锁环
 *    - 死锁解决策略：可配置的死锁受害者选择策略
 *    - 自动解除：检测到死锁后自动选择受害者并回滚
 *
 * 3. 锁升级/降级系统：
 *    - 升级时机：基于锁竞争情况的智能升级
 *    - 降级策略：在合适时机降低锁粒度
 *    - 性能优化：平衡锁开销和并发性能
 *    - 策略配置：可配置的升级和降级阈值
 *
 * 4. 异步处理框架：
 *    - 请求队列：异步锁请求的队列管理
 *    - 工作线程：专门的异步处理工作线程
 *    - 回调机制：锁请求完成后的异步回调
 *    - 超时处理：异步请求的超时和取消机制
 *
 * 5. 统计监控体系：
 *    - 锁操作统计：各种锁操作的计数和耗时统计
 *    - 性能指标：锁竞争率、死锁发生率等关键指标
 *    - 历史记录：锁操作的历史记录和趋势分析
 *    - 告警机制：异常情况的监控和告警
 *
 * 6. 并发安全设计：
 *    - 细粒度锁：使用读写锁保护锁表的不同部分
 *    - 无锁优化：某些操作的无锁优化路径
 *    - 原子操作：关键状态变更的原子性保证
 *    - 内存序：正确的内存序保证并发安全
 *
 * 🏗️ 设计模式：策略模式 + 观察者模式
 *
 * 策略模式应用：
 * - 锁管理策略：不同场景下的锁管理策略
 * - 死锁解决策略：可插拔的死锁解决算法
 * - 锁升级策略：灵活的锁升级和降级策略
 * - 超时处理策略：不同的超时处理机制
 *
 * 观察者模式应用：
 * - 锁事件通知：锁获取、释放、超时等事件的观察者
 * - 死锁事件监听：死锁检测和解决的监听机制
 * - 统计信息收集：各种锁操作统计信息的收集
 * - 性能监控：锁系统性能指标的监控和报告
 *
 * SOLID原则体现：
 *
 * 1. 单一职责原则(SRP)：
 *    - AdvancedLockManager只负责锁管理逻辑
 *    - 死锁检测由DeadlockDetector专门处理
 *    - 锁升级由LockUpgradeManager管理
 *    - 统计功能由独立的统计模块处理
 *
 * 2. 开闭原则(OCP)：
 *    - 支持新的锁模式和兼容性规则扩展
 *    - 可以通过继承和组合添加新的锁管理策略
 *    - 死锁解决策略可以通过接口替换
 *    - 统计和监控功能可以独立扩展
 *
 * 3. 里氏替换原则(LSP)：
 *    - 任何锁管理器的实现都可以替代接口使用
 *    - 保证接口契约的一致性和行为正确性
 *    - 子类可以完全替代父类的使用场景
 *
 * 4. 接口隔离原则(ISP)：
 *    - 提供简洁的锁管理接口集合
 *    - 避免客户端依赖不需要的锁管理功能
 *    - 按需暴露锁管理的各个方面
 *
 * 5. 依赖倒置原则(DIP)：
 *    - 锁管理器依赖抽象的策略接口
 *    - 不依赖具体的策略实现细节
 *    - 通过依赖注入提供策略的灵活性
 *
 * 锁管理器的性能优化：
 * - 锁表分片：减少锁竞争，提高并发性能
 * - 批量操作：支持多锁的批量获取和释放
 * - 乐观锁：某些场景下的乐观锁优化
 * - 缓存优化：锁信息的缓存和预取机制
 * - 内存池：锁对象的内存池管理减少分配开销
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

#include "src/storage_engine/concurrency_control.h"

namespace sqlcc {

// 前向声明
class TransactionManager;

// 使用concurrency_control.h中的LockType作为锁模式

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
    LockType mode;
    int32_t page_id;
    std::chrono::steady_clock::time_point request_time;
    std::chrono::milliseconds timeout;
    LockRequestStatus status;
    std::function<void(LockRequestStatus)> callback; // 异步回调

    LockRequest(int32_t tid, LockType m, int32_t pid,
                std::chrono::milliseconds t = std::chrono::milliseconds(5000))
        : transaction_id(tid), mode(m), page_id(pid),
          request_time(std::chrono::steady_clock::now()),
          timeout(t), status(WAITING) {}
};

// 锁持有信息
struct LockHolder {
    int32_t transaction_id;
    LockType mode;
    std::chrono::steady_clock::time_point acquire_time;
    int32_t reference_count; // 引用计数，支持重入

    LockHolder(int32_t tid, LockType m)
        : transaction_id(tid), mode(m),
          acquire_time(std::chrono::steady_clock::now()),
          reference_count(1) {}
};

// 页面锁状态
struct PageLockState {
    std::vector<LockHolder> holders;
    std::deque<LockRequest> waiting_queue;  // 改为std::deque以支持迭代
    std::unordered_map<int32_t, LockType> transaction_locks; // 事务->锁模式的映射

    // 锁计数器
    int shared_count = 0;
    int exclusive_count = 0;
    int intention_shared_count = 0;
    int intention_exclusive_count = 0;
    int shared_intention_exclusive_count = 0;
};



// 辅助函数：更新锁计数器
void UpdateLockCounts(PageLockState& state, LockType mode, int delta);

// 死锁检测器
class AdvancedDeadlockDetector {
public:
    AdvancedDeadlockDetector();
    ~AdvancedDeadlockDetector() = default;

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
};// 锁升级/降级管理器
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
    UpgradeStrategy CanUpgrade(LockType current, LockType requested) const;

    // 执行锁升级
    bool PerformUpgrade(PageLockState& state, int32_t transaction_id,
                       LockType new_mode);

    // 检查锁降级
    bool CanDowngrade(LockType current, LockType requested) const;

    // 执行锁降级
    bool PerformDowngrade(PageLockState& state, int32_t transaction_id,
                         LockType new_mode);

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
    LockRequestStatus AcquireLock(int32_t page_id, LockType mode, int32_t transaction_id,
                                 std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));

    LockRequestStatus ReleaseLock(int32_t page_id, int32_t transaction_id);

    // 批量锁操作
    std::vector<LockRequestStatus> AcquireLocks(const std::vector<std::pair<int32_t, LockType>>& requests,
                                               int32_t transaction_id,
                                               std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));

    size_t ReleaseAllLocks(int32_t transaction_id);

    // 异步锁操作
    bool AcquireLockAsync(int32_t page_id, LockType mode, int32_t transaction_id,
                         std::function<void(LockRequestStatus)> callback,
                         std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));

    // 锁查询
    bool HasLock(int32_t page_id, int32_t transaction_id, LockType min_mode = LockType::SHARED) const;
    LockType GetLockType(int32_t page_id, int32_t transaction_id) const;
    std::vector<int32_t> GetLockedPages(int32_t transaction_id) const;
    std::vector<int32_t> GetLockHolders(int32_t page_id) const;

    // 锁升级/降级
    LockRequestStatus UpgradeLock(int32_t page_id, int32_t transaction_id, LockType new_mode);
    LockRequestStatus DowngradeLock(int32_t page_id, int32_t transaction_id, LockType new_mode);

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
    AdvancedDeadlockDetector deadlock_detector_;
    LockUpgradeManager upgrade_manager_;    // 配置参数
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
    LockRequestStatus TryAcquireLock(int32_t page_id, LockType mode, int32_t transaction_id);
    void GrantWaitingRequests(int32_t page_id);
    void UpdateLockWaitTime(std::chrono::milliseconds wait_time);
    void ProcessAsyncRequests();
    void StartAsyncWorker();
    void StopAsyncWorker();

    // 锁兼容性检查
    bool IsLockCompatible(const PageLockState& state, LockType requested) const;
    bool CanUpgradeLock(const PageLockState& state, int32_t transaction_id, LockType new_mode) const;
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
