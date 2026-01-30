# SQLCC 测试框架文档

## 概述

本文档详细记录了SQLCC v1.3.9项目的完整测试框架体系，包括测试脚本的用途、功能、设计说明、使用指南和改进记录。

## 测试框架结构

### 目录结构

```
tests/
├── unit/                    # 单元测试（组件级测试）
│   ├── parser/             # 解析器单元测试
│   ├── executor/           # 执行器单元测试
│   ├── storage/            # 存储引擎单元测试
│   ├── transaction/        # 事务管理器单元测试
│   ├── config/             # 配置管理器单元测试
│   ├── core/               # 核心组件单元测试
│   └── network/            # 网络组件单元测试
├── integration/            # 集成测试（组件间交互测试）
│   ├── basic_sql/          # 基本SQL集成测试
│   └── advanced_sql/       # 高级SQL集成测试
├── advanced_sql/           # 高级SQL特性测试
│   ├── join/               # JOIN操作测试
│   ├── subquery/           # 子查询测试
│   ├── window/             # 窗口函数测试
│   ├── grouping/           # 分组和聚合测试
│   └── set_operation/      # 集合操作测试
├── performance/            # 性能测试
│   ├── crud/               # CRUD性能测试
│   ├── advanced/           # 高级SQL性能测试
│   └── concurrency/        # 并发性能测试
├── security/               # 安全性测试
├── regression/             # 回归测试
└── framework/              # 测试框架和工具

scripts/
├── run_tests.sh            # 主测试运行脚本
├── generate_coverage_report.sh  # 覆盖率报告生成脚本
├── run_all_tests.sh        # 完整测试套件运行脚本
├── shell/                  # Shell脚本目录
│   ├── run_crud_performance.sh      # CRUD性能测试
│   ├── run_crud_performance_test.sh # CRUD性能测试（详细版）
│   ├── run_simple_crud_test.sh      # 简化CRUD测试
│   ├── run_performance_example.sh   # 性能测试示例
│   ├── run_sql_executor_tests.sh    # SQL执行器测试
│   ├── run_parser_refactor_tests.sh # 解析器重构测试
│   └── view_performance_results.sh  # 性能结果查看
├── sql/                    # SQL测试脚本目录
│   ├── run_crud_tests.sh   # CRUD SQL测试
│   └── ...                 # 其他SQL测试脚本
└── ci/                     # CI/CD相关脚本
    └── run_tests.sh        # CI环境测试脚本
```

## 测试脚本功能说明

### 主要测试脚本

#### 1. run_tests.sh（主测试运行脚本）
- **位置**: `/home/liying/sqlcc/scripts/run_tests.sh`
- **功能**: 集成测试主入口，支持多种参数配置
- **特性**:
  - 支持 `--coverage` 参数生成覆盖率报告
  - 支持 `--parallel` 参数并行执行测试
  - 自动构建环境准备和缓存管理
  - 测试超时控制和结果汇总

#### 2. generate_coverage_report.sh（覆盖率报告生成）
- **位置**: `/home/liying/sqlcc/scripts/generate_coverage_report.sh`
- **功能**: 生成详细的代码覆盖率报告
- **特性**:
  - 清理旧的覆盖率数据文件
  - 使用lcov收集覆盖率数据
  - 生成HTML格式的详细报告
  - 过滤测试文件，只显示源码覆盖率

#### 3. run_all_tests.sh（完整测试套件）
- **位置**: `/home/liying/sqlcc/scripts/run_all_tests.sh`
- **功能**: 按顺序执行所有类型的测试
- **特性**:
  - 单元测试 → 覆盖率测试 → 性能测试
  - 支持分类测试执行
  - 结果汇总和错误处理

#### 4. run_crud_performance.sh（CRUD性能测试）
- **位置**: `/home/liying/sqlcc/scripts/shell/run_crud_performance.sh`
- **功能**: 验证CRUD操作性能（1-10万行数据，单操作<5ms）
- **特性**:
  - 自动检测存储设备类型（SSD/HDD）
  - 生成详细的性能报告
  - 支持快速测试和完整测试模式

#### 5. run_sql_executor_tests.sh（SQL执行器测试）
- **位置**: `/home/liying/sqlcc/scripts/shell/run_sql_executor_tests.sh`
- **功能**: 验证SQL执行器真实执行能力
- **特性**:
  - AST节点测试、执行引擎测试、集成测试
  - 消除假执行问题验证
  - 手动测试验证指导

