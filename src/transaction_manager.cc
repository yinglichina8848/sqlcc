#include "transaction_manager.h"
#include "exception.h"

#include <algorithm>
#include <thread>
#include <iostream>

namespace sqlcc {

// ================== Transaction 实现 ==================

Transaction::Transaction(TransactionId id, IsolationLevel level)
    : txn_id(id), isolation_level(level), state(TransactionState::ACTIVE) {
    start_time = std::chrono::system_clock::now();
}

// ================== TransactionManager 实现 ==================

TransactionManager::TransactionManager() 
    : next_txn_id_(1) {
    // 成员变量在构造函数中初始化
}

sqlcc::TransactionId TransactionManager::begin_transaction(IsolationLevel isolation_level) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    sqlcc::TransactionId txn_id = next_txn_id_++;
    Transaction txn(txn_id, isolation_level);
    
    transactions_[txn_id] = txn;
    return txn_id;
}

bool TransactionManager::commit_transaction(sqlcc::TransactionId txn_id) {
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

// 实现缺失的析构函数
TransactionManager::~TransactionManager() {
    // 清理操作
}

// 实现缺失的release_all_locks方法
void TransactionManager::release_all_locks(TransactionId txn_id) {
    // 简单实现：遍历锁表，释放该事务持有的所有锁
    for (auto& [resource, locks] : lock_table_) {
        locks.erase(
            std::remove_if(locks.begin(), locks.end(),
                           [txn_id](const LockEntry& entry) {
                               return entry.txn_id == txn_id;
                           }),
            locks.end());
        
        // 如果资源没有锁了，从锁表中删除
        if (locks.empty()) {
            lock_table_.erase(resource);
        }
    }
    
    // 清理等待图
    wait_graph_.erase(txn_id);
    for (auto& [tid, waiting_for] : wait_graph_) {
        waiting_for.erase(txn_id);
    }
}

// 实现acquire_lock方法
bool TransactionManager::acquire_lock(TransactionId txn_id, const std::string& resource, LockType lock_type, bool wait) {
  std::unique_lock<std::mutex> lock(mutex_);

  // 检查事务是否存在且活跃
  if (transactions_.find(txn_id) == transactions_.end() || 
      transactions_[txn_id].state != TransactionState::ACTIVE) {
    return false;
  }

  // 检查是否已经持有该资源的锁
  if (lock_table_.contains(resource)) {
    for (const auto& lock_entry : lock_table_[resource]) {
      if (lock_entry.txn_id == txn_id) {
        // 已经持有锁
        if (lock_entry.type == lock_type) {
          // 已持有相同类型的锁
          return true;
        }
        
        // 尝试锁升级
        if (lock_type == LockType::EXCLUSIVE && lock_entry.type == LockType::SHARED) {
          // 检查是否可以升级（没有其他事务持有锁）
          bool can_upgrade = true;
          for (const auto& other_entry : lock_table_[resource]) {
            if (other_entry.txn_id != txn_id) {
              can_upgrade = false;
              break;
            }
          }
          
          if (can_upgrade) {
            // 移除原锁
            lock_table_[resource].erase(
              std::remove_if(lock_table_[resource].begin(), lock_table_[resource].end(),
                [txn_id](const LockEntry& entry) { return entry.txn_id == txn_id; }),
              lock_table_[resource].end());
            
            // 添加排他锁
            LockEntry new_lock;
            new_lock.txn_id = txn_id;
            new_lock.resource = resource;
            new_lock.type = LockType::EXCLUSIVE;
            new_lock.acquired_time = std::chrono::system_clock::now();
            lock_table_[resource].push_back(new_lock);
            return true;
          }
          
          if (wait) {
            // 记录等待关系
            for (const auto& other_entry : lock_table_[resource]) {
              if (other_entry.txn_id != txn_id) {
                wait_graph_[txn_id].insert(other_entry.txn_id);
              }
            }
          }
          return false;
        }
        
        // 已持有更高级别的锁
        if (lock_entry.type == LockType::EXCLUSIVE && lock_type == LockType::SHARED) {
          return true;
        }
      }
    }
  }

  // 新锁获取逻辑
  if (lock_type == LockType::SHARED) {
    // 共享锁兼容性：只有排他锁不兼容
    bool has_exclusive_lock = false;
    if (lock_table_.contains(resource)) {
      for (const auto& lock_entry : lock_table_[resource]) {
        if (lock_entry.type == LockType::EXCLUSIVE) {
          has_exclusive_lock = true;
          if (wait) {
            wait_graph_[txn_id].insert(lock_entry.txn_id);
          }
        }
      }
    }
    
    if (!has_exclusive_lock) {
      // 添加共享锁
      LockEntry new_lock;
      new_lock.txn_id = txn_id;
      new_lock.resource = resource;
      new_lock.type = LockType::SHARED;
      new_lock.acquired_time = std::chrono::system_clock::now();
      lock_table_[resource].push_back(new_lock);
      return true;
    }
    return false;
  } else if (lock_type == LockType::EXCLUSIVE) {
    // 排他锁检查：不能有任何其他锁
    if (lock_table_.contains(resource) && !lock_table_[resource].empty()) {
      if (wait) {
        for (const auto& lock_entry : lock_table_[resource]) {
          wait_graph_[txn_id].insert(lock_entry.txn_id);
        }
      }
      return false;
    }
    
    // 添加排他锁
    LockEntry new_lock;
    new_lock.txn_id = txn_id;
    new_lock.resource = resource;
    new_lock.type = LockType::EXCLUSIVE;
    new_lock.acquired_time = std::chrono::system_clock::now();
    lock_table_[resource].push_back(new_lock);
    return true;
  }
  
  return false;
}

// 实现release_lock方法
void TransactionManager::release_lock(TransactionId txn_id, const std::string &resource) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    // 检查资源是否有锁
    if (lock_table_.contains(resource)) {
        auto& locks = lock_table_[resource];
        
        // 查找并移除该事务对该资源的锁
        auto it = std::remove_if(locks.begin(), locks.end(),
                                 [txn_id](const LockEntry& entry) {
                                     return entry.txn_id == txn_id;
                                 });
        
        if (it != locks.end()) {
            locks.erase(it, locks.end());
            
            // 如果资源没有锁了，从锁表中删除
            if (locks.empty()) {
                lock_table_.erase(resource);
            }
        }
    }
    
    // 清理等待图
    wait_graph_.erase(txn_id);
    for (auto& [tid, waiting_for] : wait_graph_) {
        waiting_for.erase(txn_id);
    }
}

