# BufferPoolSharded 设计文档

## 1. 概述

BufferPoolSharded 是 SQLCC 数据库系统中存储引擎的核心组件，实现了分片式缓冲池管理。它采用 RocksDB 风格的分片设计，通过将缓冲池分为多个独立分片，显著提高了高并发场景下的性能。每个分片拥有独立的锁和 LRU 队列，减少了锁竞争，提高了内存局部性和系统扩展性。

## 2. 核心功能

### 2.1 主要功能

- **页面管理**：管理内存中的数据库页面，包括获取、创建、刷新和删除页面
- **高并发支持**：通过分片设计支持高并发读写操作
- **LRU 替换策略**：实现页面替换算法，管理内存使用
- **页面固定**：通过引用计数管理页面的使用状态
- **脏页处理**：跟踪和刷新脏页到磁盘
- **统计信息**：收集性能指标和统计信息

### 2.2 设计优势

- **高性能**：分片设计减少锁竞争，提高并发性能
- **内存安全**：使用智能指针管理内存，避免内存泄漏
- **可扩展性**：支持动态调整缓冲池大小和分片数量
- **低开销**：管理结构开销小，相对于缓冲池大小可忽略
- **高效替换**：每个分片独立的 LRU 策略，提高缓存命中率

## 3. 类定义

```cpp
class BufferPoolSharded {
public:
    BufferPoolSharded(std::shared_ptr<DiskManager> disk_manager, ConfigManager& config_manager, 
                     size_t pool_size, size_t num_shards = 16);
    ~BufferPoolSharded();
    std::unique_ptr<Page> FetchPage(int32_t page_id, bool exclusive = false);
    std::unique_ptr<Page> NewPage(int32_t* page_id);
    bool FlushPage(int32_t page_id);
    void FlushAllPages();
    bool DeletePage(int32_t page_id);
    bool UnpinPage(int32_t page_id, bool is_dirty);
    std::unordered_map<std::string, double> GetStats() const;
    size_t GetPoolSize() const;
    size_t GetCurrentPageCount() const;

private:
    struct PageWrapper {
        std::unique_ptr<Page> page;
        int ref_count;
        bool is_dirty;
        std::list<int32_t>::iterator lru_iter;
        bool is_in_lru;
    };

    struct Shard {
        std::mutex mutex;
        std::unordered_map<int32_t, std::shared_ptr<PageWrapper>> page_table;
        std::list<int32_t> lru_list;
        std::unordered_map<int32_t, std::list<int32_t>::iterator> lru_map;
        size_t current_size;
        size_t max_size;
    };

    size_t GetShardIndex(int32_t page_id) const;
    int32_t ReplacePage(Shard& shard);
    void MoveToHead(Shard& shard, int32_t page_id);
    void RemoveFromLRU(Shard& shard, int32_t page_id);

    std::shared_ptr<DiskManager> disk_manager_;
    ConfigManager& config_manager_;
    size_t pool_size_;
    size_t num_shards_;
    std::vector<std::unique_ptr<Shard>> shards_;

    struct Stats {
        std::atomic<size_t> total_accesses;
        std::atomic<size_t> total_hits;
        std::atomic<size_t> total_misses;
        std::atomic<size_t> total_evictions;
    } stats_;

    std::unordered_set<int32_t> allocated_pages_;
    mutable std::mutex allocated_pages_mutex_;
    std::atomic<int32_t> next_page_id_;
};
```

## 4. 核心组件

### 4.1 构造函数

```cpp
BufferPoolSharded(std::shared_ptr<DiskManager> disk_manager, ConfigManager& config_manager, 
                 size_t pool_size, size_t num_shards = 16);
```

- **功能**：初始化分片缓冲池
- **参数**：
  - `disk_manager` - 磁盘管理器智能指针
  - `config_manager` - 配置管理器实例
  - `pool_size` - 缓冲池大小
  - `num_shards` - shard数量（必须是2的幂）
- **设计意图**：创建指定数量的分片，并初始化每个分片的大小

### 4.2 页面获取与管理

#### FetchPage

```cpp
std::unique_ptr<Page> FetchPage(int32_t page_id, bool exclusive = false);
```

- **功能**：根据页面ID从缓冲池获取页面
- **参数**：
  - `page_id` - 页面ID
  - `exclusive` - 是否需要独占锁
- **返回值**：页面智能指针，失败时返回nullptr
- **设计意图**：支持并发访问，实现页面的高效获取

#### NewPage

