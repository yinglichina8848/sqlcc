# AGENTS.md - SQLCC 编码指南（AI代理专用）

## ⚠️ AI Agent 必需阅读指南

**所有参与 SQLCC 项目的 AI Agent 必须首先阅读以下文档**：

| 优先级 | 文档 | 说明 | 状态 |
|--------|------|------|------|
| 🔴 **P0** | `docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md` | **SDD 规范驱动开发指南** | ☐ |
| 🔴 **P0** | `docs/ISSUE_MULTI_AGENT_COLLABORATION.md` | **多Agent跨平台协作规范** ⭐NEW | ☐ |
| 🔴 **P0** | `docs/ai_tools/AI_COLLABORATION_GUIDE.md` | **多Agent并行协作指南** | ☐ |
| 🟡 P1 | `docs/ai_tools/AI_DEVELOPMENT_GUIDELINES.md` | AI 开发规范 | ☐ |
| 🟡 P1 | `docs/ai_tools/CPP_DEVELOPMENT_SPECIFICATION.md` | C++ 开发规范 | ☐ |

## 维护者与身份

**项目维护者（平级）**:
- OpenClaw 高小原
- Codex 项目负责人（本仓库协作负责人）

**Codex 身份**:
- 角色: 文档与协作规范维护
- 沟通: 通过 Issue/PR/文档同步

### 多Agent协作快速入门

```bash
# 1. 配置Agent身份
source scripts/sqlcc-agent-config.sh <agent-id>

# 2. 验证配置
git config user.name && git config user.email

# 3. 创建功能分支
git checkout -b feature/xxx

# 4. 开发后提交（遵循规范）
# <类型>: <描述>
# Agent: <Agent名称>
# Scope: <影响范围>
# Refs: #<issue编号>
```

### SDD 规范遵从要求

**所有 AI Agent 必须严格遵从以下规范**：

1. **任务状态机**: `OPEN → CLAIMED → WIP → DONE → FROZEN`
2. **消息协议**: 使用标准消息格式 (TASK_CLAIM, PROGRESS_UPDATE, BLOCKER_NOTIFICATION, TASK_COMPLETE)
3. **沟通频率**: 进度更新每30分钟，阻塞即时通知
4. **验收标准**: 编译通过 → 测试100% → 覆盖率达标 → 文档完整 → CHANGELOG 更新

### 多Agent协作流程

```
┌─────────────────────────────────────────────────────────────────┐
│                    多Agent协作工作流                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   Master Agent                                                  │
│   ├── 分解任务 → 分配给 Developer Agent                         │
│   ├── 汇总进度 → 监控阻塞 → 协调资源                             │
│   └── 验证交付 → 生成报告                                       │
│                                                                 │
│   Developer Agent                                               │
│   ├── 认领任务 (TASK_CLAIM)                                     │
│   ├── 定期更新进度 (PROGRESS_UPDATE)                            │
│   ├── 遇到阻塞上报 (BLOCKER_NOTIFICATION)                       │
│   └── 完成任务 (TASK_COMPLETE)                                  │
│                                                                 │
│   Tester Agent                                                  │
│   ├── 编译验证 → 运行测试 → 生成覆盖率                           │
│   └── 验证报告 → 确认验收                                       │
│                                                                 │
│   Reviewer Agent                                                │
│   ├── 代码评审 → 规范检查 → 质量把关                             │
│   └── 反馈修改 → 通过确认                                       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 快速参考命令

```bash
# 读取 AI 协作指南
cat docs/ai_tools/AI_COLLABORATION_GUIDE.md

# 读取 SDD 规范
cat docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md

# 查看当前任务状态
cat docs/sdd/refactoring/level2_core/tasks.md | grep -A 50 "任务看板"

