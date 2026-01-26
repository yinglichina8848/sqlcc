# Storage Engine 函数设计文档

## 函数列表

### strict_mode_

**定义位置**: `src/storage_engine/record_boundary_validator.cpp`

**签名**:
```cpp
      strict_mode_(true) {
}

ValidationResult RecordSizeValidator::ValidateRecordSize(size_t record...
```

---

### type_validator_

**定义位置**: `src/storage_engine/record_boundary_validator.cpp`

**签名**:
```cpp
      type_validator_(),
      constraint_validator_(storage_engine),  // 正确调用DataIntegrityConstrain...
```

---

### running_

**定义位置**: `src/storage_engine/lazy_writer.cpp`

**签名**:
```cpp
      running_(false), enabled_(true),
      flush_interval_(std::chrono::milliseconds(100)), max_di...
```

---

### wal_file_path_

**定义位置**: `src/storage_engine/wal_writer.cpp`

**签名**:
```cpp
      wal_file_path_(wal_file),
      running_(false),
      current_lsn_(0) {
```

---

### table_storage_

**定义位置**: `src/storage_engine/partition_manager.cpp`

**签名**:
```cpp
      table_storage_(std::move(table_storage)),
      index_manager_(std::move(index_manager)) {
}

...
```

---

### current_pages_

**定义位置**: `src/storage_engine/page_allocator.cpp`

**签名**:
```cpp
      current_pages_(0),
      allocation_strategy_(strategy),
      access_pattern_analyzer_(std::m...
```

---

### sequential_threshold_

**定义位置**: `src/storage_engine/page_allocator.cpp`

**签名**:
```cpp
      sequential_threshold_(0.8),
      predictability_threshold_(0.6) {
}

void PageAllocator::Acce...
```

---

### strategy_

**定义位置**: `src/storage_engine/cache_consistency_manager.cpp`

**签名**:
```cpp
      strategy_(strategy),
      lock_timeout_(std::chrono::milliseconds(5000)),
      version_check...
```

---

### fk_validator_

**定义位置**: `src/storage_engine/data_integrity_validator.cpp`

**签名**:
```cpp
      fk_validator_(storage_engine),  // 正确调用ForeignKeyValidator构造函数
      unique_validator_(storage...
```

---

### backup_disks_

**定义位置**: `src/storage_engine/disk_error_handler.cpp`

**签名**:
```cpp
      backup_disks_(std::move(backup_disks)) {
}

bool DiskRedundancyManager::WriteWithRedundancy(in...
```

---

### monitoring_start_

**定义位置**: `src/storage_engine/disk_error_handler.cpp`

**签名**:
```cpp
      monitoring_start_(std::chrono::steady_clock::now()) {
}

void DiskIOMonitor::RecordReadOperati...
```

---

### transaction_manager_

**定义位置**: `src/storage_engine/index_manager/enhanced_index_manager.cpp`

**签名**:
```cpp
      transaction_manager_(transaction_manager),
      smart_cache_(std::make_unique<SmartIndexCache...
```

---

### UpdateLockCounts

**定义位置**: `src/storage_engine/advanced_lock_manager.cpp`

**签名**:
```cpp
void UpdateLockCounts(PageLockState& state, LockType mode, int delta) {
    switch (mode) {
```

---

### default_timeout_

**定义位置**: `src/storage_engine/advanced_lock_manager.cpp`

**签名**:
```cpp
      default_timeout_(default_timeout),
      deadlock_detection_enabled_(true),
      deadlock_che...
```

---

### config_manager_

**定义位置**: `src/storage_engine/wal_buffer.cpp`

**签名**:
```cpp
      config_manager_(config_manager),
      max_buffer_size_(buffer_size),
      buffer_(),
      b...
```

---

### storage_engine_

**定义位置**: `src/storage_engine/checkpoint.cpp`

**签名**:
```cpp
      storage_engine_(storage_engine),
      wal_writer_(wal_writer),
      running_(false) {
```

---

### stop_prefetch_

**定义位置**: `src/storage_engine/concurrency_control.cpp`

**签名**:
```cpp
      stop_prefetch_(false), enabled_(true), stats_(), max_prefetch_size_(max_prefetch_size) {
```

---

### pool_size_

**定义位置**: `src/storage_engine/buffer_pool/buffer_pool_sharded.cpp`

**签名**:
```cpp
      pool_size_(pool_size), next_page_id_(0) {
  // 确保num_shards是2的幂 - 这是分片策略的核心优化
  if (num_shards...
```

---

### page_id_

**定义位置**: `src/storage_engine/table_storage/page_raii.cpp`

**签名**:
```cpp
      page_id_(other.page_id_), pinned_(other.pinned_) {
```

---

