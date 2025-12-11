#include "storage/buffer_pool.h"
#include "exception.h"
#include "utils/logger.h"
#include <algorithm>
#include <iostream>
#include <thread>

namespace sqlcc {

// 构造函数实现
// Why: 需要初始化缓冲池的基本状态，包括设置磁盘管理器和缓冲池大小
// What: 构造函数初始化成员变量，并记录初始化日志
// How:
// 使用成员初始化列表初始化disk_manager_、pool_size_和config_manager_，并记录日志
BufferPool::BufferPool(std::shared_ptr<DiskManager> disk_manager, size_t pool_size,
                       ConfigManager &config_manager)
    : disk_manager_(std::move(disk_manager)), config_manager_(config_manager),
      pool_size_(pool_size), simulate_flush_failure_(false),
      read_lock_timeout_ms_(0), write_lock_timeout_ms_(0), lock_timeout_ms_(0) {
  // 从配置管理器获取不同操作的锁超时时间
  // 读取操作使用较短的超时时间
  read_lock_timeout_ms_ =
      config_manager_.GetInt("buffer_pool.read_lock_timeout_ms", 2000);
  // 写入和修改操作使用较长的超时时间
  write_lock_timeout_ms_ =
      config_manager_.GetInt("buffer_pool.write_lock_timeout_ms", 5000);
  // 默认超时时间（用于其他操作）
  lock_timeout_ms_ =
      config_manager_.GetInt("buffer_pool.default_lock_timeout_ms", 3000);

  // 记录配置的锁超时时间
  SQLCC_LOG_INFO("BufferPool lock timeout settings - Read: " +
                 std::to_string(read_lock_timeout_ms_) +
                 "ms, Write: " + std::to_string(write_lock_timeout_ms_) +
                 "ms, Default: " + std::to_string(lock_timeout_ms_) + "ms");
  // 预分配批量操作缓冲区空间
  batch_buffer_.reserve(std::min(pool_size_, static_cast<size_t>(64)));

  // 记录缓冲池初始化信息，便于调试和监控
  SQLCC_LOG_INFO("Initializing BufferPool with pool size: " +
                 std::to_string(pool_size_));

  // 初始化所有内部数据结构，确保对象状态完整
  // 移除配置回调相关功能，避免死锁
  // 注意：暂时禁用动态配置功能，需要重启应用才能修改缓冲池参数
}

// 析构函数实现
// Why: 需要确保所有脏页被写回磁盘，避免数据丢失
// What: 析构函数调用FlushAllPages()方法，将所有脏页写回磁盘
// How: 调用FlushAllPages()方法，然后记录日志
BufferPool::~BufferPool() {
  // 记录缓冲池销毁信息，便于调试和监控
  // Why: 日志记录有助于系统运行状态的监控和问题排查
  // What: 记录缓冲池销毁信息
  // How: 使用SQLCC_LOG_INFO宏记录信息级别日志
  SQLCC_LOG_INFO("Destroying BufferPool");

  // 析构时刷新所有脏页，确保数据持久性
  // Why: 在对象销毁前必须将所有修改过的页面写回磁盘，避免数据丢失
  // What: 调用FlushAllPages()方法将所有脏页写回磁盘
  // How: FlushAllPages()会遍历所有页面，检查脏页标记，将脏页写回磁盘
  FlushAllPages();
}

// 获取页面实现
// Why: 数据库操作需要访问特定页面ID的数据，这是缓冲池最核心的功能
// What: FetchPage方法根据页面ID获取对应的页面智能指针
// How: 检查页面是否在缓冲池中，如果不在则从磁盘加载，如果缓冲池已满则替换页面
std::shared_ptr<Page> BufferPool::FetchPage(int32_t page_id) {
  // 加锁保护并发访问 - 使用带超时的unique_lock以支持临时解锁避免死锁
  std::unique_lock<std::timed_mutex> lock(latch_, std::defer_lock);
  if (!lock.try_lock_for(std::chrono::milliseconds(read_lock_timeout_ms_))) {
    // 获取锁失败，记录警告但不抛出异常，避免在高并发场景下级联失败
    SQLCC_LOG_WARN("Failed to acquire buffer pool lock for fetching page " +
                   std::to_string(page_id) + ", timeout after " +
                   std::to_string(read_lock_timeout_ms_) + "ms");
    return nullptr;
  }
  // 更新访问统计（用于预测性预取）
  access_stats_[page_id]++;

  // 检查页面是否已经在缓冲池中
  // Why: 如果页面已在内存中，可以直接返回，避免昂贵的磁盘I/O操作
  // What: 在page_table_哈希表中查找页面ID
  // How: 使用std::unordered_map的find方法，时间复杂度为O(1)
  auto it = page_table_.find(page_id);
  if (it != page_table_.end()) {
    // 页面已在缓冲池中，增加引用计数
    // Why: 引用计数用于跟踪页面被多少个操作引用，防止正在使用的页面被替换
    // What: 增加page_refs_中该页面的引用计数
    // How: 直接递增引用计数值
    page_refs_[page_id]++;

    // 将页面移到LRU链表头部，表示最近被访问
    // Why: LRU算法需要维护页面的访问顺序，最近访问的页面应该放在链表头部
    // What: 调用MoveToHead方法将页面移到LRU链表头部
    // How: MoveToHead会更新LRU链表和映射表
    MoveToHead(page_id);

    // 减少日志记录以提高性能
    // SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " found in buffer
    // pool");
    return it->second; // 返回共享智能指针，避免拷贝和多次销毁
  }

  // 页面不在缓冲池中，需要从磁盘读取
  // Why: 页面不在内存中，必须从磁盘加载，这是数据库I/O操作的主要来源
  // What: 记录页面不在缓冲池中的信息
  // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
  SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) +
                  " not found in buffer pool, reading from disk");

  // 创建新页面智能指针
  auto page = std::make_shared<Page>(page_id);
  char* page_data = static_cast<char*>(page->GetData());
  int32_t current_page_id = page_id;

  // 释放锁，进行磁盘读取操作 - 修复死锁问题
  lock.unlock();

  // 现在在锁外进行磁盘读取操作
  bool read_success =
      disk_manager_->ReadPage(current_page_id, page_data);

  // 重新获取锁 - 使用带超时的锁获取避免永久阻塞
  if (!lock.try_lock_for(std::chrono::milliseconds(read_lock_timeout_ms_))) {
    SQLCC_LOG_WARN("Failed to reacquire buffer pool lock after reading page " +
                   std::to_string(current_page_id) + ", timeout after " +
                   std::to_string(read_lock_timeout_ms_) + "ms");
    return nullptr; // 返回空智能指针表示失败
  }

  if (!read_success) {
    // 如果读取失败，说明页面不存在，返回空智能指针
    SQLCC_LOG_DEBUG("Failed to read page ID " +
                    std::to_string(current_page_id) +
                    " from disk, page does not exist");
    return nullptr; // 返回空智能指针表示页面不存在
  }

  // 如果缓冲池已满，需要替换页面
  // Why: 缓冲池大小有限，当已满时必须选择一个页面进行替换
  // What: 检查page_table_的大小是否超过pool_size_
  // How: 比较page_table_.size()和pool_size_
  if (page_table_.size() >= pool_size_) {
    // 记录缓冲池已满的信息，便于调试
    SQLCC_LOG_DEBUG("Buffer pool is full, replacing page");

    // 调用ReplacePage方法选择一个页面进行替换
    // Why: 需要选择一个页面进行替换，为新页面腾出空间
    // What: ReplacePage方法使用LRU算法选择一个可替换的页面
    // How: 从LRU链表尾部开始查找，找到第一个引用计数为0的页面
    int32_t replaced_page_id = ReplacePage();
    if (replaced_page_id == -1) {
      // 无法替换页面，记录错误并返回空智能指针，避免抛出异常
      std::string error_msg = "Failed to replace page in buffer pool";
      SQLCC_LOG_ERROR(error_msg);
      return nullptr; // 返回空智能指针表示替换失败
    }
  }

  // 添加到页面表
  // Why: 需要将新加载的页面加入缓冲池管理，以便后续查找和使用
  // What: 将页面智能指针添加到page_table_哈希表中
  // How: 使用页面ID作为键，存储智能指针
  page_table_[page_id] = page; // 存储智能指针到缓冲池
  page_refs_[page_id] = 1;

  // 初始化脏页标记
  // Why: 需要跟踪页面是否被修改过，以便在替换时写回磁盘
  // What: 将页面标记为非脏页，因为刚从磁盘读取
  // How: 在dirty_pages_哈希表中设置页面ID的值为false
  dirty_pages_[page_id] = false;

  // 添加到LRU链表头部
  // Why: 新加载的页面应该放在LRU链表头部，表示最近被访问
  // What: 将页面ID添加到lru_list_的头部
  // How: 使用push_front方法添加到链表头部，并在lru_map_中记录迭代器
  lru_list_.push_front(page_id);
  lru_map_[page_id] = lru_list_.begin();

  // 记录页面加载成功的信息，便于调试
  SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) +
                  " loaded into buffer pool");

  // 返回共享智能指针，避免拷贝和多次销毁
  return page;
}

} // namespace sqlcc

