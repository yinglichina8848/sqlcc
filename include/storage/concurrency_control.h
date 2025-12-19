#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include <memory>

namespace sqlcc {

/**
 * @brief 层级锁管理器
 * 
 * 为BufferPool提供分层级的锁管理机制，包括页面级锁和表级锁。
 * 支持读写锁分离，提高并发性能。
 */
class HierarchicalLockManager {
public:
    /**
     * @brief 锁类型
     */
    enum class LockType {
        SHARED,    // 共享锁（读锁）
        EXCLUSIVE  // 排他锁（写锁）
    };

    /**
     * @brief 锁信息结构
     */
    struct LockInfo {
        LockType type;
        int32_t transaction_id;
        std::chrono::steady_clock::time_point acquire_time;
        
        LockInfo() : type(LockType::SHARED), transaction_id(0), acquire_time(std::chrono::steady_clock::now()) {}
        
        LockInfo(LockType t, int32_t txn_id) 
            : type(t), transaction_id(txn_id), acquire_time(std::chrono::steady_clock::now()) {}
    };

    /**
     * @brief 锁管理器统计信息
     */
    struct LockManagerStats {
        size_t shared_locks = 0;        // 当前共享锁数量
        size_t exclusive_locks = 0;     // 当前排他锁数量
        size_t total_acquires = 0;      // 总获取次数
        size_t total_waits = 0;         // 总等待次数
        size_t deadlocks_detected = 0;  // 检测到的死锁数量
    };

    /**
     * @brief 构造函数
     */
    explicit HierarchicalLockManager(size_t max_locks = 1024);

    /**
     * @brief 析构函数
     */
    ~HierarchicalLockManager();

    /**
     * @brief 获取页面锁
     * @param page_id 页面ID
     * @param lock_type 锁类型
     * @param transaction_id 事务ID
     * @param timeout_ms 超时时间（毫秒）
     * @return 是否成功获取锁
     */
    bool AcquirePageLock(int32_t page_id, LockType lock_type, 
                        int32_t transaction_id, size_t timeout_ms = 5000);

    /**
     * @brief 释放页面锁
     * @param page_id 页面ID
     * @param transaction_id 事务ID
     * @return 是否成功释放锁
     */
    bool ReleasePageLock(int32_t page_id, int32_t transaction_id);

    /**
     * @brief 获取表锁
     * @param table_id 表ID
     * @param lock_type 锁类型
     * @param transaction_id 事务ID
     * @param timeout_ms 超时时间（毫秒）
     * @return 是否成功获取锁
     */
    bool AcquireTableLock(int32_t table_id, LockType lock_type,
                         int32_t transaction_id, size_t timeout_ms = 5000);

    /**
     * @brief 释放表锁
     * @param table_id 表ID
     * @param transaction_id 事务ID
     * @return 是否成功释放锁
     */
    bool ReleaseTableLock(int32_t table_id, int32_t transaction_id);

    /**
     * @brief 获取锁统计信息
     * @return 统计信息
     */
    LockManagerStats GetStats() const;

    /**
     * @brief 检测死锁
     * @return 是否检测到死锁
     */
    bool DetectDeadlock() const;

    /**
     * @brief 清理过期锁
     */
    void CleanupExpiredLocks();

private:
    /**
     * @brief 页面锁映射：page_id -> 锁信息列表
     */
    std::unordered_map<int32_t, std::vector<std::pair<LockType, int32_t>>> page_locks_;
    
    /**
     * @brief 表锁映射：table_id -> 锁信息列表
     */
    std::unordered_map<int32_t, std::vector<std::pair<LockType, int32_t>>> table_locks_;
    
    /**
     * @brief 页面锁的互斥锁
     */
    mutable std::shared_mutex page_locks_mutex_;
    
    /**
     * @brief 表锁的互斥锁
     */
    mutable std::shared_mutex table_locks_mutex_;
    
    /**
     * @brief 统计信息
     */
    mutable std::atomic<LockManagerStats> stats_;
    
    /**
     * @brief 最大锁数量
     */
    const size_t max_locks_;
    
    /**
     * @brief 锁超时时间（毫秒）
     */
    static constexpr size_t LOCK_TIMEOUT_MS = 10000;
};

/**
 * @brief 预取器
 * 
 * 负责智能预取页面，减少磁盘I/O延迟。
 * 支持基于访问模式的预测性预取。
 */
class Prefetcher {
public:
    /**
     * @brief 预取器统计信息
     */
    struct PrefetcherStats {
        size_t prefetches_requested = 0;  // 请求的预取次数
        size_t prefetches_served = 0;     // 实际服务的预取次数
        size_t prefetch_hit_rate = 0;     // 预取命中率
        size_t sequential_prefetch = 0;   // 顺序预取次数
        size_t random_prefetch = 0;       // 随机预取次数
    };

    /**
     * @brief 访问模式
     */
    enum class AccessPattern {
        SEQUENTIAL,    // 顺序访问
        RANDOM,        // 随机访问
        STRIDED,       // 跨步访问
        PREDICTABLE    // 可预测访问
    };

    /**
     * @brief 构造函数
     * @param buffer_pool BufferPool引用
     * @param max_prefetch_size 最大预取大小
     */
    explicit Prefetcher(void* buffer_pool, size_t max_prefetch_size = 64);

    /**
     * @brief 析构函数
     */
    ~Prefetcher();

    /**
     * @brief 记录页面访问
     * @param page_id 页面ID
     * @param is_write 是否为写操作
     */
    void RecordPageAccess(int32_t page_id, bool is_write);

    /**
     * @brief 预取页面
     * @param page_id 页面ID
     * @return 是否成功触发预取
     */
    bool PrefetchPage(int32_t page_id);

    /**
     * @brief 批量预取页面
     * @param page_ids 页面ID列表
     * @return 成功预取的页面数量
     */
    size_t PrefetchPages(const std::vector<int32_t>& page_ids);

    /**
     * @brief 获取预取统计信息
     * @return 统计信息
     */
    PrefetcherStats GetStats() const;

    /**
     * @brief 启用/禁用预取
     * @param enabled 是否启用
     */
    void SetEnabled(bool enabled);

    /**
     * @brief 获取访问模式
     * @param page_id 页面ID
     * @return 访问模式
     */
    AccessPattern GetAccessPattern(int32_t page_id) const;

private:
    /**
     * @brief 页面访问历史
     */
    std::unordered_map<int32_t, std::vector<std::chrono::steady_clock::time_point>> access_history_;
    
    /**
     * @brief 访问历史互斥锁
     */
    mutable std::mutex access_history_mutex_;
    
    /**
     * @brief 预取队列
     */
    std::vector<int32_t> prefetch_queue_;
    
    /**
     * @brief 预取队列互斥锁
     */
    mutable std::mutex prefetch_queue_mutex_;
    
    /**
     * @brief 预取线程
     */
    std::thread prefetch_thread_;
    
    /**
     * @brief 停止预取线程标志
     */
    std::atomic<bool> stop_prefetch_;
    
    /**
     * @brief 是否启用预取
     */
    std::atomic<bool> enabled_;
    
    /**
     * @brief 统计信息
     */
    mutable std::atomic<PrefetcherStats> stats_;
    
    /**
     * @brief 最大预取大小
     */
    const size_t max_prefetch_size_;
    
    /**
     * @brief 预取线程函数
     */
    void PrefetchWorker();
};

} // namespace sqlcc
