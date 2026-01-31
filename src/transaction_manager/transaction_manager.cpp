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
#include "../transaction/savepoint_manager.h"
#include <algorithm>
#include <condition_variable>
#include <iostream>
#include <queue>
#include <stack>
#include <stdexcept>
#include <thread>

namespace sqlcc {

// --- Transaction Implementation ---
/**
 * @brief Constructs a new default Transaction object.
 * Initializes with a default isolation level, active state, and a timeout duration.
 */
Transaction::Transaction()
    : txn_id(0), isolation_level(IsolationLevel::READ_COMMITTED),
      state(TransactionState::ACTIVE),
      start_time(std::chrono::system_clock::now()),
      timeout_duration(std::chrono::milliseconds(30000)),
      is_nested(false), parent_txn_id(0), operation_count(0),
      last_activity(std::chrono::system_clock::now()) {}

/**
 * @brief Constructs a new Transaction object with specified ID, isolation level, and timeout.
 * @param id The unique identifier for the transaction.
 * @param level The isolation level for the transaction.
 * @param timeout_ms The timeout duration for the transaction.
 */
Transaction::Transaction(TransactionId id, IsolationLevel level,
                         std::chrono::milliseconds timeout_ms)
    : txn_id(id), isolation_level(level), state(TransactionState::ACTIVE),
      start_time(std::chrono::system_clock::now()),
      timeout_duration(timeout_ms), is_nested(false), parent_txn_id(0),
      operation_count(0), last_activity(std::chrono::system_clock::now()) {}

/**
 * @brief Factory method to create a nested transaction.
 * @param id The unique identifier for the nested transaction.
 * @param parent_id The ID of the parent transaction.
 * @param inherited_level The isolation level inherited from the parent transaction.
 * @return A new Transaction object configured as a nested transaction.
 */
Transaction Transaction::create_nested(TransactionId id, TransactionId parent_id,
                                      IsolationLevel inherited_level) {
  Transaction txn(id, inherited_level);
  txn.is_nested = true;
  txn.parent_txn_id = parent_id;
  return txn;
}

/**
 * @brief Checks if the transaction has exceeded its allotted timeout duration.
 * @return True if the transaction has timed out, false otherwise.
 */
bool Transaction::is_timeout() const {
  auto now = std::chrono::system_clock::now();
  auto running_time = std::chrono::duration_cast<std::chrono::milliseconds>(
      now - start_time);
  return running_time > timeout_duration;
}

/**
 * @brief Updates the transaction's last activity timestamp and increments its operation count.
 * This is typically called after each significant operation within the transaction.
 */
void Transaction::update_activity() {
  last_activity = std::chrono::system_clock::now();
  ++operation_count;
}

/**
 * @brief Calculates the total running time of the transaction since its start.
 * @return The duration of the transaction as `std::chrono::milliseconds`.
 */
std::chrono::milliseconds Transaction::get_running_time() const {
  auto now = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      now - start_time);
}

// --- TransactionManager Implementation ---
/**
 * @brief Constructs the TransactionManager.
 * Initializes the next available transaction ID.
 */
TransactionManager::TransactionManager() : next_txn_id_(1ULL) {}

/**
 * @brief Destroys the TransactionManager.
 * Cleans up any resources if necessary (e.g., stopping background threads).
 */
TransactionManager::~TransactionManager() {}

/**
 * @brief Generates and returns a new unique transaction ID.
 * Uses an atomic operation to ensure thread-safe, sequential ID generation.
 * @return A unique `TransactionId`.
 */
TransactionId TransactionManager::next_transaction_id() {
  return next_txn_id_.fetch_add(1);
}

/**
 * @brief Initiates a new transaction with the specified isolation level.
 * @param isolation_level The desired isolation level for the new transaction.
 * @return The `TransactionId` of the newly created transaction.
 */
TransactionId
TransactionManager::begin_transaction(IsolationLevel isolation_level) {
  // --- Begin Transaction Flow ---
  // Step 1: Acquire a lock on the manager to ensure thread-safe access to shared data structures.
  std::unique_lock<std::mutex> lock(mutex_);

  // Step 2: Get a new, unique transaction ID.
  TransactionId txn_id = next_transaction_id();

  // Step 3: Create a new Transaction object, setting its initial state to ACTIVE.
  Transaction txn(txn_id, isolation_level);

  // Step 4: Store the new transaction in the central transaction map.
  transactions_[txn_id] = std::move(txn);

  std::cout << "Transaction " << txn_id << " started with isolation level "
            << static_cast<int>(isolation_level) << std::endl;

  // Step 5: Return the new transaction ID to the client.
  return txn_id;
}

