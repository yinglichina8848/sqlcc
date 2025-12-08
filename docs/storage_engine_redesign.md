# SQLCC 存储引擎重构设计文档

## 概述

本文档描述了SQLCC存储引擎的重构设计，重点在于BufferPool组件的优化，包括可插拔替换策略、高级并发控制机制和性能监控系统。

## 设计目标

1. **可插拔替换策略**：支持多种页面替换策略，如LRU、2Q等，可根据工作负载动态切换
2. **高级并发控制**：实现死锁预防、锁层次结构和高效的并发读写机制
3. **性能监控**：提供全面的性能指标收集和分析能力
4. **可扩展性**：设计灵活的架构，便于未来添加新功能

## 架构设计

### 整体架构

```
┌─────────────────────────────────────────────────────────────┐
│                    Storage Engine                           │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │ BufferPool  │  │ DiskManager │  │ TransactionManager  │  │
│  │             │  │             │  │                     │  │
│  │ ┌─────────┐ │  │             │  │                     │  │
│  │ │Replace  │ │  │             │  │                     │  │
│  │ │Strategy │ │  │             │  │                     │  │
│  │ └─────────┘ │  │             │  │                     │  │
│  │ ┌─────────┐ │  │             │  │                     │  │
│  │ │Concurrency│ │  │             │  │                     │  │
│  │ │Control  │ │  │             │  │                     │  │
│  │ └─────────┘ │  │             │  │                     │  │
│  │ ┌─────────┐ │  │             │  │                     │  │
│  │ │Prefetcher│ │  │             │  │                     │  │
│  │ └─────────┘ │  │             │  │                     │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│                    Performance Monitor                      │
└─────────────────────────────────────────────────────────────┘
```

### 核心组件

#### 1. BufferPool

BufferPool是存储引擎的核心组件，负责管理内存中的页面缓存。重构后的BufferPool具有以下特点：

- **可插拔替换策略**：通过AbstractReplaceStrategy接口支持多种替换策略
- **细粒度并发控制**：使用per-page shared_mutex实现高效的并发读写
- **智能预取**：基于访问模式的预测性页面加载
- **全面统计**：收集详细的性能指标

#### 2. 替换策略系统

替换策略系统采用策略模式设计，支持以下策略：

- **LRU (Least Recently Used)**：经典的最近最少使用策略
- **2Q (Two Queues)**：结合LRU和FIFO的混合策略，更适合扫描型工作负载

未来可轻松扩展其他策略，如ARC、LFU等。

#### 3. 并发控制系统

并发控制系统提供以下功能：

- **死锁检测与预防**：使用等待图检测死锁，并选择牺牲者
- **锁层次结构**：通过锁的层次化组织避免死锁
- **超时重试机制**：为锁获取设置超时，避免无限等待
- **细粒度锁**：每个页面使用独立的读写锁

#### 4. 预取系统

预取系统基于访问模式预测未来可能需要的页面：

- **顺序访问检测**：识别顺序访问模式
- **历史访问分析**：基于历史访问预测未来访问
- **自适应预取**：根据命中率调整预取策略

#### 5. 性能监控系统

性能监控系统提供全面的指标收集和分析：

- **多种指标类型**：计数器、仪表盘、直方图、计时器
- **多格式导出**：支持JSON和Prometheus格式
- **组件级指标**：为每个组件提供专门的指标集合

## 详细设计

### AbstractReplaceStrategy接口

```cpp
class AbstractReplaceStrategy {
public:
    virtual ~AbstractReplaceStrategy() = default;
    
    // 通知页面访问
    virtual void NotifyPageAccess(int32_t page_id, bool is_dirty) = 0;
    
    // 通知页面释放
    virtual void NotifyPageRelease(int32_t page_id) = 0;
    
    // 选择替换页面
    virtual int32_t SelectVictim() = 0;
    
    // 更新容量
    virtual void UpdateCapacity(size_t capacity) = 0;
    
    // 获取统计信息
    virtual ReplaceStrategyStats GetStats() const = 0;
    
    // 重置统计信息
    virtual void Reset() = 0;
};
```

### LRU策略实现

LRU策略使用双向链表和哈希表实现O(1)的页面访问和替换：

```cpp
class LRUStrategy : public AbstractReplaceStrategy {
private:
    struct ListNode {
        int32_t page_id;
        bool is_dirty;
        std::shared_ptr<ListNode> prev;
        std::shared_ptr<ListNode> next;
        
        ListNode(int32_t id, bool dirty) 
            : page_id(id), is_dirty(dirty), prev(nullptr), next(nullptr) {}
    };
    
    std::unordered_map<int32_t, std::shared_ptr<ListNode>> page_map_;
    std::shared_ptr<ListNode> head_;
    std::shared_ptr<ListNode> tail_;
    size_t capacity_;
    
    // 统计信息
    std::atomic<uint64_t> accesses_;
    std::atomic<uint64_t> evictions_;
    
    // 内部方法
    void MoveToFront(std::shared_ptr<ListNode> node);
    void RemoveNode(std::shared_ptr<ListNode> node);
    std::shared_ptr<ListNode> AddToFront(int32_t page_id, bool is_dirty);
    void RemoveTail();
};
```

