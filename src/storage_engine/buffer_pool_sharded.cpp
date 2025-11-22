#include "buffer_pool_sharded.h"
#include "exception.h"
#include "logger.h"

namespace sqlcc {

BufferPoolSharded::BufferPoolSharded(DiskManager *disk_manager,
                                     ConfigManager &config_manager,
                                     size_t pool_size, size_t num_shards)
    : disk_manager_(disk_manager), config_manager_(config_manager),
      pool_size_(pool_size), next_page_id_(0) {
  // 确保num_shards是2的幂
  if (num_shards & (num_shards - 1)) {
    // 找到最接近的2的幂
    num_shards_ = 1;
    while (num_shards_ < num_shards) {
      num_shards_ <<= 1;
    }
    SQLCC_LOG_INFO("Adjusting shard count to power of 2: " +
                   std::to_string(num_shards_));
  } else {
    num_shards_ = num_shards;
  }

  // 初始化shards
  size_t shard_size = pool_size_ / num_shards_;
  shards_.resize(num_shards_);
  for (size_t i = 0; i < num_shards_; ++i) {
    shards_[i] = std::make_unique<Shard>(shard_size);
  }

  SQLCC_LOG_INFO("Sharded BufferPool initialized with " +
                 std::to_string(num_shards_) + " shards, each with " +
                 std::to_string(shard_size) + " pages");
}

BufferPoolSharded::~BufferPoolSharded() {
  SQLCC_LOG_INFO("Destroying Sharded BufferPool");
  FlushAllPages();
}

Page *BufferPoolSharded::FetchPage(int32_t page_id, bool exclusive) {
  // 避免未使用参数警告
  (void)page_id;
  (void)exclusive;

  // TODO: 实现页面获取逻辑
  return nullptr;
}

bool BufferPoolSharded::FlushPage(int32_t page_id) {
  size_t shard_idx = GetShardIndex(page_id);
  Shard &shard = *shards_[shard_idx];

  std::lock_guard<std::mutex> lock(shard.mutex);

  auto it = shard.page_table.find(page_id);
  if (it == shard.page_table.end()) {
    return false;
  }

  std::shared_ptr<PageWrapper> page_wrapper = it->second;
  if (!page_wrapper->is_dirty) {
    return true;
  }

  // 释放锁，执行磁盘I/O
  lock.~lock_guard();

  bool write_success = disk_manager_->WritePage(
      page_id, static_cast<char *>(page_wrapper->page->GetData()));

  // 重新获取锁
  std::lock_guard<std::mutex> lock2(shard.mutex);

  if (write_success) {
    page_wrapper->is_dirty = false;
  }

  return write_success;
}

void BufferPoolSharded::FlushAllPages() {
  for (size_t i = 0; i < num_shards_; ++i) {
    Shard &shard = *shards_[i];
    std::lock_guard<std::mutex> lock(shard.mutex);

    for (const auto &pair : shard.page_table) {
      int32_t page_id = pair.first;
      std::shared_ptr<PageWrapper> page_wrapper = pair.second;

      if (page_wrapper->is_dirty) {
        // 释放锁，执行磁盘I/O
        lock.~lock_guard();
        disk_manager_->WritePage(
            page_id, static_cast<char *>(page_wrapper->page->GetData()));
        // 重新获取锁
        std::lock_guard<std::mutex> lock2(shard.mutex);
        page_wrapper->is_dirty = false;
      }
    }
  }
}

bool BufferPoolSharded::UnpinPage(int32_t page_id, bool is_dirty) {
  size_t shard_idx = GetShardIndex(page_id);
  Shard &shard = *shards_[shard_idx];

  std::lock_guard<std::mutex> lock(shard.mutex);

  auto it = shard.page_table.find(page_id);
  if (it == shard.page_table.end()) {
    return false;
  }

  std::shared_ptr<PageWrapper> page_wrapper = it->second;
  if (page_wrapper->ref_count > 0) {
    page_wrapper->ref_count--;
  }

  if (is_dirty) {
    page_wrapper->is_dirty = true;
  }

  return true;
}

Page *BufferPoolSharded::NewPage(int32_t *page_id) {
  // 分配新的页面ID
  int32_t new_page_id = next_page_id_++;

  // 获取对应的shard
  size_t shard_idx = GetShardIndex(new_page_id);
  Shard &shard = *shards_[shard_idx];

  std::lock_guard<std::mutex> lock(shard.mutex);

  // 如果shard已满，需要替换页面
  if (shard.current_size >= shard.max_size) {
    int32_t replaced_page_id = ReplacePage(shard);
    if (replaced_page_id == -1) {
      SQLCC_LOG_ERROR("Failed to replace page for new page creation");
      return nullptr;
    }
  }

  // 创建新页面
  auto page = std::make_unique<Page>(new_page_id);
  Page *page_ptr = page.release();

  auto page_wrapper = std::make_shared<PageWrapper>(page_ptr);
  page_wrapper->ref_count = 1;
  page_wrapper->is_dirty = false;

  shard.page_table[new_page_id] = page_wrapper;
  shard.lru_list.push_front(new_page_id);
  shard.lru_map[new_page_id] = shard.lru_list.begin();
  page_wrapper->is_in_lru = true;
  shard.current_size++;

  // 记录已分配的页面
  {
    std::lock_guard<std::mutex> alloc_lock(allocated_pages_mutex_);
    allocated_pages_.insert(new_page_id);
  }

  *page_id = new_page_id;
  return page_ptr;
}

bool BufferPoolSharded::DeletePage(int32_t page_id) {
  size_t shard_idx = GetShardIndex(page_id);
  Shard &shard = *shards_[shard_idx];

  std::lock_guard<std::mutex> lock(shard.mutex);

  auto it = shard.page_table.find(page_id);
  if (it == shard.page_table.end()) {
    return false;
  }

  std::shared_ptr<PageWrapper> page_wrapper = it->second;
  if (page_wrapper->ref_count > 0) {
    return false; // 页面正在被使用
  }

  RemoveFromLRU(shard, page_id);
  shard.page_table.erase(it);
  shard.current_size--;

  // 从已分配页面集合中移除
  {
    std::lock_guard<std::mutex> alloc_lock(allocated_pages_mutex_);
    allocated_pages_.erase(page_id);
  }

  return true;
}

int32_t BufferPoolSharded::ReplacePage(Shard &shard) {
  for (auto it = shard.lru_list.rbegin(); it != shard.lru_list.rend(); ++it) {
    int32_t page_id = *it;
    auto page_it = shard.page_table.find(page_id);

    if (page_it != shard.page_table.end()) {
      std::shared_ptr<PageWrapper> page_wrapper = page_it->second;

      if (page_wrapper->ref_count == 0) {
        // 释放锁，执行磁盘I/O（如果需要）
        std::unique_lock<std::mutex> lock(shard.mutex, std::adopt_lock);
        lock.unlock();

        if (page_wrapper->is_dirty) {
          disk_manager_->WritePage(
              page_id, static_cast<char *>(page_wrapper->page->GetData()));
        }

        // 重新获取锁
        lock.lock();

        // 再次检查，避免在释放锁期间状态改变
        if (page_wrapper->ref_count == 0) {
          RemoveFromLRU(shard, page_id);
          shard.page_table.erase(page_it);
          shard.current_size--;
          stats_.total_evictions++;
          return page_id;
        }
      }
    }
  }

  return -1; // 无法找到可替换的页面
}

void BufferPoolSharded::MoveToHead(Shard &shard, int32_t page_id) {
  auto map_it = shard.lru_map.find(page_id);
  if (map_it != shard.lru_map.end()) {
    shard.lru_list.erase(map_it->second);
  }

  shard.lru_list.push_front(page_id);
  shard.lru_map[page_id] = shard.lru_list.begin();
}

void BufferPoolSharded::RemoveFromLRU(Shard &shard, int32_t page_id) {
  auto map_it = shard.lru_map.find(page_id);
  if (map_it != shard.lru_map.end()) {
    shard.lru_list.erase(map_it->second);
    shard.lru_map.erase(map_it);
  }

  auto page_it = shard.page_table.find(page_id);
  if (page_it != shard.page_table.end()) {
    page_it->second->is_in_lru = false;
  }
}

size_t BufferPoolSharded::GetCurrentPageCount() const {
  size_t total_count = 0;
  (void)total_count; // 避免未使用变量警告
  return 0;
}

std::unordered_map<std::string, double> BufferPoolSharded::GetStats() const {
  std::unordered_map<std::string, double> stats;
  stats["total_accesses"] = static_cast<double>(stats_.total_accesses);
  stats["total_hits"] = static_cast<double>(stats_.total_hits);
  stats["total_misses"] = static_cast<double>(stats_.total_misses);
  stats["total_evictions"] = static_cast<double>(stats_.total_evictions);

  if (stats_.total_accesses > 0) {
    stats["hit_rate"] =
        static_cast<double>(stats_.total_hits) / stats_.total_accesses;
  } else {
    stats["hit_rate"] = 0.0;
  }

  stats["current_page_count"] = GetCurrentPageCount();
  stats["pool_size"] = static_cast<double>(pool_size_);
  stats["num_shards"] = static_cast<double>(num_shards_);

  return stats;
}

} // namespace sqlcc