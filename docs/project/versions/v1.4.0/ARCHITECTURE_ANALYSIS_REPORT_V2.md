# SQLCC v1.4.0 架构重构 - 需求与设计文档

**版本**: v1.4.0  
**创建日期**: 2026-02-03  
**作者**: 高小原 🌱  
**状态**: 需求与设计分析

---

## 背景

2026年2月2日，高小原在阅读 SQLCC 项目源码时发现：
- Core 模块直接包含 Storage Engine 的具体类
- Execution 模块直接引用 Core 的具体类
- 多个模块之间存在循环依赖

这导致：
- 修改一个模块会影响多个其他模块
- 无法独立测试某个模块
- 编译顺序变得敏感

---

## 一、What - 我们要解决什么问题？

### 1.1 现状描述

**问题 1：Core 包含 Storage 具体实现**

```cpp
// core/core_database_manager.h
#include "../../src/storage_engine/buffer_pool/buffer_pool_sharded.h"  // 具体实现

class DatabaseManager {
private:
    BufferPoolShard* buffer_pool_;  // 直接使用具体类
};
```

**问题 2：Execution 引用 Core 具体类**

```cpp
// execution/unified_executor.cpp
#include "core/execution_context.h"  // 具体类
#include "core/core_database_manager.h"  // 具体类

class UnifiedExecutor {
private:
    ExecutionContext* context_;  // 直接使用具体类
    DatabaseManager* db_;  // 直接使用具体类
};
```

**问题 3：模块间循环依赖**

```python
# src/core/BUILD.bazel
deps = [
    "//src/execution:execution",  # Core 依赖 Execution
]

# src/execution/BUILD.bazel
deps = [
    "//src/core:headers",  # Execution 依赖 Core
]
```

### 1.2 造成的后果

| 后果 | 说明 |
|------|------|
| **紧耦合** | 修改 Core 会影响 Execution，反之亦然 |
| **难以测试** | 无法单独测试某个模块，必须加载所有依赖 |
| **编译敏感** | 必须按特定顺序编译，否则会出错 |
| **无法替换** | 无法在不修改 Core 的情况下替换 Storage 实现 |

### 1.3 我们要达成的目标

| 目标 | 具体说明 |
|------|----------|
| **解耦** | 模块之间通过接口通信，不直接依赖具体类 |
| **可测试** | 可以单独测试某个模块，使用 Mock 隔离依赖 |
| **可替换** | 可以替换某个模块的实现，不影响其他模块 |
| **无循环依赖** | 模块之间没有循环引用 |

---

## 二、Why - 为什么这样设计？

### 2.1 软件工程的基本原则

**依赖倒置原则 (Dependency Inversion Principle)**：

> 高层模块不应依赖低层模块，两者都应依赖抽象。
> 抽象不应依赖细节，细节应依赖抽象。

**我们的情况**：

```
当前（错误）:
┌─────────────┐       ┌─────────────┐
│   Core      │ ───→ │  Storage   │  # Core 依赖具体实现
└─────────────┘       └─────────────┘

正确（目标）:
┌─────────────┐       ┌─────────────┐       ┌─────────────┐
│   Core      │ ───→ │  接口层    │ ───→ │  Storage   │  # Core 依赖接口
└─────────────┘       └─────────────┘       └─────────────┘
```

### 2.2 为什么需要接口？

**例子 1：DatabaseManager 需要缓冲区管理**

```cpp
// ❌ 错误：依赖具体实现
class DatabaseManager {
private:
    BufferPoolShard* buffer_pool_;  // 依赖具体类
};

// 如果想换一种缓冲区实现，必须修改 DatabaseManager
```

```cpp
// ✅ 正确：依赖接口
class DatabaseManager {
private:
    IBufferPool* buffer_pool_;  // 依赖接口
};

// 换缓冲区实现时，不需要修改 DatabaseManager
```

**例子 2：Execution 需要执行 SQL**

```cpp
// ❌ 错误：依赖具体执行器
class UnifiedExecutor {
private:
    DDLExecutor* ddl_;  // 依赖具体类
};
```

```cpp
// ✅ 正确：依赖接口
class UnifiedExecutor {
private:
    IExecutor* executor_;  // 依赖接口
};
```

### 2.3 为什么拆分职责？

**ExecutionContext 职责过多**：

```cpp
// 当前：ExecutionContext 做了太多事情
class ExecutionContext {
private:
    UserManager* user_;        // 用户管理
    DatabaseManager* db_;      // 数据库管理
    PermissionValidator* perm_; // 权限验证
    Transaction* txn_;          // 事务管理
    // ... 还有更多
};
```

**拆分后**：

```cpp
// 每个类只做一件事
class UserContext { ... }        // 只管用户
class DatabaseOperations { ... }  // 只管数据库操作
class TransactionContext { ... }  // 只管事务
```

### 2.4 为什么需要测试隔离？

**测试现状**：

```
测试 Execution 
  ↓ 需要
加载 Core 
  ↓ 需要  
加载 Storage
  ↓ 需要
加载更多依赖...
```

**理想状态**：

