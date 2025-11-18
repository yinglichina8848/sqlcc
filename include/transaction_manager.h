#ifndef SQLCC_TRANSACTION_MANAGER_H
#define SQLCC_TRANSACTION_MANAGER_H

#include <memory>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <vector>
#include <string>
#include <unordered_set>
#include <bitset>
#include <functional>

#include "exception.h"

namespace sqlcc {

/**
 * 事务ID类型
 */
typedef int64_t TransactionId;

/**
 * 事务隔离级别
 */
enum class IsolationLevel {
    READ_UNCOMMITTED,
    READ_COMMITTED,
    REPEATABLE_READ,
    SNAPSHOT_ISOLATION
};

/**
 * 事务状态
 */
enum class TransactionStatus {
    ACTIVE,
    COMMITTED,
    ROLLED_BACK,
    ABORTED
};

/**
 * 增强版键锁管理器（基于Badger/RocksDB的key stripe锁机制 + 死锁检测）
 * 特点：
 * 1. 写事务只锁 hash(key) % stripe_count
 * 2. 读事务无锁，通过MVCC获取快照
 * 3. 集成Wait-for-Graph死锁检测算法
 * 4. 锁超时和性能监控支持
 *
 * 新增功能 (v0.4.7+):
 * - 死锁检测算法
 * - 锁等待图 (Wait Graph)
 * - 锁超时机制
 * - 锁性能统计
 */
class StripeLockManager {
public:
    /**
     * 锁指标结构体
     */
    struct LockMetrics {
        size_t total_locks = 0;                    // 总锁获取次数
        size_t lock_conflicts = 0;                 // 锁冲突次数
        size_t deadlocks_detected = 0;             // 检测到死锁次数
        size_t lock_timeouts = 0;                  // 锁超时次数
        std::chrono::milliseconds avg_lock_wait_time{0}; // 平均等待时间
        std::chrono::milliseconds total_lock_wait_time{0}; // 总等待时间
    };

    /**
     * 锁等待结构
     */
    struct LockWait {
        TransactionId waiter;      // 等待者
        TransactionId holder;      // 持有者
        std::string lock_key;      // 锁定的键
        std::chrono::steady_clock::time_point wait_start_time; // 开始等待时间
    };

    /**
     * 构造函数
     * @param stripe_count stripe数量（建议为2的幂）
     * @param enable_deadlock_detection 是否启用死锁检测
     */
    explicit StripeLockManager(size_t stripe_count = 64, bool enable_deadlock_detection = true);

    /**
     * 获取键的写锁（带超时和死锁检测）
     * @param key 键
     * @param txn_id 事务ID
     * @param timeout_ms 超时时间（毫秒）
     * @return 是否成功获取锁
     */
    bool AcquireWriteLock(const std::string& key, TransactionId txn_id,
                          std::chrono::milliseconds timeout_ms = std::chrono::milliseconds(5000));

    /**
     * 释放键的写锁
     * @param key 键
     * @param txn_id 事务ID
     * @return 是否成功释放锁
     */
    bool ReleaseWriteLock(const std::string& key, TransactionId txn_id);

    /**
     * 检查键是否被锁定
     * @param key 键
     * @return 是否被锁定
     */
    bool IsLocked(const std::string& key) const;

    /**
     * 检查事务是否陷入死锁
     * @param txn_id 事务ID
     * @return 是否死锁
     */
    bool HasDeadlock(TransactionId txn_id) const;

    /**
     * 获取指定事务的等待图
     * @param txn_id 事务ID
     * @return 等待列表
     */
    std::vector<LockWait> GetWaitGraph(TransactionId txn_id) const;

    /**
     * 打印当前等待图（用于调试）
     */
    void PrintWaitGraph() const;

    /**
     * 获取锁性能指标
     * @return 锁指标
     */
    LockMetrics GetMetrics() const;

