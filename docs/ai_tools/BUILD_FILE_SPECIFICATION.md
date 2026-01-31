# SQLCC BUILD.bazel 文件编写规范 v1.3.9

**版本**: 1.3.9  
**生效日期**: 2026-01-30  
**适用范围**: 所有 SQLCC 模块的 Bazel 构建配置

---

## 📋 目录

1. [核心原则](#核心原则)
2. [文件基本结构](#文件基本结构)
3. [目标类型规范](#目标类型规范)
4. [依赖声明规范](#依赖声明规范)
5. [标签路径规范](#标签路径规范)
6. [测试配置规范](#测试配置规范)
7. [常见错误与修复](#常见错误与修复)
8. [模板示例](#模板示例)

---

## 核心原则

### 1. 模块化设计
- **一个目录一个 BUILD 文件**: 每个源码目录必须有独立的 BUILD.bazel
- **最小化依赖**: 只声明直接依赖，避免传递依赖污染
- **清晰的可见性**: 使用 `visibility` 控制目标访问范围

### 2. 一致性优先
- **统一命名**: 目标名使用 snake_case，与文件名保持一致
- **统一结构**: 所有 BUILD 文件遵循相同的基本结构
- **统一风格**: 缩进、空格、引号风格一致

### 3. 可维护性
- **注释清晰**: 复杂依赖必须注释说明
- **分组管理**: 按功能分组组织源文件
- **避免重复**: 使用变量和宏减少重复代码

---

## 文件基本结构

```bazel
# =============================================================================
# 模块名称 BUILD 配置
# =============================================================================
# 描述: 简要描述本模块的功能
# 维护: SQLCC 开发团队
# 更新: 2026-01-30
# =============================================================================

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary", "cc_test")

# -----------------------------------------------------------------------------
# 导出配置
# -----------------------------------------------------------------------------
package(default_visibility = ["//visibility:public"])

# -----------------------------------------------------------------------------
# 头文件导出 (仅 include/ 目录需要)
# -----------------------------------------------------------------------------
cc_library(
    name = "headers",
    hdrs = glob(["*.h"]),  # 或明确列出头文件
    includes = ["."],  # 头文件搜索路径
    visibility = ["//visibility:public"],
)

# -----------------------------------------------------------------------------
# 核心库目标
# -----------------------------------------------------------------------------
cc_library(
    name = "模块名",
    srcs = glob(["*.cpp"], exclude = ["*_test.cpp", "*_main.cpp"]),
    hdrs = glob(["*.h"]),  # 或使用 :headers
    deps = [
        # 按层次组织依赖
        # 1. 同一项目头文件
        "//include/模块:headers",
        # 2. 其他模块
        "//src/依赖模块:依赖模块",
        # 3. 第三方库
        "@com_google_googletest//:gtest",
    ],
    copts = [
        "-std=c++20",
        "-stdlib=libc++",
    ],
    visibility = ["//visibility:public"],
)

# -----------------------------------------------------------------------------
# 可执行文件 (如需要)
# -----------------------------------------------------------------------------
cc_binary(
    name = "可执行文件名",
    srcs = ["main.cpp"],
    deps = [
        ":模块名",
    ],
)

# -----------------------------------------------------------------------------
# 测试目标
# -----------------------------------------------------------------------------
cc_test(
    name = "模块名_test",
    srcs = ["模块名_test.cpp"],
    deps = [
        ":模块名",
        "@com_google_googletest//:gtest_main",
    ],
    tags = ["单元测试", "基础层"],  # 用于测试过滤
)

# -----------------------------------------------------------------------------
# 测试套件 (可选)
# -----------------------------------------------------------------------------
test_suite(
    name = "all_tests",
    tests = [
        ":模块名_test",
    ],
)
```

---

## 目标类型规范

### cc_library - C++ 库

```bazel
cc_library(
    name = "target_name",  # 使用 snake_case，与模块名一致
    srcs = [
        # 源文件列表或 glob
        "file1.cpp",
        "file2.cpp",
    ],
    hdrs = [
        # 头文件列表或使用 :headers
        "file1.h",
    ],
    deps = [
        # 依赖列表，按层次排序
        "//include/module:headers",
        "//src/other:other",
    ],
    copts = [
        # 编译选项
        "-std=c++20",
        "-stdlib=libc++",
        "-Wall",
        "-Wextra",
    ],
    linkopts = [
        # 链接选项
        "-pthread",
    ],
    visibility = ["//visibility:public"],  # 或限制为特定包
)
```

**命名规则**:
- 主库: 与目录名一致，如 `buffer_pool`
- 接口库: `模块名_interface` 或 `模块名_api`
- 工具库: `模块名_utils` 或 `模块名_tools`

### cc_binary - 可执行文件

```bazel
cc_binary(
    name = "executable_name",  # 可执行文件名
    srcs = ["main.cpp"],       # 入口文件
    deps = [
        # 依赖的库
        "//src/module:module",
    ],
    data = [
        # 运行时数据文件
        "//config:sqlcc.conf",
    ],
)
```

### cc_test - 测试目标

```bazel
cc_test(
    name = "test_target_name",
    srcs = ["test_file.cpp"],
    deps = [
        ":被测模块",
        "@com_google_googletest//:gtest_main",
    ],
    tags = [
        "基础层",      # level1_foundation
        "核心层",      # level2_core
        "存储层",      # level2_storage_engine
        "事务层",      # level3_transaction
        "SQL层",       # level4_sql_processing
        "网络层",      # level5_network
        "集成测试",    # level6_integration/level7_integration
        "性能测试",    # performance
    ],
    timeout = "short",  # short(60s)/moderate(300s)/long(900s)/eternal
    size = "small",     # small/medium/large/enormous
)
```

---

## 依赖声明规范

### 依赖顺序

按以下优先级排序，每层内按字母顺序：

1. **项目头文件** (`//include/...`)
2. **项目源码** (`//src/...`)
3. **测试工具** (`//tests/...`)
4. **第三方库** (`@repo//...`)
5. **系统库** (`-l...`)

```bazel
deps = [
    # 1. 项目头文件
    "//include/core:headers",
    "//include/sql_parser:headers",
    
    # 2. 项目源码模块
    "//src/core:user_manager",
    "//src/storage_engine:buffer_pool",
    
    # 3. 测试工具 (仅测试目标)
    "//tests/utils:test_helpers",
    
    # 4. 第三方库
    "@com_google_googletest//:gtest",
    
    # 5. 系统库
    "-lpthread",
],
```

### 标签格式

**正确格式**:
```bazel
"//src/module:target"           # 完整路径
"//include/core:headers"        # 头文件目标
"@external_repo//package:lib"   # 外部依赖
":local_target"                 # 当前包目标（省略 //package）
```

**错误格式**:
```bazel
"//src:module/target"           # ❌ 不能使用斜杠分隔目标名
"//include:core/header.h"       # ❌ 不能直接在 include 后加冒号
"@repo/package:target"          # ❌ 缺少 //
```

---

## 标签路径规范

### 关键规则

| 场景 | 正确 | 错误 |
|------|------|------|
| include 头文件 | `//include/core:headers` | `//include:core/header.h` |
| src 源文件 | `//src/core:core` | `//src:core/core.cpp` |
| 当前包目标 | `:target` | `//current:target` |
| 子目录 | `//src/core/utils:utils` | `//src/core:utils/target` |

### 路径映射示例

```
目录结构:              Bazel 标签:
include/
  core/
    user_manager.h  -> //include/core:headers
    
src/
  core/
    user_manager.cpp -> //src/core:core
    BUILD.bazel
    
tests/
  level1_foundation/
    exception/
      exception_test.cpp -> //tests/level1_foundation/exception:exception_test
```

---

## 测试配置规范

### 测试标签体系

```bazel
tags = [
    # 层次标签 (必选其一)
    "foundation",      # Level 1: 基础组件
    "core",            # Level 2: 核心组件
    "storage",         # Level 2: 存储引擎
    "transaction",     # Level 3: 事务管理
    "sql_processing",  # Level 4: SQL处理
    "network",         # Level 5: 网络通信
    "integration",     # Level 6-7: 集成测试
    
    # 特性标签 (可选)
    "performance",     # 性能测试
    "security",        # 安全测试
    "boundary",        # 边界测试
    "concurrent",      # 并发测试
    
    # 执行标签 (可选)
    "slow",            # 慢速测试
    "flaky",           # 不稳定测试
    "manual",          # 手动执行
],
```

### 测试运行命令

```bash
# 运行特定层次测试
bazel test //tests/... --test_tag_filters=foundation
bazel test //tests/... --test_tag_filters=core

# 运行非慢速测试
bazel test //tests/... --test_tag_filters=-slow

# 组合过滤
bazel test //tests/... --test_tag_filters=core,-slow
```

---

## 常见错误与修复

### 错误1: Multiple Definition (重复定义)

**现象**:
```
error: multiple definition of `FormatConfigValue'
```

**原因**: 函数在头文件中定义但未标记 inline

**修复**:
```cpp
// 头文件中
inline std::string FormatConfigValue(const Value& v) {
    // 实现
}

// 或在 BUILD 中只保留一个实现
```

### 错误2: Invalid Label (无效标签)

**现象**:
```
Label '//include:core/execution_context.h' is invalid
```

**修复**:
```bazel
# 错误
"//include:core/execution_context.h"

# 正确
"//include/core:execution_context.h"
# 或
"//include/core:headers"
```

### 错误3: 循环依赖

**现象**:
```
Cycle in dependency graph
```

**修复**:
1. 提取公共接口到独立模块
2. 使用接口抽象层
3. 重新设计模块边界

### 错误4: 头文件找不到

**现象**:
```
fatal error: 'some_header.h' file not found
```

**修复**:
```bazel
cc_library(
    name = "module",
    srcs = ["module.cpp"],
    hdrs = ["module.h"],
    deps = [
        # 添加头文件依赖
        "//include/module:headers",
    ],
    includes = ["."],  # 添加当前目录到搜索路径
)
```

---

## 模板示例

### 标准模块模板

```bazel
# =============================================================================
# <模块名> 构建配置
# =============================================================================

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

# -----------------------------------------------------------------------------
# 头文件
# -----------------------------------------------------------------------------
exports_files(glob(["*.h"]))

# -----------------------------------------------------------------------------
# 核心库
# -----------------------------------------------------------------------------
cc_library(
    name = "<module_name>",
    srcs = glob(
        ["*.cpp"],
        exclude = ["*_test.cpp"],
    ),
    hdrs = glob(["*.h"]),
    deps = [
        # 基础依赖
        "//include/exception:headers",
        "//include/utils:headers",
        
        # 模块依赖 (按字母顺序)
        # "//src/other_module:other_module",
    ],
    copts = [
        "-std=c++20",
        "-stdlib=libc++",
        "-Wall",
        "-Wextra",
    ],
    visibility = ["//visibility:public"],
)

# -----------------------------------------------------------------------------
# 单元测试
# -----------------------------------------------------------------------------
cc_test(
    name = "<module_name>_test",
    srcs = ["<module_name>_test.cpp"],
    deps = [
        ":<module_name>",
        "@com_google_googletest//:gtest_main",
    ],
    tags = ["foundation"],  # 根据实际情况修改
    timeout = "short",
)
```

### 头文件包模板 (include/)

```bazel
# =============================================================================
# <模块名> 头文件包
# =============================================================================

package(default_visibility = ["//visibility:public"])

# -----------------------------------------------------------------------------
# 头文件导出
# -----------------------------------------------------------------------------
cc_library(
    name = "headers",
    hdrs = glob(["*.h"]),
    includes = ["."],
    visibility = ["//visibility:public"],
)

# -----------------------------------------------------------------------------
# 子目录头文件
# -----------------------------------------------------------------------------
cc_library(
    name = "<submodule>_headers",
    hdrs = glob(["<submodule>/*.h"]),
    includes = ["."],
    visibility = ["//visibility:public"],
)
```

### 测试目录模板

```bazel
# =============================================================================
# <层次>_<模块> 测试
# =============================================================================

load("@rules_cc//cc:defs.bzl", "cc_test", "test_suite")

# -----------------------------------------------------------------------------
# 单元测试
# -----------------------------------------------------------------------------
cc_test(
    name = "<module>_test",
    srcs = ["<module>_test.cpp"],
    deps = [
        "//src/<module>:<module>",
        "@com_google_googletest//:gtest_main",
    ],
    tags = ["<level_tag>"],
    timeout = "short",
)

# -----------------------------------------------------------------------------
# 边界测试
# -----------------------------------------------------------------------------
cc_test(
    name = "<module>_boundary_test",
    srcs = ["<module>_boundary_test.cpp"],
    deps = [
        "//src/<module>:<module>",
        "@com_google_googletest//:gtest_main",
    ],
    tags = ["<level_tag>", "boundary"],
    timeout = "moderate",
)

# -----------------------------------------------------------------------------
# 测试套件
# -----------------------------------------------------------------------------
test_suite(
    name = "all_tests",
    tests = [
        ":<module>_test",
        ":<module>_boundary_test",
    ],
)
```

---

## 验证清单

提交 BUILD 文件前检查:

- [ ] 标签路径格式正确 (`//package:target`)
- [ ] 依赖按层次排序
- [ ] 目标名使用 snake_case
- [ ] 头文件使用 `glob` 或明确列出
- [ ] 测试有正确的标签
- [ ] 编译选项包含 `-std=c++20`
- [ ] 可见性设置合理

验证命令:
```bash
# 语法检查
bazel build //... --nobuild

# 实际构建
bazel build //src/<module>:<module>

# 运行测试
bazel test //tests/<level>/<module>:all
```

---

**维护者**: SQLCC 开发团队  
**最后更新**: 2026-01-30  
**版本**: v1.3.9
