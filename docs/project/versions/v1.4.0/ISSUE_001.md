# SQLCC v1.4.0 Issue #001: 剥离 Core 和 Storage Engine 的相互依赖

**Issue Number**: #001  
**Version**: v1.4.0  
**Created**: 2026-02-03  
**Author**: 高小原 🌱  
**Status**: Open  
**Labels**: enhancement, refactoring, architecture, critical

---

## 📋 一、Issue 概述

### 背景

李哥指出当前代码存在严重的耦合问题：
> "你应该先从接口抽象，剥离core和storage_engine的相互依赖开始，不然就尴尬了，错误互相耦合，永远修不好。"

### 核心问题

**循环依赖链**:
```
Core ↔ Execution (循环依赖!)
    ↓
Storage Engine (Core 依赖具体实现)
```

### 影响

| 问题 | 严重程度 | 影响 |
|------|----------|------|
| Core ↔ Execution 循环依赖 | 🔴 高 | 编译顺序敏感，错误传播 |
| Core 包含 Storage 具体实现 | 🔴 高 | 无法替换存储引擎 |
| 28+ 文件依赖 Core 具体类型 | 🟠 中 | 紧耦合，难以测试 |

---

## 📊 二、问题详情

### 2.1 BUILD.bazel 循环依赖

**Core 的依赖** (`src/core/BUILD.bazel`):
```python
cc_library(
    name = "core",
    deps = [
        "//src/storage_engine:storage_engine",  # ❌ 依赖 Storage
        "//src/execution:execution",             # ❌ 循环依赖!
    ],
)
```

**Execution 的依赖** (`src/execution/BUILD.bazel`):
```python
cc_library(
    name = "execution",
    deps = [
        "//src/core:headers",  # ❌ 依赖 Core
    ],
)
```

**结果**: Core ↔ Execution 循环依赖！

### 2.2 头文件依赖（28+ 个引用）

**Execution 引用 Core 的头文件**:

| 文件 | 引用的 Core 头文件 |
|------|-------------------|
| `unified_executor.cpp` | core_database_manager.h, execution_context.h, user_manager.h |
| `ddl_execution_strategy.cpp` | permission_validator.h, core_database_manager.h |
| `window_function_executor.h` | core_database_manager.h, execution_context.h |

**被引用最多的 Core 头文件** (来自 LEVEL2_REFACTORING_REPORT.md):

| 排名 | 头文件 | 被引用次数 | 问题 |
|------|--------|------------|------|
| 1 | `execution_context.h` | 23 | 🔴 职责过重 |
| 2 | `core_database_manager.h` | 11 | 🔴 职责过重 |
| 3 | `execution_result.h` | 22 | ✅ 纯数据结构 |

### 2.3 Core 包含 Storage 具体实现

**问题文件**: `core/core_database_manager.h`
```cpp
#include "../../src/storage_engine/buffer_pool/buffer_pool_sharded.h"
```

---

## 💡 三、解决方案

### 3.1 设计原则

```
依赖方向:

    高层模块 → 抽象接口 → 低层模块
        ↓              ↓
    Core          Storage Engine (具体实现)
    
    不再直接依赖具体实现！
```

### 3.2 接口抽象方案

#### 3.2.1 Storage Engine 接口

```cpp
// src/storage_engine/storage_engine_interface.h

namespace sqlcc {
namespace storage {

class IStorageEngine {
public:
    virtual ~IStorageEngine() = default;
    
    // 缓冲区管理
    virtual std::unique_ptr<Page> FetchPage(PageId page_id) = 0;
    virtual bool UnpinPage(PageId page_id, bool is_dirty) = 0;
    
    // 表操作
    virtual bool CreateTable(const std::string& table_name) = 0;
    virtual bool DropTable(const std::string& table_name) = 0;
    virtual TableStorage* GetTable(const std::string& table_name) = 0;
    
    // 事务支持
    virtual TransactionId BeginTransaction() = 0;
    virtual bool CommitTransaction(TransactionId txn_id) = 0;
    virtual bool RollbackTransaction(TransactionId txn_id) = 0;
};

}  // namespace storage
}  // namespace sqlcc
```

#### 3.2.2 Core 接口拆分

```cpp
// core/user_context.h
class IUserContext {
public:
    virtual ~IUserContext() = default;
    virtual UserId GetCurrentUser() const = 0;
    virtual bool HasPermission(Permission perm) const = 0;
};

// core/transaction_context.h
class ITransactionContext {
public:
    virtual ~ITransactionContext() = default;
    virtual TransactionId GetCurrentTransaction() const = 0;
    virtual IsolationLevel GetIsolationLevel() const = 0;
};
```

### 3.3 依赖解耦步骤

1. **移除 Core 对 Execution 的依赖**
2. **Core 依赖 Storage 接口而非实现**
3. **Execution 依赖 Core 接口而非实现**

---

## 📝 四、任务清单（SDD 规范）

### 阶段 1: 接口抽象（8 小时）

