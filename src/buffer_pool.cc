#include "buffer_pool.h"
#include "exception.h"
#include "logger.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include <future>

namespace sqlcc {

// 构造函数实现
// Why: 需要初始化缓冲池的基本状态，包括设置磁盘管理器和缓冲池大小
// What: 构造函数初始化成员变量，并记录初始化日志
// How: 使用成员初始化列表初始化disk_manager_、pool_size_和config_manager_，并记录日志
BufferPool::BufferPool(DiskManager* disk_manager, size_t pool_size, ConfigManager& config_manager)
    : disk_manager_(disk_manager), config_manager_(config_manager), pool_size_(pool_size), 
        simulate_flush_failure_(false), read_lock_timeout_ms_(0), write_lock_timeout_ms_(0), lock_timeout_ms_(0) {
    // 从配置管理器获取不同操作的锁超时时间
    // 读取操作使用较短的超时时间
    read_lock_timeout_ms_ = config_manager_.GetInt("buffer_pool.read_lock_timeout_ms", 2000);
    // 写入和修改操作使用较长的超时时间
    write_lock_timeout_ms_ = config_manager_.GetInt("buffer_pool.write_lock_timeout_ms", 5000);
    // 默认超时时间（用于其他操作）
    lock_timeout_ms_ = config_manager_.GetInt("buffer_pool.default_lock_timeout_ms", 3000);
    
    // 记录配置的锁超时时间
    SQLCC_LOG_INFO("BufferPool lock timeout settings - Read: " + std::to_string(read_lock_timeout_ms_) + 
                  "ms, Write: " + std::to_string(write_lock_timeout_ms_) + 
                  "ms, Default: " + std::to_string(lock_timeout_ms_) + "ms");
    // 预分配批量操作缓冲区空间
    batch_buffer_.reserve(std::min(pool_size_, static_cast<size_t>(64)));
    
    // 记录缓冲池初始化信息，便于调试和监控
    SQLCC_LOG_INFO("Initializing BufferPool with pool size: " + std::to_string(pool_size_));
    
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
// What: FetchPage方法根据页面ID获取对应的页面指针
// How: 检查页面是否在缓冲池中，如果不在则从磁盘加载，如果缓冲池已满则替换页面
Page* BufferPool::FetchPage(int32_t page_id) {
    // 加锁保护并发访问 - 使用带超时的unique_lock以支持临时解锁避免死锁
    std::unique_lock<std::timed_mutex> lock(latch_, std::defer_lock);
    if (!lock.try_lock_for(std::chrono::milliseconds(read_lock_timeout_ms_))) {
        // 获取锁失败，记录警告但不抛出异常，避免在高并发场景下级联失败
        SQLCC_LOG_WARN("Failed to acquire buffer pool lock for fetching page " + std::to_string(page_id) + 
                      ", timeout after " + std::to_string(read_lock_timeout_ms_) + "ms");
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
        // SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " found in buffer pool");
        return it->second;
    }

    // 页面不在缓冲池中，需要从磁盘读取
    // Why: 页面不在内存中，必须从磁盘加载，这是数据库I/O操作的主要来源
    // What: 记录页面不在缓冲池中的信息
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " not found in buffer pool, reading from disk");
    
    // 创建新页面
    auto page = std::make_unique<Page>(page_id);
    void* page_data = page->GetData();
    int32_t current_page_id = page_id;
    
    // 释放锁，进行磁盘读取操作 - 修复死锁问题
    lock.unlock();
    
    // 现在在锁外进行磁盘读取操作
    bool read_success = disk_manager_->ReadPage(current_page_id, static_cast<char*>(page_data));
    
    // 重新获取锁 - 使用带超时的锁获取避免永久阻塞
    if (!lock.try_lock_for(std::chrono::milliseconds(read_lock_timeout_ms_))) {
        SQLCC_LOG_WARN("Failed to reacquire buffer pool lock after reading page " + std::to_string(current_page_id) + 
                      ", timeout after " + std::to_string(read_lock_timeout_ms_) + "ms");
        return nullptr;
    }
    
    if (!read_success) {
        // 如果读取失败，说明页面不存在，返回nullptr
        SQLCC_LOG_DEBUG("Failed to read page ID " + std::to_string(current_page_id) + " from disk, page does not exist");
        return nullptr;
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
            // 无法替换页面，抛出异常
            // Why: 如果所有页面都在使用中，无法为新页面腾出空间，必须报错
            // What: 创建错误消息并抛出BufferPoolException异常
            // How: 使用throw关键字抛出异常
            std::string error_msg = "Failed to replace page in buffer pool";
            SQLCC_LOG_ERROR(error_msg);
            throw BufferPoolException(error_msg);
        }
    }

    // 添加到页面表
    // Why: 需要将新加载的页面加入缓冲池管理，以便后续查找和使用
    // What: 将页面对象添加到page_table_哈希表中
    // How: 使用页面ID作为键，使用std::move转移页面对象的所有权
    // 将页面添加到缓冲池
    Page* page_ptr = page.release(); // 释放所有权，获取原始指针
    page_table_[page_id] = page_ptr;
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
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " loaded into buffer pool");

    return page_ptr;
}