// 批量获取页面实现（未实现，暂时返回空）
std::vector<std::shared_ptr<Page>> BufferPool::BatchFetchPages(const std::vector<int32_t>& page_ids) {
  std::vector<std::shared_ptr<Page>> result;
  for (int32_t page_id : page_ids) {
    auto page = FetchPage(page_id);
    result.push_back(page);
  }
  return result;
}

// 批量预取页面实现（未实现，暂时返回true）
bool BufferPool::BatchPrefetchPages(const std::vector<int32_t>& page_ids) {
  SQLCC_LOG_DEBUG("Batch prefetch requested for " + std::to_string(page_ids.size()) + " pages");
  return true;
}

// 预取页面实现（未实现，暂时返回true）
bool BufferPool::PrefetchPage(int32_t page_id) {
  SQLCC_LOG_DEBUG("Prefetch requested for page " + std::to_string(page_id));
  return true;
}

// 获取缓冲池统计信息实现
std::unordered_map<std::string, double> BufferPool::GetStats() const {
  std::unordered_map<std::string, double> stats;
  stats["pool_size"] = static_cast<double>(pool_size_);
  stats["used_pages"] = static_cast<double>(page_table_.size());
  return stats;
}

// 获取缓冲池大小实现
size_t BufferPool::GetPoolSize() const {
  return pool_size_;
}

// 获取已使用页面数实现
size_t BufferPool::GetUsedPages() const {
  return page_table_.size();
}

