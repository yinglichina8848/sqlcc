# Transaction Management Flow Design Document

**Document Version**: 1.0  
**Last Updated**: 2026-01-31  
**Author**: Gemini AI Agent  
**Related Files**: `src/transaction_manager/transaction_manager.h`, `src/transaction_manager/transaction_manager.cpp`, `src/transaction/savepoint_manager.h`

---

## 1. WHY: 为什么要进行事务管理？

在任何多用户、持久化存储数据的系统中，**事务 (Transaction)** 都是确保数据完整性、一致性和系统可靠性的核心机制。尤其在数据库管理系统 (DBMS) 中，没有事务管理，数据很快就会变得不可靠和不一致。事务管理旨在解决以下关键挑战：

1.  **数据完整性 (Data Integrity)**：确保数据的准确性和一致性，防止因并发操作或系统故障导致数据损坏。
2.  **并发控制 (Concurrency Control)**：允许多个用户或应用程序同时访问和修改数据，而不会相互干扰，保持数据的隔离性。
3.  **故障恢复 (Crash Recovery)**：在系统发生崩溃或错误时，能够将数据库恢复到一致的状态，确保已提交的数据不会丢失，未提交的数据不会影响数据库。

这些需求被总结为数据库事务的 **ACID 特性**：

*   **A - 原子性 (Atomicity)**：事务是一个不可分割的工作单位。事务中的所有操作要么全部成功，要么全部失败。
*   **C - 一致性 (Consistency)**：事务必须使数据库从一个一致的状态转换到另一个一致的状态。
*   **I - 隔离性 (Isolation)**：并发执行的事务是相互隔离的，一个事务的执行不应影响其他事务的执行，就好像它们是串行执行的一样。
*   **D - 持久性 (Durability)**：一旦事务提交，其对数据库的改变就是永久的，即使系统发生故障也不会丢失。

本事务管理模块的目标是提供一个框架，以实现这些 ACID 特性，确保 SQLCC 数据库在复杂环境下的稳定和可靠。

---

## 2. WHAT: 事务管理器的核心功能和组件？

事务管理器作为数据库的核心组件，负责协调所有事务的生命周期、并发控制和故障恢复（部分）。

### 2.1. 核心组件

1.  **`Transaction` (事务对象)**：
    *   **职责**: 封装单个事务的所有状态和元数据。
    *   **状态**: 维护事务的生命周期状态（`ACTIVE`, `COMMITTED`, `ABORTED`, `ROLLING_BACK` 等）。
    *   **元数据**: 包含事务 ID、隔离级别、开始时间、超时时间、操作计数等。
    *   **`undo_log`**: 记录事务执行过程中所有修改操作的反向操作，用于事务回滚。

2.  **`TransactionManager` (事务管理器)**：
    *   **职责**: 整个事务管理框架的中心协调者。
    *   **事务生命周期**: 提供 `begin_transaction()`, `commit_transaction()`, `rollback_transaction()` 等方法，管理事务的创建、提交和回滚。
    *   **锁管理 (简化)**: 包含一个 `lock_table_` (`std::unordered_map<std::string, std::vector<LockEntry>>`)，用于跟踪资源上的锁。提供 `acquire_lock()` 和 `release_lock()` 方法。
    *   **死锁检测 (简化)**: 维护一个 `wait_graph_` (`std::unordered_map<TransactionId, std::unordered_set<TransactionId>>`)，并提供 `detect_deadlock()` 方法，基于等待图进行周期性死锁检测。
    *   **保存点 (Savepoints)**: 提供 `create_savepoint()` 和 `rollback_to_savepoint()`，允许事务进行部分回滚。
    *   **嵌套事务 (Nested Transactions)**: 支持 `begin_nested_transaction()` 和 `commit_nested_transaction()`，允许在事务内部创建子事务。
    *   **超时处理**: `check_and_handle_timeouts()` 方法用于检测并强制回滚超时的活跃事务。
    *   **隔离级别**: 存储并利用事务的 `IsolationLevel`，并在 `check_isolation_constraints()` 中进行简化检查。

3.  **`LockEntry` (锁条目)**：
    *   **职责**: 记录一个资源上的锁信息。
    *   **包含信息**: 持有锁的事务 ID、锁类型（共享/排他）、被锁定资源以及锁的获取时间。

