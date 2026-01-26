# Index System 类设计文档

## 类列表

### StorageEngine

**定义位置**: `include/storage_engine/index_manager.h`

**定义**:
```cpp
class StorageEngine;
class BPlusTreeIndex;
class ConfigManager;
} // namespace sqlcc

namespace sqlcc {

class IndexManager {
public:
  IndexManager(std::shared_ptr<StorageEngine> storage_engine, Conf...
```

**构造函数**:
- `IndexManager`
- `IndexManager`
- `CreateIndex`
- `CreateCompositeIndex`
- `DropIndex`
- `GetIndex`
- `LoadAllIndexes`

**析构函数**:
- `IndexManager`

**公有方法**:
- `索引管理
  bool CreateIndex`
- `bool CreateCompositeIndex`
- `bool DropIndex`
- `内部方法
  void LoadAllIndexes`

---

### IndexManager

**定义位置**: `src/storage_engine/index_manager/smart_index_factory.h`

**定义**:
```cpp

class IndexManager;
class SmartIndexCache;

// Factory class for creating index managers with smart caching
class SmartIndexFactory {
public:
    explicit SmartIndexFactory(std::shared_ptr<SmartIndex...
```

**构造函数**:
- `SmartIndexFactory`
- `SmartIndexFactory`
- `create_index_manager`

**析构函数**:
- `SmartIndexFactory`

**公有方法**:
- `explicit SmartIndexFactory`

---

### IndexCacheEntry

**定义位置**: `src/storage_engine/index_manager/smart_index_cache.h`

**定义**:
```cpp

class IndexCacheEntry {
public:
    explicit IndexCacheEntry(const std::string& key);
    virtual ~IndexCacheEntry() = default;

    const std::string& get_key() const { return key_; }
    virtual si...
```

**构造函数**:
- `IndexCacheEntry`

**公有方法**:
- `explicit IndexCacheEntry`

---

### SmartIndexCache

**定义位置**: `src/storage_engine/index_manager/smart_index_cache.h`

**定义**:
```cpp

class SmartIndexCache {
public:
    explicit SmartIndexCache(size_t max_size);
    ~SmartIndexCache();

    // Cache operations
    bool put(const std::string& key, std::shared_ptr<IndexCacheEntry> e...
```

**构造函数**:
- `SmartIndexCache`
- `SmartIndexCache`
- `put`
- `get`
- `remove`
- `clear`

**析构函数**:
- `SmartIndexCache`

**公有方法**:
- `explicit SmartIndexCache`
- `operations
    bool put`
- `bool remove`
- `void clear`

---

### IndexManagerMock

**定义位置**: `include/mocks/index_manager_mock.h`

**定义**:
```cpp
class IndexManagerMock : public IndexManager {
public:
    /**
     * @brief 构造函数
     * @param storage_engine 存储引擎智能指针
     * @param config_manager 配置管理器引用
     */
    IndexManagerMock(std::shared_pt...
```

**构造函数**:
- `IndexManagerMock`
- `IndexManagerMock`
- `SetCreateIndexResult`
- `SetCreateCompositeIndexResult`
- `SetDropIndexResult`
- `SetIndexExistsResult`
- `SetGetIndexResult`
- `SetGetTableIndexesResult`
- `SetGetIndexedColumnsResult`
- `SetGetCompositeIndexedColumnsResult`
- `SetGetIndexNameResult`
- `SetGetCompositeIndexNameResult`
- `ClearCallHistory`
- `CreateIndex`
- `CreateCompositeIndex`
- `DropIndex`
- `GetIndex`

**析构函数**:
- `IndexManagerMock`

**公有方法**:
- `Mock配置方法
    void SetCreateIndexResult`
- `void SetCreateCompositeIndexResult`
- `void SetDropIndexResult`
- `void SetIndexExistsResult`
- `void SetGetIndexResult`
- `void SetGetTableIndexesResult`
- `void SetGetIndexedColumnsResult`
- `void SetGetCompositeIndexedColumnsResult`
- `void SetGetIndexNameResult`
- `void SetGetCompositeIndexNameResult`
- `void ClearCallHistory`
- `重写IndexManager接口方法
    bool CreateIndex`
- `bool CreateCompositeIndex`
- `bool DropIndex`

---

