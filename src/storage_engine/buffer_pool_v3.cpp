#include "storage/buffer_pool_v3.h" // Buffer pool implementation v3
#include "utils/logger.h"
#include <algorithm>
#include <thread>

namespace sqlcc {

BufferPool::BufferPool(std::shared_ptr<DiskManager> disk_manager,
                       ConfigManager &config_manager, size_t pool_size)
    : config_manager_(config_manager), disk_manager_(disk_manager),
      pool_size_(pool_size), next_page_id_(0), shutdown_(false) {

  // 初始化替换策略
  std::string strategy_name =
      config_manager_.GetString("buffer.replace_strategy", "LRU");
  auto strategy_type = ReplaceStrategyFactory::ParseStrategyType(strategy_name);
  replace_strategy_ = ReplaceStrategyFactory::CreateStrategy(
      strategy_type, config_manager_, pool_size_);

  // 初始化并发控制组件
  lock_manager_ = new HierarchicalLockManager(config_manager_);
  prefetcher_ = new Prefetcher(config_manager_, *disk_manager_);

  // 初始化统计信息
  stats_ = BufferPoolStats{};
  stats_.pool_size = pool_size_;
  stats_.allocated_pages = 0;
  stats_.dirty_pages = 0;
  stats_.pinned_pages = 0;
  stats_.hit_ratio = 0.0;
  stats_.total_requests = 0;
  stats_.cache_hits = 0;
  stats_.avg_access_time = std::chrono::microseconds(0);
  stats_.strategy_type = strategy_type;

  // 从磁盘管理器获取下一个页面ID
  if (disk_manager_) {
    next_page_id_ = disk_manager_->AllocatePage();
  }
}

BufferPool::~BufferPool() {
  // 标记为关闭状态
  shutdown_ = true;

  // 刷新所有脏页
  FlushAllPages();

  // 清理页面表
  {
    std::unique_lock<std::shared_mutex> lock(page_table_mutex_);
    page_table_.clear();
  }

  delete lock_manager_;
  delete prefetcher_;
}

std::shared_ptr<BufferPage> BufferPool::FetchPage(int32_t page_id,
                                                  int32_t transaction_id) {
  auto start_time = std::chrono::steady_clock::now();
  bool is_hit = false;

  // 更新统计信息
  {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.total_requests++;
  }

  // 检查页面是否在预取缓存中
  Page prefetch_page;
  if (prefetcher_ && prefetcher_->GetPrefetchedPage(page_id, prefetch_page)) {
    // 从预取缓存获取页面
    std::unique_lock<std::shared_mutex> lock(page_table_mutex_);

    // 检查是否已存在
    if (page_table_.find(page_id) != page_table_.end()) {
      auto &buffer_page = page_table_[page_id];
      buffer_page->data = prefetch_page;
      buffer_page->ref_count++;
      buffer_page->access_time = std::chrono::steady_clock::now();

      // 通知替换策略
      replace_strategy_->NotifyPageAccess(page_id, buffer_page->is_dirty);

      is_hit = true;

      return buffer_page;
    }

    // 检查是否有空间
    if (page_table_.size() >= pool_size_) {
      // 需要替换一个页面
      int32_t victim_id = -1;
      victim_id = replace_strategy_->SelectVictim();
      if (victim_id == -1) {
        // 没有可替换的页面
        return nullptr;
      }

      // 刷新脏页
      auto victim_it = page_table_.find(victim_id);
      if (victim_it != page_table_.end() && victim_it->second->is_dirty) {
        WritePageToDisk(victim_id);
      }

      // 从页面表中移除
      page_table_.erase(victim_id);
      replace_strategy_->NotifyPageRelease(victim_id);
    }

    // 添加新页面
    auto buffer_page = std::make_shared<BufferPage>(page_id);
    buffer_page->data = prefetch_page;
    buffer_page->ref_count = 1;
    buffer_page->access_time = std::chrono::steady_clock::now();

    page_table_[page_id] = buffer_page;

    // 通知替换策略
    replace_strategy_->NotifyPageAccess(page_id, false);

    is_hit = true;

    return buffer_page;
  }

  // 获取页面锁
  if (lock_manager_) {
    bool lock_acquired =
        lock_manager_->AcquirePageLock(transaction_id, page_id, false);
    if (!lock_acquired) {
      return nullptr;
    }
  }

  // 检查页面是否已在缓存中
  {
    std::shared_lock<std::shared_mutex> lock(page_table_mutex_);

    auto it = page_table_.find(page_id);
    if (it != page_table_.end()) {
      // 页面在缓存中
      auto &buffer_page = it->second;
      buffer_page->ref_count++;
      buffer_page->access_time = std::chrono::steady_clock::now();

      // 通知替换策略
      replace_strategy_->NotifyPageAccess(page_id, buffer_page->is_dirty);

      is_hit = true;

      // 释放页面锁
      if (lock_manager_) {
        lock_manager_->ReleasePageLock(transaction_id, page_id);
      }

      // 更新统计信息
      auto end_time = std::chrono::steady_clock::now();
      auto access_time = std::chrono::duration_cast<std::chrono::microseconds>(
          end_time - start_time);
      UpdateStats(is_hit, access_time);

      // 通知预取器
      if (prefetcher_) {
        prefetcher_->NotifyPageAccess(transaction_id, page_id);
      }

      return buffer_page;
    }
  }

  // 页面不在缓存中，需要从磁盘加载
  {
    std::unique_lock<std::shared_mutex> lock(page_table_mutex_);

    // 再次检查，防止其他线程已经加载了页面
    auto it = page_table_.find(page_id);
    if (it != page_table_.end()) {
      // 页面已被其他线程加载
      auto &buffer_page = it->second;
      buffer_page->ref_count++;
      buffer_page->access_time = std::chrono::steady_clock::now();

      // 通知替换策略
      replace_strategy_->NotifyPageAccess(page_id, buffer_page->is_dirty);

      // 释放页面锁
      if (lock_manager_) {
        lock_manager_->ReleasePageLock(transaction_id, page_id);
      }

      // 更新统计信息
      auto end_time = std::chrono::steady_clock::now();
      auto access_time = std::chrono::duration_cast<std::chrono::microseconds>(
          end_time - start_time);
      UpdateStats(is_hit, access_time);

      // 通知预取器
      if (prefetcher_) {
        prefetcher_->NotifyPageAccess(transaction_id, page_id);
      }

      return buffer_page;
    }

    // 检查是否有空间
    if (page_table_.size() >= pool_size_) {
      // 需要替换一个页面
      int32_t victim_id = -1;
      victim_id = replace_strategy_->SelectVictim();
      if (victim_id == -1) {
        // 没有可替换的页面
        if (lock_manager_) {
          lock_manager_->ReleasePageLock(transaction_id, page_id);
        }
        return nullptr;
      }

      // 刷新脏页
      auto victim_it = page_table_.find(victim_id);
      if (victim_it != page_table_.end() && victim_it->second->is_dirty) {
        WritePageToDisk(victim_id);
      }

      // 从页面表中移除
      page_table_.erase(victim_id);
      replace_strategy_->NotifyPageRelease(victim_id);
    }

    // 从磁盘加载页面
    Page page_data;
    if (!disk_manager_->ReadPage(page_id, page_data.GetData())) {
      // 加载失败
      if (lock_manager_) {
        lock_manager_->ReleasePageLock(transaction_id, page_id);
      }
      return nullptr;
    }

    // 创建新的缓冲页
    auto buffer_page = std::make_shared<BufferPage>(page_id);
    buffer_page->data = page_data;
    buffer_page->ref_count = 1;
    buffer_page->access_time = std::chrono::steady_clock::now();

    // 添加到页面表
    page_table_[page_id] = buffer_page;

    // 通知替换策略
    replace_strategy_->NotifyPageAccess(page_id, false);

    // 释放页面锁
    if (lock_manager_) {
      lock_manager_->ReleasePageLock(transaction_id, page_id);
    }

    // 更新统计信息
    auto end_time = std::chrono::steady_clock::now();
    auto access_time = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time);
    UpdateStats(is_hit, access_time);

    // 通知预取器
    if (prefetcher_) {
      prefetcher_->NotifyPageAccess(transaction_id, page_id);
    }

    return buffer_page;
  }
}

