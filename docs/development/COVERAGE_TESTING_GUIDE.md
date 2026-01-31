# SQLCC 覆盖率测试指南

**版本**: v1.4.0  
**日期**: 2026-01-31  
**适用范围**: Level 1 - Level 7 所有测试层级

---

## 1. 概述

本文档详细记录 SQLCC 项目覆盖率测试的成功经验和配置方法，指导如何为各个测试层级配置正确的覆盖率测试环境。

### 核心工具链

| 工具 | 版本 | 用途 |
|------|------|------|
| **Bazel** | 8.5.0+ | 构建和测试管理 |
| **Clang** | 20+ | 编译器，支持覆盖率编译选项 |
| **llvm-cov** | 20+ | 覆盖率报告生成 |
| **llvm-profdata** | 20+ | 覆盖率数据合并 |
| **Google Test** | 1.14.0 | 单元测试框架 |

---

## 2. Level 1 覆盖率测试配置（成功经验）

### 2.1 测试结果汇总

| 模块 | Region 覆盖率 | 函数覆盖率 | 行覆盖率 | 状态 |
|------|---------------|------------|----------|------|
| **exception** | 100.00% | 100.00% | 100.00% | ✅ |
| **basic** | 100.00% | 100.00% | 100.00% | ✅ |
| **logger** | 86.96% | 80.00% | 87.25% | ✅ |
| **types** | 72.29% | 100.00% | 79.55% | ✅ |
| **config** | 55.36% | 10.71% | 15.14% | ⚠️ |
| **utils** | 80.36% | 81.82% | 72.92% | ✅ |

**平均覆盖率**: ~82.56%

### 2.2 成功经验

#### 经验 1: 覆盖率库的独立配置

每个模块需要创建专门的覆盖率库，包含源码用于覆盖率统计：

```python
# src/exception/BUILD.bazel

# 普通库（不含覆盖率信息）
cc_library(
    name = "exception",
    hdrs = ["exception.h", "io_exception.h"],
    copts = ["-std=c++20"],
    visibility = ["//visibility:public"],
)

# 覆盖率专用库（包含源码和覆盖率编译选项）
cc_library(
    name = "exception_coverage",
    srcs = glob(["**/*.cpp"]),
    hdrs = glob(["**/*.h"]),
    copts = [
        "-std=c++20",
        "-fprofile-instr-generate",    # 生成覆盖率数据
        "-fcoverage-mapping",          # 生成源代码映射
    ],
    linkopts = ["-fprofile-instr-generate"],
    visibility = ["//visibility:public"],
    tags = ["coverage"],
)
```

**关键点**:
- 使用独立的 `*_coverage` 库名
- 添加 `-fprofile-instr-generate` 和 `-fcoverage-mapping` 编译选项
- `linkopts` 也需要添加覆盖率链接选项
- 标记 `tags = ["coverage"]` 用于识别

#### 经验 2: 测试目标的覆盖率配置

```python
# tests/level1_foundation/utils/BUILD.bazel

cc_test(
    name = "utils_test",
    srcs = ["utils_test.cpp"],
    copts = [
        "-fprofile-instr-generate",
        "-fcoverage-mapping",
        "-Isrc",                       # 添加源码路径以支持头文件覆盖率
    ],
    linkopts = ["-fprofile-instr-generate"],
    deps = [
        "@com_google_googletest//:gtest_main",
        "//src/utils:utils_coverage",  # 使用覆盖率库
    ],
    tags = ["coverage", "level1"],
)
```

**关键点**:
- 测试目标需要与覆盖率库相同的编译选项
- 依赖 `*_coverage` 变体库而非普通库
- 添加 `-Isrc` 以支持源码路径解析

#### 经验 3: 脚本目录查找路径修复

原始脚本使用 `-name "$MODULE"` 查找目录，但 Bazel 生成的目录名是 `{module}_test`，导致覆盖率数据收集失败。

**修复方案**:

```bash
# 错误写法
COV_DIR=$(find "$BAZEL_CACHE" -path "*_coverage*" -name "$MODULE" -type d)

# 正确写法
TEST_SUBDIR="${MODULE}_test"
COV_DIR=$(find "$BAZEL_CACHE" -path "*_coverage*" -path "*/$TEST_SUBDIR/test" -type d)
```

