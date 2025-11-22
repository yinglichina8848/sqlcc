# BufferPool类详细设计

## 概述

BufferPool是存储引擎的缓冲池管理组件，负责内存中的页面缓存、替换策略（LRU）和并发控制。它通过减少磁盘I/O操作来提高数据库性能。

## 类定义

```cpp
class BufferPool {
public:
    explicit BufferPool(DiskManager* disk_manager, size_t pool_size, ConfigManager& config_manager);
    ~BufferPool();
    
    BufferPool(const BufferPool&) = delete;
    BufferPool& operator=(const BufferPool&) = delete;
    
    Page* FetchPage(int32_t page_id);
    std::vector<Page*> BatchFetchPages(const std::vector<int32_t>& page_ids);
    Page* NewPage(int32_t* page_id);
    bool UnpinPage(int32_t page_id, bool is_dirty);
    bool FlushPage(int32_t page_id);
    void FlushAllPages();
    bool DeletePage(int32_t page_id);
    bool PrefetchPage(int32_t page_id);
    bool BatchPrefetchPages(const std::vector<int32_t>& page_ids);
    std::unordered_map<std::string, double> GetStats() const;
    size_t GetPoolSize() const;
    size_t GetUsedPages() const;
    bool IsPageInBuffer(int32_t page_id) const;

private:
    int32_t FindVictimPage();
    bool ReplacePage(int32_t victim_page_id, int32_t new_page_id);
    void UpdateLRUList(int32_t page_id);
    void MoveToHead(int32_t page_id);
    void RemoveFromLRUList(int32_t page_id);
    int32_t ReplacePageInternal();
    int32_t ReplacePage();
    void OnConfigChange(const std::string& key, const ConfigValue& value);
    void AdjustBufferPoolSize(size_t new_pool_size);
    void AdjustBufferPoolSizeNoLock(size_t new_pool_size);

private:
    DiskManager* disk_manager_;
    ConfigManager& config_manager_;
    size_t pool_size_;
    std::unordered_map<int32_t, Page*> page_table_;
    std::list<int32_t> lru_list_;
    std::unordered_map<int32_t, std::list<int32_t>::iterator> lru_map_;
    mutable std::timed_mutex latch_;
    struct Stats {
        size_t total_accesses = 0;
        size_t total_hits = 0;
        size_t total_misses = 0;
        size_t total_evictions = 0;
        size_t total_prefetches = 0;
        size_t prefetch_hits = 0;
    } stats_;
    std::unordered_map<int32_t, int> page_refs_;
    std::unordered_map<int32_t, bool> dirty_pages_;
    std::deque<int32_t> prefetch_queue_;
    std::unordered_map<int32_t, int> access_stats_;
    std::vector<char*> batch_buffer_;
    bool simulate_flush_failure_;
    size_t read_lock_timeout_ms_;
    size_t write_lock_timeout_ms_;
    size_t lock_timeout_ms_;
};
```

## 构造函数

### BufferPool(DiskManager* disk_manager, size_t pool_size, ConfigManager& config_manager)

构造函数负责初始化缓冲池：

1. 设置磁盘管理器指针、缓冲池大小和配置管理器引用
2. 初始化页面表和LRU列表
3. 创建互斥锁
4. 条件注册配置回调

## 析构函数

### ~BufferPool()

析构函数负责清理资源：

1. 调用FlushAllPages方法确保数据持久化
2. 释放所有页面对象

## 核心方法

### Page* FetchPage(int32_t page_id)

获取页面，如果页面不在缓冲池中则从磁盘加载：

1. 在页面表中查找页面
2. 如果找到则更新LRU列表并返回
3. 如果未找到则从磁盘加载

### std::vector<Page*> BatchFetchPages(const std::vector<int32_t>& page_ids)

批量获取页面，优化多个页面的加载性能：

1. 对每个页面调用FetchPage方法
2. 或者实现更高效的批量加载策略

### Page* NewPage(int32_t* page_id)

创建新页面：

1. 从空闲页面列表中获取页面
2. 或者替换一个现有页面
3. 初始化页面数据

### bool UnpinPage(int32_t page_id, bool is_dirty)

取消固定页面，减少页面的固定计数：

1. 减少页面的引用计数
2. 如果引用计数为0且页面被修改，则标记为脏页

### bool FlushPage(int32_t page_id)

刷新页面到磁盘：

1. 调用磁盘管理器的WritePage方法
2. 将页面数据写入磁盘

