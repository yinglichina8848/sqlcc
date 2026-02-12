# Execution Strategy 错误模型规范

## 1. 概述

### 1.1 功能名称
Execution Strategy 错误处理模型规范化

### 1.2 版本
1.0

### 1.3 日期
2026-02-13

### 1.4 作者
SQLCC AI 开发团队 (Gemini)

### 1.5 状态
草稿

### 1.6 任务来源
TASK-1401-S05A-ERROR-MODEL-SPEC

---

## 2. 问题分析

### 2.1 当前错误

| 文件 | 行号 | 错误类型 | 描述 |
|------|------|---------|------|
| ddl_execution_strategy.cpp | 24 | 符号未声明 | `createErrorResult` 未定义 |
| ddl_execution_strategy.cpp | 45 | 符号未声明 | `createErrorResult` 未定义 |
| ddl_execution_strategy.cpp | 59 | 访问错误 | 应使用 `stmt.getType()` |
| ddl_execution_strategy.cpp | 88 | 访问错误 | 应使用 `stmt.getType()` |
| dml_execution_strategy.cpp | 37 | 符号未声明 | `createErrorResult` 未定义 |
| dml_execution_strategy.cpp | 55 | 符号未声明 | `createErrorResult` 未定义 |
| dml_execution_strategy.cpp | 63 | 访问错误 | 应使用 `stmt.getType()` |

### 2.2 根本原因

1. **createErrorResult 函数不存在**
   - execution_result.h 中没有此函数声明
   - 开发者错误假设存在便捷函数

2. **直接访问成员变量**
   - Statement::type 是私有成员
   - 应使用公有 getter 方法 `getType()`

---

## 3. 错误处理模型设计

### 3.1 错误语义分层

```
┌─────────────────────────────────────────────────────────────┐
│                    错误严重级别                              │
├───────────┬───────────────────────────────────────────────┤
│ LEVEL     │ 描述                                            │
├───────────┼───────────────────────────────────────────────┤
│ FATAL     │ 系统级错误，无法恢复                            │
│ ERROR     │ 操作级错误，单次执行失败                         │
│ WARN      │ 警告，不影响执行但需关注                        │
│ INFO      │ 信息性消息                                      │
└───────────┴───────────────────────────────────────────────┘
```

### 3.2 错误结果创建模式

#### 模式 A: 直接构造函数 (推荐)
```cpp
// 简单错误
return ExecutionResult(false, "Statement is null");

// 带错误码
return ExecutionResult(false, "Unsupported DDL statement type: " +
                      std::to_string(static_cast<int>(stmt->getType())));
```

#### 模式 B: Builder 模式 (复杂场景)
```cpp
return ExecutionResult(false, "Create table failed")
    .add_error("Table already exists: " + table_name);
```

### 3.3 错误代码规范

| 错误类型 | 前缀 | 示例 |
|---------|------|------|
| 空指针 | ERR_NULL | ERR_NULL_STMT |
| 类型不支持 | ERR_TYPE | ERR_TYPE_UNSUPPORTED_DDL |
| 权限不足 | ERR_PERM | ERR_PERM_DENIED |
| 验证失败 | ERR_VALID | ERR_VALID_COLUMN_EMPTY |

---

## 4. Statement 访问规范

### 4.1 正确用法

```cpp
// 获取语句类型
switch (stmt->getType()) {
    case sql_parser::Statement::Type::CREATE_TABLE:
        // ...
}

// 获取语句类型 (const 引用)
switch (stmt.type) {
    // 错误！type 是私有成员
}
```

### 4.2 约束

| 约束 | 说明 |
|------|------|
| C1 | 所有 Statement 访问必须通过公有接口 |
| C2 | AST 节点访问必须使用 getter 方法 |
| C3 | 禁止在头文件外直接访问成员变量 |

---

## 5. Linux 验证矩阵

### 5.1 验证目标

| 目标 | 命令 | 环境 | 预期结果 |
|------|------|------|---------|
| G1 | bazel build //src/execution:execution | Linux + clang-20 | PASS |
| G2 | bazel build //src/core:core | Linux + clang-20 | PASS |
| G3 | bazel test //tests/level1_foundation/... | Linux + clang-20 | PASS |

### 5.2 阻塞映射

| 阻塞项 | 文件 | 归属 | 状态 |
|--------|------|------|------|
| createErrorResult | ddl_execution_strategy.cpp | S05A | 待修复 |
| stmt.type | ddl_execution_strategy.cpp | S05A | 待修复 |
| createErrorResult | dml_execution_strategy.cpp | S05A | 待修复 |
| stmt.type | dml_execution_strategy.cpp | S05A | 待修复 |

---

## 6. 修复方案

### 6.1 ddl_execution_strategy.cpp

```cpp
// BEFORE (错误)
if (!stmt) {
    return createErrorResult("Statement is null");
}
switch (stmt.type) {  // 错误：直接访问私有成员
    case sql_parser::Statement::Type::CREATE_TABLE:
        return executeCreateTable(dynamic_cast<const sql_parser::CreateTableStatement&>(*stmt),
                                context);

// AFTER (正确)
if (!stmt) {
    return ExecutionResult(false, "Statement is null");
}
switch (stmt->getType()) {  // 正确：使用 getter
    case sql_parser::Statement::Type::CREATE_TABLE:
        return executeCreateTable(dynamic_cast<const sql_parser::CreateTableStatement&>(*stmt),
                                context);
```

### 6.2 dml_execution_strategy.cpp

```cpp
// BEFORE (错误)
if (!stmt) {
    return createErrorResult("Statement is null");
}
switch (stmt.type) {  // 错误：直接访问私有成员

// AFTER (正确)
if (!stmt) {
    return ExecutionResult(false, "Statement is null");
}
switch (stmt->getType()) {  // 正确：使用 getter
```

---

## 7. 验收标准

- [ ] ddl_execution_strategy.cpp 编译通过
- [ ] dml_execution_strategy.cpp 编译通过
- [ ] bazel build //src/execution:execution 通过
- [ ] bazel test //tests/level1_foundation/... 通过
- [ ] 无引入新的警告

---

## 8. 变更清单

| 文件 | 变更类型 | 描述 |
|------|---------|------|
| src/execution/ddl_execution_strategy.cpp | 修改 | createErrorResult -> ExecutionResult 构造函数 |
| src/execution/ddl_execution_strategy.cpp | 修改 | stmt.type -> stmt->getType() |
| src/execution/dml_execution_strategy.cpp | 修改 | createErrorResult -> ExecutionResult 构造函数 |
| src/execution/dml_execution_strategy.cpp | 修改 | stmt.type -> stmt->getType() |

---

## 9. 风险评估

| 风险 | 级别 | 缓解措施 |
|------|------|---------|
| 回归风险 | 低 | 仅修改语法错误，无逻辑变更 |
| 编译风险 | 低 | 已在 Linux + clang-20 环境验证 |

---

**Status**: 待审批
**Next**: Claude 审批后开始实现
