# SQLCC 架构设计文档索引

**版本**: v1.4.0  
**创建日期**: 2026-02-03  
**作者**: 高小原 🌱  
**状态**: 文档索引和分层指南

---

## 文档说明

本文档是 SQLCC 项目所有设计文档的索引和分层指南，帮助开发者快速定位所需的文档。

---

## 目录

1. [宏观架构文档](#一宏观架构文档)
2. [分层设计文档](#二分层设计文档)
3. [模块设计文档](#三模块设计文档)
4. [详细设计文档](#四详细设计文档)
5. [重构相关文档](#五重构相关文档)
6. [按组件分类的文档](#六按组件分类的文档)
7. [如何使用本文档](#七如何使用本文档)

---

## 一、宏观架构文档

**位置**: `docs/design/` - 顶层架构说明

| 序号 | 文档名 | 说明 | 阅读优先级 |
|------|--------|------|-----------|
| 1 | `Architecture.md` | SQLCC 整体架构说明 | ⭐⭐⭐ 必读 |
| 2 | `OVERALL_DESIGN.md` | 系统总体设计文档 | ⭐⭐⭐ 必读 |
| 3 | `PROJECT_STRUCTURE.md` | 项目目录结构与模块结构 | ⭐⭐⭐ 必读 |

### 1.1 Architecture.md 核心内容

**文档地址**: `docs/design/Architecture.md`

**What**: SQLCC 是什么？  
**Why**: 为什么这样架构？  
**How**: 怎样实现？

**核心架构图**:
```
+=====================================================================+
|                        企业级应用层                                  |
+===========================+===================+=====================+
                            |                   |
+===========================v===================v=====================+
|                      云原生接口层                                   |
|  +------------------+  +------------------+  +--------------------+ |
|  |  SQL-92解析器    |  |  统一执行引擎    |  |  多任务并发管理器   | |
|  +------------------+  +------------------+  +--------------------+ |
+===========================+===================+=====================+
                            |                   |
+===========================v===================v=====================+
|                      内存安全核心层                                 |
|  +------------------+  +------------------+  +--------------------+ |
|  | 分片式存储引擎   |  |  ACID事务管理   |  |  智能配置管理器    | |
|  +------------------+  +------------------+  +--------------------+ |
+===========================+===================+=====================+
                            |                   |
+===========================v===================v=====================+
|                      企业级基础设施层                              |
+=====================================================================+
```

**核心特性**:
- **内存安全 A++ 等级**: 95%+ 智能指针化
- **分片式存储引擎**: 16 分片缓冲池
- **统一执行引擎**: SQL-92 完整支持
- **ACID 事务**: MVCC, 2PL, WAL

### 1.2 OVERALL_DESIGN.md 核心内容

**文档地址**: `docs/design/OVERALL_DESIGN.md`

**SQLCC**: Simple Compact Course Computer  
**目标**: 教学目的的轻量级关系型数据库

**分层架构**:
```
客户端层 → 服务器层 → 核心引擎层 → 存储层
```

**核心模块**:
- SQL 解析器
- 查询执行器
- 事务管理器
- 锁管理器
- 配置管理器
- 存储引擎
- 索引系统
- 缓冲池
- 磁盘管理器
- WAL 日志

### 1.3 PROJECT_STRUCTURE.md 核心内容

**文档地址**: `docs/design/PROJECT_STRUCTURE.md`

**项目目录结构**:
```
sqlcc/
├── src/               # 源代码
├── include/           # 头文件
├── tests/             # 测试目录
├── docs/              # 文档目录
├── config/            # 配置文件
└── scripts/           # 辅助脚本
```

**模块目录**:
```
src/
├── config_manager/    # 配置管理器
├── network/          # 网络模块
├── sql_executor/     # SQL执行器
├── sql_parser/       # SQL解析器
├── storage_engine/   # 存储引擎
└── transaction_manager/ # 事务管理器
```

---

## 二、分层设计文档

**位置**: `docs/design/` - 按层次分类

### 2.1 Level 1: 基础层

**位置**: `tests/level1_foundation/`

| 文档 | 说明 |
|------|------|
| 工具类测试 | SmartConfigManager, ThreadPool |
| 类型系统测试 | types_test |
| 工具函数测试 | utils_test |

### 2.2 Level 2: 核心层

**位置**: `tests/level2_core/`

**当前重构目标**:
- Core ↔ Storage 解耦
- Core ↔ Execution 解耦

### 2.3 Level 3: SQL 处理层

**位置**: `tests/level3_sql/`

| 文档 | 说明 |
|------|------|
| SQL 解析器测试 | Lexer, Parser, AST |
| 执行器测试 | SqlExecutor |
| 优化器测试 | QueryOptimizer |

### 2.4 Level 4: 网络层

**位置**: `tests/level4_network/`

| 文档 | 说明 |
|------|------|
| 协议处理测试 | Network Protocol |
| 连接管理测试 | Connection Manager |
| 消息传递测试 | Message Passing |

### 2.5 Level 5: 集成层

**位置**: `tests/level5_integration/`

| 文档 | 说明 |
|------|------|
| 端到端测试 | End-to-End Tests |
| 性能测试 | Performance Tests |
| 压力测试 | Stress Tests |

### 2.6 Level 6: 系统层

**位置**: `tests/level6_system/`

| 文档 | 说明 |
|------|------|
| 完整系统测试 | Full System Tests |
| 故障恢复测试 | Recovery Tests |
| 监控测试 | Monitoring Tests |

---

## 三、模块设计文档

### 3.1 SQL 解析器模块

**位置**: `docs/design/sql_parser/`

| 序号 | 文档名 | 说明 | 阅读优先级 |
|------|--------|------|-----------|
| 1 | `sql_parser_design.md` | SQL解析模块完整设计 | ⭐⭐⭐ 必读 |
| 2 | `AST节点结构设计.md` | AST节点结构详细说明 | ⭐⭐ 推荐 |
| 3 | `SQL语法规则BNF.md` | SQL语法规则定义 | ⭐⭐ 推荐 |
| 4 | `Token类型系统设计.md` | Token类型系统设计 | ⭐ 推荐 |

#### 3.1.1 核心组件

```
SQL文本 → 词法分析器(Lexer) → Token流 → 语法分析器(Parser) → AST
```

#### 3.1.2 支持的 SQL 语句类型

| 类型 | 示例 |
|------|------|
| DDL | CREATE DATABASE/TABLE/INDEX, ALTER, DROP |
| DML | INSERT, UPDATE, DELETE |
| DQL | SELECT (支持 JOIN, WHERE, GROUP BY, HAVING, ORDER BY) |
| DCL | CREATE USER, GRANT, REVOKE |
| TCL | BEGIN, COMMIT, ROLLBACK, SAVEPOINT |

### 3.2 SQL 执行器模块

**位置**: `docs/design/sql_executor/`

| 序号 | 文档名 | 说明 | 阅读优先级 |
|------|--------|------|-----------|
| 1 | `sql_executor_design.md` | SQL执行器完整设计 | ⭐⭐⭐ 必读 |
| 2 | `sql_executor_refactor_design.md` | 执行器重构设计 | ⭐⭐ 推荐 |
| 3 | `unified_execution_engine.md` | 统一执行引擎设计 | ⭐⭐ 推荐 |

#### 3.2.1 核心组件

```
AST → SqlExecutor → 存储引擎 → 磁盘
           ↑
      事务管理器
```

#### 3.2.2 支持的 SQL 语句类型

| 类型 | 执行方法 |
|------|---------|
| DDL | ExecuteCreate, ExecuteDrop, ExecuteAlter |
| DML | ExecuteSelect, ExecuteInsert, ExecuteUpdate, ExecuteDelete |
| DCL | ExecuteGrant, ExecuteRevoke |
| TCL | ExecuteBegin, ExecuteCommit, ExecuteRollback |

### 3.3 存储引擎模块

**位置**: `docs/design/storage_engine/`

| 序号 | 文档名 | 说明 | 阅读优先级 |
|------|--------|------|-----------|
| 1 | `storage_engine_overview.md` | 存储引擎概述 | ⭐⭐⭐ 必读 |
| 2 | `storage_engine_design.md` | 存储引擎完整设计 | ⭐⭐⭐ 必读 |
| 3 | `buffer_pool_management_algorithm.md` | 缓冲池管理算法 | ⭐⭐ 推荐 |
| 4 | `b_plus_tree_algorithm_detailed.md` | B+树算法详解 | ⭐⭐ 推荐 |

#### 3.3.1 核心组件

```
StorageEngine
├── BufferPool (缓冲池)
│   ├── Page Table
│   ├── LRU List
│   └── Dirty Pages
├── DiskManager (磁盘管理)
│   ├── File I/O
│   └── Page Allocation
├── IndexManager (索引管理)
│   └── B+ Tree
└── TableStorage (表存储)
    └── Record Management
```

#### 3.3.2 关键技术

| 技术 | 说明 |
|------|------|
| 8KB 定长页面 | 标准页面大小 |
| 分片缓冲池 | 16 分片，减少锁竞争 |
| LRU 替换策略 | 最近最少使用 |
| B+ 树索引 | 高效范围查询 |

### 3.4 事务管理模块

**位置**: `docs/design/transaction_manager/`

| 序号 | 文档名 | 说明 | 阅读优先级 |
|------|--------|------|-----------|
| 1 | `transaction_system_design.md` | 事务系统完整设计 | ⭐⭐⭐ 必读 |
| 2 | `transaction_management_flow.md` | 事务管理流程 | ⭐⭐ 推荐 |
| 3 | `concurrency_control_design.md` | 并发控制设计 | ⭐⭐ 推荐 |
| 4 | `recovery_and_wal_design.md` | 恢复和WAL设计 | ⭐⭐ 推荐 |

#### 3.4.1 ACID 实现

| 属性 | 实现机制 |
|------|---------|
| 原子性 (Atomicity) | WAL (预写式日志) |
| 一致性 (Consistency) | 约束验证 |
| 隔离性 (Isolation) | MVCC, 2PL |
| 持久性 (Durability) | WAL + 磁盘同步 |

### 3.5 配置管理模块

**位置**: `docs/design/config_manager/`

| 文档 | 说明 |
|------|------|
| `configuration_lifecycle.md` | 配置生命周期管理 |
| `DESIGN_IMPROVEMENT_PLAN.md` | 设计改进计划 |

### 3.6 网络模块

**位置**: `docs/design/network/`

| 文档 | 说明 |
|------|------|
| 网络模块设计 | 待整理 |

---

## 四、详细设计文档

### 4.1 索引系统

**位置**: `docs/design/index_system/`

| 文档 | 说明 |
|------|------|
| `index_system_design.md` | 索引系统设计 |
| `b_plus_tree_index_design.md` | B+ 树索引设计 |

### 4.2 性能优化

**位置**: `docs/design/performance/`

| 文档 | 说明 |
|------|------|
| `query_optimization_algorithm.md` | 查询优化算法 |
| `multi_task_executor_design.md` | 多任务执行器设计 |

### 4.3 安全

**位置**: `docs/design/security/`

| 文档 | 说明 |
|------|------|
| `permission_validation_framework.md` | 权限验证框架 |
| `user_manager_design.md` | 用户管理设计 |

---

## 五、重构相关文档

### 5.1 Level 2 重构

**位置**: `docs/project/versions/v1.3.9/`

| 文档 | 说明 |
|------|------|
| `LEVEL2_REFACTORING_REPORT.md` | Level 2 重构报告 |

### 5.2 Level 3 重构（当前）

**位置**: `docs/project/versions/v1.4.0/`

| 序号 | 文档名 | 说明 | 状态 |
|------|--------|------|------|
| 1 | `ARCHITECTURE_ANALYSIS_REPORT.md` | 架构分析报告 | 待完善 |
| 2 | `../project/versions/v1.4.0/MACRO_DESIGN_FOR_NEWBIES.md` | 面向新手的宏观设计 | 已创建 |
| 3 | `../project/versions/v1.4.0/ISSUE_002_IBUFFERPOOL_DESIGN.md` | IBufferPool 接口设计讨论 | 已创建 |

---

## 六、按组件分类的文档

### 6.1 组件交互关系

| 文档 | 说明 |
|------|------|
| `component_interactions.md` | 组件交互关系图 |

```
Config Manager
    ↑
    ├── Storage Engine
    ├── Network Module
    └── SQL Executor
            ↑
            ├── Transaction Manager
            └── Storage Engine
```

### 6.2 实现状态

| 文档 | 说明 |
|------|------|
| `implementation_status.md` | 实现状态报告 |

---

## 七、如何使用本文档

### 7.1 新手学习路径

1. **第一天**: 阅读宏观架构文档
   - `Architecture.md`
   - `OVERALL_DESIGN.md`
   - `PROJECT_STRUCTURE.md`

2. **第二天**: 阅读感兴趣的模块
   - `sql_parser_design.md` (如果对解析感兴趣)
   - `storage_engine_overview.md` (如果对存储感兴趣)
   - `transaction_system_design.md` (如果对事务感兴趣)

3. **第三天**: 阅读详细设计
   - 对应模块的详细文档
   - 算法详解文档

### 7.2 重构参考路径

1. **理解问题**: 阅读 `ARCHITECTURE_ANALYSIS_REPORT.md`
2. **学习设计**: 阅读 `../project/versions/v1.4.0/MACRO_DESIGN_FOR_NEWBIES.md`
3. **参考实现**: 阅读 `storage_engine_overview.md` (What/Why/How 示例)

### 7.3 快速查找

**场景 1**: 想了解 SQL 解析器
→ 跳转至 [3.1 SQL 解析器模块](#31-sql-解析器模块)

**场景 2**: 想了解事务管理
→ 跳转至 [3.4 事务管理模块](#34-事务管理模块)

**场景 3**: 想了解当前重构进度
→ 跳转至 [五、重构相关文档](#五重构相关文档)

**场景 4**: 想了解 Level 1-6 分层
→ 跳转至 [二、分层设计文档](#二分层设计文档)

---

## 八、文档统计

| 分类 | 文档数量 |
|------|---------|
| 宏观架构 | 3 |
| 模块设计 | 5 大模块 |
| 详细设计 | 10+ |
| 重构文档 | 3+ |
| **总计** | **159+** |

---

## 九、待整理文档

以下文档需要进一步整理：

| 文档 | 说明 |
|------|------|
| `advanced_joins_design.md` | 高级连接设计 |
| `advanced_sql_features_design.md` | 高级 SQL 特性 |
| `aggregation_enhancements_design.md` | 聚合增强 |
| `complex_queries_design.md` | 复杂查询设计 |
| `subquery_enhancements_design.md` | 子查询增强 |

---

## 十、参考链接

### 10.1 内部链接

- [SQLCC 架构分析报告 v2](../project/versions/v1.4.0/ARCHITECTURE_ANALYSIS_REPORT_V2.md)
- [面向新手的宏观设计](../project/versions/v1.4.0/MACRO_DESIGN_FOR_NEWBIES.md)
- [IBufferPool 接口设计讨论](../project/versions/v1.4.0/ISSUE_002_IBUFFERPOOL_DESIGN.md)

### 10.2 外部参考

- SQL-92 标准
- 数据库系统概念 (经典教材)

---

**文档版本**: 1.0  
**创建时间**: 2026-02-03  
**最后更新**: 2026-02-03  
**作者**: 高小原 🌱