bool BufferPool::UnpinPage(int32_t page_id, int32_t transaction_id) {
  // 获取页面锁
  // 注意：HierarchicalLockManager类尚未实现，暂时禁用
  /*
  if (lock_manager_) {
    bool lock_acquired =
        lock_manager_->AcquirePageLock(transaction_id, page_id, true);
    if (!lock_acquired) {
      return nullptr;
    }
  }
  */

  std::unique_lock<std::shared_mutex> lock(page_table_mutex_);

  auto it = page_table_.find(page_id);
  if (it == page_table_.end()) {
    // 页面不在缓存中
    // 注意：HierarchicalLockManager类尚未实现，暂时禁用
    /*
    if (lock_manager_) {
      lock_manager_->ReleasePageLock(transaction_id, page_id);
    }
    */
    return false;
  }

  auto &buffer_page = it->second;
  if (buffer_page->ref_count > 0) {
    buffer_page->ref_count--;

    // 通知替换策略
    if (replace_strategy_) {
      replace_strategy_->NotifyPageRelease(page_id);
    }
  }

  // 释放页面锁
  // 注意：HierarchicalLockManager类尚未实现，暂时禁用
  /*
  if (lock_manager_) {
    lock_manager_->ReleasePageLock(transaction_id, page_id);
  }
  */

  return true;
}

