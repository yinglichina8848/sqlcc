# IndexManager 设计文档

## 概述

IndexManager是SQLCC数据库系统中的索引管理组件，负责协调所有索引的创建、维护和使用。它是连接查询执行器和底层索引数据结构的桥梁，直接决定了查询优化的效果和索引维护的效率。

### 主要功能

- 索引生命周期管理：创建、删除和更新维护索引
- 索引查找协调：根据查询条件选择合适的索引
- 索引存储管理：索引数据的持久化和故障恢复
- 复合索引支持：多列复合索引的管理
- 索引信息查询：提供索引元数据和统计信息

## 类定义

```cpp
class IndexManager {
public:
  IndexManager(std::shared_ptr<StorageEngine> storage_engine, ConfigManager &config_manager);
  ~IndexManager();

  // 索引管理
  bool CreateIndex(const std::string &index_name, const std::string &table_name,
                   const std::string &column_name, bool unique = false);
  bool CreateCompositeIndex(const std::string &index_name,
                           const std::string &table_name,
                           const std::vector<std::string> &columns,
                           bool unique = false);
  bool DropIndex(const std::string &index_name, const std::string &table_name);
  bool IndexExists(const std::string &index_name,
                   const std::string &table_name) const;

  // 索引查询
  BPlusTreeIndex *GetIndex(const std::string &index_name,
                           const std::string &table_name);
  std::vector<BPlusTreeIndex *>
  GetTableIndexes(const std::string &table_name) const;

  // 获取表的索引列
  std::vector<std::string>
  GetIndexedColumns(const std::string &table_name) const;
  std::vector<std::vector<std::string>>
  GetCompositeIndexedColumns(const std::string &table_name) const;

  // 索引名称生成
  std::string GetIndexName(const std::string &table_name,
                           const std::string &column_name) const;
  std::string GetCompositeIndexName(const std::string &table_name,
                                   const std::vector<std::string> &columns) const;

private:
  std::shared_ptr<StorageEngine> storage_engine_; // 存储引擎智能指针
  std::unordered_map<std::string, std::unique_ptr<BPlusTreeIndex>>
      indexes_; // 索引映射表

  // 内部方法
  void LoadAllIndexes();
};
```

## 核心组件

### 索引管理器主类

**IndexManager** 是索引管理的核心类，提供了索引生命周期管理的所有接口。

### 索引数据结构

**BPlusTreeIndex** 是具体的索引实现，基于B+树数据结构，负责实际的索引操作。

### 索引元数据

索引元数据描述了索引的基本信息，包括索引名称、表名、列名、索引类型等。

## 实现细节

### 构造函数和析构函数

```cpp
IndexManager::IndexManager(std::shared_ptr<StorageEngine> storage_engine, ConfigManager &config_manager)
    : storage_engine_(storage_engine) {
  SQLCC_LOG_INFO("Initializing IndexManager");
  LoadAllIndexes();
}

IndexManager::~IndexManager() {
  SQLCC_LOG_INFO("Destroying IndexManager");
  // 索引对象会通过unique_ptr自动清理
  indexes_.clear();
}
```

**设计决策**：
- 使用智能指针管理StorageEngine，确保资源安全共享
- 在构造函数中加载所有索引，确保系统启动时索引可用
- 使用unique_ptr自动管理索引对象的生命周期，避免内存泄漏

### 索引创建

```cpp
bool IndexManager::CreateIndex(const std::string &index_name, 
                               const std::string &table_name, 
                               const std::string &column_name, bool unique) {
  // 检查索引是否已存在
  if (IndexExists(index_name, table_name)) {
    SQLCC_LOG_WARN("Index already exists: " + index_name);
    return false;
  }

  // 创建新的B+树索引
  auto index = std::make_unique<BPlusTreeIndex>(storage_engine_, table_name, column_name);
  if (!index->Create()) {
    SQLCC_LOG_ERROR("Failed to create index: " + index_name);
    return false;
  }

  // 将索引添加到索引映射表
  indexes_[index_name] = std::move(index);
  return true;
}
```