```
测试 Execution (使用 Mock Core)
  ↓ 隔离
测试 Core (使用 Mock Storage)
  ↓ 隔离
测试 Storage
```

---

## 三、How - 怎样做到？

### 3.1 解决方案概览

**定义 4 个接口**：

| 接口名 | 职责 | 被谁使用 |
|--------|------|---------|
| `IBufferPool` | 缓冲区管理 | DatabaseManager |
| `ITransactionManager` | 事务管理 | DatabaseManager |
| `ITableStorage` | 表操作 | Execution |
| `IUserContext` | 用户上下文 | ExecutionContext |

**修改实现类**：

| 实现类 | 修改 |
|--------|------|
| `BufferPoolManager` | 实现 `IBufferPool` 接口 |
| `TransactionManager` | 实现 `ITransactionManager` 接口 |
| `TableStorage` | 实现 `ITableStorage` 接口 |
| `UserContext` | 新建，实现 `IUserContext` 接口 |

### 3.2 接口设计示例

**IBufferPool 接口**：

```cpp
// storage_engine/buffer_pool/buffer_pool_interface.h

namespace sqlcc {
namespace storage {

/**
 * IBufferPool - 缓冲区管理接口
 * 
 * What: 管理数据库页面的缓存
 * Why: 为 DatabaseManager 提供缓冲区功能，而不暴露具体实现
 * How: 定义 6 个抽象方法，由 BufferPoolManager 实现
 */
class IBufferPool {
public:
    virtual ~IBufferPool() = default;
    
    // 获取页面
    virtual std::unique_ptr<Page> FetchPage(PageId id) = 0;
    
    // 释放页面
    virtual bool UnpinPage(PageId id, bool dirty) = 0;
    
    // 分配页面
    virtual PageId AllocatePage() = 0;
    
    // 释放页面
    virtual bool DeallocatePage(PageId id) = 0;
    
    // 刷新所有脏页
    virtual bool FlushAll() = 0;
    
    // 关闭
    virtual void Shutdown() = 0;
};

}  // namespace storage
}  // namespace sqlcc
```

### 3.3 实现类修改示例

**BufferPoolManager 实现接口**：

```cpp
// storage_engine/buffer_pool/buffer_pool.h

namespace sqlcc {
namespace storage {

/**
 * BufferPoolManager - 缓冲区管理器
 * 
 * What: 实现 IBufferPool 接口
 * Why: 提供具体的缓冲区管理功能
 * How: 使用 LRU 缓存策略管理页面
 */
class BufferPoolManager : public IBufferPool {
public:
    // 实现 IBufferPool 接口
    std::unique_ptr<Page> FetchPage(PageId id) override;
    bool UnpinPage(PageId id, bool dirty) override;
    PageId AllocatePage() override;
    bool DeallocatePage(PageId id) override;
    bool FlushAll() override;
    void Shutdown() override;
    
private:
    // 内部实现细节...
    std::vector<std::unique_ptr<BufferPoolShard>> shards_;
};

}  // namespace storage
}  // namespace sqlcc
```

### 3.4 Core 模块修改示例

**DatabaseManager 使用接口**：

```cpp
// core/database_manager.h

namespace sqlcc {

/**
 * DatabaseManager - 数据库管理器
 * 
 * What: 管理数据库和表的创建、查询等操作
 * Why: 为上层提供统一的数据库操作接口
 * How: 组合 IBufferPool 和 ITransactionManager 接口
 */
class DatabaseManager {
public:
    // 使用接口，不再依赖具体实现
    DatabaseManager(IBufferPool* buffer_pool,
                   ITransactionManager* txn_mgr);
    
private:
    // 依赖接口，不是具体类
    IBufferPool* buffer_pool_;
    ITransactionManager* txn_mgr_;
};

}  // namespace sqlcc
```

### 3.5 依赖关系变化

**重构前**：

```
Core → Storage (具体实现)
Core → Execution (具体类)
Execution → Core (具体类)
```

**重构后**：

```
Core → 接口 (IBufferPool, ITransactionManager)
Execution → 接口 (ITableStorage, IUserContext)

接口 ← 实现 (BufferPoolManager, TransactionManager)
接口 ← 实现 (TableStorage, UserContext)
```

**无循环依赖！** ✅

---

## 四、实现步骤

### 阶段 1：创建接口（预计 2 小时）

| 步骤 | 任务 | 输出文件 |
|------|------|---------|
| 1.1 | 创建 `IBufferPool` 接口 | `storage_engine_interface.h` |
| 1.2 | 创建 `ITransactionManager` 接口 | `transaction_interface.h` |
| 1.3 | 创建 `ITableStorage` 接口 | `table_storage_interface.h` |
| 1.4 | 创建 `IUserContext` 接口 | `user_context_interface.h` |

### 阶段 2：修改实现类（预计 4 小时）

| 步骤 | 任务 | 修改内容 |
|------|------|---------|
| 2.1 | `BufferPoolManager` 实现 `IBufferPool` | 添加 `: public IBufferPool` |
| 2.2 | `TransactionManager` 实现 `ITransactionManager` | 添加 `: public ITransactionManager` |
| 2.3 | `TableStorage` 实现 `ITableStorage` | 添加 `: public ITableStorage` |
| 2.4 | 新建 `UserContext` 实现 `IUserContext` | 新建类 |