// 刷新页面到磁盘实现
// Why: 需要将修改后的页面数据持久化到磁盘，保证数据的持久性和一致性
// What: FlushPage方法将指定页面的数据写入磁盘文件
// How: 检查页面是否存在，如果是脏页则调用磁盘管理器写入磁盘
bool BufferPool::FlushPage(int32_t page_id) {
    // 加锁保护并发访问 - 使用带超时的unique_lock以支持临时解锁避免死锁
    std::unique_lock<std::timed_mutex> lock(latch_, std::defer_lock);
    if (!lock.try_lock_for(std::chrono::milliseconds(lock_timeout_ms_))) {
        // 获取锁失败，记录警告但不抛出异常，避免在高并发场景下级联失败
        SQLCC_LOG_WARN("Failed to acquire buffer pool lock for flushing page " + std::to_string(page_id));
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
        SQLCC_LOG_WARN("Page ID " + std::to_string(page_id) + " not found in buffer pool");
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
        SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " is not dirty, no flush needed");
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
        std::string error_msg = "Simulated flush failure for page ID " + std::to_string(page_id);
        SQLCC_LOG_ERROR(error_msg);
        return false;
    }
    
    // 将页面数据写入磁盘 - 修复死锁问题：使用unique_lock支持锁释放机制
    // Why: 脏页的内容与磁盘不一致，需要写回磁盘以保证数据持久性，但必须在锁外进行磁盘I/O以避免死锁
    // What: 先在锁内准备数据，然后解锁调用DiskManager，再重新加锁继续操作
    // How: 与ReplacePage方法采用相同的锁释放策略，避免BufferPool latch_和DiskManager io_mutex_之间的循环等待
    Page* page = page_it->second;
    void* page_data = page->GetData();
    int32_t current_page_id = page_id;
    
    // 释放锁，进行磁盘写入操作 - 这是避免死锁的关键修复
    lock.unlock();
    
    // 现在在锁外进行磁盘写入操作
    bool write_success = disk_manager_->WritePage(current_page_id, static_cast<const char*>(page_data));
    
    // 重新获取锁 - 使用带超时的锁获取避免永久阻塞
    if (!lock.try_lock_for(std::chrono::milliseconds(write_lock_timeout_ms_))) {
        SQLCC_LOG_WARN("Failed to reacquire buffer pool lock after writing page " + std::to_string(current_page_id) + 
                      ", timeout after " + std::to_string(write_lock_timeout_ms_) + "ms");
        return false;
    }
    
    if (!write_success) {
        // 写入失败，记录错误并返回false
        std::string error_msg = "Failed to write page ID " + std::to_string(current_page_id) + " to disk";
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
        SQLCC_LOG_WARN("Failed to acquire buffer pool lock for deleting page " + std::to_string(page_id) + 
                      ", timeout after " + std::to_string(write_lock_timeout_ms_) + "ms");
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
        SQLCC_LOG_WARN("Page ID " + std::to_string(page_id) + " not found in buffer pool");
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
        SQLCC_LOG_WARN("Page ID " + std::to_string(page_id) + " is still referenced, cannot delete");
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
        Page* page = page_it->second;
        void* page_data = page->GetData();
        int32_t current_page_id = page_id;
        
        // 临时释放锁，进行磁盘写入操作
        lock.unlock();
        
        // 现在在锁外进行磁盘写入操作
        bool write_success = disk_manager_->WritePage(current_page_id, static_cast<const char*>(page_data));
        
        // 重新获取锁 - 使用带超时的锁获取避免永久阻塞
        if (!lock.try_lock_for(std::chrono::milliseconds(write_lock_timeout_ms_))) {
            SQLCC_LOG_WARN("Failed to reacquire buffer pool lock after writing dirty page before deletion " + std::to_string(current_page_id));
            return false;
        }
        
        if (!write_success) {
            // 刷新失败，记录错误并返回false
            std::string error_msg = "Failed to flush dirty page ID " + std::to_string(current_page_id) + " before deletion";
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
    Page* page = page_it->second;
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
    
    // 释放页面对象内存
    // Why: 需要释放页面对象占用的内存，避免内存泄漏
    // What: 删除页面对象，释放其内存
    // How: 使用delete运算符删除页面对象
    delete page;
    
    // 通知磁盘管理器释放页面ID
    // Why: 需要通知磁盘管理器该页面ID可以被重用
    // What: 调用磁盘管理器的DeallocatePage方法释放页面ID
    // How: 传入页面ID，让磁盘管理器更新其内部状态
    if (!disk_manager_->DeallocatePage(page_id)) {
        SQLCC_LOG_ERROR("Failed to notify disk manager to deallocate page " + std::to_string(page_id));
        // 这是一个非关键错误，我们可以继续清理内存中的页面
    }
    
    // 记录页面删除成功，便于调试
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " deleted from buffer pool");
    
    return true;
}

// 取消固定页面实现
// Why: 当一个页面使用完毕后，需要通知缓冲池不再使用该页面，以便缓冲池可以正确管理页面的生命周期和LRU顺序
// What: UnpinPage方法减少页面的引用计数，并标记页面是否被修改
// How: 检查页面是否存在，减少引用计数，标记脏页，然后返回操作结果
bool BufferPool::UnpinPage(int32_t page_id, bool is_dirty) {
    // 加锁保护并发访问 - 使用带超时的unique_lock以支持临时解锁避免死锁
    std::unique_lock<std::timed_mutex> lock(latch_, std::defer_lock);
    if (!lock.try_lock_for(std::chrono::milliseconds(write_lock_timeout_ms_))) {
        // 获取锁失败，记录警告但不抛出异常，避免在高并发场景下级联失败
        SQLCC_LOG_WARN("Failed to acquire buffer pool lock for unpinning page " + std::to_string(page_id) + 
                      ", timeout after " + std::to_string(write_lock_timeout_ms_) + "ms");
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
        SQLCC_LOG_WARN("Page ID " + std::to_string(page_id) + " not found in buffer pool");
        return false;
    }
    
    // 减少引用计数
    // Why: 页面使用完毕，需要减少引用计数，以便缓冲池可以正确管理页面的生命周期
    // What: 减少page_refs_中该页面的引用计数
    // How: 先检查当前引用计数，确保减少后不会变为负数
    if (page_refs_[page_id] > 0) {
        page_refs_[page_id]--;
    } else {
        // 引用计数已经为0或负数，记录严重错误
        // Why: 这表示程序逻辑有问题，可能是重复调用UnpinPage或缺少PinPage
        // What: 记录错误信息但不修改引用计数（避免进一步恶化）
        // How: 使用SQLCC_LOG_ERROR记录错误级别日志
        SQLCC_LOG_ERROR("Attempting to unpin page " + std::to_string(page_id) + " with reference count " + 
                      std::to_string(page_refs_[page_id]));
        // 为了安全性，确保引用计数至少为0
        if (page_refs_[page_id] < 0) {
            page_refs_[page_id] = 0;
        }
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
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " unpinned, refs: " + std::to_string(page_refs_[page_id]));
    
    return true;
}

