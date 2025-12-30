#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "utils/config_manager.h"

namespace sqlcc {

// Forward declaration for WALWriter
class WALWriter;

/**
 * @brief WAL缓冲区类
 *
 * 负责WAL日志的缓冲和批量写入，减少磁盘I/O次数
 */
class WALBuffer {
public:
  /**
   * @brief WAL缓冲区统计信息
   */
  struct WALBufferStats {
    std::atomic<size_t> total_logs{0};          // 总日志条数
    std::atomic<size_t> total_flushes{0};       // 总刷新次数
    std::atomic<size_t> buffer_hits{0};         // 缓冲区命中次数
    std::atomic<size_t> buffer_misses{0};       // 缓冲区未命中次数
    std::atomic<size_t> current_buffer_size{0}; // 当前缓冲区大小
    std::chrono::microseconds avg_flush_time{0}; // 平均刷新时间

    double hit_ratio() const {
      size_t total = buffer_hits.load() + buffer_misses.load();
      return total > 0 ? static_cast<double>(buffer_hits.load()) / total : 0.0;
    }
  };

  /**
   * @brief 日志记录结构
   */
  struct WALRecord {
    uint64_t lsn;           // 日志序列号
    uint64_t transaction_id; // 事务ID
    std::string operation;  // 操作类型
    std::string data;       // 日志数据
    std::chrono::steady_clock::time_point timestamp; // 时间戳

    WALRecord(uint64_t lsn_val, uint64_t tx_id, const std::string& op, const std::string& d)
        : lsn(lsn_val), transaction_id(tx_id), operation(op), data(d),
          timestamp(std::chrono::steady_clock::now()) {}
  };

  /**
   * @brief 构造函数
   * @param config_manager 配置管理器引用
   * @param buffer_size 缓冲区大小
   */
  explicit WALBuffer(ConfigManager& config_manager, size_t buffer_size = 64 * 1024 * 1024); // 默认64MB

  /**
   * @brief 析构函数
   */
  ~WALBuffer();

  /**
   * @brief 添加WAL记录到缓冲区
   * @param record WAL记录
   * @return 是否成功
   */
  bool AddRecord(std::unique_ptr<WALRecord> record);

  /**
   * @brief 刷新缓冲区到磁盘
   * @return 是否成功
   */
  bool Flush();

  /**
   * @brief 强制刷新缓冲区
   * @return 是否成功
   */
  bool ForceFlush();

  /**
   * @brief 获取缓冲区统计信息
   * @return 统计信息引用
   */
  const WALBufferStats& GetStats() const;

  /**
   * @brief 重置统计信息
   */
  void ResetStats();

  /**
   * @brief 获取当前缓冲区大小
   * @return 缓冲区大小（字节）
   */
  size_t GetCurrentSize() const;

  /**
   * @brief 获取缓冲区使用率
   * @return 使用率（0.0-1.0）
   */
  double GetUtilization() const;

  /**
   * @brief 设置WAL写入器
   * @param wal_writer WAL写入器指针
   */
  void SetWALWriter(WALWriter* wal_writer) { wal_writer_ = wal_writer; }

  /**
   * @brief 启动后台刷新线程
   */
  void Start();

  /**
   * @brief 停止后台刷新线程
   */
  void Stop();

private:
  // WAL写入器引用（需要在运行时设置）
  WALWriter* wal_writer_;
  /**
   * @brief 检查是否需要刷新
   * @return 是否需要刷新
   */
  bool ShouldFlush() const;

  /**
   * @brief 后台刷新线程函数
   */
  void BackgroundFlushWorker();

  // 配置和状态
  ConfigManager& config_manager_;
  const size_t max_buffer_size_;  // 最大缓冲区大小

  // 缓冲区数据
  std::vector<std::unique_ptr<WALRecord>> buffer_;
  mutable std::mutex buffer_mutex_;

  // 后台刷新线程
  std::thread flush_thread_;
  std::atomic<bool> running_;
  std::condition_variable flush_cv_;
  mutable std::mutex flush_mutex_;

  // 统计信息
  mutable std::mutex stats_mutex_;
  WALBufferStats stats_;

  // 配置参数
  std::chrono::milliseconds flush_interval_;  // 刷新间隔
  size_t flush_threshold_;                    // 刷新阈值（百分比）
  size_t max_records_per_flush_;             // 每次刷新最大记录数

  // LSN管理
  std::atomic<uint64_t> next_lsn_;           // 下一个LSN
};

} // namespace sqlcc
