# SQLCC BUILD.bazel 文件编写规范 v1.3.10

**版本**: 1.3.10
**生效日期**: 2026-02-02
**适用范围**: 所有 SQLCC 模块的 Bazel 构建配置

---

## 📋 目录

1. [核心原则](#核心原则)
2. [头文件引用规范](#头文件引用规范)
3. [文件基本结构](#文件基本结构)
4. [目标类型规范](#目标类型规范)
5. [依赖声明规范](#依赖声明规范)
6. [标签路径规范](#标签路径规范)
7. [头文件配置规范](#头文件配置规范)
8. [测试配置规范](#测试配置规范)
9. [常见错误与修复](#常见错误与修复)
10. [模板示例](#模板示例)

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

## 头文件引用规范

### 1. 头文件路径语法

Bazel 构建系统要求使用**逻辑路径**而非**文件系统相对路径**。

#### C++ 源文件中的 #include 语法

**正确写法**:
```cpp
#include "storage_engine/buffer_pool.h"    // 模块/头文件
#include "exception/exception.h"           // 模块/头文件
#include "utils/config_manager.h"          // 模块/头文件
```

**错误写法**:
```cpp
#include "../../include/page.h"            // ❌ 禁止使用相对路径
#include "../disk_manager.h"               // ❌ 禁止使用相对路径
#include "src/core/user_manager.h"         // ❌ 不需要 src/ 前缀
```

### 2. 头文件组织结构

```
sqlcc/
├── include/                    # 头文件目录
│   ├── core/                  # 核心组件头文件
│   │   ├── user_manager.h
│   │   └── permission_validator.h
│   ├── sql_parser/            # SQL解析器头文件
│   ├── storage_engine/        # 存储引擎头文件
│   ├── execution/             # 执行引擎头文件
│   ├── transaction/           # 事务管理头文件
│   ├── network/               # 网络通信头文件
│   ├── exception/             # 异常处理头文件
│   ├── utils/                 # 工具类头文件
│   └── types/                 # 数据类型头文件
├── src/                       # 源代码目录
│   ├── core/
│   ├── storage_engine/
│   └── ...
└── tests/                     # 测试目录
```

**关键规则**:
- 头文件路径: `include/<module>/<file>.h`
- 源文件路径: `src/<module>/<file>.cpp`
- 引用时使用 Bazel 逻辑路径，不含 `include/` 前缀

### 3. 头文件保护

优先使用 `#pragma once`，或使用传统宏保护：

```cpp
#pragma once

/**
 * @file buffer_pool.h
 * @brief 缓冲池管理器
 * @author SQLCC Team
 * @date 2026-01-30
 */

// 传统宏保护（备选方案）
#ifndef SQLCC_BUFFER_POOL_H
#define SQLCC_BUFFER_POOL_H

// 头文件内容...

#endif  // SQLCC_BUFFER_POOL_H
```

### 4. Include 顺序规范

按以下顺序组织 `#include` 语句：

```cpp
// 1. 对应的头文件（对于 .cpp 文件）
#include "storage_engine/buffer_pool.h"

// 2. 项目头文件（使用引号，按模块分组）
#include "exception/exception.h"
#include "utils/config_manager.h"
#include "types/value.h"

// 3. 第三方头文件（使用尖括号）
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

// 4. 系统头文件（使用尖括号）
#include <memory>
#include <vector>
#include <string>
```

**注意**: 严格遵守 include 规范，禁止引用 `src/` 目录外部的头文件，避免循环依赖。

### 5. 前向声明使用

在头文件中优先使用前向声明，减少编译依赖：

```cpp
// 前向声明（在头文件中使用）
class BufferPool;
class DiskManager;

// 当需要智能指针时
#include <memory>

// 前向声明 + unique_ptr
class TableMetadata;
std::unique_ptr<TableMetadata> CreateTable(const std::string& name);

// 前向声明 + shared_ptr
class QueryExecutor;
std::shared_ptr<QueryExecutor> GetExecutor();

// 使用 weak_ptr 打破循环引用
class BufferPool;
class BufferPoolStats;
std::weak_ptr<BufferPoolStats> stats_;  // 打破循环引用
```

**何时使用前向声明**:
- 仅使用指针或引用类型
- 不需要知道类型的大小
- 不需要调用类型的成员函数

**何时需要完整 include**:
- 需要知道类型的大小
- 需要调用成员函数
- 需要创建对象实例
- 使用模板类型（需要完整类型）

### 6. Bazel 头文件导出配置

#### include/ 目录的 BUILD.bazel

```bazel
# include/core/BUILD.bazel
package(default_visibility = ["//visibility:public"])

# 导出所有头文件
cc_library(
    name = "headers",
    hdrs = glob(["*.h"]),
    includes = ["."],
    visibility = ["//visibility:public"],
)

# 子目录头文件导出（如有需要）
cc_library(
    name = "submodule_headers",
    hdrs = glob(["submodule/*.h"]),
    includes = ["."],
    visibility = ["//visibility:public"],
)
```

#### strip_include_prefix 配置

```bazel
# src/storage_engine/disk_manager/BUILD.bazel
cc_library(
    name = "disk_manager",
    srcs = ["disk_manager.cpp"],
    hdrs = ["disk_manager.h"],
    # 关键配置：strip 后编译时可直接使用 "storage_engine/disk_manager/disk_manager.h"
    strip_include_prefix = "src/storage_engine/disk_manager",
    include_prefix = "",  # 或设置为 "storage_engine"
    visibility = ["//visibility:public"],
    deps = [
        "//src/storage_engine:storage_engine_headers",
        "//src/exception:exception_headers",
    ],
)
```

**配置说明**:
- `strip_include_prefix`: 从编译路径中移除此前缀
- `include_prefix`: 添加此前缀到 include 路径
- 两者配合使用，控制编译时头文件的查找路径

### 7. 头文件依赖原则

#### 依赖方向

```
高层模块 → 低层模块（依赖抽象，而非具体）
```

示例依赖关系:
```
execution/          (高层)
    ↓ depends on
sql_parser/         (中层)
    ↓ depends on
types/              (低层)
    ↓ depends on
exception/          (基础)
```

#### 避免循环依赖

```bazel
# 错误：A 依赖 B，B 依赖 A
# //src/core/BUILD.bazel
deps = ["//src/execution:execution"]  # ❌ 循环依赖

# 正确：提取公共接口到独立模块
# //src/core/BUILD.bazel
deps = [
    "//src/core:core_interface",  # ✅ 依赖抽象接口
]
```

### 8. 头文件工具脚本

```bash
# 检查头文件路径
python3 tools/bazel_include_fixer.py

# 修复标签问题
python3 tools/bazel_label_fixer_enhanced.py

# 系统性修复依赖
python3 tools/bazel_dep_fixer_enhanced.py .

# 验证头文件配置
python3 tools/bazel_code_checker.py
```

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

## 头文件配置规范

### 1. 头文件导出配置

在 `include/` 目录下，需要导出所有头文件供其他模块引用：

```bazel
# include/core/BUILD.bazel
package(default_visibility = ["//visibility:public"])

cc_library(
    name = "headers",
    hdrs = glob(["*.h"]),
    includes = ["."],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "submodule_headers",
    hdrs = glob(["submodule/*.h"]),
    includes = ["."],
    visibility = ["//visibility:public"],
)
```

### 2. strip_include_prefix 配置

用于控制编译时头文件的查找路径：

```bazel
# src/storage_engine/disk_manager/BUILD.bazel
cc_library(
    name = "disk_manager",
    srcs = ["disk_manager.cpp"],
    hdrs = ["disk_manager.h"],
    strip_include_prefix = "src/storage_engine/disk_manager",
    include_prefix = "",
    visibility = ["//visibility:public"],
    deps = [
        "//src/storage_engine:storage_engine_headers",
        "//src/exception:exception_headers",
    ],
)
```

**配置说明**:
- `strip_include_prefix`: 从编译路径中移除此前缀
- `include_prefix`: 添加此前缀到 include 路径
- 两者配合使用，控制编译时头文件的查找路径

### 3. 路径映射示例

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
**最后更新**: 2026-02-02
**版本**: v1.3.10