# 检查文件冲突
python3 tools/bazel_code_checker.py --check-conflicts
```

---

## 项目概述

**SQLCC**（SQL Cloud-native Cluster）是一个企业级内存安全的云原生数据库系统，采用C++20开发，实现了完整的SQL-92标准支持和高性能存储引擎。本项目专为数据库原理教学设计，同时满足企业级应用标准。

### 核心技术栈

| 组件 | 版本/规格 | 说明 |
|------|-----------|------|
| **编程语言** | C++20 | 现代C++标准，全面使用智能指针 |
| **编译器** | Clang 20+ | LLVM工具链，libc++标准库 |
| **构建系统** | Bazel 8.5.0+ | Bzlmod依赖管理 |
| **测试框架** | Google Test 1.14.0 | 单元测试和集成测试 |
| **覆盖率工具** | LLVM 20 + llvm-cov | 代码覆盖率分析 |
| **文档工具** | Doxygen | API文档生成 |
| **OS要求** | Ubuntu 20.04+ / CentOS 8+ | Linux平台 |

### 核心模块架构

```
sqlcc/
├── src/                          # 源代码（按模块组织）
│   ├── core/                     # 核心数据库组件（DatabaseManager, UserManager等）
│   ├── storage_engine/           # 存储引擎实现
│   │   ├── buffer_pool/          # V3分片缓冲池架构
│   │   ├── b_plus_tree/          # B+树索引系统
│   │   ├── table_storage/        # 表存储管理
│   │   ├── disk_manager/         # 磁盘I/O管理
│   │   └── index_manager/        # 索引管理器
│   ├── sql_parser/               # SQL解析器（ParserNew架构）
│   │   ├── parsers/              # 各类SQL解析器
│   │   ├── ast/                  # 抽象语法树
│   │   └── function/             # 函数解析
│   ├── transaction/              # 事务管理器（ACID、WAL、2PL）
│   ├── execution_ast/            # SQL执行引擎
│   ├── network/                  # 网络通信（AES/TLS加密）
│   ├── exception/                # 异常处理系统
│   ├── logger/                   # 日志系统
│   ├── security/                 # 安全模块（RBAC、审计）
│   ├── types/                    # 类型系统
│   ├── config_manager/           # 配置管理器
│   ├── procedure/                # 存储过程
│   ├── trigger/                  # 触发器
│   ├── monitoring/               # 监控系统
│   └── utils/                    # 工具类
├── tests/                        # 分层测试架构
│   ├── level1_foundation/        # 基础层（异常、日志、配置、类型、工具）
│   ├── level2_core/              # 核心层（DB管理器、执行上下文、用户管理）
│   ├── level2_storage_engine/    # 存储引擎专项测试（缓冲池、B+树、磁盘管理）
│   ├── level3_transaction_manager/  # 事务管理、查询执行
│   ├── level4_sql_processing/    # SQL处理测试（解析器、执行器）
│   ├── level5_network/           # 网络通信、协议处理
│   ├── level6_integration/       # 端到端集成测试
│   ├── level7_integration/       # 企业级集成测试（核心、演示、性能）
│   ├── unit/                     # 单元测试（安全、权限等）
│   └── sql_parser/               # SQL解析器专项测试
├── tools/                        # 开发工具（Python脚本）
├── scripts/                      # 构建和测试脚本（40+个覆盖率脚本）
├── docs/                         # 项目文档
└── examples/                     # 示例代码
```

## 构建和测试命令

### Bazel构建

```bash
# 构建所有目标
bazel build //...

# 构建特定模块
bazel build //src/core:core
bazel build //src/storage_engine:storage_engine
bazel build //src/storage_engine/buffer_pool:buffer_pool
bazel build //src/sql_parser:sql_parser

# 清理构建缓存
bazel clean

# 强制重新构建（无缓存）
bazel build //... --nocache_test_results

# 验证构建系统
bazel build --config=clang
```

### 测试执行

```bash
# 运行所有测试
bazel test //...

# 运行特定层级测试
bazel test //tests/level1_foundation:all
bazel test //tests/level2_core:all
bazel test //tests/level2_storage_engine/b_plus_tree:all
bazel test //tests/level3_transaction_manager:all
bazel test //tests/level6_integration:all
bazel test //tests/level7_integration:all

