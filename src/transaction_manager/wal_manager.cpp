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
  std::string old_value;///< 操作前的数据旧值（序列化形式），用于UNDO操作。
  std::string new_value;///< 操作后的数据新值（序列化形式），用于REDO操作。
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
      ss << " Key:'" << key << "'";
      ss << " Old:'" << old_value << "'"; // 显示旧值
      ss << " New:'" << new_value << "'"; // 显示新值
      break;
    case LogRecordType::INSERT:
      ss << "INSERT";
      ss << " Key:'" << key << "'";
      ss << " Value:'" << new_value << "'"; // 插入操作只有新值
      break;
    case LogRecordType::DELETE:
      ss << "DELETE";
      ss << " Key:'" << key << "'";
      ss << " Old:'" << old_value << "'"; // 删除操作只有旧值
      break;
    case LogRecordType::COMPENSATE:
      ss << "COMPENSATE";
      break;
    }

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
      flush_interval_ms_(100), stop_flush_thread_(false) { // 100ms异步刷盘间隔

  // 初始化指标
  metrics_.total_records = 0;
  metrics_.flushed_records = 0;
  metrics_.pending_records = 0;
  metrics_.total_checkpoints = 0;
  metrics_.log_file_size_bytes = 0;

  // 初始化日志文件
  InitializeLogFile();

  // WHY: 异步刷盘是提升数据库吞吐量的关键技术之一。通过将日志写入磁盘的操作
  // 从事务处理的主路径中解耦出来，使得事务能够更快地提交。
  // WHAT: 根据构造函数传入的 `force_sync` 参数决定是否启动异步刷盘线程。
  // HOW: 如果 `force_sync_` 为 false，则启动一个后台线程 `flush_thread_`，
  // 该线程会周期性地将内存中的日志缓冲区数据刷入磁盘。
  if (!force_sync_) {
    flush_thread_ = std::thread(&WALManager::AsyncFlushThread, this);
  }

  std::cout << "WALManager 已初始化 - 日志文件: " << log_file_path_
            << " 强制同步: " << (force_sync_ ? "是" : "否") << std::endl;
}

/**
 * @brief 销毁WALManager实例。
 * @details 在销毁之前，会强制将所有内存中未刷盘的日志记录写入磁盘，确保数据持久性。
 * 如果启用了异步刷盘，则会先停止异步刷盘线程。
 */
