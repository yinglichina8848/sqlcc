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
BufferPool::BufferPool(std::shared_ptr<DiskManager> disk_manager,
                       ConfigManager &config_manager, size_t pool_size)
    : config_manager_(config_manager), disk_manager_(std::move(disk_manager)),
      pool_size_(pool_size), page_table_(), page_table_mutex_(),
      replace_strategy_(), lock_manager_(), prefetcher_(),
      stats_mutex_(), stats_(), latch_(), page_refs_(), dirty_pages_(),
      access_stats_(), lru_list_(), lru_map_(), batch_buffer_(),
      simulate_flush_failure_(false), read_lock_timeout_ms_(0),
      write_lock_timeout_ms_(0), lock_timeout_ms_(0), next_page_id_(0),
      shutdown_(false) {  // 从配置管理器获取不同操作的锁超时时间
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

// 检查页面是否在缓冲池中
bool BufferPool::IsPageInBuffer(int32_t page_id) const {
  // 加锁保护并发访问
  std::shared_lock<std::shared_mutex> lock(page_table_mutex_);
  return page_table_.find(page_id) != page_table_.end();
}

// 获取已使用的页面数
size_t BufferPool::GetUsedPages() const {
  // 加锁保护并发访问
  std::shared_lock<std::shared_mutex> lock(page_table_mutex_);
  return page_table_.size();
}

// 获取页面实现
// Why: 数据库操作需要访问特定页面ID的数据，这是缓冲池最核心的功能
// What: FetchPage方法根据页面ID获取对应的页面智能指针
// How: 检查页面是否在缓冲池中，如果不在则从磁盘加载，如果缓冲池已满则替换页面
std::shared_ptr<BufferPage> BufferPool::FetchPage(int32_t page_id, int32_t transaction_id) {
  (void)transaction_id; // 避免未使用参数警告
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
  auto page = std::make_shared<BufferPage>(page_id);
  char* page_data = page->data.GetDataSpan().data();
  int32_t current_page_id = page_id;  // 释放锁，进行磁盘读取操作 - 修复死锁问题
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

// 获取页面的共享智能指针实现
std::shared_ptr<BufferPage> BufferPool::FetchPageShared(int32_t page_id) {
  // 直接调用FetchPage方法，因为它们的功能基本相同
  return FetchPage(page_id, -1);
}

bool BufferPool::UnpinPage(int32_t page_id, int32_t transaction_id) {
  (void)transaction_id; // 避免未使用参数警告
  // 加锁保护并发访问
  std::unique_lock<std::timed_mutex> lock(latch_, std::defer_lock);
  if (!lock.try_lock_for(std::chrono::milliseconds(read_lock_timeout_ms_))) {
    SQLCC_LOG_WARN("Failed to acquire buffer pool lock for unpinning page " +
                   std::to_string(page_id) + ", timeout after " +
                   std::to_string(read_lock_timeout_ms_) + "ms");
    return false;
  }  // 查找页面
  auto it = page_table_.find(page_id);
  if (it == page_table_.end()) {
    // 页面不在缓冲池中
    SQLCC_LOG_WARN("Page ID " + std::to_string(page_id) +
                   " not found in buffer pool when trying to unpin");
    return false;
  }

  // 减少引用计数
  if (page_refs_[page_id] > 0) {
    page_refs_[page_id]--;
  }

  // 如果引用计数为0，可以考虑替换页面
  if (page_refs_[page_id] == 0) {
    // 可以在这里添加页面替换逻辑，但现在我们只是减少引用计数
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) +
                    " unpinned, reference count is now 0");
  } else {
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) +
                    " unpinned, reference count is now " +
                    std::to_string(page_refs_[page_id]));
  }

  return true;
}

// 创建新页面实现
int32_t BufferPool::NewPage(int32_t transaction_id) {
  (void)transaction_id; // 避免未使用参数警告
  // 加锁保护并发访问
  std::unique_lock<std::timed_mutex> lock(latch_, std::defer_lock);
  if (!lock.try_lock_for(std::chrono::milliseconds(write_lock_timeout_ms_))) {
    SQLCC_LOG_WARN("Failed to acquire buffer pool lock for creating new page, timeout after " +
                   std::to_string(write_lock_timeout_ms_) + "ms");
    return -1;
  }

  // 检查缓冲池是否已满
  if (page_table_.size() >= pool_size_) {
    // 尝试替换一个页面
    int32_t replaced_page_id = ReplacePage();
    if (replaced_page_id == -1) {
      SQLCC_LOG_ERROR("Failed to replace page when creating new page, buffer pool is full");
      return -1;
    }
  }

  // 从磁盘管理器分配新页面ID
  int32_t new_page_id = disk_manager_->AllocatePage();
  if (new_page_id < 0) {
    SQLCC_LOG_ERROR("Failed to allocate new page ID from disk manager");
    return -1;
  }

  // 创建新页面
  auto page = std::make_shared<BufferPage>(new_page_id);

  // 初始化页面数据
  memset(page->data.GetDataSpan().data(), 0, PAGE_SIZE);

  // 添加到页面表
  page_table_[new_page_id] = page;
  page_refs_[new_page_id] = 1;  // 新创建的页面引用计数为1
  dirty_pages_[new_page_id] = false;

  // 添加到LRU链表头部
  lru_list_.push_front(new_page_id);
  lru_map_[new_page_id] = lru_list_.begin();

  SQLCC_LOG_DEBUG("New page created with ID: " + std::to_string(new_page_id));

  // 返回页面ID
  return new_page_id;
}

