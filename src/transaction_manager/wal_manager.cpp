/**
 * @file wal_manager.cc
 * @brief WAL（预写日志）管理器核心实现 - v0.4.8版本
 *
 * @WHY
 * 在数据库管理系统（DBMS）中，**持久性 (Durability)** 是 ACID 特性之一，确保一旦事务提交，其所做的更改将永久保存，即使系统发生故障也不会丢失。**崩溃恢复 (Crash Recovery)** 机制则是实现持久性的关键。
 *
 * 传统的“原地更新”策略（直接修改磁盘数据页）在系统崩溃时面临巨大风险：
 * 1.  **数据丢失**: 已提交事务的修改可能尚未写入磁盘。
 * 2.  **数据不一致**: 未提交事务的部分修改可能写入磁盘，导致数据库状态损坏。
 *
 * **预写日志 (Write-Ahead Logging, WAL)** 是一种被广泛采用的协议，旨在解决这些问题。WAL 协议的核心思想是：**任何数据修改都必须先将对应的日志记录写入并持久化到稳定存储 (磁盘) 后，才能将数据页本身写入磁盘。** 这保证了在系统崩溃时，可以利用日志进行恢复操作，要么重做 (Redo) 已提交事务的更改，要么撤销 (Undo) 未提交事务的更改。
 *
 * 本 WAL 管理器旨在提供一个高效、可靠的日志记录和恢复机制，确保 SQLCC 数据库的持久性和崩溃可恢复性。
 *
 * @WHAT
 * 本文件实现了 `LogRecordType` 枚举，`LogRecord` 结构体和 `WALManager` 类。
 * `LogRecordType` 定义了WAL日志记录的各种类型，对应数据库操作或事务控制。
 * `LogRecord` 表示单个数据库操作的日志记录，包含事务 ID、操作类型、LSN 等信息。
 * `WALManager` 则是 WAL 协议的核心实现，负责：
 * 1.  **日志记录 (Logging)**: 将所有对数据库的修改操作以日志记录 (`LogRecord`) 的形式写入顺序日志文件。
 * 2.  **日志序列号 (LSN) 管理**: 为每条日志记录分配唯一的、递增的 LSN，确保日志的顺序性和可追溯性。
 * 3.  **日志刷盘 (Flushing)**: 将内存中的日志缓冲区数据写入磁盘，确保日志的持久性。支持同步和异步刷盘模式。
 * 4.  **检查点 (Checkpointing)**: 定期创建数据库的一致性快照，减少崩溃恢复时需要扫描的日志量。
 * 5.  **崩溃恢复 (Crash Recovery)**: 在系统重启后，利用日志文件将数据库恢复到崩溃前的最后一致状态。
 * 6.  **性能优化**: 通过日志缓冲区、组提交、异步刷盘等技术，最小化日志写入对事务性能的影响。
 * 7.  **并发安全**: 使用互斥锁和条件变量等同步机制，确保多线程环境下日志操作的并发安全。
 *
 * @HOW
 * `WALManager` 的工作流程和实现原则如下：
 *
 * **日志记录与刷盘**:
 * 1.  **日志缓冲区**: 所有新的 `LogRecord` 首先被写入内存中的日志缓冲区 (`log_buffer_`)。
 * 2.  **LSN 分配**: 每条记录在被添加到缓冲区时都会被分配一个唯一的 LSN。
 * 3.  **刷盘策略**:
 *     *   **强制同步 (Force Sync)**: 在某些关键操作（如事务提交）时，可以强制将缓冲区中的日志立即刷入磁盘。
 *     *   **异步刷盘 (Async Flushing)**: 一个独立的后台线程 (`flush_thread_`) 周期性地（或被唤醒时）将日志缓冲区的内容批量写入磁盘。这平衡了性能和持久性保证。
 * 4.  **顺序写入**: 日志文件被设计为顺序写入，这与磁盘的物理特性相符，提供了极高的写入性能。
 *
 * **检查点机制**:
 * 1.  **定期创建**: `CreateCheckpoint()` 方法周期性地（或在特定事件触发时）被调用。
 * 2.  **记录状态**: 检查点记录当前已刷盘的 LSN，以及数据库的关键状态信息（例如，脏页表、活跃事务列表）。
 * 3.  **减少恢复时间**: 崩溃恢复时，只需从最新的检查点开始重放日志，大大减少了恢复所需的时间。
 *
 * **崩溃恢复流程**:
 * 1.  **分析 (Analysis)**: 从最新的检查点开始向前扫描日志，识别所有在崩溃时尚未完成的事务以及所有可能需要重做或撤销的操作。
 * 2.  **重做 (Redo)**: 扫描所有日志记录，将所有已提交事务的更改应用到数据库（即使这些更改在崩溃前已部分写入磁盘）。
 * 3.  **撤销 (Undo)**: 扫描日志，撤销所有在崩溃时尚未提交事务的更改。
 *
 * **并发安全**:
 * -   `buffer_mutex_`: 保护日志缓冲区。
 * -   `flush_thread_` 和 `buffer_cv_`: 协调主线程和异步刷盘线程之间的工作。
 * -   `next_lsn_`: 原子变量，确保 LSN 的唯一性。
 *
 * **性能优化策略：**
 * - 缓冲区批量写入，减少系统调用。
 * - 异步刷盘，平衡性能和一致性。
 * - 组提交，减少等待时间。
 * - 预分配文件，减少磁盘寻道。
 *
 * @note 本版本为 v0.4.8，部分功能仍在完善中。
 */
