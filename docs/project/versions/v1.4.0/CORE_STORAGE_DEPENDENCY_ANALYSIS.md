# SQLCC Core 和 Storage Engine 依赖问题分析

**版本**: v1.4.0  
**创建日期**: 2026-02-03 11:10  
**状态**: 分析中  
**分析人**: 高小原 🌱 + Claude (分析中)

---

## 📋 一、执行摘要

### 核心问题

李哥指出：
> "你应该先从接口抽象，剥离core和storage_engine的相互依赖开始，不然就尴尬了，错误互相耦合，永远修不好。"

### 发现的问题

| 问题 | 严重程度 | 影响 |
|------|----------|------|
| Core ↔ Execution 循环依赖 | 🔴 高 | 编译错误，错误传播 |
| Core 包含 Storage 具体实现 | 🔴 高 | 无法替换存储引擎 |
| 28+ 文件依赖 Core 具体类型 | 🟠 中 | 紧耦合，难以测试 |

---

## 📊 二、依赖关系分析

### 2.1 循环依赖链

```
┌─────────────────────────────────────────────────────────────┐
│                    循环依赖链                                │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   Core ↔ Execution (循环!)                                  │
│     ↓                                                       │
│   Storage Engine (Core 依赖具体实现)                         │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 BUILD.bazel 依赖关系

**Core 的依赖** (`src/core/BUILD.bazel`):
```python
cc_library(
    name = "core",
    deps = [
        "//src/utils:utils",
        "//src/storage_engine:storage_engine",  # ❌ 依赖 Storage
        "//src/execution:execution",             # ❌ 循环依赖!
        "//src/sql_parser:sql_parser",
        "//src/exception:exception",
        "//src/logger:logger",
    ],
)
```

**Execution 的依赖** (`src/execution/BUILD.bazel`):
```python
cc_library(
    name = "execution",
    deps = [
        "//src/utils:utils",
        "//src/storage_engine:storage_engine",
        "//src/sql_parser:sql_parser",
        "//src/core:headers",  # ❌ 依赖 Core
        "//src:wal_manager",
        "//src:error_handler",
    ],
)
```

**结果**: Core ↔ Execution 循环依赖！

### 2.3 头文件依赖（28+ 个引用）

**Execution 引用 Core 的头文件**:

| 文件 | 引用的 Core 头文件 |
|------|-------------------|
| `unified_executor.cpp` | core_database_manager.h, execution_context.h, user_manager.h, system_database.h |
| `ddl_execution_strategy.cpp` | permission_validator.h, core_database_manager.h |
| `window_function_executor.h` | core_database_manager.h, execution_context.h |
| `utility_execution_strategy.h` | execution_context.h, execution_result.h |
| `dml_execution_strategy.h` | execution_context.h |

**被引用最多的 Core 头文件** (来自重构报告):

| 排名 | 头文件 | 被引用次数 | 问题 |
|------|--------|------------|------|
| 1 | `execution_context.h` | 23 | 🔴 职责过重 |
| 2 | `core_database_manager.h` | 11 | 🔴 职责过重 |
| 3 | `execution_result.h` | 22 | ✅ 纯数据结构 |
| 4 | `user_manager.h` | 6 | 🟠 可拆分 |
| 5 | `execution_strategy.h` | 6 | 🟠 可分离 |

### 2.4 Core 包含 Storage 具体实现

**问题文件**: `core/core_database_manager.h`
```cpp
#include "../../src/storage_engine/buffer_pool/buffer_pool_sharded.h"
```

**影响**:
- Core 直接依赖 Storage 的具体实现
- 无法在不修改 Core 的情况下替换存储引擎
- 测试时无法 mock Storage 实现

---

## 🔴 三、核心问题详解

### 3.1 问题 1: Core ↔ Execution 循环依赖

**症状**:
- 编译顺序敏感
- 增量编译困难
- 错误可能在两个模块之间互相传播

**根因**:
```python
# Core BUILD.bazel 包含 Execution
"//src/execution:execution"

# Execution BUILD.bazel 包含 Core
"//src/core:headers"
```

### 3.2 问题 2: Core 包含 Storage 实现

**症状**:
- 无法替换存储引擎实现
- 单元测试无法隔离 Storage
- Core 的职责不清晰

**根因**:
```cpp
// core/core_database_manager.h
#include "storage_engine/buffer_pool/buffer_pool_sharded.h"
```

### 3.3 问题 3: execution_context.h 职责过重

**症状** (被引用 23 次):
- 包含了太多不相关的功能
- 修改会影响很多模块
- 难以理解和维护

**内容**:
- 用户身份和权限
- 数据库上下文
- 事务状态
- 执行统计
- 错误处理
- 资源管理

---

## 💡 四、解决方案：接口抽象

### 4.1 设计原则

```
依赖方向:
    
    高层模块 → 抽象接口 → 低层模块
        ↓              ↓
    Core          Storage Engine (具体实现)
    
    不再直接依赖具体实现！
```

### 4.2 抽象 Storage Engine 接口

**创建抽象基类**:

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

**修改 Core**:

```cpp
// core/core_database_manager.h (修改后)

#include "storage_engine_interface.h"  // ✅ 依赖接口

