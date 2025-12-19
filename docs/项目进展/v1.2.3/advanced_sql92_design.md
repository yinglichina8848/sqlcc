# SQLCC v1.3.0 高级SQL-92特性设计文档

## 概述

本设计文档描述SQLCC v1.3.0版本中高级SQL-92特性的实现方案，包括存储过程和函数、触发器完整实现、事务控制增强、用户定义类型等核心功能。

## 架构分析

### 现有架构基础

#### 1. 存储过程架构
```
ProcedureParser → ProcedureVM → ProcedureTriggerExecutor
     ↓              ↓              ↓
   解析过程语言   执行过程逻辑   统一执行接口
```

**当前状态**：
- ✅ ProcedureParser: 支持变量声明、赋值、IF-ELSE、WHILE、SQL调用
- ✅ ProcedureVM: 支持表达式求值、变量管理、控制流执行
- ✅ ProcedureTriggerExecutor: 统一存储过程和触发器执行

#### 2. 触发器架构
```
TriggerManager → TriggerExecutor → RecursionGuard
     ↓              ↓              ↓
  触发器管理     触发器执行      递归防护
```

**当前状态**：
- ✅ TriggerManager: 支持触发器注册、注销、状态管理
- ✅ 递归防护: RecursionGuard实现防止无限递归
- ❌ 触发器执行器: 需要实现具体的SQL执行逻辑

#### 3. 事务管理架构
```
TransactionManager → LockManager → WAL
     ↓              ↓              ↓
  事务协调        锁管理       预写日志
```

**当前状态**：
- ✅ TransactionManager: 支持ACID事务、隔离级别
- ✅ 锁管理: 支持共享锁、排他锁、死锁检测
- ✅ SAVEPOINT接口: 已定义create_savepoint和rollback_to_savepoint

## 高级SQL-92特性设计

### 1. 存储过程和函数扩展 (CREATE PROCEDURE/FUNCTION)

#### 当前架构扩展
```
ProcedureParser → ProcedureVM → FunctionExecutor
     ↓              ↓              ↓
  扩展语法支持   函数执行引擎    函数调用接口
```

#### 设计方案

##### 1.1 函数定义语法支持
```sql
-- 函数定义
CREATE FUNCTION function_name(param1 type1, param2 type2, ...)
RETURNS return_type
[DETERMINISTIC | NOT DETERMINISTIC]
[CONTAINS SQL | READS SQL DATA | MODIFIES SQL DATA]
BEGIN
    -- 函数体
    RETURN expression;
END;

-- 函数调用
SELECT function_name(arg1, arg2, ...);
SELECT * FROM table WHERE column = function_name(value);
```

##### 1.2 函数类型分类
- **标量函数**: 返回单个值
- **聚合函数**: 在GROUP BY中使用
- **表值函数**: 返回表结果集

##### 1.3 函数执行引擎 (FunctionExecutor)
```cpp
class FunctionExecutor {
public:
    // 执行标量函数
    Value executeScalarFunction(const std::string& function_name,
                               const std::vector<Value>& arguments);

    // 执行表值函数
    std::vector<Row> executeTableFunction(const std::string& function_name,
                                        const std::vector<Value>& arguments);

    // 注册用户定义函数
    bool registerUserFunction(std::unique_ptr<FunctionDefinition> function);

    // 注销用户定义函数
    bool unregisterUserFunction(const std::string& function_name);
};
```

### 2. 触发器完整实现 (BEFORE/AFTER触发器)

#### 当前架构完善
```
TriggerManager → SQLTriggerExecutor → EventProcessor
     ↓              ↓              ↓
  触发器管理     SQL触发执行      事件处理
```

#### 设计方案

##### 2.1 触发器执行时序
```
DML操作 → BEFORE触发器 → 实际操作 → AFTER触发器
```

##### 2.2 SQL触发器执行器 (SQLTriggerExecutor)
```cpp
class SQLTriggerExecutor : public TriggerExecutor {
public:
    bool executeTrigger(const TriggerDefinition* trigger,
                       const RowData* old_row,
                       const RowData* new_row) override;

    bool evaluateCondition(const std::string& condition,
                          const RowData* old_row,
                          const RowData* new_row) override;

private:
    // 解析触发器SQL
    std::unique_ptr<Statement> parseTriggerSQL(const std::string& sql);

    // 创建触发器执行上下文
    std::unique_ptr<ExecutionContext> createTriggerContext(
        const TriggerDefinition* trigger,
        const RowData* old_row,
        const RowData* new_row);
};
```

##### 2.3 触发器变量引用
- `:OLD.column_name` - 引用修改前的列值
- `:NEW.column_name` - 引用修改后的列值
- `OLD` 和 `NEW` 在ROW级别触发器中有效

### 3. 事务控制增强 (SAVEPOINT, 嵌套事务)

#### 当前架构扩展
```
TransactionManager → SavepointManager → NestedTransactionCoordinator
     ↓              ↓              ↓
  事务管理       保存点管理        嵌套事务协调
```

#### 设计方案

##### 3.1 保存点管理器 (SavepointManager)
```cpp
class SavepointManager {
public:
    struct Savepoint {
        std::string name;
        TransactionId txn_id;
        size_t undo_log_position;
        std::unordered_set<std::string> locked_resources;
        std::chrono::system_clock::time_point timestamp;
    };

    // 创建保存点
    bool createSavepoint(TransactionId txn_id, const std::string& name);

    // 回滚到保存点
    bool rollbackToSavepoint(TransactionId txn_id, const std::string& name);

    // 释放保存点
    bool releaseSavepoint(TransactionId txn_id, const std::string& name);

    // 获取保存点信息
    std::shared_ptr<Savepoint> getSavepoint(TransactionId txn_id,
                                           const std::string& name) const;

private:
    std::unordered_map<TransactionId, std::vector<std::unique_ptr<Savepoint>>> savepoints_;
};
```

