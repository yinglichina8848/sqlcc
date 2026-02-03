# SDD - SQLCC v1.4.0 Core/Storage Engine 依赖解耦

**版本**: v1.4.0  
**创建日期**: 2026-02-03 11:20  
**状态**: SDD 规范制定中

---

## 📋 一、需求规范 (Requirements)

### 1.1 问题陈述

**当前问题**:
- Core 和 Execution 存在循环依赖
- Core 包含 Storage Engine 的具体实现
- 错误会在模块之间互相传播，无法隔离
- 28+ 文件依赖 Core 具体类型

**影响**:
- 编译顺序敏感
- 无法增量编译和测试
- 单元测试困难
- 维护成本高

### 1.2 需求目标

| ID | 需求描述 | 验收标准 | 优先级 |
|----|---------|---------|--------|
| REQ-001 | 打破 Core ↔ Execution 循环依赖 | `bazel query` 检查无循环依赖 | P0 |
| REQ-002 | Core 依赖 Storage 接口而非实现 | 无 `#include "storage_engine/...h"` | P0 |
| REQ-003 | 创建 `IStorageEngine` 抽象接口 | 接口编译通过，单元测试通过 | P0 |
| REQ-004 | 创建 Core 关键接口 | `IUserContext`, `ITransactionContext` 定义完成 | P1 |
| REQ-005 | 所有功能测试通过 | `bazel test //tests/...` 100% 通过 | P0 |
| REQ-006 | 文档更新 | ARCHITECTURE.md 更新完成 | P2 |

### 1.3 验收定义

**编译成功**:
```bash
# 定义: bazel build //... 成功退出 (exit code 0)
bazel build //... && echo "✅ 编译成功"
```

**接口测试成功**:
```bash
# 定义: 单元测试通过 (无 FAIL, 无 ERROR)
bazel test //tests/level2_core/... --test_output=errors
# 期望: 所有测试 PASSED
```

**循环依赖检测**:
```bash
# 定义: bazel query 返回空结果
bazel query "allpaths(//src/core:..., //src/execution:...)" 2>/dev/null | grep -q "^$" && echo "✅ 无循环依赖"
```

**接口抽象成功**:
```bash
# 定义: Core 源码中无 Storage 具体实现引用
grep -r "#include.*storage_engine.*buffer_pool" ~/sqlcc/src/core/ --include="*.h" --include="*.cpp"
# 期望: 无输出 (exit code 1)
```

---

## 🏗️ 二、设计规范 (Design)

### 2.1 架构设计

```
依赖方向 (目标):

    ┌─────────────────────────────────────────┐
    │              Execution                  │
    │         (依赖 Core 接口)                │
    └──────────────────┬──────────────────────┘
                       │
                       │ depends on
                       ▼
    ┌─────────────────────────────────────────┐
    │              Core                        │
    │    IStorageEngine ◄── 抽象接口          │
    │    IUserContext                         │
    │    ITransactionContext                  │
    └──────────────────┬──────────────────────┘
                       │
                       │ implements
                       ▼
    ┌─────────────────────────────────────────┐
    │         Storage Engine                   │
    │    BufferPool, TableStorage, etc.       │
    └─────────────────────────────────────────┘
```

### 2.2 接口定义

#### 2.2.1 IStorageEngine 接口

