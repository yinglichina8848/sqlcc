# SQLCC 核心模块依赖关系分析报告

## 概述

本报告分析了 SQLCC 系统中 sql_parser、execution、storage_engine、transaction、network、session 六个核心模块之间的依赖关系，识别循环依赖问题，并提出解决方案。

## 模块概览

### 1. sql_parser 模块
- **位置**: `src/sql_parser/` 和 `include/sql_parser/`
- **功能**: SQL 语句解析，生成抽象语法树 (AST)
- **主要类**:
  - `Parser`: SQL 语句解析器
  - `Lexer`: 词法分析器
  - `ASTNode` 及其子类: AST 节点
  - `LoadDataStatement`: LOAD DATA 语句 AST

### 2. execution 模块
- **位置**: `src/execution/` 和 `include/execution/`
- **功能**: SQL 语句执行引擎
- **主要类**:
  - `LoadDataExecutor`: LOAD DATA 语句执行器
  - `JoinExecutor`: JOIN 操作执行器
  - `SetOperationExecutor`: 集合操作执行器
  - `WindowFunctionExecutor`: 窗口函数执行器

### 3. storage_engine 模块
- **位置**: `src/storage_engine/` 和 `include/storage_engine/`
- **功能**: 数据存储和索引管理
- **主要类**:
  - `StorageEngine`: 存储引擎接口
  - `TableStorage`: 表存储管理
  - `BPlusTreeIndex`: B+树索引
  - `BufferPool`: 缓冲池管理

### 4. transaction 模块
- **位置**: `src/transaction/` 和 `include/transaction/`
- **功能**: 事务管理
- **主要类**:
  - `TransactionManager`: 事务管理器
  - `SavepointManager`: 保存点管理器

### 5. network 模块
- **位置**: `src/network/` 和 `include/network/`
- **功能**: 网络通信
- **主要类**:
  - `Network`: 网络通信接口
  - `ConnectionHandler`: 连接处理器
  - `SessionManager`: 会话管理器

### 6. session 模块
- **位置**: `src/network/session.cpp` 等
- **功能**: 会话管理
- **主要类**:
  - `Session`: 会话类
  - `SessionManager`: 会话管理器

## 依赖关系分析

### 当前依赖图

```
sql_parser -> execution (使用 ExecutionResult)
    ↓
execution -> storage_engine (数据操作)
    ↓
storage_engine -> transaction (事务控制)
    ↓
transaction -> storage_engine (数据访问)
    ↓
storage_engine -> network (分布式存储)
    ↓
network -> session (会话管理)
    ↓
session -> network (网络通信)
```

### 详细依赖分析

#### 1. sql_parser -> execution
- **原因**: sql_parser 生成的 AST 需要 execution 模块来执行
- **具体**: LoadDataStatement 等 AST 节点被 execution 中的 executor 使用
- **解决方案**: 保持此依赖，属于正常的数据流向

#### 2. execution -> storage_engine
- **原因**: execution 需要调用 storage_engine 进行数据操作
- **具体**: LoadDataExecutor 调用 StorageEngine 的 insert_record、get_table_metadata 等方法
- **解决方案**: 保持此依赖，通过接口抽象

#### 3. storage_engine -> transaction
- **原因**: 数据操作需要事务控制保证一致性
- **具体**: StorageEngine 在执行操作时需要 TransactionManager
- **解决方案**: 依赖注入模式，避免直接依赖

#### 4. transaction -> storage_engine
- **原因**: 事务管理需要访问存储层的数据
- **具体**: TransactionManager 需要调用 StorageEngine 的方法
- **形成循环**: storage_engine ↔ transaction

#### 5. storage_engine -> network
- **原因**: 分布式存储需要网络通信
- **具体**: 在分布式环境中，StorageEngine 需要 Network 进行节点间通信
- **解决方案**: 可选依赖，通过配置决定

#### 6. network -> session
- **原因**: 网络通信需要会话管理
- **具体**: Network 使用 SessionManager 管理连接会话
- **解决方案**: 正常依赖关系

#### 7. session -> network
- **原因**: 会话需要底层网络支持
- **具体**: Session 类需要调用 Network 的方法
- **形成循环**: network ↔ session

## 循环依赖识别

### 1. storage_engine ↔ transaction 循环依赖

**循环路径**:
```
storage_engine -> transaction -> storage_engine
```

**具体代码位置**:
- `src/storage_engine/storage_engine.cpp` 包含 `include/transaction_manager.h`
- `src/transaction/transaction_manager.cpp` 调用 StorageEngine 方法

