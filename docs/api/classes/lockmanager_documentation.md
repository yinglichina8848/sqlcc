# AdvancedLockManager 类文档

## 类概述

`AdvancedLockManager` 是 SQLCC 的企业级锁管理器，提供完整的并发控制功能，包括同步/异步锁操作、死锁检测与解决、锁升级/降级等。

## WHY: 为什么需要高级锁管理器？

**设计动机**：数据库系统作为多用户并发访问的核心组件，锁管理直接影响系统的并发性能和数据一致性：

1. **并发控制复杂性**：现代数据库需要支持高并发的多事务访问
2. **死锁预防**：复杂的业务逻辑可能导致死锁，需要自动检测和解决
3. **锁粒度管理**：不同场景需要不同粒度的锁（行级、页级、表级）
4. **锁升级策略**：锁的动态升级和降级以优化性能
5. **性能监控**：需要详细的锁统计信息进行性能调优

**核心价值**：
- 提高并发性能：减少锁竞争，提高系统吞吐量
- 确保数据一致性：防止并发访问导致的数据不一致
- 自动死锁解决：检测并自动解决死锁情况
- 灵活的锁策略：支持多种锁模式和升级策略
- 性能监控：提供详细的锁统计和性能指标

## WHAT: 核心功能

### 锁生命周期管理
| 方法 | 功能描述 |
|------|----------|
| `LockShared()` | 获取共享锁（读锁） |
| `LockExclusive()` | 获取排他锁（写锁） |
| `Unlock()` | 释放锁 |
| `LockUpgrade()` | 锁升级（共享→排他） |
| `LockDowngrade()` | 锁降级（排他→共享） |

### 死锁处理
| 方法 | 功能描述 |
|------|----------|
| `DetectDeadlocks()` | 检测死锁 |
| `ResolveDeadlock()` | 解决死锁 |
| `SetDeadlockResolutionPolicy()` | 设置死锁解决策略 |
| `GetDeadlockStatistics()` | 获取死锁统计 |

### 异步操作
| 方法 | 功能描述 |
|------|----------|
| `LockAsync()` | 异步获取锁 |
| `CancelAsyncLock()` | 取消异步锁请求 |
| `SetAsyncCallback()` | 设置异步回调 |

### 统计监控
| 方法 | 功能描述 |
|------|----------|
| `GetLockStatistics()` | 获取锁统计信息 |
| `GetWaitGraph()` | 获取等待图 |
| `GetLockContention()` | 获取锁竞争率 |

## 锁类型

```cpp
enum class LockType {
  NONE,           // 无锁
  SHARED,         // 共享锁（读锁）
  EXCLUSIVE,      // 排他锁（写锁）
  INTENTION_SHARED,       // 意向共享锁
  INTENTION_EXCLUSIVE,    // 意向排他锁
  SHARED_INTENTION_EXCLUSIVE  // 共享意向排他锁
};
```

### 锁兼容性矩阵

| 请求锁 \ 持有锁 | 无 | 共享 | 排他 |
|---------------|-----|------|------|
| 共享 | ✓ | ✓ | ✗ |
| 排他 | ✓ | ✗ | ✗ |

## HOW: 实现机制

### 锁管理核心架构

- **锁表管理**：`page_id` 到锁状态的映射表
- **等待队列**：每个页面的锁请求等待队列
- **锁持有信息**：记录锁的持有者和模式信息
- **兼容性检查**：锁模式间的兼容性判断矩阵

### 死锁检测机制

1. **等待图构建**：事务间的等待关系图
2. **环检测算法**：深度优先搜索检测死锁环
3. **死锁解决策略**：可配置的死锁受害者选择策略
4. **自动解除**：检测到死锁后自动选择受害者并回滚

### 锁升级/降级系统

- **升级时机**：基于锁竞争情况的智能升级
- **降级策略**：在合适时机降低锁粒度
- **性能优化**：平衡锁开销和并发性能

## 使用示例

```cpp
#include "storage_engine/advanced_lock_manager.h"

// 创建锁管理器
AdvancedLockManager lock_manager;

// 获取共享锁（读操作）
if (lock_manager.LockShared(txn_id, page_id)) {
    // 读取数据
    auto data = storage_engine.FetchPage(page_id);
    // 释放锁
    lock_manager.Unlock(txn_id, page_id);
}

// 获取排他锁（写操作）
if (lock_manager.LockExclusive(txn_id, page_id)) {
    // 写入数据
    page->SetData(0, "new data", 8);
    storage_engine.UnpinPage(page_id, true);
    lock_manager.Unlock(txn_id, page_id);
}

// 检测死锁
auto deadlocks = lock_manager.DetectDeadlocks();
if (!deadlocks.empty()) {
    // 处理死锁
    lock_manager.ResolveDeadlock(deadlocks[0]);
}

// 获取统计信息
auto stats = lock_manager.GetLockStatistics();
std::cout << "锁竞争率: " << stats.contention_rate << "%" << std::endl;
```

## 设计模式

**策略模式 + 观察者模式**：
- 锁管理策略：不同场景下的锁管理策略
- 死锁解决策略：可插拔的死锁解决算法
- 事件通知：锁状态变化的观察者通知机制

## 性能优化建议

1. **锁粒度选择**：根据访问模式选择合适的锁粒度
2. **锁持有时间**：尽量减少锁持有时间
3. **死锁避免**：按固定顺序获取锁
4. **监控告警**：设置锁竞争率告警阈值

## 版本信息

- **版本**: v1.3.9
- **最后更新**: 2026-01-31
- **C++标准**: C++20
- **编译器**: Clang 18+