```cpp
// src/storage_engine/storage_engine_interface.h

#ifndef SQLCC_STORAGE_ENGINE_INTERFACE_H
#define SQLCC_STORAGE_ENGINE_INTERFACE_H

#include <memory>
#include <string>
#include <vector>
#include "../../src/page/page_id.h"

namespace sqlcc {
namespace storage {

class IStorageEngine {
public:
    virtual ~IStorageEngine() = default;
    
    // ==================== 缓冲区管理 ====================
    
    /**
     * @brief 获取页面
     * @param page_id 页面 ID
     * @return 页面指针，失败返回 nullptr
     */
    virtual std::unique_ptr<Page> FetchPage(PageId page_id) = 0;
    
    /**
     * @brief 释放页面
     * @param page_id 页面 ID
     * @param is_dirty 是否为脏页
     * @return 成功返回 true
     */
    virtual bool UnpinPage(PageId page_id, bool is_dirty) = 0;
    
    /**
     * @brief 分配新页面
     * @return 新页面 ID
     */
    virtual PageId AllocatePage() = 0;
    
    /**
     * @brief 释放页面
     * @param page_id 页面 ID
     * @return 成功返回 true
     */
    virtual bool DeallocatePage(PageId page_id) = 0;
    
    // ==================== 表操作 ====================
    
    /**
     * @brief 创建表
     * @param table_name 表名
     * @return 成功返回 true
     */
    virtual bool CreateTable(const std::string& table_name) = 0;
    
    /**
     * @brief 删除表
     * @param table_name 表名
     * @return 成功返回 true
     */
    virtual bool DropTable(const std::string& table_name) = 0;
    
    /**
     * @brief 获取表存储
     * @param table_name 表名
     * @return 表指针，不存在返回 nullptr
     */
    virtual TableStorage* GetTable(const std::string& table_name) = 0;
    
    /**
     * @brief 列出所有表
     * @return 表名列表
     */
    virtual std::vector<std::string> ListTables() = 0;
    
    // ==================== 事务支持 ====================
    
    using TransactionId = uint64_t;
    
    /**
     * @brief 开始事务
     * @return 事务 ID
     */
    virtual TransactionId BeginTransaction() = 0;
    
    /**
     * @brief 提交事务
     * @param txn_id 事务 ID
     * @return 成功返回 true
     */
    virtual bool CommitTransaction(TransactionId txn_id) = 0;
    
    /**
     * @brief 回滚事务
     * @param txn_id 事务 ID
     * @return 成功返回 true
     */
    virtual bool RollbackTransaction(TransactionId txn_id) = 0;
    
    // ==================== 生命周期 ====================
    
    /**
     * @brief 刷新所有脏页到磁盘
     * @return 成功返回 true
     */
    virtual bool FlushAllPages() = 0;
    
    /**
     * @brief 关闭存储引擎
     */
    virtual void Shutdown() = 0;
};

}  // namespace storage
}  // namespace sqlcc

#endif  // SQLCC_STORAGE_ENGINE_INTERFACE_H
```

#### 2.2.2 IUserContext 接口

```cpp
// src/core/user_context.h

#ifndef SQLCC_USER_CONTEXT_H
#define SQLCC_USER_CONTEXT_H

#include <string>
#include <vector>

namespace sqlcc {

enum class Permission {
    SELECT,
    INSERT,
    UPDATE,
    DELETE,
    CREATE,
    DROP,
    ALTER,
    INDEX,
    ALL
};

class IUserContext {
public:
    virtual ~IUserContext() = default;
    
    /**
     * @brief 获取当前用户 ID
     * @return 用户 ID
     */
    virtual uint64_t GetCurrentUser() const = 0;
    
    /**
     * @brief 获取当前用户名
     * @return 用户名
     */
    virtual std::string GetCurrentUserName() const = 0;
    
    /**
     * @brief 检查权限
     * @param perm 权限类型
     * @return 有权限返回 true
     */
    virtual bool HasPermission(Permission perm) const = 0;
    
    /**
     * @brief 获取用户角色
     * @return 角色名
     */
    virtual std::string GetUserRole() const = 0;
    
    /**
     * @brief 检查是否为超级用户
     * @return 是返回 true
     */
    virtual bool IsSuperUser() const = 0;
};

}  // namespace sqlcc

#endif  // SQLCC_USER_CONTEXT_H
```

#### 2.2.3 ITransactionContext 接口