// 实现get_active_transactions方法
std::vector<sqlcc::TransactionId> TransactionManager::get_active_transactions() const {
    std::unique_lock<std::mutex> lock(const_cast<std::mutex&>(mutex_));
    
    std::vector<sqlcc::TransactionId> active_txns;
    
    // 遍历所有事务，收集活动状态的事务ID
    for (const auto& [txn_id, txn] : transactions_) {
        if (txn.state == TransactionState::ACTIVE) {
            active_txns.push_back(txn_id);
        }
    }
    
    return active_txns;
}

// 实现can_acquire_lock方法
bool TransactionManager::can_acquire_lock(sqlcc::TransactionId txn_id, const std::string &resource, LockType lock_type) const {
    // 检查资源是否有锁
    if (lock_table_.contains(resource)) {
        for (const auto& entry : lock_table_.at(resource)) {
            // 如果是同一个事务，不需要检查
            if (entry.txn_id == txn_id) {
                continue;
            }
            
            // 如果请求的是排他锁，但资源已经有锁，无法获取
            if (lock_type == LockType::EXCLUSIVE) {
                return false;
            }
            
            // 如果请求的是共享锁，但资源已经有排他锁，无法获取
            if (lock_type == LockType::SHARED && entry.type == LockType::EXCLUSIVE) {
                return false;
            }
        }
    }
    
    return true;
}

// 实现create_savepoint方法
bool TransactionManager::create_savepoint(sqlcc::TransactionId txn_id, const std::string & /* savepoint_name */) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    // 检查事务是否存在且处于活动状态
    auto it = transactions_.find(txn_id);
    if (it == transactions_.end() || it->second.state != TransactionState::ACTIVE) {
        return false;
    }
    
    // 简化实现：因为Transaction结构体中没有savepoints成员
    // 这里只是简单地返回成功，在实际实现中应该在Transaction中添加保存点支持
    return true;
}

// 实现log_operation方法
void TransactionManager::log_operation(sqlcc::TransactionId txn_id, const LogEntry & /* entry */) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    // 检查事务是否存在
    auto it = transactions_.find(txn_id);
    if (it == transactions_.end()) {
        return;
    }
    
    // 简化实现：因为Transaction结构体中没有log_entries成员
    // 这里只是简单地不做任何操作，在实际实现中应该在Transaction中添加日志支持
}

// 实现detect_deadlock方法
bool TransactionManager::detect_deadlock(sqlcc::TransactionId /* txn_id */) {
    // 简化实现：直接返回false，表示默认没有检测到死锁
    return false;
}

// 实现next_transaction_id方法
sqlcc::TransactionId TransactionManager::next_transaction_id() {
    std::unique_lock<std::mutex> lock(mutex_);
    return next_txn_id_++;
}

// 实现rollback_to_savepoint方法
bool TransactionManager::rollback_to_savepoint(sqlcc::TransactionId /* txn_id */, const std::string & /* savepoint_name */) {
    // 简化实现：当前版本暂不支持保存点功能
    return false;
}

}  // namespace sqlcc