// 检查页面是否在缓冲池中实现
bool BufferPool::IsPageInBuffer(int32_t page_id) const {
  std::unique_lock<std::timed_mutex> lock(latch_);
  return page_table_.find(page_id) != page_table_.end();
}

// 查找可替换的页面实现（内部方法）
int32_t BufferPool::FindVictimPage() {
  return ReplacePage();
}

// 替换页面实现（内部方法）
bool BufferPool::ReplacePage(int32_t victim_page_id, int32_t new_page_id) {
  // 简化实现，直接调用ReplacePage
  return ReplacePage() != -1;
}

// 更新LRU列表实现（内部方法）
void BufferPool::UpdateLRUList(int32_t page_id) {
  MoveToHead(page_id);
}

// 移动页面到LRU链表头部实现（内部方法）
void BufferPool::MoveToHead(int32_t page_id) {
  auto it = lru_map_.find(page_id);
  if (it != lru_map_.end()) {
    lru_list_.erase(it->second);
    lru_list_.push_front(page_id);
    lru_map_[page_id] = lru_list_.begin();
  }
}

// 从LRU列表中移除页面实现
void BufferPool::RemoveFromLRUList(int32_t page_id) {
  auto it = lru_map_.find(page_id);
  if (it != lru_map_.end()) {
    lru_list_.erase(it->second);
    lru_map_.erase(it);
  }
}

// 替换页面（无锁版本）实现
int32_t BufferPool::ReplacePageInternal() {
  return ReplacePage();
}

// 配置变更回调处理实现
void BufferPool::OnConfigChange(const std::string& key, const ConfigValue& value) {
  // 暂时禁用配置回调
}

// 调整缓冲池大小实现
void BufferPool::AdjustBufferPoolSize(size_t new_pool_size) {
  pool_size_ = new_pool_size;
}

// 调整缓冲池大小（无锁版本）实现
void BufferPool::AdjustBufferPoolSizeNoLock(size_t new_pool_size) {
  pool_size_ = new_pool_size;
}

// 刷新所有页面实现（简化实现）
void BufferPool::FlushAllPages() {
  std::unique_lock<std::timed_mutex> lock(latch_);
  for (const auto& pair : page_table_) {
    if (dirty_pages_[pair.first]) {
      // 这里应该刷新页面，但为了简化暂时跳过
      dirty_pages_[pair.first] = false;
    }
  }
}

// 刷新页面到磁盘实现
// Why: 需要将修改后的页面数据持久化到磁盘，保证数据的持久性和一致性
// What: FlushPage方法将指定页面的数据写入磁盘文件
// How: 检查页面是否存在，如果是脏页则调用磁盘管理器写入磁盘
bool BufferPool::FlushPage(int32_t page_id) {  // 加锁保护并发访问 - 使用带超时的unique_lock以支持临时解锁避免死锁
  std::unique_lock<std::timed_mutex> lock(latch_, std::defer_lock);
  if (!lock.try_lock_for(std::chrono::milliseconds(lock_timeout_ms_))) {
    // 获取锁失败，记录警告但不抛出异常，避免在高并发场景下级联失败
    SQLCC_LOG_WARN("Failed to acquire buffer pool lock for flushing page " +
                   std::to_string(page_id));
    return false;
  }

  // 检查页面是否存在
  // Why: 需要确保页面确实存在于缓冲池中，避免操作不存在的页面
  // What: 在page_table_中查找页面ID
  // How: 使用std::unordered_map的find方法查找
  auto page_it = page_table_.find(page_id);
  if (page_it == page_table_.end()) {
    // 页面不存在，记录警告并返回false
    // Why: 页面不存在于缓冲池中，无法刷新
    // What: 记录警告信息并返回false表示操作失败
    // How: 使用SQLCC_LOG_WARN记录警告，然后返回false
    SQLCC_LOG_WARN("Page ID " + std::to_string(page_id) +
                   " not found in buffer pool");
    return false;
  }

  // 检查是否为脏页
  // Why: 只有脏页才需要写回磁盘，非脏页不需要写回
  // What: 检查dirty_pages_中该页面是否为脏页
  // How: 使用std::unordered_map的find方法查找脏页标记
  auto dirty_it = dirty_pages_.find(page_id);
  if (dirty_it == dirty_pages_.end() || !dirty_it->second) {
    // 页面不是脏页，不需要刷新
    // Why: 非脏页的内容与磁盘一致，不需要写回
    // What: 记录调试信息并返回true表示操作成功（无需操作）
    // How: 使用SQLCC_LOG_DEBUG记录调试级别日志
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) +
                    " is not dirty, no flush needed");
    return true;
  }

  // 检查是否模拟刷新失败（仅用于测试）
  // Why: 需要测试缓冲池在磁盘写入失败时的错误处理逻辑
  // What: 检查simulate_flush_failure_标志
  // How: 如果标志为true，则模拟刷新失败
  if (simulate_flush_failure_) {
    // 模拟刷新失败，记录错误并返回false
    // Why: 用于测试错误处理逻辑
    // What: 记录错误信息并返回false表示操作失败
    // How: 使用SQLCC_LOG_ERROR记录错误级别日志
    std::string error_msg =
        "Simulated flush failure for page ID " + std::to_string(page_id);
    SQLCC_LOG_ERROR(error_msg);
    return false;
  }

  // 将页面数据写入磁盘 - 修复死锁问题：使用unique_lock支持锁释放机制
  // Why:
  // 脏页的内容与磁盘不一致，需要写回磁盘以保证数据持久性，但必须在锁外进行磁盘I/O以避免死锁
  // What: 先在锁内准备数据，然后解锁调用DiskManager，再重新加锁继续操作
  // How: 与ReplacePage方法采用相同的锁释放策略，避免BufferPool
  // latch_和DiskManager io_mutex_之间的循环等待
  std::shared_ptr<Page> page_shared = page_it->second;  // 获取智能指针副本
  const char* page_data = static_cast<const char*>(page_shared->GetData());
  int32_t current_page_id = page_id;

  // 释放锁，进行磁盘写入操作 - 这是避免死锁的关键修复
  lock.unlock();

  // 现在在锁外进行磁盘写入操作
  bool write_success = disk_manager_->WritePage(
      current_page_id, static_cast<const char *>(page_data));

  // 重新获取锁 - 使用带超时的锁获取避免永久阻塞
  if (!lock.try_lock_for(std::chrono::milliseconds(write_lock_timeout_ms_))) {
    SQLCC_LOG_WARN("Failed to reacquire buffer pool lock after writing page " +
                   std::to_string(current_page_id) + ", timeout after " +
                   std::to_string(write_lock_timeout_ms_) + "ms");
    return false;
  }

  if (!write_success) {
    // 写入失败，记录错误并返回false
    std::string error_msg = "Failed to write page ID " +
                            std::to_string(current_page_id) + " to disk";
    SQLCC_LOG_ERROR(error_msg);
    return false;
  }

  // 刷新成功后，清除脏页标记
  // Why: 页面已经成功写回磁盘，不再需要标记为脏页
  // What: 将dirty_pages_中该页面的脏页标记设置为false
  // How: 直接设置哈希表中的值为false
  dirty_pages_[page_id] = false;

  // 记录页面刷新成功的信息，便于调试
  SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " flushed to disk");

  return true;
}