# 运行带标签过滤的测试
bazel test //tests/... --test_tag_filters=foundation
bazel test //tests/... --test_tag_filters=core
bazel test //tests/... --test_tag_filters=storage
bazel test //tests/... --test_tag_filters=transaction
bazel test //tests/... --test_tag_filters=network
bazel test //tests/... --test_tag_filters=integration
bazel test //tests/... --test_tag_filters=-manual,-slow  # 排除手动和慢速测试

# 查看测试输出
bazel test //tests/... --test_output=all
bazel test //tests/... --test_output=errors
bazel test //tests/... --test_output=summary
```

### 覆盖率测试

```bash
# 生成覆盖率报告（LLVM工具链）
bazel coverage //...

# 覆盖率测试特定模块
bazel coverage //tests/level1_foundation:all
bazel coverage //tests/level2_storage_engine/b_plus_tree:all
bazel coverage //tests/level2_core:all

# 使用项目脚本生成Level 1覆盖率报告
bash scripts/generate_l1_complete_coverage.sh
bash scripts/generate_l1_coverage_report_v2.sh
bash scripts/generate_l1_coverage_report_pro.sh
bash scripts/generate_l1_coverage_with_source.sh

# 生成HTML覆盖率报告
bash scripts/generate_llvm_cov_html_report.sh //tests/... coverage_html

# 覆盖率分析工具
bash scripts/analyze_coverage_trends.sh
bash scripts/analyze_module_coverage.sh
bash scripts/check_coverage_quality.sh

# 综合覆盖率测试
bash scripts/run_unified_coverage.sh
bash scripts/run_comprehensive_coverage_tests.sh
```

### Python开发工具

```bash
# 检查和修复BUILD文件依赖
python3 tools/bazel_code_checker.py
python3 tools/bazel_dep_fixer_enhanced.py . --dry-run  # 预览修改
python3 tools/bazel_dep_fixer_enhanced.py .             # 应用修复

# 修复头文件路径
python3 tools/bazel_include_fixer.py
python3 tools/bazel_label_fixer_enhanced.py

# 内存安全检查
python3 scripts/memory_audit.py
python3 scripts/memory_safety_audit.sh

# 测试状态跟踪
python3 scripts/test_status_tracker.py
python3 scripts/sqlcc_test_system.py

# 代码质量分析
python3 tools/comment_quality_analyzer.py
python3 tools/bazel_config_analyzer.py
python3 tools/bazel_dependency_fixer.py
```

## 代码风格指南

### C++编码规范

#### 命名约定

| 类型 | 命名风格 | 示例 |
|------|----------|------|
| **文件名** | snake_case | `buffer_pool_sharded.cpp`, `exception.h` |
| **类名** | PascalCase | `BufferPoolSharded`, `DatabaseManager` |
| **函数/方法** | PascalCase（公有方法） | `FetchPage()`, `Initialize()` |
| **变量** | snake_case | `buffer_pool_size`, `page_id` |
| **成员变量** | snake_case + 后缀下划线 | `db_path_`, `is_closed_` |
| **常量** | kPascalCase 或 UPPER_SNAKE_CASE | `kDefaultPoolSize`, `MAX_PAGE_COUNT` |
| **命名空间** | 全小写 | `namespace sqlcc` |
| **宏/头文件保护** | UPPER_SNAKE_CASE | `SQLCC_BUFFER_POOL_SHARDED_H` |

#### 头文件保护

优先使用 `#pragma once`，或使用传统宏保护：

```cpp
#pragma once
// 或
#ifndef SQLCC_MODULE_FILENAME_H
#define SQLCC_MODULE_FILENAME_H
// ...
#endif  // SQLCC_MODULE_FILENAME_H
```

#### Include顺序