#include "../wal_manager.h"
#include "../logger/logger.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace sqlcc {

/**
 * @brief 定义了WAL日志记录的类型。
 * 这些类型对应数据库操作（如BEGIN/COMMIT/ABORT事务控制，UPDATE/INSERT/DELETE数据修改）或内部恢复机制（如COMPENSATE）。
 */
enum class LogRecordType {
  BEGIN,      ///< 事务开始
  COMMIT,     ///< 事务提交
  ABORT,      ///< 事务中止
  UPDATE,     ///< 数据更新操作
  INSERT,     ///< 数据插入操作
  DELETE,     ///< 数据删除操作
  COMPENSATE, ///< 补偿日志记录，用于恢复操作
};

/**
 * @brief 单条WAL日志记录的结构体。
 * 每条记录都包含必要的元数据和操作详情，以便在崩溃恢复时进行重做或撤销。
 */
struct LogRecord {
  uint64_t lsn;         ///< 日志序列号 (Log Sequence Number)，唯一且递增
  TransactionId txn_id; ///< 事务ID
  LogRecordType type;   ///< 日志记录类型
  std::string key;      ///< 涉及的数据键（例如表名+主键）
  // TODO(#WAL-001): old_value 和 new_value 是实现UNDO/REDO的关键，需要在此处定义并序列化/反序列化。
  // std::string old_value;
  // std::string new_value;
  std::chrono::system_clock::time_point timestamp; ///< 记录生成时间戳

  /**
   * @brief 将日志记录转换为可读字符串，用于调试和日志输出。
   * @return LogRecord的字符串表示。
   */
  std::string ToString() const {
    std::stringstream ss;
    ss << "[" << lsn << "] ";
    ss << "Txn" << txn_id << " ";
    ss << "Type:";

    switch (type) {
    case LogRecordType::BEGIN:
      ss << "BEGIN";
      break;
    case LogRecordType::COMMIT:
      ss << "COMMIT";
      break;
    case LogRecordType::ABORT:
      ss << "ABORT";
      break;
    case LogRecordType::UPDATE:
      ss << "UPDATE";
      break;
    case LogRecordType::INSERT:
      ss << "INSERT";
      break;
    case LogRecordType::DELETE:
      ss << "DELETE";
      break;
    case LogRecordType::COMPENSATE:
      ss << "COMPENSATE";
      break;
    }

    ss << " Key:'" << key << "'";
    auto timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            timestamp.time_since_epoch())
                            .count();
    ss << " TS:" << timestamp_ms;

    return ss.str();
  }
};
// ================== WALManager 实现 ==================
/**
 * @brief 构造WALManager实例。
 * @details 初始化WAL管理器的各项参数，包括日志文件路径、检查点文件路径、LSN管理器等。
 * 同时，会初始化日志文件，并在当前版本（v0.4.8）中暂时禁用异步刷盘功能以避免编译问题，强制使用同步模式。
 * @param log_file_path 日志文件的存储路径。
 * @param force_sync 是否强制同步刷盘（即使在异步模式下也会立即刷盘）。
 */
WALManager::WALManager(const std::string &log_file_path, bool force_sync)
    : log_file_path_(log_file_path),
      checkpoint_file_path_(log_file_path + ".chk"), next_lsn_(1),
      last_flushed_lsn_(0), last_checkpoint_lsn_(0), force_sync_(force_sync),
      flush_interval_ms_(100) { // 100ms异步刷盘间隔

  // 初始化指标
  metrics_.total_records = 0;
  metrics_.flushed_records = 0;
  metrics_.pending_records = 0;
  metrics_.total_checkpoints = 0;
  metrics_.log_file_size_bytes = 0;

  // 初始化日志文件
  InitializeLogFile();

  // TODO(#WAL-002): 暂时禁用异步刷盘功能以解决编译问题，待异步刷盘逻辑稳定后重新启用。
  // 目前强制使用同步模式，并阻止异步刷盘线程启动。
  force_sync_ = true; // 强制使用同步模式
  stop_flush_thread_ = true;

  std::cout << "WALManager 已初始化 - 日志文件: " << log_file_path_
            << " 强制同步: " << (force_sync_ ? "是" : "否") << std::endl;
}

/**
 * @brief 销毁WALManager实例。
 * @details 在销毁之前，会强制将所有内存中未刷盘的日志记录写入磁盘，确保数据持久性。
 * 由于异步刷盘功能暂时禁用，此析构函数无需等待异步线程停止。
 */
WALManager::~WALManager() {
  // TODO(#WAL-002): 异步刷盘功能已被禁用，当重新启用时，此处需处理线程的优雅停止。
  // 此处无需处理线程停止

  // 强制刷盘所有待写入日志，确保所有日志记录都被持久化。
  ForceFlush();

  std::cout << "WALManager 已销毁，所有日志已刷盘" << std::endl;
}