#### 经验 4: 模块映射配置

```bash
# 核心模块配置 - 确保所有测试模块都有对应的源码目录映射
declare -A CORE_MODULES=(
    ["types"]="src/types"
    ["config"]="src/utils"
    ["logger"]="src/logger"
    ["exception"]="src/exception"
    ["utils"]="src/utils"           # 新增 utils 模块
    ["basic"]="src/exception"       # 新增 basic 模块（与 exception 共享）
)

# 测试模块配置 - Bazel 目标映射
declare -A TEST_MODULES=(
    ["types"]="tests/level1_foundation/types:types_test"
    ["config"]="tests/level1_foundation/config:config_test"
    ["logger"]="tests/level1_foundation/logger:logger_test"
    ["exception"]="tests/level1_foundation/exception:exception_test"
    ["utils"]="tests/level1_foundation/utils:utils_test"
    ["basic"]="tests/level1_foundation/basic:basic_test"
)
```

### 2.3 遇到的问题及解决方案

#### 问题 1: utils_test 覆盖率始终为 0%

**原因分析**:
- `utils_test.cpp` 只测试概念性的 utility 函数
- 测试中没有调用 `src/utils/` 中的实际源码
- 仅使用标准库函数，未链接覆盖率库代码

**解决方案**:
1. 重写 `utils_test.cpp`，实际测试 `ThreadPool` 类
2. 添加对源码的直接调用
3. 确保依赖正确的覆盖率库

**重写后的测试代码**:
```cpp
#include <gtest/gtest.h>
#include "src/utils/thread_pool.h"

namespace sqlcc {
namespace test {

class ThreadPoolTest : public ::testing::Test {
protected:
    void SetUp() override { pool = nullptr; }
    void TearDown() override {
        if (pool) {
            pool->shutdown();
            pool.reset();
        }
    }
    std::unique_ptr<utils::ThreadPool> pool;
};

TEST_F(ThreadPoolTest, ConstructorAndDestructor) {
    pool = std::make_unique<utils::ThreadPool>(2);
    EXPECT_TRUE(pool != nullptr);
    EXPECT_EQ(pool->queued_tasks(), 0);
}

TEST_F(ThreadPoolTest, SubmitTask) {
    pool = std::make_unique<utils::ThreadPool>(2);
    std::atomic<int> counter{0};

    for (int i = 0; i < 5; ++i) {
        pool->submit([&counter]() { counter++; });
    }

    pool->wait();
    EXPECT_EQ(counter.load(), 5);
}

} // namespace test
} // namespace sqlcc
```

#### 问题 2: 脚本生成的报告文件为空

**原因分析**:
- `llvm-cov` 命令执行失败，但错误被 `2>/dev/null` 抑制
- `coverage_summary.txt` 和 `coverage_detailed.txt` 文件存在但内容为空

**解决方案**:
1. 分离stdout和stderr输出
2. 使用 `&&` 操作符确保命令成功才写入文件
3. 添加详细的错误日志输出

```bash
# 改进后的脚本逻辑
llvm-cov-20 report \
    --instr-profile="$TEST_COVERAGE_DIR/$MODULE.profdata" \
    $OBJ_FILES \
    2>/tmp/cov_error.log \
    > "$OUTPUT_DIR/coverage_summary.txt"

if [ $? -ne 0 ] || [ ! -s "$OUTPUT_DIR/coverage_summary.txt" ]; then
    echo "    ⚠️  汇总报告生成失败:"
    cat /tmp/cov_error.log
fi
```

#### 问题 3: coverage_summary.txt 显示旧数据

**原因分析**:
- 文件确实存在但内容为空（0字节）
- Bash 的 `cat` 命令不显示空文件
- 使用 `wc -l` 检查发现行数为 0

**解决方案**:
1. 使用 `wc -l` 验证文件内容
2. 重新运行 `llvm-cov report` 生成正确数据
3. 使用 `head` 管道输出验证内容

---

## 3. 推广到 Level 2 的配置方法

### 3.1 Level 2 模块结构

```
tests/level2_core/
├── core_test.cpp
├── BUILD.bazel
├── database_manager/
│   └── database_manager_test.cpp
└── user_manager/
    └── user_manager_test.cpp

tests/level2_storage_engine/
├── buffer_pool/
│   └── buffer_pool_test.cpp
├── b_plus_tree/
│   └── b_plus_tree_test.cpp
└── disk_manager/
    └── disk_manager_test.cpp
```