```cpp
// src/core/transaction_context.h

#ifndef SQLCC_TRANSACTION_CONTEXT_H
#define SQLCC_TRANSACTION_CONTEXT_H

#include <string>

namespace sqlcc {

enum class IsolationLevel {
    READ_UNCOMMITTED,
    READ_COMMITTED,
    REPEATABLE_READ,
    SERIALIZABLE
};

enum class TransactionState {
    IDLE,
    ACTIVE,
    COMMITTED,
    ABORTED
};

class ITransactionContext {
public:
    virtual ~ITransactionContext() = default;
    
    /**
     * @brief 获取当前事务 ID
     * @return 事务 ID，无事务返回 0
     */
    virtual uint64_t GetCurrentTransaction() const = 0;
    
    /**
     * @brief 获取事务状态
     * @return 事务状态
     */
    virtual TransactionState GetTransactionState() const = 0;
    
    /**
     * @brief 获取隔离级别
     * @return 隔离级别
     */
    virtual IsolationLevel GetIsolationLevel() const = 0;
    
    /**
     * @brief 检查是否在事务中
     * @return 在事务中返回 true
     */
    virtual bool InTransaction() const = 0;
    
    /**
     * @brief 获取事务开始时间
     * @return 时间戳
     */
    virtual uint64_t GetTransactionStartTime() const = 0;
};

}  // namespace sqlcc

#endif  // SQLCC_TRANSACTION_CONTEXT_H
```

### 2.3 BUILD.bazel 修改

#### 2.3.1 Storage Engine 接口库

```python
# src/storage_engine/BUILD.bazel

# 存储引擎接口库 (新增)
cc_library(
    name = "storage_engine_interface",
    hdrs = ["storage_engine_interface.h"],
    deps = [
        "//src/page:page_id",
        "//src/storage:storage",
    ],
    visibility = ["//visibility:public"],
)
```

#### 2.3.2 Core 接口库

```python
# src/core/BUILD.bazel

# Core 接口库 (新增)
cc_library(
    name = "core_interface",
    hdrs = [
        "user_context.h",
        "transaction_context.h",
    ],
    deps = [],
    visibility = ["//visibility:public"],
)

# 主 Core 库 (修改)
cc_library(
    name = "core",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    deps = [
        "//src/utils:utils",
        "//src/storage_engine:storage_engine_interface",  # ✅ 改为接口
        "//src/sql_parser:sql_parser",
        "//src/exception:exception",
        "//src/logger:logger",
        "//src:permission_validator",
        "//src:error_handler",
        "//src:view_manager",
        ":core_interface",  # ✅ 新增接口依赖
    ],
    visibility = ["//visibility:public"],
)
```

---

## 📝 三、任务清单 (Tasks)

### 阶段 1: 接口抽象（8 小时）

#### 任务 T-1.1: 创建 IStorageEngine 接口（4 小时）

**输入**:
- `src/storage_engine/buffer_pool/buffer_pool.h`
- `src/storage_engine/table_storage/table_storage.h`
- `src/core/core_database_manager.h` (查看依赖)

**输出**:
- `src/storage_engine/storage_engine_interface.h` (300+ 行)

**验收标准**:

| 编号 | 标准 | 检验命令 |
|------|------|---------|
| AC-1.1.1 | 文件存在 | `test -f src/storage_engine/storage_engine_interface.h` |
| AC-1.1.2 | 编译通过 | `bazel build //src/storage_engine:storage_engine_interface` |
| AC-1.1.3 | 包含所有必要方法 | `grep -c "virtual.*= 0;" src/storage_engine/storage_engine_interface.h` (期望 ≥ 10) |
| AC-1.1.4 | 无实现代码 | `grep -c "{" src/storage_engine/storage_engine_interface.h` (期望 ≤ 20) |

**依赖**: 无  
**风险**: 低  
**同层任务**: T-1.2

---

#### 任务 T-1.2: 创建 Core 接口（4 小时）

**输入**:
- `src/core/execution_context.h`
- `src/core/user_manager.h`
- `src/core/transaction_manager.h`

**输出**:
- `src/core/user_context.h` (100+ 行)
- `src/core/transaction_context.h` (100+ 行)

**验收标准**:

| 编号 | 标准 | 检验命令 |
|------|------|---------|
| AC-1.2.1 | user_context.h 存在 | `test -f src/core/user_context.h` |
| AC-1.2.2 | transaction_context.h 存在 | `test -f src/core/transaction_context.h` |
| AC-1.2.3 | user_context.h 编译通过 | `bazel build //src/core:user_context` |
| AC-1.2.4 | transaction_context.h 编译通过 | `bazel build //src/core:transaction_context` |
| AC-1.2.5 | 接口方法完整 | `grep "virtual.*= 0;" src/core/user_context.h src/core/transaction_context.h` (期望 ≥ 8) |

**依赖**: T-1.1  
**风险**: 低  
**同层任务**: T-1.1

---

### 阶段 2: 依赖解耦（8 小时）

#### 任务 T-2.1: 移除 Core ↔ Execution 循环依赖（4 小时）

**输入**:
- `src/core/BUILD.bazel`
- `src/execution/BUILD.bazel`

**输出**:
- 修改后的 BUILD.bazel 文件

**验收标准**:

| 编号 | 标准 | 检验命令 |
|------|------|---------|
| AC-2.1.1 | Core 不含 Execution 依赖 | `grep "execution" src/core/BUILD.bazel` (期望无输出) |
| AC-2.1.2 | Core 编译通过 | `bazel build //src/core:core` |
| AC-2.1.3 | Execution 编译通过 | `bazel build //src/execution:execution` |
| AC-2.1.4 | 无循环依赖 | `bazel query "allpaths(//src/core:*, //src/execution:*)" 2>/dev/null` (期望空) |
| AC-2.1.5 | Level 1 测试通过 | `bazel test //tests/level1_foundation/... --test_output=errors` |

**依赖**: T-1.1, T-1.2  
**风险**: 高 (影响范围大)  
**回滚**: `git checkout HEAD -- src/core/BUILD.bazel src/execution/BUILD.bazel`

---

#### 任务 T-2.2: 隔离 Storage Engine 引用（4 小时）

**输入**:
- `src/core/core_database_manager.h`
- `src/core/database_manager.h`

**输出**:
- 修改后的 Core 源文件

**验收标准**:

| 编号 | 标准 | 检验命令 |
|------|------|---------|
| AC-2.2.1 | 无具体实现引用 | `grep -r "buffer_pool_sharded\|buffer_pool\.h" src/core/ --include="*.h"` (期望无输出) |
| AC-2.2.2 | 包含接口头文件 | `grep "storage_engine_interface.h" src/core/core_database_manager.h` (期望有输出) |
| AC-2.2.3 | Core 编译通过 | `bazel build //src/core:core` |
| AC-2.2.4 | DatabaseManager 使用接口 | `grep "IStorageEngine\|std::unique_ptr<storage::IStorageEngine>" src/core/core_database_manager.h` (期望有输出) |

**依赖**: T-1.1  
**风险**: 高 (影响核心类)  
**回滚**: `git checkout HEAD -- src/core/core_database_manager.h`

---

### 阶段 3: 验证（4 小时）

#### 任务 T-3.1: 运行完整测试套件（2 小时）

**输入**:
- 所有已修改的源文件

**输出**:
- 测试通过报告

**验收标准**:

| 编号 | 标准 | 检验命令 |
|------|------|---------|
| AC-3.1.1 | Level 1 测试 100% 通过 | `bazel test //tests/level1_foundation/...` (期望 exit code 0) |
| AC-3.1.2 | Level 2 测试 100% 通过 | `bazel test //tests/level2_core/...` (期望 exit code 0) |
| AC-3.1.3 | 无编译警告 | `bazel build //... 2>&1 | grep -i warning` (期望无输出) |
| AC-3.1.4 | 测试覆盖率统计 | `bazel coverage //tests/level1_foundation/...` (覆盖率 ≥ 67%) |

**依赖**: T-2.1, T-2.2  
**风险**: 中 (可能发现回归问题)

---

#### 任务 T-3.2: 更新文档（2 小时）

