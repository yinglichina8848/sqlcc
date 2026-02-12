# Core 模块接口解耦设计规范

**版本**: v1.0.0  
**日期**: 2026-02-11  
**作者**: SQLCC Team (OpenCode Developer [liying managed])  
**状态**: 设计中

---

## 1. 概述

### 1.1 目标

本文档定义了 SQLCC Core 模块的接口解耦设计方案，旨在：

1. **降低编译依赖**: 通过前置声明和接口抽象减少头文件包含
2. **提高可测试性**: 引入接口层便于 Mock 测试
3. **支持依赖注入**: 使用接口而非具体实现，支持灵活的组件替换
4. **遵循 SOLID 原则**: 单一职责、开闭原则、依赖倒置

### 1.2 范围

- Core 模块核心组件接口化
- 接口层目录结构定义
- 实现类适配接口
- 单元测试策略

### 1.3 术语

| 术语 | 说明 |
|------|------|
| IDatabaseManager | 数据库管理器接口 |
| IExecutionContext | 执行上下文接口 |
| IUserManager | 用户管理器接口 |
| PIMPL | Pointer to Implementation，实现隐藏模式 |
| DI | Dependency Injection，依赖注入 |

---

## 2. 现状分析

### 2.1 当前问题

#### 问题 1: 头文件循环依赖
```
execution_context.h → core_database_manager.h
                    ↓
       buffer_pool_sharded.h
```

**影响**: 
- 编译时间增加
- 修改一个头文件导致大量文件重编译
- 头文件修改风险高

#### 问题 2: 直接包含具体实现
```cpp
// core_database_manager.h
#include "../../src/storage_engine/buffer_pool/buffer_pool_sharded.h"
```

**影响**:
- Core 模块与 Storage 模块紧耦合
- 无法独立编译 Core 模块
- 单元测试困难

#### 问题 3: ExecutionContext 成员变量过多
```cpp
class ExecutionContext {
public:
    std::string current_user;      // 30+ 个公共成员变量
    std::string current_database;
    // ... 更多
};
```

**影响**:
- 违反封装原则
- 难以维护
- 线程安全问题

### 2.2 依赖关系图

```
当前依赖关系:
┌─────────────────────┐
│ ExecutionContext    │──────┐
└──────────┬──────────┘      │
           │ 包含              │ 包含
           ▼                  ▼
┌─────────────────────┐    ┌─────────────────────┐
│ DatabaseManager     │────│ BufferPool          │
└──────────┬──────────┘    └─────────────────────┘
           │
           ▼
┌─────────────────────┐
│ StorageEngine       │
└─────────────────────┘
```

---

## 3. 设计方案

### 3.1 整体架构

```
目标架构:
┌─────────────────────────────────────────┐
│           Interface Layer               │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ │
│  │IDatabase │ │IExecution│ │  IUser   │ │
│  │ Manager  │ │ Context  │ │ Manager  │ │
│  └────┬─────┘ └────┬─────┘ └────┬─────┘ │
└───────┼───────────┼───────────┼────────┘
        │           │           │
        ▼           ▼           ▼
┌─────────────────────────────────────────┐
│         Implementation Layer            │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ │
│  │Database  │ │Execution │ │  User    │ │
│  │Manager   │ │ Context  │ │ Manager  │ │
│  └──────────┘ └──────────┘ └──────────┘ │
└─────────────────────────────────────────┘
```

### 3.2 接口层目录结构

```
src/core/
├── interfaces/                 # 接口层（新增）
│   ├── core_interfaces.h       # 接口汇总头文件
│   ├── i_database_manager.h    # 数据库管理器接口
│   ├── i_execution_context.h   # 执行上下文接口
│   ├── i_user_manager.h        # 用户管理器接口
│   ├── i_storage_engine.h      # 存储引擎接口（预留）
│   ├── i_index_manager.h       # 索引管理器接口（预留）
│   └── i_transaction_manager.h # 事务管理器接口（预留）
├── core_database_manager.h     # 实现头文件
├── core_database_manager.cpp   # 实现文件
├── execution_context.h         # 实现头文件
├── execution_context.cpp       # 实现文件
└── ...
```

### 3.3 接口设计原则

#### 原则 1: 纯虚接口
```cpp
class IDatabaseManager {
public:
    virtual ~IDatabaseManager() = default;
    virtual bool CreateDatabase(const std::string& db_name) = 0;
    // ... 纯虚函数
};
```

#### 原则 2: 智能指针管理
```cpp
// 返回 shared_ptr 而非原始指针
virtual std::shared_ptr<IStorageEngine> GetStorageEngine() = 0;
```

