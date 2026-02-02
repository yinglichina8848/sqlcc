# WAL (Write-Ahead Log) Manager Design Document

**Document Version**: 1.0  
**Last Updated**: 2026-01-31  
**Author**: Gemini AI Agent  
**Related Files**: `src/transaction_manager/wal_manager.cpp`, `src/transaction_manager/transaction_manager.h`

---

## 1. WHY: 为什么要设计 WAL 管理器？

在数据库管理系统 (DBMS) 中，**持久性 (Durability)** 是 ACID 特性之一，它保证一旦事务提交，其对数据库的修改就是永久的，即使系统发生崩溃或电源故障，这些修改也不会丢失。实现持久性和确保数据库在故障后能恢复到一致状态（**崩溃恢复**）是任何生产级数据库的关键挑战。

传统的“原地更新”策略（直接修改磁盘上的数据页）在系统崩溃时面临巨大风险：
1.  **数据丢失**: 已提交事务的修改可能只存在于内存的缓冲池中，尚未“刷入”（Flush）到磁盘，系统崩溃会导致这些数据永久丢失。
2.  **数据不一致**: 未提交事务的部分修改可能已经写入磁盘，而其他部分修改则未写入，导致数据库处于一个既非崩溃前一致状态、也非崩溃后某个一致状态的中间态，即数据不一致或损坏。

**预写日志 (Write-Ahead Logging, WAL)** 协议正是为了解决这些问题而设计的一种核心机制。WAL 的核心思想是：

> **任何数据修改都必须先将对应的日志记录写入并持久化到稳定存储（磁盘），然后才能将数据页本身写入磁盘。**

这意味着每次对数据库的修改，都会在真正修改数据之前，先以日志记录的形式记录下来。如果系统崩溃，可以通过重放（Redo）日志来恢复已提交的更改，或者撤销（Undo）未提交的更改，从而确保数据持久性和数据库的一致性。

本 WAL 管理器旨在提供一个高效、可靠的日志记录和崩溃恢复机制，确保 SQLCC 数据库在面对各种故障时的数据安全和一致性。

---

## 2. WHAT: WAL 管理器的核心功能和组件？

WAL 管理器 `WALManager` 是 SQLCC 数据库中实现 WAL 协议的核心组件。它通过一系列精心设计的机制，确保日志的记录、持久化、以及在需要时进行恢复。

### 2.1. 核心组件

1.  **`LogRecordType` (日志记录类型枚举)**：
    *   **职责**: 定义了所有可能被 WAL 记录的事件类型。这些类型覆盖了事务的生命周期（`BEGIN`, `COMMIT`, `ABORT`）和数据操作（`UPDATE`, `INSERT`, `DELETE`），以及内部恢复机制所需的特定类型（`COMPENSATE`）。
    *   **优点**: 提供清晰、枚举化的方式来区分和处理不同类型的日志事件。

2.  **`LogRecord` (日志记录结构体)**：
    *   **职责**: 表示单个数据库操作（或事务控制事件）的原子日志条目。
    *   **关键字段**:
        *   `lsn` (Log Sequence Number): 唯一且递增的日志序列号，标识日志记录的顺序。
        *   `txn_id`: 产生该日志记录的事务 ID。
        *   `type`: `LogRecordType`，指示日志记录的类型。
        *   `key`: 涉及的数据键，通常是受影响的数据项的标识符（例如，表名 + 主键）。
        *   `timestamp`: 日志记录生成的时间戳。
        *   **`TODO(#WAL-001)`**: 缺少 `old_value` 和 `new_value` 字段。这两个字段对于实现完整的 ARIES (Algorithm for Recovery and Isolation Exploiting Semantics) 或其他高级恢复协议中的 Redo/Undo 操作至关重要。`old_value` 用于 Undo，`new_value` 用于 Redo。
    *   **`ToString()`**: 提供日志记录的字符串表示，方便调试和日志输出。

3.  **`CheckpointState` (检查点状态结构体)**：
    *   **职责**: 存储创建检查点时的关键恢复信息。
    *   **关键字段**:
        *   `checkpoint_lsn`: 创建检查点时已刷盘的最高 LSN。
        *   `timestamp`: 检查点创建的时间。
        *   **`TODO(#WAL-004)`**: 缺少页面状态快照 (DPT - Dirty Page Table) 和活跃事务表 (ATT - Active Transaction Table)。这些是 ARIES 算法中在恢复的“分析”阶段构建的关键信息，用于确定从何处开始 Redo。

