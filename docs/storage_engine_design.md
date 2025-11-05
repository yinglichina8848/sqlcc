# 存储引擎设计与实现文档

## 概述

本文档详细描述了SQLCC项目中存储引擎的设计和实现。我们的存储引擎采用页式存储架构，支持磁盘I/O、缓冲池管理和LRU替换策略。

## 系统架构

### 核心组件

1. **DiskManager** - 负责磁盘I/O操作
2. **BufferPool** - 负责页面缓存和替换策略
3. **Page** - 表示内存中的数据页
4. **StorageEngine** - 整合所有组件，提供统一接口

### 组件关系图

```
StorageEngine
    |
    |-- BufferPool
    |   |-- Page Table (page_id -> Page)
    |   |-- LRU List
    |   |-- Page References
    |   `-- Dirty Pages
    |
    `-- DiskManager
        |-- File I/O
        `-- Page Allocation
```

## 详细设计

### 1. DiskManager类

**职责**：
- 管理数据库文件的读写操作
- 分配和释放页面空间
- 维护页面分配状态

**关键方法**：
- `ReadPage(page_id, page)`: 从磁盘读取页面数据
- `WritePage(page_id, page)`: 将页面数据写入磁盘
- `AllocatePage()`: 分配新页面并返回页面ID
- `DeallocatePage(page_id)`: 释放页面空间

**实现细节**：
- 使用标准文件I/O操作
- 页面大小固定为8KB (PAGE_SIZE = 8192字节)
- 使用位图管理页面分配状态

### 2. BufferPool类

**职责**：
- 管理内存中的页面缓存
- 实现LRU替换策略
- 处理页面引用计数
- 管理脏页标记

**关键数据结构**：
- `page_table_`: 哈希表，存储page_id到Page对象的映射
- `lru_list_`: 双向链表，实现LRU策略
- `lru_map_`: 哈希表，存储page_id到LRU链表迭代器的映射
- `page_refs_`: 哈希表，存储页面引用计数
- `dirty_pages_`: 哈希表，存储脏页标记

**关键方法**：
- `FetchPage(page_id)`: 获取页面，如不在缓冲池则从磁盘加载
- `UnpinPage(page_id, is_dirty)`: 减少页面引用计数，标记是否为脏页
- `FlushPage(page_id)`: 将指定页面写入磁盘
- `FlushAllPages()`: 将所有脏页写入磁盘
- `NewPage(page_id)`: 创建新页面
- `DeletePage(page_id)`: 从缓冲池中删除页面

**LRU替换策略**：
- 当缓冲池满且需要加载新页面时，从LRU链表尾部选择淘汰页面
- 只有引用计数为0的页面才能被淘汰
- 淘汰前，如果页面是脏页，需要先写入磁盘

### 3. Page类

**职责**：
- 表示内存中的数据页
- 提供数据读写接口
- 管理页面元数据

**关键属性**：
- `page_id_`: 页面ID
- `data_`: 页面数据缓冲区
- `is_dirty_`: 脏页标记

**关键方法**：
- `GetData()`: 获取页面数据指针
- `GetPageId()`: 获取页面ID
- `IsDirty()`: 检查是否为脏页
- `WriteData(offset, data, size)`: 向页面写入数据
- `ReadData(offset, data, size)`: 从页面读取数据

### 4. StorageEngine类

**职责**：
- 整合DiskManager和BufferPool
- 提供统一的存储引擎接口
- 管理系统生命周期

**关键方法**：
- `FetchPage(page_id)`: 获取页面
- `UnpinPage(page_id, is_dirty)`: 取消固定页面
- `FlushPage(page_id)`: 刷新页面到磁盘
- `FlushAllPages()`: 刷新所有页面到磁盘
- `NewPage(page_id)`: 创建新页面
- `DeletePage(page_id)`: 删除页面

## 关键实现细节

### 页面生命周期

1. **创建页面**：
   - 调用`DiskManager::AllocatePage()`分配页面ID
   - 调用`BufferPool::NewPage()`创建页面对象
   - 初始化页面数据

2. **访问页面**：
   - 调用`StorageEngine::FetchPage(page_id)`
   - 如果页面不在缓冲池，从磁盘加载
   - 增加页面引用计数
   - 更新LRU链表

3. **修改页面**：
   - 获取页面数据指针
   - 修改数据
   - 调用`UnpinPage(page_id, true)`标记为脏页

4. **删除页面**：
   - 调用`StorageEngine::DeletePage(page_id)`
   - 从缓冲池中移除页面
   - 调用`DiskManager::DeallocatePage(page_id)`释放空间

### 并发控制

当前实现为单线程版本，未考虑并发访问。未来版本可以添加：
- 读写锁保护缓冲池数据结构
- 页面级锁支持并发访问
- 事务支持

### 错误处理

- 磁盘I/O错误：返回错误码，记录日志
- 内存不足：抛出异常，记录日志
- 页面不存在：返回nullptr，记录警告

## 测试

我们实现了全面的单元测试，覆盖以下场景：

1. **初始化测试**：验证存储引擎正确初始化
2. **新页面创建测试**：验证新页面创建和初始化
3. **页面获取测试**：验证页面获取和引用计数
4. **页面刷新测试**：验证脏页刷新到磁盘
5. **全量页面刷新测试**：验证所有脏页刷新
6. **大量页面操作测试**：验证大量页面操作的性能和正确性
7. **页面删除测试**：验证页面删除和后续访问返回nullptr

## 性能考虑

1. **缓冲池大小**：可配置，默认为64页
2. **LRU策略**：选择最久未使用的页面淘汰
3. **批量刷新**：支持批量刷新所有脏页
4. **预取**：未来可添加页面预取机制

## 未来改进

1. **并发支持**：添加多线程支持和锁机制
2. **事务支持**：实现ACID特性
3. **索引支持**：添加B+树索引结构
4. **压缩支持**：添加页面压缩功能
5. **预取机制**：实现智能页面预取

## 总结

我们的存储引擎实现了基本的页式存储功能，包括磁盘管理、缓冲池和LRU替换策略。通过合理的架构设计和全面的测试，确保了系统的稳定性和正确性。未来可以在当前基础上添加更多高级功能，如并发控制、事务支持和索引结构。