1. 对应的头文件（对于`.cpp`文件）
2. 项目头文件（使用引号 `#include "path/to/header.h"`）
3. 第三方头文件（使用尖括号 `#include <gtest/gtest.h>`）
4. 系统头文件（使用尖括号 `#include <memory>`, `#include <vector>`）

**重要**: 严格遵守include规范，禁止引用 src/ 目录外部的头文件，避免循环依赖。

#### 智能指针使用

- 使用 `std::unique_ptr` 表示独占所有权
- 使用 `std::shared_ptr` 表示共享所有权
- 使用 `std::weak_ptr` 打破循环引用
- 禁止裸指针用于所有权管理，仅用于非所有权引用
- 使用RAII模式管理资源（参见 `src/storage_engine/table_storage/page_raii.h`）

示例：
```cpp
// 正确：使用智能指针
std::unique_ptr<Page> FetchPage(int32_t page_id);
std::shared_ptr<DiskManager> disk_manager_;

// 错误：避免裸指针拥有资源
Page* page = new Page();  // 禁止！
```

#### 异常处理

- 使用异常处理异常情况，非控制流
- 自定义异常类继承自 `sqlcc::Exception`
- 使用RAII确保异常安全
- 显式检查返回值并处理错误

```cpp
// 自定义异常
class BufferPoolException : public sqlcc::Exception {
public:
    explicit BufferPoolException(const std::string& msg) : Exception(msg) {}
};

// 抛出异常
throw BufferPoolException("Page not found: " + std::to_string(page_id));
```

#### 注释规范

采用 **Why-What-How** 三层注释体系：

