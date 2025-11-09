#include "buffer_pool.h"
#include "exception.h"
#include "logger.h"
#include <iostream>
#include <algorithm>

namespace sqlcc {

// 构造函数实现
// Why: 需要初始化缓冲池的基本状态，包括设置磁盘管理器和缓冲池大小
// What: 构造函数初始化成员变量，并记录初始化日志
// How: 使用成员初始化列表初始化disk_manager_和pool_size_，并记录日志
BufferPool::BufferPool(DiskManager* disk_manager, size_t pool_size)
    : disk_manager_(disk_manager), pool_size_(pool_size) {
    // 预分配批量操作缓冲区空间
    batch_buffer_.reserve(std::min(pool_size_, static_cast<size_t>(64)));
    
    // 记录缓冲池初始化信息，便于调试和监控
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录缓冲池大小信息
    // How: 使用SQLCC_LOG_INFO宏记录信息级别日志
    SQLCC_LOG_INFO("Initializing BufferPool with pool size: " + std::to_string(pool_size_));
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
        return it->second.get();
    }

    // 页面不在缓冲池中，需要从磁盘读取
    // Why: 页面不在内存中，必须从磁盘加载，这是数据库I/O操作的主要来源
    // What: 记录页面不在缓冲池中的信息
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " not found in buffer pool, reading from disk");
    
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

    // 创建新页面对象
    // Why: 需要创建一个页面对象来存储从磁盘读取的数据
    // What: 使用std::make_unique创建一个新的Page对象
    // How: 传入页面ID作为构造函数参数
    auto page = std::make_unique<Page>(page_id);
    
    // 从磁盘读取页面数据
    // Why: 页面数据存储在磁盘上，需要读取到内存中才能使用
    // What: 调用磁盘管理器的ReadPage方法读取页面数据
    // How: 传入页面ID和页面对象指针，如果页面不存在则返回false
    if (!disk_manager_->ReadPage(page_id, page.get())) {
        // 如果读取失败，说明页面不存在，返回nullptr
        // Why: 页面可能不存在于磁盘上，这是正常情况，需要返回nullptr
        // What: 记录读取失败信息并返回nullptr
        // How: 使用SQLCC_LOG_DEBUG记录信息，然后返回nullptr
        SQLCC_LOG_DEBUG("Failed to read page ID " + std::to_string(page_id) + " from disk, page does not exist");
        return nullptr;
    }

    // 添加到页面表
    // Why: 需要将新加载的页面加入缓冲池管理，以便后续查找和使用
    // What: 将页面对象添加到page_table_哈希表中
    // How: 使用页面ID作为键，使用std::move转移页面对象的所有权
    Page* page_ptr = page.get();
    page_table_[page_id] = std::move(page);
    
    // 初始化引用计数
    // Why: 新加载的页面需要初始化引用计数，以跟踪其使用情况
    // What: 将页面引用计数设置为1，表示已被一个操作引用
    // How: 在page_refs_哈希表中设置页面ID的值为1
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

