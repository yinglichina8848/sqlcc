# SQLCC存储引擎性能测试框架设计文档

## 概述

SQLCC存储引擎性能测试框架是一个全面、可扩展的测试系统，旨在评估存储引擎在不同工作负载和配置下的性能表现。该框架采用模块化设计，提供了统一的测试接口和结果收集机制，支持多种性能指标的测量和分析。

## 架构设计

### 1. 核心架构

性能测试框架采用分层架构设计，主要包括以下层次：

```
┌─────────────────────────────────────────────────────┐
│                  测试运行层                           │
│             PerformanceTestRunner                   │
├─────────────────────────────────────────────────────┤
│                  测试实现层                           │
│  BufferPoolTest │ DiskIOTest │ MixedWorkloadTest    │
├─────────────────────────────────────────────────────┤
│                  基础框架层                           │
│            PerformanceTestBase                      │
├─────────────────────────────────────────────────────┤
│                  结果收集层                           │
│              TestResult & Metrics                   │
└─────────────────────────────────────────────────────┘
```

### 2. 组件说明

#### 2.1 PerformanceTestBase (基础框架层)

`PerformanceTestBase`是所有性能测试的基类，提供了通用的测试基础设施：

**主要功能：**
- 时间测量和计算
- 吞吐量计算
- 延迟统计（平均值、P95、P99）
- 结果格式化和输出
- CSV文件保存
- 报告生成

**核心方法：**
- `GetCurrentTime()`: 获取当前高精度时间
- `CalculateDuration()`: 计算操作持续时间
- `CalculateThroughput()`: 计算吞吐量（操作/秒）
- `CalculateLatencies()`: 计算延迟统计指标
- `PrintResult()`: 打印测试结果
- `SaveResultsToFile()`: 保存结果到CSV文件
- `GenerateReport()`: 生成性能报告

**数据结构：**
```cpp
struct TestResult {
    std::string test_name;          // 测试名称
    double duration_ms;             // 持续时间(毫秒)
    size_t operation_count;         // 操作总数
    double throughput_ops_per_sec;  // 吞吐量(操作/秒)
    double avg_latency_ms;          // 平均延迟(毫秒)
    double p95_latency_ms;          // P95延迟(毫秒)
    double p99_latency_ms;          // P99延迟(毫秒)
    std::map<std::string, std::string> custom_metrics;  // 自定义指标
};
```

#### 2.2 PerformanceTestRunner (测试运行层)

`PerformanceTestRunner`负责协调和执行所有性能测试：

**主要功能：**
- 命令行参数解析
- 测试选择和调度
- 输出目录管理
- 测试流程控制

**支持的命令行选项：**
- `-b, --buffer-pool`: 运行缓冲池性能测试
- `-d, --disk-io`: 运行磁盘I/O性能测试
- `-m, --mixed-workload`: 运行混合工作负载测试
- `-a, --all`: 运行所有测试（默认）
- `-v, --verbose`: 启用详细输出
- `-o, --output-dir <dir>`: 指定结果输出目录

#### 2.3 具体测试实现类 (测试实现层)

框架包含三个主要的测试实现类，每个类都继承自`PerformanceTestBase`：

1. **BufferPoolPerformanceTest**: 缓冲池性能测试
2. **DiskIOPerformanceTest**: 磁盘I/O性能测试
3. **MixedWorkloadTest**: 混合工作负载性能测试

## 测试模块详细设计

### 1. 缓冲池性能测试 (BufferPoolPerformanceTest)

#### 1.1 测试目标
评估缓冲池在不同配置和工作负载下的性能表现，包括缓存命中率、LRU效率和扩展性。

#### 1.2 测试场景

**缓存命中率测试**
- 测试不同工作集大小下的缓存命中率
- 工作集大小范围：50, 100, 200, 500, 1000
- 固定缓冲池大小：256页
- 访问模式：具有局部性的随机访问