### void FlushAllPages()

刷新所有页面到磁盘：

1. 遍历所有页面
2. 对脏页调用FlushPage方法

### bool DeletePage(int32_t page_id)

删除页面：

1. 从页面表中移除页面
2. 释放页面对象

### bool PrefetchPage(int32_t page_id)

预取页面到缓冲池：

1. 类似FetchPage，但不增加页面的固定计数

### bool BatchPrefetchPages(const std::vector<int32_t>& page_ids)

批量预取页面到缓冲池：

1. 对每个页面调用PrefetchPage方法
2. 或者实现更高效的批量预取策略

### std::unordered_map<std::string, double> GetStats() const

获取缓冲池使用统计信息：

1. 收集并返回各种统计指标

### size_t GetPoolSize() const

获取缓冲池大小：

1. 直接返回pool_size_成员变量

### size_t GetUsedPages() const

获取已使用页面数：

1. 返回页面表的大小

### bool IsPageInBuffer(int32_t page_id) const

检查页面是否在缓冲池中：

1. 检查页面表是否包含该页面ID

## 私有方法

### int32_t FindVictimPage()

查找可替换的页面：

1. 根据LRU算法找到可替换的页面
2. 从LRU列表的尾部开始查找，找到引用计数为0的页面

### bool ReplacePage(int32_t victim_page_id, int32_t new_page_id)

替换页面：

1. 将旧页面写回磁盘（如果脏）
2. 更新页面数据
3. 重新插入LRU列表

### void UpdateLRUList(int32_t page_id)

更新LRU列表：

1. 将页面移动到LRU列表的头部
2. 表示最近被访问

### void MoveToHead(int32_t page_id)

移动页面到LRU链表头部：

1. 从LRU链表中移除页面
2. 然后将其添加到头部
3. 更新LRU映射中的迭代器

### void RemoveFromLRUList(int32_t page_id)

从LRU列表中移除页面：

1. 在LRU列表中查找页面并移除

### int32_t ReplacePageInternal()

替换页面（无锁版本）：

1. 在持锁状态下替换页面
2. 直接操作LRU列表和页面表，不重新获取锁

### int32_t ReplacePage()

替换页面：

1. 使用LRU算法选择一个引用计数为0的页面进行替换
2. 从LRU链表尾部开始查找

### void OnConfigChange(const std::string& key, const ConfigValue& value)

配置变更回调处理：

1. 根据变更的配置项调整相应的缓冲池参数

### void AdjustBufferPoolSize(size_t new_pool_size)

调整缓冲池大小：

1. 在持锁状态下移除多余的页面或标记容量变更

### void AdjustBufferPoolSizeNoLock(size_t new_pool_size)

安全调整缓冲池大小（无锁版本）：

1. 通过发送消息到队列的方式触发异步调整
2. 不直接获取锁

## 成员变量

### DiskManager* disk_manager_

磁盘管理器指针，负责磁盘I/O操作。

### ConfigManager& config_manager_

配置管理器引用，用于获取配置参数。

### size_t pool_size_

缓冲池大小，表示可以缓存的页面数量。

### std::unordered_map<int32_t, Page*> page_table_

页面表，存储页面ID到页面对象的映射。

### std::list<int32_t> lru_list_

LRU列表，存储页面的访问顺序。

### std::unordered_map<int32_t, std::list<int32_t>::iterator> lru_map_

LRU列表迭代器映射，快速定位页面在LRU列表中的位置。

### mutable std::timed_mutex latch_

互斥锁，保护缓冲池的并发访问，并支持超时机制避免死锁。

### struct Stats stats_

统计信息，记录缓冲池的使用情况。

### std::unordered_map<int32_t, int> page_refs_

页面引用计数表，存储页面ID到引用计数的映射。

### std::unordered_map<int32_t, bool> dirty_pages_

脏页标记表，存储页面ID到脏页标记的映射。

### std::deque<int32_t> prefetch_queue_

预取队列，存储待预取的页面ID。

### std::unordered_map<int32_t, int> access_stats_

页面访问统计，用于预测性预取。

### std::vector<char*> batch_buffer_

批量操作缓冲区，用于批量读写操作。

### bool simulate_flush_failure_

模拟刷新失败标志，用于测试。

### size_t read_lock_timeout_ms_, write_lock_timeout_ms_, lock_timeout_ms_

锁超时时间（毫秒），限制锁获取的等待时间，避免死锁导致的长时间阻塞。