#### 任务 T-1.1: 创建 Storage Engine 接口（4 小时）
- **描述**: 创建 `IStorageEngine` 抽象基类
- **文件**: `src/storage_engine/storage_engine_interface.h`
- **输入**: `buffer_pool_sharded.h`, `table_storage.h`
- **输出**: 抽象接口定义
- **验收标准**:
  - [ ] 接口定义编译通过
  - [ ] 包含所有必要的方法声明
  - [ ] 文档完整

#### 任务 T-1.2: 创建 Core 接口（4 小时）
- **描述**: 从 `execution_context.h` 拆分出 `IUserContext` 和 `ITransactionContext`
- **文件**: `core/user_context.h`, `core/transaction_context.h`
- **输入**: `core/execution_context.h`
- **输出**: 两个新接口文件
- **验收标准**:
  - [ ] 接口定义编译通过
  - [ ] 原 `execution_context.h` 简化
  - [ ] 引用计数不增加

### 阶段 2: 依赖解耦（8 小时）

#### 任务 T-2.1: 移除 Core ↔ Execution 循环依赖（4 小时）
- **描述**: 从 Core BUILD.bazel 移除 Execution 依赖
- **文件**: `src/core/BUILD.bazel`
- **输入**: 当前 BUILD.bazel
- **输出**: 无循环依赖的 BUILD.bazel
- **验收标准**:
  - [ ] `bazel query //src/core:... rdeps //src/execution:...` 返回 0
  - [ ] `bazel build //src/core:...` 成功
  - [ ] 相关测试通过

#### 任务 T-2.2: 隔离 Storage Engine 引用（4 小时）
- **描述**: Core 依赖接口而非具体实现
- **文件**: `core/core_database_manager.h`
- **输入**: 当前实现
- **输出**: 依赖 `IStorageEngine` 的实现
- **验收标准**:
  - [ ] 无 `#include "storage_engine/buffer_pool_sharded.h"`
  - [ ] 使用 `IStorageEngine` 指针
  - [ ] `bazel build //...` 成功

### 阶段 3: 验证（4 小时）

#### 任务 T-3.1: 运行完整测试套件（2 小时）
- **描述**: 验证重构不影响功能
- **命令**: `bazel test //tests/...`
- **验收标准**:
  - [ ] Level 1 测试全部通过
  - [ ] Level 2 测试全部通过
  - [ ] 无新的编译错误

#### 任务 T-3.2: 更新文档（2 小时）
- **描述**: 记录架构变更
- **文件**: `ARCHITECTURE.md`, `TODO.md`
- **验收标准**:
  - [ ] 架构文档更新
  - [ ] TODO 完成状态更新

---

## 📊 五、验收标准

### 接口抽象完成
- [ ] `IStorageEngine` 接口定义完成
- [ ] `IUserContext` 接口定义完成
- [ ] `ITransactionContext` 接口定义完成
- [ ] Core 依赖接口，不依赖具体实现

### 依赖解耦完成
- [ ] Core BUILD.bazel 不含 `//src/execution:execution`
- [ ] 无循环依赖（`bazel query` 验证）
- [ ] `bazel build //...` 成功
- [ ] `bazel test //tests/...` 成功

### 错误隔离
- [ ] Storage 错误不会传播到 Core
- [ ] Core 错误不会传播到 Execution
- [ ] 错误边界清晰

---

## 🔗 六、参考文档

- [CORE_STORAGE_DEPENDENCY_ANALYSIS.md](CORE_STORAGE_DEPENDENCY_ANALYSIS.md) - 详细分析报告
- [v1.3.9 LEVEL2_REFACTORING_REPORT.md](../v1.3.9/LEVEL2_REFACTORING_REPORT.md) - Level 2 重构报告
- [v1.3.9 TODO.md](../v1.3.9/TODO.md) - P1 重构任务
- [SDD 规范](../../sdd/SPEC_DRIVEN_DEVELOPMENT.md)

---

## 💬 七、讨论区

### Q1: 为什么这个 Issue 这么重要？
**A**: 因为循环依赖会导致编译顺序敏感、错误传播、单元测试困难。如果不解决，后续的重构任务都无法顺利进行。

### Q2: 这个重构会影响功能吗？
**A**: 不会。这个重构只改变依赖关系，不改变功能行为。所有 public API 保持不变。

### Q3: 如何验证重构成功？
**A**: 
1. 运行 `bazel query` 检查无循环依赖
2. 运行 `bazel build` 验证编译通过
3. 运行 `bazel test` 验证测试通过

---

## 📌 八、下一步行动

1. [ ] **OpenCode 分析** - 读取本 Issue 进行验证
2. [ ] **制定详细设计** - 细化接口定义和实现步骤
3. [ ] **执行任务** - 按任务清单逐步执行
4. [ ] **小药验证** - 高小药独立验证和审核 PR
5. [ ] **合并 PR** - 审核通过后合并到主分支

---

**Labels**: enhancement, refactoring, architecture, critical, good-first-issue

**Created by**: 高小原 🌱

**Reviewers**: 
- 李哥 (架构审核)
- 高小药 (代码审核)