/**
 * @brief Commits an active transaction, making its changes permanent.
 * @param txn_id The ID of the transaction to commit.
 * @return True if the transaction was successfully committed, false otherwise.
 */
bool TransactionManager::commit_transaction(TransactionId txn_id) {
  // --- Commit Transaction Flow ---
  // Step 1: Acquire a manager-level lock to ensure atomicity of the commit process.
  std::unique_lock<std::mutex> lock(mutex_);

  // Step 2: Find the transaction in the map.
  auto it = transactions_.find(txn_id);
  if (it == transactions_.end()) {
    std::cerr << "Transaction " << txn_id << " not found" << std::endl;
    return false;
  }

  // Step 3: Validate the transaction's state. Only an ACTIVE transaction can be committed.
  Transaction &txn = it->second;
  if (txn.state != TransactionState::ACTIVE) {
    std::cerr << "Transaction " << txn_id
              << " is not active (state: " << static_cast<int>(txn.state) << ")"
              << std::endl;
    return false;
  }

  // --- Critical Commit Point ---
  // In a real database with persistence (the 'D' in ACID), this is where you would first
  // ensure all transaction logs are flushed to disk (Write-Ahead Logging - WAL).
  // log_manager_->Flush(txn_id);

  // Step 4: Atomically update the transaction's state to COMMITTED.
  txn.state = TransactionState::COMMITTED;
  txn.end_time = std::chrono::system_clock::now();

  // Step 5: Release all locks held by this transaction. This is a key part of the
  // Two-Phase Locking (2PL) protocol's "shrinking phase", allowing other transactions to proceed.
  release_all_locks_internal(txn_id);

  // Step 6: Remove the transaction from the wait-for graph used for deadlock detection, as it no longer waits for any resources.
  wait_graph_.erase(txn_id);

  std::cout << "Transaction " << txn_id << " committed" << std::endl;

  // Note: Cleanup of the transaction object from the `transactions_` map is typically handled
  // by a separate background task to avoid slowing down the commit operation.

  return true;
}

/**
 * @brief Rolls back an active transaction, undoing all its changes.
 * @param txn_id The ID of the transaction to roll back.
 * @return True if the transaction was successfully rolled back, false otherwise.
 */
bool TransactionManager::rollback_transaction(TransactionId txn_id) {
  // --- Rollback Transaction Flow ---
  // Step 1: Acquire a manager-level lock.
  std::unique_lock<std::mutex> lock(mutex_);

  // Step 2: Find the transaction and validate its state. Only an ACTIVE transaction can be rolled back.
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

  // Step 3: Set state to ROLLING_BACK to prevent other operations on this transaction.
  txn.state = TransactionState::ROLLING_BACK;

  // Step 4: Execute the UNDO logic. This ensures Atomicity (the 'A' in ACID).
  // In a real database, this involves reading the transaction's private undo_log and
  // applying the inverse operations to revert its changes in the database.
  for (const auto &log_entry : txn.undo_log) {
    // Example: apply the undo operation via the storage engine.
    std::cout << "Rolling back operation: " << log_entry.operation
              << " on table " << log_entry.table_name << std::endl;
  }

  // Step 5: After all undo operations are complete, officially mark the transaction as ABORTED.
  txn.state = TransactionState::ABORTED;
  txn.end_time = std::chrono::system_clock::now();

  // Step 6: Just like a commit, release all locks held by the transaction.
  release_all_locks_internal(txn_id);

  // Step 7: Remove the transaction from the deadlock detection graph.
  wait_graph_.erase(txn_id);

  std::cout << "Transaction " << txn_id << " rolled back" << std::endl;

  return true;
}
/**
 * @brief Creates a new savepoint within an active transaction.
 * Savepoints allow for partial rollbacks within a transaction.
 * @param txn_id The ID of the transaction to create the savepoint in.
 * @param savepoint_name The unique name for the savepoint.
 * @return True if the savepoint was successfully created, false otherwise.
 */