4.  **`LogEntry` (日志条目)**：
    *   **职责**: 记录事务执行的单个操作，用于 `undo_log`。
    *   **包含信息**: 操作类型、表名、旧值、新值等，用于支持回滚操作。

5.  **`SavepointManager` (保存点管理器 - 概念性)**：
    *   **职责**: 负责管理事务内部创建的保存点，存储保存点的名称和对应事务 `undo_log` 的位置。

### 2.2. 事务状态 (`TransactionState`)

*   **`ACTIVE`**: 事务正在进行中。
*   **`COMMITTED`**: 事务已成功提交，所有更改已永久保存。
*   **`ABORTED`**: 事务已回滚，所有更改已被撤销。
*   **`ROLLING_BACK`**: 事务正在执行回滚操作。

---

## 3. HOW: 事务管理的工作流程和实现细节？

### 3.1. 事务生命周期

#### 3.1.1. 开启事务 (`begin_transaction`)

1.  **管理器锁定**: 事务管理器获取内部互斥锁，确保线程安全。
2.  **生成 ID**: 分配一个全局唯一且递增的 `TransactionId`。
3.  **创建事务对象**: 根据 `TransactionId` 和 `IsolationLevel` 创建 `Transaction` 对象，初始状态为 `ACTIVE`。
4.  **存储**: 将新创建的 `Transaction` 对象存储在 `TransactionManager` 的内部映射 (`transactions_`) 中。
5.  **返回**: 返回新的 `TransactionId` 给调用者。

#### 3.1.2. 提交事务 (`commit_transaction`)

1.  **管理器锁定**: 事务管理器获取内部互斥锁。
2.  **查找与校验**: 根据 `TransactionId` 查找事务，并验证其状态必须是 `ACTIVE`。
3.  **关键提交点 (Write-Ahead Logging)**: 在一个实际的数据库系统中，此时会执行关键的持久化操作：确保所有事务日志（包括修改数据前的旧值和修改数据后的新值）已经成功写入持久化存储（硬盘）。这通常通过 **预写日志 (Write-Ahead Logging, WAL)** 机制实现。
4.  **状态更新**: 将事务状态原子地更新为 `COMMITTED`，并记录结束时间。
5.  **释放锁**: 调用 `release_all_locks_internal()` 释放事务持有的所有锁。这是 **两阶段锁 (Two-Phase Locking, 2PL)** 协议的“收缩阶段”。
6.  **移除死锁图**: 从死锁检测的等待图中移除该事务。

#### 3.1.3. 回滚事务 (`rollback_transaction`)

1.  **管理器锁定**: 事务管理器获取内部互斥锁。
2.  **查找与校验**: 根据 `TransactionId` 查找事务，并验证其状态必须是 `ACTIVE`。
3.  **状态更新**: 将事务状态更新为 `ROLLING_BACK`。
4.  **执行 UNDO**: 遍历事务的 `undo_log`，对其中记录的每个操作执行反向操作，将数据库的状态恢复到事务开始之前的状态。这确保了事务的 **原子性**。
5.  **状态更新**: 所有撤销操作完成后，将事务状态更新为 `ABORTED`，并记录结束时间。
6.  **释放锁**: 调用 `release_all_locks_internal()` 释放事务持有的所有锁。
7.  **移除死锁图**: 从死锁检测的等待图中移除该事务。

### 3.2. 并发控制 (简化实现)

本实现采用简化的 **基于锁的并发控制** 机制：

*   **`lock_table_`**: 一个 `std::unordered_map`，键是资源 ID (例如，表名、页 ID)，值是该资源上当前持有的 `LockEntry` 列表。
*   **`acquire_lock(txn_id, resource, lock_type)`**:
    *   检查事务是否存在且活跃。
    *   检查当前事务是否已持有该资源的锁，并处理锁升级（SHARED -> EXCLUSIVE）逻辑。
    *   检查锁请求与该资源上其他事务持有的锁的兼容性（例如，排他锁与任何其他锁都不兼容；共享锁与排他锁不兼容）。
    *   如果兼容，则创建 `LockEntry` 并将其添加到 `lock_table_`。
