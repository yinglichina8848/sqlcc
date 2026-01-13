/**
 * @file transaction_manager.h
 * @brief SQLCC事务管理器 - ACID属性实现的完整解决方案
 *
 * 事务管理器是数据库系统的核心组件之一，负责实现ACID（原子性、一致性、
 * 隔离性、持久性）四大基本属性。本文件定义了完整的事务管理架构，
 * 包括两阶段锁协议、预写日志、多版本并发控制等关键技术。
 *
 * 📚 配套教材参考：
 * - [第8章：OLTP事务处理](../../textbook/《数据库系统原理与开发实践》.md#第八章oltp事务处理)
 * - [8.1 ACID属性深度实现](../../textbook/《数据库系统原理与开发实践》.md#81-acid属性深度实现)
 * - [8.2 预写日志（WAL）机制](../../textbook/《数据库系统原理与开发实践》.md#82-预写日志wal机制)
 * - [8.3 并发控制与MVCC技术](../../textbook/《数据库系统原理与开发实践》.md#83-并发控制与mvcc技术)
 *
 * WHY层 - 设计意图：
 *   事务是数据库系统的核心抽象，提供数据操作的原子性和一致性保证。
 *   TransactionManager作为数据库引擎的核心组件，负责协调多用户并发访问，
 *   确保数据的一致性和完整性。通过精心设计的锁协议和日志机制，
 *   实现高效且可靠的事务处理能力。
 *
 * WHAT层 - 功能说明：
 *   - 事务生命周期管理：创建、提交、回滚事务
 *   - 并发控制：两阶段锁协议 + 死锁检测
 *   - 隔离级别：支持多种SQL隔离级别
 *   - 持久性保证：通过WAL机制确保事务持久性
 *   - 保存点支持：嵌套事务的回滚控制
 *
 * HOW层 - 实现机制：
 *   - 原子性：通过undo日志实现事务回滚
 *   - 一致性：通过锁协议和约束检查保证
 *   - 隔离性：通过锁和MVCC实现并发隔离
 *   - 持久性：通过预写日志确保故障恢复
 *
 * 关键设计决策：
 *   1. 采用两阶段锁协议（2PL）确保串行化调度
 *   2. 使用等待图进行死锁检测和预防
 *   3. 支持多种隔离级别以平衡性能和一致性
 *   4. 通过原子操作生成全局唯一事务ID
 *
 * 并发控制策略：
 *   - **严格两阶段锁**：事务获取所有锁后才能释放锁
 *   - **死锁检测**：使用等待图算法检测循环依赖
 *   - **锁升级**：从共享锁升级到排他锁的策略
 *   - **超时机制**：防止无限期等待的保护措施
 *
 * 性能优化考虑：
 *   - **锁粒度**：行级锁vs表级锁的选择
 *   - **锁兼容性**：共享锁和排他锁的兼容矩阵
 *   - **锁管理**：高效的锁表实现和垃圾回收
 *   - **并发度**：最大化并发事务数量
 *
 * 故障恢复机制：
 *   - **检查点**：定期创建系统一致性快照
 *   - **重做日志**：记录事务提交后的数据修改
 *   - **撤销日志**：记录事务执行中的数据修改
 *   - **ARIES协议**：先进的数据库恢复算法
 *
 * 扩展性设计：
 *   - **分布式事务**：支持跨节点事务协调
 *   - **长事务处理**：补偿事务和SAGA模式
 *   - **实时事务**：低延迟高可用性保障
 *   - **混合负载**：OLTP+OLAP混合工作负载支持
 *
 * @author SQLCC技术委员会
 * @version 1.2.6
 * @date 2025-12-24
 */

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

#include "core/core_database_manager.h"
#include "storage/concurrency_control.h"