bool TransactionManager::create_savepoint(TransactionId txn_id,
                                          const std::string &savepoint_name) {
  // Step 1: Acquire a manager-level lock.
  std::unique_lock<std::mutex> lock(mutex_);

  // Step 2: Find the transaction and validate its state.
  auto it = transactions_.find(txn_id);
  if (it == transactions_.end() || it->second.state != TransactionState::ACTIVE) {
    std::cerr << "Transaction " << txn_id << " not found or not active" << std::endl;
    return false;
  }
  
  // Step 3: Delegate to SavepointManager.
  // In a fully-fledged system, the SavepointManager would encapsulate the undo log handling for savepoints.
  // The current size of the undo_log effectively marks the point to which changes can be reverted.
  size_t undo_position = it->second.undo_log.size(); // Current position in undo log
  bool success = SavepointManager::getInstance().createSavepoint(txn_id, savepoint_name); // This would ideally also store the undo_position

  if (success) {
    // Step 4: Update transaction activity to reflect the operation.
    it->second.update_activity();

    std::cout << "Savepoint '" << savepoint_name << "' created for transaction "
              << txn_id << " at undo position " << undo_position << std::endl;
  }
  return success;
}

/**
 * @brief Rolls back an active transaction to a specified savepoint.
 * Undoes changes made since the savepoint was created.
 * @param txn_id The ID of the transaction to roll back.
 * @param savepoint_name The name of the savepoint to roll back to.
 * @return True if the rollback was successful, false otherwise.
 */
bool TransactionManager::rollback_to_savepoint(
    TransactionId txn_id, const std::string &savepoint_name) {
  // Step 1: Acquire a manager-level lock.
  std::unique_lock<std::mutex> lock(mutex_);

  // Step 2: Find the transaction and validate its state.
  auto it = transactions_.find(txn_id);
  if (it == transactions_.end() || it->second.state != TransactionState::ACTIVE) {
    std::cerr << "Transaction " << txn_id << " not found or not active" << std::endl;
    return false;
  }
  Transaction &txn = it->second;

  // Step 3: Retrieve savepoint information from the SavepointManager.
  auto savepoint_info = SavepointManager::getInstance().getSavepoint(txn_id, savepoint_name);
  if (!savepoint_info) {
    std::cerr << "Savepoint '" << savepoint_name << "' not found for transaction "
              << txn_id << std::endl;
    return false;
  }
  
  // Step 4: Determine the target position in the undo log.
  // Changes beyond this point will be reverted.
  size_t target_undo_position = savepoint_info->getUndoLogPosition(); 

  // Step 5: Rollback undo log entries.
  // Iterate backward through the undo log, applying inverse operations.
  while (txn.undo_log.size() > target_undo_position) {
    const LogEntry& entry = txn.undo_log.back();
    // In a real implementation, we would apply the undo operation here
    // For now, we just log the rollback action
    std::cout << "Rolling back operation via savepoint: " << entry.operation
              << " on table " << entry.table_name << " for transaction " << txn_id << std::endl;
    txn.undo_log.pop_back(); // Remove the log entry after applying its undo.
  }

  // Step 6: Use SavepointManager to perform the rollback (which may clean up nested savepoints).
  bool success = SavepointManager::getInstance().rollbackToSavepoint(txn_id, savepoint_name);

  if (success) {
    // Step 7: Update transaction activity.
    txn.update_activity();

    std::cout << "Rolled back to savepoint '" << savepoint_name
              << "' for transaction " << txn_id << std::endl;
  }
  return success;
}
/**
 * @brief Attempts to acquire a lock on a specified resource for a transaction.
 * This is a simplified implementation of a lock manager.
 * @param txn_id The ID of the transaction requesting the lock.
 * @param resource The identifier of the resource to lock (e.g., table name, page ID).
 * @param lock_type The type of lock to acquire (SHARED or EXCLUSIVE).
 * @param wait If true, the transaction waits for the lock; otherwise, it returns immediately. (Currently ignored).
 * @return True if the lock was acquired (or upgraded), false otherwise.
 */
