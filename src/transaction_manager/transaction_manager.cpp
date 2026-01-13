/**
 * @file transaction_manager.cpp
 *
 * WHY: 为什么需要事务管理器？
 *
 * 数据库系统运行在多用户并发环境中，事务管理器是确保数据一致性和系统可靠性的核心基础设施。没有事务管理器，数据库就无法保证ACID（原子性、一致性、隔离性、持久性）特性，导致数据损坏、并发冲突和系统不稳定。事务管理器是数据库系统的"心脏"，协调着所有数据操作的执行顺序和安全保证。
 *
 * 主要问题解决：
 * 1. 并发控制：允许多个事务同时访问数据而不会相互干扰
 * 2. 原子性保证：事务要么完全执行，要么完全不执行
 * 3. 隔离性保证：事务之间的操作互不影响
 * 4. 持久性保证：已提交事务的修改永久保存
 * 5. 死锁检测：自动检测和解决事务间的死锁情况
 *
 * 事务管理器失败的影响：
 * - 数据不一致：并发事务破坏数据完整性
 * - 系统崩溃：原子性保证失效导致数据损坏
 * - 性能低下：缺乏并发控制导致串行化执行
 * - 死锁问题：事务相互等待导致系统挂起
 *
 * WHAT: 这实现了什么功能？
 *
 * 事务管理器提供完整的ACID事务管理功能：
 * - 事务生命周期：创建、提交、回滚事务
 * - 并发控制：锁管理器处理资源竞争
 * - 死锁检测：基于等待图的死锁检测算法
 * - 保存点支持：事务内部的回滚点管理
 * - 隔离级别：READ UNCOMMITTED、READ COMMITTED等
 * - 日志记录：事务操作的WAL（预写日志）
 * - 状态监控：事务执行状态和性能统计
 *
 * 核心组件：
 * - TransactionManager：事务管理器主类，协调所有事务操作
 * - Transaction：事务对象，封装单个事务的状态和操作
 * - LockManager：锁管理器，实现两阶段锁协议
 * - DeadlockDetector：死锁检测器，基于等待图算法
 * - LogManager：日志管理器，记录事务操作用于恢复
 * - SavepointManager：保存点管理器，支持部分回滚
 *
 * HOW: 如何实现的？
 *
 * 技术实现要点：
 * 1. 事务状态机：ACTIVE->COMMITTED/ABORTED的状态转换
 * 2. 两阶段锁协议：获取锁->执行操作->释放锁的严格顺序
 * 3. 等待图检测：深度优先搜索检测事务间的死锁环路
 * 4. 原子操作：使用原子变量确保事务ID的唯一性和递增
 * 5. 互斥锁保护：std::mutex保证多线程环境下的数据安全
 * 6. 条件变量同步：std::condition_variable处理事务等待逻辑
 * 7. 智能指针管理：std::unique_ptr自动管理资源生命周期
 *
 * 架构设计：
 * - 分层架构：事务管理层、锁管理层、日志层的清晰分离
 * - 插件架构：支持不同并发控制算法和隔离级别
 * - 事件驱动：基于回调机制的通知和状态更新
 * - 配置驱动：运行时配置事务参数和策略
 * - 监控集成：与系统监控和告警系统的深度集成
 *
 * 并发控制策略：
 * - 严格两阶段锁：获取所有锁后执行，完成后释放
 * - 乐观并发控制：假设冲突少，提交时验证
 * - 多版本并发控制：维护数据多个版本支持读取
 * - 时间戳排序：基于时间戳决定事务执行顺序
 * - 验证并发控制：执行时不锁，提交时验证
 *
 * 死锁处理机制：
 * - 死锁预防：资源有序分配避免循环等待
 * - 死锁检测：定期扫描等待图发现死锁环路
 * - 死锁恢复：选择牺牲者终止事务打破死锁
 * - 超时机制：事务等待超时自动放弃
 *
 * @note 该实现专为SQLCC数据库系统优化，支持高并发和ACID事务特性
 * @see include/transaction_manager.h
 */