bool BufferPool::FlushPage(int32_t page_id) {
  // 加锁保护并发访问
  std::unique_lock<std::timed_mutex> lock(latch_, std::defer_lock);
  if (!lock.try_lock_for(std::chrono::milliseconds(write_lock_timeout_ms_))) {
    SQLCC_LOG_WARN("Failed to acquire buffer pool lock for flushing page " +
                   std::to_string(page_id) + ", timeout after " +
                   std::to_string(write_lock_timeout_ms_) + "ms");
    return false;
  }

  // 查找页面
  auto it = page_table_.find(page_id);
  if (it == page_table_.end()) {
    // 页面不在缓冲池中
    SQLCC_LOG_WARN("Page ID " + std::to_string(page_id) +
                   " not found in buffer pool when trying to flush");
    return false;
  }

  // 检查是否为脏页
  if (!dirty_pages_[page_id]) {
    // 不是脏页，无需写回
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) +
                    " is not dirty, skipping flush");
    return true;
  }

  // 写回页面到磁盘
  bool write_success =
      disk_manager_->WritePage(page_id, it->second->data.GetDataSpan().data());
  if (write_success) {
    // 清除脏页标记
    dirty_pages_[page_id] = false;
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) +
                    " flushed to disk successfully");
  } else {
    SQLCC_LOG_ERROR("Failed to flush page ID " + std::to_string(page_id) +
                    " to disk");
  }

  return write_success;
}

bool BufferPool::FlushAllPages() {
  // 加锁保护并发访问
  std::unique_lock<std::timed_mutex> lock(latch_, std::defer_lock);
  if (!lock.try_lock_for(std::chrono::milliseconds(write_lock_timeout_ms_))) {
    SQLCC_LOG_WARN("Failed to acquire buffer pool lock for flushing all pages, timeout after " +
                   std::to_string(write_lock_timeout_ms_) + "ms");
    return false;
  }

  bool all_success = true;

  // 遍历所有页面
  for (const auto& pair : page_table_) {
    int32_t page_id = pair.first;
    const auto& buffer_page = pair.second;

    // 检查是否为脏页
    if (dirty_pages_[page_id]) {
      // 写回页面到磁盘
      bool write_success =
          disk_manager_->WritePage(page_id, buffer_page->data.GetDataSpan().data());
      if (write_success) {
        // 清除脏页标记
        dirty_pages_[page_id] = false;
        SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) +
                        " flushed to disk successfully");
      } else {
        SQLCC_LOG_ERROR("Failed to flush page ID " + std::to_string(page_id) +
                        " to disk");
        all_success = false;
      }
    }
  }

  return all_success;
}

void BufferPool::MoveToHead(int32_t page_id) {
  // 查找页面在LRU链表中的位置
  auto it = lru_map_.find(page_id);
  if (it != lru_map_.end()) {
    // 从当前位置移除
    lru_list_.erase(it->second);
    // 添加到链表头部
    lru_list_.push_front(page_id);
    // 更新映射
    lru_map_[page_id] = lru_list_.begin();
  }
}

int32_t BufferPool::ReplacePage() {
  // 从LRU链表尾部开始查找，找到第一个引用计数为0的页面
  for (auto it = lru_list_.rbegin(); it != lru_list_.rend(); ++it) {
    int32_t page_id = *it;
    if (page_refs_[page_id] == 0) {
      // 找到可替换的页面
      auto buffer_page_it = page_table_.find(page_id);
      if (buffer_page_it != page_table_.end()) {
        // 如果是脏页，先写回磁盘
        if (dirty_pages_[page_id]) {
          bool write_success =
              disk_manager_->WritePage(page_id, buffer_page_it->second->data.GetDataSpan().data());
          if (!write_success) {
            SQLCC_LOG_ERROR("Failed to write dirty page " + std::to_string(page_id) +
                            " to disk during replacement");
            // 即使写回失败，我们仍然需要替换页面以避免缓冲池满
          }
        }

        // 从各种数据结构中移除页面
        page_table_.erase(buffer_page_it);
        page_refs_.erase(page_id);
        dirty_pages_.erase(page_id);
        access_stats_.erase(page_id);
        lru_map_.erase(page_id);
        lru_list_.erase(--it.base()); // 转换reverse_iterator到iterator

        SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) +
                        " replaced from buffer pool");
        return page_id;
      }
    }
  }

  // 没有找到可替换的页面
  SQLCC_LOG_WARN("No evictable page found in buffer pool");
  return -1;
}

} // namespace sqlcc
