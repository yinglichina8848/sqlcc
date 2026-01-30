/**
 * @file concurrent_access_validator.cpp
 * @brief 并发访问验证器实现 - 实现记录操作的并发访问控制
 *
 * 该文件实现了并发访问验证器的核心功能，包括：
 * - 锁管理器和锁兼容性检查
 * - 死锁检测算法
 * - MVCC版本管理
 * - 事务隔离级别控制
 */

#include "src/storage/concurrent_access_validator.h"
#include "src/storage_engine/table_storage.h"
#include "src/exception/exception.h"
#include "src/utils/logger.h"
#include <algorithm>
#include <queue>
#include <stack>
#include <thread>

namespace sqlcc {

// 死锁检测器实现
DeadlockDetector::DeadlockDetector() = default;

bool DeadlockDetector::DetectDeadlock(int32_t transaction_id, std::vector<int32_t>& deadlock_chain) {
    (void)deadlock_chain; // 避免未使用参数警告
    std::lock_guard<std::mutex> lock(graph_mutex_);
    std::unordered_set<int32_t> visited;
    std::vector<int32_t> path;

    return HasCycle(transaction_id, path, visited);
}

void DeadlockDetector::AddWaitFor(int32_t waiter, int32_t holder) {
    std::lock_guard<std::mutex> lock(graph_mutex_);
    wait_for_graph_[waiter].insert(holder);
}

void DeadlockDetector::RemoveWaitFor(int32_t waiter, int32_t holder) {
    std::lock_guard<std::mutex> lock(graph_mutex_);
    auto it = wait_for_graph_.find(waiter);
    if (it != wait_for_graph_.end()) {
        it->second.erase(holder);
        if (it->second.empty()) {
            wait_for_graph_.erase(it);
        }
    }
}

void DeadlockDetector::RemoveTransaction(int32_t transaction_id) {
    std::lock_guard<std::mutex> lock(graph_mutex_);
    wait_for_graph_.erase(transaction_id);

    // 移除所有指向此事务的边
    for (auto& pair : wait_for_graph_) {
        pair.second.erase(transaction_id);
    }
}

bool DeadlockDetector::HasCycle(int32_t start_tx, std::vector<int32_t>& path,
                               std::unordered_set<int32_t>& visited) {
    if (visited.find(start_tx) != visited.end()) {
        // 找到环
        auto it = std::find(path.begin(), path.end(), start_tx);
        if (it != path.end()) {
            path.erase(path.begin(), it + 1);
            return true;
        }
        return false;
    }

    visited.insert(start_tx);
    path.push_back(start_tx);

    auto it = wait_for_graph_.find(start_tx);
    if (it != wait_for_graph_.end()) {
        for (int32_t neighbor : it->second) {
            if (HasCycle(neighbor, path, visited)) {
                return true;
            }
        }
    }

    path.pop_back();
    return false;
}

// 锁管理器实现
LockManager::LockManager(std::shared_ptr<TransactionManager> transaction_manager)
    : transaction_manager_(transaction_manager) {
}

LockState LockManager::AcquireLock(const LockRequest& request) {
    std::unique_lock<std::shared_mutex> lock(lock_table_mutex_);

    // 检查锁兼容性
    auto& record_locks = lock_table_[request.table_name][request.record_id];
    if (!CanGrantLock(record_locks, request.mode)) {
        // 锁冲突，加入等待队列
        {
            std::lock_guard<std::mutex> wait_lock(waiting_mutex_);
            waiting_locks_[request.transaction_id].push_back(request);
        }

        // 添加到等待图中
        for (const auto& holder : record_locks) {
            if (holder.transaction_id != request.transaction_id) {
                deadlock_detector_.AddWaitFor(request.transaction_id, holder.transaction_id);
            }
        }

        // 死锁检测
        std::vector<int32_t> deadlock_chain;
        if (deadlock_detector_.DetectDeadlock(request.transaction_id, deadlock_chain)) {
            // 死锁检测到，回滚事务
            deadlock_detector_.RemoveTransaction(request.transaction_id);
            return LOCK_DEADLOCK;
        }

        return LOCK_WAITING;
    }

    // 授予锁
    auto it = std::find_if(record_locks.begin(), record_locks.end(),
                          [request](const LockHolder& holder) {
                              return holder.transaction_id == request.transaction_id;
                          });

    if (it != record_locks.end()) {
        // 已持有锁，增加引用计数
        it->reference_count++;
    } else {
        // 新锁持有者
        record_locks.emplace_back(request.transaction_id, request.mode);
    }

    return LOCK_GRANTED;
}

LockState LockManager::ReleaseLock(int32_t transaction_id, const std::string& table_name, int32_t record_id) {
    std::unique_lock<std::shared_mutex> lock(lock_table_mutex_);

    auto table_it = lock_table_.find(table_name);
    if (table_it == lock_table_.end()) {
        return LOCK_CONFLICT;
    }

    auto record_it = table_it->second.find(record_id);
    if (record_it == table_it->second.end()) {
        return LOCK_CONFLICT;
    }

    auto& holders = record_it->second;
    auto holder_it = std::find_if(holders.begin(), holders.end(),
                                 [transaction_id](const LockHolder& holder) {
                                     return holder.transaction_id == transaction_id;
                                 });

    if (holder_it == holders.end()) {
        return LOCK_CONFLICT;
    }

    // 减少引用计数
    holder_it->reference_count--;
    if (holder_it->reference_count <= 0) {
        holders.erase(holder_it);
    }

    // 如果没有锁持有者了，清理记录
    if (holders.empty()) {
        table_it->second.erase(record_it);
        if (table_it->second.empty()) {
            lock_table_.erase(table_it);
        }
    }

    // 移除等待图中的边
    deadlock_detector_.RemoveTransaction(transaction_id);

    // 通知等待的事务
    NotifyWaitingTransactions(table_name, record_id);

    return LOCK_GRANTED;
}

LockState LockManager::ReleaseAllLocks(int32_t transaction_id) {
    std::unique_lock<std::shared_mutex> lock(lock_table_mutex_);

    // 收集所有该事务持有的锁
    std::vector<std::pair<std::string, int32_t>> locks_to_release;

    for (const auto& table_pair : lock_table_) {
        for (const auto& record_pair : table_pair.second) {
            const auto& holders = record_pair.second;
            if (std::any_of(holders.begin(), holders.end(),
                           [transaction_id](const LockHolder& holder) {
                               return holder.transaction_id == transaction_id;
                           })) {
                locks_to_release.emplace_back(table_pair.first, record_pair.first);
            }
        }
    }

    // 释放所有锁
    for (const auto& lock_info : locks_to_release) {
        ReleaseLock(transaction_id, lock_info.first, lock_info.second);
    }

    // 清理等待队列
    {
        std::lock_guard<std::mutex> wait_lock(waiting_mutex_);
        waiting_locks_.erase(transaction_id);
    }

    // 移除等待图中的事务
    deadlock_detector_.RemoveTransaction(transaction_id);

    return LOCK_GRANTED;
}

bool LockManager::HasLock(int32_t transaction_id, const std::string& table_name,
                         int32_t record_id, LockType mode) const {
    std::shared_lock<std::shared_mutex> lock(lock_table_mutex_);

    auto table_it = lock_table_.find(table_name);
    if (table_it == lock_table_.end()) {
        return false;
    }

    auto record_it = table_it->second.find(record_id);
    if (record_it == table_it->second.end()) {
        return false;
    }

    const auto& holders = record_it->second;
    return std::any_of(holders.begin(), holders.end(),
                      [transaction_id, mode](const LockHolder& holder) {
                          return holder.transaction_id == transaction_id && holder.mode == mode;
                      });
}

std::vector<LockHolder> LockManager::GetLockHolders(const std::string& table_name, int32_t record_id) const {
    std::shared_lock<std::shared_mutex> lock(lock_table_mutex_);

    auto table_it = lock_table_.find(table_name);
    if (table_it == lock_table_.end()) {
        return {};
    }

    auto record_it = table_it->second.find(record_id);
    if (record_it == table_it->second.end()) {
        return {};
    }

    return record_it->second;
}

std::vector<LockRequest> LockManager::GetWaitingLocks(int32_t transaction_id) const {
    std::lock_guard<std::mutex> lock(waiting_mutex_);
    auto it = waiting_locks_.find(transaction_id);
    return it != waiting_locks_.end() ? it->second : std::vector<LockRequest>{};
}

LockState LockManager::UpgradeLock(int32_t transaction_id, const std::string& table_name,
                                 int32_t record_id, LockType new_mode) {
    std::unique_lock<std::shared_mutex> lock(lock_table_mutex_);

    auto table_it = lock_table_.find(table_name);
    if (table_it == lock_table_.end()) {
        return LOCK_CONFLICT;
    }

    auto record_it = table_it->second.find(record_id);
    if (record_it == table_it->second.end()) {
        return LOCK_CONFLICT;
    }

    auto& holders = record_it->second;
    auto holder_it = std::find_if(holders.begin(), holders.end(),
                                 [transaction_id](const LockHolder& holder) {
                                     return holder.transaction_id == transaction_id;
                                 });

    if (holder_it == holders.end()) {
        return LOCK_CONFLICT;
    }

    // 检查是否可以升级
    LockType current_mode = holder_it->mode;
    if (current_mode == new_mode) {
        return LOCK_GRANTED; // 已经是目标模式
    }

    // 检查锁升级的兼容性
    holders.erase(holder_it); // 临时移除当前锁
    bool can_upgrade = CanGrantLock(holders, new_mode);
    holders.emplace_back(transaction_id, new_mode); // 重新添加

    if (can_upgrade) {
        holder_it = std::find_if(holders.begin(), holders.end(),
                                [transaction_id](const LockHolder& holder) {
                                    return holder.transaction_id == transaction_id;
                                });
        if (holder_it != holders.end()) {
            holder_it->mode = new_mode;
            return LOCK_GRANTED;
        }
    }

    return LOCK_CONFLICT;
}

bool LockManager::IsLockCompatible(LockType existing_mode, LockType requested_mode) const {
    // 锁兼容性矩阵
    static const bool compatibility_matrix[7][7] = {
        // 现有\请求    SHARED  EXCLUSIVE  UPDATE  INTENT_S  INTENT_E  SHARED_IE
        /* SHARED */    {true,   false,     false,  true,     false,    false},
        /* EXCLUSIVE */ {false,  false,     false,  false,    false,    false},
        /* UPDATE */    {false,  false,     false,  false,    false,    false},
        /* INTENT_S */  {true,   false,     false,  true,     false,    false},
        /* INTENT_E */  {false,  false,     false,  false,    false,    false},
        /* SHARED_IE */ {false,  false,     false,  false,    false,    false}
    };

    return compatibility_matrix[existing_mode - 1][requested_mode - 1];
}

bool LockManager::CanGrantLock(const std::vector<LockHolder>& holders, LockType requested_mode) const {
    for (const auto& holder : holders) {
        if (!IsLockCompatible(holder.mode, requested_mode)) {
            return false;
        }
    }
    return true;
}

void LockManager::CleanupExpiredLocks() {
    // 简化实现：实际应该检查锁的超时时间
    // 这里暂时为空实现
}

void LockManager::NotifyWaitingTransactions(const std::string& table_name, int32_t record_id) {
    (void)table_name; // 避免未使用参数警告
    (void)record_id; // 避免未使用参数警告
    // 简化实现：实际应该通知等待队列中的事务
    // 这里暂时为空实现
}

// MVCC版本管理器实现
MVCCVersionManager::MVCCVersionManager(std::shared_ptr<StorageEngine> storage_engine)
    : storage_engine_(storage_engine) {
}

uint64_t MVCCVersionManager::GetNextVersion() {
    return current_version_.fetch_add(1, std::memory_order_relaxed) + 1;
}

bool MVCCVersionManager::IsVersionVisible(uint64_t version, int32_t transaction_id,
                                        IsolationLevel isolation_level) const {
    (void)version; // 避免未使用参数警告
    (void)transaction_id; // 避免未使用参数警告
    (void)isolation_level; // 避免未使用参数警告
    // 简化的MVCC可见性检查
    // 实际实现应该考虑事务的开始时间、提交时间等

    switch (isolation_level) {
        case READ_UNCOMMITTED:
            return true; // 可以读取未提交的数据
        case READ_COMMITTED:
            // 简化：假设所有版本都可见
            return true;
        case REPEATABLE_READ:
            // 简化：假设所有版本都可见
            return true;
        case SERIALIZABLE:
            // 简化：假设所有版本都可见
            return true;
        default:
            return false;
    }
}

std::vector<uint64_t> MVCCVersionManager::GetVisibleVersions(int32_t transaction_id,
                                                           IsolationLevel isolation_level) const {
    // 简化的实现
    std::vector<uint64_t> versions;
    uint64_t current = current_version_.load(std::memory_order_relaxed);

    for (uint64_t v = 1; v <= current; ++v) {
        if (IsVersionVisible(v, transaction_id, isolation_level)) {
            versions.push_back(v);
        }
    }

    return versions;
}

void MVCCVersionManager::CleanupOldVersions(uint64_t min_active_version) {
    (void)min_active_version; // 避免未使用参数警告
    // 简化的版本清理
    // 实际应该清理不再需要的旧版本
}

size_t MVCCVersionManager::GetActiveTransactionCount() const {
    // 简化的实现
    return 0;
}

// 并发访问验证器主类实现
ConcurrentAccessValidator::ConcurrentAccessValidator(
    std::shared_ptr<StorageEngine> storage_engine,
    std::shared_ptr<TransactionManager> transaction_manager)
    : storage_engine_(storage_engine),
      transaction_manager_(transaction_manager),
      lock_manager_(transaction_manager),
      mvcc_manager_(storage_engine) {

    // 初始化统计信息
    stats_.last_validation_time = std::chrono::steady_clock::now();
}

LockState ConcurrentAccessValidator::ValidateConcurrentAccess(
    int32_t transaction_id, const std::string& table_name,
    int32_t record_id, AccessControlType access_type) {

    auto start_time = std::chrono::steady_clock::now();

    // 获取事务隔离级别
    IsolationLevel isolation_level = GetTransactionIsolationLevel(transaction_id);
    (void)isolation_level; // 避免未使用变量警告

    // 根据访问类型确定需要的锁模式
    LockType required_mode;
    switch (access_type) {
        case READ_ACCESS:
            required_mode = SHARED_LOCK;
            break;
        case WRITE_ACCESS:
        case EXCLUSIVE_ACCESS:
            required_mode = EXCLUSIVE_LOCK;
            break;
        case INTENT_READ:
            required_mode = INTENT_SHARED;
            break;
        case INTENT_WRITE:
            required_mode = INTENT_EXCLUSIVE;
            break;
        default:
            required_mode = SHARED_LOCK;
    }

    // 尝试获取锁
    LockState lock_result = AcquireRecordLock(transaction_id, table_name, record_id,
                                            required_mode, config_.default_lock_timeout);

    auto wait_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_time);

