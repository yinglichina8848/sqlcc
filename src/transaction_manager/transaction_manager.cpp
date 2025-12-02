#include "transaction_manager.h"
#include <algorithm>
#include <condition_variable>
#include <iostream>
#include <queue>
#include <stack>
#include <thread>
#include <stdexcept>

namespace sqlcc {

// Transaction构造函数实现
Transaction::Transaction(TransactionId id, IsolationLevel level)
    : txn_id(id), isolation_level(level), state(TransactionState::ACTIVE),
      start_time(std::chrono::system_clock::now()) {}

// TransactionManager构造函数实现
TransactionManager::TransactionManager() : next_txn_id_(1ULL) {}

// 析构函数实现
TransactionManager::~TransactionManager() {}

TransactionId TransactionManager::next_transaction_id() {
    return next_txn_id_.fetch_add(1);
}

TransactionId TransactionManager::begin_transaction(IsolationLevel isolation_level) {
  std::unique_lock<std::mutex> lock(mutex_);
  TransactionId txn_id = next_transaction_id();
  Transaction txn(txn_id, isolation_level);
  transactions_[txn_id] = std::move(txn);

  std::cout << "Transaction " << txn_id << " started with isolation level "
            << static_cast<int>(isolation_level) << std::endl;

  return txn_id;
}

bool TransactionManager::commit_transaction(TransactionId txn_id) {
  std::unique_lock<std::mutex> lock(mutex_);

  auto it = transactions_.find(txn_id);
  if (it == transactions_.end()) {
    std::cerr << "Transaction " << txn_id << " not found" << std::endl;
    return false;
  }

  Transaction &txn = it->second;
  if (txn.state != TransactionState::ACTIVE) {
    std::cerr << "Transaction " << txn_id
              << " is not active (state: " << static_cast<int>(txn.state) << ")"
              << std::endl;
    return false;
  }

  // 设置事务状态为已提交
  txn.state = TransactionState::COMMITTED;
  txn.end_time = std::chrono::system_clock::now();

  // 释放事务持有的所有锁（已持有锁，调用内部版本）
  release_all_locks_internal(txn_id);

  // 从等待图中移除事务
  wait_graph_.erase(txn_id);

  std::cout << "Transaction " << txn_id << " committed" << std::endl;

  // 不要在这里直接调用cleanup_completed_transactions，因为会导致死锁
  // cleanup_completed_transactions();

  return true;
}

bool TransactionManager::rollback_transaction(TransactionId txn_id) {
  std::unique_lock<std::mutex> lock(mutex_);

  auto it = transactions_.find(txn_id);
  if (it == transactions_.end()) {
    std::cerr << "Transaction " << txn_id << " not found" << std::endl;
    return false;
  }

  Transaction &txn = it->second;
  if (txn.state != TransactionState::ACTIVE) {
    std::cerr << "Transaction " << txn_id << " cannot be rolled back (state: "
              << static_cast<int>(txn.state) << ")" << std::endl;
    return false;
  }

  // 设置事务状态为正在回滚
  txn.state = TransactionState::ROLLING_BACK;

  // 执行撤销操作（简化实现）
  // 在实际实现中，应该重放undo日志中的操作进行回滚
  for (const auto &log_entry : txn.undo_log) {
    // 执行撤销操作的逻辑
    // 这里应该调用存储引擎来实际执行撤销
    std::cout << "Rolling back operation: " << log_entry.operation
              << " on table " << log_entry.table_name << std::endl;
  }

  // 设置事务状态为已中止
  txn.state = TransactionState::ABORTED;
  txn.end_time = std::chrono::system_clock::now();

  // 释放事务持有的所有锁（已持有锁，调用内部版本）
  release_all_locks_internal(txn_id);

  // 从等待图中移除事务
  wait_graph_.erase(txn_id);

  std::cout << "Transaction " << txn_id << " rolled back" << std::endl;

  // 不要在这里直接调用cleanup_completed_transactions，因为会导致死锁
  // cleanup_completed_transactions();

  return true;
}

bool TransactionManager::create_savepoint(TransactionId txn_id,
                                        const std::string &savepoint_name) {
  std::unique_lock<std::mutex> lock(mutex_);

  auto it = transactions_.find(txn_id);
  if (it == transactions_.end()) {
    std::cerr << "Transaction " << txn_id << " not found" << std::endl;
    return false;
  }

  Transaction &txn = it->second;
  if (txn.state != TransactionState::ACTIVE) {
    std::cerr << "Transaction " << txn_id
              << " is not active (state: " << static_cast<int>(txn.state) << ")"
              << std::endl;
    return false;
  }

  // 在实际实现中，应该保存当前事务的状态和日志位置
  // 这里只是一个占位实现
  std::cout << "Savepoint '" << savepoint_name << "' created for transaction " << txn_id << std::endl;
  
  return true;
}

bool TransactionManager::rollback_to_savepoint(TransactionId txn_id,
                                             const std::string &savepoint_name) {
  std::unique_lock<std::mutex> lock(mutex_);

  auto it = transactions_.find(txn_id);
  if (it == transactions_.end()) {
    std::cerr << "Transaction " << txn_id << " not found" << std::endl;
    return false;
  }

  Transaction &txn = it->second;
  if (txn.state != TransactionState::ACTIVE) {
    std::cerr << "Transaction " << txn_id
              << " is not active (state: " << static_cast<int>(txn.state) << ")"
              << std::endl;
    return false;
  }

  // 在实际实现中，应该恢复到指定保存点的状态
  // 这里只是一个占位实现
  std::cout << "Rolled back to savepoint '" << savepoint_name << "' for transaction " << txn_id << std::endl;
  
  return true;
}

bool TransactionManager::acquire_lock(TransactionId txn_id,
                                      const std::string &resource,
                                      LockType lock_type, bool wait) {
  std::unique_lock<std::mutex> lock(mutex_);

  auto it = transactions_.find(txn_id);
  if (it == transactions_.end()) {
    std::cerr << "Transaction " << txn_id << " not found" << std::endl;
    return false;
  }

  Transaction &txn = it->second;
  if (txn.state != TransactionState::ACTIVE) {
    std::cerr << "Transaction " << txn_id
              << " is not active (state: " << static_cast<int>(txn.state) << ")"
              << std::endl;
    return false;
  }

  // 检查是否已经持有该资源的锁
  auto lit = lock_table_.find(resource);
  if (lit != lock_table_.end()) {
    for (const auto &lock_entry : lit->second) {
      if (lock_entry.txn_id == txn_id) {
        // 已经持有锁，检查是否需要升级
        if (lock_entry.type == LockType::SHARED && lock_type == LockType::EXCLUSIVE) {
          // 锁升级逻辑（简化实现）
          // 在实际实现中，需要更复杂的锁升级机制
          std::cout << "Upgrading lock for transaction " << txn_id
                    << " on resource " << resource << std::endl;
        }
        return true;
      }
    }
  }

  // 添加锁条目
  LockEntry lock_entry;
  lock_entry.txn_id = txn_id;
  lock_entry.type = lock_type;
  lock_entry.resource = resource;
  lock_entry.acquired_time = std::chrono::system_clock::now();
  lock_table_[resource].push_back(lock_entry);

  return true;
}

void TransactionManager::release_lock(TransactionId txn_id,
                                      const std::string &resource) {
  std::unique_lock<std::mutex> lock(mutex_);

  auto lit = lock_table_.find(resource);
  if (lit != lock_table_.end()) {
    auto &locks = lit->second;
    locks.erase(
        std::remove_if(locks.begin(), locks.end(),
                       [txn_id](const LockEntry &entry) { return entry.txn_id == txn_id; }),
        locks.end());

    // 如果该资源没有任何锁了，就从锁表中移除
    if (locks.empty()) {
      lock_table_.erase(lit);
    }
  }
}

bool TransactionManager::detect_deadlock(TransactionId txn_id) {
  std::unique_lock<std::mutex> lock(mutex_);
  
  // 简化的死锁检测实现
  // 在实际实现中，应该使用更复杂的算法（如等待图遍历）来检测死锁
  
  // 检查等待图中是否存在环路
  std::unordered_set<TransactionId> visited;
  std::unordered_set<TransactionId> recursion_stack;
  
  // 这里只是一个占位实现，总是返回false表示没有检测到死锁
  std::cout << "Deadlock detection performed for transaction " << txn_id << std::endl;
  
  return false;
}

TransactionState
TransactionManager::get_transaction_state(TransactionId txn_id) const {
  std::unique_lock<std::mutex> lock(mutex_);
  auto it = transactions_.find(txn_id);
  if (it == transactions_.end()) {
    throw std::runtime_error("Transaction not found");
  }
  return it->second.state;
}

std::vector<TransactionId> TransactionManager::get_active_transactions() const {
  std::unique_lock<std::mutex> lock(mutex_);
  std::vector<TransactionId> active_txns;

  for (const auto &[txn_id, txn] : transactions_) {
    if (txn.state == TransactionState::ACTIVE) {
      active_txns.push_back(txn_id);
    }
  }

  return active_txns;
}

void TransactionManager::log_operation(TransactionId txn_id,
                                       const LogEntry &entry) {
  std::unique_lock<std::mutex> lock(mutex_);
  auto it = transactions_.find(txn_id);
  if (it != transactions_.end()) {
    it->second.undo_log.push_back(entry);
  }
}

// 注意：保存点功能已在前面实现

// 释放事务持有的所有锁（内部版本，不加锁）
void TransactionManager::release_all_locks_internal(TransactionId txn_id) {
  // 遍历锁表，释放该事务持有的所有锁
  for (auto lit = lock_table_.begin(); lit != lock_table_.end();) {
    auto &locks = lit->second;
    locks.erase(
        std::remove_if(locks.begin(), locks.end(),
                       [txn_id](const LockEntry &entry) { return entry.txn_id == txn_id; }),
        locks.end());

    // 如果该资源没有任何锁了，就从锁表中移除
    if (locks.empty()) {
      lit = lock_table_.erase(lit);
    } else {
      ++lit;
    }
  }
}

// 释放事务持有的所有锁（公共版本，加锁）
void TransactionManager::release_all_locks(TransactionId txn_id) {
  std::unique_lock<std::mutex> lock(mutex_);
  release_all_locks_internal(txn_id);
}

} // namespace sqlcc