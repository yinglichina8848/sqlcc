# SQLCC 高级锁管理器设计文档

**版本**: v1.3.8  
**作者**: SQLCC开发团队  
**日期**: 2026年1月25日  
**主题**: 高级锁管理器设计与实现  

## 概述

本文档描述SQLCC数据库系统中高级锁管理器的设计与实现。锁管理器是并发控制的核心组件，负责管理数据库中各种资源的并发访问，确保事务的ACID特性。

## 设计目标

- **并发安全**: 支持多线程安全的资源访问
- **死锁预防**: 实现死锁检测和预防机制
- **锁粒度灵活**: 支持不同粒度的锁（行锁、页锁、表锁等）
- **性能优化**: 最小化锁争用，最大化并发度
- **事务隔离**: 支持多种隔离级别

## 系统架构

```
┌─────────────────┐
│  Transaction    │ ← 事务层
├─────────────────┤
│  LockManager    │ ← 锁管理器
├─────────────────┤
│  ┌───────────┐  │
│  │LockTable  │  │ ← 锁表，记录所有锁请求
│  │           │  │
│  └───────────┘  │
│         │       │
│         ▼       │
│  ┌─────────────┐│
│  │ Deadlock    ││ ← 死锁检测
│  │ Detector    ││
│  └─────────────┘│
└─────────────────┘
```

## 核心组件

### 1. LockManager 类

这是锁管理器的主要实现类，负责管理所有锁请求。

```cpp
class LockManager {
public:
    // 共享锁请求
    bool LockShared(Transaction *txn, const ResourceId &rid);
    
    // 排他锁请求
    bool LockExclusive(Transaction *txn, const ResourceId &rid);
    
    // 升级锁
    bool LockUpgrade(Transaction *txn, const ResourceId &rid);
    
    // 释放锁
    bool Unlock(Transaction *txn, const ResourceId &rid);
    
private:
    std::mutex latch_;  // 保护锁表的互斥锁
    std::unordered_map<ResourceId, LockQueue> lock_table_;  // 锁表
};
```

### 2. LockQueue 类

管理对同一资源的锁请求队列。

```cpp
struct LockQueue {
    std::list<LockRequest> request_queue_;  // 锁请求队列
    std::list<LockRequest>::iterator upgrading_txn_;  // 正在升级的事务
    std::condition_variable cv_;  // 用于等待和通知
};
```

### 3. LockRequest 结构

表示单个锁请求。

```cpp
struct LockRequest {
    TransactionId txn_id_;      // 事务ID
    LockMode lock_mode_;        // 锁模式
    bool granted_;              // 是否已授予
    std::condition_variable *cv_; // 等待条件变量
};
```

## 实现细节

### 锁兼容性矩阵

|       | S | X | IS | IX | SIX |
|-------|---|---|----|----|-----|
| S     | + | - | +  | +  | -   |
| X     | - | - | -  | -  | -   |
| IS    | + | - | +  | +  | -   |
| IX    | + | - | +  | +  | +   |
| SIX   | - | - | -  | +  | -   |

其中：
- S: 共享锁
- X: 排他锁
- IS: 意向共享锁
- IX: 意向排他锁
- SIX: S锁与IX锁的组合

### 死锁检测

使用等待图算法检测死锁：

1. 构建等待图
2. 检查是否存在环
3. 若存在环，选择牺牲者事务进行回滚

## API 接口

### 请求共享锁
```cpp
LockManager::LockShared(Transaction *txn, const ResourceId &rid)
```

### 请求排他锁
```cpp
LockManager::LockExclusive(Transaction *txn, const ResourceId &rid)
```

### 释放锁
```cpp
LockManager::Unlock(Transaction *txn, const ResourceId &rid)
```

## 性能特征

- **锁请求时间**: O(1) 平均情况
- **锁释放时间**: O(1) 平均情况
- **死锁检测**: O(T + R) 其中T为事务数，R为资源数

## 与事务管理器的集成

锁管理器与事务管理器紧密集成：

1. **事务生命周期**: 事务结束时自动释放其持有的锁
2. **隔离级别**: 根据事务的隔离级别调整锁策略
3. **两阶段锁定**: 实现严格的两阶段锁定协议

## 安全性考虑

- **死锁预防**: 使用等待图算法检测和解决死锁
- **锁升级**: 防止锁升级过程中的竞态条件
- **异常安全**: 保证异常情况下的锁正确释放

## 未来发展方向

1. **分布式锁**: 支持跨节点的分布式锁
2. **乐观并发控制**: 实现乐观并发控制机制
3. **自适应锁策略**: 根据工作负载自适应调整锁策略

## 参考资料

- 《数据库系统概念》- Silberschatz, Korth, Sudarshan
- 《数据库系统实现》- Garcia-Molina, Ullman, Widom