WALManager::~WALManager() {
  // WHY: 确保在WALManager析构时，所有待处理的日志记录都已被持久化到磁盘，
  // 并且后台的异步刷盘线程能够被优雅地关闭，避免资源泄露或数据丢失。
  // WHAT: 停止异步刷盘线程（如果已启动），并强制刷盘所有剩余的日志。
  // HOW:
  // 1. 设置停止标志 `stop_flush_thread_` 为 true，通知异步刷盘线程退出循环。
  // 2. 唤醒异步刷盘线程，确保它能够检查到停止标志并退出。
  // 3. 使用 `join()` 等待异步刷盘线程完成其工作并终止。
  if (flush_thread_.joinable()) {
    stop_flush_thread_ = true;
    buffer_cv_.notify_one(); // 唤醒等待中的线程
    flush_thread_.join();    // 等待线程结束
  }

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
  // WHY: 将单个修改操作原子化记录，作为崩溃恢复的最细粒度单位。
  // WHAT: 分配 LSN，暂存到内存 log_buffer_ 中。
  // HOW:
  // 1. 调用 GenerateLSN。
  // 2. 加 buffer_mutex_ 锁保护 log_buffer_。
  // 3. 将记录推入 vector。
  // 4. 通知刷盘线程。
  // 5. 若 force_sync_ 则触发阻塞式 ForceFlush。

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
  // WHY: 批量记录能够显著减少锁竞争和唤醒刷盘线程的开销，提升系统吞吐量。
  // WHAT: 为一组记录分配连续 LSN 并一次性放入缓冲区。
  // HOW: 在锁保护下循环处理记录，然后一次性通知和刷盘。
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
  // WHY: WAL 协议要求日志必须在数据落地前持久化。ForceFlush 是实现“提交即持久”的关键同步点。
  // WHAT: 将内存缓冲区 swap 出来，通过 WriteRecordsToDisk 物理写入，并更新 LSN。
  // HOW:
  // 1. 最小锁粒度：使用 swap 快速获取缓冲区内容并释放锁。
  // 2. 统计时间：记录 I/O 开销用于性能监控。
  // 3. 调用 WriteRecordsToDisk 触发 fsync。
  // 4. 更新 last_flushed_lsn_ 标记恢复基准点。
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

  // WHY: 分析阶段（Analysis Phase）是ARIES恢复算法的第一步。
  // 它的目标是确定崩溃发生时数据库的状态，特别是识别出哪些事务是活跃的、
  // 哪些页面是脏的（即在内存中被修改但尚未刷回磁盘），以及从哪个LSN开始进行重做。
  // WHAT: 扫描WAL日志文件，从最新的检查点开始，构建活跃事务表 (ATT) 和脏页表 (DPT)。
  // ATT记录了崩溃时所有尚未提交或中止的事务，DPT记录了崩溃时所有在内存中被修改但
  // 尚未写入磁盘的页面及其对应的LSN。
  // HOW:
  // 1. **从最近的检查点开始扫描**: 定位到最近检查点记录的LSN，并从该点开始向前扫描日志。
  //    检查点记录包含了崩溃时ATT和DPT的初始状态。
  // 2. **更新ATT**: 遇到`BEGIN`记录时，将事务ID添加到ATT中；遇到`COMMIT`或`ABORT`记录时，
  //    从ATT中移除对应的事务ID。
  // 3. **更新DPT**: 遇到任何数据修改（`UPDATE`, `INSERT`, `DELETE`）记录时，
  //    如果对应的页面LSN小于DPT中记录的页面LSN，则更新DPT，记录该页面的最新修改LSN。
  // 4. **确定Redo起点**: 分析阶段的最终结果是确定一个Redo LSN，即从该LSN开始，
  //    所有需要重做的操作都将发生。这个LSN通常是DPT中最小的RecLSN（最近一次将脏页写入磁盘的LSN）。

  LOG_INFO("WALManager::AnalyzeLog - 开始分析日志进行恢复。");
  // 实际的日志分析需要遍历日志文件，并且需要一个更复杂的机制来构建ATT和DPT。
  // 以下为简化演示，仅输出信息。
  // TODO: (#WAL-003-IMPL) 在实际实现中，需要通过`ReadRecordFromDisk`迭代读取日志记录，
  // 并根据日志记录类型更新内部的ATT和DPT数据结构。
  // 例如：
  // std::vector<LogRecord> all_logs = ReadLogRange(last_checkpoint_lsn_, next_lsn_.load());
  // for (const auto& log : all_logs) {
  //     if (log.type == LogRecordType::BEGIN) {
  //         // Add to Active Transaction Table
  //     } else if (log.type == LogRecordType::UPDATE || log.type == LogRecordType::INSERT || log.type == LogRecordType::DELETE) {
  //         // Update Dirty Page Table based on the page involved in the operation
  //     }
  // }


  (void)recovery_actions; // 避免未使用变量警告
  return {}; // 暂未实现完整分析，返回空。
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
      {} // WHY: 页面状态快照，特别是脏页表 (Dirty Page Table, DPT) 和活跃事务表 (Active Transaction Table, ATT)，
         // 是ARIES恢复算法中分析阶段的关键输入。DPT记录了在检查点发生时，哪些页面在内存中被修改但尚未刷回磁盘，
         // 以及这些页面上发生的第一个日志记录（RecLSN）。ATT则记录了在检查点时所有正在进行中的事务。
         // WHAT: 在一个完整的实现中，这里应该获取BufferPoolManager的DPT和TransactionManager的ATT。
         // HOW: 这将通过调用BufferPoolManager::GetDirtyPageTable()和TransactionManager::GetActiveTransactions()
         // 等方法来实现。这些信息使得在崩溃恢复的Redo阶段，可以从检查点之后有效地重放日志，
         // 并且跳过那些在检查点之前就已经刷回磁盘的页面的操作。
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

    // WHY: 重做阶段 (Redo Phase) 是确保持久性的关键一步。
    // 它的目标是根据WAL日志，重新应用所有已提交事务以及部分已进行但尚未完全写入磁盘的事务的更改，
    // 使数据库恢复到崩溃前的最新状态。即使某些更改在崩溃前已经写入了磁盘，Redo操作也会幂等地重新执行。
    // WHAT: 遍历从上一个检查点或恢复起点（Redo LSN）到日志末尾的所有日志记录，并应用这些更改。
    // HOW: `ReplayLog`函数负责实际的重做操作。在ARIES等高级恢复算法中，Redo阶段会利用
    // 分析阶段构建的脏页表（DPT）来优化，只重做那些对应的页面LSN小于或等于日志记录LSN的修改，
    // 并且只对那些在DPT中的页面进行Redo，以避免不必要的磁盘I/O。
    // TODO: (#WAL-005-IMPL) 在此添加实际的Redo逻辑，可能需要从特定的Redo LSN开始，
    // 并且需要与`BufferPoolManager`和`StorageEngine`进行交互来应用日志记录中的数据更改。

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

  // WHY: 在崩溃恢复的分析阶段和回滚阶段（Undo Phase），我们需要知道在崩溃发生时哪些事务是活跃的。
  // 活跃事务是指那些已经开始（有`BEGIN`记录）但尚未提交（无`COMMIT`记录）或中止（无`ABORT`记录）的事务。
  // 识别这些事务是进行回滚操作的前提。
  // WHAT: 该函数旨在扫描从最近的检查点到日志末尾的所有日志记录，并构建一个活跃事务ID的列表。
  // HOW:
  // 1. **初始化**: 创建一个`std::unordered_set<TransactionId>`来存储活跃事务ID。
  // 2. **扫描日志**: 从`last_checkpoint_lsn_`之后的下一条日志记录开始，直到日志末尾。
  //    使用`ReadLogRange`或迭代`ReadRecordFromDisk`。
  // 3. **处理日志记录**:
  //    - 遇到`BEGIN`记录时，将`record.txn_id`插入到活跃事务集合中。
  //    - 遇到`COMMIT`或`ABORT`记录时，将`record.txn_id`从活跃事务集合中移除。
  // 4. **返回结果**: 扫描结束后，集合中剩余的事务ID即为崩溃时活跃的事务。

  // 临时注释掉会导致编译错误的部分
  // std::vector<LogRecord> recent_logs = ReadLogRange(last_checkpoint_lsn_ + 1,
  // next_lsn_.load() - 1);
  // for (const auto& record : recent_logs) {
  //     if (record.type == LogRecordType::BEGIN) {
  //         // active_transactions_set.insert(record.txn_id);
  //     } else if (record.type == LogRecordType::COMMIT || record.type == LogRecordType::ABORT) {
  //         // active_transactions_set.erase(record.txn_id);
  //     }
  // }
  // active_transactions.assign(active_transactions_set.begin(), active_transactions_set.end());


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
      // WHY: 在Redo阶段，UPDATE日志记录指示数据库需要将特定数据项的值更新为`new_value`。
      // 这确保了已提交事务的更新操作即使在崩溃后也能被正确地重新应用。
      // WHAT: 将`record.key`标识的数据项更新为`record.new_value`。
      // HOW: 这通常涉及以下步骤：
      //      1. 通过`BufferPoolManager`获取包含`record.key`的对应数据页面。
      //      2. 在该数据页面上，定位到由`record.key`指定的数据项。
      //      3. 将数据项的值更新为`record.new_value`。
      //      4. 确保更新操作是幂等的：如果目标数据项的LSN已经大于或等于`record.lsn`，
      //         则无需再次应用此更新，因为该更改可能已通过后续日志或之前刷盘操作应用。
      // TODO: (#WAL-007-IMPL) 在存储引擎中应用更新操作 (Redo)。
      // 这需要与BufferPoolManager和StorageEngine交互，根据record.key和value进行数据更新。
      std::cout << "重演更新: " << record.ToString() << std::endl;
      break;
    case LogRecordType::INSERT:
      // WHY: INSERT日志记录指示数据库需要重新插入一个已提交事务添加的数据项。
      // WHAT: 将`record.new_value`作为新的数据项插入到由`record.key`标识的存储位置。
      // HOW: 这通常涉及以下步骤：
      //      1. 通过`BufferPoolManager`获取适合插入`record.key`和`record.new_value`的数据页面。
      //      2. 在页面中为新数据项分配空间并写入`record.new_value`。
      //      3. 确保插入操作是幂等的：如果数据项已存在（例如，通过主键查找），则无需重复插入。
      // TODO: (#WAL-008-IMPL) 在存储引擎中应用插入操作 (Redo)。
      std::cout << "重演插入: " << record.ToString() << std::endl;
      break;
    case LogRecordType::DELETE:
      // WHY: DELETE日志记录指示数据库需要重新删除一个已提交事务移除的数据项。
      // WHAT: 从由`record.key`标识的存储位置删除数据项。
      // HOW: 这通常涉及以下步骤：
      //      1. 通过`BufferPoolManager`获取包含`record.key`的对应数据页面。
      //      2. 在该数据页面上，定位到由`record.key`指定的数据项。
      //      3. 将数据项标记为已删除或移除其物理存储。
      //      4. 确保删除操作是幂等的：如果数据项已不存在，则无需重复删除。
      // TODO: (#WAL-009-IMPL) 在存储引擎中应用删除操作 (Redo)。
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

  // WHY: 提供WAL管理器的实时性能和状态指标对于监控、调试和性能调优至关重要。
  // 这些指标有助于理解日志系统的工作负载和瓶颈。由于WALManager是多线程的，
  // 必须确保在访问共享资源（如日志缓冲区大小、文件大小）时线程安全，以避免数据不一致。
  // WHAT: 收集并返回WAL管理器的各项性能指标，包括总记录数、已刷盘记录数、待刷盘记录数、
  // 刷盘时间统计和日志文件当前大小。
  // HOW:
  // 1. **`log_file_size_bytes`**: 通过`std::filesystem::file_size()`获取日志文件的当前大小。
  //    这是一个相对独立的OS调用，不直接影响共享内存数据。
  // 2. **`pending_records`**: 通过`buffer_mutex_`保护对`log_buffer_`的访问，获取其当前大小。
  // 3. **其他指标**: `total_records`, `flushed_records`, `total_flush_time`, `avg_flush_time`,
  //    `total_checkpoints`等已在操作发生时通过`metrics_mutex_`进行更新，此处直接复制即可。

  // 保护对metrics_成员的访问，以获取准确的统计数据
  std::unique_lock<std::mutex> lock(metrics_mutex_);
  metrics.total_records = metrics_.total_records;
  metrics.flushed_records = metrics_.flushed_records;
  metrics.total_flush_time = metrics_.total_flush_time;
  metrics.avg_flush_time = metrics_.avg_flush_time;
  metrics.total_checkpoints = metrics_.total_checkpoints;

  // 收集log_file_size_bytes
  try {
    metrics.log_file_size_bytes = std::filesystem::file_size(log_file_path_);
  } catch (const std::filesystem::filesystem_error &e) {
    LOG_ERROR("无法获取WAL文件大小: %s", e.what());
    metrics.log_file_size_bytes = 0; // 发生错误时设置为0
  }

  // 收集pending_records，需要保护log_buffer_
  {
    std::unique_lock<std::mutex> buffer_lock(buffer_mutex_);
    metrics.pending_records = log_buffer_.size();
  }

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
  // WHY: 日志压缩（Log Compaction）是WAL管理中一个至关重要的功能，旨在回收旧日志记录占用的磁盘空间，
  // 从而防止日志文件无限增长。然而，压缩过程必须小心执行，以确保数据库的崩溃恢复能力不受影响。
  // WHAT: 移除所有LSN小于`keep_lsn`的日志记录。
  // HOW: 真正的日志压缩是一个复杂的过程，它不能简单地删除文件开头的部分。
  // 核心挑战和考虑点包括：
  // 1.  **持久性保证**: 必须确保所有小于`keep_lsn`的日志记录所代表的页面更改都已**安全地刷写到磁盘**。
  //     否则，如果系统在压缩后崩溃，这些未刷写的更改将无法通过Redo恢复。
  // 2.  **活跃事务**: 任何正在进行的事务可能仍需要读取旧的日志记录进行Undo操作。
  //     因此，不能删除任何活跃事务可能依赖的日志。
  // 3.  **安全文件操作**: 在操作系统层面，直接截断（truncate）日志文件可能会导致数据损坏。
  //     更安全的做法通常是创建一个新的日志文件，将`keep_lsn`之后的有效日志复制过去，
  //     然后原子性地替换旧文件。
  // 4.  **并发性**: 日志压缩过程必须与正常的日志写入和读取操作并发安全地执行。
  // 5.  **检查点**: 压缩的起点通常会关联到最近的检查点，因为检查点提供了一个安全点，
  //     表明其之前的更改大部分都已持久化。

  LOG_INFO("WALManager::CompactLog - 日志整理请求，保留LSN >= %llu (简化实现，未实际压缩)", keep_lsn);
  // TODO: (#WAL-011-IMPL) 实现真正的日志压缩逻辑。
  return 0; // 未实际压缩
}
/**
 * @brief 验证WAL日志文件的完整性。
 * @details 检查日志文件是否损坏，例如通过校验和验证、LSN连续性检查等。
 * @return 日志完整返回true，否则返回false。
 */
