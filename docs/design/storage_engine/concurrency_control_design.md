# SQLCC 并发控制设计文档

**版本**: v1.3.8  
**作者**: SQLCC开发团队  
**日期**: 2026年1月25日  
**主题**: 并发控制设计与实现  

## 概述

本文档描述SQLCC数据库系统中并发控制的设计与实现。并发控制是数据库系统的关键组件，负责管理多个事务对共享数据的并发访问，确保数据一致性和事务隔离性。

## 设计目标

- **ACID特性**: 保证事务的原子性、一致性、隔离性和持久性
- **高并发**: 支持大量并发事务执行
- **死锁处理**: 实现死锁检测和解决机制
- **性能优化**: 最小化并发控制开销
- **隔离级别**: 支持多种事务隔离级别

## 系统架构

```
┌─────────────────┐
│   Transaction   │ ← 事务管理层
├─────────────────┤
│ Concurrency     │ ← 并发控制器
│ Controller      │
├─────────────────┤
│  ┌───────────┐  │
│  │Scheduler  │  │ ← 事务调度器
│  │           │  │
│  └───────────┘  │
│         │       │
│         ▼       │
│  ┌─────────────┐│
│  │ LockBased   ││ ← 基于锁的控制
│  │ CC          ││
│  └─────────────┘│
│         │       │
│         ▼       │
│  ┌─────────────┐│
│  │ MVCC        ││ ← 多版本并发控制
│  │ Manager     ││
│  └─────────────┘│
└─────────────────┘
```

## 核心组件

### 1. ConcurrencyController 类

这是并发控制的主要实现类，负责管理事务的并发执行。

```cpp
class ConcurrencyController {
public:
    // 开始事务
    bool BeginTransaction(Transaction *txn);
    
    // 提交事务
    bool CommitTransaction(Transaction *txn);
    
    // 回滚事务
    bool AbortTransaction(Transaction *txn);
    
    // 读取数据项
    bool ReadData(Transaction *txn, const RID &rid, Value *value);
    
    // 写入数据项
    bool WriteData(Transaction *txn, const RID &rid, const Value &value);
    
    // 执行加锁操作
    bool Lock(Transaction *txn, LockMode lock_mode, const RID &rid);
    
    // 执行解锁操作
    bool Unlock(Transaction *txn, const RID &rid);
    
private:
    std::unique_ptr<LockManager> lock_manager_;  // 锁管理器
    std::unique_ptr<TimestampManager> ts_manager_;  // 时间戳管理器
    std::unique_ptr<VersionManager> version_manager_;  // 版本管理器
};
```

### 2. LockBasedCC 类

基于锁的并发控制实现。

```cpp
class LockBasedCC {
public:
    // 请求共享锁
    bool AcquireSharedLock(txn_id_t txn_id, const RID &rid);
    
    // 请求排他锁
    bool AcquireExclusiveLock(txn_id_t txn_id, const RID &rid);
    
    // 释放锁
    bool ReleaseLock(txn_id_t txn_id, const RID &rid);
    
    // 检测死锁
    bool DetectDeadlock(txn_id_t txn_id);
    
    // 解决死锁
    void ResolveDeadlock(txn_id_t txn_id);
    
private:
    std::unordered_map<RID, LockQueue> lock_table_;  // 锁表
    std::vector<std::vector<bool>> waits_for_graph_;  // 等待图
};
```

### 3. MVCCManager 类

多版本并发控制实现。

```cpp
class MVCCManager {
public:
    // 读取适当版本的数据
    bool ReadVersion(Transaction *txn, const RID &rid, Value *value);
    
    // 创建新版本
    bool CreateVersion(Transaction *txn, const RID &rid, const Value &value);
    
    // 清理旧版本
    void GarbageCollect();
    
private:
    std::unordered_map<RID, std::list<Version>> version_store_;  // 版本存储
    std::mutex version_mutex_;  // 版本访问保护
};
```

## 实现细节

### 隔离级别

支持以下隔离级别：

1. **读未提交 (Read Uncommitted)**:
   - 最低隔离级别
   - 事务可以读取未提交的数据

2. **读已提交 (Read Committed)**:
   - 事务只能读取已提交的数据
   - 防止脏读

3. **可重复读 (Repeatable Read)**:
   - 保证在事务期间多次读取同一数据得到相同结果
   - 防止脏读和不可重复读

4. **串行化 (Serializable)**:
   - 最高隔离级别
   - 完全串行执行事务

### 死锁处理策略

1. **等待图算法**:
   - 构建事务等待图
   - 检测图中的环
   - 选择牺牲者回滚

2. **超时策略**:
   - 设置锁等待超时时间
   - 超时则回滚事务

### 两阶段锁定协议

1. **扩展阶段**: 只允许获取锁，不允许释放锁
2. **收缩阶段**: 只允许释放锁，不允许获取锁

## API 接口

### 开始事务
```cpp
ConcurrencyController::BeginTransaction(Transaction *txn)
```

### 读取数据
```cpp
ConcurrencyController::ReadData(Transaction *txn, const RID &rid, Value *value)
```

### 写入数据
```cpp
ConcurrencyController::WriteData(Transaction *txn, const RID &rid, const Value &value)
```

## 性能特征

- **并发度**: 支持数千并发事务
- **死锁检测**: O(T²) 时间复杂度，T为活跃事务数
- **锁开销**: 平均每个操作增加约10-20微秒

## 与事务管理器的集成

并发控制与事务管理器紧密集成：

1. **事务生命周期**: 根据事务状态调整并发控制策略
2. **隔离级别**: 根据事务隔离级别执行相应的控制策略
3. **恢复机制**: 与事务恢复机制协调工作

## 安全性考虑

- **数据一致性**: 保证并发事务不会破坏数据一致性
- **死锁处理**: 防止系统因死锁而挂起
- **资源泄露**: 确保事务结束后释放所有资源

## 未来发展方向

1. **乐观并发控制**: 实现乐观并发控制机制
2. **分布式并发控制**: 支持跨节点的并发控制
3. **自适应控制**: 根据工作负载自适应调整控制策略

## 参考资料

- 《数据库系统概念》- Silberschatz, Korth, Sudarshan
- 《数据库系统实现》- Garcia-Molina, Ullman, Widom
- 《事务处理概念与技术》- Gray, Reuter