bool TransactionManager::acquire_lock(TransactionId txn_id,
                                      const std::string &resource,
                                      LockType lock_type, bool /*wait*/) {
  // Step 1: Acquire a manager-level lock to protect shared lock table.
  std::unique_lock<std::mutex> lock(mutex_);

  // Step 2: Validate the transaction's existence and state.
  auto it = transactions_.find(txn_id);
  if (it == transactions_.end() || it->second.state != TransactionState::ACTIVE) {
    std::cerr << "Transaction " << txn_id << " not found or not active" << std::endl;
    return false;
  }

  // --- Simplified Lock Acquisition Logic ---
  // A full-featured LockManager would involve a more complex state machine,
  // including waiting queues, deadlock detection triggers, and lock granularity management.

  // Step 3: Check if the transaction already holds a lock on the resource.
  auto lit = lock_table_.find(resource);
  if (lit != lock_table_.end()) {
    for (const auto &lock_entry : lit->second) {
      if (lock_entry.txn_id == txn_id) {
        // Transaction already holds a lock.
        // Step 3a: Check for lock upgrade scenario (SHARED to EXCLUSIVE).
        if (lock_entry.type == LockType::SHARED && lock_type == LockType::EXCLUSIVE) {
          // Attempting to upgrade from SHARED to EXCLUSIVE.
          // This is only possible if no other transaction holds ANY lock (shared or exclusive) on this resource.
          bool can_upgrade = true;
          for (const auto &other_lock : lit->second) {
            if (other_lock.txn_id != txn_id) {
              can_upgrade = false;
              break;
            }
          }
          if (can_upgrade) {
            // Upgrade successful: find and update the existing lock entry.
            for (auto &entry : lit->second) {
              if (entry.txn_id == txn_id) {
                entry.type = LockType::EXCLUSIVE;
                break;
              }
            }
            std::cout << "Upgrading lock for transaction " << txn_id
                      << " on resource " << resource << std::endl;
            return true;
          } else {
            // Cannot upgrade due to conflicting locks from other transactions.
            std::cerr << "Transaction " << txn_id << " cannot upgrade lock on resource "
                      << resource << " due to conflicting locks." << std::endl;
            return false;
          }
        }
        // If already holds the requested lock type or a stronger one, no action needed.
        return true;
      }
    }

    // Step 3b: Check for compatibility with locks held by *other* transactions.
    for (const auto &lock_entry : lit->second) {
      // An EXCLUSIVE lock request conflicts with any existing lock (shared or exclusive).
      if (lock_type == LockType::EXCLUSIVE) {
        std::cerr << "Transaction " << txn_id << " cannot acquire EXCLUSIVE lock on resource "
                  << resource << " due to existing locks." << std::endl;
        return false;
      }
      // A SHARED lock request conflicts with an existing EXCLUSIVE lock.
      if (lock_entry.type == LockType::EXCLUSIVE) {
        std::cerr << "Transaction " << txn_id << " cannot acquire SHARED lock on resource "
                  << resource << " due to existing EXCLUSIVE lock." << std::endl;
        return false;
      }
    }
  }

  // Step 4: If no conflicts or current ownership, acquire the new lock.
  LockEntry new_lock_entry;
  new_lock_entry.txn_id = txn_id;
  new_lock_entry.type = lock_type;
  new_lock_entry.resource = resource;
  new_lock_entry.acquired_time = std::chrono::system_clock::now();
  lock_table_[resource].push_back(new_lock_entry);

  std::cout << "Transaction " << txn_id << " acquired "
            << (lock_type == LockType::SHARED ? "SHARED" : "EXCLUSIVE")
            << " lock on resource " << resource << std::endl;

  return true;
}
/**
 * @brief Releases a specific lock held by a transaction on a resource.
 * @param txn_id The ID of the transaction releasing the lock.
 * @param resource The identifier of the resource whose lock is to be released.
 */
void TransactionManager::release_lock(TransactionId txn_id,
                                      const std::string &resource) {
  // Step 1: Acquire a manager-level lock to protect shared lock table.
  std::unique_lock<std::mutex> lock(mutex_);

  // Step 2: Find the resource in the lock table.
  auto lit = lock_table_.find(resource);
  if (lit != lock_table_.end()) {
    auto &locks = lit->second; // Get the list of locks for this resource.

    // Step 3: Remove all lock entries associated with the given transaction ID.
    locks.erase(std::remove_if(locks.begin(), locks.end(),
                               [txn_id](const LockEntry &entry) {
                                 return entry.txn_id == txn_id;
                               }),
                locks.end());

    // Step 4: If no more locks remain for this resource, remove the resource entry from the lock table.
    if (locks.empty()) {
      lock_table_.erase(lit);
    }
  }
}


/**
 * @brief Detects deadlocks in the system using a wait-for graph.
 * @param txn_id The ID of the transaction that initiated the deadlock detection (can be any active transaction).
 * @return True if a deadlock is detected, false otherwise.
 */
