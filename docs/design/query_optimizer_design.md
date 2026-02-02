# SQLCC 查询优化器设计文档

## 1. WHY: 为什么要设计查询优化器？

SQL 是一种声明式语言，用户只需说明“想要什么” (What)，而不需要说明“如何去拿” (How)。同一个查询可能有成百上千种执行路径，它们的性能差异可能高达数万倍。

查询优化器的职责就是自动寻找**成本最低**的执行路径。

### 优化器解决的核心问题：
*   **连接顺序 (Join Ordering)**: 在多表连接中，先连接哪两个表？小表驱动大表可以显著减少中间结果集。
*   **访问路径选择 (Access Path Selection)**: 是全表扫描 (Table Scan) 更快，还是利用索引 (Index Scan) 更快？
*   **物理算子选择**: 是使用嵌套循环连接 (Nested Loop Join)，还是哈希连接 (Hash Join)？
*   **逻辑等价变换**: 通过代数变形（如谓词下推），在不改变结果的前提下提前减少数据量。

## 2. WHAT: 查询优化器是什么？

查询优化器是 SQL 执行流程中承上启下的核心。它接收解析器生成的逻辑计划，输出经过优化的物理执行计划。

### 优化器的两个主要阶段：
1.  **基于规则的优化 (Rule-Based Optimization, RBO)**:
    *   应用预定义的启发式规则。
    *   **谓词下推 (Predicate Pushdown)**: 将 `WHERE` 过滤尽可能推向数据源。
    *   **列裁剪 (Column Pruning)**: 只读取 `SELECT` 中提到的列，减少 I/O。
    *   **常量折叠**: 将 `WHERE a = 1 + 2` 变为 `WHERE a = 3`。

2.  **基于成本的优化 (Cost-Based Optimization, CBO)**:
    *   利用统计信息（表行数、列分布、索引基数）。
    *   计算不同计划的预估 I/O 成本和 CPU 成本。
    *   选择总成本分值最低的计划。

## 3. HOW: 查询优化器是如何实现的？

### 3.1. 逻辑计划与物理计划
*   **逻辑计划 (Logical Plan)**: 抽象的操作序列（如 `Join`, `Filter`, `Project`），不涉及具体算法。
*   **物理计划 (Physical Plan)**: 具体的执行算子（如 `IndexScan`, `HashJoin`），包含具体的存储访问逻辑。

### 3.2. 谓词下推 (Predicate Pushdown) 示例
```sql
SELECT name FROM users JOIN orders ON users.id = orders.user_id WHERE users.age > 20;
```
*   **不优化**: 先把 `users` 和 `orders` 全部 JOIN，再过滤 `age > 20`。
*   **优化后**: 先在 `users` 表上过滤出 `age > 20` 的小结果集，再与 `orders` 进行 JOIN。

### 3.3. 成本模型 (Cost Model)
SQLCC 采用简化的成本模型：
`Cost = (NumPages * IO_WEIGHT) + (NumTuples * CPU_WEIGHT)`
*   `IO_WEIGHT`: 衡量从磁盘读取页面的权重。
*   `CPU_WEIGHT`: 衡量在内存中处理一条记录的权重。

### 3.4. 连接顺序优化 (Join Reordering)
对于 N 个表的连接，理论上有 N! 种排列。
*   对于小规模连接，使用动态规划 (DP) 寻找全局最优。
*   对于大规模连接，使用贪心算法或启发式搜索。

## 4. 优化流程示意图

```mermaid
graph LR
    A[AST] --> B(Plan Generator)
    B --> C[Logical Plan]
    C --> D(RBO: Rules)
    D --> E[Optimized Logical Plan]
    E --> F(CBO: Cost Model)
    F --> G[Physical Execution Plan]
    G --> H(Unified Executor)
```

## 5. 关键组件职责

*   **`ExecutionPlanGenerator`**: 负责将 AST 转换为初始的逻辑计划树。
*   **`CostEstimator`**: 负责估算每个算子的产出行数和执行代价。
*   **`QueryPlanFactory`**: 负责根据优化器的决策创建具体的执行算子对象。

## 6. 未来演进方向

*   **直方图统计 (Histogram)**: 提供更精确的数据分布估算。
*   **自适应执行 (Adaptive Execution)**: 根据运行时收集的实际行数动态调整后续的连接算法。
*   **并行查询优化**: 考虑多核 CPU 的并行处理能力进行任务分配优化。
