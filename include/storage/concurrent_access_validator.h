/**
 * @file concurrent_access_validator.h
 * @brief 并发访问验证器头文件 - 实现记录操作的并发访问控制
 */

#ifndef SQLCC_CONCURRENT_ACCESS_VALIDATOR_H
#define SQLCC_CONCURRENT_ACCESS_VALIDATOR_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <condition_variable>

namespace sqlcc {

// 前向声明
class StorageEngine;
class TransactionManager;
class RecordBoundaryValidator;

// 并发访问控制类型
enum AccessControlType {
    READ_ACCESS = 1,       // 读访问
    WRITE_ACCESS = 2,      // 写访问
    EXCLUSIVE_ACCESS = 3,  // 独占访问
    INTENT_READ = 4,       // 意向读锁
    INTENT_WRITE = 5,      // 意向写锁
};

// 锁模式
enum LockMode {
    SHARED_LOCK = 1,       // 共享锁 (允许多读)
    EXCLUSIVE_LOCK = 2,    // 排他锁 (只允许单写)
    UPDATE_LOCK = 3,       // 更新锁 (用于防止死锁)
    INTENT_SHARED = 4,     // 意向共享锁
    INTENT_EXCLUSIVE = 5,  // 意向排他锁
    SHARED_INTENT_EXCLUSIVE = 6, // 共享意向排他锁
};

// 锁状态
enum LockState {
    LOCK_GRANTED = 0,      // 锁已授予
    LOCK_WAITING = 1,      // 锁等待中
    LOCK_TIMEOUT = 2,      // 锁超时
    LOCK_DEADLOCK = 3,     // 死锁检测
    LOCK_CONFLICT = 4,     // 锁冲突
};

// 事务隔离级别
enum IsolationLevel {
    READ_UNCOMMITTED = 1,  // 读未提交
    READ_COMMITTED = 2,    // 读已提交
    REPEATABLE_READ = 3,   // 可重复读
    SERIALIZABLE = 4,      // 串行化
};

// 锁请求
struct LockRequest {
    int32_t transaction_id;
    std::string table_name;
    int32_t record_id;
    LockMode mode;
    std::chrono::milliseconds timeout;
    std::chrono::steady_clock::time_point request_time;

    LockRequest(int32_t tx_id, const std::string& table, int32_t record,
               LockMode lock_mode, std::chrono::milliseconds lock_timeout)
        : transaction_id(tx_id), table_name(table), record_id(record),
          mode(lock_mode), timeout(lock_timeout),
          request_time(std::chrono::steady_clock::now()) {}
};

// 锁持有者信息
struct LockHolder {
    int32_t transaction_id;
    LockMode mode;
    std::chrono::steady_clock::time_point acquire_time;
    int32_t reference_count; // 引用计数，用于共享锁

    LockHolder(int32_t tx_id, LockMode lock_mode)
        : transaction_id(tx_id), mode(lock_mode),
          acquire_time(std::chrono::steady_clock::now()),
          reference_count(1) {}
};

// 并发访问控制配置
struct ConcurrencyControlConfig {
    IsolationLevel default_isolation_level = READ_COMMITTED;
    std::chrono::milliseconds default_lock_timeout = std::chrono::milliseconds(5000);
    std::chrono::milliseconds deadlock_detection_interval = std::chrono::milliseconds(1000);
    size_t max_lock_table_size = 10000;  // 最大锁表大小
    bool enable_deadlock_detection = true;
    bool enable_lock_escalation = true;  // 启用锁升级
    size_t lock_escalation_threshold = 100; // 锁升级阈值
};

// 死锁检测器
class DeadlockDetector {
public:
    DeadlockDetector();
    ~DeadlockDetector() = default;

    // 死锁检测
    bool DetectDeadlock(int32_t transaction_id, std::vector<int32_t>& deadlock_chain);
    void AddWaitFor(int32_t waiter, int32_t holder);
    void RemoveWaitFor(int32_t waiter, int32_t holder);
    void RemoveTransaction(int32_t transaction_id);

private:
    std::unordered_map<int32_t, std::unordered_set<int32_t>> wait_for_graph_;
    mutable std::mutex graph_mutex_;

    bool HasCycle(int32_t start_tx, std::vector<int32_t>& path,
                 std::unordered_set<int32_t>& visited);
};

// 锁管理器
class LockManager {
public:
    LockManager(std::shared_ptr<TransactionManager> transaction_manager);
    ~LockManager() = default;

    // 锁操作
    LockState AcquireLock(const LockRequest& request);
    LockState ReleaseLock(int32_t transaction_id, const std::string& table_name, int32_t record_id);
    LockState ReleaseAllLocks(int32_t transaction_id);

    // 锁查询
    bool HasLock(int32_t transaction_id, const std::string& table_name, int32_t record_id, LockMode mode) const;
    std::vector<LockHolder> GetLockHolders(const std::string& table_name, int32_t record_id) const;
    std::vector<LockRequest> GetWaitingLocks(int32_t transaction_id) const;

    // 锁升级
    LockState UpgradeLock(int32_t transaction_id, const std::string& table_name,
                         int32_t record_id, LockMode new_mode);

private:
    std::shared_ptr<TransactionManager> transaction_manager_;
    DeadlockDetector deadlock_detector_;

    // 锁表：table_name -> record_id -> lock_holders
    std::unordered_map<std::string,
        std::unordered_map<int32_t, std::vector<LockHolder>>> lock_table_;

    // 等待队列：transaction_id -> waiting_locks
    std::unordered_map<int32_t, std::vector<LockRequest>> waiting_locks_;

