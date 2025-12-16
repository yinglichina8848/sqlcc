#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "utils/config_manager.h"
#include "storage_engine.h"
#include "wal_writer.h"

namespace sqlcc {

/**
 * @brief 检查点管理器类
 *
 * 负责定期创建检查点，将内存中的脏数据持久化到磁盘，并清理过期的WAL日志
 */
class CheckpointManager {
public:
  /**
   * @brief 检查点统计信息
   */
  struct CheckpointStats {
    std::atomic<size_t> total_checkpoints{0};     // 总检查点次数
    std::atomic<size_t> total_pages_flushed{0};   // 总刷新页面数
    std::atomic<size_t> total_bytes_flushed{0};   // 总刷新字节数
    std::atomic<size_t> wal_logs_cleaned{0};      // 清理的WAL日志数
    std::chrono::microseconds avg_checkpoint_time{0}; // 平均检查点时间
    std::chrono::microseconds max_checkpoint_time{0}; // 最大检查点时间
    std::chrono::steady_clock::time_point last_checkpoint; // 最后检查点时间

    double avg_pages_per_checkpoint() const {
      size_t checkpoints = total_checkpoints.load();
      return checkpoints > 0
                 ? static_cast<double>(total_pages_flushed.load()) / checkpoints
                 : 0.0;
    }
  };

  /**
   * @brief 检查点配置
   */
  struct CheckpointConfig {
    std::chrono::seconds interval{300};          // 检查点间隔（默认5分钟）
    size_t max_wal_size{1024 * 1024 * 1024};     // 最大WAL大小（默认1GB）
    size_t max_checkpoint_pages{10000};          // 单次检查点最大页面数
    bool enable_auto_checkpoint{true};           // 是否启用自动检查点
    double dirty_page_threshold{0.8};            // 脏页阈值（80%）
  };

  /**
   * @brief 构造函数
   * @param config_manager 配置管理器引用
   * @param storage_engine 存储引擎引用
   * @param wal_writer WAL写入器引用
   */
  CheckpointManager(ConfigManager& config_manager,
                    StorageEngine& storage_engine,
                    WALWriter& wal_writer);

  /**
   * @brief 析构函数
   */
  ~CheckpointManager();

  /**
   * @brief 启动检查点管理器
   */
  void Start();

  /**
   * @brief 停止检查点管理器
   */
  void Stop();

  /**
   * @brief 执行检查点操作
   * @return 是否成功
   */
  bool PerformCheckpoint();

  /**
   * @brief 强制执行检查点
   * @return 是否成功
   */
  bool ForceCheckpoint();

  /**
   * @brief 获取检查点统计信息
   * @return 统计信息
   */
  CheckpointStats GetStats() const;

  /**
   * @brief 重置统计信息
   */
  void ResetStats();

  /**
   * @brief 设置检查点配置
   * @param config 检查点配置
   */
  void SetConfig(const CheckpointConfig& config);

  /**
   * @brief 获取当前检查点配置
   * @return 检查点配置
   */
  CheckpointConfig GetConfig() const;

  /**
   * @brief 检查是否需要检查点
   * @return 是否需要
   */
  bool ShouldCheckpoint() const;

private:
  /**
   * @brief 检查点工作线程函数
   */
  void CheckpointWorker();

  /**
   * @brief 执行检查点逻辑
   * @return 是否成功
   */
  bool DoCheckpoint();

  /**
   * @brief 刷新所有脏页面
   * @return 刷新页面数
   */
  size_t FlushDirtyPages();

  /**
   * @brief 清理过期的WAL日志
   * @param min_lsn 最小保留LSN
   * @return 清理的日志数
   */
  size_t CleanupWALLogs(uint64_t min_lsn);

  /**
   * @brief 更新检查点元数据
   * @param checkpoint_lsn 检查点LSN
   * @return 是否成功
   */
  bool UpdateCheckpointMetadata(uint64_t checkpoint_lsn);

  // 配置和依赖
  ConfigManager& config_manager_;
  StorageEngine& storage_engine_;
  WALWriter& wal_writer_;

  // 工作线程
  std::thread checkpoint_thread_;
  std::atomic<bool> running_;
  std::condition_variable checkpoint_cv_;
  mutable std::mutex checkpoint_mutex_;

  // 统计信息
  mutable std::mutex stats_mutex_;
  CheckpointStats stats_;

  // 配置
  mutable std::mutex config_mutex_;
  CheckpointConfig config_;
};

} // namespace sqlcc