### 3.2 覆盖率库配置

#### Level 2 Core 模块

```python
# src/core/BUILD.bazel

cc_library(
    name = "core",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    deps = [
        "//src/exception:exception",
        "//src/logger:logger",
        "//src/utils:utils",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "core_coverage",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    copts = [
        "-std=c++20",
        "-fprofile-instr-generate",
        "-fcoverage-mapping",
    ],
    linkopts = ["-fprofile-instr-generate"],
    deps = [
        "//src/exception:exception_coverage",
        "//src/logger:logger_coverage",
        "//src/utils:utils_coverage",
    ],
    visibility = ["//visibility:public"],
    tags = ["coverage"],
)
```

#### Level 2 Storage Engine 模块

```python
# src/storage_engine/buffer_pool/BUILD.bazel

cc_library(
    name = "buffer_pool",
    srcs = ["buffer_pool_sharded.cpp"],
    hdrs = ["buffer_pool_sharded.h"],
    deps = [
        "//src/storage_engine/disk_manager:disk_manager",
        "//src/utils:utils",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "buffer_pool_coverage",
    srcs = ["buffer_pool_sharded.cpp"],
    hdrs = ["buffer_pool_sharded.h"],
    copts = [
        "-std=c++20",
        "-fprofile-instr-generate",
        "-fcoverage-mapping",
    ],
    linkopts = ["-fprofile-instr-generate"],
    deps = [
        "//src/storage_engine/disk_manager:disk_manager_coverage",
        "//src/utils:utils_coverage",
    ],
    visibility = ["//visibility:public"],
    tags = ["coverage"],
)
```

### 3.3 Level 2 测试配置

#### Level 2 Core 测试

```python
# tests/level2_core/BUILD.bazel

cc_test(
    name = "core_test",
    srcs = ["core_test.cpp"],
    copts = [
        "-fprofile-instr-generate",
        "-fcoverage-mapping",
        "-Isrc",
    ],
    linkopts = ["-fprofile-instr-generate"],
    deps = [
        "@com_google_googletest//:gtest_main",
        "//src/core:core_coverage",
        "//src/exception:exception_coverage",
    ],
    tags = ["coverage", "core"],
)
```

#### Level 2 Storage Engine 测试

```python
# tests/level2_storage_engine/buffer_pool/BUILD.bazel

cc_test(
    name = "buffer_pool_test",
    srcs = ["buffer_pool_test.cpp"],
    copts = [
        "-fprofile-instr-generate",
        "-fcoverage-mapping",
    ],
    linkopts = ["-fprofile-instr-generate"],
    deps = [
        "@com_google_googletest//:gtest_main",
        "//src/storage_engine/buffer_pool:buffer_pool_coverage",
        "//src/storage_engine/disk_manager:disk_manager_coverage",
    ],
    tags = ["coverage", "storage", "buffer_pool"],
)
```

### 3.4 Level 2 覆盖率报告生成

创建 `scripts/generate_l2_coverage.sh`:

```bash
#!/bin/bash

# SQLCC Level 2 覆盖率报告生成脚本

set -e

PROJECT_ROOT="/home/liying/sqlcc"
COVERAGE_DIR="$PROJECT_ROOT/coverage_report_l2"
BAZEL_CACHE="/home/liying/.cache/bazel/_bazel_liying/68dbc53c53085b82ed46643b8af8ae0d/execroot/_main"

mkdir -p "$COVERAGE_DIR"

# Level 2 模块配置
declare -A CORE_MODULES=(
    ["core"]="src/core"
    ["buffer_pool"]="src/storage_engine/buffer_pool"
    ["b_plus_tree"]="src/storage_engine/b_plus_tree"
    ["disk_manager"]="src/storage_engine/disk_manager"
)

declare -A TEST_MODULES=(
    ["core"]="tests/level2_core:core_test"
    ["buffer_pool"]="tests/level2_storage_engine/buffer_pool:buffer_pool_test"
    ["b_plus_tree"]="tests/level2_storage_engine/b_plus_tree:b_plus_tree_test"
    ["disk_manager"]="tests/level2_storage_engine/disk_manager:disk_manager_test"
)

echo "=========================================="
echo "SQLCC Level 2 覆盖率报告生成"
echo "=========================================="

# 运行测试并收集覆盖率
for MODULE in "${!TEST_MODULES[@]}"; do
    TEST_TARGET="${TEST_MODULES[$MODULE]}"
    echo "📊 测试: $MODULE"
    bazel coverage "$TEST_TARGET" --test_output=errors 2>&1 | tail -5
done

# 收集覆盖率数据
for MODULE in "${!CORE_MODULES[@]}"; do
    SOURCE_DIR="${CORE_MODULES[$MODULE]}"
    TEST_SUBDIR="${MODULE}_test"

    echo "📊 收集: $MODULE"

    COV_DIR=$(find "$BAZEL_CACHE" -path "*_coverage*" -path "*/$TEST_SUBDIR/test" -type d 2>/dev/null | head -1)

    if [ -n "$COV_DIR" ] && [ -d "$COV_DIR" ]; then
        mkdir -p "$COVERAGE_DIR/$MODULE"
        cp "$COV_DIR"/*.profraw "$COVERAGE_DIR/$MODULE/" 2>/dev/null || true

        if ls "$COVERAGE_DIR/$MODULE"/*.profraw 1> /dev/null 2>&1; then
            llvm-profdata-20 merge -o "$COVERAGE_DIR/$MODULE/$MODULE.profdata" "$COVERAGE_DIR/$MODULE"/*.profraw 2>/dev/null || true
            echo "    ✓ 覆盖率数据: $COVERAGE_DIR/$MODULE/$MODULE.profdata"
        fi
    fi
done

# 生成报告
for MODULE in "${!CORE_MODULES[@]}"; do
    SOURCE_DIR="${CORE_MODULES[$MODULE]}"
    TEST_COVERAGE_DIR="$COVERAGE_DIR/$MODULE"

    echo "📊 生成报告: $MODULE"

    OUTPUT_DIR="$COVERAGE_DIR/$MODULE"
    mkdir -p "$OUTPUT_DIR"

    OBJ_FILES=$(find "$BAZEL_CACHE" -path "*/$SOURCE_DIR/*_coverage/*.pic.o" 2>/dev/null | tr '\n' ' ')

    if [ -f "$TEST_COVERAGE_DIR/$MODULE.profdata" ] && [ -n "$OBJ_FILES" ]; then
        llvm-cov-20 report \
            --instr-profile="$TEST_COVERAGE_DIR/$MODULE.profdata" \
            $OBJ_FILES \
            2>/dev/null > "$OUTPUT_DIR/coverage_summary.txt"

        llvm-cov-20 show \
            --instr-profile="$TEST_COVERAGE_DIR/$MODULE.profdata" \
            --show-line-counts \
            --show-branches=count \
            $OBJ_FILES \
            2>/dev/null > "$OUTPUT_DIR/coverage_detailed.txt"

        llvm-cov-20 show \
            --instr-profile="$TEST_COVERAGE_DIR/$MODULE.profdata" \
            --format=html \
            --output-dir="$OUTPUT_DIR" \
            $OBJ_FILES \
            2>/dev/null
    fi

    if [ -f "$OUTPUT_DIR/index.html" ]; then
        echo "    ✓ 报告已生成: $OUTPUT_DIR/index.html"
    fi
done

echo ""
echo "=========================================="
echo "✅ Level 2 覆盖率报告生成完成!"
echo "=========================================="
echo ""
echo "📂 报告位置: $COVERAGE_DIR"
```

### 3.5 验证 Level 2 覆盖率

```bash
# 运行 Level 2 测试覆盖率
bash scripts/generate_l2_coverage.sh

# 或使用 Bazel 直接运行
bazel coverage //tests/level2_core:all --test_output=errors
bazel coverage //tests/level2_storage_engine/buffer_pool:all --test_output=errors
bazel coverage //tests/level2_storage_engine/b_plus_tree:all --test_output=errors

# 查看覆盖率摘要
cat coverage_report_l2/core/coverage_summary.txt
cat coverage_report_l2/buffer_pool/coverage_summary.txt
```

---

## 4. 通用覆盖率测试模板

### 4.1 源码模块 BUILD.bazel 模板

