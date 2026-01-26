# 索引系统API使用示例

## B+树索引操作

```cpp
#include "index/b_plus_tree.h"

// 创建B+树索引
auto index = std::make_shared<BPlusTreeIndex<int, RID>>("idx_users_age", 4096);

// 插入索引项
index->Insert(25, RID{1, 100});
index->Insert(30, RID{2, 200});

// 查找操作
std::vector<RID> results;
index->GetValue(25, results);

// 范围查询
auto range_results = index->Range(20, 35);
```

## 复合索引

```cpp
// 创建复合索引
auto composite_idx = std::make_shared<CompositeIndex>("idx_users_name_age");
composite_idx->AddField("name", DataType::VARCHAR);
composite_idx->AddField("age", DataType::INTEGER);

// 使用复合索引
composite_idx->Insert({"John", 25}, RID{1, 100});
```
