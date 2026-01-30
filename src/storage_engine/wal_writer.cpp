#include "src/storage_engine/wal_writer.h"
#include "src/storage_engine/wal_buffer.h"
#include "src/utils/config_manager.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <sstream>

namespace sqlcc {

namespace fs = std::filesystem;

WALWriter::WALWriter(ConfigManager& config_manager, const std::string& wal_file)
    : config_manager_(config_manager),
      wal_file_path_(wal_file),
      running_(false),
      current_lsn_(0) {

  // 从配置管理器获取参数
  max_batch_size_ = 1000;  // 默认批处理大小
  sync_interval_ = std::chrono::milliseconds(100);  // 默认同步间隔

  // 确保WAL文件目录存在
  fs::path wal_path(wal_file_path_);
  fs::create_directories(wal_path.parent_path());

  // 尝试打开WAL文件
  wal_file_.open(wal_file_path_, std::ios::binary | std::ios::app);
  if (!wal_file_.is_open()) {
    throw std::runtime_error("Failed to open WAL file: " + wal_file_path_);
  }

  // 初始化统计信息
  stats_.current_log_size = 0;
}

WALWriter::~WALWriter() {
  Stop();
  if (wal_file_.is_open()) {
    wal_file_.close();
  }
}

void WALWriter::Start() {
  if (running_) {
    return;
  }

  running_ = true;
  write_thread_ = std::thread(&WALWriter::WALWriteWorker, this);
}

void WALWriter::Stop() {
  if (!running_) {
    return;
  }

  running_ = false;
  {
    std::unique_lock<std::mutex> lock(write_mutex_);
    write_cv_.notify_one();
  }

  if (write_thread_.joinable()) {
    write_thread_.join();
  }
}

bool WALWriter::WriteRecords(const std::vector<std::unique_ptr<WALBuffer::WALRecord>>& records) {
  if (records.empty()) {
    return true;
  }

  std::unique_lock<std::mutex> lock(queue_mutex_);

  // 将记录添加到写入队列
  write_queue_.emplace_back();
  auto& batch = write_queue_.back();
  batch.reserve(records.size());

  for (const auto& record : records) {
    batch.push_back(std::make_unique<WALBuffer::WALRecord>(*record));
  }

  // 通知写入线程
  {
    std::unique_lock<std::mutex> write_lock(write_mutex_);
    write_cv_.notify_one();
  }

  return true;
}

bool WALWriter::Sync() {
  std::unique_lock<std::mutex> lock(file_mutex_);
  if (wal_file_.is_open()) {
    wal_file_.flush();
    return true;
  }
  return false;
}

const WALWriter::WALWriterStats& WALWriter::GetStats() const {
  return stats_;
}

void WALWriter::ResetStats() {
  std::unique_lock<std::mutex> lock(stats_mutex_);
  stats_.total_writes.store(0);
  stats_.total_records.store(0);
  stats_.total_bytes.store(0);
  stats_.failed_writes.store(0);
  stats_.avg_write_time = std::chrono::microseconds(0);
  stats_.max_write_time = std::chrono::microseconds(0);
  if (fs::exists(wal_file_path_)) {
    stats_.current_log_size = fs::file_size(wal_file_path_);
  } else {
    stats_.current_log_size = 0;
  }
}

uint64_t WALWriter::GetCurrentLSN() const {
  return current_lsn_.load();
}

bool WALWriter::TruncateToLSN(uint64_t target_lsn) {
  (void)target_lsn; // 避免未使用参数警告
  // 简单的日志截断实现
  // 在实际系统中，这需要更复杂的逻辑来确保数据一致性
  std::unique_lock<std::mutex> lock(file_mutex_);

  if (!wal_file_.is_open()) {
    return false;
  }

  // 关闭当前文件
  wal_file_.close();

  try {
    // 备份当前WAL文件
    fs::path wal_path(wal_file_path_);
    fs::path backup_path = wal_path;
    backup_path += ".backup";

    if (fs::exists(wal_path)) {
      fs::copy_file(wal_path, backup_path, fs::copy_options::overwrite_existing);

      // 重新打开文件进行截断
      wal_file_.open(wal_file_path_, std::ios::binary | std::ios::out | std::ios::trunc);
      if (!wal_file_.is_open()) {
        // 恢复备份
        fs::copy_file(backup_path, wal_path, fs::copy_options::overwrite_existing);
        return false;
      }
    }

    // 删除备份
    if (fs::exists(backup_path)) {
      fs::remove(backup_path);
    }

    // 重置统计信息
    {
      std::unique_lock<std::mutex> stats_lock(stats_mutex_);
      stats_.current_log_size = 0;
    }

    return true;
  } catch (const std::exception& e) {
    // 尝试恢复
    fs::path wal_path(wal_file_path_);
    if (fs::exists(wal_path)) {
      fs::path backup_path = wal_path;
      backup_path += ".backup";
      if (fs::exists(backup_path)) {
        fs::copy_file(backup_path, wal_path, fs::copy_options::overwrite_existing);
      }
    }
    return false;
  }
}

void WALWriter::WALWriteWorker() {
  while (running_) {
    std::vector<std::vector<std::unique_ptr<WALBuffer::WALRecord>>> current_queue;
    
    // 1. 获取所有待写入的批次，释放队列锁
    {
      std::unique_lock<std::mutex> queue_lock(queue_mutex_);
      if (write_queue_.empty()) {
        queue_lock.unlock();
        
        // 等待写入任务或超时
        std::unique_lock<std::mutex> write_lock(write_mutex_);
        write_cv_.wait_for(write_lock, sync_interval_, [this]() {
          std::unique_lock<std::mutex> q_lock(queue_mutex_);
          return !running_ || !write_queue_.empty();
        });
        
        // 再次检查队列
        queue_lock.lock();
        if (write_queue_.empty()) {
          continue;
        }
      }
      
      // 交换队列内容
      current_queue.swap(write_queue_);
    }
    
    // 2. 处理所有批次
    for (auto& batch : current_queue) {
      if (!batch.empty()) {
        PerformWrite(batch);
      }
    }
    current_queue.clear();
    
    // 3. 检查是否需要退出
    if (!running_) {
      // 检查是否还有未处理的记录
      std::unique_lock<std::mutex> queue_lock(queue_mutex_);
      if (write_queue_.empty()) {
        break;
      }
    }
  }
}

bool WALWriter::PerformWrite(const std::vector<std::unique_ptr<WALBuffer::WALRecord>>& records) {
  if (records.empty()) {
    return true;
  }

  auto start_time = std::chrono::steady_clock::now();

  try {
    std::unique_lock<std::mutex> lock(file_mutex_);

    if (!wal_file_.is_open()) {
      stats_.failed_writes.fetch_add(1);
      return false;
    }

    size_t total_bytes = 0;

    // 写入所有记录
    for (const auto& record : records) {
      if (!record) continue;

      std::string formatted_record = FormatRecord(*record);
      wal_file_.write(formatted_record.c_str(), formatted_record.size());

      total_bytes += formatted_record.size();
      current_lsn_.store(std::max(current_lsn_.load(), record->lsn));
    }

    // 同步到磁盘
    wal_file_.flush();

    auto end_time = std::chrono::steady_clock::now();
    auto write_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    // 更新统计信息
    {
      std::unique_lock<std::mutex> stats_lock(stats_mutex_);
      stats_.total_writes.fetch_add(1);
      stats_.total_records.fetch_add(records.size());
      stats_.total_bytes.fetch_add(total_bytes);
      stats_.current_log_size += total_bytes;

      if (stats_.max_write_time.count() == 0 ||
          write_duration > stats_.max_write_time) {
        stats_.max_write_time = write_duration;
      }

      if (stats_.avg_write_time.count() == 0) {
        stats_.avg_write_time = write_duration;
      } else {
        stats_.avg_write_time = (stats_.avg_write_time + write_duration) / 2;
      }
    }

    return true;

  } catch (const std::exception& e) {
    std::unique_lock<std::mutex> stats_lock(stats_mutex_);
    stats_.failed_writes.fetch_add(1);
    return false;
  }
}

std::string WALWriter::FormatRecord(const WALBuffer::WALRecord& record) {
  std::ostringstream oss;

  // 格式：LSN|TXN_ID|OPERATION|TIMESTAMP|DATA_SIZE|DATA\n
  oss << record.lsn << "|"
      << record.transaction_id << "|"
      << record.operation << "|"
      << record.timestamp.time_since_epoch().count() << "|"
      << record.data.size() << "|"
      << record.data << "\n";

  return oss.str();
}

} // namespace sqlcc
