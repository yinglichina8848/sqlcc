# IndexManager 类文档

## 类概述

`IndexManager` 是 SQLCC 的索引管理器，负责管理数据库中的所有索引，包括单列索引、复合索引的创建、删除和查询。

## WHY: 为什么需要索引管理器？

**设计动机**：索引是数据库查询性能优化的核心手段：

1. **查询加速**：避免全表扫描，快速定位数据
2. **唯一性约束**：通过唯一索引保证数据唯一性
3. **排序优化**：索引天然有序，优化 ORDER BY
4. **范围查询**：B+树结构高效支持范围查询
5. **多列支持**：复合索引支持多列组合查询

**核心价值**：
- 集中管理所有索引
- 协调存储引擎和 B+树
- 索引生命周期管理
- 索引元数据维护

## WHAT: 核心功能

### 索引管理
| 方法 | 功能描述 |
|------|----------|
| `CreateIndex()` | 创建单列索引 |
| `CreateCompositeIndex()` | 创建复合索引 |
| `DropIndex()` | 删除索引 |
| `IndexExists()` | 检查索引是否存在 |

### 索引查询
| 方法 | 功能描述 |
|------|----------|
| `GetIndex()` | 获取指定索引 |
| `GetTableIndexes()` | 获取表的所有索引 |
| `GetIndexedColumns()` | 获取表的索引列 |
| `GetCompositeIndexedColumns()` | 获取表的复合索引列 |

### 辅助功能
| 方法 | 功能描述 |
|------|----------|
| `GetIndexName()` | 生成索引名称 |
| `GetCompositeIndexName()` | 生成复合索引名称 |

## HOW: 实现机制

### 索引存储结构

```
IndexManager
├── indexes_ (unordered_map)
│   ├── "idx_users_email" → BPlusTreeIndex
│   ├── "idx_orders_customer" → BPlusTreeIndex
│   └── "idx_orders_composite" → BPlusTreeIndex
└── storage_engine_ (共享指针)
```

### 索引创建流程

1. **验证**：检查表和列是否存在
2. **生成名称**：自动生成索引名称
3. **创建 B+树**：初始化 BPlusTreeIndex
4. **初始化**：加载现有索引数据
5. **注册**：添加到 indexes_ 映射表

### 索引类型

| 类型 | 说明 | 使用场景 |
|------|------|----------|
| 单列索引 | 单个列的索引 | 等值查询、范围查询 |
| 复合索引 | 多个列的组合索引 | 多列等值查询、前缀匹配 |
| 唯一索引 | 约束列值唯一 | 主键、唯一约束 |

## 使用示例

```cpp
#include "storage_engine/index_manager.h"

// 创建索引管理器
IndexManager index_manager(storage_engine, config_manager);

// 创建单列索引
index_manager.CreateIndex("idx_users_email", "users", "email", true);

// 创建复合索引
index_manager.CreateCompositeIndex("idx_orders_composite", "orders",
                                   {"customer_id", "order_date"});

// 检查索引是否存在
if (index_manager.IndexExists("idx_users_email", "users")) {
    auto index = index_manager.GetIndex("idx_users_email", "users");
    // 使用索引进行查询
}

// 获取表的所有索引
auto indexes = index_manager.GetTableIndexes("orders");
for (auto idx : indexes) {
    std::cout << "索引: " << idx->GetIndexName() << std::endl;
}

// 删除索引
index_manager.DropIndex("idx_users_email", "users");
```

## 索引设计建议

1. **选择高选择性的列**：选择区分度高的列建索引
2. **考虑查询模式**：根据实际查询创建索引
3. **复合索引顺序**：将选择性高的列放在前面
4. **避免过多索引**：索引会降低写入性能
5. **覆盖索引**：包含查询所需的所有列

## 性能优化

1. **索引缓存**：热点索引数据缓存到内存
2. **批量构建**：大量数据时使用批量构建
3. **异步维护**：索引维护操作异步执行
4. **监控索引使用**：分析索引使用情况优化索引

## 版本信息

- **版本**: v1.3.9
- **最后更新**: 2026-01-31
- **C++标准**: C++20
- **编译器**: Clang 18+