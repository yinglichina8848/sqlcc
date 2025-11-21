# 《数据库系统原理与开发实践》 - 第10章：存储引擎的实现：从文件到数据的艺术

**页面管理、缓冲池与持久化存储的核心技术**

---

## 🎯 **本章核心目标**

深入理解数据库存储引擎的完整实现原理：
- 页面(page)管理和磁盘I/O优化策略
- 缓冲池(buffer pool)的LRU缓存机制
- 存储格式设计与数据持久化技术
- 存储引擎架构的扩展性设计模式

---

## 10.1 页面管理系统：磁盘与内存的数据桥梁

### 10.1.1 页面结构的层次设计

#### 📄 **数据库页面的三层结构模型**

```
数据库页面的完整层次结构：

┌─────────────────────────────────────────────────────────────┐
│                    物理页面层 (Physical Layer)                │
│  ├─ 页面头 (Page Header): 元数据信息、校验和、LSN等         │
│  ├─ 数据区 (Data Area): 实际的元组数据存储区域            │
│  └─ 页面尾 (Page Footer): 校验和、页面ID等                  │
├─────────────────────────────────────────────────────────────┤

│                   数据组织层 (Organization Layer)             │
│  ├─ 槽目录 (Slot Directory): 记录位置偏移量数组             │
│  ├─ 空闲空间管理: 分配、新增、删除时的空间回收              │
│  └─ 数据的物理存储: 元组的实际存储格式和编码                │
├─────────────────────────────────────────────────────────────┤

│                  逻辑抽象层 (Logical Layer)                   │
│  ├─ 页面ID映射: 逻辑页面ID到物理位置的映射                │
│  ├─ 并发访问控制: 页面级别的读写锁机制                      │
│  └─ 页面生命周期: 创建、修改、刷盘、回收的完整管理         │
└─────────────────────────────────────────────────────────────┘
```

#### 🏗️ **页面头部的核心元数据设计**

```cpp
class PageHeader {
private:
    static const uint16_t PAGE_SIZE = 4096;         // 页面大小 (4KB)
    static const uint16_t HEADER_SIZE = 128;        // 页面头大小

    // 基础页面信息
    uint32_t page_id;                               // 页面唯一标识
    uint32_t next_page_id;                          // 链表下一个页面
    uint32_t prev_page_id;                          // 链表上一个页面
    uint8_t page_type;                              // 页面类型 (数据页/索引页等)

    // 版本与校验信息
    uint64_t lsn;                                   // 日志序列号 (用于WAL)
    uint32_t checksum;                              // 页面校验和
    uint32_t version;                               // 页面版本号

    // 空间管理信息
    uint16_t free_space_start;                       // 空闲空间起始位置
    uint16_t free_space_end;                         // 空闲空间结束位置
    uint16_t slot_count;                             // 槽位数量 (记录数量)
    uint16_t first_free_slot;                        // 第一个空闲槽位

    // 并发控制标识
    std::atomic<uint64_t> pin_count{0};             // 页面固定计数
    std::shared_mutex page_lock;                     // 页面级共享锁

public:
    // 页面空间计算
    uint16_t get_free_space() const {
        return free_space_end - free_space_start;
    }

    // 检查是否有足够空间存储新元组
    bool can_fit_tuple(size_t tuple_size) const {
        return get_free_space() >= tuple_size + sizeof(SlotEntry);
    }

    // 并发访问控制
    void pin_page() {
        pin_count.fetch_add(1, std::memory_order_relaxed);
    }

    void unpin_page() {
        pin_count.fetch_sub(1, std::memory_order_relaxed);
    }

    // 读写锁封装
    void read_lock() { page_lock.lock_shared(); }
    void read_unlock() { page_lock.unlock_shared(); }
    void write_lock() { page_lock.lock(); }
    void write_unlock() { page_lock.unlock(); }
};

// 槽目录项结构
struct SlotEntry {
    uint16_t offset;        // 记录在页面中的偏移量
    uint16_t length;        // 记录的长度
    uint8_t flags;          // 标志位 (删除标记等)
};
```

### 10.1.2 页面分配与回收机制

#### 🗂️ **页面分配策略的多种选择**

