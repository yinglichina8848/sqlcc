#include "transaction_manager.h"

#include <algorithm>
#include <thread>
#include <iostream>

namespace sqlcc {

// ================== 增强版StripeLockManager 实现 ==================

StripeLockManager::StripeLockManager(size_t stripe_count, bool enable_deadlock_detection)
    : stripe_count_(stripe_count), enable_deadlock_detection_(enable_deadlock_detection),
      metrics_({0, 0, 0, 0, std::chrono::milliseconds(0), std::chrono::milliseconds(0)}) {

    // 确保stripe_count是2的幂
    if (stripe_count < 1 || (stripe_count & (stripe_count - 1)) != 0) {
        // 找到最接近的2的幂
        stripe_count_ = 1;
        while (stripe_count_ < stripe_count) {
            stripe_count_ <<= 1;
        }
    }

    // 初始化stripe锁和相关数据结构
    stripe_locks_.resize(stripe_count_);
    for (size_t i = 0; i < stripe_count_; ++i) {
        stripe_locks_[i] = std::make_unique<std::mutex>();
    }
    locked_keys_.resize(stripe_count_);
    lock_owners_.resize(stripe_count_, -1);

    // 初始化死锁检测相关数据结构（如果启用了）
    if (enable_deadlock_detection_) {
        waiting_for_locks_.resize(stripe_count_);
    }
}

bool StripeLockManager::AcquireWriteLock(const std::string& key, TransactionId txn_id,
                                         std::chrono::milliseconds timeout_ms) {
    auto wait_start = std::chrono::steady_clock::now();
    size_t stripe_index = GetStripeIndex(key);

    // 创建unique_lock，不立即锁定
    std::unique_lock<std::mutex> stripe_lock(*stripe_locks_[stripe_index], std::defer_lock);

    // 尝试获取锁（简化超时逻辑）
    auto timeout_end = std::chrono::steady_clock::now() + timeout_ms;
    bool lock_acquired = false;

    // 首先尝试非阻塞获取锁
    if (stripe_lock.try_lock()) {
        lock_acquired = true;
    } else {
        // 如果立即获取失败，则轮询直到超时
        while (std::chrono::steady_clock::now() < timeout_end) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 短暂等待
            if (stripe_lock.try_lock()) {
                lock_acquired = true;
                break;
            }
        }

        if (!lock_acquired) {
            std::unique_lock<std::mutex> metrics_lock(metrics_mutex_);
            metrics_.lock_timeouts++;
            return false; // 超时
        }
    }

    // 检查锁是否已经被其他事务持有
    if (locked_keys_[stripe_index].find(key) != locked_keys_[stripe_index].end()) {
        // 同事务持有锁是可以的
        if (lock_owners_[stripe_index] == txn_id) {
            return true;
        }

        // 其他事务持有锁，需要等待
        metrics_.lock_conflicts++;

        if (enable_deadlock_detection_) {
            // 添加到等待图中进行死锁检测
            LockWait wait_info{txn_id, lock_owners_[stripe_index], key, wait_start};
            waiting_for_locks_[stripe_index][key] = wait_info;

            // 立即检查死锁
            bool deadlock_detected = HasDeadlock(txn_id);
            if (deadlock_detected) {
                std::unique_lock<std::mutex> metrics_lock(metrics_mutex_);
                metrics_.deadlocks_detected++;
                waiting_for_locks_[stripe_index].erase(key); // 移除等待项
                return false; // 检测到死锁
            }
        }

        // 尝试有限次的重新获取锁
        const int max_retries = 3;
        for (int retries = 0; retries < max_retries; ++retries) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            if (locked_keys_[stripe_index].find(key) == locked_keys_[stripe_index].end()) {
                // 锁已释放，可以获取
                break;
            }

            if (retries == max_retries - 1) {
                // 等待超时
                if (enable_deadlock_detection_) {
                    waiting_for_locks_[stripe_index].erase(key);
                }
                std::unique_lock<std::mutex> metrics_lock(metrics_mutex_);
                metrics_.lock_timeouts++;
                return false;
            }
        }

        // 从等待图中移除
        if (enable_deadlock_detection_) {
            waiting_for_locks_[stripe_index].erase(key);
        }
    }

    // 获取成功 - 记录锁信息
    locked_keys_[stripe_index].insert(key);
    lock_owners_[stripe_index] = txn_id;

    // 更新性能指标
    auto wait_time = std::chrono::steady_clock::now() - wait_start;
    {
        std::unique_lock<std::mutex> metrics_lock(metrics_mutex_);
        metrics_.total_locks++;
        metrics_.total_lock_wait_time += std::chrono::duration_cast<std::chrono::milliseconds>(wait_time);
        if (metrics_.total_locks > 0) {
            metrics_.avg_lock_wait_time = std::chrono::milliseconds(
                metrics_.total_lock_wait_time.count() / metrics_.total_locks);
        }
    }

    return true;
}