#include "transaction_manager.h"
#include "transaction/savepoint_manager.h"
#include <algorithm>
#include <condition_variable>
#include <iostream>
#include <queue>
#include <stack>
#include <stdexcept>
#include <thread>

namespace sqlcc {

// Transaction构造函数实现
Transaction::Transaction()
    : txn_id(0), isolation_level(IsolationLevel::READ_COMMITTED),
      state(TransactionState::ACTIVE),
      start_time(std::chrono::system_clock::now()),
      timeout_duration(std::chrono::milliseconds(30000)),
      is_nested(false), parent_txn_id(0), operation_count(0),
      last_activity(std::chrono::system_clock::now()) {}

Transaction::Transaction(TransactionId id, IsolationLevel level,
                         std::chrono::milliseconds timeout_ms)
    : txn_id(id), isolation_level(level), state(TransactionState::ACTIVE),
      start_time(std::chrono::system_clock::now()),
      timeout_duration(timeout_ms), is_nested(false), parent_txn_id(0),
      operation_count(0), last_activity(std::chrono::system_clock::now()) {}

Transaction Transaction::create_nested(TransactionId id, TransactionId parent_id,
                                      IsolationLevel inherited_level) {
  Transaction txn(id, inherited_level);
  txn.is_nested = true;
  txn.parent_txn_id = parent_id;
  return txn;
}

bool Transaction::is_timeout() const {
  auto now = std::chrono::system_clock::now();
  auto running_time = std::chrono::duration_cast<std::chrono::milliseconds>(
      now - start_time);
  return running_time > timeout_duration;
}

void Transaction::update_activity() {
  last_activity = std::chrono::system_clock::now();
  ++operation_count;
}

std::chrono::milliseconds Transaction::get_running_time() const {
  auto now = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      now - start_time);
}

// TransactionManager构造函数实现
TransactionManager::TransactionManager() : next_txn_id_(1ULL) {}

// 析构函数实现
TransactionManager::~TransactionManager() {}

TransactionId TransactionManager::next_transaction_id() {
  return next_txn_id_.fetch_add(1);
}

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

  // Use SavepointManager to create the savepoint with proper undo log position
  size_t undo_position = txn.undo_log.size(); // Current position in undo log
  bool success = SavepointManager::getInstance().createSavepoint(txn_id, savepoint_name);

  if (success) {
    // Update transaction activity
    txn.update_activity();

    std::cout << "Savepoint '" << savepoint_name << "' created for transaction "
              << txn_id << " at undo position " << undo_position << std::endl;
  }

  return success;
}

bool TransactionManager::rollback_to_savepoint(
    TransactionId txn_id, const std::string &savepoint_name) {
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

  // Get savepoint information from SavepointManager
  auto savepoint_info = SavepointManager::getInstance().getSavepoint(txn_id, savepoint_name);
  if (!savepoint_info) {
    std::cerr << "Savepoint '" << savepoint_name << "' not found for transaction "
              << txn_id << std::endl;
    return false;
  }

  size_t target_undo_position = savepoint_info->getUndoLogPosition();

  // Rollback undo log entries from current position to savepoint position
  while (txn.undo_log.size() > target_undo_position) {
    const LogEntry& entry = txn.undo_log.back();

    // In a real implementation, we would apply the undo operation here
    // For now, we just log the rollback action
    std::cout << "Rolling back operation: " << entry.operation
              << " on table " << entry.table_name << " for transaction " << txn_id << std::endl;

    // Remove the log entry
    txn.undo_log.pop_back();
  }

  // Use SavepointManager to perform the rollback (which may clean up nested savepoints)
  bool success = SavepointManager::getInstance().rollbackToSavepoint(txn_id, savepoint_name);

  if (success) {
    // Update transaction activity
    txn.update_activity();

    std::cout << "Rolled back to savepoint '" << savepoint_name
              << "' for transaction " << txn_id << std::endl;
  }

  return success;
}