    UpdateConcurrencyStats(lock_result, wait_time);

    return lock_result;
}

void ConcurrentAccessValidator::BeginTransaction(int32_t transaction_id, IsolationLevel isolation_level) {
    std::unique_lock<std::shared_mutex> lock(transaction_mutex_);

    TransactionContext context;
    context.transaction_id = transaction_id;
    context.isolation_level = isolation_level;
    context.start_time = std::chrono::steady_clock::now();

    active_transactions_[transaction_id] = context;
    stats_.active_transactions++;
}

void ConcurrentAccessValidator::CommitTransaction(int32_t transaction_id) {
    // 释放事务持有的所有锁
    lock_manager_.ReleaseAllLocks(transaction_id);

    // 清理事务上下文
    CleanupTransactionContext(transaction_id);
}

void ConcurrentAccessValidator::RollbackTransaction(int32_t transaction_id) {
    // 释放事务持有的所有锁
    lock_manager_.ReleaseAllLocks(transaction_id);

    // 清理事务上下文
    CleanupTransactionContext(transaction_id);
}

LockState ConcurrentAccessValidator::AcquireRecordLock(int32_t transaction_id,
                                                     const std::string& table_name,
                                                     int32_t record_id,
                                                     LockType mode,
                                                     std::chrono::milliseconds timeout) {

    LockRequest request(transaction_id, table_name, record_id, mode, timeout);
    return lock_manager_.AcquireLock(request);
}

