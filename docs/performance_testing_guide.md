# SQLCC 性能测试框架

本文档介绍SQLCC数据库存储引擎的性能测试框架，包括测试目标、使用方法和结果分析。

## 目录

- [概述](#概述)
- [测试目标](#测试目标)
- [测试类型](#测试类型)
- [构建与运行](#构建与运行)
- [结果分析](#结果分析)
- [性能基准](#性能基准)
- [持续集成](#持续集成)

## 概述

SQLCC性能测试框架是一个全面的性能测试套件，旨在评估数据库存储引擎在各种工作负载下的性能表现。该框架包含以下测试类型：

1. **缓冲池性能测试** - 评估缓冲池的缓存效率和LRU算法性能
2. **磁盘I/O性能测试** - 测试磁盘读写性能和并发I/O能力
3. **混合工作负载测试** - 模拟真实数据库工作负载，测试整体性能

## 测试目标

性能测试的主要目标是：

1. **性能基准建立** - 为SQLCC存储引擎建立性能基准，确保版本迭代不引入性能回归
2. **性能瓶颈识别** - 通过全面的测试识别系统性能瓶颈，指导优化工作
3. **性能特征分析** - 深入了解SQLCC在不同工作负载下的性能特征
4. **配置优化指导** - 为不同应用场景提供最优配置建议

## 测试类型

### 缓冲池性能测试

缓冲池性能测试评估以下方面：

- **缓存命中率测试** - 测试不同缓冲池大小下的缓存命中率
- **LRU效率测试** - 评估LRU替换算法的效率
- **访问模式测试** - 测试顺序、随机和混合访问模式下的性能
- **缓冲池大小扩展性测试** - 评估不同缓冲池大小对性能的影响

### 磁盘I/O性能测试

磁盘I/O性能测试包括：

- **顺序读写测试** - 测试顺序读写性能
- **随机读写测试** - 测试随机读写性能
- **不同页面大小测试** - 评估不同页面大小对I/O性能的影响
- **并发I/O测试** - 测试多线程并发I/O性能

### 混合工作负载测试

混合工作负载测试模拟真实数据库场景：

- **读写比例测试** - 测试不同读写比例下的性能
- **事务大小测试** - 评估不同事务大小对性能的影响
- **长时间运行测试** - 测试长时间运行的稳定性和性能
- **并发工作负载测试** - 测试多线程并发工作负载性能

## 构建与运行

### 构建性能测试

```bash
# 创建构建目录
mkdir -p build && cd build

# 配置CMake，启用性能测试
cmake .. -DENABLE_PERFORMANCE_TESTS=ON

# 构建项目
make -j$(nproc)
```

### 运行性能测试

#### 运行所有性能测试

```bash
# 运行所有性能测试
./tests/performance/run_all_performance_tests

# 或者使用make目标
make run_all_performance_tests
```

#### 运行特定类型的测试

```bash
# 运行缓冲池性能测试
./tests/performance/performance_test --type=buffer_pool

# 或者使用make目标
make run_buffer_pool_tests

# 运行磁盘I/O性能测试
./tests/performance/performance_test --type=disk_io

# 或者使用make目标
make run_disk_io_tests

# 运行混合工作负载测试
./tests/performance/performance_test --type=mixed_workload

# 或者使用make目标
make run_mixed_workload_tests
```

#### 自定义测试参数

```bash
# 指定输出目录
./tests/performance/performance_test --type=buffer_pool --output=/path/to/results

# 启用详细输出
./tests/performance/performance_test --type=buffer_pool --verbose

# 指定测试次数
./tests/performance/performance_test --type=buffer_pool --iterations=5
```

## 结果分析

### 查看测试结果

```bash
# 使用查看脚本查看结果
./scripts/view_performance_results.sh /path/to/results

# 或者直接查看CSV文件
ls -la /path/to/results/*.csv
```

### 生成分析图表

```bash
# 安装Python依赖（如果尚未安装）
pip install pandas matplotlib seaborn

# 生成分析图表和报告
python3 scripts/analyze_performance_results.py /path/to/results
```

### 结果目录结构

性能测试结果将保存在指定的输出目录中，结构如下：

```
results/
├── performance_summary.txt          # 测试摘要
├── buffer_pool_cache_hit_rate.csv   # 缓冲池命中率结果
├── buffer_pool_lru_efficiency.csv   # LRU效率结果
├── buffer_pool_access_pattern.csv   # 访问模式结果
├── buffer_pool_size_scalability.csv # 缓冲池大小扩展性结果
├── disk_io_sequential.csv           # 顺序I/O结果
├── disk_io_random.csv               # 随机I/O结果
├── disk_io_varying_page_size.csv    # 不同页面大小结果
├── disk_io_concurrent.csv           # 并发I/O结果
├── mixed_workload_read_write_ratio.csv # 读写比例结果
├── mixed_workload_transaction_size.csv  # 事务大小结果
├── mixed_workload_long_running.csv  # 长时间运行结果
├── mixed_workload_concurrent.csv    # 并发工作负载结果
└── analysis/                        # 分析结果目录
    ├── *.png                        # 性能图表
    └── performance_summary.md       # 性能分析报告
```

## 性能基准

以下性能基准可用于评估SQLCC存储引擎的性能表现：

### 缓冲池性能基准

- **缓存命中率**: >90%（合适的工作负载）
- **LRU效率**: >85%（工作集大小不超过缓冲池大小）
- **顺序访问吞吐量**: >100,000 ops/sec
- **随机访问吞吐量**: >50,000 ops/sec

### 磁盘I/O性能基准

- **顺序读取**: >500 MB/s
- **顺序写入**: >300 MB/s
- **随机读取**: >100 MB/s
- **随机写入**: >50 MB/s

### 混合工作负载性能基准

- **单线程页面读取**: >10,000 ops/sec
- **单线程页面写入**: >5,000 ops/sec
- **P99延迟**: <5ms（页面读取）
- **P99延迟**: <10ms（页面写入）

## 持续集成

### 集成到CI/CD流水线

可以将性能测试集成到CI/CD流水线中，确保代码更改不会引入性能回归：

```yaml
# 示例GitHub Actions工作流
name: Performance Tests

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  performance-tests:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Build with performance tests
      run: |
        mkdir build && cd build
        cmake .. -DENABLE_PERFORMANCE_TESTS=ON
        make -j$(nproc)
    
    - name: Run performance tests
      run: |
        cd build
        ./tests/performance/run_all_performance_tests --output=perf_results
    
    - name: Analyze performance results
      run: |
        pip install pandas matplotlib seaborn
        python3 scripts/analyze_performance_results.py build/perf_results
    
    - name: Upload performance results
      uses: actions/upload-artifact@v2
      with:
        name: performance-results
        path: build/perf_results
```

### 性能回归检测

可以设置性能基准，当测试结果低于基准时触发警报：

```bash
# 示例性能回归检测脚本
#!/bin/bash

RESULTS_DIR=$1
THRESHOLD_FILE=$2

# 检查结果是否低于阈值
# 实现具体的比较逻辑...
```

## 扩展与定制

### 添加新的性能测试

1. 在`tests/performance`目录下创建新的测试类
2. 继承`PerformanceTestBase`基类
3. 实现特定的测试方法
4. 更新`CMakeLists.txt`包含新测试
5. 更新`performance_test.cc`以支持新测试类型

### 自定义性能指标

可以通过修改`PerformanceTestBase`类来添加新的性能指标：

1. 在`TestResult`结构体中添加新字段
2. 在测试方法中计算新指标
3. 更新结果输出格式以包含新指标

## 故障排除

### 常见问题

1. **构建失败**: 确保安装了所有必要的依赖，包括Google Test和Threads库
2. **测试运行失败**: 检查是否有足够的磁盘空间和权限
3. **结果分析失败**: 确保安装了Python依赖（pandas, matplotlib, seaborn）

### 调试技巧

1. 使用`--verbose`选项获取详细输出
2. 检查测试日志文件了解详细错误信息
3. 使用调试工具（如gdb）分析核心转储

## 贡献指南

欢迎为SQLCC性能测试框架做出贡献！请遵循以下步骤：

1. Fork项目仓库
2. 创建功能分支
3. 实现新功能或修复问题
4. 添加适当的测试
5. 提交Pull Request

## 许可证

SQLCC性能测试框架遵循与主项目相同的许可证。