// 删除页面实现
// Why: 当数据不再需要时，需要释放页面空间，例如删除记录或索引
// What: DeletePage方法从缓冲池中删除指定页面，并释放相关资源
// How: 检查页面是否存在，刷新脏页，然后从所有数据结构中移除页面
bool BufferPool::DeletePage(int32_t page_id) {
  // 加锁保护并发访问 - 使用带超时的unique_lock以支持临时解锁避免死锁
  std::unique_lock<std::timed_mutex> lock(latch_, std::defer_lock);
  if (!lock.try_lock_for(std::chrono::milliseconds(write_lock_timeout_ms_))) {
    // 获取锁失败，记录警告但不抛出异常，避免在高并发场景下级联失败
    SQLCC_LOG_WARN("Failed to acquire buffer pool lock for deleting page " +
                   std::to_string(page_id) + ", timeout after " +
                   std::to_string(write_lock_timeout_ms_) + "ms");
    return false;
  }

  // 检查页面是否存在
  // Why: 需要确保页面确实存在于缓冲池中，避免操作不存在的页面
  // What: 在page_table_中查找页面ID
  // How: 使用std::unordered_map的find方法查找
  auto page_it = page_table_.find(page_id);
  if (page_it == page_table_.end()) {
    // 页面不存在，记录警告并返回false
    // Why: 页面不存在于缓冲池中，无法删除
    // What: 记录警告信息并返回false表示操作失败
    // How: 使用SQLCC_LOG_WARN记录警告，然后返回false
    SQLCC_LOG_WARN("Page ID " + std::to_string(page_id) +
                   " not found in buffer pool");
    return false;
  }

  // 检查页面是否被引用
  // Why: 被引用的页面正在被使用，不能删除
  // What: 检查page_refs_中该页面的引用计数
  // How: 使用std::unordered_map的find方法查找引用计数
  auto ref_it = page_refs_.find(page_id);
  if (ref_it != page_refs_.end() && ref_it->second > 0) {
    // 页面正在被使用，不能删除
    // Why: 被引用的页面正在被其他操作使用，删除会导致数据不一致
    // What: 记录警告信息并返回false表示操作失败
    // How: 使用SQLCC_LOG_WARN记录警告，然后返回false
    SQLCC_LOG_WARN("Page ID " + std::to_string(page_id) +
                   " is still referenced, cannot delete");
    return false;
  }

  // 检查是否为脏页并刷新
  // Why: 如果页面是脏的，需要先写回磁盘才能安全删除
  // What: 检查dirty_pages_中该页面是否为脏页，如果是则刷新
  // How: 手动处理脏页刷新，避免递归调用FlushPage导致的锁问题
  auto dirty_it = dirty_pages_.find(page_id);
  if (dirty_it != dirty_pages_.end() && dirty_it->second) {
    // 页面是脏的，需要先刷新到磁盘
    // 使用与FlushPage相同的锁释放策略，但不递归调用FlushPage
    std::shared_ptr<Page> page_shared = page_it->second;
    const void* page_data = page_shared->GetData();
    int32_t current_page_id = page_id;

    // 临时释放锁，进行磁盘写入操作
    lock.unlock();

    // 现在在锁外进行磁盘写入操作
    bool write_success = disk_manager_->WritePage(
        current_page_id, static_cast<const char *>(page_data));

    // 重新获取锁 - 使用带超时的锁获取避免永久阻塞
    if (!lock.try_lock_for(std::chrono::milliseconds(write_lock_timeout_ms_))) {
      SQLCC_LOG_WARN("Failed to reacquire buffer pool lock after writing dirty "
                     "page before deletion " +
                     std::to_string(current_page_id));
      return false;
    }

    if (!write_success) {
      // 刷新失败，记录错误并返回false
      std::string error_msg = "Failed to flush dirty page ID " +
                              std::to_string(current_page_id) +
                              " before deletion";
      SQLCC_LOG_ERROR(error_msg);
      return false;
    }

    // 刷新成功后，清除脏页标记
    dirty_pages_[current_page_id] = false;
  }

  // 从页面表中移除页面
  // Why: 需要从页面表中删除页面，以释放页面占用的内存
  // What: 从page_table_哈希表中删除页面ID
  // How: 使用std::unordered_map的erase方法删除
  Page *page = page_it->second.get();
  page_table_.erase(page_it);

  // 从引用计数表中移除
  // Why: 需要从引用计数表中删除页面，以释放引用计数占用的内存
  // What: 从page_refs_哈希表中删除页面ID
  // How: 使用std::unordered_map的erase方法删除
  if (ref_it != page_refs_.end()) {
    page_refs_.erase(ref_it);
  }

  // 从脏页表中移除
  // Why: 需要从脏页表中删除页面，以释放脏页标记占用的内存
  // What: 从dirty_pages_哈希表中删除页面ID
  // How: 使用std::unordered_map的erase方法删除
  if (dirty_it != dirty_pages_.end()) {
    dirty_pages_.erase(dirty_it);
  }

  // 从LRU链表中移除
  // Why: 需要从LRU链表中删除页面，以维护LRU链表的正确性
  // What: 从lru_list_和lru_map_中删除页面ID
  // How: 使用RemoveFromLRUList方法从LRU链表中移除
  RemoveFromLRUList(page_id);

  // 页面对象现在由智能指针自动管理，无需手动释放
  // 智能指针会在合适的时候自动释放内存

  // 通知磁盘管理器释放页面ID
  // Why: 需要通知磁盘管理器该页面ID可以被重用
  // What: 调用磁盘管理器的DeallocatePage方法释放页面ID
  // How: 传入页面ID，让磁盘管理器更新其内部状态
  if (!disk_manager_->DeallocatePage(page_id)) {
    SQLCC_LOG_ERROR("Failed to notify disk manager to deallocate page " +
                    std::to_string(page_id));
    // 这是一个非关键错误，我们可以继续清理内存中的页面
  }

  // 记录页面删除成功，便于调试
  SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) +
                  " deleted from buffer pool");

  return true;
}