LockState ConcurrentAccessValidator::ReleaseRecordLock(int32_t transaction_id,
                                                     const std::string& table_name,
                                                     int32_t record_id) {

    return lock_manager_.ReleaseLock(transaction_id, table_name, record_id);
}

bool ConcurrentAccessValidator::ValidateMVCCVersion(const std::string& table_name,
                                                  int32_t record_id,
                                                  int32_t transaction_id,
                                                  uint64_t expected_version) {
    (void)table_name; // 避免未使用参数警告
    (void)record_id; // 避免未使用参数警告

    IsolationLevel isolation_level = GetTransactionIsolationLevel(transaction_id);
    return mvcc_manager_.IsVersionVisible(expected_version, transaction_id, isolation_level);
}

void ConcurrentAccessValidator::SetConcurrencyConfig(const ConcurrencyControlConfig& config) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    config_ = config;
}

const ConcurrencyControlConfig& ConcurrentAccessValidator::GetConcurrencyConfig() const {
    return config_;
}

ConcurrentAccessValidator::ConcurrencyStats ConcurrentAccessValidator::GetConcurrencyStats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

// 私有辅助方法实现
IsolationLevel ConcurrentAccessValidator::GetTransactionIsolationLevel(int32_t transaction_id) const {
    std::shared_lock<std::shared_mutex> lock(transaction_mutex_);

    auto it = active_transactions_.find(transaction_id);
    if (it != active_transactions_.end()) {
        return it->second.isolation_level;
    }

    return config_.default_isolation_level;
}