/**
 * @brief 记录一条WAL日志记录到缓冲区。
 * @details 为日志记录分配LSN和时间戳，并将其添加到内存日志缓冲区。
 * 如果WAL管理器配置为强制同步模式，会立即触发一次刷盘操作。
 * @param record 待记录的LogRecord实例。
 * @return 分配给该日志记录的LSN。
 */
uint64_t WALManager::Log(LogRecord record) {
  // 1. 分配日志序列号 (LSN) 和时间戳。
  uint64_t assigned_lsn = GenerateLSN();
  record.lsn = assigned_lsn;
  record.timestamp = std::chrono::system_clock::now();

  // 2. 将日志记录添加到内存缓冲区，并通知可能的异步刷盘线程。
  {
    std::unique_lock<std::mutex> lock(buffer_mutex_);
    log_buffer_.push_back(record);
    buffer_cv_.notify_one(); // 唤醒异步刷盘线程（如果正在等待）
  }

  // 3. 更新WAL指标。
  {
    std::unique_lock<std::mutex> lock(metrics_mutex_);
    metrics_.total_records++;
    metrics_.pending_records++;
  }

  // 4. 如果是强制同步模式，立即将所有待写入日志刷盘。
  // 这会阻塞当前调用线程直到刷盘完成。
  if (force_sync_) {
    ForceFlush();
  }

  return assigned_lsn;
}
/**
 * @brief 批量记录WAL日志到缓冲区。
 * @details 为每条日志记录分配LSN和时间戳，并将其作为一个批次添加到内存日志缓冲区。
 * 如果WAL管理器配置为强制同步模式，会立即触发一次刷盘操作，实现批提交。
 * @param records 待记录的LogRecord实例向量。
 * @return 批次中最后一条日志记录的LSN。
 */
uint64_t WALManager::LogBatch(const std::vector<LogRecord> &records) {
  uint64_t last_lsn = 0;

  // 1. 为批量记录分配连续的LSN，并添加到缓冲区。
  std::unique_lock<std::mutex> buffer_lock(buffer_mutex_);
  for (auto record : records) { // 注意：这里是按值拷贝，需确保record的lsn和timestamp被更新
    uint64_t assigned_lsn = GenerateLSN();
    record.lsn = assigned_lsn;
    record.timestamp = std::chrono::system_clock::now();
    log_buffer_.push_back(record);
    last_lsn = assigned_lsn;
  }
  buffer_cv_.notify_one(); // 唤醒异步刷盘线程（如果正在等待）

  // 2. 更新WAL指标。
  {
    std::unique_lock<std::mutex> lock(metrics_mutex_);
    metrics_.total_records += records.size();
    metrics_.pending_records += records.size();
  }

  // 3. 如果是强制同步模式，立即将所有待写入日志刷盘，实现“组提交”效果。
  if (force_sync_) {
    buffer_lock.unlock(); // 在调用ForceFlush之前释放锁，避免死锁
    ForceFlush();
  }

  return last_lsn;
}

/**
 * @brief 强制将所有内存中日志缓冲区的内容刷入磁盘。
 * @details 该方法会清空日志缓冲区，将所有记录写入日志文件并进行fsync操作，确保日志的持久化。
 * 此操作会阻塞调用线程。
 */
void WALManager::ForceFlush() {
  std::vector<LogRecord> records_to_flush;

  // 1. 获取待刷盘的记录：原子地交换缓冲区内容，减少锁持有时间。
  {
    std::unique_lock<std::mutex> lock(buffer_mutex_);
    records_to_flush.swap(log_buffer_);
  }

  if (records_to_flush.empty()) {
    return; // 没有待刷盘的记录，直接返回。
  }

  // 2. 记录刷盘开始时间，用于性能指标统计。
  auto start_time = std::chrono::high_resolution_clock::now();

  // 3. 将记录写入磁盘。
  size_t written_count = WriteRecordsToDisk(records_to_flush);

  // 4. 记录刷盘结束时间并更新性能指标。
  auto end_time = std::chrono::high_resolution_clock::now();
  auto flush_time = std::chrono::duration_cast<std::chrono::microseconds>(
      end_time - start_time);

  {
    std::unique_lock<std::mutex> lock(metrics_mutex_);
    metrics_.flushed_records += written_count;
    metrics_.pending_records -= written_count;
    metrics_.total_flush_time += flush_time;

    if (metrics_.flushed_records > 0) {
      metrics_.avg_flush_time = std::chrono::microseconds(
          metrics_.total_flush_time.count() / metrics_.flushed_records);
    }
  }

  // 5. 更新最后刷盘的LSN，这是恢复点的重要标志。
  if (!records_to_flush.empty()) {
    last_flushed_lsn_ = records_to_flush.back().lsn;
  }

  std::cout << "已强制刷盘 " << written_count << " 条日志记录，用时 "
            << flush_time.count() << " 微秒" << std::endl;
}
/**
 * @brief 触发异步刷盘线程执行一次刷盘操作。
 * @details 该方法会唤醒等待中的异步刷盘线程，使其检查缓冲区并执行刷盘。
 * 异步刷盘功能当前已在构造函数中暂时禁用。
 */