**设计决策**：
- 创建索引前检查是否已存在，避免重复创建
- 使用B+树作为默认索引数据结构，保证查询性能
- 将创建的索引添加到映射表中，方便后续查找

### 索引删除

```cpp
bool IndexManager::DropIndex(const std::string &index_name, const std::string &table_name) {
  // 检查索引是否存在
  if (!IndexExists(index_name, table_name)) {
    SQLCC_LOG_WARN("Index does not exist: " + index_name);
    return false;
  }

  // 从索引映射表中移除索引
  indexes_.erase(index_name);
  return true;
}
```

**设计决策**：
- 删除前检查索引是否存在，避免错误
- 直接从映射表中移除索引，unique_ptr会自动释放资源

### 索引查找

```cpp
BPlusTreeIndex *IndexManager::GetIndex(const std::string &index_name, const std::string &table_name) {
  (void)table_name; // 目前未使用表名参数
  auto it = indexes_.find(index_name);
  if (it != indexes_.end()) {
    return it->second.get();
  }
  return nullptr;
}
```

**设计决策**：
- 使用哈希表实现O(1)时间复杂度的索引查找
- 返回裸指针，避免智能指针的所有权问题
- 目前未使用表名参数，为未来扩展预留

### 索引名称生成

```cpp
std::string IndexManager::GetIndexName(const std::string &table_name, const std::string &column_name) const {
  return table_name + "_" + column_name + "_idx";
}

std::string IndexManager::GetCompositeIndexName(const std::string &table_name, const std::vector<std::string> &columns) const {
  std::string name = table_name + "_composite_";
  for (size_t i = 0; i < columns.size(); ++i) {
    if (i > 0) name += "_";
    name += columns[i];
  }
  name += "_idx";
  return name;
}
```

**设计决策**：
- 使用标准化的命名规则，便于索引管理和识别
- 单列索引命名格式：`表名_列名_idx`
- 复合索引命名格式：`表名_composite_列1_列2_..._idx`

### 加载所有索引

```cpp
void IndexManager::LoadAllIndexes() {
  SQLCC_LOG_INFO("Loading all indexes from storage");
  // 基本实现：从存储加载索引元数据
  // 目前暂时只记录日志，不实际加载索引
}
```

**设计决策**：
- 提供了索引加载接口，为未来扩展预留
- 当前实现简化，避免系统初始化时卡住
- 实际实现中应从系统表或元数据文件加载索引定义

## 性能优化

### 智能指针管理
- 使用unique_ptr自动管理索引对象的生命周期，避免内存泄漏
- 使用shared_ptr共享StorageEngine资源，提高资源利用率

### 索引缓存
- 所有索引对象存储在内存中，避免频繁的磁盘I/O
- 哈希表实现快速索引查找

### 日志记录
- 详细记录索引操作，便于调试和性能分析

## 扩展点

### 复合索引增强
当前复合索引是简化实现，未来可以增强为真正的复合索引支持。

### 并发控制
目前实现没有显式的并发控制机制，未来可以添加锁机制支持多线程访问。

### 索引统计信息
可以添加索引使用情况和性能统计，为查询优化器提供依据。

### 索引类型扩展
当前只支持B+树索引，未来可以扩展支持其他索引类型（如哈希索引、全文索引等）。

## 错误处理

- 使用日志记录错误信息
- 返回布尔值表示操作结果
- 错误发生时提供详细的错误消息

## 测试支持

- 索引创建和删除测试
- 索引查找测试
- 复合索引测试
- 索引名称生成测试

## 总结

IndexManager类提供了完整的数据库索引生命周期管理功能，是SQLCC数据库系统中连接查询执行器和底层索引数据结构的重要组件。它使用智能指针管理资源，支持单列索引和复合索引，提供了标准化的索引命名规则，并为未来扩展预留了接口。