```cpp
std::unique_ptr<Page> NewPage(int32_t* page_id);
```

- **功能**：创建新页面
- **参数**：
  - `page_id` - 输出参数，页面ID
- **返回值**：页面智能指针，失败时返回nullptr
- **设计意图**：分配新的页面ID并创建页面

#### FlushPage

```cpp
bool FlushPage(int32_t page_id);
```

- **功能**：刷新页面到磁盘
- **参数**：
  - `page_id` - 页面ID
- **返回值**：是否刷新成功
- **设计意图**：将脏页写入磁盘，确保数据持久化

#### FlushAllPages

```cpp
void FlushAllPages();
```

- **功能**：刷新所有页面到磁盘
- **设计意图**：将所有脏页写入磁盘，确保数据一致性

#### DeletePage

```cpp
bool DeletePage(int32_t page_id);
```

- **功能**：删除页面
- **参数**：
  - `page_id` - 页面ID
- **返回值**：是否删除成功
- **设计意图**：从缓冲池和磁盘中删除页面

#### UnpinPage

```cpp
bool UnpinPage(int32_t page_id, bool is_dirty);
```

- **功能**：减少页面的固定计数
- **参数**：
  - `page_id` - 页面ID
  - `is_dirty` - 是否为脏页
- **返回值**：是否解除成功
- **设计意图**：管理页面的使用状态，当引用计数为0时将页面加入LRU替换候选列表

### 4.3 统计信息

#### GetStats

```cpp
std::unordered_map<std::string, double> GetStats() const;
```

- **功能**：获取缓冲池统计信息
- **返回值**：统计信息哈希表
- **设计意图**：收集性能指标和统计信息，用于监控和优化

### 4.4 内部数据结构

#### PageWrapper

```cpp
struct PageWrapper {
    std::unique_ptr<Page> page;
    int ref_count;
    bool is_dirty;
    std::list<int32_t>::iterator lru_iter;
    bool is_in_lru;
};
```

- **功能**：页面对象的包装类
- **成员**：
  - `page` - 页面对象智能指针
  - `ref_count` - 引用计数
  - `is_dirty` - 脏页标记
  - `lru_iter` - LRU链表迭代器
  - `is_in_lru` - 是否在LRU链表中

#### Shard

```cpp
struct Shard {
    std::mutex mutex;
    std::unordered_map<int32_t, std::shared_ptr<PageWrapper>> page_table;
    std::list<int32_t> lru_list;
    std::unordered_map<int32_t, std::list<int32_t>::iterator> lru_map;
    size_t current_size;
    size_t max_size;
};
```

- **功能**：单个分片的实现
- **成员**：
  - `mutex` - 分片独立的互斥锁
  - `page_table` - 页面表
  - `lru_list` - LRU列表
  - `lru_map` - LRU映射
  - `current_size` - 当前页面数量
  - `max_size` - 最大页面数量

## 5. 实现细节

### 5.1 分片定位算法

```cpp
inline size_t GetShardIndex(int32_t page_id) const {
    return (static_cast<size_t>(page_id) & (num_shards_ - 1));
}
```

- **功能**：根据页面ID获取对应的分片索引
- **设计意图**：使用位运算实现快速取模，仅当分片数量为2的幂时有效

### 5.2 页面替换算法

```cpp
int32_t ReplacePage(Shard& shard) {
    // 1. 从LRU链表尾部查找可替换页面
    // 2. 检查页面是否可替换（引用计数为0）
    // 3. 如果是脏页，先刷新到磁盘
    // 4. 从页面表和LRU中移除页面
    // 5. 返回被替换的页面ID
}
```

- **功能**：在指定分片内替换页面
- **设计意图**：实现LRU替换策略，管理内存使用

### 5.3 LRU管理

```cpp
void MoveToHead(Shard& shard, int32_t page_id);
void RemoveFromLRU(Shard& shard, int32_t page_id);
```

- **功能**：管理LRU链表
- **设计意图**：维护LRU链表，确保最近使用的页面在链表头部

### 5.4 并发优化

- **分片锁**：每个分片有独立的互斥锁，减少锁竞争
- **读/写锁**：根据操作类型使用不同锁级别
- **原子操作**：使用原子类型收集统计信息，避免锁开销
- **智能指针**：使用智能指针管理内存，确保线程安全

## 6. 性能优化

### 6.1 分片设计

- **减少锁竞争**：每个分片独立锁，锁竞争减少到1/num_shards
- **提高局部性**：相邻页面倾向于同一分片，提高缓存命中率
- **可扩展性**：支持更高并发负载