4.  **`WALManager` (预写日志管理器)**：
    *   **职责**: WAL 协议的核心实现者，管理日志文件的所有操作。
    *   **主要职责**:
        *   **日志记录 (`Log`, `LogBatch`)**: 将 `LogRecord` 写入内存缓冲区。
        *   **日志刷盘 (`ForceFlush`, `AsyncFlush`)**: 将缓冲区日志持久化到磁盘。
        *   **LSN 管理**: 使用 `next_lsn_` (`std::atomic<uint64_t>`) 生成唯一的 LSN。
        *   **检查点 (`CreateCheckpoint`, `GetLastCheckpoint`, `GetCheckpointHistory`)**: 管理检查点的创建和查询。
        *   **崩溃恢复 (`RecoverFromLog`, `ReplayLog`, `AnalyzeLog`)**: 负责数据库的崩溃恢复流程。
        *   **日志文件管理**: (`InitializeLogFile`, `WriteRecordsToDisk`, `ReadRecordFromDisk`) 负责日志文件的物理读写。
        *   **性能指标 (`GetMetrics`, `ResetMetrics`)**: 收集和报告 WAL 系统的运行状态和性能统计。
        *   **日志压缩 (`CompactLog`)**: (简化实现) 回收旧日志空间。
        *   **日志完整性验证 (`VerifyLogIntegrity`)**: (简化实现) 检查日志文件是否损坏。
    *   **并发控制**: 使用 `buffer_mutex_`、`metrics_mutex_`、`checkpoint_mutex_` 保护共享数据结构，并通过 `buffer_cv_` 和 `stop_flush_thread_` 协调异步刷盘线程。
    *   **异步刷盘线程**: (`AsyncFlushThread`) (目前禁用，`TODO(#WAL-002)`)，用于在后台周期性地将日志缓冲区的内容刷入磁盘，平衡性能与持久性。

### 2.2. WAL 的核心原则

*   **预写日志 (Write-Ahead Logging)**: 任何数据修改都必须先写日志。
*   **顺序写入 (Sequential Write)**: 日志文件被设计为顺序追加，利用磁盘顺序 I/O 的高吞吐量。
*   **LSN 管理 (LSN Management)**: 每条日志记录都有一个唯一的 LSN，是日志的逻辑时间戳和物理地址的抽象。
*   **原子性 (Atomicity)**: 单条日志记录的写入操作必须是原子的。
*   **持久性 (Durability)**: 通过 `flush()` 和 `fsync()` (在生产环境中) 确保日志记录真正持久化到磁盘。
*   **检查点 (Checkpointing)**: 定期创建，用于减少崩溃恢复时的扫描范围。

---

## 3. HOW: WAL 管理器的工作流程和实现细节？

### 3.1. 日志记录 (Logging) 流程 (`Log`, `LogBatch`)

1.  **分配 LSN**: 每次调用 `Log()` 或 `LogBatch()` 时，首先通过 `GenerateLSN()` 分配一个唯一的、递增的 LSN。
2.  **封装 `LogRecord`**: 根据操作类型和数据（`txn_id`, `key`, `old_value`, `new_value`, `timestamp` 等），构造 `LogRecord` 实例。
3.  **写入缓冲区**: 将 `LogRecord` 添加到内存中的 `log_buffer_` (`std::vector<LogRecord>`)。此操作受 `buffer_mutex_` 保护。
4.  **通知刷盘线程**: `buffer_cv_.notify_one()` 用于唤醒异步刷盘线程，告知有新日志需要处理。

### 3.2. 日志刷盘 (Flushing) 流程 (`ForceFlush`, `AsyncFlush`, `AsyncFlushThread`, `WriteRecordsToDisk`)

1.  **缓冲区管理**: 新生成的日志记录首先存放在内存中的 `log_buffer_` 中。
2.  **`ForceFlush()` (强制刷盘)**:
    *   在 `buffer_mutex_` 保护下，将 `log_buffer_` 的内容原子地 `swap` 到一个临时 `std::vector<LogRecord>`。
    *   调用 `WriteRecordsToDisk()` 将临时向量中的所有记录写入日志文件。
    *   更新 `last_flushed_lsn_` 和性能指标。
    *   **特点**: 阻塞调用线程，直到日志写入磁盘，确保强持久性（例如，事务提交时）。
3.  **`AsyncFlushThread()` (异步刷盘线程 - `TODO(#WAL-002)` 待启用)**:
    *   一个独立的后台线程，周期性地（由 `flush_interval_ms_` 控制）或在被 `buffer_cv_.notify_one()` 唤醒时执行刷盘。
    *   它会调用 `ForceFlush()` 将缓冲区内容写入磁盘。
    *   **特点**: 非阻塞调用线程，提高事务吞吐量，但牺牲了一定的即时持久性（日志可能在内存中停留一段时间）。
