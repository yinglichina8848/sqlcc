# Index System 函数设计文档

## 函数列表

### transaction_manager_

**定义位置**: `src/storage_engine/index_manager/enhanced_index_manager.cpp`

**签名**:
```cpp
      transaction_manager_(transaction_manager),
      smart_cache_(std::make_unique<SmartIndexCache...
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

### ClearCallHistory

**定义位置**: `include/mocks/index_manager_mock.h`

**签名**:
```cpp
    void ClearCallHistory() {
```

---

