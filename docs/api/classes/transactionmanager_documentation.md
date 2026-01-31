# TransactionManager 类文档

## 类概述

`TransactionManager` 是 SQLCC 数据库系统的 **ACID 属性实现核心**，负责协调所有并发事务，确保数据的一致性和完整性。

## WHY: 为什么需要事务管理器？

**设计动机**：事务是数据库系统的核心抽象，提供数据操作的原子性和一致性保证：

1. **原子性（Atomicity）**：事务要么全部成功，要么全部失败
2. **一致性（Consistency）**：事务将数据库从一个一致状态转换到另一个
3. **隔离性（Isolation）**：事务间的相互隔离，避免干扰
4. **持久性（Durability）**：已提交事务的结果永久保存

**核心价值**：
- 事务生命周期管理（创建、提交、回滚）
- 并发控制（两阶段锁协议 + 死锁检测）
- 隔离级别支持（多种 SQL 隔离级别）
- 持久性保证（通过 WAL 机制）
- 保存点支持（嵌套事务的回滚控制）

## ACID 属性实现策略

### 1. 原子性（Atomicity）
- **机制**：undo 日志记录事务的所有修改
- **提交时**：释放所有锁，标记事务完成
- **回滚时**：根据 undo 日志逆序撤销所有修改
- **保证**：要么全部成功，要么全部失败

### 2. 一致性（Consistency）
- **机制**：锁协议确保事务串行化执行
- **约束**：完整性约束检查和业务规则验证
- **隔离**：不同隔离级别的一致性保证
- **保证**：事务将数据库从一个一致状态转换到另一个

### 3. 隔离性（Isolation）
- **机制**：锁和多版本并发控制（MVCC）
- **级别**：READ UNCOMMITTED 到 SERIALIZABLE
- **冲突**：读-写、写-读、写-写冲突的处理
- **保证**：事务间的相互隔离，避免干扰

### 4. 持久性（Durability）
- **机制**：预写日志（WAL）和检查点
- **策略**：日志先行，数据后写
- **恢复**：系统故障后的自动恢复
- **保证**：已提交事务的结果永久保存

## WHAT: 核心功能

### 事务生命周期
| 方法 | 功能描述 |
|------|----------|
| `begin_transaction()` | 开始新事务，返回事务ID |
| `commit_transaction()` | 提交事务 |
| `rollback_transaction()` | 回滚事务 |
| `begin_nested_transaction()` | 开始嵌套事务 |

### 锁管理
| 方法 | 功能描述 |
|------|----------|
| `acquire_lock()` | 获取锁 |
| `release_lock()` | 释放锁 |
| `detect_deadlock()` | 检测死锁 |

### 保存点管理
| 方法 | 功能描述 |
|------|----------|
| `create_savepoint()` | 创建保存点 |
| `rollback_to_savepoint()` | 回滚到保存点 |

### 隔离级别
| 方法 | 功能描述 |
|------|----------|
| `set_transaction_isolation_level()` | 设置隔离级别 |
| `get_transaction_isolation_level()` | 获取隔离级别 |
| `check_isolation_constraints()` | 检查隔离级别约束 |

### 监控和管理
| 方法 | 功能描述 |
|------|----------|
| `get_transaction_state()` | 获取事务状态 |
| `get_active_transactions()` | 获取活动事务列表 |
| `get_transaction_stats()` | 获取统计信息 |
| `check_and_handle_timeouts()` | 检查和处理超时事务 |

## HOW: 实现机制

### 并发控制策略

**严格两阶段锁（Strict 2PL）**：
- 事务获取所有锁后才能释放锁
- 防止脏读和不可重复读

**死锁检测**：
- 使用等待图算法检测循环依赖
- 选择合适的事务进行回滚

**锁升级**：
- 从共享锁升级到排他锁的策略
- 减少锁数量，提高性能

### 事务状态机

```
        ┌─────────┐
        │  ACTIVE  │ ←─────────────────────┐
        └────┬────┘                       │
             │                            │
    ┌────────┴────────┐                   │
    │                 │                   │
    ▼                 ▼                   │
COMMITTED ◄─── ABORTED ◄── ROLLING_BACK ──┘
   (完成)          (回滚完成)
```

### 故障恢复机制

- **检查点**：定期创建系统一致性快照
- **重做日志**：记录事务提交后的数据修改
- **撤销日志**：记录事务执行中的数据修改
- **ARIES 协议**：先进的数据库恢复算法

## 事务状态枚举

```cpp
enum class TransactionState {
  ACTIVE,        // 事务活跃状态，可以执行数据操作
  COMMITTED,     // 事务已提交，修改永久保存
  ABORTED,       // 事务已中止，修改已回滚
  ROLLING_BACK   // 事务正在回滚过程中
};
```

## 隔离级别

```cpp
enum class IsolationLevel {
  READ_UNCOMMITTED,  // 最低隔离级别，可能脏读
  READ_COMMITTED,    // 只能读取已提交数据
  REPEATABLE_READ,   // 可重复读
  SERIALIZABLE       // 最高隔离级别，完全串行化
};
```

## 使用示例

```cpp
#include "transaction_manager/transaction_manager.h"

// 开始事务
auto txn_id = transaction_manager.begin_transaction(
    IsolationLevel::READ_COMMITTED);

try {
    // 执行操作
    db_manager->executeQuery("INSERT INTO users VALUES (1, 'Alice')");
    db_manager->executeQuery("UPDATE users SET name = 'Bob' WHERE id = 1");

    // 创建保存点
    transaction_manager.create_savepoint(txn_id, "before_update");

    // 更多操作
    db_manager->executeQuery("DELETE FROM users WHERE id = 1");

    // 提交事务
    transaction_manager.commit_transaction(txn_id);

} catch (const std::exception& e) {
    // 回滚事务
    transaction_manager.rollback_transaction(txn_id);
    throw;
}

// 检查死锁
if (transaction_manager.detect_deadlock(txn_id)) {
    // 处理死锁
}

// 获取事务统计
auto stats = transaction_manager.get_transaction_stats();
std::cout << "活动事务数: " << stats.active_transactions << std::endl;
```

## 性能优化建议

1. **锁粒度**：行级锁 vs 表级锁的选择
2. **锁兼容性**：合理使用共享锁和排他锁
3. **超时设置**：为长时间运行的事务设置超时
4. **监控死锁**：监控系统的死锁发生频率

## 版本信息

- **版本**: v1.3.9
- **最后更新**: 2026-01-31
- **C++标准**: C++20
- **编译器**: Clang 18+
- **SQL标准**: SQL-92 完整事务支持