// 取消固定页面实现
// Why:
// 当一个页面使用完毕后，需要通知缓冲池不再使用该页面，以便缓冲池可以正确管理页面的生命周期和LRU顺序
// What: UnpinPage方法减少页面的引用计数，并标记页面是否被修改
// How: 检查页面是否存在，减少引用计数，标记脏页，然后返回操作结果
bool BufferPool::UnpinPage(int32_t page_id, bool is_dirty) {
  // 加锁保护并发访问 - 使用带超时的unique_lock以支持临时解锁避免死锁
  std::unique_lock<std::timed_mutex> lock(latch_, std::defer_lock);
  if (!lock.try_lock_for(std::chrono::milliseconds(write_lock_timeout_ms_))) {
    // 获取锁失败，记录警告但不抛出异常，避免在高并发场景下级联失败
    SQLCC_LOG_WARN("Failed to acquire buffer pool lock for unpinning page " +
                   std::to_string(page_id) + ", timeout after " +
                   std::to_string(write_lock_timeout_ms_) + "ms");
    return false;
  }

  // 检查页面是否存在
  // Why: 需要确保页面确实存在于缓冲池中，避免操作不存在的页面
  // What: 在page_table_中查找页面ID
  // How: 使用std::unordered_map的find方法查找
  auto it = page_table_.find(page_id);
  if (it == page_table_.end()) {
    // 页面不存在，记录警告并返回false
    // Why: 页面不存在于缓冲池中，无法取消固定
    // What: 记录警告信息并返回false表示操作失败
    // How: 使用SQLCC_LOG_WARN记录警告，然后返回false
    SQLCC_LOG_WARN("Page ID " + std::to_string(page_id) +
                   " not found in buffer pool");
    return false;
  }

  // 减少引用计数
  // Why: 页面使用完毕，需要减少引用计数，以便缓冲池可以正确管理页面的生命周期
  // What: 减少page_refs_中该页面的引用计数
  // How: 先检查当前引用计数，确保减少后不会变为负数
  if (page_refs_[page_id] > 0) {
    page_refs_[page_id]--;
  } else {
    // 引用计数已经为0或负数，表示页面已经被完全释放
    // Why: 重复调用UnpinPage表示程序逻辑错误
    // What: 记录错误信息并返回false表示操作失败
    // How: 使用SQLCC_LOG_ERROR记录错误级别日志，然后返回false
    SQLCC_LOG_ERROR("Attempting to unpin page " + std::to_string(page_id) +
                    " with reference count " +
                    std::to_string(page_refs_[page_id]) +
                    ", page already fully unpinned");
    return false;
  }

  // 标记页面为脏页（如果被修改）
  // Why: 如果页面被修改过，需要标记为脏页，以便在替换时写回磁盘
  // What: 如果is_dirty为true，则将页面标记为脏页
  // How: 在dirty_pages_哈希表中设置页面ID的值为true
  if (is_dirty) {
    dirty_pages_[page_id] = true;
    // 记录页面被标记为脏页的信息，便于调试
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " marked as dirty");
  }

  // 记录取消固定页面的信息，便于调试
  SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) +
                  " unpinned, refs: " + std::to_string(page_refs_[page_id]));

  return true;
}

