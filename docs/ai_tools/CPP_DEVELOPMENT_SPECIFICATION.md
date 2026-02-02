# SQLCC C++ 开发、测试与重构规范 v1.3.10

**版本**: 1.3.10
**生效日期**: 2026-02-02
**适用范围**: SQLCC 项目（ Bazel + Clang-20 + C++20 ）

---

## 📋 目录

1. [核心原则](#核心原则)
2. [开发规范](#开发规范)
3. [测试驱动开发 (TDD)](#测试驱动开发-tdd)
4. [Bazel + Clang-20 构建规范](#bazel--clang-20-构建规范)
5. [重构规范](#重构规范)
6. [质量门禁](#质量门禁)
7. [工具链集成](#工具链集成)
8. [参考资源](#参考资源)

---

## 核心原则

### 1. FIRST 原则

| 原则 | 说明 |
|------|------|
| **F**ind First | 先查找、先阅读、先理解 |
| **I**nvestigate Before Implement | 调研先于实现 |
| **R**espect Existing Style | 尊重现有代码风格 |
| **S**ystematic Approach | 系统性方法 |
| **T**est Everything | 测试一切 |

### 2. 约束条件

| 约束 | 要求 |
|------|------|
| **语言标准** | C++20 |
| **构建系统** | Bazel 8.5.0+ |
| **编译器** | Clang 20+ (libc++) |
| **智能指针** | 强制使用 |
| **测试框架** | Google Test |
| **头文件保护** | `#pragma once` 或宏保护 |

### 3. 禁止行为

- ❌ 不读取文件直接修改代码
- ❌ 使用裸指针管理资源所有权
- ❌ 引入新的编码风格
- ❌ 不测试就提交代码
- ❌ 破坏向后兼容性
- ❌ 引入循环依赖
- ❌ 不更新文档

---

## 开发规范

### 1. 代码质量工具链

#### 编译器警告配置

```bash
# 严格警告级别
-Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic

# 警告即错误
-Werror

# 运行时检测（开发模式）
-fsanitize=address     # AddressSanitizer (内存错误)
-fsanitize=thread     # ThreadSanitizer (数据竞争)
-fsanitize=undefined  # UB Sanitizer (未定义行为)
```

#### 静态分析工具

```bash
# Clang Static Analyzer
clang-check src/*.cpp --

# Cppcheck
cppcheck --enable=all src/

# SonarLint (IDE 插件)
# 在 VSCode/CLion 中安装 SonarLint 插件
```

### 2. 命名规范

| 类型 | 规范 | 示例 |
|------|------|------|
| **文件名** | snake_case | `buffer_pool.cpp` |
| **类名** | PascalCase | `BufferPoolManager` |
| **公有函数** | PascalCase | `FetchPage()` |
| **私有函数** | snake_case | `get_free_slot()` |
| **变量** | snake_case | `buffer_pool_size` |
| **成员变量** | snake_case + `_` | `page_id_` |
| **常量** | kPascalCase | `kDefaultPoolSize` |
| **宏** | UPPER_SNAKE_CASE | `SQLCC_BUFFER_SIZE` |
| **命名空间** | 全小写 | `namespace sqlcc` |

### 3. 语法规范

#### 头文件保护

```cpp
#pragma once

/**
 * @file buffer_pool.h
 * @brief 缓冲池管理器
 * @author SQLCC Team
 * @date 2026-02-02
 */
```

或传统宏保护：

```cpp
#ifndef SQLCC_BUFFER_POOL_H
#define SQLCC_BUFFER_POOL_H

// 头文件内容...

#endif  // SQLCC_BUFFER_POOL_H
```

#### Include 顺序

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

#### 语言特性使用

```cpp
// ✅ 使用 nullptr
auto ptr = std::make_unique<Page>();

// ❌ 避免使用 0 或 NULL
int* ptr = 0;  // 禁止！

// ✅ 所有代码块必须使用 {}
if (condition) {
    do_something();
}

// ✅ 单参数构造函数使用 explicit
class Connection {
public:
    explicit Connection(const std::string& url);
};

// ✅ 零规则：使用智能指针管理资源
class ResourceManager {
private:
    std::shared_ptr<DiskManager> disk_manager_;
    std::unique_ptr<ReplaceStrategy> replace_strategy_;
};

// ✅ 使用 weak_ptr 打破循环引用
std::weak_ptr<BufferPoolStats> stats_;
```

### 4. 注释规范 (Why-What-How)

```cpp
/**
 * WHY: 为什么需要分片缓冲池而不是单锁设计？
 * 传统缓冲池使用单一互斥锁保护所有操作，导致高并发场景下的锁竞争激烈。
 * 分片设计通过减少锁粒度，提高并发性能。
 *
 * WHAT: 基于 RocksDB 风格的 Sharded Buffer Pool 实现
 * 特点：
 * 1. 按 2^n 分 shard，使用 page_id 哈希取模定位 shard
 * 2. 每个 shard 独立 LRU + 独立 mutex
 * 3. 支持高并发访问
 *
 * HOW: 分片并发访问算法
 * 1. 计算分片索引：page_id % num_shards
 * 2. 获取对应分片的锁
 * 3. 在分片内查找页面
 * 4. 处理页面固定计数
 * 5. 释放锁，返回页面
 */
class BufferPoolSharded {
    // ...
};
```

### 5. 前向声明使用

```cpp
// 前向声明（在头文件中使用）
class BufferPool;
class DiskManager;

// ✅ 何时使用前向声明
// - 仅使用指针或引用类型
// - 不需要知道类型的大小
// - 不需要调用类型的成员函数

// ❌ 何时需要完整 include
// - 需要知道类型的大小
// - 需要调用成员函数
// - 需要创建对象实例
// - 使用模板类型（需要完整类型）
```

---

## 测试驱动开发 (TDD)

### 1. TDD 流程

```
┌─────────────────────────────────────────────────────────────┐
│  1. 编写失败的测试 (Red)                                     │
│     - 明确功能需求                                           │
│     - 编写测试用例                                           │
│     - 运行测试确认失败                                       │
├─────────────────────────────────────────────────────────────┤
│  2. 编写最少代码使测试通过 (Green)                           │
│     - 快速实现功能                                           │
│     - 不追求完美，只求通过测试                               │
├─────────────────────────────────────────────────────────────┤
│  3. 重构代码 (Refactor)                                     │
│     - 改善代码结构                                           │
│     - 提升可读性                                             │
│     - 保持测试通过                                           │
├─────────────────────────────────────────────────────────────┤
│  4. 重复                                                     │
└─────────────────────────────────────────────────────────────┘
```

### 2. GoogleTest 框架使用

#### 断言选择

| 断言类型 | 说明 | 使用场景 |
|---------|------|---------|
| `ASSERT_*` | 致命失败，立即中止 | 检查前置条件、初始化错误 |
| `EXPECT_*` | 非致命失败，继续报告 | 常规验证（推荐） |

```cpp
// ✅ 推荐：使用 EXPECT_* 报告更多错误
TEST(BufferPoolTest, FetchPageReturnsValidPage) {
    auto pool = BufferPool(100);
    auto page = pool.FetchPage(0);

    EXPECT_NE(page, nullptr);        // 继续报告
    EXPECT_EQ(page->GetId(), 0);     // 继续报告
    EXPECT_TRUE(page->IsPinned());   // 继续报告
}

// ❌ 避免：使用 ASSERT_* 可能错过多个错误
TEST(BufferPoolTest, BadExample) {
    auto pool = BufferPool(100);
    ASSERT_NE(pool.FetchPage(0), nullptr);  // 失败后停止
    // 后续错误无法报告
}
```

#### 测试定义

```cpp
// 简单测试
TEST(TestSuiteName, TestName) {
    // 测试代码
    EXPECT_TRUE(condition);
}

// 测试夹具
class TransactionTest : public testing::Test {
protected:
    void SetUp() override {
        // 测试前初始化
        mgr = std::make_unique<TransactionManager>();
    }

    void TearDown() override {
        // 测试后清理
        mgr->RollbackAll();
    }

    std::unique_ptr<TransactionManager> mgr;
};

TEST_F(TransactionTest, CommitPersistsChanges) {
    EXPECT_TRUE(mgr->Begin());
    EXPECT_TRUE(mgr->Commit());
}
```

#### 测试夹具模式

```cpp
// 参数化测试
class PrimeTest : public testing::TestWithParam<int> {};

TEST_P(PrimeTest, IsPrime) {
    int n = GetParam();
    EXPECT_TRUE(IsPrimeNumber(n));
}

INSTANTIATE_TEST_SUITE_P(
    PrimeValues,
    PrimeTest,
    testing::Values(2, 3, 5, 7, 11, 13, 17, 19, 23, 29)
);

// 死亡测试
TEST(DeathTest, DivisionByZero) {
    EXPECT_DEATH(Divide(1, 0), "division by zero");
}
```

### 3. 测试原则

| 原则 | 说明 |
|------|------|
| **独立性** | 测试应相互独立，避免调试困难 |
| **可重复性** | 每次运行结果一致 |
| **快速性** | 单元测试应在毫秒级完成 |
| **信息量** | 失败时提供详细上下文 |
| **可维护性** | 测试代码也应保持良好风格 |

### 4. 测试分类

| 类型 | 说明 | 示例 |
|------|------|------|
| **单元测试** | 测试单个函数/类 | `*_test.cpp` |
| **集成测试** | 测试模块交互 | `*_integration_test.cpp` |
| **边界测试** | 测试边界条件 | `*_boundary_test.cpp` |
| **性能测试** | 测量性能指标 | `*_performance_test.cpp` |
| **安全测试** | 测试安全特性 | `*_security_test.cpp` |

---

## Bazel + Clang-20 构建规范

### 1. .bazelrc 配置

```bash
# =============================================================================
# SQLCC Bazel 构建配置
# =============================================================================

# -----------------------------------------------------------------------------
# 编译器配置
# -----------------------------------------------------------------------------
build --action_env=CC=clang-20
build --action_env=CXX=clang++-20

# -----------------------------------------------------------------------------
# C++ 标准
# -----------------------------------------------------------------------------
build --cxxopt=-std=c++20
build --copt=-stdlib=libc++
build --linkopt=-stdlib=libc++
build --linkopt=-lc++abi

# -----------------------------------------------------------------------------
# 警告和错误
# -----------------------------------------------------------------------------
build --copt=-Wall
build --copt=-Wextra
build --copt=-Wshadow
build --copt=-Wnon-virtual-dtor
build --copt=-pedantic
build --copt=-Werror

# -----------------------------------------------------------------------------
# 优化配置
# -----------------------------------------------------------------------------
build:release --copt=-O2
build:release --copt=-DNDEBUG

# -----------------------------------------------------------------------------
# 调试配置
# -----------------------------------------------------------------------------
build:debug --copt=-g
build:debug --copt=-O0
build:debug --copt=-fsanitize=address

# -----------------------------------------------------------------------------
# 代码覆盖率
# -----------------------------------------------------------------------------
build:coverage --copt=-fprofile-instr-generate
build:coverage --copt=-fcoverage-mapping
build:coverage --linkopt=-fprofile-instr-generate

# -----------------------------------------------------------------------------
# 构建模式选择
# -----------------------------------------------------------------------------
# bazel build --config=release    # 发布模式
# bazel build --config=debug      # 调试模式
# bazel build --config=coverage   # 覆盖率模式
```

### 2. BUILD.bazel 模板

#### 标准模块模板

```bazel
# =============================================================================
# <模块名> 构建配置
# =============================================================================

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test", "test_suite")

package(default_visibility = ["//visibility:public"])

# -----------------------------------------------------------------------------
# 核心库
# -----------------------------------------------------------------------------
cc_library(
    name = "<module_name>",
    srcs = glob(
        ["*.cpp"],
        exclude = ["*_test.cpp", "*_main.cpp"],
    ),
    hdrs = glob(["*.h"]),
    deps = [
        # 1. 项目头文件
        "//include/exception:headers",
        "//include/utils:headers",

        # 2. 项目源码模块
        "//src/other_module:other_module",

        # 3. 第三方库
        "@com_google_googletest//:gtest",
    ],
    copts = [
        "-std=c++20",
        "-stdlib=libc++",
        "-Wall",
        "-Wextra",
        "-Werror",
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
    tags = [
        "foundation",     # Level 1: 基础组件
        "core",           # Level 2: 核心组件
        "storage",        # Level 2: 存储引擎
        "transaction",    # Level 3: 事务管理
        "sql_processing", # Level 4: SQL处理
        "network",        # Level 5: 网络通信
    ],
    timeout = "short",   # short(60s)/moderate(300s)/long(900s)
)

# -----------------------------------------------------------------------------
# 测试套件
# -----------------------------------------------------------------------------
test_suite(
    name = "all_tests",
    tests = [
        ":<module_name>_test",
    ],
)
```

#### 头文件包模板 (include/)

```bazel
# =============================================================================
# <模块名> 头文件包
# =============================================================================

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

### 3. 头文件引用规范

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

#### Bazel 标签路径规范

| 场景 | 正确 | 错误 |
|------|------|------|
| include 头文件 | `//include/core:headers` | `//include:core/header.h` |
| src 源文件 | `//src/core:core` | `//src:core/core.cpp` |
| 当前包目标 | `:target` | `//current:target` |
| 子目录 | `//src/core/utils:utils` | `//src/core:utils/target` |

#### strip_include_prefix 配置

```bazel
cc_library(
    name = "disk_manager",
    srcs = ["disk_manager.cpp"],
    hdrs = ["disk_manager.h"],
    strip_include_prefix = "src/storage_engine/disk_manager",
    include_prefix = "",  # 或设置为 "storage_engine"
    visibility = ["//visibility:public"],
    deps = [
        "//src/storage_engine:storage_engine_headers",
        "//src/exception:exception_headers",
    ],
)
```

### 4. 依赖声明规范

按以下优先级排序，每层内按字母顺序：

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

### 5. 常见 BUILD 错误与修复

#### 错误1: 头文件找不到

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
        "//include/module:headers",  # 添加头文件依赖
    ],
)
```

#### 错误2: 无效标签

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
"//include/core:headers"
```

#### 错误3: 循环依赖

**现象**:
```
Cycle in dependency graph
```

**修复**:
```bazel
# 错误：A 依赖 B，B 依赖 A
deps = ["//src/execution:execution"]  # ❌ 循环依赖

# 正确：提取公共接口到独立模块
deps = ["//src/core:core_interface"]  # ✅ 依赖抽象接口
```

---

## 重构规范

### 1. 重构原则

| 原则 | 说明 |
|------|------|
| **小步前进** | 每次只做一处改动 |
| **测试保护** | 每次更改后运行测试 |
| **渐进式** | 不追求一步到位 |
| **可逆性** | 保持代码可工作状态 |

### 2. 重构流程

```
┌─────────────────────────────────────────────────────────────────────┐
│ 阶段 1: 分析与规划                                                   │
│   - 理解现有代码                                                     │
│   - 识别代码异味                                                     │
│   - 制定重构计划                                                     │
├─────────────────────────────────────────────────────────────────────┤
│ 阶段 2: 紧急修复                                                     │
│   - 修复阻塞性错误                                                   │
│   - 添加缺失的测试                                                   │
├─────────────────────────────────────────────────────────────────────┤
│ 阶段 3: 系统性重构                                                   │
│   - 小步重构，每步测试                                               │
│   - 记录重构历史                                                     │
├─────────────────────────────────────────────────────────────────────┤
│ 阶段 4: 验证与优化                                                   │
│   - 全量测试验证                                                     │
│   - 性能测试验证                                                     │
│   - 文档更新                                                         │
└─────────────────────────────────────────────────────────────────────┘
```

### 3. 常见重构模式

| 模式 | 适用场景 |
|------|----------|
| **Extract Method** | 函数过长（> 30行） |
| **Rename Variable** | 变量名不清晰 |
| **Extract Class** | 职责过多 |
| **Move Method/Field** | 职责位置不当 |
| **Replace Conditional with Polymorphism** | 复杂 switch/if |
| **Introduce Parameter Object** | 参数过多 |
| **Replace Error Code with Exception** | 错误处理混乱 |

### 4. 重构验证

```bash
# 1. 运行所有测试
bazel test //... --test_output=errors

# 2. 检查覆盖率
bazel coverage //...

# 3. 代码质量检查
clang-tidy src/*.cpp --checks=* -p bazel-bin/

# 4. 静态分析
cppcheck --enable=all --std=c++20 src/
```

---

## 质量门禁

### 1. 提交前检查清单

- [ ] 代码编译通过 (`bazel build //...`)
- [ ] 所有测试通过 (`bazel test //...`)
- [ ] 无内存泄漏 (sanitizer)
- [ ] 代码风格一致 (`clang-format`)
- [ ] 注释完整 (Why-What-How)
- [ ] 文档已更新
- [ ] 覆盖率达标

### 2. 覆盖率要求

| 层级 | 目标覆盖率 | 最低覆盖率 | 状态 |
|------|-----------|-----------|------|
| **Level 1 Foundation** | 100% | 90% | ✅ 达标 |
| **Level 2 Core** | 80% | 70% | 🔄 进行中 |
| **Level 2 Storage Engine** | 70% | 60% | 🔄 进行中 |
| **SQL Parser** | 65% | 55% | 🔄 进行中 |
| **整体平均** | 70% | 56% | 🔄 进行中 |

### 3. 测试标签体系

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

---

## 工具链集成

### 1. 日常开发命令

```bash
# 构建
bazel build //src/<module>:<target>
bazel build //...                    # 构建所有

# 测试
bazel test //tests/<level>/<module>:all
bazel test //... --test_output=errors

# 覆盖率
bazel coverage //tests/<level>/<module>:all
bazel coverage //...

# 查询依赖
bazel query 'deps(//src/<module>:<target>)'
```

### 2. 代码质量工具

```bash
# 格式化代码
clang-format -i src/*.cpp
clang-format -i tests/*.cpp

# 静态分析
clang-tidy src/*.cpp --checks=* -p bazel-bin/

# 检查 BUILD 文件
python3 tools/bazel_code_checker.py

# 修复依赖
python3 tools/bazel_dep_fixer_enhanced.py . --dry-run
python3 tools/bazel_dep_fixer_enhanced.py .

# 检查头文件路径
python3 tools/bazel_include_fixer.py
```

### 3. 项目专用工具

```bash
# 内存安全检查
python3 scripts/memory_audit.py
bash scripts/memory_safety_audit.sh

# 测试状态跟踪
python3 scripts/test_status_tracker.py

# 覆盖率分析
bash scripts/generate_l1_complete_coverage.sh
bash scripts/generate_llvm_cov_html_report.sh //tests/... coverage_html

# 代码质量分析
python3 tools/comment_quality_analyzer.py
python3 tools/bazel_config_analyzer.py
```

### 4. CI/CD 集成

```yaml
# .github/workflows/ci.yml
name: CI

on:
  push:
    branches: [main, feature/*]
  pull_request:

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Build
        run: bazel build //...

      - name: Test
        run: bazel test //... --test_output=errors

      - name: Coverage
        run: bazel coverage //...

      - name: Quality Check
        run: |
          clang-format -i src/*.cpp
          python3 tools/bazel_code_checker.py
```

---

## 参考资源

### C++ 开发规范

| 资源 | 链接 |
|------|------|
| C++ 最佳实践 | [cpp-best-practices](https://github.com/cpp-best-practices/cppbestpractices) |
| GoogleTest 文档 | [googletest/primer.md](https://github.com/google/googletest/blob/main/docs/primer.md) |
| C++ 重构模式 | [refactoring.guru](https://refactoring.guru/refactoring) |
| 优秀 C++ 项目列表 | [awesome-cpp](https://github.com/fffaraz/awesome-cpp) |

### SQLCC 内部文档

| 资源 | 路径 |
|------|------|
| AI 开发规范 | `docs/ai_tools/AI_DEVELOPMENT_GUIDELINES.md` |
| BUILD 文件规范 | `docs/ai_tools/BUILD_FILE_SPECIFICATION.md` |
| 测试规范 | `docs/ai_tools/improvement_guide.md` |
| 重构知识库 | `docs/ai_tools/systematic_refactoring_knowledge_base.md` |
| Bazel 工具手册 | `docs/ai_tools/bazel_tools_manual.md` |
| 项目规范 | `AGENTS.md` |

### 外部工具

| 工具 | 用途 |
|------|------|
| Clang-Tidy | 静态代码分析 |
| Clang-Format | 代码格式化 |
| Cppcheck | 静态分析器 |
| Valgrind | 内存分析 |
| AddressSanitizer | 内存错误检测 |
| ThreadSanitizer | 数据竞争检测 |

---

## 附录

### A. SQLCC 项目结构

```
sqlcc/
├── src/                          # 源代码
│   ├── core/                     # 核心组件
│   ├── storage_engine/           # 存储引擎
│   │   ├── buffer_pool/          # V3 分片缓冲池
│   │   ├── b_plus_tree/          # B+ 树索引
│   │   └── disk_manager/         # 磁盘管理
│   ├── sql_parser/               # SQL 解析器
│   ├── execution/                # 执行引擎
│   ├── transaction/              # 事务管理
│   ├── network/                  # 网络通信
│   ├── exception/                # 异常处理
│   ├── logger/                   # 日志系统
│   └── utils/                    # 工具类
├── include/                      # 头文件
├── tests/                        # 测试
│   ├── level1_foundation/        # 基础层测试
│   ├── level2_core/              # 核心层测试
│   ├── level2_storage_engine/    # 存储引擎测试
│   └── ...
├── docs/                         # 文档
├── tools/                        # 开发工具
└── scripts/                      # 构建脚本
```

### B. 版本历史

| 版本 | 日期 | 更新内容 |
|------|------|---------|
| v1.3.10 | 2026-02-02 | 整合 C++ 开发、测试、重构规范 |

---

**维护者**: SQLCC AI 开发团队
**最后更新**: 2026-02-02
**版本**: v1.3.10