namespace sqlcc {

/**
 * @brief 事务状态枚举 - 定义事务生命周期的各个阶段
 *
 * WHY层 - 设计意图：
 *   事务状态是事务管理器的核心状态机，精确描述事务从开始到结束的完整生命周期。
 *   状态转换必须遵循严格的规则，确保事务的ACID属性得到正确维护。
 *
 * WHAT层 - 状态说明：
 *   - ACTIVE: 事务正在执行，可以进行数据操作
 *   - COMMITTED: 事务已成功提交，所有修改永久保存
 *   - ABORTED: 事务已失败终止，所有修改已回滚
 *   - ROLLING_BACK: 事务正在回滚过程中
 *
 * HOW层 - 状态转换规则：
 *   ACTIVE → COMMITTED: 成功执行COMMIT命令
 *   ACTIVE → ABORTED: 遇到错误或执行ROLLBACK命令
 *   ACTIVE → ROLLING_BACK: 开始回滚过程
 *   ROLLING_BACK → ABORTED: 回滚完成
 *
 * 状态机约束：
 *   - 一旦进入COMMITTED或ABORTED状态，事务生命周期结束
 *   - ROLLING_BACK是COMMITTED和ABORTED之间的过渡状态
 *   - 只有ACTIVE状态的事务可以进行数据修改
 */
enum class TransactionState {
  ACTIVE,       ///< 事务活跃状态，可以执行数据操作
  COMMITTED,    ///< 事务已提交，修改永久保存
  ABORTED,      ///< 事务已中止，修改已回滚
  ROLLING_BACK  ///< 事务正在回滚过程中
};

/**
 * @brief 操作日志条目 - 多版本并发控制（MVCC）的核心数据结构
 *
 * WHY层 - 设计意图：
 *   LogEntry是实现MVCC和故障恢复的关键数据结构，为每个事务操作记录完整的
 *   前后镜像信息。通过日志记录实现事务的原子性和持久性，支持并发事务的
 *   版本控制和故障后的数据恢复。
 *
 * WHAT层 - 记录内容：
 *   - 事务标识：关联到具体的事务
 *   - 操作对象：表名和记录标识
 *   - 操作类型：INSERT/UPDATE/DELETE
 *   - 时间戳：操作发生的时间点
 *   - 数据镜像：操作前后的完整数据
 *
 * HOW层 - MVCC实现机制：
 *   - 版本链：通过时间戳建立数据版本链
 *   - 可见性规则：根据事务隔离级别确定版本可见性
 *   - 垃圾回收：清理不再需要的旧版本数据
 *   - 冲突检测：检测并发事务间的写-写冲突
 *
 * 存储优化：
 *   - 压缩存储：对重复数据进行压缩
 *   - 索引组织：按事务ID和时间戳建立索引
 *   - 分层存储：热数据和冷数据的分层管理
 *   - 批量写入：日志的批量写入优化
 *
 * 故障恢复应用：
 *   - 重做（Redo）：故障后重放已提交事务的操作
 *   - 撤销（Undo）：故障后撤销未提交事务的操作
 *   - 检查点：定期创建一致性快照点
 */
struct LogEntry {
  TransactionId txn_id;                           ///< 事务ID，关联到具体事务
  std::string table_name;                         ///< 操作的表名
  std::string operation;                          ///< 操作类型：INSERT/UPDATE/DELETE
  size_t record_id;                               ///< 记录ID，标识具体的数据行
  std::chrono::system_clock::time_point timestamp; ///< 操作时间戳，用于版本控制
  std::vector<char> old_data;                     ///< 操作前的数据镜像（撤销用）
  std::vector<char> new_data;                     ///< 操作后的数据镜像（重做用）
};

/**
 * @brief 嵌套事务信息 - 支持事务嵌套的核心数据结构
 *
 * WHY层 - 设计意图：
 *   NestedTransaction结构体支持复杂的企业级事务场景，实现事务的层次化管理。
 *   通过嵌套事务，可以在不影响父事务的情况下执行子事务操作，提供更灵活的事务控制。
 *
 * WHAT层 - 嵌套事务属性：
 *   - 父子关系：明确的父子事务关联
 *   - 隔离继承：子事务继承父事务的隔离级别
 *   - 保存点继承：继承父事务的保存点状态
 *   - 独立回滚：子事务可以独立回滚而不影响父事务
 *
 * HOW层 - 嵌套机制：
 *   - 作用域隔离：子事务的操作在独立的作用域中
 *   - 锁继承：子事务可以继承父事务的某些锁
 *   - 状态同步：父子事务间的状态协调机制
 *   - 资源管理：嵌套事务的资源生命周期管理
 */
struct NestedTransaction {
  TransactionId parent_txn_id;                    ///< 父事务ID
  TransactionId nested_txn_id;                    ///< 子事务ID
  IsolationLevel inherited_isolation;             ///< 继承的隔离级别
  std::vector<std::string> inherited_savepoints;  ///< 继承的保存点列表
  std::chrono::system_clock::time_point start_time; ///< 子事务开始时间
};

/**
 * @brief 事务信息结构体 - 事务元数据的完整描述
 *
 * WHY层 - 设计意图：
 *   Transaction结构体封装了事务的完整状态信息，是事务管理器的核心数据结构。
 *   通过集中管理事务元数据，实现对事务生命周期的精确控制和监控，为并发控制、
 *   故障恢复和性能优化提供必要的信息支持。
 *
 * WHAT层 - 事务属性：
 *   - 标识信息：全局唯一的事务ID
 *   - 隔离属性：事务的隔离级别设置
 *   - 状态信息：事务的当前执行状态
 *   - 时间信息：事务的开始和结束时间
 *   - 访问信息：事务读写的表集合
 *   - 日志信息：事务的操作历史记录
 *   - 超时管理：事务超时控制和自动回滚
 *   - 嵌套支持：支持事务嵌套的层次管理
 *
 * HOW层 - 并发控制应用：
 *   - 读写集合：用于冲突检测和死锁预防
 *   - 隔离级别：控制事务间的可见性和冲突处理
 *   - 状态同步：确保多线程环境下的状态一致性
 *   - 性能监控：统计事务的执行时间和资源使用
 *   - 超时检测：自动检测和处理超时的长事务
 *   - 嵌套管理：协调父子事务间的状态和资源
 *
 * 内存管理优化：
 *   - 对象池复用：减少事务对象的创建销毁开销
 *   - 延迟初始化：按需初始化复杂的数据结构
 *   - 引用计数：智能指针管理资源生命周期
 *   - 缓存友好：优化数据结构在内存中的布局
 */
struct Transaction {
  TransactionId txn_id;                           ///< 全局唯一事务标识符
  IsolationLevel isolation_level;                 ///< 事务隔离级别
  TransactionState state;                         ///< 当前事务状态
  std::chrono::system_clock::time_point start_time; ///< 事务开始时间
  std::chrono::system_clock::time_point end_time;   ///< 事务结束时间
  std::chrono::milliseconds timeout_duration;     ///< 事务超时时间
  bool is_nested;                                 ///< 是否为嵌套事务
  TransactionId parent_txn_id;                    ///< 父事务ID（嵌套事务）
  std::unordered_set<std::string> read_tables;    ///< 读取的表集合
  std::unordered_set<std::string> write_tables;   ///< 写入的表集合
  std::vector<LogEntry> undo_log;                 ///< 撤销日志，用于事务回滚
  size_t operation_count;                         ///< 事务操作计数，用于监控
  std::chrono::system_clock::time_point last_activity; ///< 最后活动时间