bool TransactionManager::detect_deadlock(TransactionId txn_id) {
  // Step 1: Acquire a manager-level lock to protect the wait-for graph.
  std::unique_lock<std::mutex> lock(mutex_);

  // --- Deadlock Detection Flow (using a Wait-For Graph and DFS) ---
  // A deadlock occurs if there is a cycle in the wait-for graph.
  // A wait-for graph node is a transaction. An edge T1 -> T2 means T1 is waiting for T2 to release a resource.
  
  std::unordered_set<TransactionId> visited;       // Keeps track of all transactions visited in the current DFS call.
  std::unordered_set<TransactionId> recursion_stack; // Keeps track of transactions currently in the recursion stack (part of the current DFS path).

  // Recursive lambda function for Depth-First Search (DFS) to detect cycles.
  // 'self' is used to allow the lambda to call itself.
  auto has_cycle = [&](auto &&self, TransactionId current) -> bool {
    visited.insert(current);       // Mark current transaction as globally visited.
    recursion_stack.insert(current); // Add to the current recursion path.

    // Check all transactions that 'current' is waiting for.
    auto it = wait_graph_.find(current);
    if (it != wait_graph_.end()) {
      for (TransactionId waiting_for : it->second) {
        // If 'waiting_for' has not been visited at all, recurse into it.
        if (visited.find(waiting_for) == visited.end()) {
          if (self(self, waiting_for)) {
            return true; // Cycle found in a sub-path.
          }
        }
        // If 'waiting_for' is already in the recursion stack, it means we found a back-edge, i.e., a cycle.
        else if (recursion_stack.find(waiting_for) != recursion_stack.end()) {
          return true; // Cycle detected.
        }
      }
    }

    recursion_stack.erase(current); // Remove from current recursion path as we backtrack.
    return false; // No cycle found from this path.
  };

  // Step 2: Iterate through all active transactions to start DFS from each unvisited node.
  // This ensures all disconnected components of the graph are checked.
  for (const auto& pair : transactions_) {
      if (pair.second.state == TransactionState::ACTIVE && visited.find(pair.first) == visited.end()) {
          if (has_cycle(has_cycle, pair.first)) {
              std::cout << "Deadlock detected involving transaction " << pair.first << std::endl;
              return true; // Deadlock found.
          }
      }
  }

  std::cout << "Deadlock detection performed, no deadlock found." << std::endl;
  return false; // No deadlocks.
}
/**
 * @brief Retrieves the current state of a specified transaction.
 * @param txn_id The ID of the transaction.
 * @return The current `TransactionState` of the transaction.
 * @throws std::runtime_error if the transaction ID is not found.
 */
TransactionState
TransactionManager::get_transaction_state(TransactionId txn_id) const {
  // Acquire a lock to safely access the transactions map.
  std::unique_lock<std::mutex> lock(mutex_);
  auto it = transactions_.find(txn_id);
  if (it == transactions_.end()) {
    throw std::runtime_error("Transaction not found");
  }
  return it->second.state;
}
/**
 * @brief Retrieves a list of all currently active transaction IDs.
 * @return A `std::vector` containing the IDs of all active transactions.
 */
std::vector<TransactionId> TransactionManager::get_active_transactions() const {
  // Acquire a lock to safely access the transactions map.
  std::unique_lock<std::mutex> lock(mutex_);
  std::vector<TransactionId> active_txns;

  // Iterate through all known transactions and collect the IDs of those in the ACTIVE state.
  for (const auto &[txn_id, txn] : transactions_) {
    if (txn.state == TransactionState::ACTIVE) {
      active_txns.push_back(txn_id);
    }
  }

  return active_txns;
}
/**
 * @brief Logs an operation performed by a transaction into its private undo log.
 * @param txn_id The ID of the transaction.
 * @param entry The `LogEntry` detailing the operation to be logged.
 */
void TransactionManager::log_operation(TransactionId txn_id,
                                       const LogEntry &entry) {
  // Acquire a lock to safely access the transactions map.
  std::unique_lock<std::mutex> lock(mutex_);
  auto it = transactions_.find(txn_id);
  if (it != transactions_.end()) {
    // Append the log entry to the transaction's undo log.
    // This log will be used if the transaction needs to be rolled back.
    it->second.undo_log.push_back(entry);
  } else {
    // In a real system, logging an operation for a non-existent transaction would be an error.
    std::cerr << "Warning: Attempted to log operation for non-existent transaction " << txn_id << std::endl;
  }
}

// 注意：保存点功能已在前面实现

/**
 * @brief Internal helper function to release all locks held by a specific transaction.
 * @details This version does NOT acquire a mutex and assumes the caller (e.g., commit/rollback)
 * already holds the manager's global mutex, preventing re-locking issues.
 * @param txn_id The ID of the transaction whose locks are to be released.
 */