**LRU效率测试**
- 测试LRU替换策略的效率
- 工作集大小：50, 100, 200, 500, 1000
- 固定缓冲池大小：256页
- 访问模式：具有时间局部性的访问序列

**访问模式测试**
- 顺序访问模式
- 随机访问模式
- 具有局部性的访问模式
- 缓冲池大小：256页
- 工作集大小：1000页

**缓冲池大小扩展性测试**
- 缓冲池大小：32, 64, 128, 256, 512, 1024, 2048页
- 固定工作集大小：1000页
- 访问模式：具有局部性的随机访问

#### 1.3 关键方法

- `SetupBufferPool(size_t pool_size)`: 设置指定大小的缓冲池
- `GenerateSequentialAccess()`: 生成顺序访问序列
- `GenerateRandomAccess()`: 生成随机访问序列
- `GenerateLocalityAccess()`: 生成具有局部性的访问序列
- `ExecutePageAccesses()`: 执行页面访问操作并记录延迟
- `SimulatePageAccess()`: 模拟页面访问

### 2. 磁盘I/O性能测试 (DiskIOPerformanceTest)

#### 2.1 测试目标
评估磁盘管理器的I/O性能，包括顺序读写、随机读写、不同页面大小和并发I/O的性能表现。

#### 2.2 测试场景

**顺序读写测试**
- 顺序读取和写入操作
- 页面大小：4KB, 8KB, 16KB
- 页面数量：1000页
- 测试文件大小：40MB

**随机读写测试**
- 随机读取和写入操作
- 页面大小：4KB, 8KB, 16KB
- 页面数量：1000页
- 测试文件大小：40MB

**不同页面大小测试**
- 页面大小：4KB, 8KB, 16KB
- 固定总数据量：40MB
- 混合读写比例：70%读，30%写

**并发I/O测试**
- 并发线程数：1, 2, 4, 8
- 页面大小：8KB
- 页面数量：1000页
- 每线程操作数：250

#### 2.3 关键方法

- `PrepareTestFile()`: 准备测试文件
- `CleanupTestFile()`: 清理测试文件
- `SimulatePageRead()`: 模拟页面读取操作
- `SimulatePageWrite()`: 模拟页面写入操作
- `ExecuteSequentialReads()`: 执行顺序读取操作
- `ExecuteSequentialWrites()`: 执行顺序写入操作
- `ExecuteRandomReads()`: 执行随机读取操作
- `ExecuteRandomWrites()`: 执行随机写入操作

### 3. 混合工作负载测试 (MixedWorkloadTest)

#### 3.1 测试目标
模拟真实数据库工作负载，评估系统在混合读写操作、不同事务大小和长时间运行下的整体性能表现。

#### 3.2 测试场景

**读写比例测试**
- 读密集型：90%读，10%写
- 读写均衡：70%读，30%写
- 均衡型：50%读，50%写
- 写密集型：30%读，70%写
- 固定事务大小：10页
- 线程数：4
- 工作集大小：1000页

**事务大小测试**
- 事务大小：1, 5, 10, 20页
- 读写比例：70%读，30%写
- 线程数：4
- 工作集大小：1000页

**长时间稳定性测试**
- 持续时间：1分钟，5分钟
- 读写比例：70%读，30%写
- 事务大小：10页
- 线程数：4
- 工作集大小：1000页

**并发工作负载测试**
- 线程数：1, 2, 4, 8, 16
- 读写比例：70%读，30%写
- 事务大小：10页
- 工作集大小：1000页

#### 3.3 关键数据结构和方法