bool StripeLockManager::ReleaseWriteLock(const std::string& key, TransactionId txn_id) {
    size_t stripe_index = GetStripeIndex(key);
    std::unique_lock<std::mutex> lock(*stripe_locks_[stripe_index]);

    // 检查键是否存在且被当前事务锁定
    auto it = locked_keys_[stripe_index].find(key);
    if (it == locked_keys_[stripe_index].end()) {
        return false; // 键未被锁定
    }

    if (lock_owners_[stripe_index] != txn_id) {
        return false; // 并非由当前事务锁定
    }

    // 释放锁
    locked_keys_[stripe_index].erase(it);
    lock_owners_[stripe_index] = -1;

    // 从等待图中移除相关等待项
    if (enable_deadlock_detection_) {
        waiting_for_locks_[stripe_index].erase(key);
    }

    return true;
}

bool StripeLockManager::IsLocked(const std::string& key) const {
    size_t stripe_index = GetStripeIndex(key);
    // 使用const_cast是因为我们只读取数据，不会修改
    std::unique_lock<std::mutex> lock(*const_cast<std::unique_ptr<std::mutex>&>(stripe_locks_[stripe_index]));
    return locked_keys_[stripe_index].find(key) != locked_keys_[stripe_index].end();
}

bool StripeLockManager::HasDeadlock(TransactionId txn_id) const {
    if (!enable_deadlock_detection_) return false;

    // 实现Wait-for-Graph死锁检测算法
    // 使用DFS检测是否有循环依赖

    std::unordered_set<TransactionId> visited;
    std::unordered_set<TransactionId> recursion_stack;

    return DetectDeadlockCycle(txn_id, visited, recursion_stack);
}

bool StripeLockManager::DetectDeadlockCycle(TransactionId txn_id,
                                           std::unordered_set<TransactionId>& visited,
                                           std::unordered_set<TransactionId>& recursion_stack) const {

    // 标记当前事务已访问和在递归栈中
    visited.insert(txn_id);
    recursion_stack.insert(txn_id);

    // 检查所有等待关系
    for (size_t stripe_index = 0; stripe_index < stripe_count_; ++stripe_index) {
        // 这里需要临时访问等待图，这是一个简化实现
        // 在真实实现中，需要同步访问等待图数据
        auto it = waiting_for_locks_[stripe_index].begin();
        while (it != waiting_for_locks_[stripe_index].end()) {
            const LockWait& wait_info = it->second;
            if (wait_info.waiter == txn_id) {
                TransactionId holder = wait_info.holder;

                // 如果持有者正在等待但还未标记，递归检测
                if (recursion_stack.find(holder) != recursion_stack.end()) {
                    return true; // 发现死锁
                }

                if (visited.find(holder) == visited.end() &&
                    DetectDeadlockCycle(holder, visited, recursion_stack)) {
                    return true; // 发现死锁
                }
            }
            ++it;
        }
    }

    // 移除当前事务，从递归栈中弹出
    recursion_stack.erase(txn_id);
    return false;
}

std::vector<StripeLockManager::LockWait> StripeLockManager::GetWaitGraph(TransactionId txn_id) const {
    if (!enable_deadlock_detection_) {
        return {};
    }

    std::vector<LockWait> result;

    for (size_t stripe_index = 0; stripe_index < stripe_count_; ++stripe_index) {
        auto it = waiting_for_locks_[stripe_index].begin();
        while (it != waiting_for_locks_[stripe_index].end()) {
            const LockWait& wait_info = it->second;
            if (wait_info.waiter == txn_id || wait_info.holder == txn_id) {
                result.push_back(wait_info);
            }
            ++it;
        }
    }

    return result;
}

void StripeLockManager::PrintWaitGraph() const {
    if (!enable_deadlock_detection_) {
        std::cout << "死锁检测功能已禁用" << std::endl;
        return;
    }

    std::cout << "\n=== 锁等待图 ===" << std::endl;
    for (size_t stripe_index = 0; stripe_index < stripe_count_; ++stripe_index) {
        if (!waiting_for_locks_[stripe_index].empty()) {
            std::cout << "Stripe " << stripe_index << ":" << std::endl;
            auto it = waiting_for_locks_[stripe_index].begin();
            while (it != waiting_for_locks_[stripe_index].end()) {
                const LockWait& wait_info = it->second;
                auto wait_time = std::chrono::steady_clock::now() - wait_info.wait_start_time;
                std::cout << "  Txn" << wait_info.waiter << " -> Txn" << wait_info.holder
                         << " (key: '" << wait_info.lock_key << "', wait: "
                         << std::chrono::duration_cast<std::chrono::milliseconds>(wait_time).count() << "ms)" << std::endl;
                ++it;
            }
        }
    }
    std::cout << "===============" << std::endl;
}

StripeLockManager::LockMetrics StripeLockManager::GetMetrics() const {
    std::unique_lock<std::mutex> lock(metrics_mutex_);
    return metrics_;
}

void StripeLockManager::ResetMetrics() {
    std::unique_lock<std::mutex> lock(metrics_mutex_);
    metrics_ = {0, 0, 0, 0, std::chrono::milliseconds(0), std::chrono::milliseconds(0)};
}

// ================== TransactionManager 实现 ==================