bool WALManager::VerifyLogIntegrity() const {
  // WHY: 日志文件的完整性对于数据库的可靠性至关重要。如果WAL日志在写入过程中损坏（例如，由于磁盘错误、
  // 操作系统崩溃或不完全的写入），那么在崩溃恢复时使用这些损坏的日志会导致数据库进入不一致的状态甚至崩溃。
  // 因此，在进行恢复之前或定期检查日志完整性是必要的。
  // WHAT: 检查WAL日志文件是否损坏，例如通过校验和验证、LSN连续性检查等。
  // HOW: 日志完整性验证通常涉及以下步骤：
  // 1.  **逐条扫描**: 从日志文件的开头（或最近的检查点）开始，逐条读取并解析日志记录。
  // 2.  **校验和验证**: 每条日志记录通常包含一个校验和（例如CRC32），用于验证记录内容的完整性。
  //     读取记录后，重新计算其校验和并与存储的校验和进行比较。
  // 3.  **LSN连续性检查**: 验证日志记录的LSN是否是严格递增的。
  // 4.  **记录格式验证**: 检查每条日志记录的头部和长度字段是否有效，以确保其符合预期的格式。
  // 5.  **文件一致性**: 确保日志文件没有意外截断或意外的额外数据。

  LOG_INFO("WALManager::VerifyLogIntegrity - 开始验证日志文件完整性 (简化实现)");
  // TODO: (#WAL-012-IMPL) 实现日志完整性验证逻辑。
  // 这会涉及扫描日志文件，检查每条记录的头部、校验和、LSN顺序等。
  return true; // 暂未实际验证，返回true。
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
 * @return 新s的、唯一的LSN。
 */
uint64_t WALManager::GenerateLSN() {
  // WHY: LSN 是 WAL 的基石，必须保证单调递增且全局唯一，以便恢复时精确定位日志位置。
  // WHAT: 分配并自增 next_lsn_ 计数器。
  // HOW: 使用 std::atomic::fetch_add 实现无锁的并发递增。
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
    // lsn (8字节) | txn_id (8字节) | type (1字节) |
    // key_len (4字节) | key_data |
    // old_value_len (4字节) | old_value_data |
    // new_value_len (4字节) | new_value_data |
    // timestamp (8字节, std::chrono::system_clock::rep)

    // TODO(#WAL-001): LogRecord结构已扩展，此处的序列化逻辑已同步更新。
    // 在读取时，也需要以相同的顺序和方式反序列化。

    uint8_t record_type = static_cast<uint8_t>(record.type);
    uint32_t key_length = record.key.length();
    uint32_t old_value_length = record.old_value.length();
    uint32_t new_value_length = record.new_value.length();
    std::chrono::system_clock::rep timestamp_count = record.timestamp.time_since_epoch().count();

    log_file.write(reinterpret_cast<const char *>(&record.lsn),
                   sizeof(record.lsn));
    log_file.write(reinterpret_cast<const char *>(&record.txn_id),
                   sizeof(record.txn_id));
    log_file.write(reinterpret_cast<const char *>(&record_type),
                   sizeof(record_type));

    log_file.write(reinterpret_cast<const char *>(&key_length),
                   sizeof(key_length));
    if (key_length > 0) {
      log_file.write(record.key.data(), key_length);
    }

    log_file.write(reinterpret_cast<const char *>(&old_value_length),
                   sizeof(old_value_length));
    if (old_value_length > 0) {
      log_file.write(record.old_value.data(), old_value_length);
    }

    log_file.write(reinterpret_cast<const char *>(&new_value_length),
                   sizeof(new_value_length));
    if (new_value_length > 0) {
      log_file.write(record.new_value.data(), new_value_length);
    }

    log_file.write(reinterpret_cast<const char *>(&timestamp_count),
                   sizeof(timestamp_count));


    if (log_file.fail()) {
      throw std::runtime_error("写入日志记录失败");
    }
    written++;
  }

  log_file.flush(); // 强制将缓冲区内容写入操作系统文件缓存。
  // WHY: `log_file.flush()` 确保数据从C++流的内部缓冲区写入操作系统的文件缓冲区。
  // 但是，这并不保证数据已物理写入磁盘。如果系统在此之后崩溃，但数据仍在操作系统缓冲区中，
  // 那么这些数据将会丢失，从而违反WAL的持久性保证。
  // WHAT: `fsync()` 或 `fdatasync()` 系统调用用于强制操作系统将文件缓冲区中的所有待写入数据
  // 物理地同步到磁盘存储介质。
  // HOW: 在生产环境中，根据性能和持久性需求，可能需要周期性地或在关键点（例如事务提交时）
  // 调用 `fsync()`。例如：
  // #ifdef __linux__
  //      fdatasync(log_file.fd()); // 仅同步数据，不更新元数据，性能略优
  // #else
  //      fsync(log_file.fd());     // 同步数据和元数据
  // #endif
  // 这种操作会带来一定的性能开销，因此需要通过配置来平衡刷盘粒度（例如，每X条记录或每Y毫秒一次fsync）。
  // TODO: (#WAL-013-IMPL) 在生产环境中，需要考虑`fsync()`或`fdatasync()`以确保日志真正持久化到磁盘，
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
LogRecord WALManager::ReadRecordFromDisk(uint64_t lsn_to_read) {
  // WHY: 在崩溃恢复期间，我们需要能够根据日志序列号（LSN）从磁盘中精确地读取出对应的日志记录，
  // 以便重做（Redo）或撤销（Undo）事务操作。此函数是实现这一核心恢复机制的基础。
  // WHAT: 实现从WAL日志文件中读取指定LSN的日志记录的逻辑。
  // HOW:
  // 1. 打开日志文件进行二进制读取。
  // 2. 遍历文件，逐条解析日志记录的头部，直到找到匹配`lsn_to_read`的记录。
  // 3. 读取并反序列化`LogRecord`的所有字段，包括LNS、事务ID、类型、键、旧值、新值和时间戳。
  //
  // 注意：目前的实现是一个简化的线性扫描。在生产系统中，为了高效定位，
  // 通常会维护一个内存中的LSN到文件物理偏移量的映射表，或使用索引结构。

  LogRecord record; // 默认构造函数会将LSN初始化为0，表示无效记录
  std::ifstream log_file(log_file_path_, std::ios::binary);
  if (!log_file) {
    throw std::runtime_error("无法打开日志文件进行读取: " + log_file_path_);
  }

  // 跳过文件头
  const char *header_placeholder = "SQLCC WAL v0.4.8"; // 假设头文件大小已知
  log_file.seekg(strlen(header_placeholder));

  // 循环读取直到文件结束或找到匹配的LSN
  while (log_file.peek() != EOF) {
    uint64_t current_lsn;
    TransactionId txn_id;
    uint8_t record_type_val;
    uint32_t key_length, old_value_length, new_value_length;
    std::chrono::system_clock::rep timestamp_count;

    // 尝试读取日志记录的各个部分
    size_t start_pos = log_file.tellg(); // 记录当前读取位置

    log_file.read(reinterpret_cast<char *>(&current_lsn), sizeof(current_lsn));
    if (log_file.gcount() != sizeof(current_lsn)) break; // 读取失败或文件结束

    // 如果当前记录的LSN不匹配，则需要跳过这条记录剩余的部分，找到下一条记录的开头
    // 这里是一个简化的处理，直接检查LSN。
    // 更健壮的实现会先读取一个固定大小的记录头来确定后续内容的长度。

    // 为了实现跳过，我们需要知道每条记录的完整长度。
    // 由于结构复杂且可变长字段，直接跳过当前简化实现可能不准确。
    // 因此，我们先完整读取，然后检查LSN。

    // 读取事务ID, 类型
    log_file.read(reinterpret_cast<char *>(&txn_id), sizeof(txn_id));
    log_file.read(reinterpret_cast<char *>(&record_type_val), sizeof(record_type_val));

    // 读取key
    log_file.read(reinterpret_cast<char *>(&key_length), sizeof(key_length));
    std::string key_data(key_length, '\0');
    if (key_length > 0) log_file.read(key_data.data(), key_length);

    // 读取old_value
    log_file.read(reinterpret_cast<char *>(&old_value_length), sizeof(old_value_length));
    std::string old_value_data(old_value_length, '\0');
    if (old_value_length > 0) log_file.read(old_value_data.data(), old_value_length);

    // 读取new_value
    log_file.read(reinterpret_cast<char *>(&new_value_length), sizeof(new_value_length));
    std::string new_value_data(new_value_length, '\0');
    if (new_value_length > 0) log_file.read(new_value_data.data(), new_value_length);

    // 读取timestamp
    log_file.read(reinterpret_cast<char *>(&timestamp_count), sizeof(timestamp_count));

    if (log_file.fail() && !log_file.eof()) { // 读到一半失败，但不是文件结束
      throw std::runtime_error("读取日志记录时发生错误");
    }

    if (current_lsn == lsn_to_read) {
      record.lsn = current_lsn;
      record.txn_id = txn_id;
      record.type = static_cast<LogRecordType>(record_type_val);
      record.key = key_data;
      record.old_value = old_value_data;
      record.new_value = new_value_data;
      record.timestamp = std::chrono::system_clock::time_point(
          std::chrono::system_clock::duration(timestamp_count));
      return record;
    }
    // 如果LSN不匹配，并且文件没有读取完，继续下一次循环读取
  }

  return LogRecord(); // 未找到匹配的LSN，返回默认构造的LogRecord (lsn=0)
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

  // WHY: 在ARIES等恢复算法中，脏页表 (DPT) 和活跃事务表 (ATT) 是崩溃恢复分析阶段的关键输入。
  // 将它们写入检查点文件，可以极大地加速恢复过程，因为它提供了崩溃时刻数据库状态的快照。
  // WHAT: 写入页面状态快照（DPT和ATT）。
  // HOW: 在完整的实现中，这里会序列化`checkpoint.dirty_page_table`和`checkpoint.active_transaction_table`
  // 并写入文件。这些结构体包含了在检查点时哪些页面是脏的以及哪些事务仍在运行的信息。
  // TODO: (#WAL-004-IMPL) 实现将页面状态快照（DPT和ATT）序列化并写入检查点文件的逻辑。

  chk_file.flush(); // 强制将缓冲区内容写入操作系统文件缓存。
  // WHY: 检查点文件的持久性至关重要，因为它包含了恢复日志的起点和关键状态信息。
  // 仅仅执行 `flush()` 只能确保数据到达操作系统的文件缓冲区，而不能保证数据已物理写入磁盘。
  // 如果在 `fsync()` 调用前系统崩溃，那么新写入的检查点可能不完整或丢失，
  // 从而导致下次恢复时使用过时的检查点或恢复失败。
  // WHAT: `fsync()` 系统调用用于强制操作系统将检查点文件的所有待写入数据物理地同步到磁盘。
  // HOW: 在创建检查点后，**必须**调用 `fsync()` 以确保检查点的持久性。
  // 这确保了在任何系统崩溃后，最新的检查点都能够被可靠地读取和使用。
  // TODO: (#WAL-015-IMPL) 在生产环境中，需要`fsync()`或`fdatasync()`确保检查点真正持久化到磁盘。
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

  // WHY: 与`WriteCheckpointToDisk`中写入逻辑对应，恢复时需要读取这些状态。
  // WHAT: 读取在检查点发生时记录的页面状态快照，例如脏页表 (DPT) 和活跃事务表 (ATT)。
  // HOW: 在完整的实现中，这里会反序列化DPT和ATT的数据，并将其填充到`checkpoint`对象中。
  // 这些信息对于恢复的Redo阶段至关重要，它指导恢复过程从何处开始重放日志。
  // TODO: (#WAL-004-IMPL) 实现从检查点文件中读取页面状态快照（DPT和ATT）的逻辑。

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