int32_t BufferPool::NewPage(int32_t transaction_id) {
  // 分配新页面ID
  int32_t new_page_id;
  if (disk_manager_) {
    new_page_id = disk_manager_->AllocatePage();
  } else {
    // 如果没有磁盘管理器，使用内部计数器
    new_page_id = next_page_id_.fetch_add(1);
  }

  // 获取页面锁
  if (lock_manager_) {
    bool lock_acquired =
        lock_manager_->AcquirePageLock(transaction_id, new_page_id, true);
    if (!lock_acquired) {
      return -1;
    }
  }

  std::unique_lock<std::shared_mutex> lock(page_table_mutex_);

  // 检查是否有空间
  if (page_table_.size() >= pool_size_) {
    // 需要替换一个页面
    int32_t victim_id = -1;
    if (replace_strategy_) {
      victim_id = replace_strategy_->SelectVictim();
    }
    if (victim_id == -1) {
      // 没有可替换的页面
      if (lock_manager_) {
        lock_manager_->ReleasePageLock(transaction_id, new_page_id);
      }
      return -1;
    }

    // 刷新脏页
    auto victim_it = page_table_.find(victim_id);
    if (victim_it != page_table_.end() && victim_it->second->is_dirty) {
      WritePageToDisk(victim_id);
    }

    // 从页面表中移除
    page_table_.erase(victim_id);
    if (replace_strategy_) {
      replace_strategy_->NotifyPageRelease(victim_id);
    }
  }

  // 创建新页面
  auto buffer_page = std::make_shared<BufferPage>(new_page_id);
  buffer_page->data = Page(new_page_id); // 创建新页面数据
  buffer_page->ref_count = 0;   // 新页面初始引用计数为0，等待FetchPage调用
  buffer_page->is_dirty = true; // 新页面是脏页
  buffer_page->access_time = std::chrono::steady_clock::now();

  // 添加到页面表
  page_table_[new_page_id] = buffer_page;

  // 通知替换策略
  if (replace_strategy_) {
    replace_strategy_->NotifyPageAccess(new_page_id, true);
  }

  // 释放页面锁
  if (lock_manager_) {
    lock_manager_->ReleasePageLock(transaction_id, new_page_id);
  }

  return new_page_id;
}

bool BufferPool::FlushPage(int32_t page_id) {
  std::shared_lock<std::shared_mutex> lock(page_table_mutex_);

  auto it = page_table_.find(page_id);
  if (it == page_table_.end()) {
    // 页面不在缓存中，可能已经在磁盘上
    return true;
  }

  auto &buffer_page = it->second;
  if (!buffer_page->is_dirty) {
    // 页面不是脏页，不需要刷新
    return true;
  }

  // 写入磁盘
  bool success = WritePageToDisk(page_id);
  if (success) {
    buffer_page->is_dirty = false;
  }

  return success;
}

bool BufferPool::DeletePage(int32_t page_id) {
  // 获取页面锁
  if (lock_manager_) {
    bool lock_acquired = lock_manager_->AcquirePageLock(-1, page_id, true);
    if (!lock_acquired) {
      return false;
    }
  }

  std::unique_lock<std::shared_mutex> lock(page_table_mutex_);

  auto it = page_table_.find(page_id);
  if (it != page_table_.end()) {
    // 页面在缓存中，直接移除
    page_table_.erase(it);
    if (replace_strategy_) {
      replace_strategy_->NotifyPageRelease(page_id);
    }
  }

  // 从磁盘删除页面
  bool success = true;
  if (disk_manager_) {
    success = disk_manager_->DeallocatePage(page_id);
  }

  // 释放页面锁
  if (lock_manager_) {
    lock_manager_->ReleasePageLock(-1, page_id);
  }

  return success;
}

