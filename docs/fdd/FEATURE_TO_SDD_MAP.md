# SQLCC FDD → SDD 映射表

**版本**: v1.0  
**日期**: 2026-02-06  
**说明**: 连接功能分解与 SDD 规范的映射关系。

---

## 1. 映射规则

- 每个 FDD 功能点必须对应一个 SDD feature
- 每个 SDD feature 必须提供 requirements/design/tasks/verification

---

## 2. 映射表

| FDD 功能点 | SDD Feature | 状态 | 备注 |
|---|---|---|---|
| 存储引擎核心能力 | `docs/sdd/features/storage_engine/` | 已建立 | 待细化 |
| SQL Parser 核心解析 | `docs/sdd/features/sql_parser/` | 已建立 | 待细化 |
| 事务管理与一致性 | `docs/sdd/features/transaction_manager/` | 已建立 | 待细化 |
| 核心管理与接口稳定 | `docs/sdd/features/core/` | 已建立 | 待细化 |
| 线程池基础组件 | `docs/sdd/features/thread_pool/` | 已建立 | 已补齐验证 |

