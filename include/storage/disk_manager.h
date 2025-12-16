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



} // namespace sqlcc
