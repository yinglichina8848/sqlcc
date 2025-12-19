## 待办（精简）

### ✅ v1.3.0 高级SQL-92特性 - 已完成 (2025-12-18)
**里程碑成就**: SQLCC正式迈入企业级数据库系统行列

- ✅ **存储过程和函数系统**: CREATE FUNCTION语法支持，FunctionExecutor高性能执行引擎
- ✅ **触发器完整实现**: BEFORE/AFTER触发器，SQLTriggerExecutor，:OLD/:NEW变量引用
- ✅ **事务控制增强**: SAVEPOINT保存点管理，嵌套事务支持，SavepointManager
- ✅ **用户定义类型**: DOMAIN自定义类型，约束验证，DomainManager
- ✅ **综合测试套件**: 16个单元测试用例，AdvancedSQL92Test完整覆盖
- ✅ **企业级架构**: 模块化设计，高并发安全，完整的错误处理

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