**输入**:
- 所有已修改的文件

**输出**:
- 更新的文档文件

**验收标准**:

| 编号 | 标准 | 检验命令 |
|------|------|---------|
| AC-3.2.1 | ARCHITECTURE.md 更新 | `grep "IStorageEngine" docs/architecture/ARCHITECTURE.md` (期望有输出) |
| AC-3.2.2 | API 文档更新 | `grep "IStorageEngine" docs/api/` (期望有输出) |
| AC-3.2.3 | CHANGELOG.md 记录 | `grep "v1.4.0" CHANGELOG.md` (期望有输出) |
| AC-3.2.4 | WORKLOG.md 完成 | `grep "✅" docs/project/versions/v1.4.0/WORKLOG.md` (期望 ≥ 4) |

**依赖**: T-2.1, T-2.2  
**风险**: 低

---

## 🎯 四、合并标准 (Merge Criteria)

### 4.1 PR 合并前置条件

| 条件 | 检验命令 | 期望结果 |
|------|---------|---------|
| 所有测试通过 | `bazel test //tests/...` | exit code 0 |
| 无循环依赖 | `bazel query` | 空结果 |
| 编译无警告 | `bazel build 2>&1 | grep -i warning` | 无输出 |
| 覆盖率达标 | `bazel coverage` | ≥ 67% |
| 代码审查通过 | 李哥 or 小药 approved | approved |

### 4.2 代码审查清单

- [ ] 接口设计合理
- [ ] 无循环依赖
- [ ] 遵循 C++20 规范
- [ ] 智能指针使用正确
- [ ] 头文件保护完整
- [ ] 注释清晰完整
- [ ] 测试覆盖新增代码

### 4.3 合并流程

```
1. 创建分支: git checkout -b feature/core-storage-decoupling
2. 执行任务: 按任务清单逐步实现
3. 本地测试: bazel test //tests/...
4. 提交 PR: git push origin feature/core-storage-decoupling
5. 代码审查: 李哥 or 小药 review
6. 问题修复: 根据审查意见修改
7. 合并 PR: squash and merge
```

---

## 📊 五、任务统计

### 时间统计

| 阶段 | 任务数 | 总时间 | 验收标准数 |
|------|--------|--------|-----------|
| 阶段 1: 接口抽象 | 2 | 8 小时 | 9 |
| 阶段 2: 依赖解耦 | 2 | 8 小时 | 9 |
| 阶段 3: 验证 | 2 | 4 小时 | 9 |
| **总计** | **6** | **20 小时** | **27** |

### 风险统计

| 风险等级 | 任务数 | 任务 |
|----------|--------|------|
| 🔴 高 | 2 | T-2.1, T-2.2 |
| 🟠 中 | 1 | T-3.1 |
| 🟢 低 | 3 | T-1.1, T-1.2, T-3.2 |

---

## 📌 六、执行顺序

```
T-1.1 (4h) ───────────────┐
                          │
T-1.2 (4h) ───────────────┼──→ T-2.1 (4h) ───→ T-3.1 (2h)
                          │         │
                          │         ↓
                          │    T-2.2 (4h) ───→ T-3.2 (2h)
                          │
关键路径: T-1.1 → T-2.1 → T-2.2 → T-3.1 (14 小时)
```

---

## 🔗 七、参考文档

- [CORE_STORAGE_DEPENDENCY_ANALYSIS.md](CORE_STORAGE_DEPENDENCY_ANALYSIS.md)
- [ISSUE_001.md](ISSUE_001.md)
- [v1.3.9 LEVEL2_REFACTORING_REPORT.md](../v1.3.9/LEVEL2_REFACTORING_REPORT.md)
- [SDD 规范模板](../../sdd/templates/design_template.md)

---

**创建时间**: 2026-02-03 11:20  
**最后更新**: 2026-02-03 11:20

---

*我是高小原 🌱，按照 SDD + TDD 方式工作。*
*每个任务都有明确的验收标准，合并前必须通过所有检查。*
