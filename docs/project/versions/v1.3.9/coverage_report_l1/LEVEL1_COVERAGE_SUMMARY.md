# SQLCC Level 1 Foundation 覆盖率测试报告

## 测试覆盖汇总

| 模块 | 测试目标 | 源码目录 | Region 覆盖率 | 函数覆盖率 | 行覆盖率 | 状态 |
|------|----------|----------|---------------|------------|----------|------|
| **exception** | //tests/level1_foundation/exception:exception_test | src/exception | 100.00% | 100.00% | 100.00% | ✅ |
| **basic** | //tests/level1_foundation/basic:basic_test | src/exception | 100.00% | 100.00% | 100.00% | ✅ |
| **logger** | //tests/level1_foundation/logger:logger_test | src/logger | 86.96% | 80.00% | 87.25% | ✅ |
| **types** | //tests/level1_foundation/types:types_test | src/types | 72.29% | 100.00% | 79.55% | ✅ |
| **config** | //tests/level1_foundation/config:config_test | src/utils | 55.36% | 10.71% | 15.14% | ⚠️ |
| **utils** | //tests/level1_foundation/utils:utils_test | src/utils | 80.36% | 81.82% | 72.92% | ✅ |

## 详细分析

### 1. exception 模块 (src/exception)
- **覆盖率**: 100%
- **主要文件**: base_exception.cpp (100%)
- **说明**: 测试覆盖了所有异常类的核心功能

### 2. basic 模块 (src/exception)
- **覆盖率**: 100%
- **说明**: 与 exception 模块使用相同源码，覆盖率一致

### 3. logger 模块 (src/logger)
- **覆盖率**: 86.96%
- **主要文件**: logger.cpp
- **未覆盖**: 部分错误处理和边缘情况

### 4. types 模块 (src/types)
- **覆盖率**: 72.29%
- **主要文件**: domain_manager.cpp
- **说明**: 测试覆盖了域管理的核心功能

### 5. config 模块 (src/utils)
- **覆盖率**: 55.36%
- **主要文件**: config_lifecycle.cpp (100%), config_snapshot.cpp, config_manager.cpp
- **说明**: config_lifecycle 完全覆盖，但其他模块覆盖不足

### 6. utils 模块 (src/utils)
- **覆盖率**: 80.36%
- **主要文件**: thread_pool.cpp
- **状态**: ✅ 已修复（原为 0%）

## 覆盖率测试修复

### 问题 1: 脚本目录查找路径错误
**修复**: 修改脚本中目录查找逻辑，从 `-name "$MODULE"` 改为 `-path "*/${MODULE}_test/test"`

### 问题 2: utils_test 测试内容问题
**原问题**: utils_test 只测试概念性 utility 函数，未调用实际源码
**修复**: 重写 utils_test.cpp，实际测试 ThreadPool 类

### 问题 3: 模块映射不完整
**修复**: 在 CORE_MODULES 中添加 utils 和 basic 模块映射

## 测试配置确认

### BUILD.bazel 配置（各模块）
- ✅ exception: 使用 `//src/exception:exception_coverage`
- ✅ logger: 使用 `//src/logger:logger_coverage`
- ✅ types: 使用 `//src/types:types_coverage`
- ✅ config: 使用 `//src/utils:utils_coverage`
- ✅ utils: 使用 `//src/utils:utils_coverage`
- ✅ basic: 使用 `//src/exception:exception_coverage`

### 覆盖率编译选项
- ✅ `-fprofile-instr-generate`
- ✅ `-fcoverage-mapping`
- ✅ `linkopts: -fprofile-instr-generate`

## 总体评估

| 指标 | 数值 |
|------|------|
| 通过 | 5/6 模块 |
| 警告 | 1/6 模块 |
| 失败 | 0/6 模块 |

**Level 1 Foundation 平均覆盖率**: ~82.56%

## 工具链
- **覆盖率工具**: llvm-cov-20
- **构建系统**: Bazel 8.5.0+
- **编译器**: Clang 20+

## 报告位置
- 覆盖率数据: `/home/liying/sqlcc/coverage_report_l1_complete/`
- 生成时间: 2026-01-31
