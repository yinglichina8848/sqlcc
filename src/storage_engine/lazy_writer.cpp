#include "src/storage/lazy_writer.h"
#include <algorithm>
#include <iostream>

namespace sqlcc {

LazyWriter::LazyWriter(ConfigManager &config_manager, DiskManager &disk_manager)
    : config_manager_(config_manager), disk_manager_(disk_manager),
      running_(false), enabled_(true),
      flush_interval_(std::chrono::milliseconds(100)), max_dirty_pages_(1024), batch_size_(64),
      max_dirty_age_(std::chrono::milliseconds(1000)) {
  UpdateConfig();
}

LazyWriter::~LazyWriter() { Stop(); }

void LazyWriter::Start() {
  if (running_.exchange(true)) {
    return; // 已经在运行
  }

  if (!enabled_.load()) {
    return; // 未启用
  }

  worker_thread_ = std::thread([this]() { LazyWriteWorker(); });
}

void LazyWriter::Stop() {
  if (!running_.exchange(false)) {
    return; // 已经停止
  }

  {
    std::lock_guard<std::mutex> lock(flush_mutex_);
    flush_cv_.notify_all();  // 唤醒所有等待线程
  }

  if (worker_thread_.joinable()) {
    worker_thread_.join();
  }
}

void LazyWriter::MarkDirty(int32_t page_id, const PageData &page_data) {
  if (!enabled_.load()) {
    return;
  }

  std::lock_guard<std::mutex> lock(dirty_mutex_);

  auto it = dirty_pages_.find(page_id);
  if (it == dirty_pages_.end()) {
    // 新脏页
    dirty_pages_.emplace(page_id, DirtyPageInfo(page_id));
    stats_.current_dirty_pages.fetch_add(1);
  } else {
    // 更新现有脏页
    it->second.update();
  }

  // 存储页面数据
  dirty_page_data_[page_id] = page_data;

  // 检查是否需要触发写入
  if (ShouldFlush()) {
    std::lock_guard<std::mutex> lock(flush_mutex_);
    flush_cv_.notify_one();
  }
}

void LazyWriter::ForceFlush() {
  if (!enabled_.load()) {
    return;
  }

  std::vector<DirtyPageInfo> pages_to_flush;
  {
    std::lock_guard<std::mutex> lock(dirty_mutex_);
    for (const auto &pair : dirty_pages_) {
      if (!pair.second.is_pinned) {
        pages_to_flush.push_back(pair.second);
      }
    }
  }

  if (!pages_to_flush.empty()) {
    FlushBatch(pages_to_flush);
    stats_.forced_writes.fetch_add(1);
  }
}

LazyWriter::LazyWriterStats LazyWriter::GetStats() const {
  std::lock_guard<std::mutex> lock(stats_mutex_);
  return stats_;
}

void LazyWriter::ResetStats() {
  std::lock_guard<std::mutex> lock(stats_mutex_);
  stats_ = LazyWriterStats();
}

void LazyWriter::SetEnabled(bool enabled) { enabled_.store(enabled); }

bool LazyWriter::IsEnabled() const { return enabled_.load(); }

void LazyWriter::LazyWriteWorker() {
  while (running_.load()) {
    std::vector<DirtyPageInfo> pages_to_flush;

    {
      std::unique_lock<std::mutex> lock(flush_mutex_);
      // 等待触发条件或超时
      flush_cv_.wait_for(lock, flush_interval_, [this]() {
        std::lock_guard<std::mutex> dirty_lock(dirty_mutex_);
        return !running_.load() || ShouldFlush();
      });

      if (!running_.load()) {
        break;
      }

      // 获取需要刷新的页面
      pages_to_flush = GetPagesToFlush();
    }

    // 执行批量写入
    if (!pages_to_flush.empty()) {
      FlushBatch(pages_to_flush);
    }
  }
}

void LazyWriter::FlushBatch(const std::vector<DirtyPageInfo> &dirty_pages) {
  if (dirty_pages.empty()) {
    return;
  }

  auto start_time = std::chrono::high_resolution_clock::now();

  // 批量写入页面
  for (const auto &page_info : dirty_pages) {
    auto data_it = dirty_page_data_.find(page_info.page_id);
    if (data_it != dirty_page_data_.end()) {
      // 写入磁盘
      disk_manager_.WritePage(page_info.page_id, data_it->second.data());

      // 从脏页列表中移除
      {
        std::lock_guard<std::mutex> lock(dirty_mutex_);
        dirty_pages_.erase(page_info.page_id);
        dirty_page_data_.erase(page_info.page_id);
        stats_.current_dirty_pages.fetch_sub(1);
      }
    }
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

  // 更新统计信息
  {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.total_writes.fetch_add(1);
    stats_.total_pages_written.fetch_add(dirty_pages.size());
    stats_.total_bytes_written.fetch_add(dirty_pages.size() * 4096); // 假设4KB页面

    // 更新平均写入时间
    if (stats_.total_writes.load() == 1) {
      stats_.avg_write_time = duration;
    } else {
      auto total_time = stats_.avg_write_time * (stats_.total_writes.load() - 1) + duration;
      stats_.avg_write_time = total_time / stats_.total_writes.load();
    }
  }
}

bool LazyWriter::ShouldFlush() const {
  std::lock_guard<std::mutex> lock(dirty_mutex_);

  // 检查脏页数量
  if (dirty_pages_.size() >= max_dirty_pages_) {
    return true;
  }

  // 检查是否有页面超过最大脏页年龄
  auto now = std::chrono::steady_clock::now();
  for (const auto &pair : dirty_pages_) {
    auto age = now - pair.second.last_modified;
    if (age >= max_dirty_age_) {
      return true;
    }
  }

  return false;
}

void LazyWriter::UpdateConfig() {
  // 从配置管理器读取参数
  // 这里使用默认值，实际实现应从配置中读取
  flush_interval_ = std::chrono::milliseconds(100);
  max_dirty_pages_ = 1024;
  batch_size_ = 64;
  max_dirty_age_ = std::chrono::milliseconds(1000);
}

std::vector<DirtyPageInfo> LazyWriter::GetPagesToFlush() {
  std::lock_guard<std::mutex> lock(dirty_mutex_);

  std::vector<DirtyPageInfo> pages_to_flush;

  // 按修改时间排序，选择最旧的页面
  std::vector<std::pair<int32_t, DirtyPageInfo>> sorted_pages;
  for (const auto &pair : dirty_pages_) {
    if (!pair.second.is_pinned) {
      sorted_pages.emplace_back(pair.first, pair.second);
    }
  }

  // 按最后修改时间排序（最早修改的优先）
  std::sort(sorted_pages.begin(), sorted_pages.end(),
            [](const auto &a, const auto &b) {
              return a.second.last_modified < b.second.last_modified;
            });

  // 选择前batch_size个页面
  for (size_t i = 0; i < std::min(batch_size_, sorted_pages.size()); ++i) {
    pages_to_flush.push_back(sorted_pages[i].second);
  }

  return pages_to_flush;
}

} // namespace sqlcc