4.  **`WriteRecordsToDisk()` (物理写入磁盘)**:
    *   以追加模式打开日志文件 (`std::ofstream`)。
    *   遍历 `LogRecord` 向量，将每条记录序列化为二进制格式并写入文件。
    *   **`TODO(#WAL-001)`**: 序列化格式需扩展以包含 `old_value` 和 `new_value`。
    *   执行 `log_file.flush()` 将数据从 C++ 缓冲区刷新到操作系统文件缓存。
    *   **`TODO(#WAL-013)`**: 在生产环境中，需要调用 `fsync()` 或 `fdatasync()` 来确保数据真正写入物理磁盘，这对于确保持久性至关重要。

### 3.3. 检查点 (Checkpointing) 流程 (`CreateCheckpoint`, `WriteCheckpointToDisk`, `ReadCheckpointFromDisk`)

1.  **`CreateCheckpoint(sync)`**:
    *   首先调用 `ForceFlush()` 确保所有日志都已刷盘。
    *   记录当前 `last_flushed_lsn_` 作为检查点 LSN。
    *   **`TODO(#WAL-004)`**: 构建 `CheckpointState`，除了 LSN 和时间戳，还需要记录当前**脏页表 (Dirty Page Table, DPT)** 和**活跃事务表 (Active Transaction Table, ATT)** 的快照。这些是 ARIES 恢复算法的关键。
    *   调用 `WriteCheckpointToDisk()` 将 `CheckpointState` 写入检查点文件。
    *   更新 `checkpoint_history_`，保留最近的检查点记录。
2.  **`WriteCheckpointToDisk(checkpoint)`**: 将 `CheckpointState` 序列化为二进制格式写入 `checkpoint_file_path_`。
3.  **`ReadCheckpointFromDisk()`**: 从 `checkpoint_file_path_` 读取 `CheckpointState`。

### 3.4. 崩溃恢复 (Crash Recovery) 流程 (`RecoverFromLog`, `AnalyzeLog`, `ReplayLog`, `GetInProgressTransactions`)

崩溃恢复通常遵循 ARIES 协议的三个阶段：

1.  **分析阶段 (Analysis Phase)**:
    *   `RecoverFromLog()` 首先调用 `GetLastCheckpoint()` 获取最近的检查点。
    *   从检查点开始扫描日志，构建崩溃时的 **活跃事务表 (ATT)** 和 **脏页表 (DPT)**。
    *   **`TODO(#WAL-003)`**: `AnalyzeLog()` 方法将负责此阶段，但目前是简化实现。
    *   **`TODO(#WAL-006)`**: `GetInProgressTransactions()` 需要扫描日志以识别所有已 BEGIN 但未 COMMIT/ABORT 的事务。

2.  **重做阶段 (Redo Phase)**:
    *   **`TODO(#WAL-005)`**: 从分析阶段确定的 Redo LSN 开始，`ReplayLog()` 方法扫描日志，将所有已提交事务（以及在崩溃时已部分写入磁盘的未提交事务）的修改重新应用到数据库，确保数据库达到崩溃前的最新状态。
    *   `ReplayLog()` 调用 `ReadLogRange()` 从磁盘读取日志记录，并根据 `LogRecordType` 调用存储引擎接口（例如 `TODO(#WAL-007, #WAL-008, #WAL-009)`）应用 `UPDATE`, `INSERT`, `DELETE` 等操作。

3.  **撤销阶段 (Undo Phase)**:
    *   对于分析阶段识别出的所有未提交事务，`ReplayLog()` 负责执行撤销操作。这通常通过反向遍历这些事务的日志记录，应用 `old_value` 来恢复数据到事务开始前的状态。
    *   **`TODO(#WAL-001)`**: 完整实现需要 `LogRecord` 包含 `old_value`。

### 3.5. 简化的类图