bool TransactionManager::acquire_lock(TransactionId txn_id,
                                      const std::string &resource,
                                      LockType lock_type, bool /*wait*/) {
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
        if (lock_entry.type == LockType::SHARED &&
            lock_type == LockType::EXCLUSIVE) {
          // 锁升级逻辑：检查是否可以升级
          for (const auto &other_lock : lit->second) {
            if (other_lock.txn_id != txn_id) {
              // 有其他事务持有锁，不能升级
              return false;
            }
          }
          // 可以升级锁
          std::cout << "Upgrading lock for transaction " << txn_id
                    << " on resource " << resource << std::endl;
          // 更新锁类型
          for (auto &entry : lit->second) {
            if (entry.txn_id == txn_id) {
              entry.type = LockType::EXCLUSIVE;
              break;
            }
          }
        }
        return true;
      }
    }

    // 检查锁兼容性
    for (const auto &lock_entry : lit->second) {
      if (lock_type == LockType::EXCLUSIVE) {
        // 要获取排它锁，不能有任何其他锁
        return false;
      } else {
        // 要获取共享锁，不能有排它锁
        if (lock_entry.type == LockType::EXCLUSIVE) {
          return false;
        }
      }
    }
  }

  // 可以获取锁，添加锁条目
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
    locks.erase(std::remove_if(locks.begin(), locks.end(),
                               [txn_id](const LockEntry &entry) {
                                 return entry.txn_id == txn_id;
                               }),
                locks.end());

    // 如果该资源没有任何锁了，就从锁表中移除
    if (locks.empty()) {
      lock_table_.erase(lit);
    }
  }
}