### 2Q策略实现

2Q策略使用三个队列管理页面：

```cpp
class TwoQStrategy : public AbstractReplaceStrategy {
private:
    // A1in队列：新页面，FIFO
    std::queue<int32_t> a1in_;
    std::unordered_set<int32_t> a1in_set_;
    
    // Am队列：多次访问的页面，LRU
    std::list<int32_t> am_;
    std::unordered_map<int32_t, std::list<int32_t>::iterator> am_map_;
    
    // A1out队列：曾经在新队列但现在被替换的页面
    std::queue<int32_t> a1out_;
    std::unordered_set<int32_t> a1out_set_;
    
    // 页面访问计数
    std::unordered_map<int32_t, uint32_t> access_counts_;
    
    // 配置参数
    size_t capacity_;
    size_t a1in_capacity_;
    size_t a1out_capacity_;
    size_t am_capacity_;
    
    // 统计信息
    std::atomic<uint64_t> accesses_;
    std::atomic<uint64_t> evictions_;
    std::atomic<uint64_t> a1in_evictions_;
    std::atomic<uint64_t> am_evictions_;
};
```

### 并发控制系统

#### 死锁检测器

```cpp
class DeadlockDetector {
public:
    struct DeadlockResult {
        bool is_deadlock;
        std::vector<int32_t> victim_transactions;
    };
    
    // 检测死锁
    DeadlockResult DetectDeadlock(const LockGraph& lock_graph);
    
    // 选择牺牲者
    int32_t SelectVictim(const std::vector<int32_t>& cycle);
    
private:
    // DFS检测环
    bool HasCycle(int32_t transaction_id, 
                 std::unordered_set<int32_t>& visited,
                 std::unordered_set<int32_t>& recursion_stack,
                 std::vector<int32_t>& path);
};
```

#### 分层锁管理器

```cpp
class HierarchicalLockManager {
public:
    // 获取页面锁
    bool AcquirePageLock(int32_t transaction_id, int32_t page_id, bool exclusive);
    
    // 释放页面锁
    void ReleasePageLock(int32_t transaction_id, int32_t page_id);
    
    // 获取锁统计
    LockManagerStats GetStats() const;
    
private:
    // 页面锁信息
    struct PageLockInfo {
        std::shared_mutex mutex;
        std::unordered_set<int32_t> shared_owners;
        int32_t exclusive_owner;
        std::condition_variable cv;
    };
    
    std::unordered_map<int32_t, std::unique_ptr<PageLockInfo>> page_locks_;
    mutable std::shared_mutex locks_mutex_;
    
    // 死锁检测器
    std::unique_ptr<DeadlockDetector> deadlock_detector_;
    
    // 配置参数
    std::chrono::milliseconds lock_timeout_;
    int max_retries_;
    
    // 统计信息
    std::atomic<uint64_t> lock_acquisitions_;
    std::atomic<uint64_t> lock_contentions_;
    std::atomic<uint64_t> lock_timeouts_;
    std::atomic<uint64_t> deadlock_detections_;
};
```

### 预取系统

```cpp
class Prefetcher {
public:
    // 通知页面访问
    void NotifyPageAccess(int32_t transaction_id, int32_t page_id);
    
    // 获取预取的页面
    bool GetPrefetchedPage(int32_t page_id, Page& page);
    
    // 启用/禁用预取
    void SetEnabled(bool enabled);
    
    // 获取预取统计
    PrefetcherStats GetStats() const;
    
private:
    // 访问历史
    struct AccessHistory {
        std::vector<int32_t> sequence;
        std::chrono::steady_clock::time_point last_access;
    };
    
    std::unordered_map<int32_t, AccessHistory> access_histories_;
    
    // 预取队列
    std::queue<int32_t> prefetch_queue_;
    std::unordered_map<int32_t, Page> prefetched_pages_;
    
    // 配置参数
    bool enabled_;
    size_t max_prefetch_pages_;
    size_t access_history_size_;
    std::chrono::milliseconds prefetch_threshold_;
    
    // 统计信息
    std::atomic<uint64_t> prefetch_requests_;
    std::atomic<uint64_t> prefetch_hits_;
    std::atomic<uint64_t> prefetch_misses_;
    
    // 内部方法
    void AnalyzeAccessPattern(int32_t transaction_id);
    void PrefetchPages(const std::vector<int32_t>& candidates);
    bool IsSequentialAccess(const AccessHistory& history);
    std::vector<int32_t> PredictNextPages(const AccessHistory& history);
};
```

### 性能监控系统

#### 指标类型

1. **计数器 (Counter)**：只增不减的值，如缓存命中次数
2. **仪表盘 (Gauge)**：可增可减的当前值，如当前缓存大小
3. **直方图 (Histogram)**：值的分布统计，如I/O大小分布
4. **计时器 (Timer)**：时间统计，如平均响应时间

#### 存储引擎专用指标