##### 3.2 嵌套事务协调器 (NestedTransactionCoordinator)
```cpp
class NestedTransactionCoordinator {
public:
    // 开始嵌套事务
    TransactionId beginNestedTransaction(TransactionId parent_txn_id,
                                       IsolationLevel isolation_level);

    // 提交嵌套事务
    bool commitNestedTransaction(TransactionId nested_txn_id);

    // 回滚嵌套事务
    bool rollbackNestedTransaction(TransactionId nested_txn_id);

    // 检查事务嵌套层级
    size_t getNestingLevel(TransactionId txn_id) const;

private:
    struct NestedTransactionInfo {
        TransactionId parent_txn_id;
        size_t nesting_level;
        std::vector<LogEntry> local_undo_log;
    };

    std::unordered_map<TransactionId, NestedTransactionInfo> nested_transactions_;
};
```

### 4. 用户定义类型 (DOMAIN, 用户类型)

#### 新增架构组件
```
TypeSystem → DomainManager → UserTypeValidator
     ↓              ↓              ↓
  类型系统      域管理        用户类型验证
```

#### 设计方案

##### 4.1 域定义 (DOMAIN)
```sql
-- 域定义
CREATE DOMAIN email_domain AS VARCHAR(255)
CHECK (VALUE LIKE '%@%');

CREATE DOMAIN positive_int AS INTEGER
CHECK (VALUE > 0);

-- 使用域
CREATE TABLE users (
    id INTEGER PRIMARY KEY,
    email email_domain,
    age positive_int
);
```

##### 4.2 域管理器 (DomainManager)
```cpp
class DomainManager {
public:
    struct DomainDefinition {
        std::string name;
        std::string base_type;
        std::string check_constraint;
        std::string default_value;
        bool nullable;
    };

    // 创建域
    bool createDomain(std::unique_ptr<DomainDefinition> domain);

    // 删除域
    bool dropDomain(const std::string& domain_name);

    // 验证域约束
    bool validateDomainValue(const std::string& domain_name,
                           const Value& value) const;

    // 获取域定义
    std::shared_ptr<const DomainDefinition> getDomain(const std::string& name) const;

private:
    std::unordered_map<std::string, std::unique_ptr<DomainDefinition>> domains_;
};
```

##### 4.3 用户定义类型验证器 (UserTypeValidator)
```cpp
class UserTypeValidator {
public:
    // 验证值是否符合域定义
    bool validateValue(const std::string& type_name, const Value& value) const;

    // 检查类型兼容性
    bool isTypeCompatible(const std::string& source_type,
                         const std::string& target_type) const;

    // 转换类型值
    Value convertValue(const Value& value,
                      const std::string& source_type,
                      const std::string& target_type) const;

private:
    std::shared_ptr<DomainManager> domain_manager_;
};
```

## 实现计划

### Phase 1: 存储过程和函数 (2周)
1. 扩展ProcedureParser支持函数语法
2. 实现FunctionExecutor执行引擎
3. 添加函数注册和调用接口
4. 创建函数相关的测试用例

### Phase 2: 触发器完整实现 (2周)
1. 实现SQLTriggerExecutor
2. 添加触发器变量(:OLD/:NEW)支持
3. 完善触发器执行时序
4. 增强递归防护机制

### Phase 3: 事务控制增强 (2周)
1. 完善SavepointManager实现
2. 实现NestedTransactionCoordinator
3. 添加事务嵌套层级管理
4. 创建事务控制测试

### Phase 4: 用户定义类型 (2周)
1. 实现DomainManager
2. 添加DOMAIN语法解析
3. 实现UserTypeValidator
4. 扩展类型系统集成

### Phase 5: 集成和测试 (2周)
1. 统一所有组件到主执行器
2. 创建高级SQL-92特性综合测试
3. 性能优化和错误处理
4. 文档和示例更新

## 技术指标

### 功能完整性
- **存储过程**: 100% 支持SQL/PSM语法
- **函数**: 支持标量函数和表值函数
- **触发器**: 100% 支持BEFORE/AFTER触发器
- **事务控制**: 支持SAVEPOINT和嵌套事务
- **用户类型**: 支持DOMAIN定义和验证

### 性能指标
- **函数调用**: <1ms 平均执行时间
- **触发器执行**: <2ms 触发延迟
- **事务保存点**: <0.5ms 创建时间
- **类型验证**: <0.1ms 验证时间

### 可靠性指标
- **测试覆盖率**: 95%+ 高级特性代码
- **错误处理**: 100% 边界条件覆盖
- **并发安全**: 支持多线程并发访问
- **内存安全**: 零内存泄漏验证

## 架构优势

1. **模块化设计**: 每个高级特性独立模块，便于维护和扩展
2. **统一接口**: 通过统一的执行器接口集成所有功能
3. **可扩展性**: 易于添加新的高级SQL特性
4. **性能优化**: 专门的执行引擎提供高性能处理
5. **安全保证**: 完善的错误处理和边界检查

---

**设计完成时间**: 2025年12月17日
**预计实现周期**: 10周 (2026年3月)
**技术评审状态**: ✅ 通过
**架构设计**: ✅ 完成
