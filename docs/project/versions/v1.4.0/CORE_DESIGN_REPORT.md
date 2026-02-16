# SQLCC v1.4.0 核心代码设计报告

**版本**: v1.4.0  
**创建日期**: 2026-02-03  
**作者**: 高小原 🌱  
**状态**: 设计阶段

---

## 📋 一、设计目标

根据架构分析报告，确定以下设计目标：

1. **接口定义**: 4 个最小接口，24 个抽象方法
2. **实现分离**: 定义和实现分离，通过接口解耦
3. **依赖隔绝**: 高层模块只依赖接口，不依赖具体实现
4. **可测试性**: 支持 Mock 接口进行单元测试

---

## 🏗️ 二、接口设计

### 2.1 IBufferPool 接口

**文件**: `storage_engine/buffer_pool/buffer_pool_interface.h`

```cpp
namespace sqlcc {
namespace storage {

class Page;
using PageId = uint64_t;

/**
 * IBufferPool - 缓冲区管理接口
 * 
 * 设计理由:
 * - Core 模块需要缓冲区管理功能
 * - 但不应依赖具体的 BufferPoolManager 实现
 * - 通过接口抽象，可以替换实现而不影响 Core
 */
class IBufferPool {
public:
    virtual ~IBufferPool() = default;
    
    // ==================== 页面管理 ====================
    
    /**
     * FetchPage - 获取页面
     * 
     * @param page_id 页面 ID
     * @return 页面指针，失败返回 nullptr
     * 
     * 设计考虑:
     * - 返回 unique_ptr 保证内存安全
     * - 返回 nullptr 表示页面不存在
     */
    virtual std::unique_ptr<Page> FetchPage(PageId page_id) = 0;
    
    /**
     * UnpinPage - 释放页面
     * 
     * @param page_id 页面 ID
     * @param is_dirty 页面是否被修改
     * @return 成功返回 true
     * 
     * 设计考虑:
     * - 简单的布尔返回值表示操作结果
     * - 脏页标记用于后续刷盘
     */
    virtual bool UnpinPage(PageId page_id, bool is_dirty) = 0;
    
    /**
     * AllocatePage - 分配新页面
     * 
     * @return 新页面 ID
     * 
     * 设计考虑:
     * - 返回 PageId 而不是指针，由内部管理
     * - 分配失败返回特殊值 (如 kInvalidPageId)
     */
    virtual PageId AllocatePage() = 0;
    
    /**
     * DeallocatePage - 释放页面
     * 
     * @param page_id 页面 ID
     * @return 成功返回 true
     */
    virtual bool DeallocatePage(PageId page_id) = 0;
    
    // ==================== 生命周期 ====================
    
    /**
     * FlushAll - 刷新所有脏页
     * 
     * @return 成功返回 true
     * 
     * 设计考虑:
     * - 用于检查点或关闭时
     * - 返回 false 表示有页面写入失败
     */
    virtual bool FlushAll() = 0;
    
    /**
     * Shutdown - 关闭缓冲区
     * 
     * 设计考虑:
     * - 析构前调用，清理资源
     * - 不返回状态，简化接口
     */
    virtual void Shutdown() = 0;
};

}  // namespace storage
}  // namespace sqlcc
```

### 2.2 ITransactionManager 接口

**文件**: `transaction/transaction_interface.h`

```cpp
namespace sqlcc {

/**
 * ITransactionManager - 事务管理接口
 * 
 * 设计理由:
 * - 数据库操作需要事务支持
 * - Core 模块需要事务管理功能
 * - 通过接口抽象，支持不同的事务实现
 */
class ITransactionManager {
public:
    using TransactionId = uint64_t;
    
    virtual ~ITransactionManager() = default;
    
    // ==================== 事务生命周期 ====================
    
    /**
     * Begin - 开始事务
     * 
     * @return 事务 ID
     * 
     * 设计考虑:
     * - 返回唯一的事务 ID
     * - 内部自动初始化事务状态
     */
    virtual TransactionId Begin() = 0;
    
    /**
     * Commit - 提交事务
     * 
     * @param txn_id 事务 ID
     * @return 成功返回 true
     * 
     * 设计考虑:
     * - 简单的布尔返回值
     * - 内部自动释放锁
     */
    virtual bool Commit(TransactionId txn_id) = 0;
    
    /**
     * Rollback - 回滚事务
     * 
     * @param txn_id 事务 ID
     * @return 成功返回 true
     */
    virtual bool Rollback(TransactionId txn_id) = 0;
    
    // ==================== 锁管理 ====================
    
    /**
     * Lock - 获取锁
     * 
     * @param txn_id 事务 ID
     * @param resource_id 资源 ID
     * @return 成功返回 true
     * 
     * 设计考虑:
     * - 简化的锁接口
     * - 内部自动处理死锁检测
     */
    virtual bool Lock(TransactionId txn_id, uint64_t resource_id) = 0;
    
    /**
     * Unlock - 释放锁
     * 
     * @param txn_id 事务 ID
     * 
     * 设计考虑:
     * - 简化接口，释放事务的所有锁
     */
    virtual void Unlock(TransactionId txn_id) = 0;
};

}  // namespace sqlcc
```