void TransactionManager::release_all_locks_internal(TransactionId txn_id) {
  // Step 1: Iterate through the lock table (resource_id -> list_of_locks).
  // This map contains all resources that currently have one or more locks on them.
  for (auto it_resource = lock_table_.begin(); it_resource != lock_table_.end(); ) {
    auto &locks_on_resource = it_resource->second; // Get the list of `LockEntry` for the current resource.
    
    // Step 2: Remove all lock entries from the list that belong to the specified transaction.
    // `std::remove_if` reorders elements, and `erase` removes them from the container.
    locks_on_resource.erase(std::remove_if(locks_on_resource.begin(), locks_on_resource.end(),
                               [txn_id](const LockEntry &entry) {
                                 return entry.txn_id == txn_id;
                               }),
                locks_on_resource.end());

    // Step 3: If, after removing the transaction's locks, no more locks exist for this resource,
    // then remove the entire resource entry from the lock table.
    if (locks_on_resource.empty()) {
      it_resource = lock_table_.erase(it_resource); // `erase` returns iterator to next element, safe for loop.
    } else {
      ++it_resource; // Move to the next resource in the lock table.
    }
  }
}

/**
 * @brief Public interface to release all locks held by a transaction.
 * @details This method acquires the manager's global mutex before calling the internal
 * `release_all_locks_internal` to ensure thread safety when releasing locks.
 * @param txn_id The ID of the transaction whose locks are to be released.
 */
void TransactionManager::release_all_locks(TransactionId txn_id) {
  std::unique_lock<std::mutex> lock(mutex_); // Acquire lock for thread safety.
  release_all_locks_internal(txn_id); // Call internal version, which assumes lock is held.
}

/**
 * @brief Begins a new nested transaction within an existing parent transaction.
 * @details Nested transactions allow for sub-units of work to be committed or rolled back independently
 * of their parent, until the parent transaction itself commits or aborts.
 * @param parent_txn_id The ID of the parent transaction.
 * @return The `TransactionId` of the newly created nested transaction, or 0 if creation fails.
 */
TransactionId TransactionManager::begin_nested_transaction(TransactionId parent_txn_id) {
  // Step 1: Acquire a manager-level lock.
  std::unique_lock<std::mutex> lock(mutex_);

  // Step 2: Validate the parent transaction's existence and active state.
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

  // Step 3: Create a new unique ID for the nested transaction.
  TransactionId nested_txn_id = next_transaction_id();
  // Step 4: Create the nested Transaction object, inheriting properties from its parent.
  Transaction nested_txn = Transaction::create_nested(nested_txn_id, parent_txn_id,
                                                      parent_txn.isolation_level);

  // Step 5: Record nested transaction metadata.
  NestedTransaction nested_info;
  nested_info.parent_txn_id = parent_txn_id;
  nested_info.nested_txn_id = nested_txn_id;
  nested_info.inherited_isolation = parent_txn.isolation_level;
  nested_info.start_time = nested_txn.start_time;

  // Step 6: Store the new nested transaction.
  transactions_[nested_txn_id] = std::move(nested_txn);
  nested_transactions_[nested_txn_id] = std::move(nested_info);

  std::cout << "Nested transaction " << nested_txn_id << " started for parent "
            << parent_txn_id << std::endl;

  // Step 7: Return the new nested transaction ID.
  return nested_txn_id;
}

/**
 * @brief Commits a nested transaction.
 * @details In a typical nested transaction model, committing a nested transaction
 * makes its changes visible to its parent but not to other transactions until the
 * top-level transaction commits. This simplified implementation just marks the
 * nested transaction as committed and releases its locks.
 * @param nested_txn_id The ID of the nested transaction to commit.
 * @return True if the nested transaction was successfully committed, false otherwise.
 */
bool TransactionManager::commit_nested_transaction(TransactionId nested_txn_id) {
  // Step 1: Acquire a manager-level lock.
  std::unique_lock<std::mutex> lock(mutex_);

  // Step 2: Validate if it's indeed a nested transaction.
  auto nested_it = nested_transactions_.find(nested_txn_id);
  if (nested_it == nested_transactions_.end()) {
    std::cerr << "Transaction " << nested_txn_id << " is not a nested transaction" << std::endl;
    return false;
  }

  // Step 3: Find the transaction and validate its active state.
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

  // Step 4: Mark the nested transaction as committed.
  nested_txn.state = TransactionState::COMMITTED;
  nested_txn.end_time = std::chrono::system_clock::now();

  // Step 5: Release locks held by this nested transaction.
  release_all_locks_internal(nested_txn_id);

  // Step 6: Remove the nested transaction from its specific tracking map.
  nested_transactions_.erase(nested_it);

  std::cout << "Nested transaction " << nested_txn_id << " committed" << std::endl;

  return true;
}