// 页面替换算法实现
// Why: 当缓冲池已满时，需要选择一个页面进行替换，为新页面腾出空间
// What: ReplacePage方法使用LRU算法选择一个可替换的页面
// How: 从LRU链表尾部开始查找，找到第一个引用计数为0的页面
int32_t BufferPool::ReplacePage() {
    // 加锁保护并发访问 - 使用带超时的unique_lock以支持临时解锁避免死锁
    std::unique_lock<std::timed_mutex> lock(latch_, std::defer_lock);
    if (!lock.try_lock_for(std::chrono::milliseconds(lock_timeout_ms_))) {
        // 获取锁失败，记录警告并抛出异常
        SQLCC_LOG_WARN("Failed to acquire buffer pool lock for page replacement");
        throw LockTimeoutException("Failed to acquire buffer pool lock for page replacement");
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
        // Why: 如果页面是脏的，需要先写回磁盘才能替换，但必须在锁外进行磁盘I/O以避免死锁
        // What: 使用std::unique_lock的unlock()方法临时释放锁，在锁外执行磁盘写入操作
        // How: 先在锁内准备数据，然后解锁调用DiskManager，再重新加锁继续操作
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
            Page* page = page_it->second;
            void* page_data = page->GetData();
            int32_t current_page_id = page_id;
            
            // 先从脏页列表中移除，标记为正在处理
            dirty_pages_.erase(dirty_it);
            
            // 释放锁，进行磁盘写入操作 - 这是避免死锁的关键修复
            // Why: 需要先释放BufferPool锁，再调用DiskManager避免循环锁等待
            // What: 使用std::unique_lock的unlock()方法临时释放锁
            // How: 调用unlock()释放latch_锁，然后进行磁盘写入，再重新加锁
            lock.unlock();
            
            // 现在在锁外进行磁盘写入操作
            // Why: DiskManager::WritePage会获取io_mutex_，必须在BufferPool latch_释放后调用
            // What: 执行实际的页面写入磁盘操作
            // How: 调用disk_manager_->WritePage进行磁盘写入
            bool write_success = disk_manager_->WritePage(current_page_id, static_cast<const char*>(page_data));
            
            // 重新获取锁
            // Why: 需要重新获取锁以继续保护BufferPool的数据结构
            // What: 使用带超时的方式重新加锁
            // How: 调用try_lock_for()尝试重新获取latch_锁
            if (!lock.try_lock_for(std::chrono::milliseconds(lock_timeout_ms_))) {
                // 获取锁失败，记录警告并抛出异常
                SQLCC_LOG_WARN("Failed to re-acquire buffer pool lock after writing page " + std::to_string(current_page_id));
                throw LockTimeoutException("Failed to re-acquire buffer pool lock after writing page");
            }
            
            if (!write_success) {
                // 写入失败，恢复脏页标记
                // Why: 写入磁盘失败可能导致数据丢失，必须恢复脏页状态
                // What: 重新获取锁后恢复页面的脏页标记
                // How: 在dirty_pages_哈希表中重新设置脏页标记
                dirty_pages_[current_page_id] = true;
                std::string error_msg = "Failed to write dirty page ID " + std::to_string(current_page_id) + " to disk";
                SQLCC_LOG_ERROR(error_msg);
                
                // 跳过此页面，继续查找下一个可替换页面
                continue;
            }
            
            // 写入成功，记录日志
            // Why: 成功写入脏页到磁盘，可以安全进行页面替换
            // What: 记录成功的磁盘写入操作，便于调试和监控
            // How: 使用SQLCC_LOG_DEBUG记录调试级别日志
            SQLCC_LOG_DEBUG("Successfully wrote dirty page ID " + std::to_string(current_page_id) + " to disk");
            
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
        if (dirty_it != dirty_pages_.end()) {
            dirty_pages_.erase(dirty_it);
        }
        
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
        // How: 使用pop_back方法从链表尾部删除，并从lru_map_中删除映射
        lru_list_.pop_back();
        lru_map_.erase(page_id);
        
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
// Why: 当页面被访问时，需要将其移动到LRU链表头部，表示最近被访问，这是LRU算法的核心操作
// What: MoveToHead方法将指定页面移动到LRU链表头部，更新LRU映射，以维护页面的访问顺序
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
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " moved to head of LRU list");
}

