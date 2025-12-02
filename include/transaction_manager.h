#ifndef SQLCC_TRANSACTION_MANAGER_H
#define SQLCC_TRANSACTION_MANAGER_H

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace sqlcc {

/**
 * 事务隔离级别
 */
enum class IsolationLevel {
  READ_UNCOMMITTED,
  READ_COMMITTED,
  REPEATABLE_READ,
  SERIALIZABLE
};

/**
 * 事务状态
 */
enum class TransactionState { ACTIVE, COMMITTED, ABORTED, ROLLING_BACK };

/**
 * 事务标识符
 */
using TransactionId = uint64_t;

/**
 * 操作日志条目，用于实现多版本并发控制
 */
struct LogEntry {
  TransactionId txn_id;
  std::string table_name;
  std::string operation; // INSERT, UPDATE, DELETE
  size_t record_id;
  std::chrono::system_clock::time_point timestamp;
  std::vector<char> old_data;
  std::vector<char> new_data;
};

/**
 * 事务信息
 */
struct Transaction {
  TransactionId txn_id;
  IsolationLevel isolation_level;
  TransactionState state;
  std::chrono::system_clock::time_point start_time;
  std::chrono::system_clock::time_point end_time;
  std::unordered_set<std::string> read_tables;
  std::unordered_set<std::string> write_tables;
  std::vector<LogEntry> undo_log;

  Transaction() = default;
  Transaction(TransactionId id, IsolationLevel level);
};

/**
 * 锁类型
 */
enum class LockType { SHARED, EXCLUSIVE };

/**
 * 锁信息
 */
struct LockEntry {
  TransactionId txn_id;
  std::string resource;
  LockType type;
  std::chrono::system_clock::time_point acquired_time;
};

/**
 * 事务管理器 - 核心事务处理组件
 */
class TransactionManager {
public:
  /**
   * 构造函数
   */
  TransactionManager();

  /**
   * 析构函数
   */
  ~TransactionManager();

  /**
   * 开始新事务
   * @param isolation_level 隔离级别
   * @return 事务ID
   */
  TransactionId begin_transaction(
      IsolationLevel isolation_level = IsolationLevel::READ_COMMITTED);

  /**
   * 提交事务
   * @param txn_id 事务ID
   * @return 是否成功
   */
  bool commit_transaction(TransactionId txn_id);

  /**
   * 回滚事务
   * @param txn_id 事务ID
   * @return 是否成功
   */
  bool rollback_transaction(TransactionId txn_id);

  /**
   * 创建保存点
   * @param txn_id 事务ID
   * @param savepoint_name 保存点名称
   * @return 是否成功
   */
  bool create_savepoint(TransactionId txn_id,
                        const std::string &savepoint_name);

  /**
   * 回滚到保存点
   * @param txn_id 事务ID
   * @param savepoint_name 保存点名称
   * @return 是否成功
   */
  bool rollback_to_savepoint(TransactionId txn_id,
                             const std::string &savepoint_name);

  /**
   * 获取锁
   * @param txn_id 事务ID
   * @param resource 资源标识
   * @param lock_type 锁类型
   * @param wait 是否等待锁
   * @return 是否获取成功
   */
  bool acquire_lock(TransactionId txn_id, const std::string &resource,
                    LockType lock_type, bool wait = true);

  /**
   * 释放锁
   * @param txn_id 事务ID
   * @param resource 资源标识
   */
  void release_lock(TransactionId txn_id, const std::string &resource);

  /**
   * 检测死锁
   * @param txn_id 事务ID
   * @return 是否存在死锁
   */
  bool detect_deadlock(TransactionId txn_id);

  /**
   * 获取事务状态
   * @param txn_id 事务ID
   * @return 事务状态
   */
  TransactionState get_transaction_state(TransactionId txn_id) const;

  /**
   * 获取活动事务列表
   * @return 活动事务ID列表
   */
  std::vector<TransactionId> get_active_transactions() const;

  /**
   * 记录事务操作到日志
   * @param txn_id 事务ID
   * @param entry 日志条目
   */
  void log_operation(TransactionId txn_id, const LogEntry &entry);

  /**
   * 获取下一个事务ID
   * @return 新事务ID
   */
  TransactionId next_transaction_id();

private:
  /**
   * 清理已完成的事务
   */
  void cleanup_completed_transactions();

  /**
   * 检查是否可以获取锁（用于死锁检测）
   * @param txn_id 事务ID
   * @param resource 资源
   * @param lock_type 锁类型
   * @return 是否可以获取
   */
  bool can_acquire_lock(TransactionId txn_id, const std::string &resource,
                        LockType lock_type) const;

  /**
   * 释放事务持有的所有锁
   * @param txn_id 事务ID
   */
  void release_all_locks(TransactionId txn_id);

  /**
   * 释放事务持有的所有锁（内部版本，不加锁）
   * @param txn_id 事务ID
   */
  void release_all_locks_internal(TransactionId txn_id);

  /**
   * 等待图结构（用于死锁检测）
   */
  std::unordered_map<TransactionId, std::unordered_set<TransactionId>>
      wait_graph_;

  /**
   * 锁表
   */
  std::unordered_map<std::string, std::vector<LockEntry>> lock_table_;

  /**
   * 事务表
   */
  std::unordered_map<TransactionId, Transaction> transactions_;

  /**
   * 事务ID生成器
   */
  std::atomic<TransactionId> next_txn_id_;

  /**
   * 互斥锁
   */
  mutable std::mutex mutex_;
};

} // namespace sqlcc

#endif // SQLCC_TRANSACTION_MANAGER_H