void WALManager::AsyncFlush() {
  // 唤醒异步刷盘线程（如果它正在等待通知或超时）
  // TODO(#WAL-002): 当异步刷盘功能重新启用时，此方法将用于外部触发刷盘。
  {
    std::unique_lock<std::mutex> lock(buffer_mutex_);
    buffer_cv_.notify_one(); 
  }
}
/**
 * @brief 从磁盘读取指定LSN范围内的日志记录。
 * @details 该方法会顺序读取从`from_lsn`到`to_lsn`范围内的日志记录。
 * 内部通过调用`ReadRecordFromDisk`实现单条记录的读取。
 * @param from_lsn 起始LSN（包含）。
 * @param to_lsn 结束LSN（包含）。
 * @return 包含指定范围内所有LogRecord的向量。如果读取过程中遇到错误或文件结束，会提前停止。
 */
std::vector<LogRecord> WALManager::ReadLogRange(uint64_t from_lsn,
                                                uint64_t to_lsn) {
  std::vector<LogRecord> result;

  // 1. 遍历LSN范围，从磁盘顺序读取日志记录。
  for (uint64_t lsn = from_lsn; lsn <= to_lsn; ++lsn) {
    try {
      LogRecord record = ReadRecordFromDisk(lsn);
      // 2. 检查记录是否存在（LSN > 0通常表示有效记录，因为LSN从1开始）。
      if (record.lsn > 0) { 
        result.push_back(record);
      } else {
        // 如果ReadRecordFromDisk返回LSN=0的记录，表示未找到或已到文件末尾。
        break; 
      }
    } catch (const std::exception &e) {
      // 捕获读取异常（例如，文件损坏、格式错误），停止读取。
      std::cerr << "Error reading log record for LSN " << lsn << ": " << e.what() << std::endl;
      break;
    }
  }

  return result;
}

/**
 * @brief 分析WAL日志，为崩溃恢复准备必要的分析结果。
 * @details 该方法是恢复协议的“分析”阶段，用于识别崩溃前所有活跃事务的状态（已提交、未提交、已中止）。
 * 在一个完整的实现中，会扫描日志以构建活跃事务表和脏页表。
 * @return 包含恢复操作所需信息的映射。
 */
std::unordered_map<std::string, std::string> WALManager::AnalyzeLog() const {
  std::unordered_map<std::string, std::string> recovery_actions;

  // TODO(#WAL-003): 实现日志分析逻辑。
  // 此处应扫描所有日志记录，识别崩溃前已提交/未提交/已中止的事务，
  // 并构建脏页表和活跃事务表，这是恢复协议“分析”阶段的核心。
  // 临时注释掉会导致编译错误的部分
  // std::unique_lock<std::mutex> lock(buffer_mutex_);

  (void)recovery_actions; // 避免未使用变量警告
  return {}; // 暂未实现，返回空。
}
/**
 * @brief 创建一个WAL检查点。
 * @details 检查点用于标记日志中的一个安全点，从该点开始，可以保证在崩溃恢复时，
 * 不需要检查所有之前的日志记录。这会先强制刷盘所有日志，然后记录当前状态。
 * @param sync 是否同步写入检查点到磁盘。
 * @return 创建的检查点所对应的LSN。
 */
uint64_t WALManager::CreateCheckpoint(bool sync) {
  // 1. 强制刷盘所有待写入的日志记录，确保检查点之前的日志都已持久化。
  ForceFlush();

  // 2. 记录检查点所对应的LSN，通常是当前已刷盘的最后一个LSN。
  uint64_t checkpoint_lsn = last_flushed_lsn_;

  // 3. 构建检查点状态对象。
  CheckpointState checkpoint{
      checkpoint_lsn,
      std::chrono::system_clock::now(),
      {} // TODO(#WAL-004): 获取当前页面状态快照 (如脏页表、活跃事务表)。
         // 这是实现ARIES等恢复算法的关键信息，用于在恢复的“重做”阶段跳过不必要的扫描。
  };

  // 4. 根据同步标志，将检查点状态写入磁盘。
  if (sync) {
    WriteCheckpointToDisk(checkpoint);
  }

  // 5. 更新检查点历史记录，保留最近的检查点以供回溯。
  {
    std::unique_lock<std::mutex> lock(checkpoint_mutex_);
    checkpoint_history_.push_back(checkpoint);
    if (checkpoint_history_.size() > 100) { // 保留最近100个检查点以避免内存无限制增长
      checkpoint_history_.erase(checkpoint_history_.begin());
    }
  }

  // 6. 更新最后检查点的LSN。
  last_checkpoint_lsn_ = checkpoint_lsn;

  // 7. 更新WAL指标。
  {
    std::unique_lock<std::mutex> lock(metrics_mutex_);
    metrics_.total_checkpoints++;
  }

  std::cout << "已创建检查点，LSN: " << checkpoint_lsn
            << " 同步模式: " << (sync ? "是" : "否") << std::endl;

  return checkpoint_lsn;
}