// 创建新页面实现
// Why: 数据库需要新的存储空间来存储数据，例如插入新记录或创建索引
// What: NewPage方法在缓冲池中分配一个新的页面，并返回页面指针和页面ID
// How: 先确保有空间，然后分配页面ID，创建页面对象，添加到缓冲池管理
Page* BufferPool::NewPage(int32_t* page_id) {
    // 加锁保护并发访问 - 使用带超时的unique_lock以支持临时解锁避免死锁
    std::unique_lock<std::timed_mutex> lock(latch_, std::defer_lock);
    if (!lock.try_lock_for(std::chrono::milliseconds(write_lock_timeout_ms_))) {
        // 获取锁失败，记录警告但不抛出异常，避免在高并发场景下级联失败
        SQLCC_LOG_WARN("Failed to acquire buffer pool lock for creating new page, timeout after " + 
                      std::to_string(write_lock_timeout_ms_) + "ms");
        if (page_id != nullptr) {
            *page_id = -1;  // 使用-1作为无效页面ID
        }
        return nullptr;
    }
    
    // 如果缓冲池已满，先进行页面替换
    // Why: 必须在分配页面ID之前确保有空间，避免页面ID重用造成数据混乱
    // What: 检查page_table_的大小是否达到pool_size_
    // How: 比较page_table_.size()和pool_size_
    if (page_table_.size() >= pool_size_) {
        // 记录缓冲池已满的信息，便于调试
        SQLCC_LOG_DEBUG("Buffer pool is full, replacing page for new page allocation");
        
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
            std::string error_msg = "Failed to replace page in buffer pool for new page allocation";
            SQLCC_LOG_ERROR(error_msg);
            return nullptr;
        }
    }
    
    // 现在分配新的页面ID（此时已有可用空间）
    // Why: 分配页面ID必须在有空间之后，避免ID重用混乱
    // What: 调用磁盘管理器的AllocatePage方法获取新页面ID
    // How: AllocatePage方法返回新的页面ID
    *page_id = disk_manager_->AllocatePage();
    if (*page_id < 0) {
        // 无法分配新页面，记录错误并返回nullptr
        // Why: 磁盘空间不足或磁盘管理器错误导致无法分配新页面
        // What: 记录错误信息并返回nullptr表示失败
        // How: 使用SQLCC_LOG_ERROR记录错误级别日志
        std::string error_msg = "Failed to allocate new page from disk manager";
        SQLCC_LOG_ERROR(error_msg);
        return nullptr;
    }
    
    // 记录新页面分配成功，便于调试
    SQLCC_LOG_DEBUG("Allocated new page ID " + std::to_string(*page_id) + " from disk manager");
    
    // 创建新页面对象
    // Why: 需要创建页面对象来管理新页面的数据和元数据
    // What: 使用页面ID创建新的Page对象
    // How: 使用std::make_unique创建页面对象，然后释放所有权获取指针
    auto page = std::make_unique<Page>(*page_id);
    
    // 初始化页面数据（可选）
    // Why: 新页面可能需要初始化为空或特定值
    // What: 可以选择初始化页面数据为特定值
    // How: 这里不初始化，让调用者自行处理页面数据
    
    // 添加到页面表
    // Why: 需要将新创建的页面加入缓冲池管理，以便后续查找和使用
    // What: 将页面对象添加到page_table_哈希表中
    // How: 使用页面ID作为键，使用std::move转移页面对象的所有权
    Page* page_ptr = page.release(); // 释放所有权，获取原始指针
    page_table_[*page_id] = page_ptr;
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
    SQLCC_LOG_DEBUG("New page ID " + std::to_string(*page_id) + " created in buffer pool");
    
    return page_ptr;
}

std::vector<Page*> BufferPool::BatchFetchPages(const std::vector<int32_t>& page_ids) {
    // 加锁保护并发访问 - 使用unique_lock以支持临时解锁避免死锁
    std::unique_lock<std::timed_mutex> lock(latch_);
    
    std::vector<Page*> result;
    result.reserve(page_ids.size());
    
    // 分离已在缓冲池中和不在缓冲池中页面
    std::vector<int32_t> pages_to_read;
    
    // 首先处理已在缓冲池中的页面
    for (int32_t page_id : page_ids) {
        // 检查页面ID是否有效
        if (page_id < 0) {
            SQLCC_LOG_ERROR("Invalid page ID: " + std::to_string(page_id));
            result.push_back(nullptr);
            continue;
        }
        
        // 更新访问统计
        access_stats_[page_id]++;
        
        auto page_it = page_table_.find(page_id);
        if (page_it != page_table_.end()) {
            // 页面已在缓冲池中
            Page* page = page_it->second;
            page_refs_[page_id]++;
            MoveToHead(page_id);
            result.push_back(page);
        } else {
            // 页面不在缓冲池中，需要从磁盘读取
            pages_to_read.push_back(page_id);
            result.push_back(nullptr); // 占位符，稍后填充
        }
    }
    
    // 如果没有需要从磁盘读取的页面，直接返回
    if (pages_to_read.empty()) {
        return result;
    }
    
    // 按页面ID排序，优化磁盘访问模式
    std::sort(pages_to_read.begin(), pages_to_read.end());
    
    // 检查缓冲池是否有足够空间
    size_t available_space = pool_size_ - page_table_.size();
    if (available_space < pages_to_read.size()) {
        // 需要替换一些页面
        for (size_t i = 0; i < pages_to_read.size() - available_space; ++i) {
            int32_t victim_page_id = ReplacePage();
            if (victim_page_id == -1) {
                // 没有可替换的页面
                SQLCC_LOG_ERROR("Buffer pool is full and no pages can be replaced during batch fetch");
                pages_to_read.resize(available_space);
                break;
            }
        }
    }
    
    // 创建页面对象和数据缓冲区
    std::vector<std::unique_ptr<Page>> new_pages;
    new_pages.reserve(pages_to_read.size());
    std::vector<char*> data_buffers;
    data_buffers.reserve(pages_to_read.size());
    
    for (int32_t page_id : pages_to_read) {
        auto page = std::make_unique<Page>(page_id);
        data_buffers.push_back(page->GetData());
        new_pages.push_back(std::move(page));
    }
    
    // 保存当前需要读取的页面信息，用于后续添加到缓冲池
    std::vector<int32_t> current_pages_to_read = pages_to_read;
    
    // 释放锁，进行磁盘读取操作 - 这是避免死锁的关键修复
    lock.unlock();
    
    // 现在在锁外进行磁盘读取操作
    size_t success_count = disk_manager_->BatchReadPages(current_pages_to_read, data_buffers);
    
    // 重新获取锁
    lock.lock();
    
    // 将成功读取的页面添加到缓冲池
    for (size_t i = 0; i < success_count; ++i) {
        int32_t page_id = pages_to_read[i];
        
        // 将页面添加到缓冲池
        Page* page_ptr = new_pages[i].release(); // 释放所有权，获取原始指针
        page_table_[page_id] = page_ptr;
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
        
        // 在结果中填充页面指针
        for (size_t j = 0; j < result.size(); ++j) {
            if (result[j] == nullptr) {
                result[j] = page_ptr;
                break;
            }
        }
    }
    
    // 记录批量获取页面的信息，便于调试
    SQLCC_LOG_DEBUG("Batch fetched " + std::to_string(success_count) + " pages from disk");
    
    return result;
}