### 阶段 3：修改 Core 模块（预计 2 小时）

| 步骤 | 任务 | 修改内容 |
|------|------|---------|
| 3.1 | `DatabaseManager` 改用接口 | `BufferPoolShard*` → `IBufferPool*` |
| 3.2 | `ExecutionContext` 改用接口 | 相关类 → `IUserContext*` |

### 阶段 4：测试验证（预计 2 小时）

| 步骤 | 任务 | 验证方法 |
|------|------|---------|
| 4.1 | 编译所有模块 | `bazel build //...` |
| 4.2 | 运行所有测试 | `bazel test //...` |
| 4.3 | 检查无循环依赖 | `bazel query` |

---

## 五、验收标准

### 5.1 功能标准

| 标准 | 检验方法 |
|------|---------|
| 所有接口方法都有实现 | `grep "override"` 返回 24 个结果 |
| Core 不包含 Storage 具体类 | `grep "buffer_pool_sharded" src/core/*.h` 无输出 |
| 无循环依赖 | `bazel query` 返回空 |

### 5.2 质量标准

| 标准 | 检验方法 |
|------|---------|
| 编译成功 | `bazel build //...` 退出码 0 |
| 测试通过 | `bazel test //...` 100% PASS |
| 覆盖率不降 | `bazel coverage` ≥ 67% |

---

## 六、文档说明

### 6.1 文档面向谁？

| 读者 | 说明 |
|------|------|
| **高小彝** | 需要理解为什么要这样做，才能正确实现 |
| **高小药** | 需要理解整体设计，才能正确编码 |
| **审核者** | 需要清晰的验收标准，才能判断是否完成 |

### 6.2 文档结构

| 章节 | What/Why/How |
|------|---------------|
| 一、What | 我们要解决什么问题？ |
| 二、Why | 为什么这样设计？ |
| 三、How | 怎样做到？ |
| 四、实现步骤 | 具体怎么做？ |
| 五、验收标准 | 怎样判断完成？ |

---

## 七、参考信息

### 7.1 相关文档

- `docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md` - SDD 规范
- `docs/ai_tools/CPP_DEVELOPMENT_SPECIFICATION.md` - C++ 开发规范
- `docs/project/versions/v1.3.9/LEVEL2_REFACTORING_REPORT.md` - 前期重构报告

### 7.2 相关人员

| 人员 | 角色 |
|------|------|
| 李哥 | 架构师，审核设计 |
| 高小原 | 主开发，执行重构 |
| 高小彝 | 协助开发，学习实践 |
| 高小药 | 协助开发，学习实践 |

---

## 八、总结

### 8.1 核心要点

| 要点 | 说明 |
|------|------|
| **What** | 解决 Core ↔ Storage 的紧耦合问题 |
| **Why** | 实现依赖倒置，提高可测试性、可维护性 |
| **How** | 定义 4 个接口，修改 4 个实现类 |

### 8.2 预期结果

| 指标 | 重构前 | 重构后 |
|------|--------|--------|
| 循环依赖 | 有 | 无 |
| 接口数 | 0 | 4 |
| Core ↔ Storage 依赖 | 具体实现 | 接口 |
| 可测试性 | 困难 | 容易 |

---

**文档版本**: 1.0  
**创建时间**: 2026-02-03  
**最后更新**: 2026-02-03  
**作者**: 高小原 🌱

---

## 附录：面向新手的说明

### A. 什么是接口？

**简单理解**：接口是一份合同，规定"能做什么"，不规定"怎么做"。

**生活例子**：空调的遥控器

```
遥控器（接口）:
  - 按"开" → 空调开机
  - 按"关" → 空调关机
  - 按"调温" → 调整温度

不管空调内部是定频还是变频，遥控器都能用。

在代码中：
  DatabaseManager 使用 IBufferPool 接口
  不管底层是 BufferPoolManager 还是其他实现，都能正常工作
```

### B. 为什么要解耦？

**简单理解**：减少"我改变会影响你"的情况。

**生活例子**：手机和充电器

```
紧密耦合（不好）:
  手机 ←→ 特定充电器
  只有原装充电器能充电

松散耦合（好）:
  手机 ←→ USB-C 接口
  任何 USB-C 充电器都能用

在代码中：
  Core 使用 IBufferPool 接口
  任何实现 IBufferPool 的类都能替换
```

### C. 怎样理解 What/Why/How？

| 维度 | 问题 | 例子 |
|------|------|------|
| **What** | 要做什么？ | 创建 4 个接口 |
| **Why** | 为什么做？ | 实现依赖倒置，解耦 |
| **How** | 怎样做？ | 定义抽象方法，实现类实现接口 |

---

**李哥，这样写是否更清晰？让高小彝和高小药也能看懂？**

请审查后告诉我：
- 还需要补充什么？
- 哪些地方解释不够清楚？
- 可以开始实现了吗？💪