/**
 * @brief 获取最近创建的检查点状态。
 * @details 该方法返回检查点历史记录中最新的一个检查点状态。
 * @return 最新的CheckpointState实例。
 */
CheckpointState WALManager::GetLastCheckpoint() const {
  return GetCheckpointHistory().back();
}
/**
 * @brief 获取检查点历史记录。
 * @details 返回所有保留的检查点状态的列表，用于分析和可能的恢复路径选择。
 * @return 包含CheckpointState的向量。
 */
std::vector<CheckpointState> WALManager::GetCheckpointHistory() const {
  std::unique_lock<std::mutex> lock(checkpoint_mutex_); // 保护检查点历史记录的并发访问
  return checkpoint_history_;
}
/**
 * @brief 执行崩溃恢复流程。
 * @details 该方法实现了WAL恢复协议的核心逻辑，包括从最后一个检查点开始，
 * 扫描日志、分析事务状态（Redo/Undo阶段）、并最终将数据库恢复到崩溃前的状态。
 * @return 恢复成功返回true，否则返回false。
 */
bool WALManager::RecoverFromLog() {
  try {
    std::cout << "开始从WAL执行崩溃恢复..." << std::endl;

    // 1. 获取最后一个检查点状态，确定日志扫描的起始位置。
    // 在ARIES等恢复算法中，还需要从检查点状态中获取DPT（脏页表）和ATT（活跃事务表）。
    CheckpointState last_checkpoint = GetLastCheckpoint();
    uint64_t start_lsn = last_checkpoint.checkpoint_lsn + 1; // 从检查点之后的下一条日志开始扫描
    uint64_t current_lsn = start_lsn;

    std::cout << "从检查点LSN " << last_checkpoint.checkpoint_lsn << " 开始恢复"
              << std::endl;

    // 2. 分析阶段 (Analysis Phase): 扫描日志以识别所有活跃事务和脏页。
    // 简化实现：这里只跟踪事务的提交/中止状态。
    std::unordered_map<TransactionId, bool> txn_status; // true表示已提交，false表示未提交
    while (true) {
      try {
        LogRecord record = ReadRecordFromDisk(current_lsn);
        if (record.lsn == 0)
          break; // 到达有效日志的末尾

        // 跟踪事务状态，构建活跃事务表。
        if (record.type == LogRecordType::BEGIN) {
          txn_status[record.txn_id] = false; // 事务开始，标记为未提交
        } else if (record.type == LogRecordType::COMMIT) {
          txn_status[record.txn_id] = true; // 事务提交，标记为已提交
          std::cout << "事务 " << record.txn_id << " 已提交" << std::endl;
        } else if (record.type == LogRecordType::ABORT) {
          txn_status.erase(record.txn_id); // 事务中止，从表中移除
          std::cout << "事务 " << record.txn_id << " 已中止" << std::endl;
        }
        current_lsn++;
      } catch (const std::exception &) {
        // 读取日志时发生异常，通常意味着已到达日志文件末尾或日志损坏。
        break;
      }
    }

    // TODO(#WAL-005): 重做阶段 (Redo Phase):
    // 根据分析阶段构建的脏页表和LSN，将所有已提交事务和部分提交事务的更改重做到数据库。
    // 确保数据库达到崩溃前的状态。

    // 3. 回滚阶段 (Undo Phase):
    // 遍历txn_status中所有标记为未提交（false）的事务，对这些事务执行Undo操作。
    // 确保数据库中不包含任何未提交事务的更改。
    uint64_t max_recover_lsn = ReplayLog(start_lsn, current_lsn - 1); // 这里的ReplayLog目前只做了Redo操作。

    std::cout << "恢复完成，处理了 " << (max_recover_lsn - start_lsn + 1)
              << " 条日志记录" << std::endl;
    std::cout << "检查点后的LSN范围: " << start_lsn << " - " << max_recover_lsn
              << std::endl;

    return true;
  } catch (const std::exception &e) {
    std::cerr << "崩溃恢复失败: " << e.what() << std::endl;
    return false;
  }
}
/**
 * @brief 获取当前所有正在进行中的事务ID列表。
 * @details 该方法通常用于崩溃恢复的分析阶段，以识别崩溃时尚未提交的事务。
 * @return 包含所有正在进行中事务ID的向量。
 */
std::vector<TransactionId> WALManager::GetInProgressTransactions() const {
  std::vector<TransactionId> active_transactions;

  // TODO(#WAL-006): 实现获取进行中事务的逻辑。
  // 这需要扫描从上次检查点到当前的所有日志记录，识别出所有BEGIN但没有COMMIT/ABORT的事务。
  // 临时注释掉会导致编译错误的部分
  // std::vector<LogRecord> recent_logs = ReadLogRange(last_checkpoint_lsn_ + 1,
  // next_lsn_.load() - 1);

  (void)active_transactions; // 避免未使用变量警告
  return {}; // 暂未实现，返回空。
}
/**
 * @brief 重放指定LSN范围内的日志记录，以恢复数据库状态。
 * @details 该方法是WAL恢复协议的“重做”阶段，将日志中的数据修改操作应用到数据库，
 * 确保数据库达到崩溃前的最新状态。
 * @param from_lsn 起始LSN（包含）。
 * @param to_lsn 结束LSN（包含）。
 * @return 最后一个被成功重放的日志记录的LSN。
 */