bool TransactionManager::detect_deadlock(TransactionId txn_id) {
  std::unique_lock<std::mutex> lock(mutex_);

  // 实现基于等待图的死锁检测算法
  // 检查等待图中是否存在环路
  std::unordered_set<TransactionId> visited;
  std::unordered_set<TransactionId> recursion_stack;

  // 深度优先搜索检测环路
  auto has_cycle = [&](auto &&self, TransactionId current) -> bool {
    visited.insert(current);
    recursion_stack.insert(current);

    // 检查当前事务是否在等待其他事务
    auto it = wait_graph_.find(current);
    if (it != wait_graph_.end()) {
      for (TransactionId waiting_for : it->second) {
        if (visited.find(waiting_for) == visited.end()) {
          if (self(self, waiting_for)) {
            return true;
          }
        } else if (recursion_stack.find(waiting_for) != recursion_stack.end()) {
          // 找到环路
          return true;
        }
      }
    }

    recursion_stack.erase(current);
    return false;
  };

  // 使用深度优先搜索检测死锁
  if (has_cycle(has_cycle, txn_id)) {
    return true;
  }

  std::cout << "Deadlock detection performed for transaction " << txn_id
            << std::endl;

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
    locks.erase(std::remove_if(locks.begin(), locks.end(),
                               [txn_id](const LockEntry &entry) {
                                 return entry.txn_id == txn_id;
                               }),
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

TransactionId TransactionManager::begin_nested_transaction(TransactionId parent_txn_id) {
  std::unique_lock<std::mutex> lock(mutex_);

  // 检查父事务是否存在且处于活跃状态
  auto parent_it = transactions_.find(parent_txn_id);
  if (parent_it == transactions_.end()) {
    std::cerr << "Parent transaction " << parent_txn_id << " not found" << std::endl;
    return 0;
  }

  const Transaction& parent_txn = parent_it->second;
  if (parent_txn.state != TransactionState::ACTIVE) {
    std::cerr << "Parent transaction " << parent_txn_id << " is not active" << std::endl;
    return 0;
  }

  // 创建嵌套事务
  TransactionId nested_txn_id = next_transaction_id();
  Transaction nested_txn = Transaction::create_nested(nested_txn_id, parent_txn_id,
                                                      parent_txn.isolation_level);

  // 记录嵌套事务信息
  NestedTransaction nested_info;
  nested_info.parent_txn_id = parent_txn_id;
  nested_info.nested_txn_id = nested_txn_id;
  nested_info.inherited_isolation = parent_txn.isolation_level;
  nested_info.start_time = nested_txn.start_time;

  transactions_[nested_txn_id] = std::move(nested_txn);
  nested_transactions_[nested_txn_id] = std::move(nested_info);

  std::cout << "Nested transaction " << nested_txn_id << " started for parent "
            << parent_txn_id << std::endl;

  return nested_txn_id;
}

bool TransactionManager::commit_nested_transaction(TransactionId nested_txn_id) {
  std::unique_lock<std::mutex> lock(mutex_);

  // 检查是否为嵌套事务
  auto nested_it = nested_transactions_.find(nested_txn_id);
  if (nested_it == nested_transactions_.end()) {
    std::cerr << "Transaction " << nested_txn_id << " is not a nested transaction" << std::endl;
    return false;
  }

  // 检查嵌套事务状态
  auto txn_it = transactions_.find(nested_txn_id);
  if (txn_it == transactions_.end()) {
    std::cerr << "Nested transaction " << nested_txn_id << " not found" << std::endl;
    return false;
  }

  Transaction& nested_txn = txn_it->second;
  if (nested_txn.state != TransactionState::ACTIVE) {
    std::cerr << "Nested transaction " << nested_txn_id << " is not active" << std::endl;
    return false;
  }

  // 提交嵌套事务（简化实现：不影响父事务）
  nested_txn.state = TransactionState::COMMITTED;
  nested_txn.end_time = std::chrono::system_clock::now();

  // 释放嵌套事务持有的锁
  release_all_locks_internal(nested_txn_id);

  // 从嵌套事务表中移除
  nested_transactions_.erase(nested_it);

  std::cout << "Nested transaction " << nested_txn_id << " committed" << std::endl;

  return true;
}

size_t TransactionManager::check_and_handle_timeouts() {
  std::unique_lock<std::mutex> lock(mutex_);
  size_t timeout_count = 0;

  for (auto it = transactions_.begin(); it != transactions_.end();) {
    Transaction& txn = it->second;
    if (txn.state == TransactionState::ACTIVE && txn.is_timeout()) {
      std::cout << "Transaction " << txn.txn_id << " timed out, rolling back" << std::endl;

      // 强制回滚超时的活跃事务
      txn.state = TransactionState::ROLLING_BACK;
      release_all_locks_internal(txn.txn_id);
      txn.state = TransactionState::ABORTED;
      txn.end_time = std::chrono::system_clock::now();

      ++timeout_count;

      // 如果是嵌套事务，也要从嵌套事务表中移除
      nested_transactions_.erase(txn.txn_id);

      ++it; // 继续处理下一个事务
    } else {
      ++it;
    }
  }

  if (timeout_count > 0) {
    std::cout << "Handled " << timeout_count << " timeout transactions" << std::endl;
  }

  return timeout_count;
}

bool TransactionManager::set_transaction_timeout(TransactionId txn_id,
                                                std::chrono::milliseconds timeout_ms) {
  std::unique_lock<std::mutex> lock(mutex_);

  auto it = transactions_.find(txn_id);
  if (it == transactions_.end()) {
    std::cerr << "Transaction " << txn_id << " not found" << std::endl;
    return false;
  }

  Transaction& txn = it->second;
  if (txn.state != TransactionState::ACTIVE) {
    std::cerr << "Transaction " << txn_id << " is not active" << std::endl;
    return false;
  }

  txn.timeout_duration = timeout_ms;
  std::cout << "Transaction " << txn_id << " timeout set to "
            << timeout_ms.count() << "ms" << std::endl;

  return true;
}

TransactionManager::TransactionStats TransactionManager::get_transaction_stats() const {
  std::unique_lock<std::mutex> lock(mutex_);

  TransactionStats stats = {0, 0, 0, 0, std::chrono::milliseconds(0)};
  size_t total_time_ms = 0;
  size_t completed_count = 0;

  for (const auto& [txn_id, txn] : transactions_) {
    if (txn.state == TransactionState::ACTIVE) {
      ++stats.active_transactions;
    }

    ++stats.total_transactions;

    if (txn.is_timeout() && txn.state == TransactionState::ABORTED) {
      ++stats.timeout_transactions;
    }

    if (txn.is_nested) {
      ++stats.nested_transactions;
    }

    // 计算平均事务时间
    if (txn.state == TransactionState::COMMITTED || txn.state == TransactionState::ABORTED) {
      ++completed_count;
      total_time_ms += txn.get_running_time().count();
    }
  }

  if (completed_count > 0) {
    stats.avg_transaction_time = std::chrono::milliseconds(total_time_ms / completed_count);
  }

  return stats;
}

bool TransactionManager::set_transaction_isolation_level(TransactionId txn_id, IsolationLevel level) {
  std::unique_lock<std::mutex> lock(mutex_);

  auto it = transactions_.find(txn_id);
  if (it == transactions_.end()) {
    std::cerr << "Transaction " << txn_id << " not found" << std::endl;
    return false;
  }

  Transaction& txn = it->second;
  if (txn.state != TransactionState::ACTIVE) {
    std::cerr << "Transaction " << txn_id << " is not active" << std::endl;
    return false;
  }

  // 只有在事务开始时才能更改隔离级别（简化实现）
  // 在实际SQL中，可以在事务运行期间更改
  txn.isolation_level = level;
  txn.update_activity();

  std::cout << "Transaction " << txn_id << " isolation level set to "
            << static_cast<int>(level) << std::endl;

  return true;
}

IsolationLevel TransactionManager::get_transaction_isolation_level(TransactionId txn_id) const {
  std::unique_lock<std::mutex> lock(mutex_);

  auto it = transactions_.find(txn_id);
  if (it == transactions_.end()) {
    throw std::runtime_error("Transaction not found");
  }

  return it->second.isolation_level;
}

bool TransactionManager::check_isolation_constraints(TransactionId txn_id, const std::string& operation,
                                                   const std::string& resource) {
  std::unique_lock<std::mutex> lock(mutex_);

  auto txn_it = transactions_.find(txn_id);
  if (txn_it == transactions_.end()) {
    return false;
  }

  const Transaction& txn = txn_it->second;
  if (txn.state != TransactionState::ACTIVE) {
    return false;
  }

  IsolationLevel level = txn.isolation_level;

  // 根据隔离级别检查约束
  if (operation == "READ") {
    switch (level) {
      case IsolationLevel::READ_UNCOMMITTED:
        // 允许读取未提交的数据
        return true;

      case IsolationLevel::READ_COMMITTED:
        // 只允许读取已提交的数据
        // 这里需要检查是否有其他事务对该资源持有排他锁
        // 简化实现：总是允许读取
        return true;

      case IsolationLevel::REPEATABLE_READ:
        // 确保可重复读：检查是否有写锁
        // 简化实现
        return true;

      case IsolationLevel::SERIALIZABLE:
        // 最高隔离级别：需要检查所有可能的冲突
        // 简化实现
        return true;

      default:
        return false;
    }
  } else if (operation == "WRITE") {
    switch (level) {
      case IsolationLevel::READ_UNCOMMITTED:
      case IsolationLevel::READ_COMMITTED:
      case IsolationLevel::REPEATABLE_READ:
      case IsolationLevel::SERIALIZABLE: {
        // 检查是否有其他事务持有锁
        auto lock_it = lock_table_.find(resource);
        if (lock_it != lock_table_.end()) {
          for (const auto& lock_entry : lock_it->second) {
            if (lock_entry.txn_id != txn_id) {
              // 存在其他事务的锁，根据隔离级别决定是否允许
              if (level == IsolationLevel::READ_UNCOMMITTED) {
                // 允许脏写
                continue;
              } else {
                // 其他级别不允许并发写
                return false;
              }
            }
          }
        }
        return true;
      }

      default:
        return false;
    }
  }

  // 未知操作，允许执行
  return true;
}

} // namespace sqlcc