```cpp
/**
 * WHY: 为什么需要分片缓冲池而不是单锁设计？
 * 传统缓冲池使用单一互斥锁保护所有操作，导致高并发场景下的锁竞争激烈。
 * 分片设计通过减少锁粒度，提高并发性能。
 *
 * WHAT: 基于RocksDB风格的Sharded Buffer Pool实现
 * 特点：
 * 1. 按2^n分shard，使用page_id哈希取模定位shard
 * 2. 每个shard独立LRU + 独立mutex
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

### Bazel BUILD文件规范

- 每个目录一个 `BUILD.bazel` 文件
- 使用 `glob()` 匹配源文件时谨慎，避免包含测试文件
- 显式声明可见性 `visibility = ["//visibility:public"]`
- 使用 `test_suite` 组织相关测试
- 为测试添加有意义的标签

```python
# 示例BUILD.bazel
load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "buffer_pool",
    srcs = [
        "buffer_pool_sharded.cpp",
        "replace_strategy.cpp",
    ],
    hdrs = [
        "buffer_pool_sharded.h",
    ],
    deps = [
        "//src/utils:utils",
        "//src/storage_engine/disk_manager",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "buffer_pool_test",
    srcs = ["buffer_pool_test.cpp"],
    deps = [
        ":buffer_pool",
        "@com_google_googletest//:gtest_main",
    ],
    tags = ["storage", "foundation"],
)
```

## 测试策略

### 分层测试架构

| 层级 | 目录 | 测试范围 | 标签 |
|------|------|----------|------|
| **Level 1** | `tests/level1_foundation/` | 基础组件（异常、日志、配置、类型、工具） | `foundation` |
| **Level 2** | `tests/level2_core/` | 核心组件（DB管理器、执行上下文、用户管理） | `core` |
| **Level 2** | `tests/level2_storage_engine/` | 存储引擎（缓冲池、B+树、磁盘管理） | `storage` |
| **Level 3** | `tests/level3_transaction_manager/` | 事务管理、查询执行 | `transaction` |
| **Level 4** | `tests/level4_sql_processing/` | SQL处理（解析器、执行器） | `sql_processing` |
| **Level 5** | `tests/level5_network/` | 网络通信、协议处理 | `network` |
| **Level 6** | `tests/level6_integration/` | 端到端集成测试（分布式、网络、系统集成） | `integration` |
| **Level 7** | `tests/level7_integration/` | 企业级集成测试（核心、演示、性能） | `enterprise` |
| **Unit** | `tests/unit/` | 单元测试（安全、权限等） | `unit` |
| **SQL Parser** | `tests/sql_parser/` | SQL解析器专项测试 | `sql_parser` |

### Level 1 Foundation 测试状态（v1.3.9）

| 模块 | 测试用例数 | 通过率 | 状态 |
|------|------------|--------|------|
| **Exception** | 32 | 100% | ✅ 完整 |
| **Types** | ~60 | 100% | ✅ 完整 |
| **Logger** | ~30 | 100% | ✅ 完整 |
| **Config** | ~40 | 100% | ✅ 完整 |
| **Utils** | ~20 | 100% | ✅ 完整 |
| **总计** | **~160** | **100%** | **✅ 完整** |

### 测试标签规范

- `foundation`: 基础层测试，快速执行
- `core`: 核心模块测试
- `storage`: 存储引擎相关测试
- `b_plus_tree`: B+树索引测试
- `transaction`: 事务相关测试
- `network`: 网络通信测试
- `sql_processing`: SQL处理测试
- `integration`: 集成测试
- `enterprise`: 企业级集成测试
- `slow`: 慢速测试（CI中可能跳过）
- `manual`: 需要手动执行的测试
- `coverage`: 覆盖率相关测试
- `performance`: 性能测试

### 测试文件命名

- 单元测试：`*_test.cpp`
- 集成测试：`*_integration_test.cpp`
- 性能测试：`*_performance_test.cpp`
- 基准测试：`*_benchmark.cpp`
- 安全测试：`*_security_test.cpp`
- 边界测试：`*_boundary_test.cpp`

### Mock使用规范

- 在 `tests/level2_core/mocks/` 中定义Mock类
- Mock类命名：`Mock{InterfaceName}`
- 使用Google Mock框架
- 优先使用真实实现而非Mock（Level 1原则）

## 开发工作流

### 添加新模块流程

1. **创建目录结构**
   ```bash
   mkdir -p src/new_module
   touch src/new_module/new_module.h
   touch src/new_module/new_module.cpp
   ```

2. **编写BUILD.bazel**
   ```python
   cc_library(
       name = "new_module",
       srcs = glob(["*.cpp"]),
       hdrs = glob(["*.h"]),
       deps = ["//src/utils:utils"],
       visibility = ["//visibility:public"],
   )
   ```

3. **添加测试**
   ```bash
   mkdir -p tests/level2_core/new_module
   touch tests/level2_core/new_module/new_module_test.cpp
   ```

4. **修复依赖**
   ```bash
   python3 tools/bazel_dep_fixer_enhanced.py .
   ```

5. **验证构建和测试**
   ```bash
   bazel build //src/new_module:all
   bazel test //tests/level2_core/new_module:all
   ```

6. **生成覆盖率报告**
   ```bash
   bazel coverage //tests/level2_core/new_module:all
   ```

### 代码提交流程

1. 本地构建和测试通过
   ```bash
   bazel build //...
   bazel test //... --test_output=errors
   ```

2. 运行代码检查工具
   ```bash
   python3 tools/bazel_code_checker.py
   python3 tools/comment_quality_analyzer.py
   ```

3. 生成覆盖率报告（重大变更时）
   ```bash
   bazel coverage //tests/...
   bash scripts/generate_l1_complete_coverage.sh
   ```

4. 提交前确认
   - 所有测试通过
   - 无编译警告
   - 代码符合命名规范
   - 注释完整（Why-What-How）
   - 覆盖率达标

### 覆盖率质量门禁

当前覆盖率目标（v1.3.9）：

| 层级 | 当前覆盖率 | 目标覆盖率 | 状态 |
|------|------------|------------|------|
| **Level 1 Foundation** | ~100% | 100% | ✅ 达标 |
| **Level 2 Core** | ~60% | 70% | 🔄 进行中 |
| **Level 2 Storage Engine** | ~57% | 70% | 🔄 进行中 |
| **SQL Parser** | ~55% | 65% | 🔄 进行中 |
| **整体平均** | ~56% | 70% | 🔄 进行中 |

## CI/CD集成

### GitHub Actions工作流

位于 `.github/workflows/` 目录：

- `ci.yml`: 主CI流水线（构建、测试、覆盖率）
- `coverage.yml`: 覆盖率报告生成
- `include_check.yml`: 头文件检查
- `sqlcc-ci.yml`: SQLCC专用CI配置

### CI环境配置

```yaml
环境变量:
  BAZEL_VERSION: 8.5.0
  LLVM_VERSION: 20.1.8
  CC: clang-20
  CXX: clang++-20
  GTEST_VERSION: 1.14.0
