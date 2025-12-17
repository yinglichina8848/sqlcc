#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#include "utils/config_manager.h"
#include "../disk_manager.h"

// 定义页面数据类型
using PageData = std::vector<char>;

namespace sqlcc {

/**
 * @brief 脏页信息结构
 */
struct DirtyPageInfo {
  int32_t page_id;                                    // 页面ID
  std::chrono::steady_clock::time_point last_modified; // 最后修改时间
  int modification_count;                             // 修改次数
  bool is_pinned;                                     // 是否被固定

  DirtyPageInfo(int32_t pid)
      : page_id(pid), modification_count(1), is_pinned(false),
        last_modified(std::chrono::steady_clock::now()) {}

  // 更新修改时间和计数
  void update() {
    last_modified = std::chrono::steady_clock::now();
    modification_count++;
  }
};

/**
 * @brief 延迟写入器类
 *
 * 负责将脏页异步批量写入磁盘，减少I/O等待时间
 */
class LazyWriter {
public:
  /**
   * @brief 延迟写入器统计信息
   */
  struct LazyWriterStats {
    std::atomic<size_t> total_writes{0};          // 总写入次数
    std::atomic<size_t> total_pages_written{0};   // 总写入页面数
    std::atomic<size_t> total_bytes_written{0};   // 总写入字节数
    std::atomic<size_t> forced_writes{0};         // 强制写入次数
    std::chrono::microseconds avg_write_time{0};  // 平均写入时间
    std::atomic<size_t> current_dirty_pages{0};   // 当前脏页数

    // 默认构造函数
    LazyWriterStats() = default;

    // 拷贝构造函数
    LazyWriterStats(const LazyWriterStats &other) {
      total_writes.store(other.total_writes.load());
      total_pages_written.store(other.total_pages_written.load());
      total_bytes_written.store(other.total_bytes_written.load());
      forced_writes.store(other.forced_writes.load());
      avg_write_time = other.avg_write_time;
      current_dirty_pages.store(other.current_dirty_pages.load());
    }

    // 移动构造函数
    LazyWriterStats(LazyWriterStats &&other) noexcept {
      total_writes.store(other.total_writes.load());
      total_pages_written.store(other.total_pages_written.load());
      total_bytes_written.store(other.total_bytes_written.load());
      forced_writes.store(other.forced_writes.load());
      avg_write_time = std::move(other.avg_write_time);
      current_dirty_pages.store(other.current_dirty_pages.load());
    }

    // 拷贝赋值操作符
    LazyWriterStats &operator=(const LazyWriterStats &other) {
      if (this != &other) {
        total_writes.store(other.total_writes.load());
        total_pages_written.store(other.total_pages_written.load());
        total_bytes_written.store(other.total_bytes_written.load());
        forced_writes.store(other.forced_writes.load());
        avg_write_time = other.avg_write_time;
        current_dirty_pages.store(other.current_dirty_pages.load());
      }
      return *this;
    }

    // 移动赋值操作符
    LazyWriterStats &operator=(LazyWriterStats &&other) noexcept {
      if (this != &other) {
        total_writes.store(other.total_writes.load());
        total_pages_written.store(other.total_pages_written.load());
        total_bytes_written.store(other.total_bytes_written.load());
        forced_writes.store(other.forced_writes.load());
        avg_write_time = std::move(other.avg_write_time);
        current_dirty_pages.store(other.current_dirty_pages.load());
      }
      return *this;
    }

    double avg_pages_per_write() const {
      size_t writes = total_writes.load();
      return writes > 0
                 ? static_cast<double>(total_pages_written.load()) / writes
                 : 0.0;
    }

    double avg_bytes_per_write() const {
      size_t writes = total_writes.load();
      return writes > 0
                 ? static_cast<double>(total_bytes_written.load()) / writes
                 : 0.0;
    }
  };

  /**
   * @brief 构造函数
   * @param config_manager 配置管理器引用
   * @param disk_manager 磁盘管理器引用
   */
  explicit LazyWriter(ConfigManager &config_manager, DiskManager &disk_manager);

  /**
   * @brief 析构函数
   */
  ~LazyWriter();

  /**
   * @brief 启动延迟写入器
   */
  void Start();

  /**
   * @brief 停止延迟写入器
   */
  void Stop();

  /**
   * @brief 标记页面为脏页
   * @param page_id 页面ID
   * @param page_data 页面数据
   */
  void MarkDirty(int32_t page_id, const PageData &page_data);

  /**
   * @brief 强制刷新所有脏页
   */
  void ForceFlush();

  /**
   * @brief 获取延迟写入器统计信息
   * @return 统计信息
   */
  LazyWriterStats GetStats() const;

  /**
   * @brief 重置统计信息
   */
  void ResetStats();

  /**
   * @brief 设置是否启用
   * @param enabled 是否启用
   */
  void SetEnabled(bool enabled);

  /**
   * @brief 检查是否启用
   * @return 是否启用
   */
  bool IsEnabled() const;

private:
  /**
   * @brief 延迟写入工作线程函数
   */
  void LazyWriteWorker();

  /**
   * @brief 执行批量写入
   * @param dirty_pages 要写入的脏页列表
   */
  void FlushBatch(const std::vector<DirtyPageInfo> &dirty_pages);

  /**
   * @brief 检查是否应该触发写入
   * @return 是否应该写入
   */
  bool ShouldFlush() const;

  /**
   * @brief 更新配置参数
   */
  void UpdateConfig();

  /**
   * @brief 获取需要刷新的脏页
   * @return 脏页列表
   */
  std::vector<DirtyPageInfo> GetPagesToFlush();

  // 配置和依赖
  ConfigManager &config_manager_;
  DiskManager &disk_manager_;

  // 脏页管理
  std::unordered_map<int32_t, DirtyPageInfo> dirty_pages_;
  std::unordered_map<int32_t, PageData> dirty_page_data_;
  mutable std::mutex dirty_mutex_;

  // 工作线程
  std::thread worker_thread_;
  std::atomic<bool> running_;
  std::atomic<bool> enabled_;

  // 条件变量用于唤醒工作线程
  std::condition_variable flush_cv_;
  mutable std::mutex flush_mutex_;

  // 统计信息
  mutable std::mutex stats_mutex_;
  LazyWriterStats stats_;

  // 配置参数
  std::chrono::milliseconds flush_interval_;
  size_t max_dirty_pages_;
  size_t batch_size_;
  std::chrono::milliseconds max_dirty_age_;
};

} // namespace sqlcc
