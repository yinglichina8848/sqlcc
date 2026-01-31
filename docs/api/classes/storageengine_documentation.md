# StorageEngine 类文档

## 类概述

`StorageEngine` 是 SQLCC 存储子系统的顶层接口，封装了磁盘管理、缓冲池管理和索引管理的核心功能。

## WHY: 为什么需要统一的存储引擎接口？

**设计动机**：数据库系统需要一个统一的接口来管理数据的存储和访问，隐藏底层实现的复杂性：

1. **抽象底层细节**：上层应用无需关心数据如何物理存储
2. **统一资源管理**：集中管理页面、索引、日志等资源
3. **性能优化**：协调各组件实现最优性能
4. **故障恢复**：提供一致的数据恢复机制
5. **扩展性**：支持不同的存储后端实现

**核心价值**：
- 页面生命周期管理（创建、读取、写入、删除）
- 缓冲池协调（内存缓存与磁盘 I/O）
- 索引协调（B+树索引管理）
- 事务支持（与 TransactionManager 协作）

## WHAT: 核心功能

### 页面管理
| 方法 | 功能描述 |
|------|----------|
| `NewPage()` | 创建新页面，分配唯一页面ID |
| `FetchPage()` | 根据页面ID获取页面 |
| `UnpinPage()` | 释放页面固定，允许替换 |
| `FlushPage()` | 将页面写入磁盘 |
| `DeletePage()` | 删除页面 |
| `FlushAllPages()` | 刷新所有脏页 |

### 组件访问
| 方法 | 功能描述 |
|------|----------|
| `GetDiskManager()` | 获取磁盘管理器指针 |
| `GetBufferPool()` | 获取缓冲池指针 |
| `GetIndexManager()` | 获取索引管理器指针 |
| `GetStats()` | 获取数据库统计信息 |

### 初始化
| 方法 | 功能描述 |
|------|----------|
| `InitializeIndexManager()` | 延迟初始化索引管理器 |

## HOW: 实现机制

### 架构设计

```
┌─────────────────────────────────────────────────────────┐
│                    StorageEngine                         │
│  (enable_shared_from_this, 支持安全shared_ptr创建)       │
├─────────────────────────────────────────────────────────┤
│ 组合组件:                                                 │
│  ├── DiskManager (磁盘I/O管理)                          │
│  ├── BufferPoolSharded (分片缓冲池)                      │
│  └── IndexManager (索引管理)                            │
└─────────────────────────────────────────────────────────┘
```

### 页面操作流程

**NewPage 流程**：
1. 通过 DiskManager 分配新的页面ID
2. 通过 BufferPool 创建页面对象
3. 将页面标记为脏页
4. 返回页面智能指针

**FetchPage 流程**：
1. 通过 BufferPool 获取页面
2. 如果页面不在内存中，从磁盘加载
3. 返回页面共享指针

**FlushPage 流程**：
1. 调用 BufferPool 的 FlushPage 方法
2. BufferPool 协调与 DiskManager 的交互
3. 确保数据写入磁盘

### 资源管理

- **智能指针管理**：使用 `std::unique_ptr` 管理组件生命周期
- **RAII 模式**：析构函数自动刷新所有脏页
- **禁止拷贝**：删除拷贝构造函数和赋值操作符
- **延迟初始化**：IndexManager 在首次访问时初始化

## 使用示例

```cpp
#include "storage_engine/storage_engine.h"

// 初始化存储引擎
ConfigManager config_manager;
config_manager.loadConfig("sqlcc.conf");

StorageEngine storage_engine(config_manager, "./data");

// 创建新页面
int32_t page_id;
auto page = storage_engine.NewPage(&page_id);

// 使用页面
if (page) {
    // 写入数据
    page->SetData(0, "Hello, SQLCC!", 13);

    // 页面使用完毕，释放固定
    storage_engine.UnpinPage(page_id, true);  // 标记为脏页
}

// 刷新页面到磁盘
storage_engine.FlushPage(page_id);

// 获取统计信息
std::string stats = storage_engine.GetStats();
std::cout << stats << std::endl;

// 关闭时自动刷新所有脏页
storage_engine.FlushAllPages();
```

## 与其他组件的交互

### 与 TransactionManager 交互
- 事务获取锁后访问页面
- 脏页在事务提交时刷新
- 支持 MVCC 的版本管理

### 与 UnifiedExecutor 交互
- 执行器调用存储引擎执行 I/O 操作
- 索引操作通过 IndexManager 执行
- 查询优化器使用统计信息

## 性能优化建议

1. **页面大小**：根据工作负载调整页面大小
2. **缓冲池大小**：确保足够容纳热点数据
3. **预读策略**：根据访问模式启用预读
4. **批量刷新**：减少频繁的小 I/O 操作

## 版本信息

- **版本**: v1.3.9
- **最后更新**: 2026-01-31
- **C++标准**: C++20
- **编译器**: Clang 18+