bool ConcurrentAccessValidator::IsAccessAllowed(int32_t transaction_id,
                                              const std::string& table_name,
                                              int32_t record_id,
                                              AccessControlType access_type) const {
    (void)transaction_id; // 避免未使用参数警告
    (void)table_name; // 避免未使用参数警告
    (void)record_id; // 避免未使用参数警告
    (void)access_type; // 避免未使用参数警告

    // 简化的访问控制检查
    // 实际应该根据事务隔离级别和锁状态进行检查

    return true;
}

void ConcurrentAccessValidator::UpdateConcurrencyStats(LockState result,
                                                     std::chrono::milliseconds wait_time) {

    std::lock_guard<std::mutex> lock(stats_mutex_);

    stats_.total_access_validations++;
    lock_wait_times_.push_back(wait_time.count());

    switch (result) {
        case LOCK_GRANTED:
            stats_.successful_validations++;
            break;
        case LOCK_CONFLICT:
            stats_.lock_conflicts++;
            break;
        case LOCK_DEADLOCK:
            stats_.deadlocks_detected++;
            break;
        case LOCK_TIMEOUT:
            stats_.lock_timeouts++;
            break;
        default:
            break;
    }

    // 计算平均锁等待时间
    if (!lock_wait_times_.empty()) {
        double sum = 0.0;
        for (double time : lock_wait_times_) {
            sum += time;
        }
        stats_.average_lock_wait_time_ms = sum / lock_wait_times_.size();
    }

    // 限制时间记录数量
    if (lock_wait_times_.size() > 1000) {
        lock_wait_times_.erase(lock_wait_times_.begin());
    }

    stats_.last_validation_time = std::chrono::steady_clock::now();
}

