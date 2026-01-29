# SQLCC 测试脚本详细指南

## 概述

本文档详细说明SQLCC项目中所有测试脚本的具体功能、参数配置和使用方法。

## 主测试运行脚本

### run_tests.sh

**文件位置**: `/home/liying/sqlcc/scripts/run_tests.sh`

#### 功能描述
- SQLCC集成测试主入口脚本
- 支持多种测试模式和参数配置
- 自动构建环境准备和测试执行

#### 参数说明
```bash
# 基本用法
./scripts/run_tests.sh

# 启用覆盖率测试
./scripts/run_tests.sh --coverage

# 并行执行测试
./scripts/run_tests.sh --parallel

# 指定构建目录
./scripts/run_tests.sh --build-dir custom_build

# 详细输出模式
./scripts/run_tests.sh --verbose
```

#### 核心功能
1. **构建环境准备**
   - 自动创建build_test目录
   - 设置缓存目录和符号链接
   - 检查项目根目录有效性

2. **CMake配置**
   - 使用项目根目录作为源目录
   - 支持覆盖率编译标志
   - 并行构建优化

3. **测试执行**
   - 支持多种测试套件
   - 超时控制和错误处理
   - 结果汇总和报告生成

#### 输出示例
```
=== SQLCC v1.1.1 集成测试 ===
构建目录: build_test
启用覆盖率: 是
并行测试: 否
开始构建项目...
构建完成，开始运行测试...
测试完成，成功率: 95%
```

## 覆盖率测试脚本

### generate_coverage_report.sh

**文件位置**: `/home/liying/sqlcc/scripts/generate_coverage_report.sh`

#### 功能描述
- 生成详细的代码覆盖率报告
- 支持HTML和XML格式输出
- 自动过滤测试文件

#### 使用示例
```bash
# 生成覆盖率报告
./scripts/generate_coverage_report.sh

# 指定构建目录和输出目录
./scripts/generate_coverage_report.sh -b build_coverage -o coverage_reports

# 设置覆盖率阈值
./scripts/generate_coverage_report.sh -t 90
```

#### 处理流程
1. **清理旧数据**: 删除已有的gcda/gcno文件
2. **数据收集**: 使用lcov收集覆盖率数据
3. **报告生成**: 使用genhtml生成HTML报告
4. **结果过滤**: 过滤测试文件，只显示源码覆盖率

#### 输出文件
- `coverage/index.html`: HTML格式覆盖率报告
- `coverage/coverage.xml`: XML格式覆盖率数据
- 详细的文件级和函数级覆盖率统计

## 性能测试脚本

### run_crud_performance.sh

**文件位置**: `/home/liying/sqlcc/scripts/shell/run_crud_performance.sh`

#### 功能描述
- CRUD操作性能验证测试
- 验证1-10万行数据下单操作<5ms的性能要求
- 自动检测存储设备类型和环境配置

#### 测试类型
1. **快速CRUD测试**: 1-1万行数据规模
2. **标准CRUD测试**: 1-10万行数据规模
3. **完整性能测试**: 包含所有CRUD操作类型

#### 使用示例
```bash
# 运行完整CRUD性能测试
./scripts/shell/run_crud_performance.sh

# 只运行快速测试
./scripts/shell/run_crud_performance.sh --quick
```

#### 性能指标
- **插入性能**: 批量插入和单条插入延迟
- **点查性能**: 基于主键的查询延迟
- **范围扫描**: 范围查询性能
- **更新/删除**: 数据修改操作性能
- **混合操作**: 模拟真实业务场景

### run_crud_performance_test.sh

**文件位置**: `/home/liying/sqlcc/scripts/shell/run_crud_performance_test.sh`

#### 功能描述
- 详细的CRUD性能测试套件
- 包含7种不同类型的性能测试
- 生成Markdown格式的详细报告

#### 测试套件
1. **插入性能测试**: 10,000行数据插入
2. **点查性能测试**: 1,000次点查询
3. **范围扫描测试**: 100次范围扫描
4. **更新性能测试**: 1,000次更新操作
5. **删除性能测试**: 1,000次删除操作
6. **混合CRUD测试**: 5,000次混合操作
7. **压力测试**: 100,000行数据规模

#### 报告内容
- 详细的性能数据表格
- 延迟和吞吐量指标
- 达标情况分析（<5ms要求）
- 系统环境信息

## SQL执行器测试脚本

### run_sql_executor_tests.sh

**文件位置**: `/home/liying/sqlcc/scripts/shell/run_sql_executor_tests.sh`

#### 功能描述
- 验证SQL执行器真实执行能力
- 测试AST节点、执行引擎和集成功能
- 消除假执行问题的验证

#### 测试内容
1. **AST节点测试**: 抽象语法树节点功能验证
2. **执行引擎测试**: 执行器核心功能测试
3. **集成测试**: 完整SQL执行流程测试