  /**
   * 默认构造函数
   */
  Transaction();

  /**
   * 参数化构造函数
   * @param id 事务ID
   * @param level 隔离级别
   * @param timeout_ms 超时时间（毫秒）
   */
  Transaction(TransactionId id, IsolationLevel level,
               std::chrono::milliseconds timeout_ms = std::chrono::milliseconds(30000));

  /**
   * 创建嵌套事务
   * @param id 子事务ID
   * @param parent_id 父事务ID
   * @param inherited_level 继承的隔离级别
   * @return 嵌套事务对象
   */
  static Transaction create_nested(TransactionId id, TransactionId parent_id,
                                  IsolationLevel inherited_level);

  /**
   * 检查事务是否超时
   * @return 是否已超时
   */
  bool is_timeout() const;

  /**
   * 更新最后活动时间
   */
  void update_activity();

  /**
   * 获取事务运行时间
   * @return 运行时间（毫秒）
   */
  std::chrono::milliseconds get_running_time() const;
};



/**
 * @brief 锁信息结构体 - 锁表条目的完整描述
 *
 * WHY层 - 设计意图：
 *   LockEntry记录了锁的详细信息，支持复杂的锁管理策略。通过跟踪锁的获取时间、
 *   持有者和资源信息，实现死锁检测、锁超时和锁升级等高级功能，为并发控制
 *   提供精确的状态信息。
 *
 * WHAT层 - 锁信息内容：
 *   - 持有者：哪个事务持有这个锁
 *   - 资源：锁保护的具体资源（表名、行ID等）
 *   - 类型：共享锁还是排他锁
 *   - 时间：锁的获取时间戳
 *
 * HOW层 - 死锁检测应用：
 *   - 等待图构造：基于锁持有关系构建等待图
 *   - 循环检测：深度优先搜索检测死锁环
 *   - 牺牲者选择：选择合适的事务进行回滚
 *   - 锁升级策略：从行级锁升级到表级锁
 *
 * 性能监控：
 *   - 锁等待时间：统计事务等待锁的时间
 *   - 锁竞争率：计算锁的竞争激烈程度
 *   - 死锁频率：监控系统的死锁发生频率
 *   - 锁利用率：评估锁管理的有效性
 */
struct LockEntry {
  TransactionId txn_id;                          ///< 持有锁的事务ID
  std::string resource;                          ///< 被锁保护的资源标识
  LockType type;                                 ///< 锁的类型（共享或排他）
  std::chrono::system_clock::time_point acquired_time; ///< 锁的获取时间
};

/**
 * @brief 事务管理器类 - SQLCC数据库系统的ACID属性实现核心
 *
 * WHY层 - 设计意图：
 *   TransactionManager是数据库系统的"心脏"，负责协调所有并发事务，确保数据
 *   的一致性和完整性。通过精心设计的锁协议、日志机制和恢复策略，实现企业级
 *   数据库系统的四大ACID属性，为应用提供可靠的数据服务基础。
 *
 * WHAT层 - 核心功能：
 *   - 事务生命周期：创建、提交、回滚事务的完整管理
 *   - 并发控制：基于锁的并发访问控制和死锁检测
 *   - 隔离级别：支持多种SQL标准隔离级别的实现
 *   - 保存点：嵌套事务的回滚点管理
 *   - 故障恢复：基于日志的事务持久性保证
 *
 * HOW层 - 架构设计：
 *   - 锁管理器：维护锁表和等待队列
 *   - 日志系统：记录事务的所有操作历史
 *   - 死锁检测器：实时监控和预防死锁发生
 *   - 隔离控制器：根据隔离级别控制事务可见性
 *   - 恢复管理器：系统故障后的数据恢复
 *
 * ACID属性实现策略：
 *
 * 1. **原子性（Atomicity）**：
 *    - 机制：undo日志记录事务的所有修改
 *    - 提交时：释放所有锁，标记事务完成
 *    - 回滚时：根据undo日志逆序撤销所有修改
 *    - 保证：要么全部成功，要么全部失败
 *
 * 2. **一致性（Consistency）**：
 *    - 机制：锁协议确保事务串行化执行
 *    - 约束：完整性约束检查和业务规则验证
 *    - 隔离：不同隔离级别的一致性保证
 *    - 保证：事务将数据库从一个一致状态转换到另一个
 *
 * 3. **隔离性（Isolation）**：
 *    - 机制：锁和多版本并发控制（MVCC）
 *    - 级别：READ UNCOMMITTED到SERIALIZABLE
 *    - 冲突：读-写、写-读、写-写冲突的处理
 *    - 保证：事务间的相互隔离，避免干扰
 *
 * 4. **持久性（Durability）**：
 *    - 机制：预写日志（WAL）和检查点
 *    - 策略：日志先行，数据后写
 *    - 恢复：系统故障后的自动恢复
 *    - 保证：已提交事务的结果永久保存
 *
 * 性能优化策略：
 *   - **锁优化**：最小化锁的持有时间和粒度
 *   - **日志优化**：批量写入和日志压缩
 *   - **并发优化**：减少锁竞争和死锁发生
 *   - **缓存优化**：事务级缓存和预取策略
 *
 * 扩展性考虑：
 *   - **分布式事务**：跨节点事务的协调机制
 *   - **长事务处理**：补偿事务和SAGA模式支持
 *   - **实时事务**：低延迟高可用性保障
 *   - **混合负载**：OLTP和OLAP的混合优化
 *
 * 监控和诊断：
 *   - **性能指标**：事务吞吐量、响应时间、冲突率
 *   - **健康检查**：死锁检测、锁表大小、日志积压
 *   - **故障诊断**：事务状态追踪、错误日志分析
 *   - **容量规划**：基于负载模式的资源配置建议
 *
 * @note 事务管理器是数据库系统的核心组件，直接影响系统的并发性能和数据一致性
 * @note 实现了严格的两阶段锁协议，确保事务的串行化执行
 * @note 支持完整的SQL事务语法，包括保存点和嵌套事务
 * @note 通过日志机制保证事务的原子性和持久性
 *
 * @see LogEntry 操作日志条目的定义
 * @see Transaction 事务信息结构体的定义
 * @see LockEntry 锁信息结构体的定义
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

  /**
   * 开始嵌套事务
   * @param parent_txn_id 父事务ID
   * @return 子事务ID，失败返回0
   */
  TransactionId begin_nested_transaction(TransactionId parent_txn_id);