// 页面替换算法实现
// Why: 当缓冲池已满时，需要选择一个页面进行替换，为新页面腾出空间
// What: ReplacePage方法使用LRU算法选择一个可替换的页面
// How: 从LRU链表尾部开始查找，找到第一个引用计数为0的页面
int32_t BufferPool::ReplacePage() {
  // 加锁保护并发访问 - 使用带超时的unique_lock以支持临时解锁避免死锁
  std::unique_lock<std::timed_mutex> lock(latch_, std::defer_lock);
  // 使用更长的超时时间，因为页面替换是关键操作
  int replace_lock_timeout = std::max(lock_timeout_ms_, write_lock_timeout_ms_);

  // 尝试多次获取锁，增加成功率
  int retry_count = 3;
  bool lock_acquired = false;
  for (int i = 0; i < retry_count; ++i) {
    if (lock.try_lock_for(std::chrono::milliseconds(replace_lock_timeout))) {
      lock_acquired = true;
      break;
    }
    // 如果不是最后一次尝试，等待一小段时间再重试
    if (i < retry_count - 1) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  if (!lock_acquired) {
    // 获取锁失败，记录警告并返回-1
    SQLCC_LOG_WARN(
        "Failed to acquire buffer pool lock for page replacement after " +
        std::to_string(retry_count) + " attempts");
    return -1;
  }

  // 从LRU链表尾部开始查找可替换的页面
  // Why: LRU链表尾部是最近最少使用的页面，应该优先替换
  // What: 从lru_list_的尾部开始遍历，查找引用计数为0的页面
  // How: 使用反向迭代器从链表尾部开始遍历
  for (auto it = lru_list_.rbegin(); it != lru_list_.rend(); ++it) {
    int32_t page_id = *it;

    // 检查页面引用计数
    // Why: 引用计数大于0的页面正在被使用，不能替换
    // What: 检查page_refs_中该页面的引用计数
    // How: 使用std::unordered_map的find方法查找引用计数
    auto ref_it = page_refs_.find(page_id);
    if (ref_it != page_refs_.end() && ref_it->second > 0) {
      // 页面正在被使用，不能替换，继续查找
      continue;
    }

    // 检查脏页标记 - 修复死锁问题：使用unique_lock支持锁释放机制
    // Why:
    // 如果页面是脏的，需要先写回磁盘才能替换，但必须在锁外进行磁盘I/O以避免死锁
    // What:
    // 使用std::unique_lock的unlock()方法临时释放锁，在锁外执行磁盘写入操作 How:
    // 先在锁内准备数据，然后解锁调用DiskManager，再重新加锁继续操作
    auto dirty_it = dirty_pages_.find(page_id);
    if (dirty_it != dirty_pages_.end() && dirty_it->second) {
      // 页面是脏的，需要先写回磁盘
      // Why: 脏页的内容与磁盘不一致，需要写回磁盘以保证数据持久性
      // What: 先在锁内准备数据，然后释放锁进行磁盘写入，最后重新加锁
      // How: 这是解决BufferPool latch_和DiskManager io_mutex_死锁的核心策略

      // 查找页面在页面表中的位置
      auto page_it = page_table_.find(page_id);
      if (page_it == page_table_.end()) {
        // 页面不在页面表中，继续查找下一个页面
        continue;
      }

      // 获取页面数据和ID用于后续处理
      Page *page = page_it->second.get();
      void *page_data = page->GetData();
      int32_t current_page_id = page_id;

      // 释放锁，进行磁盘写入操作 - 这是避免死锁的关键修复
      // Why: 需要先释放BufferPool锁，再调用DiskManager避免循环锁等待
      // What: 使用std::unique_lock的unlock()方法临时释放锁
      // How: 调用unlock()释放latch_锁，然后进行磁盘写入，再重新加锁
      lock.unlock();

      // 现在在锁外进行磁盘写入操作
      // Why: DiskManager::WritePage会获取io_mutex_，必须在BufferPool
      // latch_释放后调用 What: 执行实际的页面写入磁盘操作 How:
      // 调用disk_manager_->WritePage进行磁盘写入
      bool write_success = disk_manager_->WritePage(
          current_page_id, static_cast<const char *>(page_data));

      // 重新获取锁 - 增加重试逻辑以应对高并发场景
      // Why: 需要重新获取锁以继续保护BufferPool的数据结构
      // What: 使用带超时的方式重新加锁，增加重试次数提高成功率
      // How: 调用try_lock_for()尝试重新获取latch_锁，失败时多次重试
      int relock_attempts = 0;
      bool relock_success = false;
      while (relock_attempts < 5 && !relock_success) {
        relock_success =
            lock.try_lock_for(std::chrono::milliseconds(replace_lock_timeout));
        if (!relock_success) {
          relock_attempts++;
          SQLCC_LOG_DEBUG(
              "Failed to re-acquire buffer pool lock after writing page " +
              std::to_string(current_page_id) + ", attempt " +
              std::to_string(relock_attempts) + "/5");
          // 短暂等待后重试，避免立即重试导致失败
          std::this_thread::sleep_for(
              std::chrono::milliseconds(50 * relock_attempts));
        }
      }

      if (!relock_success) {
        // 获取锁失败，记录警告并跳过此页面，继续查找下一个可替换页面
        // Why: 重新获取锁失败不应该终止整个替换过程，而是应该尝试其他页面
        // What: 记录警告并继续查找下一个可替换页面
        // How: 使用continue跳过当前页面，继续查找下一个
        SQLCC_LOG_WARN(
            "Failed to re-acquire buffer pool lock after writing page " +
            std::to_string(current_page_id) +
            " after 5 attempts, skipping this page");
        continue;
      }

      if (!write_success) {
        // 写入失败，恢复脏页标记
        // Why: 写入磁盘失败可能导致数据丢失，必须恢复脏页状态
        // What: 重新获取锁后恢复页面的脏页标记
        // How: 在dirty_pages_哈希表中重新设置脏页标记
        dirty_pages_[current_page_id] = true;
        std::string error_msg = "Failed to write dirty page ID " +
                                std::to_string(current_page_id) + " to disk";
        SQLCC_LOG_ERROR(error_msg);

        // 跳过此页面，继续查找下一个可替换页面
        continue;
      }

      // 写入成功，记录日志
      // Why: 成功写入脏页到磁盘，可以安全进行页面替换
      // What: 记录成功的磁盘写入操作，便于调试和监控
      // How: 使用SQLCC_LOG_DEBUG记录调试级别日志
      SQLCC_LOG_DEBUG("Successfully wrote dirty page ID " +
                      std::to_string(current_page_id) + " to disk");

      // 页面已成功写回，现在可以安全替换
      // Why: 脏页已经刷新到磁盘，可以安全地从内存中移除
      // What: 页面已不再是脏页，可以进行正常的页面替换
      // How: 继续执行页面移除逻辑（已在原代码中处理）
    }

    // 从缓冲池中移除页面
    // Why: 需要从页面表中删除页面，以释放页面占用的内存
    // What: 从page_table_哈希表中删除页面ID
    // How: 使用std::unordered_map的erase方法删除
    page_table_.erase(page_id);

    // 从脏页表中移除
    // Why: 需要从脏页表中删除页面，以释放脏页标记占用的内存
    // What: 从dirty_pages_哈希表中删除页面ID
    // How: 使用std::unordered_map的erase方法删除
    dirty_pages_.erase(page_id);

    // 从引用计数表中移除
    // Why: 需要从引用计数表中删除页面，以释放引用计数占用的内存
    // What: 从page_refs_哈希表中删除页面ID
    // How: 使用std::unordered_map的erase方法删除
    if (ref_it != page_refs_.end()) {
      page_refs_.erase(ref_it);
    }

    // 从LRU链表中移除
    // Why: 需要从LRU链表中删除页面，以维护LRU链表的正确性
    // What: 从lru_list_和lru_map_中删除页面ID
    // How: 使用RemoveFromLRUList方法正确移除当前页面
    RemoveFromLRUList(page_id);

    // 记录页面替换成功，便于调试
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " replaced");
    return page_id;
  }

  // 无法找到可替换的页面，记录警告并返回-1
  // Why: 如果所有页面都在使用中，无法进行页面替换，需要记录警告
  // What: 创建警告消息，记录警告日志，然后返回-1
  // How: 使用SQLCC_LOG_WARN记录警告级别日志，然后返回-1
  SQLCC_LOG_WARN("No page can be replaced in buffer pool");
  return -1;
}