*   **`release_lock(txn_id, resource)`**: 移除特定事务在特定资源上的锁。
*   **`release_all_locks_internal(txn_id)`**: 在事务提交或回滚时，遍历 `lock_table_`，释放 `txn_id` 持有的所有锁。

### 3.3. 死锁检测 (简化实现)

*   **等待图 (`wait_graph_`)**: 一个 `std::unordered_map<TransactionId, std::unordered_set<TransactionId>>`，表示事务之间的等待关系。`wait_graph_[T1] = {T2, T3}` 表示事务 T1 正在等待事务 T2 和 T3 释放资源。
*   **`detect_deadlock(txn_id)`**:
    *   使用 **深度优先搜索 (DFS)** 算法遍历 `wait_graph_`。
    *   如果 DFS 过程中发现一个节点（事务）再次被访问（即 `recursion_stack` 中已存在），则说明存在一个 **环**，即发生了死锁。
    *   该函数可以由一个后台线程周期性调用，以检测死锁并触发死锁解决策略（例如，选择一个“牺牲者”事务进行回滚）。

### 3.4. 保存点与嵌套事务 (简化实现)

*   **保存点**: 允许事务在执行过程中设置“标记点”。`create_savepoint()` 记录当前 `undo_log` 的大小，`rollback_to_savepoint()` 则将 `undo_log` 截断并执行部分回滚到该标记点。
*   **嵌套事务**: 允许在现有事务内部创建子事务。本实现简化了嵌套事务的提交逻辑，将其更改立即对父事务可见，但仍需要父事务提交才能使更改永久化。

### 3.5. 简化的类图

```mermaid
classDiagram
    class Transaction {
        +txn_id: TransactionId
        +isolation_level: IsolationLevel
        +state: TransactionState
        +start_time: chrono::system_clock::time_point
        +undo_log: list<LogEntry>
        +is_nested: bool
        +parent_txn_id: TransactionId
        +create_nested(...): Transaction
        +is_timeout(): bool
        +update_activity(): void
        +get_running_time(): chrono::milliseconds
    }

    class TransactionManager {
        -next_txn_id_: atomic<TransactionId>
        -transactions_: unordered_map<TransactionId, Transaction>
        -lock_table_: unordered_map<string, list<LockEntry>>
        -wait_graph_: unordered_map<TransactionId, unordered_set<TransactionId>>
        +begin_transaction(...): TransactionId
        +commit_transaction(...): bool
        +rollback_transaction(...): bool
        +create_savepoint(...): bool
        +rollback_to_savepoint(...): bool
        +acquire_lock(...): bool
        +release_lock(...): void
        +detect_deadlock(...): bool
        +begin_nested_transaction(...): TransactionId
        +commit_nested_transaction(...): bool
        +check_and_handle_timeouts(): size_t
        -release_all_locks_internal(...): void
    }

    class LockEntry {
        +txn_id: TransactionId
        +type: LockType
        +resource: string
        +acquired_time: chrono::system_clock::time_point
    }

    class LogEntry {
        +operation: string
        +table_name: string
        // ... old_value, new_value, etc.
    }

    enum TransactionState {
        ACTIVE
        COMMITTED
        ABORTED
        ROLLING_BACK
    }
    
    enum IsolationLevel {
        READ_UNCOMMITTED
        READ_COMMITTED
        REPEATABLE_READ
        SERIALIZABLE
    }

    TransactionManager "1" *-- "N" Transaction : manages
    TransactionManager "1" *-- "N" LockEntry : uses_via_table
    Transaction "1" *-- "N" LogEntry : owns_undo_log
    TransactionManager "1" o-- "1" SavepointManager : uses
    TransactionManager "1" *-- "N" NestedTransaction : tracks_nested
```

---

## 4. 总结

事务管理是构建可靠数据库系统的基石。本设计文档概述了 SQLCC 数据库中事务管理器的核心功能和工作流程，涵盖了事务的生命周期、简化的并发控制、死锁检测和保存点等关键概念。通过这些机制，数据库能够保证 ACID 特性，即使在并发和故障场景下也能维护数据的一致性和完整性。对于学生而言，理解事务管理是理解现代数据库系统如何运行的关键一步。未来的工作可以包括实现更复杂的两阶段锁协议、多版本并发控制 (MVCC)、分布式事务支持以及与日志恢复系统 (ARIES 等) 的深度集成。