### SmartIndexCache

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp
    SmartIndexCache(size_t max_cache_size = 1000, std::chrono::minutes default_ttl = std::chrono::mi...
```

---

### CacheIndex

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp
    void CacheIndex(const std::string& index_name, std::unique_ptr<BPlusTreeIndex> index,
          ...
```

---

### RemoveIndex

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp
    bool RemoveIndex(const std::string& index_name) {
```

---

### WarmupCache

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp
    void WarmupCache(const std::vector<std::string>& predicted_indexes) {
```

---

### IntelligentCleanup

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp
    void IntelligentCleanup() {
```

---

### EvictCacheEntries

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp
    void EvictCacheEntries() {
```

---

### CleanupExpiredCache

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp
    void CleanupExpiredCache(std::chrono::minutes max_age = std::chrono::minutes(30)) {
```

---

### TransactionalIndexManager

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp
    TransactionalIndexManager(std::shared_ptr<StorageEngine> storage_engine)
        : storage_engin...
```

---

### CreateIndexTransactional

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp
    bool CreateIndexTransactional(const std::string& index_name,
                                con...
```

---

### DropIndexTransactional

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp
    bool DropIndexTransactional(const std::string& index_name,
                              const s...
```

---

### CommitTransaction

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp
    void CommitTransaction(int32_t transaction_id) {
```

---

### RollbackTransaction

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp
    void RollbackTransaction(int32_t transaction_id) {
```

---

### EnhancedIndexManager

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp
    EnhancedIndexManager(std::shared_ptr<StorageEngine> storage_engine,
                        std:...
```

---

### CreateIndex

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp
    bool CreateIndex(const std::string& index_name,
                    const std::string& table_nam...
```

---

### DropIndex

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp
    bool DropIndex(const std::string& index_name,
                  const std::string& table_name,
 ...
```

---

### StartCacheCleanupTimer

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp

    void StartCacheCleanupTimer() {
```

---

### IndexLifetimeGuard

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp
        IndexLifetimeGuard(std::unique_ptr<BPlusTreeIndex> index,
                          std::fun...
```

---

### SafeRelease

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp
    static void SafeRelease(std::unique_ptr<BPlusTreeIndex>& index) {
        if (index) {
```

---

### BatchRelease

**定义位置**: `src/storage_engine/index_manager/index_manager_smart_ptr_enhancement.cpp`

**签名**:
```cpp
    static void BatchRelease(std::vector<std::unique_ptr<BPlusTreeIndex>>& indexes) {
```

---

### SQLCC_LOG_INFO

**定义位置**: `src/storage_engine/index_manager/index_manager.cpp`

**签名**:
```cpp
  SQLCC_LOG_INFO("Creating composite index: " + index_name + " on table: " + table_name +
          ...
```

---

### next_page_id_

**定义位置**: `src/storage_engine/disk_manager/disk_manager.cpp`

**签名**:
```cpp
      next_page_id_(0) {
```

---

### 支持删除重复键中的特定条目

**定义位置**: `src/storage_engine/b_plus_tree/index/b_plus_tree_index.cpp`

**签名**:
```cpp
 *   支持删除重复键中的特定条目(page_id, offset组合)。
 *   自动处理节点合并，保持树的平衡和空间效率。
 *
 * HOW层 - 实现细节：
 *   1. 从根节点开始查...
```

---

### is_leaf_

**定义位置**: `src/storage_engine/b_plus_tree/core/b_plus_tree.cpp`

**签名**:
```cpp
      is_leaf_(is_leaf) {
  // 获取页面对象用于数据存储
  if (storage_engine_) {
```

---

### column_name_

**定义位置**: `src/storage_engine/b_plus_tree/core/b_plus_tree.cpp`

**签名**:
```cpp
      column_name_(column_name), root_page_id_(-1) {
```

---

### LockRequest

**定义位置**: `src/storage_engine/advanced_lock_manager.h`

**签名**:
```cpp

    LockRequest(int32_t tid, LockType m, int32_t pid,
                std::chrono::milliseconds t =...
```

---

### LockHolder

**定义位置**: `src/storage_engine/advanced_lock_manager.h`

**签名**:
```cpp

    LockHolder(int32_t tid, LockType m)
        : transaction_id(tid), mode(m),
          acquire_t...
```

---

### PrefetchRequest

**定义位置**: `src/storage_engine/buffer_pool/prefetcher.h`

**签名**:
```cpp

  PrefetchRequest(int32_t pid, int32_t tid, int p = 0)
      : page_id(pid), transaction_id(tid), p...
```

---

### PrefetcherStats

**定义位置**: `src/storage_engine/buffer_pool/prefetcher.h`

**签名**:
```cpp
    PrefetcherStats(const PrefetcherStats &other) {
```

---

### ConstraintRule

**定义位置**: `src/storage_engine/data_integrity_validator.h`

**签名**:
```cpp

    ConstraintRule(const std::string& name, IntegrityConstraintType type, const std::string& table)...
```

---

### WALRecord

**定义位置**: `src/storage_engine/wal_buffer.h`

**签名**:
```cpp

    WALRecord(uint64_t lsn_val, uint64_t tx_id, const std::string& op, const std::string& d)
      ...
```

---

### SetWALWriter

**定义位置**: `src/storage_engine/wal_buffer.h`

**签名**:
```cpp
  void SetWALWriter(WALWriter* wal_writer) {
```

---

### FieldValidationRule

**定义位置**: `src/storage_engine/record_boundary_validator.h`

**签名**:
```cpp

    FieldValidationRule(const std::string& name, const std::string& type)
        : field_name(name...
```

---

### HistogramBucket

**定义位置**: `src/storage_engine/performance_monitor.h`

**签名**:
```cpp

  HistogramBucket(double bound) : upper_bound(bound), count(0) {}

  // 拷贝构造函数（需要处理原子变量的不可拷贝性）
  Hi...
```

---

### Increment

**定义位置**: `src/storage_engine/performance_monitor.h`

**签名**:
```cpp
  void Increment(uint64_t delta) {
```

---

### Set

**定义位置**: `src/storage_engine/performance_monitor.h`

**签名**:
```cpp

  void Set(uint64_t value) {
```

---

### DiskErrorInfo

**定义位置**: `src/storage_engine/disk_error_handler.h`

**签名**:
```cpp

    DiskErrorInfo(DiskErrorType type, int32_t pid, const std::string& msg,
                  const ...
```

---

### PageChecksum

**定义位置**: `src/storage_engine/disk_error_handler.h`

**签名**:
```cpp
    PageChecksum() : page_id(0), checksum(0), last_verified(std::chrono::steady_clock::now()) {}

  ...
```

---

### PageWrapper

**定义位置**: `src/storage_engine/buffer_pool/buffer_pool_sharded.h`

**签名**:
```cpp

        PageWrapper(std::unique_ptr<Page> page_ptr = nullptr)
            : page(std::move(page_ptr...
```

---

### Shard

**定义位置**: `src/storage_engine/buffer_pool/buffer_pool_sharded.h`

**签名**:
```cpp

        Shard(size_t max_size = 0) : current_size(0), max_size(max_size) {
```

---

### LockInfo

**定义位置**: `src/storage_engine/concurrency_control.h`

**签名**:
```cpp
    LockInfo() : type(LockType::SHARED), transaction_id(0),
                 acquire_time(std::chron...
```

---

### setState

**定义位置**: `src/storage_engine/partition_manager.h`

**签名**:
```cpp
    void setState(State state) {
```

---

### setLastModified

**定义位置**: `src/storage_engine/partition_manager.h`

**签名**:
```cpp
    void setLastModified(const std::chrono::system_clock::time_point& time) {
```

---

### setRange

**定义位置**: `src/storage_engine/partition_manager.h`

**签名**:
```cpp
    void setRange(const Range& range) {
```

---

### setValueList

**定义位置**: `src/storage_engine/partition_manager.h`

**签名**:
```cpp
    void setValueList(const ValueList& value_list) {
```

---

### DirtyPageInfo

**定义位置**: `src/storage_engine/lazy_writer.h`

**签名**:
```cpp

  DirtyPageInfo(int32_t pid)
      : page_id(pid), modification_count(1), is_pinned(false),
       ...
```

---

### LazyWriterStats

**定义位置**: `src/storage_engine/lazy_writer.h`

**签名**:
```cpp
    LazyWriterStats(const LazyWriterStats &other) {
```

---

### SetParentPageId

**定义位置**: `src/storage_engine/b_plus_tree/node/b_plus_tree_node.h`

**签名**:
```cpp
    void SetParentPageId(int32_t parent_id) {
```

---

### IndexEntry

**定义位置**: `src/storage_engine/b_plus_tree/node/b_plus_tree_node.h`

**签名**:
```cpp

    IndexEntry() : page_id(-1), offset(0) {}
    IndexEntry(const std::string& k, int32_t p, size_t...
```

---

### SetNextPageId

**定义位置**: `src/storage_engine/b_plus_tree/node/b_plus_tree_nodes.h`

**签名**:
```cpp
    void SetNextPageId(int32_t next_page_id) {
```

---

### PageAccessInfo

**定义位置**: `src/storage_engine/replace_strategy/abstract_strategy.h`

**签名**:
```cpp

        PageAccessInfo(int32_t id)
            : page_id(id), access_count(0), is_dirty(false), pin...
```

---

### UpdateHitRate

**定义位置**: `src/storage_engine/replace_strategy/abstract_strategy.h`

**签名**:
```cpp

        void UpdateHitRate() {
```

---