// 取消固定页面实现
// Why: 当一个页面使用完毕后，需要通知缓冲池不再使用该页面，以便缓冲池可以正确管理页面的生命周期和LRU顺序
// What: UnpinPage方法减少页面的引用计数，并标记页面是否被修改
// How: 检查页面是否存在，减少引用计数，标记脏页，然后返回操作结果
bool BufferPool::UnpinPage(int32_t page_id, bool is_dirty) {
    // 记录取消固定页面操作，便于调试和监控
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在取消固定的页面ID和是否脏页
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Unpinning page ID: " + std::to_string(page_id) + ", is_dirty: " + std::to_string(is_dirty));
    
    // 检查页面是否在缓冲池中
    // Why: 只能取消固定存在于缓冲池中的页面
    // What: 在page_table_哈希表中查找页面ID
    // How: 使用std::unordered_map的find方法，时间复杂度为O(1)
    auto it = page_table_.find(page_id);
    if (it == page_table_.end()) {
        // 页面不在缓冲池中，记录警告并返回false
        // Why: 尝试取消固定不存在的页面是错误操作，需要记录警告
        // What: 创建错误消息，记录警告日志，然后返回false
        // How: 使用SQLCC_LOG_WARN记录警告级别日志，然后返回false
        std::string error_msg = "Page ID " + std::to_string(page_id) + " not found in buffer pool";
        SQLCC_LOG_WARN(error_msg);
        return false;
    }

    // 减少引用计数
    // Why: 页面使用完毕，需要减少引用计数，当计数为0时页面可以被替换
    // What: 在page_refs_哈希表中查找并减少页面ID的引用计数
    // How: 使用std::unordered_map的find方法找到引用计数，然后递减
    auto ref_it = page_refs_.find(page_id);
    if (ref_it == page_refs_.end() || ref_it->second <= 0) {
        // 引用计数错误，记录错误并返回false
        // Why: 引用计数不应该不存在或小于等于0，这表示系统状态不一致
        // What: 创建错误消息，记录错误日志，然后返回false
        // How: 使用SQLCC_LOG_ERROR记录错误级别日志，然后返回false
        std::string error_msg = "Invalid reference count for page ID " + std::to_string(page_id);
        SQLCC_LOG_ERROR(error_msg);
        return false;
    }
    
    // 递减引用计数
    ref_it->second--;
    
    // 记录引用计数变化，便于调试
    // Why: 记录引用计数的变化有助于监控页面使用情况
    // What: 记录页面ID和新的引用计数值
    // How: 使用SQLCC_LOG_DEBUG记录调试级别日志
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " reference count decreased to " + std::to_string(ref_it->second));
    
    // 如果页面被修改，标记为脏页
    // Why: 需要跟踪哪些页面被修改过，以便在替换时写回磁盘
    // What: 如果is_dirty为true，在dirty_pages_哈希表中标记页面为脏页
    // How: 在dirty_pages_哈希表中设置页面ID的值为true
    if (is_dirty) {
        dirty_pages_[page_id] = true;
        // 记录页面被标记为脏页，便于调试
        SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " marked as dirty");
    }

    return true;
}

// 刷新页面到磁盘实现
// Why: 为了确保持久性，被修改过的页面(脏页)需要及时写回磁盘，这在检查点操作或系统关闭时尤为重要
// What: FlushPage方法将指定页面写回磁盘，无论其引用计数是多少，写入成功后清除脏页标记
// How: 检查页面是否存在和是否为脏页，如果是脏页则调用磁盘管理器写回磁盘，然后清除脏页标记
bool BufferPool::FlushPage(int32_t page_id) {
    // 记录刷新页面操作，便于调试和监控
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在刷新的页面ID
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Flushing page ID: " + std::to_string(page_id));
    
    // 检查页面是否在缓冲池中
    // Why: 只能刷新存在于缓冲池中的页面
    // What: 在page_table_哈希表中查找页面ID
    // How: 使用std::unordered_map的find方法，时间复杂度为O(1)
    auto it = page_table_.find(page_id);
    if (it == page_table_.end()) {
        // 页面不在缓冲池中，记录警告并返回false
        // Why: 尝试刷新不存在的页面是错误操作，需要记录警告
        // What: 创建错误消息，记录警告日志，然后返回false
        // How: 使用SQLCC_LOG_WARN记录警告级别日志，然后返回false
        std::string error_msg = "Page ID " + std::to_string(page_id) + " not found in buffer pool";
        SQLCC_LOG_WARN(error_msg);
        return false;
    }

    // 如果不是脏页，无需刷新
    // Why: 非脏页的内容与磁盘上一致，无需写回磁盘，避免不必要的I/O操作
    // What: 在dirty_pages_哈希表中查找页面ID，检查是否为脏页
    // How: 使用std::unordered_map的find方法查找脏页标记
    auto dirty_it = dirty_pages_.find(page_id);
    if (dirty_it == dirty_pages_.end() || !dirty_it->second) {
        // 记录页面不是脏页的信息，便于调试
        SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " is not dirty, no need to flush");
        return true;
    }

    // 将页面写入磁盘
    // Why: 脏页的内容与磁盘不一致，需要写回磁盘以保证数据持久性
    // What: 调用磁盘管理器的WritePage方法将页面写入磁盘
    // How: 传入页面对象引用，由磁盘管理器负责实际的磁盘I/O操作
    if (!disk_manager_->WritePage(*it->second)) {
        // 写入失败，记录错误并返回false
        // Why: 写入磁盘失败可能导致数据丢失，需要记录错误
        // What: 创建错误消息，记录错误日志，然后返回false
        // How: 使用SQLCC_LOG_ERROR记录错误级别日志，然后返回false
        std::string error_msg = "Failed to write page ID " + std::to_string(page_id) + " to disk";
        SQLCC_LOG_ERROR(error_msg);
        return false;
    }

    // 清除脏页标记
    // Why: 页面已写回磁盘，内容与磁盘一致，需要清除脏页标记
    // What: 在dirty_pages_哈希表中设置页面ID的值为false
    // How: 直接设置脏页标记为false
    dirty_it->second = false;
    
    // 记录页面刷新成功，便于调试
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " flushed to disk and marked as clean");
    return true;
}