### 6.2 内存优化

- **智能指针**：使用 `std::unique_ptr` 和 `std::shared_ptr` 管理内存
- **减少复制**：通过引用和指针传递数据
- **预分配**：预分配分片和页面表内存

### 6.3 操作优化

- **位运算**：使用位运算实现快速取模
- **LRU映射**：使用哈希表快速定位LRU链表中的页面
- **原子统计**：使用原子操作收集统计信息，避免锁开销

## 7. 扩展点

### 7.1 替换策略扩展

可以轻松添加新的页面替换策略：

```cpp
// 实现新的替换策略
int32_t ReplacePageLRU(Shard& shard);
int32_t ReplacePageLFU(Shard& shard);
int32_t ReplacePageMRU(Shard& shard);

// 在配置中选择替换策略
if (config_manager_.GetString("buffer_pool.replacement_policy") == "lru") {
    // 使用LRU策略
}
```

### 7.2 分片数量动态调整

支持动态调整分片数量：

```cpp
void ResizeShards(size_t new_num_shards) {
    // 1. 创建新的分片数组
    // 2. 将现有页面重新分配到新分片
    // 3. 更新分片数量
}
```

### 7.3 统计信息扩展

可以轻松扩展统计信息收集：

```cpp
struct Stats {
    std::atomic<size_t> total_accesses{0};
    std::atomic<size_t> total_hits{0};
    std::atomic<size_t> total_misses{0};
    std::atomic<size_t> total_evictions{0};
    std::atomic<size_t> total_dirty_writes{0}; // 新统计项
    std::atomic<double> avg_latency{0.0};      // 新统计项
};
```

## 8. 错误处理

BufferPoolSharded 采用以下错误处理策略：

- **异常抛出**：使用 `BufferPoolException` 处理错误情况
- **错误码返回**：部分操作返回布尔值表示成功或失败
- **空指针检查**：对输入参数进行空指针检查
- **边界检查**：对页面ID和索引进行边界检查

## 9. 测试支持

### 9.1 单元测试

```cpp
TEST(BufferPoolShardedTest, FetchAndUnpinPage) {
    // 创建磁盘管理器和配置管理器
    auto disk_manager = std::make_shared<DiskManager>();
    ConfigManager config_manager;
    
    // 创建缓冲池
    BufferPoolSharded buffer_pool(disk_manager, config_manager, 10, 4);
    
    // 获取页面
    auto page = buffer_pool.FetchPage(1);
    ASSERT_NE(page, nullptr);
    
    // 解除固定
    bool success = buffer_pool.UnpinPage(1, false);
    ASSERT_TRUE(success);
}
```

### 9.2 性能测试

```cpp
TEST(BufferPoolShardedTest, ConcurrentPerformance) {
    // 测试高并发场景下的性能
    // ...
}
```

## 10. 使用示例

### 10.1 基本使用

```cpp
// 创建磁盘管理器和配置管理器
auto disk_manager = std::make_shared<DiskManager>();
ConfigManager config_manager;

// 创建缓冲池
BufferPoolSharded buffer_pool(disk_manager, config_manager, 1000, 16);

// 获取页面
auto page = buffer_pool.FetchPage(1);
if (page != nullptr) {
    // 使用页面
    // ...
    
    // 标记为脏页并解除固定
    buffer_pool.UnpinPage(1, true);
}

// 刷新所有页面
buffer_pool.FlushAllPages();
```

### 10.2 并发使用

```cpp
// 多线程并发访问缓冲池
std::vector<std::thread> threads;
for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&buffer_pool, i]() {
        for (int j = 0; j < 100; ++j) {
            auto page = buffer_pool.FetchPage(j);
            if (page != nullptr) {
                // 使用页面
                buffer_pool.UnpinPage(j, false);
            }
        }
    });
}

// 等待所有线程完成
for (auto& thread : threads) {
    thread.join();
}

// 获取统计信息
auto stats = buffer_pool.GetStats();
std::cout << "Hit rate: " << (stats["hit_rate"] * 100) << "%" << std::endl;
```

## 11. 总结

BufferPoolSharded 是 SQLCC 数据库系统中高性能存储引擎的核心组件，通过分片设计显著提高了高并发场景下的性能。它实现了页面管理、高并发支持、LRU 替换策略和统计信息收集等功能，具有高性能、内存安全、可扩展性和低开销等优点。其设计理念和实现技术为构建高性能数据库系统提供了重要参考。