```cpp
class PageAllocator {
private:
    enum AllocationStrategy {
        FIRST_FIT,         // 首次适应算法
        BEST_FIT,          // 最佳适应算法
        WORST_FIT,         // 最坏适应算法
        NEXT_FIT           // 下次适应算法
    };

    AllocationStrategy strategy;
    std::set<PageId> free_pages;                    // 可用页面集合
    std::map<PageId, PageMetadata> page_metadata;   // 页面元数据

    struct PageMetadata {
        bool is_allocated;
        size_t free_space;
        std::chrono::system_clock::time_point last_accessed;
        uint32_t access_count;
    };

public:
    PageId allocate_page(PageType type = DATA_PAGE) {
        PageId page_id = find_suitable_page(type);

        if (page_id.is_valid()) {
            // 页面已存在，标记为已分配
            mark_page_allocated(page_id);
        } else {
            // 需要分配新页面
            page_id = allocate_new_page(type);
        }

        initialize_page(page_id, type);
        return page_id;
    }

    void deallocate_page(PageId page_id) {
        // 清理页面内容
        clear_page_content(page_id);

        // 添加到空闲页面列表
        free_pages.insert(page_id);

        // 更新元数据
        page_metadata[page_id].is_allocated = false;
        page_metadata[page_id].free_space = PageHeader::PAGE_SIZE - PageHeader::HEADER_SIZE;
    }

private:
    PageId find_suitable_page(PageType type) {
        switch (strategy) {
            case FIRST_FIT:
                return find_first_fit_page(type);
            case BEST_FIT:
                return find_best_fit_page(type);
            case WORST_FIT:
                return find_worst_fit_page(type);
            case NEXT_FIT:
                return find_next_fit_page(type);
        }
        return PageId::INVALID;
    }

    PageId find_best_fit_page(PageType type) {
        PageId best_page = PageId::INVALID;
        size_t min_wasted_space = SIZE_MAX;

        for (const auto& page_id : free_pages) {
            const auto& metadata = page_metadata[page_id];
            if (metadata.free_space >= required_space(type)) {
                size_t wasted = metadata.free_space - required_space(type);
                if (wasted < min_wasted_space) {
                    min_wasted_space = wasted;
                    best_page = page_id;
                }
            }
        }

        return best_page;
    }
};
```

## 10.2 缓冲池管理：内存与磁盘的性能桥梁

### 10.2.1 LRU缓存算法的精确实现

#### 🔄 **双向链表+哈希表的经典LRU实现**