// 刷新所有页面实现
// Why: 需要将所有脏页写回磁盘，确保数据持久性
// What: FlushAllPages方法遍历所有页面，检查脏页标记，将脏页写回磁盘
// How: 遍历page_table_，检查每个页面的脏页标记，如果是脏页则调用WritePage写回磁盘
void BufferPool::FlushAllPages() {
    // 加锁保护并发访问 - 使用带超时的unique_lock以支持临时解锁避免死锁
    std::unique_lock<std::timed_mutex> lock(latch_, std::defer_lock);
    if (!lock.try_lock_for(std::chrono::milliseconds(write_lock_timeout_ms_))) {
        throw LockTimeoutException("Failed to acquire buffer pool lock within timeout: " + std::to_string(write_lock_timeout_ms_) + "ms");
    }
    
    // 记录开始刷新所有页面的信息，便于调试
    SQLCC_LOG_DEBUG("Starting to flush all pages in buffer pool");
    
    // 收集所有脏页信息，以便在锁外处理
    std::vector<std::pair<int32_t, Page*>> dirty_pages_to_flush;
    for (const auto& pair : page_table_) {
        int32_t page_id = pair.first;
        Page* page = pair.second;
        
        // 检查页面是否为脏页
        auto dirty_it = dirty_pages_.find(page_id);
        if (dirty_it != dirty_pages_.end() && dirty_it->second) {
            dirty_pages_to_flush.emplace_back(page_id, page);
        }
    }
    
    lock.unlock(); // 释放锁，进行磁盘I/O操作 - 避免死锁的关键修复
    
    size_t flushed_count = 0;
    // 在锁外执行磁盘I/O操作
    for (const auto& [page_id, page] : dirty_pages_to_flush) {
        // 执行磁盘写入操作
        if (disk_manager_->WritePage(page_id, page->GetData())) {
            // 成功写入后尝试重新获取锁以更新脏页标记
            std::unique_lock<std::timed_mutex> update_lock(latch_, std::defer_lock);
            if (update_lock.try_lock_for(std::chrono::milliseconds(write_lock_timeout_ms_))) {
                dirty_pages_[page_id] = false;
                update_lock.unlock();
            } else {
                // 获取锁失败，记录警告，但继续处理其他页面
                SQLCC_LOG_WARN("Failed to acquire buffer pool lock for updating dirty page flag after flushing page " + std::to_string(page_id));
            }
            
            flushed_count++;
            // 记录页面刷新成功的信息，便于调试
            SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " flushed to disk");
        } else {
            // 写入失败，记录错误
            std::string error_msg = "Failed to flush page ID " + std::to_string(page_id) + " to disk";
            SQLCC_LOG_ERROR(error_msg);
        }
    }
    
    // 重新获取锁以记录最终统计信息
    lock.lock();
    
    // 记录刷新所有页面的统计信息，便于调试
    SQLCC_LOG_DEBUG("Flushed " + std::to_string(flushed_count) + " pages to disk");
}

// 批量预取页面实现
// Why: 根据访问模式预测将要访问的页面，提前从磁盘加载到内存，减少后续访问的延迟
// What: BatchPrefetchPages方法根据配置的预取策略和窗口大小，预取可能需要的页面
// How: 根据访问统计和预取策略，选择要预取的页面，使用磁盘管理器的批量读取功能
bool BufferPool::BatchPrefetchPages(const std::vector<int32_t>& page_ids) {
    // 检查预取是否启用
    bool prefetch_enabled = config_manager_.GetBool("buffer_pool.enable_prefetch", true);
    if (!prefetch_enabled) {
        // 预取功能被禁用，直接返回true
        return true;
    }
    
    // 加锁保护并发访问 - 使用带超时的unique_lock以支持临时解锁避免死锁
    std::unique_lock<std::timed_mutex> lock(latch_, std::defer_lock);
    if (!lock.try_lock_for(std::chrono::milliseconds(read_lock_timeout_ms_))) {
        // 获取锁失败，避免长时间等待，记录警告并返回
        SQLCC_LOG_WARN("Failed to acquire buffer pool lock for batch prefetch, timeout after " + 
                      std::to_string(read_lock_timeout_ms_) + "ms, skipping");
        return false;
    }
    
    // 获取预取策略
    std::string prefetch_strategy = config_manager_.GetString("buffer_pool.prefetch_strategy", "sequential");
    
    // 过滤掉已经在缓冲池或预取队列中的页面
    std::vector<int32_t> pages_to_prefetch;
    for (int32_t page_id : page_ids) {
        // 检查页面ID是否有效
        if (page_id < 0) {
            continue;
        }
        
        // 检查页面是否已在缓冲池中
        if (page_table_.find(page_id) != page_table_.end()) {
            continue;
        }
        
        // 检查页面是否已在预取队列中
        auto it = std::find(prefetch_queue_.begin(), prefetch_queue_.end(), page_id);
        if (it != prefetch_queue_.end()) {
            continue;
        }
        
        pages_to_prefetch.push_back(page_id);
    }
    
    // 如果没有需要预取的页面，直接返回true
    if (pages_to_prefetch.empty()) {
        return true;
    }
    
    // 按页面ID排序，优化磁盘访问模式
    std::sort(pages_to_prefetch.begin(), pages_to_prefetch.end());
    
    // 保存需要预取的页面列表的副本，以便在锁外使用
    std::vector<int32_t> pages_to_prefetch_copy = pages_to_prefetch;
    
    // 释放锁，进行磁盘I/O操作 - 避免死锁的关键修复
    lock.unlock();
    
    // 使用磁盘管理器的批量预取功能（在锁外执行）
    bool result = disk_manager_->BatchPrefetchPages(pages_to_prefetch_copy);
    
    // 重新获取锁以更新预取队列
    if (!lock.try_lock_for(std::chrono::milliseconds(read_lock_timeout_ms_))) {
        // 获取锁失败，避免长时间等待，记录警告并返回
        SQLCC_LOG_WARN("Failed to acquire buffer pool lock for updating prefetch queue, timeout after " + 
                      std::to_string(read_lock_timeout_ms_) + "ms, skipping");
        return result; // 返回磁盘操作的结果
    }
    
    // 将页面ID添加到预取队列
    for (int32_t page_id : pages_to_prefetch_copy) {
        // 再次检查页面是否已在预取队列中（可能已被其他线程添加）
        auto it = std::find(prefetch_queue_.begin(), prefetch_queue_.end(), page_id);
        if (it == prefetch_queue_.end()) {
            prefetch_queue_.push_back(page_id);
        }
    }
    
    // 限制预取队列大小，避免内存占用过多
    if (prefetch_queue_.size() > 1000) {
        // 移除队列头部的旧预取页面
        prefetch_queue_.erase(prefetch_queue_.begin(), prefetch_queue_.begin() + (prefetch_queue_.size() - 1000));
    }
    
    // 记录预取页面的信息，便于调试
    SQLCC_LOG_DEBUG("Added " + std::to_string(pages_to_prefetch_copy.size()) + " pages to prefetch queue");
    
    return result;
}

