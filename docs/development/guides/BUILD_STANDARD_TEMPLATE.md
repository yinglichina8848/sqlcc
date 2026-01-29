# SQLCC Bazel构建配置标准化模板

## 概述

本模板定义了SQLCC项目中BUILD.bazel文件的标准化格式，确保整个项目的构建配置保持一致性。

## 模板结构

### 1. src/*/BUILD.bazel 模板

用于源代码目录的构建配置：

```bazel
# src/{module_name}/BUILD.bazel - {ModuleName}模块
# 遵循设计文档的包层次结构

load("@rules_cc//cc:defs.bzl", "cc_library")

# 导出源文件以供其他包使用
exports_files([
    "{component1}.cpp",
    "{component2}.cpp",
    # ...
])

cc_library(
    name = "{component1}",
    srcs = ["{component1}.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/core:core",
        "//include/{module_name}:{component1_hdr}",
        # 其他依赖
    ],
)

cc_library(
    name = "{component2}",
    srcs = ["{component2}.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/core:core",
        "//include/{module_name}:{component2_hdr}",
        # 其他依赖
    ],
)

# {ModuleName}模块聚合库
cc_library(
    name = "{module_name}",
    visibility = ["//visibility:public"],
    deps = [
        ":{component1}",
        ":{component2}",
        # ...
    ],
)
```

### 2. include/*/BUILD.bazel 模板

用于头文件目录的构建配置：

```bazel
package(default_visibility = ["//visibility:public"])

cc_library(
    name = "{component1_hdr}",
    hdrs = ["{component1}.h"],
)

cc_library(
    name = "{component2_hdr}",
    hdrs = ["{component2}.h"],
)

# -----------------------------------------------------------------------------
# 综合头文件库 - 所有 {module_name} 头文件的集合
# -----------------------------------------------------------------------------

cc_library(
    name = "headers",
    hdrs = glob(["*.h"]),
    visibility = ["//visibility:public"],
)
```

### 3. tests/*/BUILD.bazel 模板

用于测试目录的构建配置：

```bazel
# tests/{test_type}/{module_name}/BUILD.bazel - {ModuleName}测试

load("@rules_cc//cc:defs.bzl", "cc_test")

cc_test(
    name = "{component1}_test",
    srcs = ["{component1}_test.cpp"],
    deps = [
        "//src/{module_name}:{component1}",
        "//src/core:core",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "{component2}_test",
    srcs = ["{component2}_test.cpp"],
    deps = [
        "//src/{module_name}:{component2}",
        "//src/core:core",
        "@com_google_googletest//:gtest_main",
    ],
)

# 集成测试
cc_test(
    name = "{module_name}_integration_test",
    srcs = ["{module_name}_integration_test.cpp"],
    deps = [
        "//src/{module_name}:{module_name}",
        "//src/core:core",
        "@com_google_googletest//:gtest_main",
    ],
)
```

## 标准化规则

### 1. 命名规范

- **模块名**: 全小写，使用下划线分隔，如 `storage_engine`
- **组件名**: 全小写，使用下划线分隔，如 `b_plus_tree_index`
- **头文件库**: 组件名后加 `_hdr` 后缀，如 `b_plus_tree_index_hdr`
- **聚合库**: 使用模块名，如 `storage_engine`

### 2. 依赖管理

- **内部依赖**: 使用 `//{path}:{target}` 格式
- **外部依赖**: 使用 `@repository//:{target}` 格式
- **循环依赖**: 严格禁止，应通过接口分离或前向声明解决

### 3. 可见性

- **公共组件**: `visibility = ["//visibility:public"]`
- **内部组件**: `visibility = ["//{package}:__subpackages__"]`
- **私有组件**: `visibility = ["//visibility:private"]`

### 4. 文件组织

- **源文件导出**: 使用 `exports_files` 导出 `.cpp` 文件供其他包使用
- **头文件分组**: 相关头文件组织在同一目录下
- **测试分离**: 测试文件与源代码分离

## 应用示例

### storage_engine 模块

```bazel
# src/storage_engine/BUILD.bazel
load("@rules_cc//cc:defs.bzl", "cc_library")

exports_files([
    "b_plus_tree_index.cpp",
    "buffer_pool.cpp",
    # ...
])

cc_library(
    name = "b_plus_tree_index",
    srcs = ["b_plus_tree_index.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/core:core",
        "//include/storage_engine:b_plus_tree_index_hdr",
    ],
)

cc_library(
    name = "buffer_pool",
    srcs = ["buffer_pool.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        "//src/core:core",
        "//include/storage_engine:buffer_pool_hdr",
    ],
)

cc_library(
    name = "storage_engine",
    visibility = ["//visibility:public"],
    deps = [
        ":b_plus_tree_index",
        ":buffer_pool",
        # ...
    ],
)
```

## 维护指南

1. **定期检查**: 定期检查BUILD.bazel文件是否符合模板规范
2. **自动化验证**: 使用工具自动检查构建配置的一致性
3. **文档同步**: 更新模板时同步更新相关文档
4. **培训**: 确保团队成员了解并遵循标准化规范

## 工具支持

- **bazel_dep_fixer.py**: 自动修复依赖关系
- **bazel_label_fixer.py**: 检查和修复标签格式
- **validate_build_system.sh**: 验证构建系统完整性
