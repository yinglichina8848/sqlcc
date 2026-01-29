# 存储引擎组件

## 概述

存储引擎是SQLCC数据库管理系统的核心组件，负责数据的持久化存储和高效访问。它提供了页面式存储架构，支持定长和变长记录的存储，以及高效的页面缓存和管理机制。

## 核心类

1. [StorageEngine](storage_engine.md) - 存储引擎主类，协调磁盘管理器和缓冲池
2. [DiskManager](disk_manager.md) - 磁盘管理器，负责磁盘I/O操作
3. [BufferPool](buffer_pool.md) - 缓冲池，负责页面缓存和替换策略
4. [Page](page.md) - 页面类，表示内存中的数据页

## 组件关系

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

## 主要功能

1. **页面管理**：创建、获取、释放和刷新页面
2. **缓冲池管理**：页面缓存、替换策略（LRU）、并发控制
3. **磁盘I/O操作**：页面的读写、批量操作、预取等
4. **事务支持**：脏页管理、页面固定/取消固定
5. **性能监控**：统计信息收集和报告