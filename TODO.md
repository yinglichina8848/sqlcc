## 待办（精简）

目标：按优先级（P0/P1/P2）推进 SQL-92 支持，当前聚焦 P0（聚合、GROUP BY、JOIN 扩展等）。

- P0（近期）
  - 聚合：COUNT/SUM/AVG/MIN/MAX；`GROUP BY`、`HAVING`（里程碑与工时已在 docs）
  - 扩展 JOIN：支持 LEFT/RIGHT/FULL
  - SELECT DISTINCT 与 UNION
  - INSERT ... SELECT

- P1（中期）
  - 子查询（相关/非相关）
  - CREATE VIEW、索引管理（CREATE/ALTER INDEX）
  - 高级约束（FK/CHECK/UNIQUE）
  - 角色与权限管理

- P2（后期）
  - 窗口函数、CTE（含递归）、存储过程/UDF

状态与下一步：已完成聚合函数 Parser/AST MVP 与差距审计；下一步为实现 Parser 的 `GROUP BY` 支持并推进 Planner/Executor 设计。

参见：docs/项目进展/v1.1.6/P0_聚合_里程碑_简明.md