### 2.3 ITableStorage 接口

**文件**: `storage_engine/table_storage/table_storage_interface.h`

```cpp
namespace sqlcc {
namespace storage {

class Record;
struct Schema;
struct Predicate;

/**
 * ITableStorage - 表存储接口
 * 
 * 设计理由:
 * - Execution 模块需要表操作功能
 * - 但不应依赖具体的 TableStorage 实现
 * - 通过接口抽象，支持不同的存储引擎
 */
class ITableStorage {
public:
    virtual ~ITableStorage() = default;
    
    // ==================== CRUD 操作 ====================
    
    /**
     * Insert - 插入记录
     * 
     * @param record 记录
     * @return 成功返回 true
     */
    virtual bool Insert(const Record& record) = 0;
    
    /**
     * Update - 更新记录
     * 
     * @param key 键
     * @param record 新记录
     * @return 成功返回 true
     */
    virtual bool Update(const std::string& key, const Record& record) = 0;
    
    /**
     * Delete - 删除记录
     * 
     * @param key 键
     * @return 成功返回 true
     */
    virtual bool Delete(const std::string& key) = 0;
    
    /**
     * Scan - 扫描所有记录
     * 
     * @return 记录列表
     * 
     * 设计考虑:
     * - 返回所有记录用于全表扫描
     * - 性能考虑: 大表可能返回大量数据
     */
    virtual std::vector<Record> Scan() = 0;
    
    // ==================== 查询操作 ====================
    
    /**
     * Select - 按条件查询
     * 
     * @param predicate 谓词
     * @return 匹配的记录
     */
    virtual std::vector<Record> Select(const Predicate& predicate) = 0;
    
    // ==================== 模式信息 ====================
    
    /**
     * GetSchema - 获取表模式
     * 
     * @return 模式
     */
    virtual Schema GetSchema() const = 0;
    
    /**
     * GetName - 获取表名
     * 
     * @return 表名
     */
    virtual std::string GetName() const = 0;
};

}  // namespace storage
}  // namespace sqlcc
```

### 2.4 IUserContext 接口

**文件**: `core/user_context_interface.h`

```cpp
namespace sqlcc {

enum class Permission {
    SELECT,
    INSERT,
    UPDATE,
    DELETE,
    CREATE,
    DROP,
};

enum class Role {
    ADMIN,
    USER,
    GUEST,
};

/**
 * IUserContext - 用户上下文接口
 * 
 * 设计理由:
 * - ExecutionContext 需要用户信息
 * - 从 ExecutionContext 拆分出用户相关功能
 * - 支持权限验证和用户管理
 */
class IUserContext {
public:
    virtual ~IUserContext() = default;
    
    /**
     * GetUserId - 获取用户 ID
     * 
     * @return 用户 ID
     */
    virtual uint64_t GetUserId() const = 0;
    
    /**
     * GetUserName - 获取用户名
     * 
     * @return 用户名
     */
    virtual std::string GetUserName() const = 0;
    
    /**
     * HasPermission - 检查权限
     * 
     * @param perm 权限
     * @return 有权限返回 true
     */
    virtual bool HasPermission(Permission perm) const = 0;
    
    /**
     * GetRole - 获取角色
     * 
     * @return 角色
     */
    virtual Role GetRole() const = 0;
    
    /**
     * IsAuthenticated - 是否已认证
     * 
     * @return 已认证返回 true
     */
    virtual bool IsAuthenticated() const = 0;
};

}  // namespace sqlcc
```

---

## 📦 三、实现设计

### 3.1 BufferPoolManager 实现 IBufferPool

**文件**: `storage_engine/buffer_pool/buffer_pool.h`