void ConcurrentAccessValidator::CleanupTransactionContext(int32_t transaction_id) {
    std::unique_lock<std::shared_mutex> lock(transaction_mutex_);

    auto it = active_transactions_.find(transaction_id);
    if (it != active_transactions_.end()) {
        active_transactions_.erase(it);
        if (stats_.active_transactions > 0) {
            stats_.active_transactions--;
        }
    }
}

// 并发访问验证器工厂实现
std::shared_ptr<ConcurrentAccessValidator> ConcurrentAccessValidatorFactory::CreateBasicValidator(
    std::shared_ptr<StorageEngine> storage_engine,
    std::shared_ptr<TransactionManager> transaction_manager) {

    return std::make_shared<ConcurrentAccessValidator>(storage_engine, transaction_manager);
}

std::shared_ptr<ConcurrentAccessValidator> ConcurrentAccessValidatorFactory::CreateStrictValidator(
    std::shared_ptr<StorageEngine> storage_engine,
    std::shared_ptr<TransactionManager> transaction_manager) {

    auto validator = std::make_shared<ConcurrentAccessValidator>(storage_engine, transaction_manager);

    ConcurrencyControlConfig config;
    config.default_isolation_level = SERIALIZABLE;
    config.enable_deadlock_detection = true;
    config.enable_lock_escalation = true;
    config.default_lock_timeout = std::chrono::milliseconds(3000);

    validator->SetConcurrencyConfig(config);

    return validator;
}

