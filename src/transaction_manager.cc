#include "transaction_manager.h"
#include "exception.h"

#include <algorithm>
#include <thread>
#include <iostream>

namespace sqlcc {

// ================== TransactionManager 实现 ==================

TransactionManager::TransactionManager() 
    : next_txn_id_(1) {
}

TransactionId TransactionManager::begin_transaction(IsolationLevel isolation_level) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    TransactionId txn_id = next_txn_id_++;
    Transaction txn(txn_id, isolation_level);
    
    transactions_[txn_id] = txn;
    return txn_id;
}

bool TransactionManager::commit_transaction(TransactionId txn_id) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    auto it = transactions_.find(txn_id);
    if (it == transactions_.end() || it->second.state != TransactionState::ACTIVE) {
        return false;
    }
    
    // 释放所有锁定的资源
    release_all_locks(txn_id);
    
    // 更新事务状态
    it->second.state = TransactionState::COMMITTED;
    it->second.end_time = std::chrono::system_clock::now();
    
    // 从事务表中保留但标记为已完成
    // transactions_.erase(it); // 不立即删除，由cleanup_completed_transactions处理
    
    return true;
}

bool TransactionManager::rollback_transaction(TransactionId txn_id) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    auto it = transactions_.find(txn_id);
    if (it == transactions_.end() || it->second.state != TransactionState::ACTIVE) {
        return false;
    }
    
    // 释放所有锁定的资源
    release_all_locks(txn_id);
    
    // 更新事务状态
    it->second.state = TransactionState::ABORTED;
    it->second.end_time = std::chrono::system_clock::now();
    
    // 从事务表中保留但标记为已完成
    // transactions_.erase(it); // 不立即删除，由cleanup_completed_transactions处理
    
    return true;
}

// 注释掉未在头文件中定义的方法
// bool TransactionManager::LockForWrite(TransactionId txn_id, const std::string& key) {
//    std::unique_lock<std::mutex> lock(mutex_);
//    
//    auto it = transactions_.find(txn_id);
//    if (it == transactions_.end() || it->second.state != TransactionState::ACTIVE) {
//        return false;
//    }
//    
//    // 使用标准的acquire_lock方法替代
//    return acquire_lock(txn_id, key, LockType::EXCLUSIVE);
// }

// 注释掉未在头文件中定义的方法
// bool TransactionManager::UnlockForWrite(TransactionId txn_id, const std::string& key) {
//    std::unique_lock<std::mutex> lock(mutex_);
//    
//    auto it = transactions_.find(txn_id);
//    if (it == transactions_.end()) {
//        return false;
//    }
//    
//    // 使用标准的release_lock方法替代
//    release_lock(txn_id, key);
//    return true;
// }

// 移除未在头文件中定义的方法
// IsolationLevel TransactionManager::GetIsolationLevel(TransactionId txn_id) const {
//    std::unique_lock<std::mutex> lock(const_cast<std::mutex&>(txn_mutex_));
//    
//    auto it = active_txns_.find(txn_id);
//    if (it == active_txns_.end()) {
//        throw Exception("Transaction not found");
//    }
//    
//    return it->second.isolation_level;
// }

TransactionState TransactionManager::get_transaction_state(TransactionId txn_id) const {
    std::unique_lock<std::mutex> lock(const_cast<std::mutex&>(mutex_));
    
    auto it = transactions_.find(txn_id);
    if (it == transactions_.end()) {
        throw Exception("Transaction not found");
    }
    
    return it->second.state;
}

}  // namespace sqlcc