TransactionManager::TransactionManager(size_t stripe_count) 
    : next_txn_id_(1), global_version_(0), lock_manager_(stripe_count) {
}

TransactionId TransactionManager::BeginTransaction(IsolationLevel isolation_level) {
    std::unique_lock<std::mutex> lock(txn_mutex_);
    
    TransactionId txn_id = next_txn_id_++;
    TransactionInfo txn_info{
        txn_id,
        isolation_level,
        TransactionStatus::ACTIVE,
        global_version_, // 记录当前全局版本作为快照
        std::unordered_set<std::string>()
    };
    
    active_txns_[txn_id] = txn_info;
    return txn_id;
}

bool TransactionManager::CommitTransaction(TransactionId txn_id) {
    std::unique_lock<std::mutex> lock(txn_mutex_);
    
    auto it = active_txns_.find(txn_id);
    if (it == active_txns_.end() || it->second.status != TransactionStatus::ACTIVE) {
        return false;
    }
    
    // 释放所有锁定的键
    for (const auto& key : it->second.locked_keys) {
        lock.unlock(); // 释放txn_mutex，避免死锁
        lock_manager_.ReleaseWriteLock(key, txn_id);
        lock.lock();
    }
    
    // 更新事务状态
    it->second.status = TransactionStatus::COMMITTED;
    
    // 对于写事务，增加全局版本号
    if (!it->second.locked_keys.empty()) {
        global_version_.fetch_add(1, std::memory_order_relaxed);
    }
    
    // 从活跃事务中移除
    active_txns_.erase(it);
    
    return true;
}

bool TransactionManager::RollbackTransaction(TransactionId txn_id) {
    std::unique_lock<std::mutex> lock(txn_mutex_);
    
    auto it = active_txns_.find(txn_id);
    if (it == active_txns_.end() || it->second.status != TransactionStatus::ACTIVE) {
        return false;
    }
    
    // 释放所有锁定的键
    for (const auto& key : it->second.locked_keys) {
        lock.unlock(); // 释放txn_mutex，避免死锁
        lock_manager_.ReleaseWriteLock(key, txn_id);
        lock.lock();
    }
    
    // 更新事务状态
    it->second.status = TransactionStatus::ROLLED_BACK;
    
    // 从活跃事务中移除
    active_txns_.erase(it);
    
    return true;
}

bool TransactionManager::LockForWrite(TransactionId txn_id, const std::string& key) {
    std::unique_lock<std::mutex> lock(txn_mutex_);
    
    auto it = active_txns_.find(txn_id);
    if (it == active_txns_.end() || it->second.status != TransactionStatus::ACTIVE) {
        return false;
    }
    
    // 检查事务是否已经锁定了这个键
    if (it->second.locked_keys.find(key) != it->second.locked_keys.end()) {
        return true; // 已锁定，无需再次锁定
    }
    
    lock.unlock(); // 释放txn_mutex，避免死锁
    
    // 尝试获取写锁
    bool locked = lock_manager_.AcquireWriteLock(key, txn_id);
    
    if (locked) {
        lock.lock();
        // 更新事务锁定的键列表
        it = active_txns_.find(txn_id); // 需要重新查找，因为锁被释放过
        if (it != active_txns_.end()) {
            it->second.locked_keys.insert(key);
        }
    }
    
    return locked;
}

bool TransactionManager::UnlockForWrite(TransactionId txn_id, const std::string& key) {
    std::unique_lock<std::mutex> lock(txn_mutex_);
    
    auto it = active_txns_.find(txn_id);
    if (it == active_txns_.end()) {
        return false;
    }
    
    // 检查事务是否锁定了这个键
    if (it->second.locked_keys.find(key) == it->second.locked_keys.end()) {
        return false;
    }
    
    // 从事务锁定的键列表中移除
    it->second.locked_keys.erase(key);
    
    lock.unlock(); // 释放txn_mutex，避免死锁
    
    // 释放写锁
    return lock_manager_.ReleaseWriteLock(key, txn_id);
}

IsolationLevel TransactionManager::GetIsolationLevel(TransactionId txn_id) const {
    std::unique_lock<std::mutex> lock(const_cast<std::mutex&>(txn_mutex_));
    
    auto it = active_txns_.find(txn_id);
    if (it == active_txns_.end()) {
        throw Exception("Transaction not found");
    }
    
    return it->second.isolation_level;
}

TransactionStatus TransactionManager::GetTransactionStatus(TransactionId txn_id) const {
    std::unique_lock<std::mutex> lock(const_cast<std::mutex&>(txn_mutex_));
    
    auto it = active_txns_.find(txn_id);
    if (it == active_txns_.end()) {
        throw Exception("Transaction not found");
    }
    
    return it->second.status;
}

uint64_t TransactionManager::GetTransactionSnapshot(TransactionId txn_id) const {
    std::unique_lock<std::mutex> lock(const_cast<std::mutex&>(txn_mutex_));
    
    auto it = active_txns_.find(txn_id);
    if (it == active_txns_.end()) {
        throw Exception("Transaction not found");
    }
    
    return it->second.snapshot_version;
}

}  // namespace sqlcc