// 单个预取页面实现
// Why: 预取可以提前加载可能需要的页面，减少未来的磁盘I/O延迟
// What: PrefetchPage方法将指定页面预加载到缓冲池
// How: 类似FetchPage，但不增加页面的固定计数，用于预测性预取
bool BufferPool::PrefetchPage(int32_t page_id) {
    // 检查页面ID是否有效
    if (page_id < 0) {
        SQLCC_LOG_ERROR("Invalid page ID for prefetch: " + std::to_string(page_id));
        return false;
    }
    
    // 检查预取功能是否启用
    bool prefetch_enabled = config_manager_.GetBool("buffer_pool.enable_prefetch", true);
    if (!prefetch_enabled) {
        // 预取功能被禁用
        SQLCC_LOG_DEBUG("Prefetch disabled, skipping page " + std::to_string(page_id));
        return false;
    }
    
    // 加锁保护并发访问 - 使用带超时的unique_lock以支持临时解锁避免死锁
    std::unique_lock<std::timed_mutex> lock(latch_, std::defer_lock);
    if (!lock.try_lock_for(std::chrono::milliseconds(read_lock_timeout_ms_))) {
        // 获取锁失败，避免长时间等待，记录警告并返回
        SQLCC_LOG_WARN("Failed to acquire buffer pool lock for prefetch, timeout after " + 
                      std::to_string(read_lock_timeout_ms_) + "ms, skipping page " + std::to_string(page_id));
        return false;
    }
    
    // 检查页面是否已在缓冲池中
    auto it = page_table_.find(page_id);
    if (it != page_table_.end()) {
        // 页面已在缓冲池中，无需预取
        SQLCC_LOG_DEBUG("Page " + std::to_string(page_id) + " already in buffer pool");
        return true;
    }
    
    // 检查页面是否已在预取队列中
    auto queue_it = std::find(prefetch_queue_.begin(), prefetch_queue_.end(), page_id);
    if (queue_it != prefetch_queue_.end()) {
        // 页面已在预取队列中，无需重复预取
        SQLCC_LOG_DEBUG("Page " + std::to_string(page_id) + " already in prefetch queue");
        return true;
    }
    
    // 检查缓冲池是否已满
    if (page_table_.size() >= pool_size_) {
        // 尝试替换一个页面
        int32_t victim_page_id = ReplacePage();
        if (victim_page_id == -1) {
            // 没有可替换的页面，预取失败
            SQLCC_LOG_WARN("Buffer pool is full and no pages can be replaced, prefetch failed for page " + std::to_string(page_id));
            return false;
        }
    }
    
    // 创建新页面对象
    Page* page = new Page();
    page->SetPageId(page_id);
    
    // 释放锁，进行磁盘I/O操作 - 避免死锁的关键修复
    lock.unlock();
    
    // 从磁盘加载页面数据（在锁外执行）
    bool read_success = disk_manager_->ReadPage(page_id, page->GetData());
    if (!read_success) {
        // 磁盘读取失败，清理资源并返回
        SQLCC_LOG_ERROR("Failed to read page " + std::to_string(page_id) + " from disk");
        delete page;
        return false;
    }
    
    // 重新获取锁以更新缓冲池状态
    if (!lock.try_lock_for(std::chrono::milliseconds(read_lock_timeout_ms_))) {
        // 获取锁失败，清理资源并返回
        SQLCC_LOG_WARN("Failed to acquire buffer pool lock after disk read, timeout after " + 
                      std::to_string(read_lock_timeout_ms_) + "ms, skipping page " + std::to_string(page_id));
        delete page;
        return false;
    }
    
    // 再次检查页面是否已在缓冲池中（可能已被其他线程添加）
    if (page_table_.find(page_id) != page_table_.end()) {
        // 页面已被其他线程添加到缓冲池，清理资源并返回
        delete page;
        return true;
    }
    
    // 再次检查页面是否已在预取队列中
    queue_it = std::find(prefetch_queue_.begin(), prefetch_queue_.end(), page_id);
    if (queue_it != prefetch_queue_.end()) {
        // 页面已被其他线程添加到预取队列，清理资源并返回
        delete page;
        return true;
    }
    
    // 将页面添加到缓冲池
    page_table_[page_id] = page;
    lru_list_.push_front(page_id);
    lru_map_[page_id] = lru_list_.begin();
    
    // 预取页面的引用计数为0（表示预取）
    page_refs_[page_id] = 0;
    
    // 预取页面不是脏页
    dirty_pages_[page_id] = false;
    
    // 添加到预取队列
    prefetch_queue_.push_back(page_id);
    
    // 更新统计信息
    stats_.total_prefetches++;
    
    // 记录预取成功信息
    SQLCC_LOG_DEBUG("Successfully prefetched page " + std::to_string(page_id) + " into buffer pool");
    
    return true;
}

