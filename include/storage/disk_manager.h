#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#include "page.h"
#include "utils/config_manager.h"

namespace sqlcc {

/**
 * @brief 磁盘I/O统计信息
 */
struct DiskIOStats {
  std::atomic<size_t> total_reads{0};       // 总读取次数
  std::atomic<size_t> total_writes{0};      // 总写入次数
  std::atomic<size_t> total_syncs{0};       // 总同步次数
  std::atomic<size_t> failed_reads{0};      // 读取失败次数
  std::atomic<size_t> failed_writes{0};     // 写入失败次数
  std::atomic<size_t> failed_syncs{0};      // 同步失败次数
  std::chrono::microseconds total_read_time{0};  // 总读取时间
  std::chrono::microseconds total_write_time{0}; // 总写入时间
};

/**
 * @brief 磁盘管理器异常类
 */
class DiskManagerException : public std::runtime_error {
public:
  explicit DiskManagerException(const std::string& message)
      : std::runtime_error(message) {}
};

/**
 * @brief 磁盘管理器类
 *
 * 负责数据库文件的I/O操作，包括页面读写、文件管理等
 */
class DiskManager {
public:
  /**
   * @brief 构造函数
   * @param db_file 数据库文件路径
   * @param config_manager 配置管理器引用
   */
  DiskManager(const std::string& db_file, ConfigManager& config_manager);

  /**
   * @brief 析构函数
   */
  ~DiskManager();

  /**
   * @brief 写入页面到磁盘
   * @param page_id 页面ID
   * @param page_data 页面数据
   * @return 是否成功
   */
  bool WritePage(int32_t page_id, const char* page_data);

  /**
   * @brief 从磁盘读取页面
   * @param page_id 页面ID
   * @param page_data 页面数据缓冲区
   * @return 是否成功
   */
  bool ReadPage(int32_t page_id, char* page_data);

  /**
   * @brief 分配新页面
   * @return 新页面ID，失败返回-1
   */
  int32_t AllocatePage();

  /**
   * @brief 释放页面
   * @param page_id 页面ID
   * @return 是否成功
   */
  bool DeallocatePage(int32_t page_id);

  /**
   * @brief 获取文件大小
   * @return 文件大小（字节）
   */
  int32_t GetFileSize() const;

  /**
   * @brief 批量读取页面
   * @param page_ids 页面ID列表
   * @param data_buffers 数据缓冲区列表
   * @return 是否成功
   */
  bool BatchReadPages(const std::vector<int32_t>& page_ids,
                      std::vector<char*>& data_buffers);

  /**
   * @brief 预取页面
   * @param page_id 页面ID
   * @return 是否成功
   */
  bool PrefetchPage(int32_t page_id);

  /**
   * @brief 批量预取页面
   * @param page_ids 页面ID列表
   * @return 是否成功
   */
  bool BatchPrefetchPages(const std::vector<int32_t>& page_ids);

  /**
   * @brief 同步文件到磁盘
   * @return 是否成功
   */
  bool Sync();

  /**
   * @brief 获取I/O统计信息
   * @return 统计信息
   */
  const DiskIOStats& GetIOStats() const { return io_stats_; }

private:
  /**
   * @brief 配置变更回调
   * @param key 配置键
   * @param value 配置值
   */
  void OnConfigChange(const std::string& key, const ConfigValue& value);

  // 文件和配置
  std::string db_file_name_;
  std::fstream db_io_;
  ConfigManager& config_manager_;

  // 文件状态
  size_t file_size_;
  int32_t next_page_id_;
  std::vector<int32_t> free_pages_;  // 空闲页面列表

  // 并发控制
  mutable std::recursive_timed_mutex io_mutex_;
  int lock_timeout_ms_;

  // I/O统计
  DiskIOStats io_stats_;

  // 测试辅助标志
  bool simulate_seek_failure_ = false;
  bool simulate_read_failure_ = false;
  bool simulate_write_failure_ = false;
  bool simulate_flush_failure_ = false;
};

} // namespace sqlcc