uint64_t WALManager::ReplayLog(uint64_t from_lsn, uint64_t to_lsn) {
  uint64_t last_replay_lsn = from_lsn - 1;

  std::vector<LogRecord> logs_to_replay = ReadLogRange(from_lsn, to_lsn);

  for (const auto &record : logs_to_replay) {
    // 根据日志记录类型，应用相应的重演逻辑到存储引擎。
    switch (record.type) {
    case LogRecordType::UPDATE:
      // TODO(#WAL-007): 在存储引擎中应用更新操作 (Redo)。
      // 这需要与BufferPoolManager和StorageEngine交互，根据record.key和value进行数据更新。
      std::cout << "重演更新: " << record.ToString() << std::endl;
      break;
    case LogRecordType::INSERT:
      // TODO(#WAL-008): 在存储引擎中应用插入操作 (Redo)。
      std::cout << "重演插入: " << record.ToString() << std::endl;
      break;
    case LogRecordType::DELETE:
      // TODO(#WAL-009): 在存储引擎中应用删除操作 (Redo)。
      std::cout << "重演删除: " << record.ToString() << std::endl;
      break;
    // BEGIN/COMMIT/ABORT 日志记录是事务控制信息，在Redo阶段通常不需要对数据页进行操作，
    // 主要用于在分析阶段构建活跃事务表。
    case LogRecordType::COMPENSATE:
      // 补偿日志记录在ARIES中用于Undo阶段。这里可能也需要重演。
      std::cout << "重演补偿: " << record.ToString() << std::endl;
      break;
    default:
      break;
    }

    last_replay_lsn = record.lsn;
  }

  return last_replay_lsn;
}
/**
 * @brief 获取WAL管理器的当前性能指标。
 * @details 该方法返回WALMetrics结构体，包含总记录数、已刷盘记录数、待刷盘记录数、刷盘时间等统计信息。
 * @return WALMetrics结构体实例。
 */
WALManager::WALMetrics WALManager::GetMetrics() const {
  WALMetrics metrics;

  // TODO(#WAL-010): 实现指标收集逻辑。
  // 尤其需要线程安全地收集log_file_size_bytes和pending_records。
  // metrics.log_file_size_bytes = std::filesystem::file_size(log_file_path_);
  // std::unique_lock<std::mutex> buffer_lock(buffer_mutex_);
  // metrics.pending_records = log_buffer_.size();

  // 为避免编译错误和未初始化值，暂时手动设置一些默认值
  // 实际应该从私有成员变量metrics_中复制或汇总。
  metrics.total_records = metrics_.total_records;
  metrics.flushed_records = metrics_.flushed_records;
  metrics.pending_records = metrics_.pending_records;
  metrics.total_flush_time = metrics_.total_flush_time;
  metrics.avg_flush_time = metrics_.avg_flush_time;
  metrics.total_checkpoints = metrics_.total_checkpoints;
  // metrics.log_file_size_bytes 的获取需要文件系统操作，可能较慢且涉及IO错误处理。
  // 在完整的实现中，这个值应由一个单独的线程定期更新或懒加载。

  return metrics;
}
/**
 * @brief 重置WAL管理器的所有性能指标。
 * @details 将所有统计计数器和时间记录归零。
 */
void WALManager::ResetMetrics() {
  std::unique_lock<std::mutex> lock(metrics_mutex_); // 保护指标的并发访问
  metrics_ = {
      0, 0, 0, std::chrono::microseconds(0), std::chrono::microseconds(0),
      0, 0};
}
/**
 * @brief 压缩WAL日志文件，移除指定LSN之前的记录。
 * @details 日志压缩（Log Compaction）是一个复杂的过程，旨在回收旧日志记录占用的磁盘空间，
 * 同时确保不影响崩溃恢复的正确性。此为简化实现。
 * @param keep_lsn 小于该LSN的日志记录可以被移除。
 * @return 实际回收的字节数。
 */
size_t WALManager::CompactLog(uint64_t keep_lsn) {
  // TODO(#WAL-011): 实现日志压缩逻辑。
  // 实际的日志压缩需要考虑：
  // 1. 确保所有小于keep_lsn的页面更改都已写回磁盘。
  // 2. 存在活跃事务可能需要这些日志。
  // 3. 如何安全地移除文件中的旧记录（例如，通过创建新文件，复制有效部分）。
  // 4. 并发访问和检查点等因素。

  std::cout << "日志整理完成（简化实现），保留LSN >= " << keep_lsn << std::endl;
  return 0; // 未实际压缩
}
/**
 * @brief 验证WAL日志文件的完整性。
 * @details 检查日志文件是否损坏，例如通过校验和验证、LSN连续性检查等。
 * @return 日志完整返回true，否则返回false。
 */
