#pragma once

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "utils/config_manager.h"
#include "wal_buffer.h"

namespace sqlcc {

/**
 * @brief WAL写入器类
 *
 * 负责将WAL缓冲区中的日志记录写入磁盘文件
 */
class WALWriter {
public:
  /**
   * @brief WAL写入器统计信息
   */
  struct WALWriterStats {
    std::atomic<size_t> total_writes{0};          // 总写入次数
    std::atomic<size_t> total_records{0};         // 总写入记录数
    std::atomic<size_t> total_bytes{0};           // 总写入字节数
    std::atomic<size_t> failed_writes{0};         // 写入失败次数
    std::chrono::microseconds avg_write_time{0};  // 平均写入时间
    std::chrono::microseconds max_write_time{0};  // 最大写入时间
    std::atomic<size_t> current_log_size{0};      // 当前日志文件大小

    double write_success_rate() const {
      size_t total = total_writes.load() + failed_writes.load();
      return total > 0 ? static_cast<double>(total_writes.load()) / total : 0.0;
    }
  };

  /**
   * @brief 构造函数
   * @param config_manager 配置管理器引用
   * @param wal_file WAL文件路径
   */
  WALWriter(ConfigManager& config_manager, const std::string& wal_file);

  /**
   * @brief 析构函数
   */
  ~WALWriter();

  /**
   * @brief 启动WAL写入器
   */
  void Start();

  /**
   * @brief 停止WAL写入器
   */
  void Stop();

  /**
   * @brief 写入WAL记录到磁盘
   * @param records WAL记录列表
   * @return 是否成功
   */
  bool WriteRecords(const std::vector<std::unique_ptr<WALBuffer::WALRecord>>& records);

  /**
   * @brief 同步WAL文件到磁盘
   * @return 是否成功
   */
  bool Sync();

  /**
   * @brief 获取WAL写入器统计信息
   * @return 统计信息
   */
  WALWriterStats GetStats() const;

  /**
   * @brief 重置统计信息
   */
  void ResetStats();

  /**
   * @brief 获取当前LSN
   * @return 当前LSN
   */
  uint64_t GetCurrentLSN() const;

  /**
   * @brief 截断WAL日志到指定LSN
   * @param target_lsn 目标LSN
   * @return 是否成功
   */
  bool TruncateToLSN(uint64_t target_lsn);

private:
  /**
   * @brief WAL写入工作线程函数
   */
  void WALWriteWorker();

  /**
   * @brief 执行记录写入
   * @param records 要写入的记录
   * @return 是否成功
   */
  bool PerformWrite(const std::vector<std::unique_ptr<WALBuffer::WALRecord>>& records);

  /**
   * @brief 格式化WAL记录为字符串
   * @param record WAL记录
   * @return 格式化后的字符串
   */
  std::string FormatRecord(const WALBuffer::WALRecord& record);

  // 配置和状态
  ConfigManager& config_manager_;
  const std::string wal_file_path_;

  // 文件操作
  std::ofstream wal_file_;
  mutable std::mutex file_mutex_;

  // 工作线程
  std::thread write_thread_;
  std::atomic<bool> running_;
  std::condition_variable write_cv_;
  mutable std::mutex write_mutex_;

  // 待写入队列
  std::vector<std::vector<std::unique_ptr<WALBuffer::WALRecord>>> write_queue_;
  mutable std::mutex queue_mutex_;

  // 统计信息
  mutable std::mutex stats_mutex_;
  WALWriterStats stats_;

  // LSN管理
  std::atomic<uint64_t> current_lsn_;

  // 配置参数
  size_t max_batch_size_;       // 最大批处理大小
  std::chrono::milliseconds sync_interval_; // 同步间隔
};

} // namespace sqlcc