bool BufferPool::FlushAllPages() {
  std::unique_lock<std::shared_mutex> lock(page_table_mutex_);

  bool success = true;

  // 遍历所有页面，刷新脏页
  for (auto &pair : page_table_) {
    int32_t page_id = pair.first;
    auto &buffer_page = pair.second;

    if (buffer_page->is_dirty) {
      if (!WritePageToDisk(page_id)) {
        success = false;
      } else {
        buffer_page->is_dirty = false;
      }
    }
  }

  return success;
}

bool BufferPool::Resize(size_t new_pool_size) {
  if (new_pool_size == pool_size_) {
    return true; // 大小相同，无需调整
  }

  std::unique_lock<std::shared_mutex> lock(page_table_mutex_);

  if (new_pool_size < page_table_.size()) {
    // 新大小小于当前页面数，需要替换一些页面
    size_t pages_to_remove = page_table_.size() - new_pool_size;

    for (size_t i = 0; i < pages_to_remove; ++i) {
      int32_t victim_id = -1;
      if (replace_strategy_) {
        victim_id = replace_strategy_->SelectVictim();
      }

      if (victim_id == -1) {
        // 没有可替换的页面，无法缩小
        // 当replace_strategy_为空时，我们无法选择替换页面
        // 因此直接返回false
        return false;
      }

      // 刷新脏页
      auto victim_it = page_table_.find(victim_id);
      if (victim_it != page_table_.end() && victim_it->second->is_dirty) {
        WritePageToDisk(victim_id);
      }

      // 从页面表中移除
      page_table_.erase(victim_id);
      if (replace_strategy_) {
        replace_strategy_->NotifyPageRelease(victim_id);
      }
    }
  }

  // 更新池大小
  pool_size_ = new_pool_size;

  // 更新替换策略容量
  if (replace_strategy_) {
    // AbstractReplaceStrategy没有UpdateCapacity方法
    // 但可以通过max_pages_成员变量更新容量
    // 这需要具体的策略实现支持动态调整容量
    // 这里我们仅记录日志，实际容量调整在策略内部处理
    SQLCC_LOG_INFO("Buffer pool resized to " + std::to_string(new_pool_size) +
                   " pages");
  }

  // 更新统计信息
  {
    std::lock_guard<std::mutex> stats_lock(stats_mutex_);
    stats_.pool_size = pool_size_;
  }

  return true;
}

BufferPool::BufferPoolStats BufferPool::GetStats() const {
  std::lock_guard<std::mutex> lock(stats_mutex_);

  BufferPoolStats current_stats = stats_;

  // 更新动态统计信息
  {
    std::shared_lock<std::shared_mutex> page_lock(page_table_mutex_);

    current_stats.allocated_pages = page_table_.size();
    current_stats.dirty_pages = 0;
    current_stats.pinned_pages = 0;

    for (const auto &pair : page_table_) {
      const auto &buffer_page = pair.second;
      if (buffer_page->is_dirty) {
        current_stats.dirty_pages++;
      }
      if (buffer_page->ref_count > 0) {
        current_stats.pinned_pages++;
      }
    }
  }

  // 计算命中率
  if (current_stats.total_requests > 0) {
    current_stats.hit_ratio = static_cast<double>(current_stats.cache_hits) /
                              current_stats.total_requests;
  }

  // 获取替换策略统计
  if (replace_strategy_) {
    current_stats.strategy_stats = replace_strategy_->GetStats();
  }

  // 获取锁管理器统计
  if (lock_manager_) {
    current_stats.lock_stats = lock_manager_->GetStats();
  }

  // 获取预取器统计
  if (prefetcher_) {
    current_stats.prefetch_stats = prefetcher_->GetStats();
  }

  return current_stats;
}

void BufferPool::ResetStats() {
  std::lock_guard<std::mutex> lock(stats_mutex_);

  stats_.total_requests = 0;
  stats_.cache_hits = 0;
  stats_.hit_ratio = 0.0;
  stats_.avg_access_time = std::chrono::microseconds(0);

  // 重置替换策略统计
  if (replace_strategy_) {
    replace_strategy_->Reset();
  }

  // 重置锁管理器统计
  if (lock_manager_) {
    lock_manager_->ResetStats();
  }

  // 重置预取器统计
  if (prefetcher_) {
    prefetcher_->ResetStats();
  }
}