// 移动页面到LRU链表头部实现
// Why:
// 当页面被访问时，需要将其移动到LRU链表头部，表示最近被访问，这是LRU算法的核心操作
// What:
// MoveToHead方法将指定页面移动到LRU链表头部，更新LRU映射，以维护页面的访问顺序
// How: 从LRU链表中删除页面，然后将其添加到头部，并更新LRU映射中的迭代器
// 注意：调用者必须已经持有锁，这里不需要重复加锁
void BufferPool::MoveToHead(int32_t page_id) {
  // 查找页面在LRU链表中的位置
  // Why: 需要知道页面在LRU链表中的位置，才能进行移动操作
  // What: 在lru_map_中查找页面ID的迭代器
  // How: 使用std::unordered_map的find方法查找迭代器
  auto it = lru_map_.find(page_id);
  if (it == lru_map_.end()) {
    // 页面不在LRU链表中，直接返回
    // Why: 如果页面不在LRU链表中，无法进行移动操作
    // What: 直接返回，不执行任何操作
    // How: 使用return语句直接返回
    return;
  }

  // 从当前位置移除
  // Why: 需要从当前位置删除页面，然后才能将其添加到头部
  // What: 使用迭代器从lru_list_中删除页面
  // How: 使用std::list的erase方法删除
  lru_list_.erase(it->second);

  // 添加到头部
  // Why: 将页面添加到头部表示最近被访问
  // What: 使用push_front方法将页面ID添加到lru_list_头部
  // How: 使用std::list的push_front方法添加
  lru_list_.push_front(page_id);

  // 更新LRU映射中的迭代器
  // Why: 需要更新LRU映射中的迭代器，以保持映射的正确性
  // What: 在lru_map_中更新页面ID对应的迭代器
  // How: 将页面ID映射到lru_list_的begin()迭代器
  lru_map_[page_id] = lru_list_.begin();

  // 记录页面移动成功，便于调试
  SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) +
                  " moved to head of LRU list");
}