// 获取缓冲池统计信息实现
// Why: 需要提供缓冲池的运行时统计信息，用于监控和性能分析
// What: GetStats方法返回包含缓冲池各种统计信息的哈希表
// How: 收集缓冲池的当前状态信息，包括页面数量、引用计数、脏页数量等
std::unordered_map<std::string, double> BufferPool::GetStats() const {
    // 加锁保护并发访问 - 使用unique_lock以支持临时解锁避免死锁
    std::unique_lock<std::timed_mutex> lock(latch_);
    
    // 创建统计信息哈希表
    // Why: 需要返回一个包含所有统计信息的哈希表
    // What: 创建std::unordered_map来存储各种统计信息
    // How: 使用默认构造函数创建空的哈希表
    std::unordered_map<std::string, double> stats;
    
    // 计算基本统计信息
    // Why: 需要提供缓冲池的基本状态信息
    // What: 计算页面数量、引用计数、脏页数量等
    // How: 使用哈希表的size()方法和遍历计算各种统计信息
    stats["pool_size"] = static_cast<int32_t>(pool_size_);
    stats["current_pages"] = static_cast<int32_t>(page_table_.size());
    stats["free_pages"] = static_cast<int32_t>(pool_size_ - page_table_.size());
    
    // 计算引用统计信息
    int32_t total_refs = 0;
    int32_t max_refs = 0;
    int32_t min_refs = INT32_MAX;
    
    for (const auto& pair : page_refs_) {
        int32_t refs = pair.second;
        total_refs += refs;
        max_refs = std::max(max_refs, refs);
        min_refs = std::min(min_refs, refs);
    }
    
    stats["total_refs"] = total_refs;
    stats["max_refs"] = (page_refs_.empty() ? 0 : max_refs);
    stats["min_refs"] = (page_refs_.empty() ? 0 : min_refs);
    stats["avg_refs"] = (page_refs_.empty() ? 0 : total_refs / static_cast<int32_t>(page_refs_.size()));
    
    // 计算脏页统计信息
    int32_t dirty_pages = 0;
    for (const auto& pair : dirty_pages_) {
        if (pair.second) {
            dirty_pages++;
        }
    }
    
    stats["dirty_pages"] = dirty_pages;
    stats["clean_pages"] = static_cast<int32_t>(page_table_.size() - dirty_pages);
    stats["dirty_ratio"] = (page_table_.empty() ? 0 : (dirty_pages * 100) / static_cast<int32_t>(page_table_.size()));
    
    // LRU链表统计信息
    stats["lru_size"] = static_cast<int32_t>(lru_list_.size());
    stats["lru_front"] = (lru_list_.empty() ? -1 : lru_list_.front());
    stats["lru_back"] = (lru_list_.empty() ? -1 : lru_list_.back());
    
    // 预取队列统计信息
    stats["prefetch_queue_size"] = static_cast<int32_t>(prefetch_queue_.size());
    stats["prefetch_enabled"] = (config_manager_.GetBool("buffer_pool.enable_prefetch", true) ? 1 : 0);
    
    // 命中率统计信息（这里使用简单的访问计数作为示例）
    // 注意：实际的命中率计算需要更复杂的逻辑，这里仅提供基础统计
    stats["cache_hits"] = static_cast<int32_t>(page_table_.size()); // 简化实现
    stats["cache_misses"] = static_cast<int32_t>(prefetch_queue_.size()); // 简化实现
    stats["hit_ratio"] = (page_table_.empty() ? 0 : (static_cast<int32_t>(page_table_.size()) * 100) / 
                        (static_cast<int32_t>(page_table_.size()) + static_cast<int32_t>(prefetch_queue_.size())));
    
    // 记录获取统计信息的调试信息
    SQLCC_LOG_DEBUG("Buffer pool stats collected: " + std::to_string(stats.size()) + " metrics");
    
    return stats;
}

// 配置变更回调处理实现
// Why: 需要响应配置变更，动态调整缓冲池行为
// What: OnConfigChange方法处理配置变更事件，根据变更的配置项调整相应的缓冲池参数
// How: 根据配置键判断变更类型，执行相应的调整操作
void BufferPool::OnConfigChange(const std::string& key, const ConfigValue& /*value*/) {
    // 注意：由于配置回调功能已禁用，此方法现在不会被调用
    // 保留此方法以保持接口兼容性
    SQLCC_LOG_DEBUG("Dynamic configuration change handling is disabled: " + key);
}

// 从LRU列表中移除页面实现
// Why: 当页面被替换或删除时，需要从LRU列表中移除，保持LRU列表的准确性
// What: RemoveFromLRUList方法从LRU列表中移除指定页面ID
// How: 在lru_map_中查找页面ID对应的迭代器，然后从lru_list_中移除该元素
void BufferPool::RemoveFromLRUList(int32_t page_id) {
    // 查找页面在LRU映射中的位置
    auto it = lru_map_.find(page_id);
    if (it != lru_map_.end()) {
        // 从LRU列表中移除页面
        lru_list_.erase(it->second);
        // 从LRU映射中移除条目
        lru_map_.erase(it);
        
        // 记录页面从LRU列表移除的调试信息
        SQLCC_LOG_DEBUG("Page " + std::to_string(page_id) + " removed from LRU list");
    } else {
        // 页面不在LRU列表中，记录警告信息
        SQLCC_LOG_WARN("Page " + std::to_string(page_id) + " not found in LRU list for removal");
    }
}

