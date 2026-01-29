# SQLCC Bazel 循环依赖分析报告

## 分析时间
2025-12-21 01:50:16 (UTC+8)

## 概述
通过分析SQLCC项目的Bazel构建文件，发现了多个严重的循环依赖问题，这些问题会导致构建失败或不一致性。

## 发现的问题

### 1. 核心循环依赖
**位置**: `src/core/BUILD.bazel`
**问题**: `//src/core:core` 库依赖自身
```bazel
cc_library(
    name = "core",
    ...
    deps = [
        "//src/core:core",  # ❌ 循环依赖！
        ...
    ],
)
```

### 2. SQL解析器循环依赖
**位置**: `src/sql_parser/BUILD.bazel`
**问题**: `//src/sql_parser:sqlcc_parser` 依赖不存在的 `//src/sql_parser:sql_parser`
```bazel
cc_library(
    name = "sqlcc_parser",
    ...
    deps = [
        "//src/sql_parser:sql_parser",  # ❌ 不存在的依赖
        ...
    ],
)
```

### 3. 正确的库命名
**实际库名**: `//src/sql_parser:sqlcc_parser`
**测试中使用的**: 混合使用 `sqlcc_parser` 和错误的 `sql_parser`

### 4. 头文件库问题
**问题**: 多个BUILD文件引用不存在的头文件库
- `//include/execution:task_executor_hdr`
- `//include/execution:join_executor_hdr`
- `//include/execution:set_operation_executor_hdr`
- `//include/execution:load_data_executor_hdr`
- `//include/execution:window_function_executor_hdr`

### 5. 构建配置问题
**位置**: `tests/debug/BUILD.bazel`
**问题**: 使用了不支持的 `allow_empty` 参数
```bazel
exports_files(glob(["*.cpp"]), allow_empty=True)  # ❌ 不支持的参数
```

## 影响评估

### 严重程度
- **高**: 核心组件的循环依赖会导致构建失败
- **中**: 命名不一致导致依赖解析失败
- **低**: 头文件库问题可能导致链接错误

### 受影响组件
- 核心系统 (`src/core`)
- SQL解析器 (`src/sql_parser`)
- 所有测试 (`tests/`)

## 改进计划

### 阶段1: 紧急修复 (1-2天)
1. **修复核心循环依赖**
   - 移除 `//src/core:core` 对自身的依赖
   - 重新组织依赖关系，确保层次清晰

2. **修复SQL解析器依赖**
   - 澄清 `sql_parser` vs `sqlcc_parser` 的关系
   - 移除不必要的循环依赖

3. **修复构建配置错误**
   - 移除不支持的 `allow_empty` 参数

### 阶段2: 系统性重构 (3-5天)
1. **标准化库命名**
   - 统一使用 `sql_parser` 而不是 `sqlcc_parser`
   - 确保所有测试使用正确的库名

2. **重构头文件库**
   - 创建适当的头文件库或移除不必要的引用
   - 确保头文件库只包含头文件，不包含实现

3. **优化依赖关系**
   - 分析并移除不必要的依赖
   - 建立清晰的依赖层次结构

### 阶段3: 验证和测试 (2-3天)
1. **构建验证**
   - 确保所有组件可以成功构建
   - 运行完整的测试套件

2. **性能优化**
   - 分析构建时间和依赖关系
   - 优化构建配置

## 具体修复方案

### 修复1: 移除核心循环依赖
```bazel
# src/core/BUILD.bazel
deps = [
    # 移除 "//src/core:core",
    "//src/utils:utils",
    "//src/storage_engine:storage_engine",
    "//src/sql_parser:sql_parser",
    "//src/logger:logger",
    "//include:error_handler",
],
```

### 修复2: 澄清SQL解析器依赖
```bazel
# src/sql_parser/BUILD.bazel
cc_library(
    name = "sql_parser",  # 使用标准名称
    deps = [
        # 移除对自身的依赖
        "//src/types:types",
        # 其他必要依赖
    ],
)
```

### 修复3: 修复测试依赖
```bazel
# tests/unit/executor/BUILD.bazel
deps = [
    "@com_google_googletest//:gtest_main",
    "//src/core:core",
    "//src/storage_engine:storage_engine",
    "//src/sql_parser:sql_parser",  # 使用标准名称
    "//src/execution:execution",
],
```

## 验证方法
1. 运行 `bazel build //tests/...` 确保无循环依赖错误
2. 运行 `bazel test //tests/...` 确保所有测试通过
3. 使用 `bazel query` 验证依赖图的合理性

## 后续建议
1. 建立定期依赖检查机制
2. 在CI/CD中添加循环依赖检测
3. 建立依赖关系文档和规范
4. 培训开发团队了解正确的依赖管理实践
