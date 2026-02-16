# SQLCC v1.4.0 Issue #002: IBufferPool 接口设计讨论

**Issue Number**: #002  
**Version**: v1.4.0  
**创建日期**: 2026-02-03  
**作者**: 高小原 🌱  
**状态**: 开放讨论  
**标签**: interface, design, buffer_pool

---

## 📋 一、问题背景

### 1.1 当前问题

**位置**: `core/core_database_manager.h`

```cpp
// ❌ 问题: Core 包含 Storage 的具体实现
#include "../../src/storage_engine/buffer_pool/buffer_pool_sharded.h"

class DatabaseManager {
private:
    BufferPoolShard* buffer_pool_;  // 具体实现
};
```

**影响**:
- Core ↔ Storage 紧耦合
- 无法替换 Storage 实现
- 无法 Mock 测试

### 1.2 解决方案

定义 `IBufferPool` 接口，让 Core 只依赖接口。

---

## 📐 二、接口设计草案

### 2.1 当前草案

```cpp
// storage_engine/buffer_pool/buffer_pool_interface.h

class IBufferPool {
public:
    virtual ~IBufferPool() = default;
    
    // 页面管理 (4 个方法)
    virtual std::unique_ptr<Page> FetchPage(PageId id) = 0;
    virtual bool UnpinPage(PageId id, bool dirty) = 0;
    virtual PageId AllocatePage() = 0;
    virtual bool DeallocatePage(PageId id) = 0;
    
    // 生命周期 (2 个方法)
    virtual bool FlushAll() = 0;
    virtual void Shutdown() = 0;
};
```

### 2.2 问题讨论

#### Q1: 返回值类型是否合适？

**当前设计**:
```cpp
virtual std::unique_ptr<Page> FetchPage(PageId id) = 0;
```

**优点**:
- `unique_ptr` 保证内存安全
- 自动管理生命周期

**潜在问题**:
- 每次调用都分配内存
- 性能开销

**替代方案**:
```cpp
// 返回原始指针，调用方负责释放
virtual Page* FetchPage(PageId id) = 0;

// 或者返回 shared_ptr
virtual std::shared_ptr<Page> FetchPage(PageId id) = 0;
```

#### Q2: 页面分配失败如何表示？

**当前设计**:
```cpp
virtual PageId AllocatePage() = 0;
```

**问题**: 返回什么值表示失败？

**方案 A**: 返回特殊值
```cpp
virtual PageId AllocatePage() = 0;
// 返回 kInvalidPageId (如 0 或 UINT64_MAX) 表示失败
```

**方案 B**: 抛出异常
```cpp
virtual PageId AllocatePage() = 0;
// 抛出 NoSpaceException 表示失败
```

**方案 C**: 返回 optional
```cpp
virtual std::optional<PageId> AllocatePage() = 0;
// C++17，需要升级编译器
```

#### Q3: FlushAll 返回值是否必要？

**当前设计**:
```cpp
virtual bool FlushAll() = 0;
```

**问题**: 返回 false 意味着什么？

- 部分页面写入失败？
- 所有页面都写入失败？
- 如何知道哪些页面失败？

#### Q4: Shutdown 需要参数吗？

**当前设计**:
```cpp
virtual void Shutdown() = 0;
```

**问题**: 是否需要：
- 强制刷新所有脏页？
- 等待所有挂起操作完成？

---

## 🎯 三、讨论议题

### 议题 1: 返回值类型

请选择或提出建议：

| 选项 | 说明 | 优点 | 缺点 |
|------|------|------|------|
| A. unique_ptr | 当前草案 | 内存安全 | 性能开销 |
| B. raw pointer | 返回原始指针 | 性能高 | 需要手动管理 |
| C. shared_ptr | 共享所有权 | 引用计数 | 性能开销更大 |

**我的建议**: A. unique_ptr（内存安全优先）

---

### 议题 2: 分配失败处理

请选择或提出建议：