    /**
     * 重置锁性能指标
     */
    void ResetMetrics();

private:
    // 计算键的hash值
    inline size_t HashKey(const std::string& key) const {
        // 使用简单的字符串哈希函数
        size_t hash = 0;
        for (char c : key) {
            hash = hash * 131 + static_cast<unsigned char>(c);
        }
        return hash;
    }

    // 获取键对应的stripe索引
    inline size_t GetStripeIndex(const std::string& key) const {
        return HashKey(key) & (stripe_count_ - 1);
    }

    size_t stripe_count_;                      // stripe数量
    std::vector<std::unique_ptr<std::mutex>> stripe_locks_; // stripe锁数组（使用智能指针）
    std::vector<std::unordered_set<std::string>> locked_keys_; // 每个stripe中被锁定的键
    std::vector<TransactionId> lock_owners_;   // 每个stripe的锁拥有者

    // 死锁检测相关数据
    bool enable_deadlock_detection_;           // 是否启用死锁检测
    mutable std::mutex metrics_mutex_;         // 性能指标锁
    mutable std::vector<std::unordered_map<std::string, LockWait>> waiting_for_locks_; // 每个stripe的等待锁信息

    // 死锁检测帮助方法声明
    bool DetectDeadlockCycle(TransactionId txn_id,
                           std::unordered_set<TransactionId>& visited,
                           std::unordered_set<TransactionId>& recursion_stack) const;

    // 性能指标
    mutable LockMetrics metrics_;              // 锁性能指标
};

/**
 * 事务管理器
 */
class TransactionManager {
public:
    /**
     * 构造函数
     * @param stripe_count stripe锁数量
     */
    explicit TransactionManager(size_t stripe_count = 64);

    /**
     * 开始新事务
     * @param isolation_level 隔离级别
     * @return 事务ID
     */
    TransactionId BeginTransaction(IsolationLevel isolation_level = IsolationLevel::SNAPSHOT_ISOLATION);

    /**
     * 提交事务
     * @param txn_id 事务ID
     * @return 是否提交成功
     */
    bool CommitTransaction(TransactionId txn_id);

    /**
     * 回滚事务
     * @param txn_id 事务ID
     * @return 是否回滚成功
     */
    bool RollbackTransaction(TransactionId txn_id);

    /**
     * 获取键的写锁（在事务中）
     * @param txn_id 事务ID
     * @param key 键
     * @return 是否成功获取锁
     */
    bool LockForWrite(TransactionId txn_id, const std::string& key);

    /**
     * 释放键的写锁（在事务中）
     * @param txn_id 事务ID
     * @param key 键
     * @return 是否成功释放锁
     */
    bool UnlockForWrite(TransactionId txn_id, const std::string& key);

    /**
     * 获取事务隔离级别
     * @param txn_id 事务ID
     * @return 隔离级别
     */
    IsolationLevel GetIsolationLevel(TransactionId txn_id) const;

    /**
     * 获取事务状态
     * @param txn_id 事务ID
     * @return 事务状态
     */
    TransactionStatus GetTransactionStatus(TransactionId txn_id) const;

    /**
     * 获取事务的快照版本
     * @param txn_id 事务ID
     * @return 快照版本号
     */
    uint64_t GetTransactionSnapshot(TransactionId txn_id) const;

private:
    // 事务信息
    struct TransactionInfo {
        TransactionId txn_id;
        IsolationLevel isolation_level;
        TransactionStatus status;
        uint64_t snapshot_version;
        std::unordered_set<std::string> locked_keys;
    };

    std::mutex txn_mutex_;                      // 事务管理互斥锁
    std::atomic<TransactionId> next_txn_id_;    // 下一个事务ID
    std::atomic<uint64_t> global_version_;      // 全局版本号
    std::unordered_map<TransactionId, TransactionInfo> active_txns_; // 活跃事务
    StripeLockManager lock_manager_;           // 键锁管理器
};

}  // namespace sqlcc

#endif  // SQLCC_TRANSACTION_MANAGER_H
