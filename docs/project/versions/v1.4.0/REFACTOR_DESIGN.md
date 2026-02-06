# SQLCC v1.4.0 重构设计说明（Core/Execution/Storage 解耦）

**版本**: v1.4.0  
**日期**: 2026-02-04  
**状态**: Draft

---

## 1. 重构目标

- 打破 `core` 与 `execution` 循环依赖
- 用接口替代对 `storage_engine` 具体实现的直接依赖
- 降低核心模块的编译耦合度
- 降低测试与实现的绑定

---

## 2. 现状问题

- `src/core/BUILD.bazel` 依赖 `//src/execution:execution`
- `core` 引用 `storage_engine` 的具体头文件
- `core` 与 `storage_engine` 之间存在编译链条过长问题

---

## 3. 设计方案概述

### 3.1 依赖关系调整（目标）

```
Execution -> Core Interface
Core      -> Storage Interface
Storage   -> Implements Interfaces
```

### 3.2 核心接口抽象

- `IStorageEngine`
- `IUserContext`
- `ITransactionContext`

### 3.3 代码层约束

- `core` 不允许直接 include `storage_engine/*` 具体实现头文件
- `execution` 只依赖 `core_interface`

---

## 4. 变更范围

### 4.1 预计新增

- `src/storage_engine/storage_engine_interface.h`
- `src/core/user_context.h`
- `src/core/transaction_context.h`

### 4.2 预计修改

- `src/core/BUILD.bazel`
- `src/execution/BUILD.bazel`
- `src/storage_engine/BUILD.bazel`
- `src/core/core_database_manager.h`

---

## 5. 约束与不做项

- 不改现有业务逻辑
- 不改接口实现细节
- 不触发模块 API 大规模重命名

---

## 6. 验收标准

- `bazel build //src/core:core`
- `bazel build //src/execution:execution`
- `bazel query "allpaths(//src/core:core, //src/execution:execution)"` 为空
- 新增接口通过编译

---

**维护者**: OpenClaw 高小原 / Codex 项目负责人  