// 分配新页面实现
// Why: 当数据库需要存储新数据时，需要分配一个新的页面来存储这些数据，这是数据库扩展存储空间的基础操作
// What: NewPage方法分配一个新的页面，并返回其页面ID和页面指针，新页面会被初始化为全零，并加入缓冲池管理
// How: 分配新页面ID，检查缓冲池是否已满，创建新页面对象，初始化各项数据结构，然后返回页面指针
Page* BufferPool::NewPage(int32_t* page_id) {
    // 记录创建新页面操作，便于调试和监控
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在创建新页面
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Creating new page");
    
    // 分配新页面ID
    // Why: 需要一个唯一的页面ID来标识新页面
    // What: 调用磁盘管理器的AllocatePage方法分配一个新的页面ID
    // How: 磁盘管理器负责维护页面ID的分配，确保唯一性
    *page_id = disk_manager_->AllocatePage();
    
    // 记录分配的页面ID，便于调试
    SQLCC_LOG_DEBUG("Allocated new page ID: " + std::to_string(*page_id));
    
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

    // 创建新页面对象
    // Why: 需要创建一个页面对象来存储新数据
    // What: 使用std::make_unique创建一个新的Page对象
    // How: 传入新分配的页面ID作为构造函数参数
    auto page = std::make_unique<Page>(*page_id);
    
    // 添加到页面表
    // Why: 需要将新页面加入缓冲池管理，以便后续查找和使用
    // What: 将页面对象添加到page_table_哈希表中
    // How: 使用页面ID作为键，使用std::move转移页面对象的所有权
    Page* page_ptr = page.get();
    page_table_[*page_id] = std::move(page);
    
    // 初始化引用计数
    // Why: 新页面需要初始化引用计数，以跟踪其使用情况
    // What: 将页面引用计数设置为1，表示已被一个操作引用
    // How: 在page_refs_哈希表中设置页面ID的值为1
    page_refs_[*page_id] = 1;
    
    // 初始化脏页标记
    // Why: 新页面是空的，但分配操作本身应该被持久化，所以标记为脏页
    // What: 将页面标记为脏页，因为分配操作需要被持久化
    // How: 在dirty_pages_哈希表中设置页面ID的值为true
    dirty_pages_[*page_id] = true;  // 新页面标记为脏页
    
    // 添加到LRU链表头部
    // Why: 新分配的页面应该放在LRU链表头部，表示最近被访问
    // What: 将页面ID添加到lru_list_的头部
    // How: 使用push_front方法添加到链表头部，并在lru_map_中记录迭代器
    lru_list_.push_front(*page_id);
    lru_map_[*page_id] = lru_list_.begin();
    
    // 记录新页面创建成功，便于调试
    SQLCC_LOG_DEBUG("New page ID " + std::to_string(*page_id) + " created and loaded into buffer pool");

    return page_ptr;
}