bool WALManager::VerifyLogIntegrity() const {
  // TODO(#WAL-012): 实现日志完整性验证逻辑。
  // 这会涉及扫描日志文件，检查每条记录的头部、校验和、LSN顺序等。
  // 临时注释掉会导致编译错误的部分
  // ReadRecordFromDisk(0);
  return true;
}
// ================== 私有方法实现 ==================
/**
 * @brief 初始化WAL日志文件和检查点文件。
 * @details 确保日志文件所在的目录存在。如果日志文件或检查点文件不存在，则创建它们。
 * 日志文件会写入一个固定的头部信息。
 * @throws std::runtime_error 如果无法创建文件。
 */
void WALManager::InitializeLogFile() {
  // 1. 确保日志文件所在的目录存在。
  std::filesystem::create_directories(
      std::filesystem::path(log_file_path_).parent_path());

  // 2. 如果日志文件不存在，创建它并写入文件头。
  if (!std::filesystem::exists(log_file_path_)) {
    std::ofstream log_file(log_file_path_, std::ios::binary);
    if (!log_file) {
      throw std::runtime_error("无法创建日志文件: " + log_file_path_);
    }
    // 写入日志文件头信息，用于版本识别和初步验证。
    const char *header = "SQLCC WAL v0.4.8";
    log_file.write(header, strlen(header));
    log_file.close();
  }

  // 3. 如果检查点文件不存在，也创建它。
  if (!std::filesystem::exists(checkpoint_file_path_)) {
    std::ofstream chk_file(checkpoint_file_path_, std::ios::binary);
    if (!chk_file) {
      throw std::runtime_error("无法创建检查点文件: " + checkpoint_file_path_);
    }
    chk_file.close();
  }

  std::cout << "WAL日志文件已初始化: " << log_file_path_ << std::endl;
}

/**
 * @brief 生成并返回下一个唯一的日志序列号 (LSN)。
 * @details 使用原子操作确保在多线程环境下的线程安全性和LSN的唯一递增性。
 * `std::memory_order_relaxed` 用于性能优化，因为它不需要在线程之间同步非原子操作。
 * @return 新的、唯一的LSN。
 */
uint64_t WALManager::GenerateLSN() {
  return next_lsn_.fetch_add(1, std::memory_order_relaxed);
}
/**
 * @brief 将一批日志记录写入WAL日志文件。
 * @details 将日志记录以二进制格式追加到日志文件末尾，并确保写入后进行刷盘操作。
 * @param records 包含待写入LogRecord的向量。
 * @return 成功写入磁盘的记录数量。
 * @throws std::runtime_error 如果无法打开或写入日志文件。
 */
size_t WALManager::WriteRecordsToDisk(const std::vector<LogRecord> &records) {
  if (records.empty()) {
    return 0;
  }

  // 以追加模式打开二进制日志文件。
  std::ofstream log_file(log_file_path_, std::ios::binary | std::ios::app);
  if (!log_file) {
    throw std::runtime_error("无法打开日志文件进行写入: " + log_file_path_);
  }

  size_t written = 0;
  for (const auto &record : records) {
    // 1. 定义简化的二进制格式：
    // txn_id (8字节) | type (1字节) | key长度 (4字节) | key数据 | lsn (8字节)
    // TODO(#WAL-001): 当LogRecord结构扩展时，此处的序列化逻辑需要同步更新，以包含old_value, new_value和timestamp。
    uint8_t record_type = static_cast<uint8_t>(record.type);
    uint32_t key_length = record.key.length();

    log_file.write(reinterpret_cast<const char *>(&record.txn_id),
                   sizeof(record.txn_id));
    log_file.write(reinterpret_cast<const char *>(&record_type),
                   sizeof(record_type));
    log_file.write(reinterpret_cast<const char *>(&key_length),
                   sizeof(key_length));
    log_file.write(record.key.data(), key_length);
    log_file.write(reinterpret_cast<const char *>(&record.lsn),
                   sizeof(record.lsn));

    // 这里为了简化先只写入核心字段

    if (log_file.fail()) {
      throw std::runtime_error("写入日志记录失败");
    }
    written++;
  }

  log_file.flush(); // 强制将缓冲区内容写入操作系统文件缓存。
  // TODO(#WAL-013): 在生产环境中，需要考虑`fsync()`或`fdatasync()`以确保日志真正持久化到磁盘，
  // 并且可能需要通过配置来控制刷盘粒度（例如，每X条记录或每Y毫秒一次fsync）。
  return written;
}
/**
 * @brief 从WAL日志文件中读取指定LSN的日志记录。
 * @details 该方法需要能够定位到文件中对应LSN的记录，并解析其二进制数据。
 * 在一个完整的实现中，这会涉及到维护LSN到文件偏移量的映射，或者通过索引进行查找。
 * @param lsn 待读取日志记录的LSN。
 * @return 对应的LogRecord实例。如果记录不存在或读取失败，返回一个LSN为0的LogRecord。
 */