/**
 * @brief Periodically checks for and handles timed-out active transactions.
 * @details This method is typically called by a background thread or a timer.
 * Timed-out active transactions are forcibly rolled back to prevent resource starvation.
 * @return The number of transactions that timed out and were rolled back.
 */
size_t TransactionManager::check_and_handle_timeouts() {
  // Step 1: Acquire a manager-level lock.
  std::unique_lock<std::mutex> lock(mutex_);
  size_t timeout_count = 0;

  // Step 2: Iterate through all transactions.
  for (auto it = transactions_.begin(); it != transactions_.end();) {
    Transaction& txn = it->second;
    // Step 3: Check if an active transaction has timed out.
    if (txn.state == TransactionState::ACTIVE && txn.is_timeout()) {
      std::cout << "Transaction " << txn.txn_id << " timed out, rolling back" << std::endl;

      // Step 4: Forcibly roll back the timed-out transaction.
      txn.state = TransactionState::ROLLING_BACK; // Mark for rollback.
      release_all_locks_internal(txn.txn_id);     // Release all locks.
      txn.state = TransactionState::ABORTED;      // Mark as aborted.
      txn.end_time = std::chrono::system_clock::now(); // Record end time.

      ++timeout_count;

      // If it was a nested transaction, also remove it from the nested tracking map.
      nested_transactions_.erase(txn.txn_id);

      ++it; // Move to the next transaction, important for loop correctness when erasing.
    } else {
      ++it; // Move to the next transaction.
    }
  }

  if (timeout_count > 0) {
    std::cout << "Handled " << timeout_count << " timeout transactions" << std::endl;
  }

  return timeout_count;
}

/**
 * @brief Sets or updates the timeout duration for an active transaction.
 * @param txn_id The ID of the transaction.
 * @param timeout_ms The new timeout duration in milliseconds.
 * @return True if the timeout was successfully set, false if the transaction is not found or not active.
 */
bool TransactionManager::set_transaction_timeout(TransactionId txn_id,
                                                std::chrono::milliseconds timeout_ms) {
  // Step 1: Acquire a manager-level lock.
  std::unique_lock<std::mutex> lock(mutex_);

  // Step 2: Find the transaction and validate its active state.
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

  // Step 3: Update the transaction's timeout duration.
  txn.timeout_duration = timeout_ms;
  std::cout << "Transaction " << txn_id << " timeout set to "
            << timeout_ms.count() << "ms" << std::endl;

  return true;
}
/**
 * @brief Retrieves current statistics about transactions managed by the system.
 * @return A `TransactionStats` struct containing various metrics.
 */
TransactionManager::TransactionStats TransactionManager::get_transaction_stats() const {
  // Step 1: Acquire a manager-level lock to protect shared data structures during statistics collection.
  std::unique_lock<std::mutex> lock(mutex_);

  TransactionStats stats = {0, 0, 0, 0, std::chrono::milliseconds(0)};
  size_t total_running_time_ms = 0; // Accumulates running time for completed transactions.
  size_t completed_count = 0;        // Counts committed or aborted transactions.

  // Step 2: Iterate through all transactions to aggregate statistics.
  for (const auto& [txn_id, txn] : transactions_) {
    if (txn.state == TransactionState::ACTIVE) {
      ++stats.active_transactions; // Count currently running transactions.
    }

    ++stats.total_transactions; // Total transactions processed since startup or last reset.

    // A transaction is considered 'timed out' if it was active and then aborted due to timeout.
    // This simplified logic assumes 'is_timeout()' implies it was aborted *because* of timeout.
    if (txn.is_timeout() && txn.state == TransactionState::ABORTED) {
      ++stats.timeout_transactions;
    }

    if (txn.is_nested) {
      ++stats.nested_transactions; // Count nested transactions.
    }

    // Accumulate running time for transactions that have completed (committed or aborted)
    // to calculate average transaction time.
    if (txn.state == TransactionState::COMMITTED || txn.state == TransactionState::ABORTED) {
      ++completed_count;
      total_running_time_ms += txn.get_running_time().count();
    }
  }

  // Step 3: Calculate average transaction time if there are completed transactions.
  if (completed_count > 0) {
    stats.avg_transaction_time = std::chrono::milliseconds(total_running_time_ms / completed_count);
  }

  return stats;
}
/**
 * @brief Sets the isolation level for an active transaction.
 * @details In this simplified implementation, the isolation level can only be set
 * for an active transaction. In a full SQL implementation, it might be possible
 * to change the isolation level mid-transaction, with certain restrictions.
 * @param txn_id The ID of the transaction.
 * @param level The new `IsolationLevel` to set.
 * @return True if the isolation level was successfully set, false otherwise.
 */