// 删除页面实现
// Why: 当数据库不再需要某个页面时，需要将其从缓冲池中删除，释放内存空间，这是数据库回收存储空间的基础操作
// What: DeletePage方法从缓冲池中删除指定页面，如果页面是脏页，会先将其写回磁盘，然后释放页面占用的所有资源
// How: 检查页面是否存在和引用计数，从所有数据结构中删除页面记录，最后释放页面对象
bool BufferPool::DeletePage(int32_t page_id) {
    // 记录删除页面操作，便于调试和监控
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在删除的页面ID
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Deleting page ID: " + std::to_string(page_id));
    
    // 检查页面是否在缓冲池中
    // Why: 只能删除存在于缓冲池中的页面
    // What: 在page_table_哈希表中查找页面ID
    // How: 使用std::unordered_map的find方法，时间复杂度为O(1)
    auto it = page_table_.find(page_id);
    if (it == page_table_.end()) {
        // 页面不在缓冲池中，记录警告并返回false
        // Why: 尝试删除不存在的页面是错误操作，需要记录警告
        // What: 创建错误消息，记录警告日志，然后返回false
        // How: 使用SQLCC_LOG_WARN记录警告级别日志，然后返回false
        std::string error_msg = "Page ID " + std::to_string(page_id) + " not found in buffer pool";
        SQLCC_LOG_WARN(error_msg);
        return false;
    }

    // 检查页面引用计数
    // Why: 只能删除引用计数为0的页面，表示当前没有操作在使用该页面
    // What: 在page_refs_哈希表中查找页面ID的引用计数
    // How: 使用std::unordered_map的find方法查找引用计数
    auto ref_it = page_refs_.find(page_id);
    if (ref_it != page_refs_.end() && ref_it->second > 0) {
        // 页面正在被使用，无法删除，记录错误并返回false
        // Why: 删除正在使用的页面会导致系统错误，必须阻止
        // What: 创建错误消息，记录错误日志，然后返回false
        // How: 使用SQLCC_LOG_ERROR记录错误级别日志，然后返回false
        std::string error_msg = "Page ID " + std::to_string(page_id) + " is in use, cannot delete";
        SQLCC_LOG_ERROR(error_msg);
        return false;
    }

    // 从缓冲池中移除页面
    // Why: 需要从页面表中删除页面，以释放页面占用的内存
    // What: 从page_table_哈希表中删除页面ID
    // How: 使用std::unordered_map的erase方法删除
    page_table_.erase(it);
    
    // 从脏页表中移除
    // Why: 需要从脏页表中删除页面，以释放脏页标记占用的内存
    // What: 从dirty_pages_哈希表中删除页面ID
    // How: 使用std::unordered_map的erase方法删除
    dirty_pages_.erase(page_id);
    
    // 从引用计数表中移除
    // Why: 需要从引用计数表中删除页面，以释放引用计数占用的内存
    // What: 从page_refs_哈希表中删除页面ID
    // How: 使用std::unordered_map的erase方法删除
    page_refs_.erase(page_id);
    
    // 从LRU链表中移除
    // Why: 需要从LRU链表中删除页面，以维护LRU链表的正确性
    // What: 在lru_map_中查找页面ID的迭代器，然后从lru_list_中删除
    // How: 使用lru_map_快速定位页面在链表中的位置，然后使用erase方法删除
    auto lru_it = lru_map_.find(page_id);
    if (lru_it != lru_map_.end()) {
        lru_list_.erase(lru_it->second);
        lru_map_.erase(lru_it);
    }
    
    // 记录页面删除成功，便于调试
    SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " deleted from buffer pool");

    return true;
}

