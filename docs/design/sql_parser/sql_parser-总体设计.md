# SQL解析器组件

## 概述

SQL解析器是SQLCC数据库管理系统的重要组成部分，负责将用户输入的SQL文本解析为抽象语法树(AST)。它采用经典的编译器前端架构，包括词法分析和语法分析两个阶段。

## 核心类

1. [Parser](parser.md) - 语法分析器，负责将Token流构建为抽象语法树(AST)
2. [Lexer](lexer.md) - 词法分析器，负责将SQL文本分解为标记(Token)流
3. [Token](token.md) - 标记类，表示SQL语言中的各种标记
4. [AST Nodes](ast_nodes.md) - 抽象语法树节点类，表示SQL语句的结构化表示

## 组件关系

```
SQL文本 -> Lexer -> Token流 -> Parser -> 抽象语法树(AST)
```

## 主要功能

1. **词法分析**：识别关键字、标识符、字面量、运算符和标点符号等
2. **语法分析**：根据SQL语法规则构建抽象语法树
3. **错误处理**：提供清晰的语法错误信息
4. **扩展支持**：支持DDL、DML、DCL和事务处理语句的解析
5. **AST遍历**：提供AST节点访问器，便于后续处理

## 支持的SQL语句类型

1. **数据库操作语句**
   - CREATE DATABASE
   - ALTER DATABASE
   - DROP DATABASE
   - USE DATABASE

2. **表操作语句**
   - CREATE TABLE
   - ALTER TABLE
   - DROP TABLE
   - TRUNCATE TABLE

3. **数据查询语句**
   - SELECT (包含WHERE, JOIN, GROUP BY, HAVING, ORDER BY等子句)

4. **数据操作语句**
   - INSERT
   - UPDATE
   - DELETE

5. **事务控制语句**
   - BEGIN TRANSACTION
   - COMMIT
   - ROLLBACK
   - SAVEPOINT
   - SET TRANSACTION