bool BufferPool::ChangeReplaceStrategy(
    ReplaceStrategyFactory::StrategyType strategy_type) {
  std::unique_lock<std::shared_mutex> lock(page_table_mutex_);

  // 创建新的替换策略
  auto new_strategy = ReplaceStrategyFactory::CreateStrategy(
      strategy_type, config_manager_, pool_size_);

  // 初始化新策略，添加所有现有页面
  for (const auto &pair : page_table_) {
    int32_t page_id = pair.first;
    const auto &buffer_page = pair.second;

    new_strategy->NotifyPageAccess(page_id, buffer_page->is_dirty);
  }

  // 替换策略
  replace_strategy_ = std::move(new_strategy);

  // 更新统计信息
  {
    std::lock_guard<std::mutex> stats_lock(stats_mutex_);
    stats_.strategy_type = strategy_type;
  }

  return true;
}

void BufferPool::SetPrefetcherEnabled(bool enabled) {
  if (prefetcher_) {
    prefetcher_->SetEnabled(enabled);
  }
}

bool BufferPool::LoadPageFromDisk(int32_t page_id) {
  if (!disk_manager_) {
    return false;
  }

  Page page_data;
  if (!disk_manager_->ReadPage(page_id, page_data.GetData())) {
    return false;
  }

  // 创建新的缓冲页
  auto buffer_page = std::make_shared<BufferPage>(page_id);
  buffer_page->data = page_data;
  buffer_page->ref_count = 0;
  buffer_page->access_time = std::chrono::steady_clock::now();

  // 添加到页面表
  page_table_[page_id] = buffer_page;

  return true;
}

bool BufferPool::WritePageToDisk(int32_t page_id) {
  if (!disk_manager_) {
    return false;
  }

  auto it = page_table_.find(page_id);
  if (it == page_table_.end()) {
    return false;
  }

  return disk_manager_->WritePage(page_id, it->second->data.GetData());
}

int32_t BufferPool::EvictPage() {
  if (replace_strategy_) {
    return replace_strategy_->SelectVictim();
  }
  return -1;
}

bool BufferPool::AddPageToPool(int32_t page_id) {
  // 检查是否已存在
  if (page_table_.find(page_id) != page_table_.end()) {
    return false;
  }

  // 检查是否有空间
  if (page_table_.size() >= pool_size_) {
    // 需要替换一个页面
    int32_t victim_id = -1;
    if (replace_strategy_) {
      victim_id = replace_strategy_->SelectVictim();
    }
    if (victim_id == -1) {
      return false;
    }

    // 刷新脏页
    auto victim_it = page_table_.find(victim_id);
    if (victim_it != page_table_.end() && victim_it->second->is_dirty) {
      WritePageToDisk(victim_id);
    }

    // 从页面表中移除
    page_table_.erase(victim_id);
    if (replace_strategy_) {
      replace_strategy_->NotifyPageRelease(victim_id);
    }
  }

  // 从磁盘加载页面
  return LoadPageFromDisk(page_id);
}

bool BufferPool::RemovePageFromPool(int32_t page_id) {
  auto it = page_table_.find(page_id);
  if (it == page_table_.end()) {
    return false;
  }

  // 如果是脏页，先刷新
  if (it->second->is_dirty) {
    WritePageToDisk(page_id);
  }

  // 从页面表中移除
  page_table_.erase(it);
  if (replace_strategy_) {
    replace_strategy_->NotifyPageRelease(page_id);
  }

  return true;
}

void BufferPool::UpdateStats(bool is_hit,
                             std::chrono::microseconds access_time) {
  std::lock_guard<std::mutex> lock(stats_mutex_);

  if (is_hit) {
    stats_.cache_hits++;
  }

  // 更新平均访问时间
  if (stats_.total_requests == 1) {
    stats_.avg_access_time = access_time;
  } else {
    auto total_time =
        stats_.avg_access_time * (stats_.total_requests - 1) + access_time;
    stats_.avg_access_time = total_time / stats_.total_requests;
  }

  // 计算命中率
  stats_.hit_ratio =
      static_cast<double>(stats_.cache_hits) / stats_.total_requests;
}

} // namespace sqlcc