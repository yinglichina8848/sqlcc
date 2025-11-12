#include "buffer_pool.h"
#include "exception.h"
#include "logger.h"
#include <iostream>
#include <algorithm>

namespace sqlcc {

// 构造函数实现
// Why: 需要初始化缓冲池的基本状态，包括设置磁盘管理器和缓冲池大小
// What: 构造函数初始化成员变量，并记录初始化日志
// How: 使用成员初始化列表初始化disk_manager_、pool_size_和config_manager_，并记录日志
BufferPool::BufferPool(DiskManager* disk_manager, size_t pool_size, ConfigManager& config_manager)
    : disk_manager_(disk_manager), config_manager_(config_manager), pool_size_(pool_size), simulate_flush_failure_(false) {
    // 预分配批量操作缓冲区空间
    batch_buffer_.reserve(std::min(pool_size_, static_cast<size_t>(64)));
    
    // 记录缓冲池初始化信息，便于调试和监控
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录缓冲池大小信息
    // How: 使用SQLCC_LOG_INFO宏记录信息级别日志
    SQLCC_LOG_INFO("Initializing BufferPool with pool size: " + std::to_string(pool_size_));
    
    // 注册配置变更回调
    // Why: 需要响应配置变更，动态调整缓冲池行为
    // What: 注册配置变更回调函数，当配置发生变更时调用OnConfigChange方法
    // How: 使用配置管理器的RegisterChangeCallback方法注册回调
    config_manager_.RegisterChangeCallback("buffer_pool.pool_size", 
        [this](const std::string& key, const ConfigValue& value) {
            this->OnConfigChange(key, value);
        });
    
    config_manager_.RegisterChangeCallback("buffer_pool.enable_prefetch", 
        [this](const std::string& key, const ConfigValue& value) {
            this->OnConfigChange(key, value);
        });
    
    config_manager_.RegisterChangeCallback("buffer_pool.prefetch_strategy", 
        [this](const std::string& key, const ConfigValue& value) {
            this->OnConfigChange(key, value);
        });
    
    config_manager_.RegisterChangeCallback("buffer_pool.prefetch_window", 
        [this](const std::string& key, const ConfigValue& value) {
            this->OnConfigChange(key, value);
        });
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
    // 加锁保护并发访问
    std::lock_guard<std::mutex> lock(latch_);
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
    
    // 从磁盘读取页面数据
    if (!disk_manager_->ReadPage(page_id, page->GetData())) {
        // 如果读取失败，说明页面不存在，返回nullptr
        SQLCC_LOG_DEBUG("Failed to read page ID " + std::to_string(page_id) + " from disk, page does not exist");
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
    // 加锁保护并发访问
    std::lock_guard<std::mutex> lock(latch_);
    
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
    
    // 将页面数据写入磁盘
    // Why: 脏页的内容与磁盘不一致，需要写回磁盘以保证数据持久性
    // What: 调用磁盘管理器的WritePage方法将页面写入磁盘
    // How: 传入页面ID和页面数据指针
    Page* page = page_it->second;
    if (!disk_manager_->WritePage(page_id, page->GetData())) {
        // 写入失败，记录错误并返回false
        // Why: 磁盘写入失败可能导致数据丢失，需要记录错误
        // What: 记录错误信息并返回false表示操作失败
        // How: 使用SQLCC_LOG_ERROR记录错误级别日志
        std::string error_msg = "Failed to write page ID " + std::to_string(page_id) + " to disk";
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
    // 加锁保护并发访问
    std::lock_guard<std::mutex> lock(latch_);
    
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
    // How: 使用FlushPage方法刷新脏页
    auto dirty_it = dirty_pages_.find(page_id);
    if (dirty_it != dirty_pages_.end() && dirty_it->second) {
        // 页面是脏的，需要先刷新到磁盘
        // Why: 脏页的内容与磁盘不一致，需要写回磁盘以保证数据持久性
        // What: 调用FlushPage方法将页面写入磁盘
        // How: 递归调用FlushPage方法，传入页面ID
        if (!FlushPage(page_id)) {
            // 刷新失败，记录错误并返回false
            // Why: 无法将脏页写回磁盘，不能安全删除页面
            // What: 记录错误信息并返回false表示操作失败
            // How: 使用SQLCC_LOG_ERROR记录错误级别日志
            std::string error_msg = "Failed to flush dirty page ID " + std::to_string(page_id) + " before deletion";
            SQLCC_LOG_ERROR(error_msg);
            return false;
        }
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
    // 加锁保护并发访问
    std::lock_guard<std::mutex> lock(latch_);
    
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
    // How: 直接递减引用计数值，并确保不会变成负数
    page_refs_[page_id]--;
    if (page_refs_[page_id] < 0) {
        // 引用计数不应该为负数，记录错误并修正
        // Why: 引用计数为负数表示逻辑错误，需要记录并修正
        // What: 记录错误信息并将引用计数重置为0
        // How: 使用SQLCC_LOG_ERROR记录错误，然后将引用计数设为0
        SQLCC_LOG_ERROR("Page ID " + std::to_string(page_id) + " reference count is negative");
        page_refs_[page_id] = 0;
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
        // What: 检查dirty_pages_中该页面是否为脏页
        // How: 使用std::unordered_map的find方法查找脏页标记
        auto dirty_it = dirty_pages_.find(page_id);
        if (dirty_it != dirty_pages_.end() && dirty_it->second) {
            // 页面是脏的，需要先写回磁盘
            // Why: 脏页的内容与磁盘不一致，需要写回磁盘以保证数据持久性
            // What: 调用磁盘管理器的WritePage方法将页面写入磁盘
            // How: 传入页面对象引用，由磁盘管理器负责实际的磁盘I/O操作
            // 如果页面是脏的，需要先写回磁盘
            if (dirty_pages_[page_id]) {
                // 查找页面在页面表中的位置
                auto page_it = page_table_.find(page_id);
                if (page_it != page_table_.end() && !disk_manager_->WritePage(page_id, page_it->second->GetData())) {
                    // 写入失败，记录错误并继续查找下一个页面
                    // Why: 写入磁盘失败可能导致数据丢失，不应替换此页面
                    // What: 创建错误消息，记录错误日志，然后继续查找下一个页面
                    // How: 使用SQLCC_LOG_ERROR记录错误级别日志，然后继续循环
                    std::string error_msg = "Failed to write dirty page ID " + std::to_string(page_id) + " to disk";
                    SQLCC_LOG_ERROR(error_msg);
                    // 无法刷新脏页，不能替换此页面，继续查找
                    continue;
                }
            }
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
// How: 从磁盘管理器获取新页面ID，创建页面对象，添加到缓冲池管理
Page* BufferPool::NewPage(int32_t* page_id) {
    // 加锁保护并发访问
    std::lock_guard<std::mutex> lock(latch_);
    
    // 从磁盘管理器获取新页面ID
    // Why: 需要磁盘管理器分配一个新的页面ID
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
    
    // 如果缓冲池已满，需要替换页面
    // Why: 缓冲池大小有限，当已满时必须选择一个页面进行替换
    // What: 检查page_table_的大小是否超过pool_size_
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
            // 无法替换页面，释放分配的页面ID并返回nullptr
            // Why: 如果所有页面都在使用中，无法为新页面腾出空间，必须报错
            // What: 释放已分配的页面ID并返回nullptr
            // How: 调用磁盘管理器的DeallocatePage方法释放页面ID
            disk_manager_->DeallocatePage(*page_id);
            std::string error_msg = "Failed to replace page in buffer pool for new page allocation";
            SQLCC_LOG_ERROR(error_msg);
            return nullptr;
        }
    }
    
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
    // 加锁保护并发访问
    std::lock_guard<std::mutex> lock(latch_);
    
    std::vector<Page*> result;
    result.reserve(page_ids.size());
    
    // 分离已在缓冲池中和不在缓冲池中的页面
    std::vector<int32_t> pages_to_read;
    std::vector<char*> data_buffers;
    
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
    data_buffers.reserve(pages_to_read.size());
    
    for (int32_t page_id : pages_to_read) {
        auto page = std::make_unique<Page>(page_id);
        data_buffers.push_back(page->GetData());
        new_pages.push_back(std::move(page));
    }
    
    // 使用磁盘管理器的批量读取功能
    size_t success_count = disk_manager_->BatchReadPages(pages_to_read, data_buffers);
    
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
    // 加锁保护并发访问
    std::lock_guard<std::mutex> lock(latch_);
    
    // 记录开始刷新所有页面的信息，便于调试
    SQLCC_LOG_DEBUG("Starting to flush all pages in buffer pool");
    
    size_t flushed_count = 0;
    
    // 遍历所有页面
    // Why: 需要检查每个页面是否为脏页，如果是则需要写回磁盘
    // What: 遍历page_table_中的所有页面
    // How: 使用范围for循环遍历std::unordered_map
    for (const auto& pair : page_table_) {
        int32_t page_id = pair.first;
        Page* page = pair.second;
        
        // 检查页面是否为脏页
        // Why: 只有脏页才需要写回磁盘，非脏页不需要写回
        // What: 检查dirty_pages_中该页面是否为脏页
        // How: 使用std::unordered_map的find方法查找脏页标记
        auto dirty_it = dirty_pages_.find(page_id);
        if (dirty_it != dirty_pages_.end() && dirty_it->second) {
            // 页面是脏的，需要写回磁盘
            // Why: 脏页的内容与磁盘不一致，需要写回磁盘以保证数据持久性
            // What: 调用磁盘管理器的WritePage方法将页面写入磁盘
            // How: 传入页面对象引用，由磁盘管理器负责实际的磁盘I/O操作
            if (!disk_manager_->WritePage(page_id, page->GetData())) {
                // 写入失败，记录错误并继续处理其他页面
                // Why: 单个页面写入失败不应该影响其他页面的刷新
                // What: 记录错误信息，继续处理其他页面
                // How: 使用SQLCC_LOG_ERROR记录错误级别日志
                std::string error_msg = "Failed to flush page ID " + std::to_string(page_id) + " to disk";
                SQLCC_LOG_ERROR(error_msg);
                continue;
            }
            
            // 刷新成功后，清除脏页标记
            // Why: 页面已经成功写回磁盘，不再需要标记为脏页
            // What: 将dirty_pages_中该页面的脏页标记设置为false
            // How: 直接设置哈希表中的值为false
            dirty_pages_[page_id] = false;
            flushed_count++;
            
            // 记录页面刷新成功的信息，便于调试
            SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " flushed to disk");
        }
    }
    
    // 记录刷新所有页面的统计信息，便于调试
    SQLCC_LOG_DEBUG("Flushed " + std::to_string(flushed_count) + " pages to disk");
}

// 批量预取页面实现
// Why: 根据访问模式预测将要访问的页面，提前从磁盘加载到内存，减少后续访问的延迟
// What: BatchPrefetchPages方法根据配置的预取策略和窗口大小，预取可能需要的页面
// How: 根据访问统计和预取策略，选择要预取的页面，使用磁盘管理器的批量读取功能
bool BufferPool::BatchPrefetchPages(const std::vector<int32_t>& page_ids) {
    // 加锁保护并发访问
    std::lock_guard<std::mutex> lock(latch_);
    
    // 检查预取是否启用
    // Why: 预取功能可能通过配置禁用，需要检查配置状态
    // What: 检查config_manager_中预取功能的配置值
    // How: 使用配置管理器的GetBool方法获取配置值
    if (!config_manager_.GetBool("buffer_pool.enable_prefetch", true)) {
        // 预取功能被禁用，直接返回true
        return true;
    }
    
    // 获取预取策略
    // Why: 需要根据配置的预取策略来决定预取哪些页面
    // What: 从配置管理器获取预取策略
    // How: 使用配置管理器的GetString方法获取配置值
    std::string prefetch_strategy = config_manager_.GetString("buffer_pool.prefetch_strategy", "sequential");
    
    // 过滤掉已经在缓冲池或预取队列中的页面
    // Why: 避免重复预取已经在内存中的页面，提高预取效率
    // What: 过滤掉已经在page_table_或prefetch_queue_中的页面ID
    // How: 使用std::find算法检查页面是否已存在
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
    
    // 使用磁盘管理器的批量预取功能
    bool result = disk_manager_->BatchPrefetchPages(pages_to_prefetch);
    
    // 将页面ID添加到预取队列
    for (int32_t page_id : pages_to_prefetch) {
        prefetch_queue_.push_back(page_id);
    }
    
    // 记录预取页面的信息，便于调试
    SQLCC_LOG_DEBUG("Added " + std::to_string(pages_to_prefetch.size()) + " pages to prefetch queue");
    
    return result;
}

// 单个预取页面实现
// Why: 预取可以提前加载可能需要的页面，减少未来的磁盘I/O延迟
// What: PrefetchPage方法将指定页面预加载到缓冲池
// How: 类似FetchPage，但不增加页面的固定计数，用于预测性预取
bool BufferPool::PrefetchPage(int32_t page_id) {
    // 加锁保护并发访问
    std::lock_guard<std::mutex> lock(latch_);
    
    // 检查页面ID是否有效
    if (page_id < 0) {
        SQLCC_LOG_ERROR("Invalid page ID for prefetch: " + std::to_string(page_id));
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
    
    // 检查预取功能是否启用
    if (!config_manager_.GetBool("buffer_pool.enable_prefetch", true)) {
        // 预取功能被禁用
        SQLCC_LOG_DEBUG("Prefetch disabled, skipping page " + std::to_string(page_id));
        return false;
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
    
    // 从磁盘加载页面数据
    bool read_success = disk_manager_->ReadPage(page_id, page->GetData());
    if (!read_success) {
        // 磁盘读取失败，清理资源并返回
        SQLCC_LOG_ERROR("Failed to read page " + std::to_string(page_id) + " from disk");
        delete page;
        return false;
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
    // 加锁保护并发访问
    std::lock_guard<std::mutex> lock(latch_);
    
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
void BufferPool::OnConfigChange(const std::string& key, const ConfigValue& value) {
    // 使用访问者模式处理配置值
    auto visitor = [this, &key](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        
        if (key == "buffer_pool.pool_size") {
            // 缓冲池大小配置变更
            if constexpr (std::is_same_v<T, std::string>) {
                size_t new_pool_size = std::stoull(arg);
                if (new_pool_size < pool_size_) {
                    // 新的池大小小于当前大小，需要替换一些页面
                    while (page_table_.size() > new_pool_size) {
                        int32_t victim_page_id = ReplacePage();
                        if (victim_page_id == -1) {
                            SQLCC_LOG_WARN("Cannot reduce buffer pool size, no pages can be replaced");
                            break;
                        }
                    }
                }
                pool_size_ = new_pool_size;
                SQLCC_LOG_INFO("Buffer pool size changed to " + std::to_string(pool_size_));
            } else if constexpr (std::is_same_v<T, int>) {
                size_t new_pool_size = static_cast<size_t>(arg);
                if (new_pool_size < pool_size_) {
                    while (page_table_.size() > new_pool_size) {
                        int32_t victim_page_id = ReplacePage();
                        if (victim_page_id == -1) {
                            SQLCC_LOG_WARN("Cannot reduce buffer pool size, no pages can be replaced");
                            break;
                        }
                    }
                }
                pool_size_ = new_pool_size;
                SQLCC_LOG_INFO("Buffer pool size changed to " + std::to_string(pool_size_));
            }
        } else if (key == "buffer_pool.enable_prefetch") {
            // 预取功能配置变更
            if constexpr (std::is_same_v<T, bool>) {
                bool enable_prefetch = arg;
                SQLCC_LOG_INFO("Buffer pool prefetch " + std::string(enable_prefetch ? "enabled" : "disabled"));
            } else if constexpr (std::is_same_v<T, std::string>) {
                bool enable_prefetch = (arg == "true" || arg == "1");
                SQLCC_LOG_INFO("Buffer pool prefetch " + std::string(enable_prefetch ? "enabled" : "disabled"));
            }
        } else if (key == "buffer_pool.prefetch_strategy") {
            // 预取策略配置变更
            if constexpr (std::is_same_v<T, std::string>) {
                SQLCC_LOG_INFO("Buffer pool prefetch strategy changed to " + arg);
            }
        } else if (key == "buffer_pool.prefetch_window") {
            // 预取窗口大小配置变更
            if constexpr (std::is_same_v<T, int>) {
                SQLCC_LOG_INFO("Buffer pool prefetch window changed to " + std::to_string(arg));
            } else if constexpr (std::is_same_v<T, std::string>) {
                int prefetch_window = std::stoi(arg);
                SQLCC_LOG_INFO("Buffer pool prefetch window changed to " + std::to_string(prefetch_window));
            }
        }
    };
    
    // 记录配置变更信息，便于调试和监控
    std::string value_str;
    if (std::holds_alternative<bool>(value)) {
        value_str = std::get<bool>(value) ? "true" : "false";
    } else if (std::holds_alternative<int>(value)) {
        value_str = std::to_string(std::get<int>(value));
    } else if (std::holds_alternative<double>(value)) {
        value_str = std::to_string(std::get<double>(value));
    } else if (std::holds_alternative<std::string>(value)) {
        value_str = std::get<std::string>(value);
    }
    SQLCC_LOG_DEBUG("Configuration changed: " + key + " = " + value_str);
    
    // 应用配置变更
    std::visit(visitor, value);
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
    // 加锁保护并发访问
    std::lock_guard<std::mutex> lock(latch_);
    
    // 检查页面是否在页面表中
    // Why: 页面表存储了当前在缓冲池中的所有页面
    // What: 检查page_table_哈希表是否包含指定的页面ID
    // How: 使用std::unordered_map的find方法查找页面ID
    return page_table_.find(page_id) != page_table_.end();
}

} // namespace sqlcc