#### 手动验证指导
脚本提供手动测试SQL示例：
```sql
-- 创建表
CREATE TABLE users (id INTEGER, name VARCHAR, age INTEGER);

-- 插入数据
INSERT INTO users (id, name, age) VALUES (1, 'Alice', 25);

-- 查询数据
SELECT * FROM users;

-- 更新数据
UPDATE users SET age = 26 WHERE id = 1;

-- 删除数据
DELETE FROM users WHERE id = 1;
```

## 解析器测试脚本

### run_parser_refactor_tests.sh

**文件位置**: `/home/liying/sqlcc/scripts/shell/run_parser_refactor_tests.sh`

#### 功能描述
- 完整的DFA-based解析器系统测试
- 多组件集成验证
- 详细的测试统计和错误诊断

#### 测试组件
1. **AST核心测试**: 抽象语法树基础功能
2. **表达式测试**: SQL表达式解析和计算
3. **解析器集成测试**: 完整解析流程验证
4. **错误处理测试**: 异常情况处理能力
5. **AST访问者测试**: 访问者模式实现验证

#### 测试统计
- 总测试数、通过数、失败数、跳过数
- 测试成功率计算
- 颜色化输出和错误详情

## 简化测试脚本

### run_simple_crud_test.sh

**文件位置**: `/home/liying/sqlcc/scripts/shell/run_simple_crud_test.sh`

#### 功能描述
- 使用现有性能测试框架模拟CRUD操作
- 快速验证基础性能
- 生成简化版测试报告

#### 模拟测试类型
1. **百万插入测试**: 模拟插入性能
2. **缓冲池性能测试**: 模拟点查性能
3. **磁盘I/O性能测试**: 模拟范围扫描
4. **索引性能测试**: 模拟更新/删除性能
5. **混合工作负载测试**: 综合性能评估

## CI/CD测试脚本

### ci/run_tests.sh

**文件位置**: `/home/liying/sqlcc/scripts/ci/run_tests.sh`

#### 功能描述
- CI环境专用的综合测试脚本
- 支持分类测试和参数解析
- 结果摘要输出和错误处理

#### CI集成特性
- 自动环境检测和配置
- 并行测试执行优化
- 测试结果聚合和报告
- 错误码处理和退出状态

## 实用工具脚本

### view_performance_results.sh

**文件位置**: `/home/liying/sqlcc/scripts/shell/view_performance_results.sh`

#### 功能描述
- 查看和分析性能测试结果
- 支持多种格式的结果展示
- 性能数据可视化

### run_performance_example.sh

**文件位置**: `/home/liying/sqlcc/scripts/shell/run_performance_example.sh`

#### 功能描述
- 性能测试示例和演示
- 包含完整的构建和运行流程
- 结果分析和图表生成

## 脚本参数汇总

### 通用参数
| 参数 | 说明 | 示例 |
|------|------|------|
| `--coverage` | 启用覆盖率测试 | `--coverage` |
| `--parallel` | 并行执行测试 | `--parallel` |
| `--build-dir` | 指定构建目录 | `--build-dir custom_build` |
| `--verbose` | 详细输出模式 | `--verbose` |
| `--help` | 显示帮助信息 | `--help` |

### 性能测试参数
| 参数 | 说明 | 示例 |
|------|------|------|
| `--quick` | 快速测试模式 | `--quick` |
| `--all` | 完整测试套件 | `--all` |
| `--rows` | 数据行数设置 | `--rows 10000` |
| `--output` | 结果输出目录 | `--output results` |

## 最佳实践

### 测试环境准备
1. 确保在项目根目录运行脚本
2. 检查系统依赖（gcc、cmake、python等）
3. 确保有足够的磁盘空间存储测试数据

### 性能测试注意事项
1. 在SSD存储环境下运行性能测试
2. 关闭不必要的后台进程
3. 多次运行取平均值以获得稳定结果

### 覆盖率测试优化
1. 使用专门的构建目录进行覆盖率测试
2. 定期清理旧的覆盖率数据文件
3. 关注低覆盖率模块的测试补充

## 故障排除

### 常见问题

#### 构建失败
- 检查CMake版本和编译器兼容性
- 验证项目依赖是否完整安装
- 查看详细构建日志定位问题

#### 测试超时
- 调整测试超时设置
- 优化测试用例执行效率
- 考虑使用并行测试模式

#### 覆盖率数据缺失
- 确保启用覆盖率编译标志
- 检查gcov工具是否可用
- 验证测试执行是否正常完成

### 调试技巧

1. **启用详细输出**: 使用 `--verbose` 参数
2. **检查日志文件**: 查看生成的测试日志
3. **分步执行**: 单独运行特定测试类型
4. **环境验证**: 使用诊断脚本检查环境配置

---

*本文档最后更新: 2025年12月*  
*维护者: SQLCC开发团队*