```cpp
class StorageEngineMetrics {
public:
    // BufferPool指标
    std::shared_ptr<CounterMetric> buffer_pool_hits;
    std::shared_ptr<CounterMetric> buffer_pool_misses;
    std::shared_ptr<CounterMetric> buffer_pool_evictions;
    std::shared_ptr<GaugeMetric> buffer_pool_size;
    std::shared_ptr<TimerMetric> buffer_pool_fetch_time;
    
    // 替换策略指标
    std::shared_ptr<CounterMetric> replacement_strategy_accesses;
    std::shared_ptr<HistogramMetric> replacement_strategy_access_frequency;
    
    // 锁管理器指标
    std::shared_ptr<CounterMetric> lock_acquisitions;
    std::shared_ptr<CounterMetric> lock_contentions;
    std::shared_ptr<TimerMetric> lock_wait_time;
    
    // 死锁检测指标
    std::shared_ptr<CounterMetric> deadlock_detections;
    std::shared_ptr<TimerMetric> deadlock_detection_time;
    
    // 预取器指标
    std::shared_ptr<CounterMetric> prefetch_requests;
    std::shared_ptr<CounterMetric> prefetch_hits;
    
    // 磁盘I/O指标
    std::shared_ptr<CounterMetric> disk_reads;
    std::shared_ptr<CounterMetric> disk_writes;
    std::shared_ptr<TimerMetric> disk_read_time;
    std::shared_ptr<HistogramMetric> disk_io_size;
    
    // 事务指标
    std::shared_ptr<CounterMetric> transaction_commits;
    std::shared_ptr<TimerMetric> transaction_duration;
};
```

## 使用示例

### 创建和配置BufferPool

```cpp
// 创建配置管理器
auto config_manager = std::make_unique<ConfigManager>();
config_manager->SetValue("buffer.replace_strategy", "2Q");
config_manager->SetValue("buffer.pool_size", "1000");
config_manager->SetValue("lock.timeout_ms", "1000");
config_manager->SetValue("prefetch.enabled", "true");

// 创建磁盘管理器
auto disk_manager = std::make_shared<DiskManager>(db_path);

// 创建BufferPool
BufferPool buffer_pool(disk_manager, *config_manager, 1000);

// 初始化性能指标
StorageEngineMetrics::GetInstance().Initialize();
```

### 基本操作

```cpp
// 创建新页面
int32_t page_id = buffer_pool.NewPage(transaction_id);

// 获取页面
auto page = buffer_pool.FetchPage(page_id, transaction_id);
if (page) {
    // 修改页面数据
    page->data.data()[0] = 'X';
    
    // 释放页面
    buffer_pool.UnpinPage(page_id, transaction_id);
}

// 刷新页面
buffer_pool.FlushPage(page_id);

// 删除页面
buffer_pool.DeletePage(page_id);
```

### 动态切换替换策略

```cpp
// 切换到LRU策略
buffer_pool.ChangeReplaceStrategy(ReplaceStrategyFactory::StrategyType::LRU);

// 切换到2Q策略
buffer_pool.ChangeReplaceStrategy(ReplaceStrategyFactory::StrategyType::TWO_Q);
```

### 性能监控

```cpp
// 获取BufferPool统计信息
auto stats = buffer_pool.GetStats();
std::cout << "Hit ratio: " << stats.hit_ratio << std::endl;
std::cout << "Dirty pages: " << stats.dirty_pages << std::endl;

// 导出所有性能指标
std::string metrics_json = PerformanceMonitor::GetInstance().ExportMetrics("json");
std::string metrics_prometheus = PerformanceMonitor::GetInstance().ExportMetrics("prometheus");
```

## 性能优化

### 1. 缓存局部性优化

- 使用per-page shared_mutex减少锁竞争
- 预取器基于访问模式预测未来访问
- 替换策略考虑页面访问模式

### 2. 并发优化

- 细粒度锁减少锁竞争
- 读写锁支持多读单写
- 死锁检测与预防避免系统阻塞

### 3. 内存优化

- 智能预取减少磁盘I/O
- 高效的替换策略提高缓存命中率
- 动态调整缓冲池大小适应工作负载

## 测试

### 单元测试

- 基本功能测试
- 替换策略测试
- 并发访问测试
- 死锁预防测试
- 预取功能测试
- 性能监控测试

### 性能测试

- 不同替换策略的命中率比较
- 并发性能测试
- 不同工作负载下的性能表现
- 预取效果测试

## 未来扩展

1. **更多替换策略**：ARC、LFU、Clock等
2. **自适应预取**：基于机器学习的预取策略
3. **NUMA感知**：针对NUMA架构的优化
4. **持久化内存支持**：利用持久化内存技术
5. **分布式缓存**：支持分布式环境下的缓存一致性

## 总结

本重构设计提供了一个高性能、可扩展的存储引擎架构，特别是BufferPool组件的优化。通过可插拔替换策略、高级并发控制和全面的性能监控，系统能够适应各种工作负载并提供优异的性能。

该设计保持了良好的模块化和可扩展性，便于未来添加新功能和优化。同时，全面的测试套件确保了系统的可靠性和正确性。