#### 原则 3: 常量正确性
```cpp
// 查询方法标记为 const
virtual std::string GetCurrentDatabase() const = 0;

// 修改方法不标记为 const
virtual void SetCurrentDatabase(const std::string& db) = 0;
```

#### 原则 4: 返回值规范
```cpp
// bool 表示成功/失败
virtual bool CreateDatabase(const std::string& db_name) = 0;

// 空字符串/0/nullptr 表示失败
virtual std::string ExecuteQuery(const std::string& sql) = 0;
```

---

## 4. 接口详细设计

### 4.1 IDatabaseManager 接口

```cpp
class IDatabaseManager {
public:
    // 生命周期
    virtual bool Initialize() = 0;
    virtual bool Close() = 0;
    virtual bool IsInitialized() const = 0;
    
    // 数据库管理
    virtual bool CreateDatabase(const std::string& db_name) = 0;
    virtual bool DropDatabase(const std::string& db_name) = 0;
    virtual bool UseDatabase(const std::string& db_name) = 0;
    virtual std::string GetCurrentDatabase() const = 0;
    
    // 表管理
    virtual bool CreateTable(const std::string& table_name,
                           const std::vector<std::pair<std::string, std::string>>& columns) = 0;
    virtual bool DropTable(const std::string& table_name) = 0;
    
    // 事务
    virtual TransactionId BeginTransaction(IsolationLevel level) = 0;
    virtual bool CommitTransaction(TransactionId txn_id) = 0;
    virtual bool RollbackTransaction(TransactionId txn_id) = 0;
    
    // SQL 执行
    virtual bool Execute(const std::string& sql) = 0;
    virtual std::string ExecuteQuery(const std::string& sql) = 0;
    
    // 组件访问
    virtual std::shared_ptr<IStorageEngine> GetStorageEngine() = 0;
    virtual std::shared_ptr<IIndexManager> GetIndexManager() = 0;
};
```

### 4.2 IExecutionContext 接口

```cpp
class IExecutionContext {
public:
    // 用户和数据库上下文
    virtual std::string GetCurrentUser() const = 0;
    virtual void SetCurrentUser(const std::string& user) = 0;
    virtual std::string GetCurrentDatabase() const = 0;
    virtual void SetCurrentDatabase(const std::string& db) = 0;
    
    // 事务状态
    virtual bool IsTransactional() const = 0;
    virtual void SetTransactional(bool is_txn) = 0;
    virtual std::string GetTransactionId() const = 0;
    virtual void SetTransactionId(const std::string& txn_id) = 0;
    
    // 执行统计
    virtual size_t GetRowsAffected() const = 0;
    virtual void SetRowsAffected(size_t rows) = 0;
    virtual void IncrementRowsAffected(size_t rows = 1) = 0;
    
    // 错误处理
    virtual bool HasError() const = 0;
    virtual void SetError(bool has_error, const std::string& msg = "") = 0;
    virtual std::string GetErrorMessage() const = 0;
    virtual void ClearError() = 0;
    
    // 上下文操作
    virtual void Reset() = 0;
    virtual std::shared_ptr<IExecutionContext> Clone() const = 0;
};
```

### 4.3 IUserManager 接口

```cpp
class IUserManager {
public:
    // 用户管理
    virtual bool CreateUser(const std::string& username,
                          const std::string& password,
                          const std::string& role = "USER") = 0;
    virtual bool DropUser(const std::string& username) = 0;
    virtual bool AuthenticateUser(const std::string& username,
                                 const std::string& password) = 0;
    
    // 角色管理
    virtual bool CreateRole(const std::string& role_name) = 0;
    virtual bool GrantRoleToRole(const std::string& parent, 
                                const std::string& child) = 0;
    
    // 权限管理
    virtual bool GrantPrivilege(const std::string& grantee,
                               const std::string& database,
                               const std::string& table,
                               const std::string& privilege) = 0;
    virtual bool CheckPermission(const std::string& username,
                                const std::string& database,
                                const std::string& table,
                                const std::string& privilege) const = 0;
};
```

---

## 5. 实现适配

### 5.1 DatabaseManager 适配

```cpp
// core_database_manager.h
#include "interfaces/i_database_manager.h"

class DatabaseManager : public sqlcc::core::interfaces::IDatabaseManager {
public:
    // 实现接口方法
    bool Initialize() override;
    bool CreateDatabase(const std::string& db_name) override;
    // ...
};
```

### 5.2 ExecutionContext 适配

