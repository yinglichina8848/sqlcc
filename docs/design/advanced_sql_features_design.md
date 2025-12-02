# SQLCC 高级SQL功能设计文档

## 1. 概述

本文档旨在为SQLCC数据库系统设计高级SQL功能的实现方案。基于SQLCC v1.0.5版本的评估，当前系统缺少以下高级SQL功能：

### 1.1 缺失功能分类
1. **复杂查询功能**：窗口函数、CTE、递归查询、集合操作
2. **高级JOIN支持**：FULL OUTER JOIN、CROSS JOIN、NATURAL JOIN、复杂ON条件
3. **子查询增强**：相关子查询、EXISTS/NOT EXISTS、IN/ANY/ALL、标量子查询
4. **聚合和分组增强**：HAVING子句、GROUPING SETS、ROLLUP/CUBE、窗口聚合

### 1.2 设计目标
- 保持与现有统一查询计划架构的兼容性
- 提供可扩展的设计，便于未来功能增强
- 确保性能可接受，避免引入重大性能瓶颈
- 支持标准的SQL语法和语义

## 2. 总体架构设计

### 2.1 基于统一查询计划的扩展

SQLCC现有的统一查询计划架构提供了良好的扩展基础。高级SQL功能将通过以下方式集成：

```cpp
class UnifiedQueryPlan {
    // ... 现有基础 ...
    
    // 新增高级功能扩展点
    virtual bool analyzeAdvancedFeatures();    // 分析高级功能
    virtual bool optimizeAdvancedQueries();    // 优化高级查询
    virtual ExecutionResult executeAdvanced(); // 执行高级功能
};
```

### 2.2 模块化设计

每个高级功能将作为独立的模块实现：

```
sqlcc/
├── src/sql_parser/
│   ├── advanced_ast/          # 高级AST节点
│   │   ├── window_function_node.cpp
│   │   ├── cte_node.cpp
│   │   └── recursive_query_node.cpp
│   └── advanced_parser/       # 高级语法解析
│       ├── window_parser.cpp
│       └── cte_parser.cpp
├── src/sql_executor/
│   └── advanced_executor/     # 高级执行器
│       ├── window_executor.cpp
│       ├── cte_executor.cpp
│       └── subquery_executor.cpp
└── include/sql_parser/
    └── advanced/
        ├── window_functions.h
        └── common_table_expressions.h
```

## 3. 复杂查询功能设计

### 3.1 窗口函数实现

#### 3.1.1 AST节点扩展

```cpp
class WindowFunctionNode : public ExpressionNode {
public:
    WindowFunctionNode(const std::string& function_name,
                      std::vector<std::unique_ptr<ExpressionNode>> args,
                      std::unique_ptr<WindowSpecificationNode> window_spec);
    
private:
    std::string function_name_;
    std::vector<std::unique_ptr<ExpressionNode>> args_;
    std::unique_ptr<WindowSpecificationNode> window_spec_;
};

class WindowSpecificationNode : public ASTNode {
public:
    struct Frame {
        enum class Type { ROWS, RANGE, GROUPS };
        enum class Bound { UNBOUNDED_PRECEDING, PRECEDING, CURRENT_ROW, FOLLOWING, UNBOUNDED_FOLLOWING };
        
        Type type;
        Bound start_bound;
        Bound end_bound;
        int start_offset;  // 用于PRECEDING/FOLLOWING
        int end_offset;    // 用于PRECEDING/FOLLOWING
    };
    
private:
    std::vector<std::unique_ptr<ExpressionNode>> partition_by_;
    std::vector<std::unique_ptr<ExpressionNode>> order_by_;
    std::unique_ptr<Frame> frame_;
};
```

#### 3.1.2 支持的窗口函数
1. **排名函数**：ROW_NUMBER(), RANK(), DENSE_RANK(), PERCENT_RANK()
2. **分析函数**：FIRST_VALUE(), LAST_VALUE(), NTH_VALUE(), LAG(), LEAD()
3. **聚合窗口函数**：SUM(), AVG(), COUNT(), MIN(), MAX() OVER窗口

#### 3.1.3 执行流程
1. 解析阶段：识别窗口函数语法，构建WindowFunctionNode
2. 计划构建阶段：创建窗口函数执行计划
3. 执行阶段：按分区排序，计算窗口函数值
4. 结果合并：将窗口函数结果与原始查询结果合并

### 3.2 公用表表达式(CTE)实现

#### 3.2.1 CTE语法支持
```sql
WITH cte_name (col1, col2) AS (
    SELECT col1, col2 FROM table1
    WHERE condition
)
SELECT * FROM cte_name JOIN table2 ON ...
```