```

### 本地CI验证

```bash
# 模拟CI构建
bazel build //src/exception:io_exception
bazel build //src/logger:logger
bazel build //src/core:core
bazel build //src/sql_parser:sql_parser

# 运行单元测试
bazel test //tests/... --test_output=errors

# 生成覆盖率报告
bash scripts/generate_llvm_cov_html_report.sh //tests/... coverage_html

# 运行质量门禁检查
bazel test //:coverage_quality_gate
```

## 安全考虑

### 内存安全

- **禁止裸指针所有权**: 使用智能指针管理动态内存
- **RAII模式**: 所有资源获取必须立即初始化
- **边界检查**: 数组和缓冲区访问必须验证边界
- **异常安全**: 确保异常发生时资源正确释放

### 并发安全

- **锁粒度**: 使用分片锁减少竞争（如BufferPoolSharded）
- **死锁预防**: 统一锁获取顺序，使用超时机制
- **原子操作**: 优先使用 `std::atomic` 而非锁
- **线程局部存储**: 适当使用TLS减少共享状态

### 数据安全

- **加密传输**: 网络通信使用AES/TLS加密
- **权限控制**: 完整的用户认证和授权系统（RBAC）
- **WAL日志**: 预写日志保证数据持久性
- **备份恢复**: 定期备份机制
- **审计跟踪**: 完整的操作审计日志

### 企业级安全特性

- **RBAC权限模型**: 基于角色的访问控制
- **权限继承**: 支持角色层次结构和权限继承
- **审计日志**: 记录所有敏感操作
- **并发访问控制**: 多用户并发访问权限控制
- **权限提升防护**: 防止权限提升攻击

## 故障排查

### 构建问题

```bash
# 清理并重新构建
bazel clean
bazel build //...

# 详细错误输出
bazel build //... --verbose_failures

# 检查依赖问题
python3 tools/bazel_code_checker.py
python3 tools/bazel_dep_fixer_enhanced.py . --dry-run

# 验证编译环境
bash scripts/validate_build_environment.sh
bash scripts/validate_compilation.sh
```

### 测试失败

```bash
# 运行单个测试查看详细输出
bazel test //tests/path/to:test_name --test_output=all

# 检查测试状态
python3 scripts/test_status_tracker.py

# 运行内存检查
python3 scripts/memory_audit.py
bash scripts/memory_safety_audit.sh

# 运行失败的测试
bazel test //tests/... --test_filter=TestName* --test_output=all
```

### 覆盖率问题

```bash
# 验证覆盖率数据完整性
bash scripts/verify_coverage_integrity.sh

# 重新收集覆盖率数据
bash scripts/collect_coverage_data.sh

# 分析覆盖率趋势
bash scripts/analyze_coverage_trends.sh

# 检查覆盖率质量
bash scripts/check_coverage_quality.sh

# 模块级覆盖率分析
bash scripts/analyze_module_coverage.sh
```

### 头文件问题

```bash
# 检查头文件路径
python3 tools/bazel_include_fixer.py

# 修复标签问题
python3 tools/bazel_label_fixer_enhanced.py

