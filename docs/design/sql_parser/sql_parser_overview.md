# SQL Parser 概述

SQL Parser 是 SQLCC 数据库系统中的核心组件之一，负责...

## 主要职责

- 职责1
- 职责2
- 职责3

## 架构设计

组件采用分层架构设计，主要包括...

## 性能特点

- 特点1
- 特点2
- 特点3

---

## 接口与依赖

**核心接口**:
- Parser / Lexer / ASTNode

**关键依赖**:
- types / utils / exception

---

## 验证方式

- 编译验证: `bazel build //src/sql_parser:sql_parser`
- 测试验证: `bazel test //tests/sql_parser/...`
- 覆盖率验证: `bazel coverage //tests/sql_parser/...`

---

## 风险与权衡

- 风险: 语法兼容性与错误恢复复杂度
- 权衡: 以正确性优先，逐步优化性能