```python
package(default_visibility = ["//visibility:public"])

cc_library(
    name = "{module_name}",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    deps = [
        "//src/exception:exception",
        "//src/logger:logger",
        "//src/utils:utils",
    ],
)

cc_library(
    name = "{module_name}_coverage",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    copts = [
        "-std=c++20",
        "-fprofile-instr-generate",
        "-fcoverage-mapping",
    ],
    linkopts = ["-fprofile-instr-generate"],
    deps = [
        "//src/exception:exception_coverage",
        "//src/logger:logger_coverage",
        "//src/utils:utils_coverage",
    ],
    visibility = ["//visibility:public"],
    tags = ["coverage"],
)
```

### 4.2 测试模块 BUILD.bazel 模板

```python
cc_test(
    name = "{module_name}_test",
    srcs = ["{module_name}_test.cpp"],
    copts = [
        "-fprofile-instr-generate",
        "-fcoverage-mapping",
    ],
    linkopts = ["-fprofile-instr-generate"],
    deps = [
        "@com_google_googletest//:gtest_main",
        "//src/{module_path}:{module_name}_coverage",
    ],
    tags = ["coverage", "{level_tag}"],
)
```

### 4.3 替换变量说明

| 变量 | 说明 | 示例 |
|------|------|------|
| `{module_name}` | 模块名（小写） | `buffer_pool` |
| `{module_path}` | 源码路径 | `src/storage_engine/buffer_pool` |
| `{level_tag}` | 测试层级标签 | `core`, `storage`, `transaction` |

---

## 5. 故障排除指南

### 5.1 常见问题

#### Q1: 覆盖率数据文件为空

```bash
# 检查 profraw 文件
ls -la /home/liying/.cache/bazel/*/execroot/_main/bazel-out/*/testlogs/_coverage/*/*/test/*.profraw

# 手动合并 profraw
llvm-profdata-20 merge -sparse /path/to/*.profraw -o merged.profdata

# 验证 profdata
llvm-cov-20 report --instr-profile=merged.profdata /path/to/*.pic.o
```

#### Q2: 覆盖率报告显示 0%

```bash
# 检查测试是否实际执行
bazel test //tests/level1_foundation/utils:utils_test --test_output=all

# 检查覆盖率数据是否生成
find /home/liying/.cache/bazel -name "*.profraw" | head -10

# 重新运行覆盖率测试
bazel coverage //tests/level1_foundation/utils:utils_test --test_output=errors
```

#### Q3: llvm-cov 找不到源码

```bash
# 检查源码路径
llvm-cov-20 show --sources=/home/liying/sqlcc/src/utils/thread_pool.cpp ...

# 确保编译时使用绝对路径
# 在 BUILD.bazel 中添加 -fdebug-info-for-profiling（可选）
```

### 5.2 调试命令

```bash
# 详细覆盖率输出
bazel coverage //tests/... --test_output=all 2>&1 | grep -A 5 "coverage"

# 检查编译器标志
clang++ -v -c test.cpp -fprofile-instr-generate -fcoverage-mapping

# 查看 profraw 文件内容
llvm-profdata-20 show --all /path/to/*.profraw
```

---

## 6. 质量门禁

### 6.1 覆盖率目标

| 层级 | Region 覆盖率目标 | 函数覆盖率目标 | 行覆盖率目标 |
|------|------------------|----------------|--------------|
| **Level 1** | 90% | 95% | 90% |
| **Level 2** | 80% | 85% | 80% |
| **Level 3+** | 70% | 75% | 70% |

### 6.2 验证命令

```bash
# 运行所有 Level 1 覆盖率测试
bash scripts/generate_l1_complete_coverage.sh

# 检查覆盖率摘要
cat coverage_report_l1_complete/LEVEL1_COVERAGE_SUMMARY.md

# 验证是否达标
bash scripts/check_coverage_quality.sh --level=1
```

---

## 7. 参考资料

- **Bazel 覆盖率测试**: https://bazel.build/external/coverage
- **LLVM Coverage Mapping Format**: https://llvm.org/docs/CoverageMappingFormat.html
- **Google Test 覆盖率**: https://github.com/google/googletest/blob/master/googletest/docs/advanced.md#code-coverage

---

**维护者**: SQLCC 开发团队  
**最后更新**: 2026-01-31  
**版本**: v1.4.0