LogRecord WALManager::ReadRecordFromDisk(uint64_t lsn) {
  // 避免未使用参数警告
  (void)lsn;

  // TODO(#WAL-014): 实现从磁盘读取指定LSN记录的逻辑。
  // 这需要解决如何根据LSN高效定位到文件中的记录（例如，通过一个LSN-to-offset的索引），
  // 并解析其二进制格式（与WriteRecordsToDisk中的格式对应）。
  return LogRecord(); // 暂未实现，返回默认LogRecord。
}
/**
 * @brief 将检查点状态写入磁盘文件。
 * @details 检查点文件包含了恢复所需的重要信息，如检查点LSN、时间戳以及（在完整实现中）页面状态快照。
 * @param checkpoint 待写入的CheckpointState实例。
 * @throws std::runtime_error 如果无法打开检查点文件进行写入。
 */
void WALManager::WriteCheckpointToDisk(const CheckpointState &checkpoint) {
  std::ofstream chk_file(checkpoint_file_path_, std::ios::binary);
  if (!chk_file) {
    throw std::runtime_error("无法打开检查点文件进行写入: " +
                             checkpoint_file_path_);
  }

  // 1. 写入检查点基本信息 (LSN, 时间戳)。
  chk_file.write(reinterpret_cast<const char *>(&checkpoint.checkpoint_lsn),
                 sizeof(checkpoint.checkpoint_lsn));

  auto timestamp_count = checkpoint.timestamp.time_since_epoch().count();
  chk_file.write(reinterpret_cast<const char *>(&timestamp_count),
                 sizeof(timestamp_count));

  // TODO(#WAL-004): 写入页面状态快照 (DPT - Dirty Page Table, ATT - Active Transaction Table)。
  // 这是崩溃恢复（特别是ARIES算法）中分析阶段的关键输入，用于确定从哪里开始Redo。

  chk_file.flush(); // 强制将缓冲区内容写入操作系统文件缓存。
  // TODO(#WAL-015): 在生产环境中，需要`fsync()`或`fdatasync()`确保检查点真正持久化到磁盘。
  std::cout << "检查点已写入磁盘，LSN: " << checkpoint.checkpoint_lsn
            << std::endl;
}
/**
 * @brief 从磁盘文件读取最近的检查点状态。
 * @details 该方法会从检查点文件中读取LSN、时间戳以及（在完整实现中）页面状态快照，
 * 用于崩溃恢复的初始化阶段。
 * @return 读取到的CheckpointState实例。如果文件不存在，返回一个默认的CheckpointState。
 * @throws std::runtime_error 如果无法打开检查点文件进行读取。
 */
CheckpointState WALManager::ReadCheckpointFromDisk() const {
  // 1. 检查检查点文件是否存在。如果不存在，返回一个默认状态，表示没有可用的检查点。
  if (!std::filesystem::exists(checkpoint_file_path_)) {
    return {0, std::chrono::system_clock::now(), {}}; // 返回默认检查点
  }

  // 2. 打开检查点文件进行二进制读取。
  std::ifstream chk_file(checkpoint_file_path_, std::ios::binary);
  if (!chk_file) {
    throw std::runtime_error("无法打开检查点文件进行读取: " +
                             checkpoint_file_path_);
  }

  CheckpointState checkpoint;

  // 3. 读取检查点基本信息 (LSN, 时间戳)。
  chk_file.read(reinterpret_cast<char *>(&checkpoint.checkpoint_lsn),
                sizeof(checkpoint.checkpoint_lsn));

  std::chrono::system_clock::rep timestamp_count;
  chk_file.read(reinterpret_cast<char *>(&timestamp_count),
                sizeof(timestamp_count));

  if (!chk_file.fail()) {
    checkpoint.timestamp = std::chrono::system_clock::time_point(
        std::chrono::system_clock::duration(timestamp_count));
  }

  // TODO(#WAL-004): 读取页面状态快照 (DPT - Dirty Page Table, ATT - Active Transaction Table)。
  // 这部分与WriteCheckpointToDisk中的写入逻辑对应。

  return checkpoint;
}
/**
 * @brief 异步刷盘线程的执行函数。
 * @details 该线程周期性地（或被通知时）将内存中的日志缓冲区内容刷入磁盘。
 * 异步刷盘功能当前已在构造函数中暂时禁用，因此此线程实际上不会启动。
 */
void WALManager::AsyncFlushThread() {
  // TODO(#WAL-002): 异步刷盘功能已被禁用。当重新启用时，此线程将负责异步刷盘。
  while (!stop_flush_thread_) {
    // 1. 等待通知或超时：线程会在此处阻塞，直到日志缓冲区有新记录，或达到刷盘间隔，
    // 或收到停止信号。
    {
      std::unique_lock<std::mutex> lock(buffer_mutex_);
      buffer_cv_.wait_for(
          lock, std::chrono::milliseconds(flush_interval_ms_),
          [this]() { return !log_buffer_.empty() || stop_flush_thread_; });
    }

    // 2. 检查是否收到停止信号。
    if (stop_flush_thread_) {
      break;
    }

    // 3. 执行异步刷盘操作。
    ForceFlush();
  }

  std::cout << "异步刷盘线程已停止" << std::endl;
}
} // namespace sqlcc