// 检查页面是否在缓冲池中实现
// Why: 需要快速判断页面是否已加载到内存中，避免不必要的磁盘I/O
// What: IsPageInBuffer方法检查指定页面ID是否存在于缓冲池中
// How: 检查页面表是否包含该页面ID
bool BufferPool::IsPageInBuffer(int32_t page_id) const {
    // 加锁保护并发访问 - 使用unique_lock以支持临时解锁避免死锁
    std::unique_lock<std::timed_mutex> lock(latch_);
    
    // 检查页面是否在页面表中
    // Why: 页面表存储了当前在缓冲池中的所有页面
    // What: 检查page_table_哈希表是否包含指定的页面ID
    // How: 使用std::unordered_map的find方法查找页面ID
    return page_table_.find(page_id) != page_table_.end();
}

// 内部方法：在持有锁状态下移除页面，避免死锁
// Why: AdjustBufferPoolSize方法需要在持锁状态下移除页面，需要专门的内部方法避免与ReplacePage产生死锁
// What: ReplacePageInternal方法在持锁状态下移除一个页面，不涉及重新获取锁
// How: 在当前锁保护下直接移除页面，不调用需要重新获取锁的方法
int32_t BufferPool::ReplacePageInternal() {
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
        
        // 检查脏页标记
        // Why: 如果页面是脏的，需要先写回磁盘才能替换
        // What: 检查页面是否在脏页列表中
        // How: 使用std::unordered_map的find方法查找页面ID
        auto dirty_it = dirty_pages_.find(page_id);
        if (dirty_it != dirty_pages_.end() && dirty_it->second) {
            // 跳过脏页，不在此次调整中处理，避免复杂的磁盘I/O操作
            // Why: 在配置调整过程中处理脏页可能引起复杂的状态同步问题
            // What: 跳过脏页，专注于页面数量调整
            // How: 继续查找下一个可替换的页面
            continue;
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
        if (dirty_it != dirty_pages_.end()) {
            dirty_pages_.erase(dirty_it);
        }
        
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
        // How: 使用pop_back方法从链表尾部删除，并从lru_map_中删除映射
        lru_list_.pop_back();
        lru_map_.erase(page_id);
        
        // 记录页面移除成功，便于调试
        SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " removed during buffer pool size adjustment");
        return page_id;
    }
    
    // 无法找到可替换的页面，记录警告并返回-1
    SQLCC_LOG_WARN("No clean page can be removed in buffer pool size adjustment");
    return -1;
}

// 异步调整缓冲池大小实现
// Why: 避免在配置回调中直接调用ReplacePage可能引起的死锁问题
// What: AdjustBufferPoolSize方法安全地调整缓冲池大小
// How: 在独立线程中执行大小调整，使用专用的内部方法避免死锁
void BufferPool::AdjustBufferPoolSize(size_t new_pool_size) {
    // 加锁保护并发访问 - 使用unique_lock以支持临时解锁避免死锁
    std::unique_lock<std::timed_mutex> lock(latch_);
    
    SQLCC_LOG_INFO("Adjusting buffer pool size from " + std::to_string(pool_size_) + " to " + std::to_string(new_pool_size));
    
    size_t current_size = page_table_.size();
    
    // 如果新大小小于当前大小，需要移除多余的页面
    if (new_pool_size < current_size) {
        // 需要移除的页面数量
        size_t pages_to_remove = current_size - new_pool_size;
        SQLCC_LOG_INFO("Need to remove " + std::to_string(pages_to_remove) + " clean pages from buffer pool");
        
        // 使用内部方法移除页面，避免死锁
        for (size_t i = 0; i < pages_to_remove; ++i) {
            // 检查是否已经达到目标大小
            if (page_table_.size() <= new_pool_size) {
                break;
            }
            
            // 使用内部方法移除页面，不涉及锁的重新获取
            int32_t removed_page_id = ReplacePageInternal();
            if (removed_page_id == -1) {
                SQLCC_LOG_WARN("Failed to remove page during buffer pool size adjustment");
                break;
            }
        }
        
    } else if (new_pool_size > pool_size_) {
        SQLCC_LOG_INFO("Increasing buffer pool size from " + std::to_string(pool_size_) + " to " + std::to_string(new_pool_size));
        // 新大小更大时，不需要立即移除页面，新的页面会在需要时添加
    }
    
    // 更新缓冲池大小
    pool_size_ = new_pool_size;
    SQLCC_LOG_INFO("Buffer pool size adjustment completed. New size: " + std::to_string(pool_size_));
}

// 无锁版本的安全调整缓冲池大小方法实现
// Why: 需要在不需要获取锁的情况下调整缓冲池大小，避免死锁
// What: AdjustBufferPoolSizeNoLock方法通过发送消息到队列的方式触发异步调整
// How: 不直接获取锁，而是通过std::async启动异步任务来执行调整
void BufferPool::AdjustBufferPoolSizeNoLock(size_t new_pool_size) {
    // 使用std::async启动异步任务，避免在配置回调线程中直接获取锁
    // Why: std::async会在独立的线程中执行，避免与当前线程产生锁竞争
    // What: 启动一个异步任务来执行缓冲池大小调整
    // How: 传递原始指针引用，确保BufferPool对象在异步任务执行期间保持有效
    auto future = std::async(std::launch::async, [this, new_pool_size]() {
        // 在异步任务中调用原始的AdjustBufferPoolSize方法
        // Why: 异步任务会在独立的线程中执行，避免与配置回调线程产生锁竞争
        // What: 在独立的执行线程中调整缓冲池大小
        // How: 调用AdjustBufferPoolSize方法执行实际的大小调整逻辑
        this->AdjustBufferPoolSize(new_pool_size);
    });
    // 忽略future对象，不等待异步任务完成，避免阻塞当前线程
    (void)future;
    SQLCC_LOG_INFO("Buffer pool size adjustment initiated asynchronously to " + std::to_string(new_pool_size));
}

} // namespace sqlcc