**问题影响**:
- 编译时序依赖问题
- 模块化设计违反
- 测试困难

### 2. network ↔ session 循环依赖

**循环路径**:
```
network -> session -> network
```

**具体代码位置**:
- `src/network/network.cpp` 使用 SessionManager
- `src/network/session.cpp` 调用 Network 方法

**问题影响**:
- 网络模块无法独立测试
- 代码重用性差

## 解决方案设计

### 方案1: 依赖倒置原则 (DIP)

#### 对于 storage_engine ↔ transaction:

1. **定义接口**:
```cpp
class TransactionContext {
public:
    virtual ~TransactionContext() = default;
    virtual void beginTransaction() = 0;
    virtual void commitTransaction() = 0;
    virtual void rollbackTransaction() = 0;
};

class StorageAccessor {
public:
    virtual ~StorageAccessor() = default;
    virtual bool insertRecord(const std::string& table, const std::vector<std::string>& record) = 0;
    virtual std::shared_ptr<TableMetadata> getTableMetadata(const std::string& table) = 0;
};
```

2. **重构 storage_engine**:
```cpp
class StorageEngine {
private:
    std::shared_ptr<TransactionContext> transaction_context_;
public:
    void setTransactionContext(std::shared_ptr<TransactionContext> ctx) {
        transaction_context_ = ctx;
    }
};
```

3. **重构 transaction**:
```cpp
class TransactionManager : public TransactionContext {
private:
    std::shared_ptr<StorageAccessor> storage_accessor_;
public:
    void setStorageAccessor(std::shared_ptr<StorageAccessor> accessor) {
        storage_accessor_ = accessor;
    }
};
```

#### 对于 network ↔ session:

1. **定义网络接口**:
```cpp
class NetworkInterface {
public:
    virtual ~NetworkInterface() = default;
    virtual bool sendMessage(const std::string& message) = 0;
    virtual std::string receiveMessage() = 0;
};
```

2. **重构 session**:
```cpp
class Session {
private:
    std::shared_ptr<NetworkInterface> network_;
public:
    void setNetworkInterface(std::shared_ptr<NetworkInterface> network) {
        network_ = network;
    }
};
```

### 方案2: 事件驱动架构

#### 实现思路:
1. 使用事件总线(Event Bus)模式
2. 模块间通过事件通信，而非直接调用
3. 减少直接依赖

#### 优点:
- 松耦合设计
- 易于测试和维护
- 支持异步通信

#### 缺点:
- 实现复杂度较高
- 调试困难

### 方案3: 模块重组

#### 重组策略:
1. **合并相关模块**: 将 transaction 和 storage_engine 合并为 data_access 模块
2. **分离关注点**: 将 session 功能移到 network 模块内部
3. **接口分离**: 为每个模块定义清晰的边界接口

## 实施计划

### Phase 1: 接口定义 (1-2 天)
1. 定义 TransactionContext 接口
2. 定义 StorageAccessor 接口
3. 定义 NetworkInterface 接口
4. 创建接口头文件

### Phase 2: 依赖注入实现 (2-3 天)
1. 修改 StorageEngine 接受 TransactionContext
2. 修改 TransactionManager 接受 StorageAccessor
3. 修改 Session 接受 NetworkInterface
4. 更新构造函数和工厂方法

### Phase 3: 重构现有代码 (3-5 天)
1. 重构 storage_engine 中的事务调用
2. 重构 transaction 中的存储访问
3. 重构 session 中的网络调用
4. 更新所有相关的测试代码

### Phase 4: 测试和验证 (2-3 天)
1. 单元测试验证
2. 集成测试验证
3. 性能测试确保无性能下降
4. 文档更新

## 风险评估

### 技术风险
1. **接口兼容性**: 新接口可能破坏现有代码
2. **性能影响**: 间接调用可能增加开销
3. **测试覆盖**: 重构可能引入新的 bug

### 缓解措施
1. **渐进式重构**: 分阶段实施，逐步验证
2. **性能监控**: 重构前后进行性能对比
3. **全面测试**: 确保测试覆盖率不下降

## 结论

通过依赖倒置原则和接口抽象，可以有效解决循环依赖问题。推荐采用方案1，因为它：
- 保持了代码的清晰性和可维护性
- 最小化了对现有代码的改动
- 提高了模块的可测试性
- 为未来的扩展留出了空间

## 下一步行动

1. 创建接口定义文档
2. 实施 Phase 1 接口定义
3. 开始 Phase 2 依赖注入实现
4. 逐步验证重构效果