#### 3.2.2 AST节点设计
```cpp
class CommonTableExpressionNode : public StatementNode {
public:
    CommonTableExpressionNode(const std::string& cte_name,
                             std::vector<std::string> column_names,
                             std::unique_ptr<SelectStatementNode> query);
    
private:
    std::string cte_name_;
    std::vector<std::string> column_names_;
    std::unique_ptr<SelectStatementNode> query_;
};

class WithClauseNode : public ASTNode {
public:
    void addCTE(std::unique_ptr<CommonTableExpressionNode> cte);
    
private:
    std::vector<std::unique_ptr<CommonTableExpressionNode>> ctes_;
};
```

#### 3.2.3 执行策略
1. **物化策略**：执行CTE查询，将结果存储在临时表中
2. **内联策略**：将CTE内联到主查询中（适用于简单CTE）
3. **递归CTE**：支持递归查询，使用迭代执行方式

### 3.3 递归查询实现

#### 3.3.1 语法扩展
```sql
WITH RECURSIVE cte_name (col1, col2) AS (
    -- 初始查询
    SELECT col1, col2 FROM table1
    UNION ALL
    -- 递归部分
    SELECT ... FROM cte_name WHERE ...
)
SELECT * FROM cte_name;
```

#### 3.3.2 执行算法
1. **初始步骤**：执行非递归部分，获取初始结果集
2. **递归步骤**：重复执行递归部分，直到结果集为空
3. **终止条件**：达到最大递归深度或结果集不再变化
4. **结果合并**：合并所有迭代的结果

### 3.4 集合操作实现

#### 3.4.1 支持的操作
- UNION：合并结果集，去重
- UNION ALL：合并结果集，不去重
- INTERSECT：交集
- EXCEPT：差集

#### 3.4.2 实现策略
```cpp
class SetOperationNode : public QueryNode {
public:
    enum class Operation { UNION, UNION_ALL, INTERSECT, EXCEPT };
    
    SetOperationNode(Operation op,
                    std::unique_ptr<QueryNode> left,
                    std::unique_ptr<QueryNode> right);
    
private:
    Operation operation_;
    std::unique_ptr<QueryNode> left_;
    std::unique_ptr<QueryNode> right_;
};
```

## 4. 高级JOIN支持设计

### 4.1 JOIN类型扩展

#### 4.1.1 FULL OUTER JOIN实现
```cpp
class FullOuterJoinNode : public JoinNode {
public:
    FullOuterJoinNode(std::unique_ptr<TableReferenceNode> left,
                     std::unique_ptr<TableReferenceNode> right,
                     std::unique_ptr<ExpressionNode> condition);
    
    // 执行策略：LEFT JOIN结果 ∪ RIGHT JOIN结果
};
```

#### 4.1.2 CROSS JOIN实现
```cpp
class CrossJoinNode : public JoinNode {
public:
    CrossJoinNode(std::unique_ptr<TableReferenceNode> left,
                 std::unique_ptr<TableReferenceNode> right);
    
    // 执行策略：笛卡尔积
};
```

#### 4.1.3 NATURAL JOIN实现
```cpp
class NaturalJoinNode : public JoinNode {
public:
    NaturalJoinNode(std::unique_ptr<TableReferenceNode> left,
                   std::unique_ptr<TableReferenceNode> right);
    
    // 自动推导连接条件：相同列名的等值连接
};
```

### 4.2 复杂ON条件支持

#### 4.2.1 多条件表达式
支持AND、OR、NOT等逻辑运算符组合的复杂连接条件：
```sql
SELECT * FROM t1 JOIN t2 ON t1.a = t2.a AND (t1.b > t2.b OR t1.c IS NULL)
```

#### 4.2.2 非等值连接
支持>、<、>=、<=、!=等比较运算符的连接条件。

## 5. 子查询增强设计

### 5.1 相关子查询实现

#### 5.1.1 执行策略
1. **嵌套循环**：对外部查询的每一行执行子查询
2. **物化**：执行子查询一次，缓存结果
3. **半连接转换**：将相关子查询转换为JOIN操作

#### 5.1.2 AST节点
```cpp
class CorrelatedSubqueryNode : public SubqueryNode {
public:
    CorrelatedSubqueryNode(std::unique_ptr<SelectStatementNode> subquery,
                          const std::vector<std::string>& correlated_columns);
    
private:
    std::vector<std::string> correlated_columns_; // 外部查询引用的列
};
```

### 5.2 EXISTS/NOT EXISTS实现

#### 5.2.1 执行优化
- **半连接转换**：EXISTS转换为SEMI JOIN
- **反连接转换**：NOT EXISTS转换为ANTI JOIN
- **去重优化**：利用索引避免重复计算

### 5.3 IN/ANY/ALL实现

#### 5.3.1 支持类型
- IN：成员检查
- ANY/SOME：与子查询结果的任意值比较
- ALL：与子查询结果的所有值比较