```mermaid
classDiagram
    class LogRecord {
        +lsn: uint64_t
        +txn_id: TransactionId
        +type: LogRecordType
        +key: string
        // +old_value: string // TODO(#WAL-001)
        // +new_value: string // TODO(#WAL-001)
        +timestamp: chrono::system_clock::time_point
        +ToString(): string
    }

    class CheckpointState {
        +checkpoint_lsn: uint64_t
        +timestamp: chrono::system_clock::time_point
        // +page_state_snapshot: map<PageId, LSN> // TODO(#WAL-004)
    }

    class WALManager {
        -log_file_path_: string
        -checkpoint_file_path_: string
        -next_lsn_: atomic<uint64_t>
        -last_flushed_lsn_: uint64_t
        -last_checkpoint_lsn_: uint64_t
        -log_buffer_: vector<LogRecord>
        -buffer_mutex_: mutex
        -buffer_cv_: condition_variable
        -flush_thread_: thread // TODO(#WAL-002)
        -stop_flush_thread_: atomic<bool> // TODO(#WAL-002)
        -flush_interval_ms_: uint32_t
        -force_sync_: bool // TODO(#WAL-002)
        -checkpoint_history_: vector<CheckpointState>
        -checkpoint_mutex_: mutex
        -metrics_: WALMetrics
        -metrics_mutex_: mutex
        
        +WALManager(log_path, force_sync)
        +~WALManager()
        +Log(record): uint64_t
        +LogBatch(records): uint64_t
        +ForceFlush(): void
        +AsyncFlush(): void // Triggers async flush
        +ReadLogRange(from_lsn, to_lsn): vector<LogRecord>
        +AnalyzeLog(): map<string, string> // TODO(#WAL-003)
        +CreateCheckpoint(sync): uint64_t // TODO(#WAL-004)
        +GetLastCheckpoint(): CheckpointState
        +GetCheckpointHistory(): vector<CheckpointState>
        +RecoverFromLog(): bool // TODO(#WAL-005)
        +GetInProgressTransactions(): vector<TransactionId> // TODO(#WAL-006)
        +ReplayLog(from_lsn, to_lsn): uint64_t // TODO(#WAL-007, #WAL-008, #WAL-009)
        +GetMetrics(): WALMetrics // TODO(#WAL-010)
        +ResetMetrics(): void
        +CompactLog(keep_lsn): size_t // TODO(#WAL-011)
        +VerifyLogIntegrity(): bool // TODO(#WAL-012)
        
        -InitializeLogFile(): void
        -GenerateLSN(): uint64_t
        -WriteRecordsToDisk(records): size_t // TODO(#WAL-001, #WAL-013)
        -ReadRecordFromDisk(lsn): LogRecord // TODO(#WAL-014)
        -WriteCheckpointToDisk(chk): void // TODO(#WAL-004, #WAL-015)
        -ReadCheckpointFromDisk(): CheckpointState // TODO(#WAL-004)
        -AsyncFlushThread(): void // TODO(#WAL-002)
    }

    enum LogRecordType {
        BEGIN
        COMMIT
        ABORT
        UPDATE
        INSERT
        DELETE
        COMPENSATE
    }
```

---

## 4. 总结与 TODO 列表

WAL 管理器是实现数据库持久性和崩溃恢复的基石。本设计文档和对应的代码注释详细阐述了 `WALManager` 的原理、组件和主要工作流程。

**当前版本的关键 TODO 列表：**

*   **`LogRecord` 结构扩展 (`#WAL-001`)**: 需要为 `LogRecord` 结构体添加 `old_value` 和 `new_value` 字段，并更新对应的序列化/反序列化逻辑。这是实现完整 Redo/Undo 操作的基础。
*   **异步刷盘功能启用 (`#WAL-002`)**: 需要修复异步刷盘线程的编译问题，并在 `WALManager` 构造函数和析构函数中处理线程的启动与优雅停止。
*   **日志分析逻辑 (`#WAL-003`)**: `AnalyzeLog()` 方法是恢复协议“分析”阶段的核心，需完整实现扫描日志，构建活跃事务表和脏页表的逻辑。
*   **检查点页面状态快照 (`#WAL-004`)**: `CheckpointState` 需要记录脏页表和活跃事务表快照，并更新 `WriteCheckpointToDisk()` 和 `ReadCheckpointFromDisk()` 以支持这些信息的序列化/反序列化。
*   **恢复协议重做阶段 (`#WAL-005`)**: `RecoverFromLog()` 中的 Redo 阶段需要根据分析阶段的结果，重新应用日志中的修改。
*   **进行中事务识别 (`#WAL-006`)**: `GetInProgressTransactions()` 需要完整实现通过扫描日志来识别尚未提交/中止的事务。
*   **存储引擎集成 (`#WAL-007`, `#WAL-008`, `#WAL-009`)**: `ReplayLog()` 中需要调用存储引擎接口来实际应用 `UPDATE`, `INSERT`, `DELETE` 操作。
*   **指标收集 (`#WAL-010`)**: `GetMetrics()` 需要线程安全地收集日志文件大小和待处理记录数等详细指标。
*   **日志压缩 (`#WAL-011`)**: `CompactLog()` 需要实现完整的日志压缩逻辑，包括空间回收和旧日志移除。
*   **日志完整性验证 (`#WAL-012`)**: `VerifyLogIntegrity()` 需要实现日志文件损坏检测和一致性验证。
*   **持久化刷盘 (`#WAL-013`, `#WAL-015`)**: `WriteRecordsToDisk()` 和 `WriteCheckpointToDisk()` 在生产环境中需要考虑使用 `fsync()` 等系统调用，确保数据真正写入物理磁盘。
*   **记录读取定位 (`#WAL-014`)**: `ReadRecordFromDisk()` 需要解决如何高效地根据 LSN 定位并读取文件中的日志记录。

未来的工作将集中在完成上述 TODO 项，并进一步集成 WAL 管理器与缓冲池管理器、存储引擎和事务管理器，以实现一个完整、高效且可靠的崩溃恢复系统。