**WorkloadConfig结构**：
```cpp
struct WorkloadConfig {
    double read_ratio;          // 读操作比例 (0.0-1.0)
    double write_ratio;         // 写操作比例 (0.0-1.0)
    double create_ratio;        // 创建操作比例 (0.0-1.0)
    double delete_ratio;        // 删除操作比例 (0.0-1.0)
    size_t transaction_size;    // 事务大小（页面数）
    size_t duration_ms;         // 测试持续时间（毫秒）
    size_t thread_count;        // 线程数
    size_t working_set_size;    // 工作集大小（页面数）
    std::string name;           // 测试名称
};
```

**关键方法**：
- `ExecuteWorkload()`: 执行工作负载
- `WorkerThread()`: 工作线程函数
- `GenerateOperationSequence()`: 生成操作序列
- `ExecuteOperation()`: 执行单个操作
- `SimulatePageRead()`: 模拟页面读取操作
- `SimulatePageWrite()`: 模拟页面写入操作
- `SimulatePageCreate()`: 模拟页面创建操作
- `SimulatePageDelete()`: 模拟页面删除操作

## 结果收集与分析

### 1. 结果格式

所有测试结果以统一的CSV格式保存，包含以下字段：
- TestName: 测试名称
- DurationMs: 持续时间(毫秒)
- OperationCount: 操作总数
- ThroughputOpsPerSec: 吞吐量(操作/秒)
- AvgLatencyMs: 平均延迟(毫秒)
- P95LatencyMs: P95延迟(毫秒)
- P99LatencyMs: P99延迟(毫秒)
- 自定义指标：根据测试类型而定

### 2. 自定义指标

不同测试类型收集特定的自定义指标：

**缓冲池测试**：
- HitRate: 缓存命中率
- PoolSize: 缓冲池大小
- WorkingSetSize: 工作集大小

**磁盘I/O测试**：
- PageSize: 页面大小
- ThroughputMBPerSec: 吞吐量(MB/秒)
- ReadRatio: 读取比例
- ThreadCount: 线程数

**混合工作负载测试**：
- ReadRatio: 读取比例
- WriteRatio: 写入比例
- TransactionSize: 事务大小
- ThreadCount: 线程数
- WorkingSetSize: 工作集大小

### 3. 结果分析

框架提供基本的结果分析功能，包括：
- 延迟分布统计
- 吞吐量趋势分析
- 命中率计算
- 性能对比

## 扩展性设计

### 1. 添加新测试类型

要添加新的测试类型，需要：

1. 继承`PerformanceTestBase`类
2. 实现`RunAllTests()`和`Cleanup()`纯虚函数
3. 添加测试特定的方法和数据成员
4. 在`PerformanceTestRunner`中注册新测试类型

### 2. 添加新指标

要添加新的性能指标，需要：

1. 在`TestResult`结构中添加新字段
2. 在相应的测试类中计算和设置新指标
3. 更新CSV输出格式以包含新指标

### 3. 自定义报告格式

可以通过重写`GenerateReport()`方法来实现自定义报告格式。

## 使用指南

### 1. 编译和运行

```bash
# 编译性能测试
make performance_test

# 运行所有测试
./performance_test

# 运行特定测试
./performance_test -b  # 缓冲池测试
./performance_test -d  # 磁盘I/O测试
./performance_test -m  # 混合工作负载测试

# 指定输出目录
./performance_test -o /path/to/results

# 启用详细输出
./performance_test -v
```

### 2. 结果查看

测试结果保存在指定的输出目录中，包括：
- CSV格式的原始数据
- 文本格式的测试报告
- 可选的图表和可视化结果

### 3. 自定义测试参数

可以通过修改测试类中的参数来调整测试行为，例如：
- 缓冲池大小范围
- 页面大小选项
- 工作负载配置
- 测试持续时间

## 总结

SQLCC存储引擎性能测试框架提供了一个全面、可扩展的测试平台，能够评估存储引擎在各种场景下的性能表现。框架采用模块化设计，易于扩展和维护，为存储引擎的性能优化提供了有力的支持。通过该框架，开发者可以快速识别性能瓶颈，验证优化效果，并确保存储引擎满足性能要求。