    mutable std::shared_mutex lock_table_mutex_;
    mutable std::mutex waiting_mutex_;

    // 锁兼容性检查
    bool IsLockCompatible(LockMode existing_mode, LockMode requested_mode) const;
    bool CanGrantLock(const std::vector<LockHolder>& holders, LockMode requested_mode) const;

    // 锁清理
    void CleanupExpiredLocks();
    void NotifyWaitingTransactions(const std::string& table_name, int32_t record_id);
};

// MVCC版本管理器
class MVCCVersionManager {
public:
    MVCCVersionManager(std::shared_ptr<StorageEngine> storage_engine);
    ~MVCCVersionManager() = default;

    // 版本管理
    uint64_t GetNextVersion();
    bool IsVersionVisible(uint64_t version, int32_t transaction_id, IsolationLevel isolation_level) const;
    std::vector<uint64_t> GetVisibleVersions(int32_t transaction_id, IsolationLevel isolation_level) const;

    // 版本清理
    void CleanupOldVersions(uint64_t min_active_version);
    size_t GetActiveTransactionCount() const;

private:
    std::shared_ptr<StorageEngine> storage_engine_;
    std::atomic<uint64_t> current_version_{0};
    mutable std::mutex version_mutex_;
};

// 并发访问验证器主类
class ConcurrentAccessValidator {
public:
    ConcurrentAccessValidator(std::shared_ptr<StorageEngine> storage_engine,
                            std::shared_ptr<TransactionManager> transaction_manager);
    ~ConcurrentAccessValidator() = default;

    // 并发访问验证
    LockState ValidateConcurrentAccess(int32_t transaction_id,
                                     const std::string& table_name,
                                     int32_t record_id,
                                     AccessControlType access_type);

    // 事务管理
    void BeginTransaction(int32_t transaction_id, IsolationLevel isolation_level);
    void CommitTransaction(int32_t transaction_id);
    void RollbackTransaction(int32_t transaction_id);

    // 锁管理
    LockState AcquireRecordLock(int32_t transaction_id, const std::string& table_name,
                              int32_t record_id, LockMode mode,
                              std::chrono::milliseconds timeout = std::chrono::milliseconds(5000));
    LockState ReleaseRecordLock(int32_t transaction_id, const std::string& table_name, int32_t record_id);

    // MVCC版本验证
    bool ValidateMVCCVersion(const std::string& table_name, int32_t record_id,
                           int32_t transaction_id, uint64_t expected_version);

    // 配置管理
    void SetConcurrencyConfig(const ConcurrencyControlConfig& config);
    const ConcurrencyControlConfig& GetConcurrencyConfig() const;

    // 统计信息
    struct ConcurrencyStats {
        size_t total_access_validations = 0;
        size_t successful_validations = 0;
        size_t lock_conflicts = 0;
        size_t deadlocks_detected = 0;
        size_t lock_timeouts = 0;
        double average_lock_wait_time_ms = 0.0;
        size_t active_transactions = 0;
        std::chrono::steady_clock::time_point last_validation_time;
    };

    ConcurrencyStats GetConcurrencyStats() const;

private:
    std::shared_ptr<StorageEngine> storage_engine_;
    std::shared_ptr<TransactionManager> transaction_manager_;

    LockManager lock_manager_;
    MVCCVersionManager mvcc_manager_;
    DeadlockDetector deadlock_detector_;

    ConcurrencyControlConfig config_;

    // 事务上下文
    struct TransactionContext {
        int32_t transaction_id;
        IsolationLevel isolation_level;
        std::chrono::steady_clock::time_point start_time;
        std::vector<std::tuple<std::string, int32_t, LockMode>> held_locks;
    };

    std::unordered_map<int32_t, TransactionContext> active_transactions_;
    mutable std::shared_mutex transaction_mutex_;

    // 统计信息
    ConcurrencyStats stats_;
    std::vector<double> lock_wait_times_; // 锁等待时间记录
    mutable std::mutex stats_mutex_;

    // 私有辅助方法
    IsolationLevel GetTransactionIsolationLevel(int32_t transaction_id) const;
    bool IsAccessAllowed(int32_t transaction_id, const std::string& table_name,
                        int32_t record_id, AccessControlType access_type) const;

    void UpdateConcurrencyStats(LockState result, std::chrono::milliseconds wait_time);
    void CleanupTransactionContext(int32_t transaction_id);
};

// 并发访问验证器工厂
class ConcurrentAccessValidatorFactory {
public:
    static std::shared_ptr<ConcurrentAccessValidator> CreateBasicValidator(
        std::shared_ptr<StorageEngine> storage_engine,
        std::shared_ptr<TransactionManager> transaction_manager);

    static std::shared_ptr<ConcurrentAccessValidator> CreateStrictValidator(
        std::shared_ptr<StorageEngine> storage_engine,
        std::shared_ptr<TransactionManager> transaction_manager);

    static std::shared_ptr<ConcurrentAccessValidator> CreateEnterpriseValidator(
        std::shared_ptr<StorageEngine> storage_engine,
        std::shared_ptr<TransactionManager> transaction_manager,
        const ConcurrencyControlConfig& config);
};

// 并发访问验证结果格式化器
class ConcurrentAccessResultFormatter {
public:
    static std::string FormatLockState(LockState state);
    static std::string FormatAccessControlType(AccessControlType type);
    static std::string FormatLockMode(LockMode mode);
    static std::string FormatIsolationLevel(IsolationLevel level);
    static bool IsCriticalError(LockState state);
};

} // namespace sqlcc

#endif // SQLCC_CONCURRENT_ACCESS_VALIDATOR_H