class DatabaseManager {
private:
    std::unique_ptr<storage::IStorageEngine> storage_engine_;  // ✅ 依赖抽象
};
```

### 4.3 抽象 Core 关键接口

**拆分 execution_context.h**:

```cpp
// core/user_context.h (新文件)
class IUserContext {
public:
    virtual ~IUserContext() = default;
    virtual UserId GetCurrentUser() const = 0;
    virtual bool HasPermission(Permission perm) const = 0;
};

// core/transaction_context.h (新文件)
class ITransactionContext {
public:
    virtual ~ITransactionContext() = default;
    virtual TransactionId GetCurrentTransaction() const = 0;
    virtual IsolationLevel GetIsolationLevel() const = 0;
};

// core/execution_context.h (简化为组合)
class ExecutionContext {
private:
    std::unique_ptr<IUserContext> user_context_;
    std::unique_ptr<ITransactionContext> txn_context_;
};
```

### 4.4 打破循环依赖

**方案**:

1. **移除 Core 对 Execution 的依赖**
   - 从 `src/core/BUILD.bazel` 移除 `//src/execution:execution`
   - Core 不再直接调用 Execution 的功能

2. **Execution 依赖 Core 接口而非实现**
   - 使用前向声明
   - 依赖头文件库 `//src/core:headers`
   - 不依赖 `//src/core:core` (实现库)

---

## 📝 五、重构步骤（2-4 小时粒度）

### 阶段 1: 接口抽象（8 小时）

#### 任务 1.1: 创建 Storage Engine 接口（4 小时）
- [ ] 创建 `storage_engine_interface.h`
- [ ] 定义 IStorageEngine 抽象基类
- [ ] 修改 `core_database_manager.h` 依赖接口
- [ ] 验证编译成功

#### 任务 1.2: 创建 Core 接口（4 小时）
- [ ] 创建 `user_context.h` (从 execution_context.h 拆分)
- [ ] 创建 `transaction_context.h` (从 execution_context.h 拆分)
- [ ] 简化 `execution_context.h`
- [ ] 验证编译成功

### 阶段 2: 依赖解耦（8 小时）

#### 任务 2.1: 移除 Core ↔ Execution 循环依赖（4 小时）
- [ ] 从 Core BUILD.bazel 移除 Execution 依赖
- [ ] 更新受影响的源文件
- [ ] 验证编译成功
- [ ] 运行测试

#### 任务 2.2: 隔离 Storage Engine 引用（4 小时）
- [ ] 移除 Core 对 Storage 具体实现的引用
- [ ] 使用接口替代
- [ ] 验证编译成功
- [ ] 运行测试

### 阶段 3: 验证和优化（4 小时）

#### 任务 3.1: 运行完整测试套件（2 小时）
- [ ] `bazel test //tests/level1_foundation/...`
- [ ] `bazel test //tests/level2_core/...`
- [ ] 修复发现的问题

#### 任务 3.2: 更新文档（2 小时）
- [ ] 更新 ARCHITECTURE.md
- [ ] 更新 TODO.md
- [ ] 记录重构变更

---

## 📊 六、风险评估

### 高风险任务

| 任务 | 风险 | 影响 | 缓解措施 |
|------|------|------|----------|
| 任务 1.1 | 🔴 高 | 4h 延期 | 先实现最小接口 |
| 任务 2.1 | 🔴 高 | 编译失败 | 逐步移除依赖 |
| 任务 2.2 | 🟠 中 | 行为变更 | 充分测试 |

### 回滚方案

```bash
# 如果出现问题，可以回滚
git checkout HEAD -- .
git clean -fd
```

---

## 🎯 七、验收标准

### 接口抽象完成
- [ ] `IStorageEngine` 接口定义完成
- [ ] `IUserContext` 接口定义完成
- [ ] `ITransactionContext` 接口定义完成
- [ ] Core 依赖接口，不依赖具体实现

### 依赖解耦完成
- [ ] Core BUILD.bazel 不含 `//src/execution:execution`
- [ ] 无循环依赖（bazel query 验证）
- [ ] `bazel build //...` 成功
- [ ] `bazel test //tests/...` 成功

### 错误隔离
- [ ] Storage 错误不会传播到 Core
- [ ] Core 错误不会传播到 Execution
- [ ] 错误边界清晰

---

## 📌 八、下一步行动

1. [ ] 等待 Claude 分析完成
2. [ ] 审查 Claude 的分析建议
3. [ ] 生成 GitHub Issue
4. [ ] 用 OpenCode 分析 Issue
5. [ ] 制定详细设计
6. [ ] 执行重构任务
7. [ ] 小药验证和审核 PR

---

## 🔗 九、参考文档

- [v1.3.9 TODO.md](../v1.3.9/TODO.md)
- [v1.3.9 LEVEL2_REFACTORING_REPORT.md](../v1.3.9/LEVEL2_REFACTORING_REPORT.md)
- [v1.3.10 RELEASE_NOTES.md](../v1.3.10/RELEASE_NOTES_v1.3.10.md)
- [SDD 规范](../../sdd/SPEC_DRIVEN_DEVELOPMENT.md)

---

**创建时间**: 2026-02-03 11:10  
**最后更新**: 2026-02-03 11:10

---

*我是高小原 🌱，正在分析 SQLCC 的依赖问题。*
*核心问题：Core 和 Storage Engine 的相互依赖需要通过接口抽象来剥离。*
