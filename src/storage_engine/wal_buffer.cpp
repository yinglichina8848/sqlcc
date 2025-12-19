#include "storage/wal_buffer.h"
#include "storage/wal_writer.h"
#include "utils/config_manager.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>

namespace sqlcc {

WALBuffer::WALBuffer(ConfigManager& config_manager, size_t buffer_size)
    : wal_writer_(nullptr),
      config_manager_(config_manager),
      max_buffer_size_(buffer_size),
      buffer_(),
      buffer_mutex_(),
      flush_thread_(),
      running_(true),
      flush_cv_(),
      flush_mutex_(),
      stats_mutex_(),
      stats_(),
      flush_interval_(std::chrono::milliseconds(1000)),  // 默认1秒
      flush_threshold_(80),                              // 默认80%
      max_records_per_flush_(1000),                      // 默认1000条记录
      next_lsn_(1) {

  // 从配置管理器获取参数，设置默认值
  flush_interval_ = std::chrono::milliseconds(1000);  // 默认1秒
  flush_threshold_ = 80;                              // 默认80%
  max_records_per_flush_ = 1000;                      // 默认1000条记录

  // 启动后台刷新线程
  flush_thread_ = std::thread(&WALBuffer::BackgroundFlushWorker, this);

  // 初始化统计信息
  stats_.current_buffer_size = 0;
}

WALBuffer::~WALBuffer() {
  // 停止后台线程
  {
    std::unique_lock<std::mutex> lock(flush_mutex_);
    running_ = false;
    flush_cv_.notify_one();
  }

  if (flush_thread_.joinable()) {
    flush_thread_.join();
  }

  // 最后一次强制刷新
  ForceFlush();
}

bool WALBuffer::AddRecord(std::unique_ptr<WALRecord> record) {
  if (!record) {
    return false;
  }

  // 分配LSN
  record->lsn = next_lsn_.fetch_add(1);

  std::unique_lock<std::mutex> lock(buffer_mutex_);

  // 检查缓冲区大小限制
  size_t record_size = record->data.size() + sizeof(WALRecord);
  if (buffer_.size() >= max_records_per_flush_ ||
      GetCurrentSize() + record_size > max_buffer_size_) {

    // 触发同步刷新
    lock.unlock();
    if (!Flush()) {
      return false;
    }
    lock.lock();
  }

  // 添加记录到缓冲区
  buffer_.push_back(std::move(record));
  stats_.current_buffer_size.fetch_add(record_size);
  stats_.total_logs.fetch_add(1);

  // 检查是否需要异步刷新
  if (ShouldFlush()) {
    std::unique_lock<std::mutex> flush_lock(flush_mutex_);
    flush_cv_.notify_one();
  }

  return true;
}

bool WALBuffer::Flush() {
  std::vector<std::unique_ptr<WALRecord>> records_to_flush;

  {
    std::unique_lock<std::mutex> lock(buffer_mutex_);
    if (buffer_.empty()) {
      return true;
    }

    // 移动记录到待刷新队列
    records_to_flush.reserve(buffer_.size());
    for (auto& record : buffer_) {
      records_to_flush.push_back(std::move(record));
    }
    buffer_.clear();

    size_t flushed_size = stats_.current_buffer_size.load();
    (void)flushed_size; // 避免未使用变量警告
    stats_.current_buffer_size = 0;
    stats_.total_flushes.fetch_add(1);

    // 记录刷新时间
    auto start_time = std::chrono::steady_clock::now();

    // 调用WALWriter写入记录
    bool write_success = false;
    if (wal_writer_) {
      write_success = wal_writer_->WriteRecords(records_to_flush);
    }

    auto end_time = std::chrono::steady_clock::now();
    auto flush_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    // 更新平均刷新时间
    {
      std::unique_lock<std::mutex> stats_lock(stats_mutex_);
      if (stats_.avg_flush_time.count() == 0) {
        stats_.avg_flush_time = flush_duration;
      } else {
        stats_.avg_flush_time = (stats_.avg_flush_time + flush_duration) / 2;
      }
    }

    if (write_success) {
      stats_.buffer_hits.fetch_add(records_to_flush.size());
    } else {
      stats_.buffer_misses.fetch_add(records_to_flush.size());
      return false;
    }
  }

  return true;
}

bool WALBuffer::ForceFlush() {
  std::unique_lock<std::mutex> lock(buffer_mutex_);
  // 强制刷新所有待处理记录
  return Flush();
}

const WALBuffer::WALBufferStats& WALBuffer::GetStats() const {
  return stats_;
}

void WALBuffer::ResetStats() {
  std::unique_lock<std::mutex> lock(stats_mutex_);
  stats_.total_logs.store(0);
  stats_.total_flushes.store(0);
  stats_.buffer_hits.store(0);
  stats_.buffer_misses.store(0);
  stats_.current_buffer_size.store(0);
  stats_.avg_flush_time = std::chrono::microseconds(0);
}

size_t WALBuffer::GetCurrentSize() const {
  return stats_.current_buffer_size.load();
}

double WALBuffer::GetUtilization() const {
  return static_cast<double>(GetCurrentSize()) / max_buffer_size_;
}

bool WALBuffer::ShouldFlush() const {
  size_t current_size = GetCurrentSize();
  size_t current_records = buffer_.size();

  // 基于大小阈值
  double utilization = static_cast<double>(current_size) / max_buffer_size_;
  if (utilization >= static_cast<double>(flush_threshold_) / 100.0) {
    return true;
  }

  // 基于记录数量
  if (current_records >= max_records_per_flush_) {
    return true;
  }

  return false;
}

void WALBuffer::BackgroundFlushWorker() {
  while (running_) {
    std::unique_lock<std::mutex> lock(flush_mutex_);

    // 等待刷新信号或超时
    flush_cv_.wait_for(lock, flush_interval_, [this]() {
      return !running_ || ShouldFlush();
    });

    if (!running_) {
      break;
    }

    // 执行后台刷新
    lock.unlock();
    Flush();
  }
}

} // namespace sqlcc