// 刷新所有页面到磁盘实现
// Why: 在系统关闭或检查点操作时，需要将所有脏页写回磁盘，以保证数据持久性和一致性
// What: FlushAllPages方法遍历缓冲池中的所有页面，将脏页写回磁盘，然后清除所有脏页标记
// How: 遍历page_table_哈希表，对每个脏页调用磁盘管理器的WritePage方法写回磁盘
void BufferPool::FlushAllPages() {
    // 记录刷新所有页面操作，便于调试和监控
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在刷新所有页面
    // How: 使用SQLCC_LOG_INFO宏记录信息级别日志
    SQLCC_LOG_INFO("Flushing all pages in buffer pool");
    
    // 遍历所有页面，刷新脏页到磁盘
    // Why: 需要将所有脏页写回磁盘，以保证数据持久性
    // What: 遍历page_table_哈希表，对每个脏页调用WritePage方法
    // How: 使用范围for循环遍历page_table_，对每个脏页调用WritePage
    for (auto& pair : page_table_) {
        int32_t page_id = pair.first;
        // 检查是否为脏页
        // Why: 只需要刷新脏页，非脏页的内容与磁盘一致
        // What: 在dirty_pages_哈希表中查找页面ID，检查是否为脏页
        // How: 使用std::unordered_map的find方法查找脏页标记
        auto dirty_it = dirty_pages_.find(page_id);
        if (dirty_it != dirty_pages_.end() && dirty_it->second) {
            // 是脏页，需要刷新到磁盘
            // Why: 脏页的内容与磁盘不一致，需要写回磁盘以保证数据持久性
            // What: 调用磁盘管理器的WritePage方法将页面写入磁盘
            // How: 传入页面对象引用，由磁盘管理器负责实际的磁盘I/O操作
            if (!disk_manager_->WritePage(*pair.second)) {
                // 写入失败，记录错误但继续处理其他页面
                // Why: 写入磁盘失败可能导致数据丢失，但不应中断整个刷新过程
                // What: 创建错误消息，记录错误日志，然后继续处理下一个页面
                // How: 使用SQLCC_LOG_ERROR记录错误级别日志，然后使用continue继续循环
                std::string error_msg = "Failed to write page ID " + std::to_string(page_id) + " to disk during flush all";
                SQLCC_LOG_ERROR(error_msg);
                // 继续刷新其他页面，不中断整个过程
                continue;
            }
            // 清除脏页标记
            // Why: 页面已写回磁盘，内容与磁盘一致，需要清除脏页标记
            // What: 在dirty_pages_哈希表中设置页面ID的值为false
            // How: 直接设置脏页标记为false
            dirty_it->second = false;
            // 记录页面刷新成功，便于调试
            SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " flushed to disk and marked as clean");
        }
    }
    
    // 记录所有页面刷新成功，便于调试
    SQLCC_LOG_INFO("All pages flushed to disk");
}

