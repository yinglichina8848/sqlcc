# TransactionManager类详细设计

## 概述

TransactionManager是事务管理器的核心类，负责事务的创建、提交、回滚等操作，以及并发控制、死锁检测和隔离级别管理。它确保数据库操作满足ACID特性。

## 类定义

```cpp
class TransactionManager {
public:
    TransactionManager();
    ~TransactionManager();
    
    TransactionId begin_transaction(
        IsolationLevel isolation_level = IsolationLevel::READ_COMMITTED);
    bool commit_transaction(TransactionId txn_id);
    bool rollback_transaction(TransactionId txn_id);
    bool create_savepoint(TransactionId txn_id, const std::string &savepoint_name);
    bool rollback_to_savepoint(TransactionId txn_id, const std::string &savepoint_name);
    bool acquire_lock(TransactionId txn_id, const std::string &resource,
                     LockType lock_type, bool wait = true);
    void release_lock(TransactionId txn_id, const std::string &resource);
    bool detect_deadlock(TransactionId txn_id);
    TransactionState get_transaction_state(TransactionId txn_id) const;
    std::vector<TransactionId> get_active_transactions() const;
    void log_operation(TransactionId txn_id, const LogEntry &entry);
    TransactionId next_transaction_id();

private:
    void cleanup_completed_transactions();
    bool can_acquire_lock(TransactionId txn_id, const std::string &resource,
                         LockType lock_type) const;
    void release_all_locks(TransactionId txn_id);

private:
    std::unordered_map<TransactionId, std::unordered_set<TransactionId>> wait_graph_;
    std::unordered_map<std::string, std::vector<LockEntry>> lock_table_;
    std::unordered_map<TransactionId, Transaction> transactions_;
    std::atomic<TransactionId> next_txn_id_;
    mutable std::mutex mutex_;
};
```

## 构造函数

### TransactionManager()

构造函数：

1. 初始化next_txn_id_为1
2. 初始化互斥锁

## 析构函数

### ~TransactionManager()

析构函数：

1. 清理资源

## 公共方法

### TransactionId begin_transaction(IsolationLevel isolation_level)

开始新事务：

1. 获取互斥锁
2. 生成新的事务ID
3. 创建Transaction对象
4. 将事务添加到transactions_映射表
5. 返回事务ID

### bool commit_transaction(TransactionId txn_id)

提交事务：

1. 获取互斥锁
2. 查找指定事务
3. 检查事务状态是否为ACTIVE
4. 将事务状态设置为COMMITTED
5. 释放事务持有的所有锁
6. 清理已完成的事务

### bool rollback_transaction(TransactionId txn_id)

回滚事务：

1. 获取互斥锁
2. 查找指定事务
3. 检查事务状态
4. 将事务状态设置为ABORTED
5. 回滚事务操作
6. 释放事务持有的所有锁
7. 清理已完成的事务

### bool create_savepoint(TransactionId txn_id, const std::string &savepoint_name)

创建保存点：

1. 查找指定事务
2. 创建保存点记录
3. 保存当前事务状态

### bool rollback_to_savepoint(TransactionId txn_id, const std::string &savepoint_name)

回滚到保存点：

1. 查找指定事务
2. 查找指定保存点
3. 恢复到保存点时的事务状态
4. 回滚保存点之后的操作

### bool acquire_lock(TransactionId txn_id, const std::string &resource, LockType lock_type, bool wait)

获取锁：

1. 检查是否可以获取锁
2. 如果可以则获取锁
3. 如果不可以且wait为true则等待
4. 如果不可以且wait为false则返回失败

### void release_lock(TransactionId txn_id, const std::string &resource)

释放锁：

1. 查找指定资源的锁
2. 移除事务持有的锁
3. 更新等待图

### bool detect_deadlock(TransactionId txn_id)

检测死锁：

1. 构建等待图
2. 使用拓扑排序检测循环等待
3. 如果存在死锁则返回true

### TransactionState get_transaction_state(TransactionId txn_id) const

获取事务状态：

1. 查找指定事务
2. 返回事务状态

### std::vector<TransactionId> get_active_transactions() const

获取活动事务列表：

1. 遍历事务表
2. 收集状态为ACTIVE的事务ID

### void log_operation(TransactionId txn_id, const LogEntry &entry)

记录事务操作到日志：

1. 将操作记录添加到事务日志中

### TransactionId next_transaction_id()

获取下一个事务ID：

1. 原子地增加并返回事务ID

## 私有方法

### void cleanup_completed_transactions()

清理已完成的事务：

1. 遍历事务表
2. 移除已提交或已回滚的事务

### bool can_acquire_lock(TransactionId txn_id, const std::string &resource, LockType lock_type) const

检查是否可以获取锁（用于死锁检测）：

1. 检查锁兼容性
2. 检查是否会导致死锁

### void release_all_locks(TransactionId txn_id)

释放事务持有的所有锁：

1. 遍历锁表
2. 移除指定事务持有的所有锁

## 成员变量

### std::unordered_map<TransactionId, std::unordered_set<TransactionId>> wait_graph_

等待图结构（用于死锁检测）：

1. 记录事务间的等待关系
2. 用于死锁检测算法

### std::unordered_map<std::string, std::vector<LockEntry>> lock_table_

锁表：

1. 记录资源上的锁信息
2. 每个资源对应一个锁条目列表

### std::unordered_map<TransactionId, Transaction> transactions_

事务表：

1. 记录所有活动事务的信息
2. 事务ID到Transaction对象的映射

### std::atomic<TransactionId> next_txn_id_

事务ID生成器：

1. 原子地生成唯一事务ID

### mutable std::mutex mutex_

互斥锁：

1. 保护事务管理器的并发访问