```cpp
// execution_context.h
#include "interfaces/i_execution_context.h"

class ExecutionContext : public sqlcc::core::interfaces::IExecutionContext {
public:
    // 实现接口方法
    std::string GetCurrentUser() const override;
    void SetCurrentUser(const std::string& user) override;
    // ...
    
private:
    // PIMPL 模式隐藏实现细节
    class Impl;
    std::unique_ptr<Impl> impl_;
};
```

---

## 6. 测试策略

### 6.1 Mock 实现

```cpp
// tests/level2_core/mocks/mock_database_manager.h
#include "src/core/interfaces/i_database_manager.h"

class MockDatabaseManager : public sqlcc::core::interfaces::IDatabaseManager {
public:
    MOCK_METHOD(bool, CreateDatabase, (const std::string&), (override));
    MOCK_METHOD(bool, DropDatabase, (const std::string&), (override));
    MOCK_METHOD(bool, Execute, (const std::string&), (override));
    // ...
};
```

### 6.2 接口测试

```cpp
// tests/level2_core/interface_decoupling_test.cpp
TEST(CoreInterfaceTest, DatabaseManagerImplementsInterface) {
    auto db_mgr = std::make_unique<DatabaseManager>("/tmp/test_db");
    IDatabaseManager* interface = db_mgr.get();
    EXPECT_NE(interface, nullptr);
    EXPECT_TRUE(interface->Initialize());
}

TEST(CoreInterfaceTest, ExecutionContextImplementsInterface) {
    auto mock_db = std::make_shared<MockDatabaseManager>();
    ExecutionContext ctx(mock_db);
    IExecutionContext* interface = &ctx;
    EXPECT_NE(interface, nullptr);
}
```

### 6.3 覆盖率要求

| 接口 | 目标覆盖率 |
|------|-----------|
| IDatabaseManager | ≥ 90% |
| IExecutionContext | ≥ 90% |
| IUserManager | ≥ 90% |

---

## 7. 迁移计划

### Phase 1: 前置声明修复（已完成）

- [x] 移除 `core_database_manager.h` 中的直接包含
- [x] 添加前向声明
- [x] 修复 `execution_context.cpp` 包含

### Phase 2: 接口层创建（已完成）

- [x] 创建 `src/core/interfaces/` 目录
- [x] 创建 `i_database_manager.h`
- [x] 创建 `i_execution_context.h`
- [x] 创建 `i_user_manager.h`
- [x] 创建 `core_interfaces.h` 汇总头文件

### Phase 3: 实现类适配（待完成）

- [ ] 修改 `DatabaseManager` 实现接口
- [ ] 修改 `ExecutionContext` 实现接口（PIMPL 模式）
- [ ] 修改 `UserManager` 实现接口

### Phase 4: 依赖注入重构（待完成）

- [ ] 创建依赖注入容器
- [ ] 更新所有使用点
- [ ] 单元测试覆盖

---

## 8. 验收标准

### 8.1 编译基线

```bash
# 必须全部通过
bazel build //src/core:core
bazel build //src/core/interfaces:interfaces
bazel test //tests/level2_core:all
```

### 8.2 代码规范

- [ ] 所有接口文件包含完整文件头注释
- [ ] 所有接口方法包含 Doxygen 注释
- [ ] 遵循命名规范（PascalCase 接口名）
- [ ] 使用智能指针管理资源

### 8.3 功能验证

- [ ] DatabaseManager 正确实现 IDatabaseManager
- [ ] ExecutionContext 正确实现 IExecutionContext
- [ ] 可以成功创建 Mock 实现
- [ ] 单元测试全部通过

---

## 9. 风险与缓解

| 风险 | 影响 | 缓解措施 |
|------|------|---------|
| 接口设计不合理 | 高 | 预留版本号，支持未来扩展 |
| 性能下降 | 中 | 虚函数开销可接受，必要时使用 CRTP |
| 编译失败 | 高 | 分阶段实施，每个阶段验证编译 |
| 测试失败 | 中 | 保持向后兼容，逐步替换 |

---

## 10. 参考资料

- [AGENTS.md](../AGENTS.md) - SQLCC 编码指南
- [SPEC_DRIVEN_DEVELOPMENT.md](../docs/sdd/SPEC_DRIVEN_DEVELOPMENT.md) - SDD 规范
- [SOLID Principles](https://en.wikipedia.org/wiki/SOLID) - SOLID 设计原则
- [PIMPL Idiom](https://cpppatterns.com/patterns/pimpl.html) - PIMPL 模式

---

**变更历史**:

| 版本 | 日期 | 变更 | 作者 |
|------|------|------|------|
| 1.0.0 | 2026-02-11 | 初始版本 | OpenCode Developer [liying managed] |