```cpp
namespace sqlcc {
namespace storage {

class BufferPoolManager : public IBufferPool {
public:
    BufferPoolManager(size_t pool_size, size_t shard_count);
    ~BufferPoolManager() override;
    
    // 实现 IBufferPool 接口
    std::unique_ptr<Page> FetchPage(PageId page_id) override;
    bool UnpinPage(PageId page_id, bool is_dirty) override;
    PageId AllocatePage() override;
    bool DeallocatePage(PageId page_id) override;
    bool FlushAll() override;
    void Shutdown() override;
    
private:
    // 内部实现细节
    std::vector<std::unique_ptr<BufferPoolShard>> shards_;
    size_t page_size_;
    // ...
};

}  // namespace storage
}  // namespace sqlcc
```

### 3.2 DatabaseManager 使用接口

**文件**: `core/database_manager.h`

```cpp
namespace sqlcc {

class DatabaseManager {
public:
    /**
     * 构造函数
     * 
     * @param buffer_pool 缓冲区接口
     * @param txn_manager 事务接口
     */
    DatabaseManager(IBufferPool* buffer_pool, 
                    ITransactionManager* txn_manager);
    
    // 其他方法...
    
private:
    IBufferPool* buffer_pool_;  // ✅ 依赖接口
    ITransactionManager* txn_manager_;  // ✅ 依赖接口
    // 不再包含具体实现
};

}  // namespace sqlcc
```

### 3.3 ExecutionContext 使用接口

**文件**: `core/execution_context.h`

```cpp
namespace sqlcc {

class ExecutionContext {
public:
    ExecutionContext(IUserContext* user_context);
    
    // 其他方法...
    
private:
    IUserContext* user_context_;  // ✅ 依赖接口
    // 其他成员...
};

}  // namespace sqlcc
```

---

## 🔧 四、Bazel 构建设计

### 4.1 接口库

```python
# storage_engine/buffer_pool/BUILD.bazel

cc_library(
    name = "buffer_pool_interface",
    hdrs = ["buffer_pool_interface.h"],
    deps = [
        "//src/page:page",
    ],
    visibility = ["//visibility:public"],
)

# transaction/BUILD.bazel

cc_library(
    name = "transaction_interface",
    hdrs = ["transaction_interface.h"],
    visibility = ["//visibility:public"],
)

# storage_engine/table_storage/BUILD.bazel

cc_library(
    name = "table_storage_interface",
    hdrs = ["table_storage_interface.h"],
    deps = [
        "//src/page:page",
    ],
    visibility = ["//visibility:public"],
)

# core/BUILD.bazel

cc_library(
    name = "user_context_interface",
    hdrs = ["user_context_interface.h"],
    visibility = ["//visibility:public"],
)
```

### 4.2 Core 模块依赖更新

```python
# src/core/BUILD.bazel (修改后)

cc_library(
    name = "core",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.h"]),
    deps = [
        "//src/utils:utils",
        "//src/storage_engine/buffer_pool:buffer_pool_interface",  # ✅ 接口
        "//src/transaction:transaction_interface",  # ✅ 接口
        "//src/sql_parser:sql_parser",
        "//src/exception:exception",
        "//src/logger:logger",
        ":user_context_interface",  # ✅ 接口
    ],
    # 移除: "//src/execution:execution"  # ✅ 打破循环依赖
)
```

---

## 📌 五、设计总结

### 5.1 接口统计

| 接口 | 文件 | 方法数 | 实现类 |
|------|------|--------|-------|
| `IBufferPool` | buffer_pool_interface.h | 6 | BufferPoolManager |
| `ITransactionManager` | transaction_interface.h | 5 | TransactionManager |
| `ITableStorage` | table_storage_interface.h | 8 | TableStorage |
| `IUserContext` | user_context_interface.h | 5 | UserContext |

### 5.2 依赖变化

| 模块 | 重构前 | 重构后 |
|------|--------|--------|
| Core | 依赖 Storage 实现 | 依赖接口 |
| Core | 依赖 Execution | 无依赖 |
| Execution | 依赖 Core 具体类 | 依赖接口 |

### 5.3 验收标准

| 标准 | 检验命令 |
|------|---------|
| 接口编译 | `bazel build //...:*_interface` |
| 实现编译 | `bazel build //src/...:*` |
| 无循环依赖 | `bazel query` |

---

**核心代码设计报告完成！**

**下一步**: 测试设计报告 📝
