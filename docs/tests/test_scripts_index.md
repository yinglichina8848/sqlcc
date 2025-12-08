# SQLCC 测试脚本索引

## 概述

本文档提供了SQLCC项目中所有测试脚本的完整索引，包括脚本位置、功能描述、使用方法和相关文档链接。

## 主测试脚本

### 核心测试执行脚本

#### run_tests.sh
- **位置**: `/scripts/run_tests.sh`
- **功能**: 主测试执行脚本，支持完整测试套件执行
- **特性**: 
  - 支持覆盖率测试
  - 并行测试执行
  - 多种测试类型选择
  - 详细的测试报告
- **使用**: `./scripts/run_tests.sh [--coverage] [--parallel=N]`
- **文档**: [主测试脚本指南](../scripts_guide.md#主测试脚本-run_tests.sh)

#### run_all_tests.sh
- **位置**: `/scripts/run_all_tests.sh`
- **功能**: 简化版测试脚本，按顺序执行所有测试
- **特性**:
  - 单元测试 → 覆盖率测试 → 性能测试
  - 简单的执行流程
  - 适合快速验证
- **使用**: `./scripts/run_all_tests.sh`

#### generate_coverage_report.sh
- **位置**: `/scripts/generate_coverage_report.sh`
- **功能**: 生成代码覆盖率报告
- **特性**:
  - 使用lcov收集覆盖率数据
  - 生成HTML格式报告
  - 支持报告过滤和清理
- **使用**: `./scripts/generate_coverage_report.sh [--clean]`
- **文档**: [覆盖率测试指南](../scripts_guide.md#覆盖率测试-generate_coverage_report.sh)

### CI/CD测试脚本

#### ci/run_tests.sh
- **位置**: `/scripts/ci/run_tests.sh`
- **功能**: CI环境专用测试脚本
- **特性**:
  - 支持分类测试执行
  - 详细的参数解析
  - 测试结果摘要输出
  - CI环境优化配置
- **使用**: `./scripts/ci/run_tests.sh [--category=unit|integration|performance]`

## Shell测试脚本

### 功能测试脚本

#### run_sql_executor_tests.sh
- **位置**: `/scripts/shell/run_sql_executor_tests.sh`
- **功能**: SQL执行器真实执行能力测试
- **测试内容**:
  - AST节点测试
  - 执行引擎测试
  - 集成测试
  - CRUD操作验证
- **使用**: `./scripts/shell/run_sql_executor_tests.sh [--test-ast|--test-execution|--test-integration]`

#### run_parser_refactor_tests.sh
- **位置**: `/scripts/shell/run_parser_refactor_tests.sh`
- **功能**: DFA-based解析器系统测试
- **测试内容**:
  - AST核心组件测试
  - 表达式解析测试
  - 集成测试
  - 错误处理测试
- **使用**: `./scripts/shell/run_parser_refactor_tests.sh [--test-ast-core|--test-expression|--test-integration]`

### 性能测试脚本

#### run_crud_performance.sh
- **位置**: `/scripts/shell/run_crud_performance.sh`
- **功能**: CRUD操作性能测试
- **测试规模**: 1-10万行数据
- **性能目标**: 单操作耗时 < 5ms
- **测试项目**:
  - 插入性能测试
  - 查询性能测试
  - 更新性能测试
  - 删除性能测试
- **使用**: `./scripts/shell/run_crud_performance.sh [--rows=N] [--test-type=TYPE]`

#### run_crud_performance_test.sh
- **位置**: `/scripts/shell/run_crud_performance_test.sh`
- **功能**: 详细的CRUD性能测试套件
- **测试项目** (7项):
  1. 插入性能测试
  2. 点查询性能测试
  3. 范围扫描性能测试
  4. 更新性能测试
  5. 删除性能测试
  6. 混合CRUD操作测试
  7. 压力测试
- **报告**: 生成Markdown格式性能报告
- **使用**: `./scripts/shell/run_crud_performance_test.sh`

#### run_performance_example.sh
- **位置**: `/scripts/shell/run_performance_example.sh`
- **功能**: 性能测试示例和演示
- **演示内容**:
  - 缓冲池性能测试
  - 磁盘I/O性能测试
  - 混合工作负载测试
- **特性**:
  - Python依赖自动安装
  - 图表生成支持
  - 详细的性能分析
- **使用**: `./scripts/shell/run_performance_example.sh`

#### run_simple_crud_test.sh
- **位置**: `/scripts/shell/run_simple_crud_test.sh`
- **功能**: 简化版CRUD测试
- **特性**:
  - 使用现有性能测试框架
  - 生成包含测试环境和建议的报告
  - 适合快速功能验证
- **使用**: `./scripts/shell/run_simple_crud_test.sh`

### 其他功能脚本

#### generate_docs.sh
- **位置**: `/scripts/shell/generate_docs.sh`
- **功能**: 文档生成脚本
- **特性**:
  - 支持多种文档格式
  - 自动化文档构建
  - 集成到测试流程

#### release_*.sh
- **位置**: `/scripts/shell/release_*.sh`
- **功能**: 发布相关脚本
- **包括**:
  - 版本管理
  - 发布准备
  - 质量检查

#### run_encryption_test.sh
- **位置**: `/scripts/shell/run_encryption_test.sh`
- **功能**: 加密功能测试
- **测试内容**:
  - 数据加密验证
  - 安全性测试
  - 性能影响评估

## SQL测试脚本

### 基础SQL测试

#### run_crud_tests.sh
- **位置**: `/scripts/sql/run_crud_tests.sh`
- **功能**: CRUD操作SQL测试
- **测试内容**:
  - CREATE TABLE测试
  - INSERT操作测试
  - SELECT查询测试
  - UPDATE更新测试
  - DELETE删除测试

#### run_ddl_tests.sh
- **位置**: `/scripts/sql/run_ddl_tests.sh`
- **功能**: DDL语句测试
- **测试内容**:
  - 表创建和修改
  - 索引管理
  - 约束定义
  - 视图创建

#### run_dml_tests.sh
- **位置**: `/scripts/sql/run_dml_tests.sh`
- **功能**: DML语句测试
- **测试内容**:
  - 数据插入和更新
  - 查询优化
  - 事务处理
  - 并发控制

### 高级SQL测试

#### run_advanced_sql_tests.sh
- **位置**: `/scripts/sql/run_advanced_sql_tests.sh`
- **功能**: 高级SQL功能测试
- **测试内容**:
  - 复杂查询优化
  - 子查询和连接
  - 窗口函数
  - 存储过程和函数

#### run_performance_sql_tests.sh
- **位置**: `/scripts/sql/run_performance_sql_tests.sh`
- **功能**: SQL性能测试
- **测试内容**:
  - 查询性能基准
  - 索引优化效果
  - 执行计划分析
  - 资源使用监控

## Python测试脚本

### 工具和辅助脚本

#### 性能分析工具
- **位置**: `/scripts/python/`
- **功能**: Python实现的性能分析工具
- **包括**:
  - 数据可视化
  - 性能监控
  - 报告生成
  - 基准测试

## 测试目录结构

### tests/ 目录

#### 单元测试 (unit/)
- **位置**: `/tests/unit/`
- **功能**: 核心组件单元测试
- **包括**:
  - 数据结构测试
  - 算法实现测试
  - 工具函数测试

#### 集成测试 (integration/)
- **位置**: `/tests/integration/`
- **功能**: 模块间集成测试
- **包括**:
  - 组件协作测试
  - 端到端流程测试
  - 系统集成验证

#### 高级SQL测试 (advanced_sql/)
- **位置**: `/tests/advanced_sql/`
- **功能**: 复杂SQL功能测试
- **包括**:
  - 高级查询测试
  - 优化器测试
  - 执行引擎测试

#### 性能测试 (performance/)
- **位置**: `/tests/performance/`
- **功能**: 系统性能测试
- **子目录**:
  - `concurrent/`: 并发性能测试
  - `cpu/`: CPU性能测试
  - `crud/`: CRUD性能测试

#### SQL解析器测试 (sql_parser/)
- **位置**: `/tests/sql_parser/`
- **功能**: SQL解析器相关测试
- **包括**:
  - 词法分析测试
  - 语法分析测试
  - AST构建测试
  - 错误恢复测试

#### SQL执行器测试 (sql_executor/)
- **位置**: `/tests/sql_executor/`
- **功能**: SQL执行引擎测试
  - 查询执行测试
  - 事务处理测试
  - 并发控制测试

#### 客户端-服务器测试 (client_server/)
- **位置**: `/tests/client_server/`
- **功能**: 客户端-服务器通信测试
- **包括**:
  - 网络通信测试
  - 协议兼容性测试
  - 连接管理测试

## 工具脚本

### 工具和辅助脚本

#### utils/ 目录
- **位置**: `/scripts/utils/`
- **功能**: 公共工具函数和脚本
- **包括**:
  - 环境检测工具
  - 日志记录工具
  - 错误处理工具
  - 配置管理工具

## 使用建议

### 根据测试目的选择脚本

#### 快速功能验证
```bash
# 使用简化测试脚本
./scripts/run_all_tests.sh

# 或使用快速模式
./scripts/run_tests.sh --quick
```

#### 完整测试套件
```bash
# 运行完整测试（包含覆盖率）
./scripts/run_tests.sh --coverage --parallel=4
```

#### 性能测试
```bash
# CRUD性能测试
./scripts/shell/run_crud_performance.sh

# 详细性能分析
./scripts/shell/run_crud_performance_test.sh
```

#### 特定功能测试
```bash
# SQL执行器测试
./scripts/shell/run_sql_executor_tests.sh

# 解析器测试
./scripts/shell/run_parser_refactor_tests.sh
```

### 测试执行顺序建议

1. **环境准备**: 确保依赖安装和配置正确
2. **快速验证**: 使用 `run_all_tests.sh` 或 `--quick` 模式
3. **功能测试**: 执行特定功能测试脚本
4. **性能测试**: 运行性能相关测试
5. **覆盖率测试**: 生成覆盖率报告
6. **结果分析**: 检查测试报告和日志

## 维护说明

### 脚本更新
- 修改脚本时请更新本文档
- 保持向后兼容性
- 添加适当的版本注释

### 新脚本添加
- 按照现有分类组织
- 提供详细的使用说明
- 集成到主测试框架

### 问题报告
- 发现脚本问题时请及时报告
- 提供详细的复现步骤
- 包含相关日志信息

---

*本文档最后更新: 2025年12月*  
*维护者: SQLCC开发团队*  
*版本: v1.1.1*