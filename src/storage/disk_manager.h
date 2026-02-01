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

#include "../page/page.h"
#include "../utils/config_manager.h"
#include "../exception/exception.h"

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
 * @class DiskManager
 * @brief 磁盘管理器 - 实现数据库文件与物理磁盘之间的底层 I/O 通信
 *
 * WHY层 - 设计意图：
 *   数据库是持久化系统。内存数据如果不写入磁盘，在断电或进程重启时将永久丢失。
 *   DiskManager 作为存储引擎的最底层组件，屏蔽了操作系统文件系统的复杂性，
 *   提供了一种以“固定大小页面（8KB）”为单位的逻辑磁盘访问模型，
 *   支持快速定位、空间分配和数据持久化保障。
 *
 * WHAT层 - 功能说明：
 *   实现页面的物理读写（ReadPage, WritePage），映射 page_id 到文件的物理偏移量（Offset = page_id * PAGE_SIZE）。
 *   管理数据库文件的增长与收缩（AllocatePage, DeallocatePage）。
 *   提供异步预读支持（PrefetchPage）以优化顺序扫描性能。
 *   支持强制落盘（Sync/fsync）确保事务的持久性（Durability）。
 *
 * HOW层 - 实现机制：
 *   1. 文件指针操作：使用 std::fstream 并配合 seekp/seekg 实现随机访问。
 *   2. 空间分配：维护一个简单的 Atomic 计数器（next_page_id_）和空闲列表（free_pages_）实现 O(1) 分配。
 *   3. 线程安全：使用 recursive_timed_mutex 保护文件流指针，防止并发读写导致的数据竞争。
 *   4. 错误容忍：集成了模拟故障机制（simulate_failure_），便于在测试环境下验证系统在磁盘损坏时的恢复能力。
 */
class DiskManager {
public:
  /**
   * @brief 构造函数
   *
   * @param db_file 数据库文件名
   * @param config_manager 配置管理器引用
   */
  DiskManager(const std::string &db_file, ConfigManager &config_manager);

  /**
   * @brief 析构函数
   */
  ~DiskManager();

  /**
   * @brief 写入页面数据到磁盘
   *
   * @param page_id 页面ID
   * @param page_data 页面数据缓冲区
   * @return 是否写入成功
   */
  bool WritePage(int32_t page_id, const char *page_data);

  /**
   * @brief 从磁盘读取页面数据
   *
   * @param page_id 页面ID
   * @param page_data 页面数据缓冲区
   * @return 是否读取成功
   */
  bool ReadPage(int32_t page_id, char *page_data);

  /**
   * @brief 分配新页面
   *
   * @return 新分配的页面ID，失败返回-1
   */
  int32_t AllocatePage();

  /**
   * @brief 释放页面
   *
   * @param page_id 要释放的页面ID
   * @return 是否释放成功
   */
  bool DeallocatePage(int32_t page_id);

  /**
   * @brief 获取文件大小
   *
   * @return 文件大小（字节数）
   */
  int32_t GetFileSize() const;

  /**
   * @brief 批量读取页面
   *
   * @param page_ids 页面ID列表
   * @param data_buffers 数据缓冲区列表
   * @return 是否批量读取成功
   */
  bool BatchReadPages(const std::vector<int32_t> &page_ids,
                      std::vector<char *> &data_buffers);

  /**
   * @brief 预读取页面
   *
   * @param page_id 要预读取的页面ID
   * @return 是否预读取成功
   */
  bool PrefetchPage(int32_t page_id);

  /**
   * @brief 批量预读取页面
   *
   * @param page_ids 页面ID列表
   * @return 是否批量预读取成功
   */
  bool BatchPrefetchPages(const std::vector<int32_t> &page_ids);

  /**
   * @brief 同步文件到磁盘
   *
   * @return 是否同步成功
   */
  bool Sync();

  /**
   * @brief 配置变更回调
   *
   * @param key 配置键
   * @param value 配置值
   */
  void OnConfigChange(const std::string &key, const ConfigValue &value);

  // I/O统计信息
  DiskIOStats io_stats_;

private:
  // 数据库文件名
  std::string db_file_name_;

  // 文件流
  std::fstream db_io_;

  // 配置管理器引用
  ConfigManager &config_manager_;

  // 文件大小
  std::atomic<size_t> file_size_;

  // 下一个可用的页面ID
  std::atomic<int32_t> next_page_id_;

  // 锁超时时间
  int lock_timeout_ms_;

  // I/O操作互斥锁
  mutable std::recursive_timed_mutex io_mutex_;

  // 空闲页面列表
  std::vector<int32_t> free_pages_;

  // 模拟失败标志（用于测试）
  bool simulate_seek_failure_ = false;
  bool simulate_read_failure_ = false;
  bool simulate_write_failure_ = false;
  bool simulate_flush_failure_ = false;

  // 友元测试类
  friend class DiskManagerTest;
};

} // namespace sqlcc