std::shared_ptr<ConcurrentAccessValidator> ConcurrentAccessValidatorFactory::CreateEnterpriseValidator(
    std::shared_ptr<StorageEngine> storage_engine,
    std::shared_ptr<TransactionManager> transaction_manager,
    const ConcurrencyControlConfig& config) {

    auto validator = std::make_shared<ConcurrentAccessValidator>(storage_engine, transaction_manager);
    validator->SetConcurrencyConfig(config);

    return validator;
}

// 并发访问验证结果格式化器实现
std::string ConcurrentAccessResultFormatter::FormatLockState(LockState state) {
    switch (state) {
        case LOCK_GRANTED: return "LOCK_GRANTED";
        case LOCK_WAITING: return "LOCK_WAITING";
        case LOCK_TIMEOUT: return "LOCK_TIMEOUT";
        case LOCK_DEADLOCK: return "LOCK_DEADLOCK";
        case LOCK_CONFLICT: return "LOCK_CONFLICT";
        default: return "UNKNOWN";
    }
}

std::string ConcurrentAccessResultFormatter::FormatAccessControlType(AccessControlType type) {
    switch (type) {
        case READ_ACCESS: return "READ_ACCESS";
        case WRITE_ACCESS: return "WRITE_ACCESS";
        case EXCLUSIVE_ACCESS: return "EXCLUSIVE_ACCESS";
        case INTENT_READ: return "INTENT_READ";
        case INTENT_WRITE: return "INTENT_WRITE";
        default: return "UNKNOWN";
    }
}

std::string ConcurrentAccessResultFormatter::FormatLockType(LockType mode) {
    switch (mode) {
        case SHARED_LOCK: return "SHARED_LOCK";
        case EXCLUSIVE_LOCK: return "EXCLUSIVE_LOCK";
        case UPDATE_LOCK: return "UPDATE_LOCK";
        case INTENT_SHARED: return "INTENT_SHARED";
        case INTENT_EXCLUSIVE: return "INTENT_EXCLUSIVE";
        case SHARED_INTENT_EXCLUSIVE: return "SHARED_INTENT_EXCLUSIVE";
        default: return "UNKNOWN";
    }
}

std::string ConcurrentAccessResultFormatter::FormatIsolationLevel(IsolationLevel level) {
    switch (level) {
        case READ_UNCOMMITTED: return "READ_UNCOMMITTED";
        case READ_COMMITTED: return "READ_COMMITTED";
        case REPEATABLE_READ: return "REPEATABLE_READ";
        case SERIALIZABLE: return "SERIALIZABLE";
        default: return "UNKNOWN";
    }
}

bool ConcurrentAccessResultFormatter::IsCriticalError(LockState state) {
    return state == LOCK_DEADLOCK || state == LOCK_TIMEOUT;
}

} // namespace sqlcc
