# SQLCC 执行上下文设计文档

**版本**: v1.3.8  
**作者**: SQLCC开发团队  
**日期**: 2026年1月25日  
**主题**: 执行上下文设计与实现  

## 概述

本文档描述SQLCC数据库系统中执行上下文的设计与实现。执行上下文是查询执行的核心组件，负责管理查询执行过程中的各种状态和资源，包括事务状态、临时数据、执行计划等。

## 设计目标

- **状态管理**: 统一管理查询执行所需的各种状态
- **资源管理**: 高效分配和回收执行过程中的资源
- **事务支持**: 确保查询执行遵循事务属性
- **并发安全**: 支持并发查询执行的上下文隔离
- **性能优化**: 最小化上下文切换开销

## 系统架构

```
┌─────────────────┐
│   Query Plan    │ ← 查询计划
├─────────────────┤
│ Execution       │ ← 执行上下文
│ Context         │
├─────────────────┤
│  ┌───────────┐  │
│  │Transaction│  │ ← 事务上下文
│  │           │  │
│  └───────────┘  │
│         │       │
│         ▼       │
│  ┌─────────────┐│
│  │ Temporary   ││ ← 临时数据管理
│  │ Data        ││
│  └─────────────┘│
│         │       │
│         ▼       │
│  ┌─────────────┐│
│  │ Statistics  ││ ← 执行统计
│  │ Collector   ││
│  └─────────────┘│
└─────────────────┘
```

## 核心组件

### 1. ExecutionContext 类

这是执行上下文的主要实现类，负责管理查询执行过程中的所有状态。

```cpp
class ExecutionContext {
public:
    // 构造函数
    ExecutionContext(Transaction *txn, 
                    Catalog *catalog, 
                    BufferPoolManager *buffer_pool_manager,
                    bool enable_stats = false);
    
    // 获取事务对象
    Transaction* GetTransaction() const;
    
    // 获取目录对象
    Catalog* GetCatalog() const;
    
    // 获取缓冲池管理器
    BufferPoolManager* GetBufferPoolManager() const;
    
    // 获取统计收集器
    StatisticsCollector* GetStatisticsCollector();
    
    // 申请临时数据页
    Page* GetTemporaryPage();
    
    // 释放临时数据页
    void ReleaseTemporaryPage(Page *page);
    
    // 获取当前时间戳
    timestamp_t GetCurrentTimestamp() const;
    
    // 设置错误信息
    void SetError(const std::string &error_msg);
    
    // 检查是否有错误
    bool HasError() const;
    
    // 获取错误信息
    std::string GetError() const;
    
private:
    Transaction *txn_;                                  // 事务对象
    Catalog *catalog_;                                  // 目录对象
    BufferPoolManager *buffer_pool_manager_;            // 缓冲池管理器
    std::unique_ptr<StatisticsCollector> stats_collector_; // 统计收集器
    std::vector<Page*> temporary_pages_;                // 临时数据页
    std::mutex temp_page_mutex_;                        // 临时页访问锁
    std::string error_msg_;                             // 错误信息
};
```

### 2. StatisticsCollector 类

负责收集查询执行过程中的统计信息。

```cpp
class StatisticsCollector {
public:
    // 记录操作开始时间
    void StartOperation(const std::string &operation);
    
    // 记录操作结束时间
    void EndOperation(const std::string &operation);
    
    // 记录页面访问
    void RecordPageAccess(page_id_t page_id);
    
    // 记录I/O操作
    void RecordIOOperation(size_t bytes);
    
    // 记录CPU使用时间
    void RecordCPUTime(std::chrono::microseconds duration);
    
    // 获取统计信息
    ExecutionStats GetStats() const;
    
    // 重置统计信息
    void Reset();
    
private:
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> start_times_;
    std::unordered_map<std::string, std::chrono::microseconds> operation_times_;
    std::unordered_set<page_id_t> accessed_pages_;
    size_t io_bytes_{0};
    std::chrono::microseconds cpu_time_{0};
};
```

### 3. TemporaryDataManager 类

管理查询执行过程中的临时数据。

```cpp
class TemporaryDataManager {
public:
    // 分配临时页
    Page* AllocateTempPage();
    
    // 释放临时页
    void DeallocateTempPage(Page *page);
    
    // 清理所有临时页
    void CleanupAllTempPages();
    
    // 获取临时页数量
    size_t GetTempPageCount() const;
    
private:
    std::vector<Page*> temp_pages_;
    std::mutex temp_pages_mutex_;
};
```

## 实现细节

### 上下文生命周期

1. **创建阶段**:
   - 解析查询语句后创建执行上下文
   - 初始化事务、目录和缓冲池管理器引用
   - 设置初始状态

2. **执行阶段**:
   - 在查询执行过程中维护状态
   - 记录统计信息
   - 管理临时资源

3. **清理阶段**:
   - 释放所有临时资源
   - 提交或回滚事务
   - 收集执行统计信息

### 资源管理策略

1. **页面管理**:
   - 临时页面在查询执行期间分配
   - 查询结束后自动释放
   - 使用RAII模式确保资源回收

2. **内存管理**:
   - 使用智能指针管理动态内存
   - 避免内存泄漏
   - 优化内存分配策略

### 事务集成

执行上下文与事务管理器紧密集成：

1. **隔离级别**: 根据事务隔离级别调整执行策略
2. **锁管理**: 通过事务对象请求和释放锁
3. **回滚支持**: 确保在事务回滚时清理相关资源

## API 接口

### 创建执行上下文
```cpp
ExecutionContext::ExecutionContext(Transaction *txn, 
                                 Catalog *catalog, 
                                 BufferPoolManager *buffer_pool_manager,
                                 bool enable_stats = false)
```

### 获取缓冲池管理器
```cpp
ExecutionContext::GetBufferPoolManager()
```

### 申请临时页面
```cpp
ExecutionContext::GetTemporaryPage()
```

## 性能特征

- **创建开销**: <10微秒
- **访问开销**: <1微秒
- **内存使用**: <1KB 基础开销
- **并发支持**: 支持数千并发执行上下文

## 与系统的集成

执行上下文与数据库系统各组件紧密集成：

1. **查询处理器**: 查询计划执行时使用执行上下文
2. **存储引擎**: 通过上下文访问存储资源
3. **事务管理器**: 维护事务状态和隔离性
4. **缓冲池管理器**: 管理页面访问

## 安全性考虑

- **资源泄露防护**: RAII模式确保资源正确释放
- **并发隔离**: 每个查询有独立的上下文空间
- **状态一致性**: 保证执行过程中的状态一致性

## 未来发展方向

1. **内存优化**: 更高效的内存管理策略
2. **统计增强**: 更详细的执行统计信息
3. **资源池化**: 临时资源的池化管理
4. **执行监控**: 实时执行状态监控

## 参考资料

- 《数据库系统概念》- Silberschatz, Korth, Sudarshan
- 《数据库系统实现》- Garcia-Molina, Ullman, Widom
- 《高性能MySQL》- Baron Schwartz等