# 存储引擎API使用示例

## 基本用法

```cpp
#include "storage_engine/storage_engine.h"

// 初始化存储引擎
auto storage = std::make_shared<StorageEngine>();
storage->Initialize();

// 创建表
TableSchema schema;
schema.SetName("users");
// ... 设置表结构 ...

storage->CreateTable(schema);

// 插入数据
Record record;
// ... 设置记录数据 ...
storage->Insert("users", record);
```

## 高级用法

```cpp
// 批量操作
std::vector<Record> records;
// ... 填充记录 ...
storage->BatchInsert("users", records);

// 查询操作
Query query;
query.SetTable("users");
query.AddCondition("age > 18");

auto results = storage->Select(query);
```
