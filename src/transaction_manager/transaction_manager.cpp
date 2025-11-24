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

  // 检查事务是否存在
  auto txn_it = transactions_.find(txn_id);
  if (txn_it == transactions_.end() ||
      txn_it->second.state != TransactionState::ACTIVE) {
    return false;
  }

  // 检查是否已经持有该资源的锁
  if (lock_table_.count(resource)) {
    auto &locks = lock_table_[resource];
    for (const auto &lock_entry : locks) {
      if (lock_entry.txn_id == txn_id) {
        // 已经持有更高级别的锁
        if (lock_entry.type == LockType::EXCLUSIVE &&
            lock_type == LockType::SHARED) {
          return true;
        }
        // 已经持有相同类型的锁
        if (lock_entry.type == lock_type) {
          return true;
        }
      }
    }
  }

  // 详细的锁获取逻辑
  if (lock_type == LockType::SHARED) {
    // 共享锁兼容性：只有排他锁不兼容
    bool has_exclusive_lock = false;
    if (lock_table_.find(resource) != lock_table_.end()) {
      for (const auto &lock_entry : lock_table_[resource]) {
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
      std::cout << "Transaction " << txn_id
                << " acquired shared lock on resource '" << resource << "'"
                << std::endl;
      return true;
    }
    return false;
  } else if (lock_type == LockType::EXCLUSIVE) {
    // 排他锁检查：不能有任何其他锁
    if (lock_table_.find(resource) != lock_table_.end() && !lock_table_[resource].empty()) {
      if (wait) {
        for (const auto &lock_entry : lock_table_[resource]) {
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
    std::cout << "Transaction " << txn_id
              << " acquired exclusive lock on resource '" << resource << "'"
              << std::endl;
    return true;
  }

  return false;
}

void TransactionManager::release_lock(TransactionId txn_id,
                                      const std::string &resource) {
  std::unique_lock<std::mutex> lock(mutex_);

  // 检查资源是否有锁
  if (lock_table_.find(resource) != lock_table_.end()) {
    auto &locks = lock_table_[resource];

    // 查找并移除该事务对该资源的锁
    auto it = std::remove_if(
        locks.begin(), locks.end(),
        [txn_id](const LockEntry &entry) { return entry.txn_id == txn_id; });

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
  for (auto &[tid, waiting_for] : wait_graph_) {
    waiting_for.erase(txn_id);
  }

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
  // 使用原子操作确保线程安全
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
  // 检查资源是否有锁
  if (lock_table_.find(resource) != lock_table_.end()) {
    for (const auto &entry : lock_table_.at(resource)) {
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

// 注意：保存点功能已在前面实现

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
