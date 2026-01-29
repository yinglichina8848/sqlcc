# LoadDataExecutor 重构设计报告

## 概述

LoadDataExecutor 是 SQLCC 中负责执行 LOAD DATA 语句的核心组件。当前实现存在严重的依赖问题，导致编译失败。本报告分析了现有问题，并提出了重构方案。

## 问题分析

### 当前问题

1. **循环依赖问题**
   - execution 包依赖 sql_parser 包
   - LoadDataExecutor 使用 LoadDataStatement（来自 sql_parser）
   - 但同时需要访问 StorageEngine 和 TableMetadata

2. **API 不匹配问题**
   - ExecutionResult 结构体缺少 `error_message` 和 `rows_affected` 字段
   - TableMetadata 类缺少 `get_columns()` 方法
   - StorageEngine 类缺少 `insert_record()`、`get_table_metadata()`、`table_exists()` 方法

3. **类型引用问题**
   - ColumnDefinition 类型引用不正确
   - 缺少必要的头文件包含

## 重构方案

### 架构设计原则

1. **依赖倒置原则**
   - 通过接口抽象减少直接依赖
   - 使用回调机制避免循环依赖

2. **单一职责原则**
   - LoadDataExecutor 只负责 LOAD DATA 的执行逻辑
   - 文件处理、数据验证、存储操作分离到独立组件

3. **接口隔离原则**
   - 定义专门的数据访问接口
   - 避免暴露不必要的 StorageEngine API

### 组件重构

#### 1. 数据访问接口

```cpp
class LoadDataDataAccess {
public:
    virtual ~LoadDataDataAccess() = default;

    // 表操作
    virtual bool tableExists(const std::string& table_name) = 0;
    virtual std::shared_ptr<TableMetadata> getTableMetadata(const std::string& table_name) = 0;

    // 数据操作
    virtual bool insertRecord(const std::string& table_name, const std::vector<std::string>& record) = 0;

    // 事务控制（可选）
    virtual bool beginTransaction() = 0;
    virtual bool commitTransaction() = 0;
    virtual bool rollbackTransaction() = 0;
};
```

#### 2. 文件处理组件

```cpp
class LoadDataFileHandler {
public:
    virtual ~LoadDataFileHandler() = default;

    virtual bool openFile(const std::string& filename, bool is_local) = 0;
    virtual bool readLine(std::string& line) = 0;
    virtual void closeFile() = 0;
    virtual bool isOpen() const = 0;
};
```

#### 3. 数据解析组件

```cpp
class LoadDataParser {
public:
    virtual ~LoadDataParser() = default;

    virtual std::vector<std::string> parseLine(const std::string& line, const LoadDataStatement& stmt) = 0;
    virtual bool validateData(const std::vector<std::string>& fields, const LoadDataStatement& stmt) = 0;
};
```

### 执行流程重构

#### 新执行流程

1. **预处理阶段**
   - 验证表存在性
   - 获取表元数据
   - 检查文件权限
   - 初始化统计信息

2. **解析阶段**
   - 逐行读取文件
   - 解析字段数据
   - 应用数据转换

3. **验证阶段**
   - 数据类型验证
   - 约束检查
   - 自定义验证规则

4. **插入阶段**
   - 批量插入数据
   - 错误处理和回滚
   - 进度报告

### 接口适配方案

#### ExecutionResult 扩展

```cpp
struct ExecutionResult {
    // 现有字段
    bool success;
    std::string message;

    // 新增字段
    int64_t rows_affected = 0;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};
```

#### TableMetadata 接口扩展

```cpp
class TableMetadata {
public:
    // 新增方法
    const std::vector<ColumnDefinition>& get_columns() const;
    bool validateRecord(const std::vector<std::string>& record) const;
    bool checkConstraints(const std::vector<std::string>& record) const;
};
```

### 依赖注入设计

#### 构造函数重构

```cpp
class LoadDataExecutor {
public:
    LoadDataExecutor(
        std::shared_ptr<LoadDataDataAccess> data_access,
        std::shared_ptr<LoadDataFileHandler> file_handler,
        std::shared_ptr<LoadDataParser> parser
    );

    ExecutionResult execute(const LoadDataStatement& stmt);
};
```

### 错误处理重构

#### 统一错误处理

```cpp
class LoadDataErrorHandler {
public:
    enum ErrorType {
        FILE_ACCESS_ERROR,
        PARSING_ERROR,
        VALIDATION_ERROR,
        CONSTRAINT_VIOLATION,
        STORAGE_ERROR
    };

    void addError(ErrorType type, const std::string& message);
    void addWarning(const std::string& message);
    const std::vector<std::string>& getErrors() const;
    const std::vector<std::string>& getWarnings() const;
};
```

## 实施计划

### Phase 1: 接口定义
- 定义 LoadDataDataAccess 接口
- 定义 LoadDataFileHandler 接口
- 定义 LoadDataParser 接口
- 扩展 ExecutionResult 结构体

### Phase 2: 组件实现
- 实现 StorageEngine 适配器
- 实现文件处理组件
- 实现数据解析组件
- 实现错误处理组件

### Phase 3: 执行器重构
- 重构 LoadDataExecutor 类
- 实现依赖注入
- 更新构造函数和方法签名

### Phase 4: 集成测试
- 编写单元测试
- 集成测试 LOAD DATA 功能
- 性能测试和优化

## 优势

1. **消除循环依赖**
   - 通过接口抽象解耦组件
   - 依赖注入模式减少直接依赖

2. **提高可测试性**
   - 组件独立测试
   - Mock 对象支持

3. **增强可维护性**
   - 单一职责原则
   - 清晰的接口契约

4. **性能优化**
   - 批量处理支持
   - 异步处理准备

## 风险评估

1. **向后兼容性**
   - ExecutionResult 结构体扩展可能影响现有代码
   - 建议使用渐进式迁移

2. **性能影响**
   - 接口调用开销
   - 通过内联优化和缓存缓解

3. **复杂性增加**
   - 组件数量增加
   - 通过良好的文档和示例代码缓解

## 结论

通过这次重构，LoadDataExecutor 将具备更好的架构设计，消除循环依赖问题，提高代码的可维护性和可测试性。虽然增加了实现的复杂性，但从长期来看，这将为系统的稳定性和扩展性奠定坚实的基础。

## 下一步行动

1. 实施 Phase 1 接口定义
2. 实现基础组件
3. 重构 LoadDataExecutor
4. 编写测试用例
5. 性能优化和调优