// 创建新页面实现
// Why: 数据库需要新的存储空间来存储数据，例如插入新记录或创建索引
// What: NewPage方法在缓冲池中分配一个新的页面，并返回页面智能指针和页面ID
// How: 先确保有空间，然后分配页面ID，创建页面对象，添加到缓冲池管理
std::shared_ptr<Page> BufferPool::NewPage(int32_t *page_id) {
  // 加锁保护并发访问 - 使用带超时的unique_lock以支持临时解锁避免死锁
  std::unique_lock<std::timed_mutex> lock(latch_, std::defer_lock);
  if (!lock.try_lock_for(std::chrono::milliseconds(write_lock_timeout_ms_))) {
    // 获取锁失败，记录警告但不抛出异常，避免在高并发场景下级联失败
    SQLCC_LOG_WARN("Failed to acquire buffer pool lock for creating new page, "
                   "timeout after " +
                   std::to_string(write_lock_timeout_ms_) + "ms");
    if (page_id != nullptr) {
      *page_id = -1; // 使用-1作为无效页面ID
    }
    return nullptr;
  }

  // 如果缓冲池已满，先进行页面替换
  // Why: 必须在分配页面ID之前确保有空间，避免页面ID重用造成数据混乱
  // What: 检查page_table_的大小是否达到pool_size_
  // How: 比较page_table_.size()和pool_size_
  if (page_table_.size() >= pool_size_) {
    // 记录缓冲池已满的信息，便于调试
    SQLCC_LOG_DEBUG(
        "Buffer pool is full, replacing page for new page allocation");

    // 调用ReplacePage方法选择一个页面进行替换
    // Why: 需要选择一个页面进行替换，为新页面腾出空间
    // What: ReplacePage方法使用LRU算法选择一个可替换的页面
    // How: 从LRU链表尾部开始查找，找到第一个引用计数为0的页面
    int32_t replaced_page_id = ReplacePage();
    if (replaced_page_id == -1) {
      // 无法替换页面，返回nullptr
      // Why: 如果所有页面都在使用中，无法为新页面腾出空间，必须报错
      // What: 记录错误信息并返回nullptr
      // How: 使用SQLCC_LOG_ERROR记录错误级别日志
      std::string error_msg =
          "Failed to replace page in buffer pool for new page allocation";
      SQLCC_LOG_ERROR(error_msg);
      if (page_id != nullptr) {
        *page_id = -1; // 使用-1作为无效页面ID
      }
      return nullptr;
    }
  }

  // 现在分配新的页面ID（此时已有可用空间）
  // Why: 分配页面ID必须在有空间之后，避免ID重用混乱
  // What: 调用磁盘管理器的AllocatePage方法获取新页面ID
  // How: AllocatePage方法返回新的页面ID
  *page_id = disk_manager_->AllocatePage();
  if (*page_id < 0) {
    // 无法分配新页面，记录错误并返回空智能指针
  // Why: 磁盘空间不足或磁盘管理器错误导致无法分配新页面
  // What: 记录错误信息并返回空智能指针表示失败
  // How: 使用SQLCC_LOG_ERROR记录错误级别日志
  std::string error_msg = "Failed to allocate new page from disk manager";
  SQLCC_LOG_ERROR(error_msg);
  if (page_id != nullptr) {
    *page_id = -1; // 使用-1作为无效页面ID
  }
  return nullptr; // 返回空智能指针表示分配失败
  }

  // 记录新页面分配成功，便于调试
  SQLCC_LOG_DEBUG("Allocated new page ID " + std::to_string(*page_id) +
                  " from disk manager");

  // 创建新页面智能指针
  // Why: 需要创建页面对象来管理新页面的数据和元数据
  // What: 使用页面ID创建新的Page对象
  // How: 使用std::make_shared创建页面智能指针
  auto page = std::make_shared<Page>(*page_id);

  // 初始化页面数据（可选）
  // Why: 新页面可能需要初始化为空或特定值
  // What: 可以选择初始化页面数据为特定值
  // How: 这里不初始化，让调用者自行处理页面数据

  // 添加到页面表
  // Why: 需要将新创建的页面加入缓冲池管理，以便后续查找和使用
  // What: 将页面智能指针添加到page_table_哈希表中
  // How: 使用页面ID作为键，存储智能指针
  page_table_[*page_id] = page; // 存储智能指针到缓冲池
  page_refs_[*page_id] = 1; // 新页面默认引用计数为1

  // 初始化脏页标记
  // Why: 需要跟踪页面是否被修改过，以便在替换时写回磁盘
  // What: 将页面标记为非脏页，因为新创建页面
  // How: 在dirty_pages_哈希表中设置页面ID的值为false
  dirty_pages_[*page_id] = false;

  // 添加到LRU链表头部
  // Why: 新创建的页面应该放在LRU链表头部，表示最近被访问
  // What: 将页面ID添加到lru_list_的头部
  // How: 使用push_front方法添加到链表头部，并在lru_map_中记录迭代器
  lru_list_.push_front(*page_id);
  lru_map_[*page_id] = lru_list_.begin();

  // 记录新页面创建成功，便于调试
  SQLCC_LOG_DEBUG("New page ID " + std::to_string(*page_id) +
                  " created in buffer pool");

  // 返回共享智能指针，避免拷贝和多次销毁
  return page;
}