# 系统性修复
python3 tools/bazel_dep_fixer_enhanced.py .
```

## 常用资源

### 文档索引

- [项目文档索引](docs/index.md): 完整的文档导航
- [开发环境配置](docs/development/guides/DEVELOPMENT_ENVIRONMENT_SETUP.md)
- [构建和测试指南](docs/development/guides/BUILD_AND_TEST_GUIDE.md)
- [API文档](docs/api/): 类文档和接口说明
- [AI辅助开发指南](docs/ai_tools/AI_TOOLS_USAGE_GUIDE.md)
- [AI协作开发指南](docs/ai_tools/AI_COLLABORATION_GUIDE.md): ⭐ **多Agent并行协作**
- [SDD规范指南](docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md): ⭐ **规范驱动开发**
- [测试驱动开发指南](docs/development/guides/TEST_DRIVEN_DEVELOPMENT_GUIDE.md)

### 版本发布说明

- [v1.3.9 Release Notes](docs/project/versions/v1.3.9/) - Level 1 Foundation完整单元测试
- [v1.3.8 Release Notes](docs/project/versions/v1.3.8/) - SQL Parser模块化重构
- [v1.3.7 Release Notes](docs/project/versions/v1.3.7/) - Bazel构建系统重构
- [v1.3.6 Release Notes](docs/project/versions/v1.3.6/) - LLVM覆盖率工具链完善
- [CHANGELOG.md](CHANGELOG.md) - 完整变更日志

### 学习资源

- [《数据库系统原理与开发实践》](docs/textbook/《数据库系统原理与开发实践》.md)
- [源码注释指南](docs/api/code/source_code_comments_guide.md)
- [编码标准](docs/api/code/coding_standards.md)
- [API设计原则](docs/api/code/api_design_principles.md)
- [Bazel知识库](docs/ai_tools/bazel_knowledge_base.md)
- [Bazel工具手册](docs/ai_tools/bazel_tools_manual.md)

### 工具和脚本

**覆盖率工具（40+个脚本）**:
- `scripts/generate_l1_complete_coverage.sh` - Level 1完整覆盖率
- `scripts/generate_llvm_cov_html_report.sh` - LLVM HTML覆盖率报告
- `scripts/analyze_coverage_trends.sh` - 覆盖率趋势分析
- `scripts/analyze_module_coverage.sh` - 模块覆盖率分析
- `scripts/run_unified_coverage.sh` - 统一覆盖率测试
- `scripts/coverage_pipeline.sh` - 覆盖率流水线

**构建工具**:
- `tools/bazel_code_checker.py` - Bazel代码检查
- `tools/bazel_dep_fixer_enhanced.py` - 依赖修复工具
- `tools/bazel_include_fixer.py` - 头文件路径修复
- `tools/comment_quality_analyzer.py` - 注释质量分析

**测试工具**:
- `scripts/test_status_tracker.py` - 测试状态跟踪
- `scripts/sqlcc_test_system.py` - SQLCC测试系统
- `scripts/memory_audit.py` - 内存安全审计

### 示例代码

- `examples/basic_transaction_test.cpp` - 基础事务测试
- `examples/unified_executor_demo.cpp` - 统一执行器演示
- `examples/index_optimization_demo.cpp` - 索引优化演示
- `examples/demonstrate_transaction_manager.cpp` - 事务管理器演示
- `examples/aes_demo.cpp` - AES加密演示
- `examples/advanced_sql_demo.cpp` - 高级SQL演示

## 版本信息

- **当前版本**: v1.3.9
- **发布日期**: 2026-01-30
- **主要特性**: Level 1 Foundation完整单元测试（~160个测试用例，100%通过率）
- **核心成就**: SQL-92标准100%支持，真实实现测试，企业级安全特性
- **覆盖率目标**: 整体平均70%（当前56%）

---

**最后更新**: 2026-01-30
**版本**: v1.3.9
**维护者**: SQLCC开发团队
**仓库**: https://gitee.com/yinglichina/sqlcc.git