| 选项 | 说明 | 优点 | 缺点 |
|------|------|------|------|
| A. 特殊值 | 返回 kInvalidPageId | 简单 | 需要约定特殊值 |
| B. 异常 | 抛出 NoSpaceException | 语义清晰 | 异常开销 |
| C. optional | 返回 std::optional | 语义清晰 | 需要 C++17 |

**我的建议**: A. 特殊值（保持简单）

---

### 议题 3: FlushAll 语义

请选择或提出建议：

| 选项 | 说明 | 优点 | 缺点 |
|------|------|------|------|
| A. 返回 bool | 简单表示成功/失败 | 简单 | 不知道哪些失败 |
| B. 返回数量 | 返回成功写入的页面数 | 精确 | 需要额外逻辑 |
| C. 无返回值 | 静默刷新 | 简单 | 无法知道状态 |

**我的建议**: A. 返回 bool（简化接口）

---

### 议题 4: Shutdown 行为

请选择或提出建议：

| 选项 | 说明 | 优点 | 缺点 |
|------|------|------|------|
| A. 静默关闭 | 自动刷新，不返回状态 | 简单 | 无法知道状态 |
| B. 强制刷新 | 刷新所有脏页 | 完整性 | 可能阻塞 |
| C. 参数控制 | 传入 flush 参数 | 灵活 | 接口复杂 |

**我的建议**: A. 静默关闭（简化接口）

---

## 📊 四、最终草案（待讨论后确定）

```cpp
// storage_engine/buffer_pool/buffer_pool_interface.h

namespace sqlcc {
namespace storage {

class Page;
using PageId = uint64_t;

constexpr PageId kInvalidPageId = 0;

/**
 * IBufferPool - 缓冲区管理接口
 * 
 * 职责: 管理数据库页面的缓存和生命周期
 */
class IBufferPool {
public:
    virtual ~IBufferPool() = default;
    
    // ==================== 页面管理 ====================
    
    /**
     * FetchPage - 获取页面
     * @param page_id 页面 ID
     * @return 页面指针，失败返回 nullptr
     */
    virtual std::unique_ptr<Page> FetchPage(PageId page_id) = 0;
    
    /**
     * UnpinPage - 释放页面
     * @param page_id 页面 ID
     * @param is_dirty 是否为脏页
     * @return 成功返回 true
     */
    virtual bool UnpinPage(PageId page_id, bool is_dirty) = 0;
    
    /**
     * AllocatePage - 分配新页面
     * @return 新页面 ID，失败返回 kInvalidPageId
     */
    virtual PageId AllocatePage() = 0;
    
    /**
     * DeallocatePage - 释放页面
     * @param page_id 页面 ID
     * @return 成功返回 true
     */
    virtual bool DeallocatePage(PageId page_id) = 0;
    
    // ==================== 生命周期 ====================
    
    /**
     * FlushAll - 刷新所有脏页
     * @return 全部成功返回 true，有失败返回 false
     */
    virtual bool FlushAll() = 0;
    
    /**
     * Shutdown - 关闭缓冲区
     * 自动刷新所有脏页，不返回状态
     */
    virtual void Shutdown() = 0;
};

}  // namespace storage
}  // namespace sqlcc
```

---

## 📝 五、待确认问题

1. **返回值类型**: unique_ptr vs raw pointer vs shared_ptr？
2. **分配失败**: 特殊值 vs 异常 vs optional？
3. **FlushAll 返回值**: bool vs 数量？
4. **Shutdown 行为**: 静默 vs 强制？

---

## 🔗 六、相关文档

- [ARCHITECTURE_ANALYSIS_REPORT.md](ARCHITECTURE_ANALYSIS_REPORT.md)
- [CORE_DESIGN_REPORT.md](CORE_DESIGN_REPORT.md)

---

## 💬 讨论区

请李哥和各位审查以下问题：

1. **这个接口设计是否满足需求？**
2. **4 个议题的选择？**
3. **是否有遗漏的方法？**
4. **方法签名是否合适？**

---

**李哥，请审查这个 ISSUE，提出你的意见！**

讨论清楚了，我们再动手实现！💪

---

**Labels**: interface, design, buffer_pool, discussion-needed

**Created by**: 高小原 🌱
