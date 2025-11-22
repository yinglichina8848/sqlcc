# SQL执行引擎组件

## 概述

SQL执行引擎（SQLExecutor）是SQLCC数据库管理系统的核心组件之一，负责将SQL解析器生成的抽象语法树（AST）转换为实际的数据库操作。它是连接SQL解析和底层存储引擎的关键桥梁，确保SQL语句能够正确、高效地执行。

## 核心类

1. [SqlExecutor](sql_executor.md) - SQL执行引擎主类，负责协调各种SQL语句的执行
2. [TableMetadata](table_metadata.md) - 表元数据类，存储表的结构信息
3. [Record](record.md) - 数据记录类，表示表中的一行数据
4. [ConstraintExecutor](constraint_executor.md) - 约束执行器，处理表约束验证
5. [WhereCondition](where_condition.md) - WHERE条件类，表示查询过滤条件
6. [SqlExecutionException](sql_execution_exception.md) - SQL执行异常类，处理执行过程中的异常

## 组件关系

```
SqlExecutor
    |
    |-- TableMetadata (表元数据管理)
    |-- Record (记录操作)
    |-- ConstraintExecutor (约束验证)
    |-- WhereCondition (查询条件)
    |-- StorageEngine (存储引擎接口)
    `-- TransactionManager (事务管理)
```

## 主要功能

1. **SQL语句执行**：执行各种类型的SQL语句（DDL、DML、DCL、事务控制）
2. **元数据管理**：管理表结构和约束信息
3. **记录操作**：执行记录的增删改查操作
4. **约束验证**：在数据操作前后验证表约束
5. **事务支持**：与事务管理器集成，提供ACID事务保证
6. **错误处理**：提供清晰的错误信息和异常处理机制

## 支持的SQL语句类型

### DDL（数据定义语言）
- CREATE DATABASE, CREATE TABLE
- ALTER DATABASE, ALTER TABLE
- DROP DATABASE, DROP TABLE
- USE DATABASE

### DML（数据操作语言）
- INSERT
- SELECT
- UPDATE
- DELETE

### DCL（数据控制语言）
- GRANT
- REVOKE

### 事务控制语句
- BEGIN TRANSACTION
- COMMIT
- ROLLBACK
- SAVEPOINT

### 索引操作语句
- CREATE INDEX
- DROP INDEX

## 执行流程

1. **SQL解析**：接收SQL解析器生成的AST
2. **语句分发**：根据语句类型分发到相应的执行方法
3. **元数据检查**：验证表和列是否存在
4. **约束验证**：执行约束检查
5. **事务处理**：与事务管理器交互
6. **存储操作**：调用存储引擎执行实际数据操作
7. **结果返回**：格式化并返回执行结果

## 设计特点

1. **模块化设计**：将不同类型的SQL语句执行分离到独立的方法中，便于维护和扩展
2. **异常安全**：提供完善的异常处理机制，确保系统稳定运行
3. **线程安全**：通过互斥锁保护并发访问
4. **易于扩展**：支持添加新的SQL语句类型和功能