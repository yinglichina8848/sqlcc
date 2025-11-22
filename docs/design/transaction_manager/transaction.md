# Transaction类详细设计

## 概述

Transaction类表示数据库中的单个事务，包含事务的所有状态信息和操作记录。它跟踪事务的ID、隔离级别、状态、开始时间以及执行的所有操作。

## 类定义

```cpp
struct Transaction {
    TransactionId txn_id;
    IsolationLevel isolation_level;
    TransactionState state;
    std::chrono::system_clock::time_point start_time;
    std::vector<LogEntry> log_entries;
    std::unordered_map<std::string, Savepoint> savepoints;
    
    Transaction(TransactionId id, IsolationLevel level);
};
```

## 构造函数

### Transaction(TransactionId id, IsolationLevel level)

构造函数：

1. 初始化事务ID
2. 设置隔离级别
3. 设置事务状态为ACTIVE
4. 记录开始时间
5. 初始化日志条目和保存点列表

## 成员变量

### TransactionId txn_id

事务标识符：

1. 唯一标识一个事务
2. 由TransactionManager分配

### IsolationLevel isolation_level

事务隔离级别：

1. 控制事务的可见性和并发行为
2. 支持READ_UNCOMMITTED、READ_COMMITTED、REPEATABLE_READ和SERIALIZABLE

### TransactionState state

事务状态：

1. ACTIVE：事务正在进行中
2. COMMITTED：事务已提交
3. ABORTED：事务已回滚
4. ROLLING_BACK：事务正在回滚

### std::chrono::system_clock::time_point start_time

事务开始时间：

1. 记录事务开始的系统时间
2. 用于性能监控和超时检测

### std::vector<LogEntry> log_entries

日志条目列表：

1. 记录事务执行的所有操作
2. 用于事务回滚和恢复

### std::unordered_map<std::string, Savepoint> savepoints

保存点映射表：

1. 保存事务内的保存点信息
2. 支持部分回滚操作

## 隔离级别

### IsolationLevel枚举

```cpp
enum class IsolationLevel {
    READ_UNCOMMITTED,  // 读未提交
    READ_COMMITTED,    // 读已提交
    REPEATABLE_READ,   // 可重复读
    SERIALIZABLE       // 可串行化
};
```

### READ_UNCOMMITTED

最低的隔离级别：

1. 允许读取未提交的数据
2. 可能出现脏读、不可重复读和幻读

### READ_COMMITTED

读已提交：

1. 只能读取已提交的数据
2. 避免脏读，但可能出现不可重复读和幻读

### REPEATABLE_READ

可重复读：

1. 保证在同一事务中多次读取相同数据的结果一致
2. 避免脏读和不可重复读，但可能出现幻读

### SERIALIZABLE

可串行化：

1. 最高的隔离级别
2. 完全避免并发问题，但性能最低

## 事务状态

### TransactionState枚举

```cpp
enum class TransactionState {
    ACTIVE,        // 活动状态
    COMMITTED,     // 已提交
    ABORTED,       // 已回滚
    ROLLING_BACK   // 正在回滚
};
```

### ACTIVE

活动状态：

1. 事务正在进行中
2. 可以执行读写操作

### COMMITTED

已提交：

1. 事务已成功提交
2. 所有更改已持久化

### ABORTED

已回滚：

1. 事务已回滚
2. 所有更改已撤销

### ROLLING_BACK

正在回滚：

1. 事务正在执行回滚操作
2. 不能执行新的操作