```cpp
class BufferPool {
private:
    struct Frame {
        PageId page_id;
        char* data;                                    // 页面数据指针
        bool is_dirty;                                // 脏页面标记
        std::chrono::steady_clock::time_point last_access;
        uint64_t pin_count;                           // 固定计数
        std::mutex frame_mutex;                       // 帧级锁

        // 双向链表指针
        Frame* prev;
        Frame* next;
    };

    // LRU链表结构
    Frame* lru_head;     // LRU头部 (最近最少使用)
    Frame* lru_tail;     // LRU尾部 (最近最少使用)
    std::mutex lru_mutex; // 保护LRU链表

    // 页面到帧的快速映射
    std::unordered_map<PageId, Frame*> page_table;
    std::mutex page_table_mutex;

    // 帧数组
    std::vector<Frame> frames;
    size_t max_frames;

    // 统计信息
    std::atomic<uint64_t> cache_hits{0};
    std::atomic<uint64_t> cache_misses{0};

public:
    BufferPool(size_t pool_size_mb) {
        // 计算帧数量 (假设页面大小4KB)
        max_frames = (pool_size_mb * 1024 * 1024) / PageHeader::PAGE_SIZE;
        frames.resize(max_frames);

        initialize_frames();
        initialize_lru_list();
    }

    Frame* get_page(PageId page_id, bool is_write = false) {
        // 第一步：检查页面是否已在缓存中
        {
            std::unique_lock<std::mutex> lock(page_table_mutex);
            auto it = page_table.find(page_id);
            if (it != page_table.end()) {
                Frame* frame = it->second;
                cache_hits++;

                // 如果需要写访问，检查版本冲突
                if (is_write && frame->pin_count > 0) {
                    // 等待其他读者完成
                    std::unique_lock<std::mutex> frame_lock(frame->frame_mutex);
                    // 这里可以实现MVCC版本检查
                }

                move_to_lru_front(frame);
                return frame;
            }
        }

        // 第二步：页面不在缓存中，需要加载
        cache_misses++;
        return load_page_from_disk(page_id, is_write);
    }

private:
    Frame* load_page_from_disk(PageId page_id, bool is_write) {
        // 1. 找到或创建受害者帧
        Frame* victim_frame = find_victim_frame();

        // 2. 如果受害者帧是脏页面，需要刷盘
        if (victim_frame->is_dirty) {
            flush_page_to_disk(victim_frame);
        }

        // 3. 从磁盘加载新页面
        load_page_data(victim_frame, page_id);

        // 4. 更新页面表映射
        {
            std::unique_lock<std::mutex> lock(page_table_mutex);
            page_table[page_id] = victim_frame;
        }

        // 5. 移动到LRU头部
        move_to_lru_front(victim_frame);

        return victim_frame;
    }

    Frame* find_victim_frame() {
        std::unique_lock<std::mutex> lru_lock(lru_mutex);

        // 从LRU尾部开始查找未固定的帧
        Frame* current = lru_tail;
        while (current) {
            std::unique_lock<std::mutex> frame_lock(current->frame_mutex);

            if (current->pin_count == 0) {
                // 找到合适的受害者
                remove_from_lru_list(current);
                return current;
            }

            current = current->prev;
        }

        // 没有找到受害者，等待或扩展缓冲池
        return handle_no_victim_found();
    }

    void move_to_lru_front(Frame* frame) {
        std::unique_lock<std::mutex> lock(lru_mutex);

        // 从当前位置移除
        remove_from_lru_list(frame);

        // 插入到头部
        frame->next = lru_head;
        frame->prev = nullptr;

        if (lru_head) {
            lru_head->prev = frame;
        } else {
            lru_tail = frame;
        }

        lru_head = frame;
        frame->last_access = std::chrono::steady_clock::now();
    }

    void flush_page_to_disk(Frame* frame) {
        // 异步刷盘实现
        async_write_page(frame->page_id, frame->data, PageHeader::PAGE_SIZE);

        frame->is_dirty = false;

        // 更新WAL日志
        log_page_flush(frame->page_id);
    }
};
```

### 10.2.2 缓冲池的并发优化策略

#### 🔒 **页面级锁与闩锁的并发控制**

```cpp
class ConcurrentBufferPool {
private:
    // 哈希分区减少锁竞争
    static const int NUM_PARTITIONS = 16;

    struct Partition {
        std::unordered_map<PageId, Frame*> page_table;
        std::mutex partition_mutex;

        // 每个分区的LRU链表
        Frame* partition_lru_head;
        Frame* partition_lru_tail;
        std::mutex lru_mutex;
    };

    std::array<Partition, NUM_PARTITIONS> partitions;
    std::hash<PageId> page_id_hasher;

public:
    Frame* get_page_concurrent(PageId page_id) {
        // 1. 计算分区索引
        size_t partition_idx = page_id_hasher(page_id) % NUM_PARTITIONS;
        Partition& partition = partitions[partition_idx];

        // 2. 在分区内查找页面
        {
            std::unique_lock<std::mutex> lock(partition.partition_mutex);
            auto it = partition.page_table.find(page_id);
            if (it != partition.page_table.end()) {
                Frame* frame = it->second;

                // 移动到分区LRU头部
                move_to_partition_front(partition, frame);

                cache_hits++;
                return frame;
            }
        }

        // 3. 页面不在分区中，全局查找或加载
        return load_page_across_partitions(page_id, partition_idx);
    }

private:
    Frame* load_page_across_partitions(PageId page_id, size_t source_partition) {
        // 首先尝试从其他分区窃取
        for (size_t i = 0; i < NUM_PARTITIONS; i++) {
            if (i == source_partition) continue;

            Partition& other_partition = partitions[i];
            std::unique_lock<std::mutex> lock(other_partition.partition_mutex);

            auto it = other_partition.page_table.find(page_id);
            if (it != other_partition.page_table.end()) {
                // 找到页面，从其他分区移动到当前分区
                Frame* frame = it->second;
                move_frame_between_partitions(frame, i, source_partition);
                return frame;
            }
        }

        // 页面确实不存在，需要从磁盘加载
        return load_page_from_disk_to_partition(page_id, source_partition);
    }

    void move_frame_between_partitions(Frame* frame, size_t from_partition,
                                     size_t to_partition) {
        Partition& from_part = partitions[from_partition];
        Partition& to_part = partitions[to_partition];

        // 从原分区移除
        {
            std::unique_lock<std::mutex> lock(from_part.partition_mutex);
            from_part.page_table.erase(frame->page_id);
        }

        // 添加到目标分区
        {
            std::unique_lock<std::mutex> lock(to_part.partition_mutex);
            to_part.page_table[frame->page_id] = frame;
            move_to_partition_front(to_part, frame);
        }
    }
};
```

