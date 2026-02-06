# SQL Executor 概述

SQL Executor 是 SQLCC 数据库系统中的核心组件之一，负责...

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
- Executor / ExecutionContext / PlanNode

**关键依赖**:
- sql_parser / storage_engine / transaction

---

## 验证方式

- 编译验证: `bazel build //src/execution:execution`
- 测试验证: `bazel test //tests/level4_sql_processing/...`
- 覆盖率验证: `bazel coverage //tests/level4_sql_processing/...`

---

## 风险与权衡

- 风险: 计划执行路径复杂导致性能波动
- 权衡: 保证正确性优先，逐步优化执行链路
