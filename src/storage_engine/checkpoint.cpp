#include "storage/checkpoint.h"
#include "storage/storage_engine.h"
#include "storage/wal_writer.h"
#include "utils/config_manager.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <unordered_set>

namespace sqlcc {

CheckpointManager::CheckpointManager(ConfigManager& config_manager,
                                     StorageEngine& storage_engine,
                                     WALWriter& wal_writer)
    : config_manager_(config_manager),
      storage_engine_(storage_engine),
      wal_writer_(wal_writer),
      running_(false) {

  // 从配置管理器获取检查点配置
  config_.interval = std::chrono::seconds(300);  // 默认5分钟
  config_.max_wal_size = 1024ULL * 1024 * 1024;  // 默认1GB
  config_.max_checkpoint_pages = 10000;
  config_.enable_auto_checkpoint = true;
  config_.dirty_page_threshold = 0.8;

  // 初始化统计信息
  stats_.last_checkpoint = std::chrono::steady_clock::now();
}

CheckpointManager::~CheckpointManager() {
  Stop();
}

void CheckpointManager::Start() {
  if (running_) {
    return;
  }

  running_ = true;
  checkpoint_thread_ = std::thread(&CheckpointManager::CheckpointWorker, this);
}

void CheckpointManager::Stop() {
  if (!running_) {
    return;
  }

  running_ = false;
  {
    std::unique_lock<std::mutex> lock(checkpoint_mutex_);
    checkpoint_cv_.notify_one();
  }

  if (checkpoint_thread_.joinable()) {
    checkpoint_thread_.join();
  }
}

bool CheckpointManager::PerformCheckpoint() {
  std::unique_lock<std::mutex> lock(config_mutex_);

  if (!config_.enable_auto_checkpoint) {
    return true;
  }

  lock.unlock();
  return DoCheckpoint();
}

bool CheckpointManager::ForceCheckpoint() {
  return DoCheckpoint();
}

const CheckpointManager::CheckpointStats& CheckpointManager::GetStats() const {
  return stats_;
}

void CheckpointManager::ResetStats() {
  std::unique_lock<std::mutex> lock(stats_mutex_);
  stats_ = CheckpointStats{};
  stats_.last_checkpoint = std::chrono::steady_clock::now();
}

void CheckpointManager::SetConfig(const CheckpointConfig& config) {
  std::unique_lock<std::mutex> lock(config_mutex_);
  config_ = config;
}

CheckpointManager::CheckpointConfig CheckpointManager::GetConfig() const {
  std::unique_lock<std::mutex> lock(config_mutex_);
  return config_;
}

bool CheckpointManager::ShouldCheckpoint() const {
  std::unique_lock<std::mutex> lock(config_mutex_);

  if (!config_.enable_auto_checkpoint) {
    return false;
  }

  auto now = std::chrono::steady_clock::now();
  auto time_since_last = now - stats_.last_checkpoint;

  // 检查时间间隔
  if (time_since_last >= config_.interval) {
    return true;
  }

  // 检查WAL文件大小
  auto wal_stats = wal_writer_.GetStats();
  if (wal_stats.current_log_size >= config_.max_wal_size) {
    return true;
  }

  // 检查脏页比例（需要缓冲池支持，这里简化处理）
  // 在实际实现中，需要从缓冲池获取脏页统计

  return false;
}

void CheckpointManager::CheckpointWorker() {
  while (running_) {
    std::unique_lock<std::mutex> lock(checkpoint_mutex_);

    // 等待检查点信号或超时
    checkpoint_cv_.wait_for(lock, config_.interval, [this]() {
      return !running_ || ShouldCheckpoint();
    });

    if (!running_) {
      break;
    }

    // 执行检查点
    lock.unlock();
    DoCheckpoint();
  }
}

bool CheckpointManager::DoCheckpoint() {
  auto start_time = std::chrono::steady_clock::now();

  try {
    // 1. 获取当前LSN作为检查点LSN
    uint64_t checkpoint_lsn = wal_writer_.GetCurrentLSN();

    // 2. 刷新所有脏页面到磁盘
    size_t flushed_pages = FlushDirtyPages();

    // 3. 确保WAL日志同步到磁盘
    wal_writer_.Sync();

    // 4. 清理过期的WAL日志
    size_t cleaned_logs = CleanupWALLogs(checkpoint_lsn);

    // 5. 更新检查点元数据
    bool metadata_updated = UpdateCheckpointMetadata(checkpoint_lsn);

    auto end_time = std::chrono::steady_clock::now();
    auto checkpoint_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    // 更新统计信息
    {
      std::unique_lock<std::mutex> stats_lock(stats_mutex_);
      stats_.total_checkpoints.fetch_add(1);
      stats_.total_pages_flushed.fetch_add(flushed_pages);
      stats_.wal_logs_cleaned.fetch_add(cleaned_logs);
      stats_.last_checkpoint = std::chrono::steady_clock::now();

      if (stats_.max_checkpoint_time.count() == 0 ||
          checkpoint_duration > stats_.max_checkpoint_time) {
        stats_.max_checkpoint_time = checkpoint_duration;
      }

      if (stats_.avg_checkpoint_time.count() == 0) {
        stats_.avg_checkpoint_time = checkpoint_duration;
      } else {
        stats_.avg_checkpoint_time = (stats_.avg_checkpoint_time + checkpoint_duration) / 2;
      }

      // 估算刷新字节数（假设每页4KB）
      stats_.total_bytes_flushed.fetch_add(flushed_pages * 4096);
    }

    return metadata_updated;

  } catch (const std::exception& e) {
    std::cerr << "Checkpoint failed: " << e.what() << std::endl;
    return false;
  }
}

size_t CheckpointManager::FlushDirtyPages() {
  // 获取缓冲池并刷新所有脏页
  auto buffer_pool = storage_engine_.GetBufferPool();
  if (!buffer_pool) {
    return 0;
  }

  // 在实际实现中，这里应该调用缓冲池的FlushAllDirtyPages方法
  // 现在简化处理，假设所有页面都被刷新
  size_t total_pages = 1000;  // 模拟值，在实际系统中应该从缓冲池获取

  // 模拟刷新过程
  std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 模拟I/O时间

  return total_pages;
}

size_t CheckpointManager::CleanupWALLogs(uint64_t min_lsn) {
  // WAL日志清理逻辑
  // 在检查点完成后，可以清理LSN小于检查点LSN的日志

  // 这里简化实现，实际应该：
  // 1. 确定可以安全删除的日志文件
  // 2. 删除过期的WAL日志段
  // 3. 更新WAL写入器的状态

  // 模拟清理操作
  size_t cleaned_count = 5;  // 模拟清理的日志文件数

  // 截断WAL文件到检查点LSN
  if (!wal_writer_.TruncateToLSN(min_lsn)) {
    return 0;
  }

  return cleaned_count;
}

bool CheckpointManager::UpdateCheckpointMetadata(uint64_t checkpoint_lsn) {
  // 更新检查点元数据
  // 在实际系统中，这应该：
  // 1. 将检查点信息写入专门的检查点文件
  // 2. 更新数据库元数据
  // 3. 确保元数据持久化

  // 简化实现：将检查点信息写入内存中的配置或文件
  try {
    // 模拟元数据更新
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // 在实际实现中，这里应该原子性地更新检查点文件
    // 包含：检查点LSN、时间戳、活跃事务列表等

    return true;
  } catch (const std::exception& e) {
    return false;
  }
}

} // namespace sqlcc