## 10.3 存储引擎的架构设计与扩展性

### 10.3.1 插件化存储引擎框架

#### 🔌 **存储引擎接口的设计模式**

```cpp
class StorageEngine {
public:
    virtual ~StorageEngine() {}

    // 核心存储操作
    virtual PageId allocate_page() = 0;
    virtual void deallocate_page(PageId page_id) = 0;

    virtual void read_page(PageId page_id, char* buffer) = 0;
    virtual void write_page(PageId page_id, const char* buffer) = 0;

    // 索引管理
    virtual Index* create_index(const std::string& name,
                               const std::vector<ColumnDef>& columns) = 0;
    virtual void drop_index(const std::string& name) = 0;

    // 事务支持
    virtual Transaction* begin_transaction() = 0;
    virtual void commit_transaction(Transaction* txn) = 0;
    virtual void rollback_transaction(Transaction* txn) = 0;

    // 元数据管理
    virtual Table* create_table(const TableDef& table_def) = 0;
    virtual void drop_table(const std::string& name) = 0;

    // 统计信息
    virtual StorageStats get_storage_stats() = 0;
};

// MySQL存储引擎接口示例
class MySQLStorageEngine : public StorageEngine {
private:
    std::unique_ptr<BufferPool> buffer_pool;
    std::unique_ptr<PageAllocator> page_allocator;
    std::unique_ptr<LogManager> log_manager;

    // 引擎特定配置
    bool support_transactions;        // 是否支持事务
    bool support_mvcc;               // 是否支持MVCC
    IsolationLevel max_isolation;     // 支持的最大隔离级别

public:
    MySQLStorageEngine(const EngineConfig& config) {
        // 初始化组件
        buffer_pool = std::make_unique<BufferPool>(config.buffer_pool_size_mb);
        page_allocator = std::make_unique<PageAllocator>(config.data_directory);
        log_manager = std::make_unique<LogManager>(config.log_directory);

        // 设置引擎特性
        initialize_engine_capabilities(config);
    }

    bool supports_feature(StorageFeature feature) const {
        switch (feature) {
            case TRANSACTIONS:
                return support_transactions;
            case MVCC:
                return support_mvcc;
            case FOREIGN_KEYS:
                return supports_foreign_keys;
            case FULLTEXT_INDEX:
                return supports_fulltext;
            default:
                return false;
        }
    }
};
```

### 10.3.2 存储格式与序列化技术

#### 📦 **元组存储格式的设计与优化**

