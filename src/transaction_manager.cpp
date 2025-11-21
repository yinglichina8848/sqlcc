#include "transaction_manager.h"
#include <algorithm>
#include <condition_variable>
#include <iostream>
#include <queue>
#include <stack>
#include <thread>

namespace sqlcc {

// Transaction构造函数实现
Transaction::Transaction(TransactionId id, IsolationLevel level)
    : txn_id(id), isolation_level(level), state(TransactionState::ACTIVE),
      start_time(std::chrono::system_clock::now()) {}

// TransactionManager构造函数实现
TransactionManager::TransactionManager() : next_txn_id_(1ULL) {}

// 析构函数实现
TransactionManager::~TransactionManager() {}

// 实现主要方法

TransactionId
TransactionManager::begin_transaction(IsolationLevel isolation_level) {
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

  // 释放事务持有的所有锁
  release_all_locks(txn_id);

  // 从等待图中移除事务
  wait_graph_.erase(txn_id);

  std::cout << "Transaction " << txn_id << " committed" << std::endl;

  // 异步清理已完成事务
  std::thread([this]() { cleanup_completed_transactions(); }).detach();

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

  // 释放事务持有的所有锁
  release_all_locks(txn_id);

  // 从等待图中移除事务
  wait_graph_.erase(txn_id);

  std::cout << "Transaction " << txn_id << " rolled back" << std::endl;

  // 异步清理已完成事务
  std::thread([this]() { cleanup_completed_transactions(); }).detach();

  return true;
}

bool TransactionManager::create_savepoint(TransactionId txn_id,
                                          const std::string &savepoint_name) {
  std::unique_lock<std::mutex> lock(mutex_);

  auto it = transactions_.find(txn_id);
  if (it == transactions_.end()) {
    return false;
  }

  Transaction &txn = it->second;
  if (txn.state != TransactionState::ACTIVE) {
    return false;
  }

  // 创建保存点 - 在实际实现中，这里应该记录当前undo日志的位置
  // 这里只是简单记录保存点名称
  std::cout << "Savepoint '" << savepoint_name << "' created for transaction "
            << txn_id << std::endl;

  return true;
}

bool TransactionManager::rollback_to_savepoint(
    TransactionId txn_id, const std::string &savepoint_name) {
  std::unique_lock<std::mutex> lock(mutex_);

  auto it = transactions_.find(txn_id);
  if (it == transactions_.end()) {
    return false;
  }

  Transaction &txn = it->second;
  if (txn.state != TransactionState::ACTIVE) {
    return false;
  }

  // 回滚到保存点 - 在实际实现中，这里应该回滚到指定的保存点位置
  std::cout << "Transaction " << txn_id << " rolled back to savepoint '"
            << savepoint_name << "'" << std::endl;

  return true;
}

bool TransactionManager::acquire_lock(TransactionId txn_id,
                                      const std::string &resource,
                                      LockType lock_type, bool wait) {
  std::unique_lock<std::mutex> lock(mutex_);

  // 检查事务是否存在且为活动状态
  auto txn_it = transactions_.find(txn_id);
  if (txn_it == transactions_.end() ||
      txn_it->second.state != TransactionState::ACTIVE) {
    return false;
  }

  // 检查是否可以获取锁
  if (!can_acquire_lock(txn_id, resource, lock_type)) {
    if (!wait) {
      return false; // 不等待，直接返回失败
    }

    // 等待锁的逻辑（简化实现）
    // 在实际实现中，这里应该实现更复杂的等待机制
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 重新检查
    if (!can_acquire_lock(txn_id, resource, lock_type)) {
      return false;
    }
  }

  // 添加锁条目
  LockEntry lock_entry;
  lock_entry.txn_id = txn_id;
  lock_entry.resource = resource;
  lock_entry.type = lock_type;
  lock_entry.acquired_time = std::chrono::system_clock::now();

  lock_table_[resource].push_back(lock_entry);

  std::cout << "Transaction " << txn_id << " acquired "
            << (lock_type == LockType::SHARED ? "SHARED" : "EXCLUSIVE")
            << " lock on resource '" << resource << "'" << std::endl;

  return true;
}

void TransactionManager::release_lock(TransactionId txn_id,
                                      const std::string &resource) {
  std::unique_lock<std::mutex> lock(mutex_);

  auto resource_it = lock_table_.find(resource);
  if (resource_it == lock_table_.end()) {
    return;
  }

  // 移除该事务持有在该资源上的锁
  auto &locks = resource_it->second;
  locks.erase(std::remove_if(locks.begin(), locks.end(),
                             [txn_id](const LockEntry &entry) {
                               return entry.txn_id == txn_id;
                             }),
              locks.end());

  std::cout << "Transaction " << txn_id << " released lock on resource '"
            << resource << "'" << std::endl;
}

bool TransactionManager::detect_deadlock(TransactionId txn_id) {
  // 简化实现：使用拓扑排序检测死锁环
  std::unordered_map<TransactionId, int> indegree;
  std::queue<TransactionId> q;

  // 初始化入度
  for (const auto &[tid, _] : wait_graph_) {
    indegree[tid] = 0;
  }

  for (const auto &[tid, dependents] : wait_graph_) {
    for (TransactionId dependent : dependents) {
      indegree[dependent]++;
    }
  }

  // 将入度为0的节点加入队列
  for (const auto &[tid, degree] : indegree) {
    if (degree == 0) {
      q.push(tid);
    }
  }

  // 拓扑排序
  while (!q.empty()) {
    TransactionId current = q.front();
    q.pop();

    if (wait_graph_.count(current)) {
      for (TransactionId dependent : wait_graph_[current]) {
        indegree[dependent]--;
        if (indegree[dependent] == 0) {
          q.push(dependent);
        }
      }
    }
  }

  // 检查是否存在环（即入度不为0的节点）
  for (const auto &[tid, degree] : indegree) {
    if (degree > 0) {
      std::cout << "Deadlock detected involving transaction " << txn_id
                << std::endl;
      return true;
    }
  }

  return false;
}

TransactionState
TransactionManager::get_transaction_state(TransactionId txn_id) const {
  std::unique_lock<std::mutex> lock(mutex_);
  auto it = transactions_.find(txn_id);
  if (it != transactions_.end()) {
    return it->second.state;
  }
  return TransactionState::ABORTED; // 返回中止状态表示事务不存在
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

TransactionId TransactionManager::next_transaction_id() {
  return next_txn_id_.fetch_add(1, std::memory_order_relaxed);
}

// 私有方法实现

void TransactionManager::cleanup_completed_transactions() {
  std::unique_lock<std::mutex> lock(mutex_);

  // 找到所有已完成的事务
  std::vector<TransactionId> completed_txns;
  for (const auto &[txn_id, txn] : transactions_) {
    if (txn.state == TransactionState::COMMITTED ||
        txn.state == TransactionState::ABORTED) {
      // 检查事务是否已经完成一段时间（简化：这里直接删除）
      completed_txns.push_back(txn_id);
    }
  }

  // 删除已完成的事务
  for (TransactionId txn_id : completed_txns) {
    transactions_.erase(txn_id);
  }

  std::cout << "Cleaned up " << completed_txns.size()
            << " completed transactions" << std::endl;
}

bool TransactionManager::can_acquire_lock(TransactionId txn_id,
                                          const std::string &resource,
                                          LockType lock_type) const {
  auto resource_it = lock_table_.find(resource);
  if (resource_it == lock_table_.end()) {
    // 没有其他事务持有锁
    return true;
  }

  const auto &locks = resource_it->second;

  // 检查兼容性
  switch (lock_type) {
  case LockType::SHARED:
    // 对于共享锁，只要没有其他事务持有排他锁就可以
    for (const LockEntry &entry : locks) {
      if (entry.txn_id != txn_id && entry.type == LockType::EXCLUSIVE) {
        // 添加到等待图
        wait_graph_[entry.txn_id].insert(txn_id);
        return false;
      }
    }
    return true;

  case LockType::EXCLUSIVE:
    // 对于排他锁，所有其他持锁事务都必须释放
    for (const LockEntry &entry : locks) {
      if (entry.txn_id != txn_id) {
        // 添加到等待图
        wait_graph_[entry.txn_id].insert(txn_id);
        return false;
      }
    }
    return true;

  default:
    return false;
  }
}

void TransactionManager::release_all_locks(TransactionId txn_id) {
  // 遍历锁表，释放该事务持有的所有锁
  for (auto &[resource, locks] : lock_table_) {
    locks.erase(std::remove_if(locks.begin(), locks.end(),
                               [txn_id](const LockEntry &entry) {
                                 return entry.txn_id == txn_id;
                               }),
                locks.end());
  }

  std::cout << "Released all locks for transaction " << txn_id << std::endl;
}

} // namespace sqlcc