// 替换页面实现
// Why: 当缓冲池已满且需要加载新页面时，必须选择一个现有页面进行替换，这是LRU替换算法的核心实现
// What: ReplacePage方法使用LRU算法选择一个引用计数为0的页面进行替换，如果是脏页则先写回磁盘，然后从缓冲池中删除
// How: 从LRU链表尾部开始查找，找到第一个引用计数为0的页面，如果是脏页则先写回磁盘，然后删除该页面
int32_t BufferPool::ReplacePage() {
    // 记录替换页面操作，便于调试和监控
    // Why: 日志记录有助于系统运行状态的监控和问题排查
    // What: 记录正在执行页面替换操作
    // How: 使用SQLCC_LOG_DEBUG宏记录调试级别日志
    SQLCC_LOG_DEBUG("Replacing page using LRU algorithm");
    
    // 使用LRU算法查找最久未使用的页面
    // Why: LRU算法规定应该替换最久未使用的页面
    // What: 从LRU链表尾部开始查找，找到第一个引用计数为0的页面
    // How: 使用while循环遍历LRU链表，检查每个页面的引用计数
    while (!lru_list_.empty()) {
        // 获取LRU链表尾部的页面ID（最久未使用）
        // Why: LRU链表尾部存储的是最久未使用的页面
        // What: 使用back方法获取链表尾部的页面ID
        // How: 直接调用std::list的back方法
        int32_t page_id = lru_list_.back();
        
        // 检查页面引用计数
        // Why: 只能替换引用计数为0的页面，表示当前没有操作在使用该页面
        // What: 在page_refs_哈希表中查找页面ID的引用计数
        // How: 使用std::unordered_map的find方法查找引用计数
        auto ref_it = page_refs_.find(page_id);
        if (ref_it != page_refs_.end() && ref_it->second > 0) {
            // 页面正在被使用，不能替换，移到头部
            // Why: 引用计数大于0的页面不能被替换，需要将其移到LRU链表头部
            // What: 调用MoveToHead方法将页面移到LRU链表头部
            // How: 调用MoveToHead方法，传入页面ID作为参数
            SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " is in use, moving to head of LRU list");
            MoveToHead(page_id);
            continue;
        }
        
        // 检查是否为脏页，如果是则需要刷新到磁盘
        // Why: 替换脏页前需要先写回磁盘，以保证数据持久性
        // What: 在dirty_pages_哈希表中查找页面ID，检查是否为脏页
        // How: 使用std::unordered_map的find方法查找脏页标记
        auto dirty_it = dirty_pages_.find(page_id);
        if (dirty_it != dirty_pages_.end() && dirty_it->second) {
            // 获取页面对象
            // Why: 需要页面对象才能调用磁盘管理器的WritePage方法
            // What: 在page_table_哈希表中查找页面ID对应的页面对象
            // How: 使用std::unordered_map的find方法查找页面对象
            auto page_it = page_table_.find(page_id);
            if (page_it != page_table_.end()) {
                // 刷新脏页到磁盘
                // Why: 脏页的内容与磁盘不一致，需要写回磁盘以保证数据持久性
                // What: 调用磁盘管理器的WritePage方法将页面写入磁盘
                // How: 传入页面对象引用，由磁盘管理器负责实际的磁盘I/O操作
                SQLCC_LOG_DEBUG("Flushing dirty page ID " + std::to_string(page_id) + " to disk before replacement");
                if (!disk_manager_->WritePage(*page_it->second)) {
                    // 写入失败，记录错误并继续查找下一个页面
                    // Why: 写入磁盘失败可能导致数据丢失，不应替换此页面
                    // What: 创建错误消息，记录错误日志，然后继续循环
                    // How: 使用SQLCC_LOG_ERROR记录错误级别日志，然后使用continue继续循环
                    std::string error_msg = "Failed to write dirty page ID " + std::to_string(page_id) + " to disk";
                    SQLCC_LOG_ERROR(error_msg);
                    // 无法刷新脏页，不能替换此页面
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

std::vector<Page*> BufferPool::BatchFetchPages(const std::vector<int32_t>& page_ids) {
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
            Page* page = page_it->second.get();
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
        Page* page_ptr = new_pages[i].get();
        page_table_[page_id] = std::move(new_pages[i]);
        page_refs_[page_id] = 1;
        dirty_pages_[page_id] = false;
        
        // 添加到LRU链表头部
        lru_list_.push_front(page_id);
        lru_map_[page_id] = lru_list_.begin();
        
        // 更新结果数组中的对应位置
        auto it = std::find(page_ids.begin(), page_ids.end(), page_id);
        if (it != page_ids.end()) {
            size_t index = std::distance(page_ids.begin(), it);
            result[index] = page_ptr;
        }
    }
    
    return result;
}

void BufferPool::PrefetchPage(int32_t page_id) {
    // 检查页面ID是否有效
    if (page_id < 0) {
        SQLCC_LOG_ERROR("Invalid page ID for prefetch: " + std::to_string(page_id));
        return;
    }
    
    // 检查页面是否已在缓冲池中
    if (page_table_.find(page_id) != page_table_.end()) {
        // 页面已在缓冲池中，无需预取
        return;
    }
    
    // 检查页面是否已在预取队列中
    auto it = std::find(prefetch_queue_.begin(), prefetch_queue_.end(), page_id);
    if (it != prefetch_queue_.end()) {
        // 页面已在预取队列中，无需重复预取
        return;
    }
    
    // 使用磁盘管理器的预取功能
    disk_manager_->PrefetchPage(page_id);
    
    // 将页面ID添加到预取队列
    prefetch_queue_.push_back(page_id);
    
    // 限制预取队列大小
    if (prefetch_queue_.size() > 64) {
        prefetch_queue_.pop_front();
    }
}

void BufferPool::BatchPrefetchPages(const std::vector<int32_t>& page_ids) {
    // 筛选需要预取的页面（不在缓冲池中且不在预取队列中）
    std::vector<int32_t> pages_to_prefetch;
    pages_to_prefetch.reserve(page_ids.size());
    
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
    
    // 如果没有需要预取的页面，直接返回
    if (pages_to_prefetch.empty()) {
        return;
    }
    
    // 按页面ID排序，优化磁盘访问模式
    std::sort(pages_to_prefetch.begin(), pages_to_prefetch.end());
    
    // 使用磁盘管理器的批量预取功能
    disk_manager_->BatchPrefetchPages(pages_to_prefetch);
    
    // 将页面ID添加到预取队列
    for (int32_t page_id : pages_to_prefetch) {
        prefetch_queue_.push_back(page_id);
    }
    
    // 限制预取队列大小
    while (prefetch_queue_.size() > 64) {
        prefetch_queue_.pop_front();
    }
}

}  // namespace sqlcc