```cpp
class TupleSerializer {
private:
    // 支持的数据类型
    enum DataType {
        INTEGER, VARCHAR, BLOB, BOOLEAN, FLOAT, DOUBLE, TIMESTAMP
    };

    // 存储格式定义
    struct StoredTuple {
        uint16_t tuple_header_size;    // 元组头部大小
        uint16_t field_count;         // 字段数量
        uint32_t null_bitmap_offset;  // null位图偏移
        uint32_t data_offset;         // 数据区域偏移

        // 可变长字段的偏移量数组
        std::vector<uint16_t> field_offsets;

        // null位图 (按字节对齐)
        char null_bits[0];           // 柔性数组成员

        // 实际数据 (紧凑存储)
        char data[0];
    };

public:
    std::vector<char> serialize_tuple(const Tuple& tuple) {
        std::vector<char> buffer;

        // 计算需要的空间
        size_t required_size = calculate_serialized_size(tuple);
        buffer.reserve(required_size);

        // 构建元组头部
        build_tuple_header(buffer, tuple);

        // 序列化null位图
        serialize_null_bitmap(buffer, tuple);

        // 序列化字段数据
        serialize_field_data(buffer, tuple);

        // 压缩优化 (optional)
        if (should_compress(tuple)) {
            return compress_buffer(buffer);
        }

        return buffer;
    }

    Tuple deserialize_tuple(const std::vector<char>& buffer) {
        Tuple tuple;

        // 解析头部
        const StoredTuple* stored = reinterpret_cast<const StoredTuple*>(buffer.data());

        // 验证校验和 (如果有)
        validate_tuple_checksum(stored);

        // 解析字段
        parse_tuple_fields(tuple, stored, buffer);

        return tuple;
    }

private:
    size_t calculate_serialized_size(const Tuple& tuple) {
        size_t size = sizeof(StoredTuple);

        // 字段偏移量数组
        size += tuple.fields.size() * sizeof(uint16_t);

        // null位图 (按字节对齐)
        size += (tuple.fields.size() + 7) / 8;

        // 实际数据
        for (const Field& field : tuple.fields) {
            size += get_field_storage_size(field);
        }

        return size;
    }

    void serialize_field_data(std::vector<char>& buffer, const Tuple& tuple) {
        for (size_t i = 0; i < tuple.fields.size(); i++) {
            const Field& field = tuple.fields[i];

            // 检查字段是否为null
            if (is_field_null(tuple, i)) {
                continue;  // null字段不存储任何数据
            }

            // 根据字段类型序列化
            switch (field.type) {
                case DataType::INTEGER:
                    serialize_int32(buffer, field.int_value);
                    break;
                case DataType::VARCHAR:
                    serialize_varchar(buffer, field.string_value);
                    break;
                case DataType::BLOB:
                    serialize_blob(buffer, field.blob_value);
                    break;
                // 其他类型...
            }
        }
    }

    // 针对不同工作负载的压缩策略
    enum CompressionStrategy {
        NONE, LZ4, ZSTD, ADAPTIVE
    };

    bool should_compress(const Tuple& tuple) const {
        // 根据数据特征决定是否压缩
        size_t uncompressed_size = calculate_tuple_size(tuple);

        // 小元组不压缩
        if (uncompressed_size < 1024) {
            return false;
        }

        // 高重复性数据适合压缩
        if (has_high_redundancy(tuple)) {
            return true;
        }

        // 查询模式驱动的压缩决策
        return is_analytics_workload();
    }
};
```

## 📚 **本章总结：存储引擎是数据库系统的核心心脏**

存储引擎是数据库系统的核心组件，它直接决定了数据库的性能、可靠性和扩展性。一个优秀的存储引擎需要平衡多个相互冲突的设计目标：

**核心设计挑战**：
- **性能与持久性**: 缓冲池提供内存速度的同时，WAL确保数据持久性
- **并发与一致性**: 多版本并发控制(MVCC)允许多个事务同时访问同一数据
- **存储效率与访问速度**: B+树索引在磁盘I/O和查找性能之间找到最佳平衡点
- **扩展性与简洁性**: 插件化架构允许扩展新的功能而不破坏现有代码

**设计原则的权衡**：
- **以页面为基本单位**: 平衡磁盘I/O效率和内存利用率
- **延迟刷盘策略**: 在崩溃恢复复杂性和性能之间找到平衡
- **乐观并发控制**: 减少锁竞争但可能增加冲突解决开销
- **自适应调整**: 根据工作负载动态调整各种参数

理解存储引擎的实现原理，是掌握数据库系统核心技术的关键。通过本章的学习，读者不仅理解了数据库的内部实现机制，更培养了对复杂系统设计的思维方法和工程素养。

---

**思考题**：
1. 为什么数据库使用页面(page)作为存储的基本单位？页面大小的选择有什么考虑因素？
2. LRU替换算法在缓冲池中有哪些变体？它们各自适用于什么场景？