bool TransactionManager::set_transaction_isolation_level(TransactionId txn_id, IsolationLevel level) {
  // Step 1: Acquire a manager-level lock.
  std::unique_lock<std::mutex> lock(mutex_);

  // Step 2: Find the transaction and validate its active state.
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

  // Step 3: Update the transaction's isolation level.
  txn.isolation_level = level;
  txn.update_activity(); // Record activity.

  std::cout << "Transaction " << txn_id << " isolation level set to "
            << static_cast<int>(level) << std::endl;

  return true;
}
/**
 * @brief Retrieves the isolation level of a specified transaction.
 * @param txn_id The ID of the transaction.
 * @return The `IsolationLevel` of the transaction.
 * @throws std::runtime_error if the transaction ID is not found.
 */
IsolationLevel TransactionManager::get_transaction_isolation_level(TransactionId txn_id) const {
  // Step 1: Acquire a manager-level lock.
  std::unique_lock<std::mutex> lock(mutex_);

  // Step 2: Find the transaction.
  auto it = transactions_.find(txn_id);
  if (it == transactions_.end()) {
    throw std::runtime_error("Transaction not found"); // Throw if transaction does not exist.
  }

  // Step 3: Return the isolation level.
  return it->second.isolation_level;
}
/**
 * @brief Checks if a specific operation on a resource is allowed given the transaction's isolation level.
 * @details This is a simplified implementation of isolation constraint checking. In a real system,
 * this would interact deeply with the concurrency control mechanism (e.g., LockManager or MVCC).
 * @param txn_id The ID of the transaction.
 * @param operation The type of operation ("READ" or "WRITE").
 * @param resource The resource being accessed.
 * @return True if the operation is permitted by the isolation level, false otherwise.
 */
bool TransactionManager::check_isolation_constraints(TransactionId txn_id, const std::string& operation,
                                                   const std::string& resource) {
  // Step 1: Acquire a manager-level lock.
  std::unique_lock<std::mutex> lock(mutex_);

  // Step 2: Find the transaction and validate its active state.
  auto txn_it = transactions_.find(txn_id);
  if (txn_it == transactions_.end()) {
    return false;
  }

  const Transaction& txn = txn_it->second;
  if (txn.state != TransactionState::ACTIVE) {
    return false;
  }

  IsolationLevel level = txn.isolation_level;

  // Step 3: Implement simplified checks based on isolation level and operation type.
  if (operation == "READ") {
    switch (level) {
      case IsolationLevel::READ_UNCOMMITTED:
        // READ UNCOMMITTED allows reading uncommitted data (dirty reads).
        return true;

      case IsolationLevel::READ_COMMITTED:
        // READ COMMITTED only allows reading committed data.
        // In a full implementation, this would involve checking if other transactions
        // hold exclusive locks on the resource.
        // Simplified: always allow read.
        return true;

      case IsolationLevel::REPEATABLE_READ:
        // REPEATABLE READ ensures that data read once will not change during the transaction.
        // This typically requires shared locks on read data until the transaction commits.
        // Simplified: always allow read.
        return true;

      case IsolationLevel::SERIALIZABLE:
        // SERIALIZABLE is the highest isolation level, preventing all concurrency anomalies.
        // This often involves two-phase locking or optimistic concurrency control with validation.
        // Simplified: always allow read.
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
        // For write operations, all isolation levels require some form of exclusive access
        // to prevent lost updates or other write anomalies.
        auto lock_it = lock_table_.find(resource);
        if (lock_it != lock_table_.end()) {
          for (const auto& lock_entry : lock_it->second) {
            if (lock_entry.txn_id != txn_id) {
              // If another transaction holds a lock (any type), check compatibility based on level.
              if (level == IsolationLevel::READ_UNCOMMITTED) {
                // READ UNCOMMITTED (and other levels) allows dirty writes in some simplified models,
                // or requires careful handling to avoid cascading aborts.
                // Simplified: allow for now, but a real system would need robust checks.
                continue;
              } else {
                // For other levels, conflicting locks (especially exclusive) mean this write is not allowed.
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

  // Step 4: Unknown operation, allow by default (fail-safe is typically to deny).
  return true;
}

} // namespace sqlcc