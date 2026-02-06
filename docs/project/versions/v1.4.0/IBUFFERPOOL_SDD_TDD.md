# IBufferPool 接口变更 SDD/TDD 定义（v1.4.0）

**版本**: v1.4.0  
**日期**: 2026-02-04  
**状态**: Draft  

---

## 1. 需求规范 (SDD)

### 1.1 背景

当前 `buffer_pool` 模块的接口依赖具体实现，导致核心模块难以抽象与替换。

### 1.2 需求列表

| ID | 需求描述 | 验收标准 | 优先级 |
|----|----------|----------|--------|
| REQ-BP-001 | 抽象缓冲池接口 | 新增 `IBufferPool` 接口并可编译 | P0 |
| REQ-BP-002 | Core 不依赖具体 BufferPool 实现 | `rg -n "buffer_pool_sharded" src/core` 无输出 | P0 |
| REQ-BP-003 | 行为保持一致 | 现有单元测试全部通过 | P0 |

---

## 2. 设计规范 (SDD)

### 2.1 接口草案

```cpp
class IBufferPool {
public:
  virtual ~IBufferPool() = default;
  virtual std::unique_ptr<Page> FetchPage(PageId page_id) = 0;
  virtual bool UnpinPage(PageId page_id, bool is_dirty) = 0;
  virtual PageId AllocatePage() = 0;
  virtual bool DeallocatePage(PageId page_id) = 0;
  virtual bool FlushAllPages() = 0;
};
```

### 2.2 依赖关系

```
Core -> IBufferPool (interface)
BufferPoolSharded -> implements IBufferPool
```

---

## 3. 任务分解 (SDD)

### T-BP-1: 创建 IBufferPool 接口
- 输出: `src/storage_engine/buffer_pool/buffer_pool_interface.h`
- 验收: `bazel build //src/storage_engine/buffer_pool:buffer_pool_interface`

### T-BP-2: 迁移 Core 引用
- 输出: Core 引用接口而非具体实现
- 验收: `rg -n "buffer_pool_sharded" src/core` 无输出

### T-BP-3: 测试验证
- 验收: `bazel test //tests/level2_storage_engine/buffer_pool/...`

---

## 4. 测试定义 (TDD)

### 4.1 Red/Green/Refactor 记录

```
Test: BufferPool_FetchPage_NotFound
Red: bazel test //tests/level2_storage_engine/buffer_pool:buffer_pool_test --test_filter=FetchPage_NotFound
Green: commit after fix
Refactor: rerun tests
```

### 4.2 最小覆盖矩阵

| 类型 | 用例 |
|------|------|
| 正常 | FetchPage_Valid | 
| 边界 | FetchPage_NotFound |
| 异常 | UnpinPage_Invalid |

---

**维护者**: OpenClaw 高小原 / Codex 项目负责人
