# UnifiedExecutor 类文档

## 类概述

`UnifiedExecutor` 是 SQLCC 的统一查询执行器，使用**策略模式**统一处理所有类型的 SQL 语句（DDL、DML、DCL、TCL）。

## WHY: 为什么需要统一执行器？

**设计动机**：
- **简化架构**：将分散的执行逻辑统一管理
- **策略模式**：每种语句类型有独立的执行策略
- **可扩展性**：新增语句类型只需添加新策略
- **代码复用**：公共逻辑抽取到基类
- **职责分离**：各策略专注于特定语句类型

**核心价值**：
- 统一的执行入口
- 清晰的策略接口
- 灵活的执行策略切换
- 良好的扩展性

## WHAT: 核心功能

### 执行器组件

| 组件 | 功能描述 |
|------|----------|
| `UnifiedExecutor` | 主执行器，协调各策略 |
| `ExecutionStrategy` | 执行策略抽象基类 |
| `DDLExecutionStrategy` | DDL 语句执行策略 |
| `DMLExecutionStrategy` | DML 语句执行策略 |
| `DCLExecutionStrategy` | DCL 语句执行策略 |
| `UtilityExecutionStrategy` | 工具语句执行策略 |
| `AggregateEngine` | 聚合函数引擎 |
| `GroupByExecutor` | 分组查询执行器 |
| `ExecutionPlanGenerator` | 执行计划生成器 |
| `QueryOptimizer` | 查询优化器 |

### 执行策略

#### DDLExecutionStrategy
| 方法 | 功能描述 |
|------|----------|
| `executeCreate()` | 执行 CREATE 语句 |
| `executeDrop()` | 执行 DROP 语句 |
| `executeAlter()` | 执行 ALTER 语句 |
| `executeCreateIndex()` | 执行 CREATE INDEX |
| `executeDropIndex()` | 执行 DROP INDEX |

#### DMLExecutionStrategy
| 方法 | 功能描述 |
|------|----------|
| `executeInsert()` | 执行 INSERT 语句 |
| `executeUpdate()` | 执行 UPDATE 语句 |
| `executeDelete()` | 执行 DELETE 语句 |
| `executeSelect()` | 执行 SELECT 语句 |
| `executeJoinSelect()` | 执行 JOIN 查询 |
| `executeGroupBySelect()` | 执行 GROUP BY 查询 |
| `executeAggregateSelect()` | 执行聚合查询 |

#### DCLExecutionStrategy
| 方法 | 功能描述 |
|------|----------|
| `executeCreateUser()` | 创建用户 |
| `executeDropUser()` | 删除用户 |
| `executeGrant()` | 授予权限 |
| `executeRevoke()` | 撤销权限 |

#### UtilityExecutionStrategy
| 方法 | 功能描述 |
|------|----------|
| `executeUse()` | 切换数据库 |
| `executeShow()` | 显示信息 |

## HOW: 实现机制

### 策略模式架构

```
UnifiedExecutor
├── strategies_ (unordered_map)
│   ├── Statement::CREATE → DDLExecutionStrategy
│   ├── Statement::SELECT → DMLExecutionStrategy
│   ├── Statement::INSERT → DMLExecutionStrategy
│   ├── Statement::UPDATE → DMLExecutionStrategy
│   ├── Statement::DELETE → DMLExecutionStrategy
│   ├── Statement::GRANT → DCLExecutionStrategy
│   ├── Statement::REVOKE → DCLExecutionStrategy
│   └── Statement::SHOW → UtilityExecutionStrategy
├── plan_generator_ → ExecutionPlanGenerator
├── query_optimizer_ → QueryOptimizer
└── execution_context_ → ExecutionContext
```

### 执行流程

1. **接收语句**：接收解析后的 AST
2. **选择策略**：根据语句类型选择执行策略
3. **权限检查**：调用策略的权限检查方法
4. **验证语句**：调用策略的验证方法
5. **执行语句**：调用策略的执行方法
6. **返回结果**：返回执行结果

### 执行计划类型

```cpp
enum class Type {
  FULL_TABLE_SCAN,   // 全表扫描
  INDEX_SCAN,        // 索引扫描
  INDEX_SEEK,        // 索引查找
  JOIN,              // 连接操作
  AGGREGATE,         // 聚合操作
  SORT               // 排序操作
};
```

## 使用示例

```cpp
#include "execution/unified_executor.h"

// 创建执行器
auto executor = std::make_shared<UnifiedExecutor>(db_manager);

// 执行 DDL 语句
auto ddl_result = executor->execute(
    std::make_unique<CreateStatement>(/* ... */));

// 执行 DML 语句
auto dml_result = executor->execute(
    std::make_unique<InsertStatement>(/* ... */));

// 执行查询
auto select_result = executor->execute(
    std::make_unique<SelectStatement>(/* ... */));

// 获取执行统计
const auto& stats = executor->getLastExecutionContext();
std::cout << "影响行数: " << stats.records_affected_ << std::endl;
```

## 权限检查

执行器内置完整的权限检查机制：

| 语句类型 | 所需权限 |
|----------|----------|
| SELECT | SELECT |
| INSERT | INSERT |
| UPDATE | UPDATE |
| DELETE | DELETE |
| CREATE | CREATE |
| DROP | DROP |
| ALTER | ALTER |
| GRANT | GRANT OPTION |

## 性能优化

### 查询优化器功能
- **成本估算**：估算不同执行计划的成本
- **索引选择**：自动选择最优索引
- **连接顺序**：优化多表连接顺序
- **重写规则**：SQL 重写优化

### 执行计划缓存
```cpp
// 相同结构的 SQL 可以复用执行计划
auto result = executor->execute(sql, /* use_cached_plan */ true);
```

## 扩展开发

### 自定义执行策略

```cpp
class CustomExecutionStrategy : public ExecutionStrategy {
public:
  ExecutionResult execute(std::unique_ptr<Statement> stmt,
                          ExecutionContext &context) override {
    // 自定义执行逻辑
    return ExecutionResult::Success();
  }
};

// 注册自定义策略
executor->registerStrategy(Statement::CUSTOM, 
    std::make_unique<CustomExecutionStrategy>());
```

## 版本信息

- **版本**: v1.3.9
- **最后更新**: 2026-01-31
- **C++标准**: C++20
- **编译器**: Clang 18+