#### 6. run_parser_refactor_tests.sh（解析器重构测试）
- **位置**: `/home/liying/sqlcc/scripts/shell/run_parser_refactor_tests.sh`
- **功能**: 完整的DFA-based解析器系统测试
- **特性**:
  - 多组件测试（AST核心、表达式、集成测试等）
  - 详细的测试统计和成功率计算
  - 颜色化输出和错误诊断

## 测试框架设计

### 设计原则

1. **模块化设计**: 测试脚本按功能模块划分，职责明确
2. **可配置性**: 支持参数化配置，适应不同测试需求
3. **自动化**: 一键执行，减少人工干预
4. **可扩展性**: 易于添加新的测试类型和功能
5. **报告完整性**: 提供详细的测试报告和性能分析

### 技术架构

#### 构建系统集成
- 与CMake构建系统深度集成
- 支持多种构建类型（Debug、Release、Coverage）
- 自动检测构建状态和依赖关系

#### 测试执行框架
- 基于Google Test框架
- 支持单元测试、集成测试、性能测试
- 并行执行和超时控制

#### 覆盖率分析
- 使用gcov/lcov工具链
- 支持行覆盖率、分支覆盖率分析
- HTML报告生成和可视化

#### 性能监控
- 系统资源监控（CPU、内存、存储）
- 延迟和吞吐量指标收集
- 性能阈值检测和告警

## 使用指南

### 快速开始

#### 运行所有测试
```bash
cd /home/liying/sqlcc
./scripts/run_tests.sh
```

#### 生成覆盖率报告
```bash
./scripts/run_tests.sh --coverage
```

#### 运行性能测试
```bash
./scripts/shell/run_crud_performance.sh
```

### 高级用法

#### 并行执行测试
```bash
./scripts/run_tests.sh --parallel
```

#### 只运行特定测试类型
```bash
# 只运行单元测试
./scripts/run_tests.sh --unit-only

# 只运行性能测试
./scripts/run_tests.sh --performance-only
```

#### 自定义构建目录
```bash
./scripts/run_tests.sh --build-dir custom_build
```

### 测试结果解读

#### 覆盖率报告
- **行覆盖率**: 代码行执行比例
- **分支覆盖率**: 条件分支覆盖比例
- **文件覆盖率**: 每个文件的覆盖率详情

#### 性能测试报告
- **延迟指标**: 单操作平均延迟（要求<5ms）
- **吞吐量**: 每秒操作数（ops/sec）
- **资源使用**: CPU、内存、磁盘I/O使用情况

## 改进记录

### v1.1.1 版本改进

#### 测试框架重构
- ✅ 统一测试执行器，支持参数化配置
- ✅ 集成覆盖率测试，支持HTML报告生成
- ✅ 性能测试框架完善，支持真实执行环境

#### 测试脚本优化
- ✅ 脚本模块化重构，提高可维护性
- ✅ 错误处理和日志记录改进
- ✅ 自动化构建和依赖管理

#### 新功能添加
- ✅ SQL执行器真实执行能力测试
- ✅ 解析器重构全面测试套件
- ✅ CRUD性能测试自动化脚本

### 已知问题和待改进项

#### 当前限制
- 部分测试脚本存在重复功能
- 覆盖率报告生成存在格式兼容性问题
- 性能测试数据规模验证需要加强

#### 未来改进计划
- 实现更精确的CRUD延迟测量
- 添加分布式测试支持
- 集成持续测试和监控

## 维护指南

### 添加新测试

1. **创建测试文件**: 在适当的测试目录下创建新的测试文件
2. **更新CMakeLists.txt**: 添加新的测试可执行文件配置
3. **集成到测试脚本**: 更新相关测试脚本以包含新测试
4. **验证功能**: 确保新测试能够正确执行和报告结果

### 问题排查

#### 常见问题
1. **构建失败**: 检查依赖关系和编译器版本
2. **测试超时**: 调整测试超时设置或优化测试逻辑
3. **覆盖率数据缺失**: 确保启用覆盖率编译标志
4. **性能测试不达标**: 检查测试环境和系统资源

#### 调试工具
- 使用 `--verbose` 参数获取详细输出
- 检查测试日志文件分析具体错误
- 使用性能分析工具（如perf）定位性能瓶颈

## 贡献指南

欢迎对测试框架进行改进和扩展。在提交贡献时请确保：

1. 遵循现有的代码风格和目录结构
2. 添加相应的测试用例验证新功能
3. 更新相关文档说明变更内容
4. 确保所有现有测试仍然通过

---

*本文档最后更新: 2025年12月*  
*维护者: SQLCC开发团队*