  /**
   * 提交嵌套事务
   * @param nested_txn_id 嵌套事务ID
   * @return 是否成功
   */
  bool commit_nested_transaction(TransactionId nested_txn_id);

  /**
   * 检查并处理超时事务
   * @return 处理的超时事务数量
   */
  size_t check_and_handle_timeouts();

  /**
   * 设置事务超时时间
   * @param txn_id 事务ID
   * @param timeout_ms 超时时间（毫秒）
   * @return 是否成功
   */
  bool set_transaction_timeout(TransactionId txn_id,
                              std::chrono::milliseconds timeout_ms);

  /**
   * 获取事务统计信息
   * @return 事务统计数据
   */
  struct TransactionStats {
    size_t active_transactions;
    size_t total_transactions;
    size_t timeout_transactions;
    size_t nested_transactions;
    std::chrono::milliseconds avg_transaction_time;
  };
  TransactionStats get_transaction_stats() const;

  /**
   * 设置事务隔离级别
   * @param txn_id 事务ID
   * @param level 隔离级别
   * @return 是否成功
   */
  bool set_transaction_isolation_level(TransactionId txn_id, IsolationLevel level);

  /**
   * 获取事务隔离级别
   * @param txn_id 事务ID
   * @return 隔离级别
   */
  IsolationLevel get_transaction_isolation_level(TransactionId txn_id) const;

  /**
   * 检查隔离级别约束
   * @param txn_id 事务ID
   * @param operation 操作类型
   * @param resource 资源标识
   * @return 是否允许操作
   */
  bool check_isolation_constraints(TransactionId txn_id, const std::string& operation,
                                  const std::string& resource);

  /**
   * 获取嵌套事务表
   */
  std::unordered_map<TransactionId, NestedTransaction> nested_transactions_;

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