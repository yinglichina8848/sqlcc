# SQL-92 支持差距矩阵（概要）

说明：对于每一条 SQL-92 特性，标注当前实现状态（Implemented / Partial/MVP / Not Implemented）以及建议优先级（P0/P1/P2）。此文为快速审计结果，基于代码库扫描与已有文档（v1.1.4/v1.1.5 评估）得出。

- 聚合函数
  - COUNT / SUM: MVP implemented (parser 收到文本函数，如 `count(*)` / `sum(col)`；executor 有简单 quick-path 逐行计算) — 状态: MVP, 优先级: P0
  - AVG / MIN / MAX / DISTINCT in aggregates: Not Implemented (planned) — 状态: Not Implemented, 优先级: P0

- GROUP BY / HAVING
  - GROUP BY: Not Implemented (parser/AST/docs 已有计划条目) — 状态: Not Implemented, 优先级: P0
  - HAVING: Not Implemented — 状态: Not Implemented, 优先级: P0

- JOIN 扩展
  - INNER JOIN: 基本支持（需逐项验证） — 状态: Partial, 优先级: P0
  - LEFT/RIGHT/FULL JOIN: Not Implemented / 需扩展 — 状态: Not Implemented, 优先级: P0

- SELECT 语法
  - DISTINCT: 词法识别存在，但 Select 层未完整支持 — 状态: Partial, 优先级: P0
  - UNION / INTERSECT / EXCEPT: Parser 中 `parseUnion()` 未实现（存在 archive 代码片段） — 状态: Not Implemented, 优先级: P0
  - INSERT ... SELECT: Not Implemented / 未完整集成 — 状态: Not Implemented, 优先级: P0

- 子查询与视图
  - 子查询（相关/非相关）: Not Implemented / 规划中 — 状态: Not Implemented, 优先级: P1
  - CREATE VIEW: Not Implemented — 状态: Not Implemented, 优先级: P1

- 索引与约束
  - CREATE/ALTER INDEX: Parser 有相关 debug 路径，功能需逐步实现 — 状态: Partial, 优先级: P1
  - 外键 / CHECK / UNIQUE: Not Implemented — 状态: Not Implemented, 优先级: P1

- 权限与管理
  - 角色管理 / 权限继承: Not Implemented — 状态: Not Implemented, 优先级: P1

- 进阶特性（后期）
  - 窗口函数: Not Implemented — 状态: Not Implemented, 优先级: P2
  - CTE / 递归 CTE: Not Implemented — 状态: Not Implemented, 优先级: P2
  - 存储过程 / UDF: Not Implemented — 状态: Not Implemented, 优先级: P2

- 备注与建议行动项（短期）
  1. 将 `COUNT`/`SUM` 的 MVP 实现纳入回归测试并固定 API/AST 表达（已完成 parser 测试）。
  2. 立即实现 `GROUP BY` 的 parser/AST 扩展并设计 `AggregatePlan`（P0-A），随后实现内存 hash 聚合算子（P0-C）。
  3. 将 `DISTINCT`、`UNION`、`LEFT/RIGHT/FULL JOIN` 列为并行 P0 子任务，分配到下一开发冲刺。

- 下一步交付（短期）
  - 交付：`docs/项目进展/v1.1.5/SQL-92_gap_matrix.md`（本文件）
  - 将差距矩阵结果合并回项目 TODO 并为 P0 项目分配更详细里程碑（Planner/Executor 子任务）


(审计由自动扫描与先前工作日志汇总得出；欲要更细粒度的 feature-by-feature 补丁映射与代码引用，我可以继续生成一份带文件/行号引用的详细清单。)