#### 5.3.2 优化策略
- **哈希连接**：对IN子查询使用哈希表
- **排序合并**：对有序子查询使用排序合并
- **索引查找**：利用索引加速成员检查

### 5.4 标量子查询实现

#### 5.4.1 限制条件
- 必须返回单行单列
- 可在SELECT列表、WHERE条件、HAVING子句中使用

#### 5.4.2 执行策略
- **缓存结果**：对于非相关标量子查询，缓存执行结果
- **延迟执行**：在需要时执行，避免不必要的计算

## 6. 聚合和分组增强设计

### 6.1 HAVING子句实现

#### 6.1.1 语法位置
```sql
SELECT column, aggregate_function(column)
FROM table
WHERE condition
GROUP BY column
HAVING aggregate_function(column) > value;
```

#### 6.1.2 执行流程
1. WHERE条件过滤
2. GROUP BY分组
3. 聚合函数计算
4. HAVING条件过滤

### 6.2 GROUPING SETS实现

#### 6.2.1 语法支持
```sql
SELECT column1, column2, SUM(column3)
FROM table
GROUP BY GROUPING SETS (
    (column1, column2),
    (column1),
    (column2),
    ()
);
```

#### 6.2.2 执行策略
1. 为每个分组集单独执行分组操作
2. 使用UNION ALL合并结果
3. 添加GROUPING()函数标识分组级别

### 6.3 ROLLUP/CUBE实现

#### 6.3.1 ROLLUP层次分组
```sql
SELECT column1, column2, SUM(column3)
FROM table
GROUP BY ROLLUP (column1, column2);
```

#### 6.3.2 CUBE多维分组
```sql
SELECT column1, column2, SUM(column3)
FROM table
GROUP BY CUBE (column1, column2);
```

#### 6.3.3 实现算法
- 基于GROUPING SETS的语法糖
- 自动生成所有需要的分组组合

### 6.4 窗口聚合实现

#### 6.4.1 与普通聚合的区别
- 不减少行数，为每行计算聚合值
- 支持滑动窗口定义
- 可与OVER()子句配合使用

#### 6.4.2 执行优化
- **增量计算**：对于滑动窗口，重用部分计算结果
- **并行计算**：窗口函数可并行执行

## 7. 性能优化设计

### 7.1 查询优化器增强

#### 7.1.1 基于成本的优化
- 为高级功能添加成本估算
- 选择最优的执行策略
- 考虑内存使用和I/O成本

#### 7.1.2 重写优化
- 子查询展开
- 谓词下推
- 连接顺序优化

### 7.2 执行引擎优化

#### 7.2.1 流水线执行
- 窗口函数的流式处理
- CTE的惰性求值
- 并行执行集合操作

#### 7.2.2 内存管理
- 窗口函数的缓冲区管理
- CTE结果的缓存策略
- 递归查询的深度控制

## 8. 兼容性和迁移考虑

### 8.1 向后兼容性
- 现有SQL语句应继续正常工作
- 新增功能不应影响现有性能
- 错误处理保持一致性

### 8.2 配置选项
```cpp
struct AdvancedSQLConfig {
    bool enable_window_functions = true;
    bool enable_cte = true;
    bool enable_recursive_queries = false; // 默认禁用，需要显式启用
    size_t max_recursion_depth = 100;
    size_t window_buffer_size = 1024 * 1024; // 1MB
};
```

### 8.3 错误处理
- 清晰的错误消息
- 适当的资源限制
- 优雅的失败恢复

## 9. 测试策略

### 9.1 单元测试
- 每个AST节点的解析测试
- 每个执行器的功能测试
- 边界条件测试

### 9.2 集成测试
- 完整查询的端到端测试
- 性能基准测试
- 并发执行测试

### 9.3 回归测试
- 确保新功能不影响现有功能
- 性能回归测试
- 内存泄漏检测

## 10. 实施路线图

### 10.1 第一阶段：基础框架
1. 扩展AST节点定义
2. 实现基础解析器扩展
3. 创建测试框架

### 10.2 第二阶段：核心功能
1. 实现窗口函数
2. 实现CTE支持
3. 实现HAVING子句

### 10.3 第三阶段：高级功能
1. 实现高级JOIN
2. 实现子查询增强
3. 实现集合操作

### 10.4 第四阶段：优化和完善
1. 性能优化
2. 错误处理完善
3. 文档和示例

## 11. 总结

本设计文档为SQLCC的高级SQL功能实现提供了全面的技术方案。通过模块化设计和基于现有统一查询计划架构的扩展，可以逐步实现所有缺失的高级功能，同时保持系统的稳定性和性能。

建议按照实施路线图分阶段进行开发，每个阶段都进行充分的测试和验证，确保功能的正确性和系统的可靠性。
