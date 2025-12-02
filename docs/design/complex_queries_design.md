# SQLCC 复杂查询功能设计文档

## 1. 概述

本文档详细设计SQLCC数据库系统的复杂查询功能实现方案，包括：
- 窗口函数 (Window Functions)
- 公用表表达式 (Common Table Expressions, CTE)
- 递归查询 (Recursive Queries)
- 集合操作 (Set Operations: UNION, INTERSECT, EXCEPT)

## 2. 窗口函数设计

### 2.1 语法规范

#### 2.1.1 基本语法
```sql
SELECT 
    column1,
    column2,
    window_function() OVER (
        PARTITION BY partition_column
        ORDER BY order_column
        ROWS BETWEEN 1 PRECEDING AND 1 FOLLOWING
    ) AS window_result
FROM table_name;
```

#### 2.1.2 支持的窗口函数分类

1. **排名函数**
   - `ROW_NUMBER()`: 顺序排名，不重复
   - `RANK()`: 排名，相同值有并列，跳过后续名次
   - `DENSE_RANK()`: 排名，相同值有并列，不跳过名次
   - `PERCENT_RANK()`: 百分比排名
   - `NTILE(n)`: 分桶排名

2. **分析函数**
   - `FIRST_VALUE(expr)`: 窗口第一行的值
   - `LAST_VALUE(expr)`: 窗口最后一行的值
   - `NTH_VALUE(expr, n)`: 窗口第n行的值
   - `LAG(expr, offset, default)`: 前一行
   - `LEAD(expr, offset, default)`: 后一行

3. **聚合窗口函数**
   - `SUM(expr) OVER (...)`
   - `AVG(expr) OVER (...)`
   - `COUNT(expr) OVER (...)`
   - `MIN(expr) OVER (...)`
   - `MAX(expr) OVER (...)`

### 2.2 AST节点设计

#### 2.2.1 窗口函数节点
```cpp
class WindowFunctionNode : public ExpressionNode {
public:
    WindowFunctionNode(const std::string& function_name,
                      std::vector<std::unique_ptr<ExpressionNode>> args,
                      std::unique_ptr<WindowSpecificationNode> window_spec);
    
    const std::string& function_name() const { return function_name_; }
    const std::vector<std::unique_ptr<ExpressionNode>>& args() const { return args_; }
    const WindowSpecificationNode* window_spec() const { return window_spec_.get(); }
    
    void accept(NodeVisitor& visitor) override;
    
private:
    std::string function_name_;
    std::vector<std::unique_ptr<ExpressionNode>> args_;
    std::unique_ptr<WindowSpecificationNode> window_spec_;
};
```

#### 2.2.2 窗口规范节点
```cpp
class WindowSpecificationNode : public ASTNode {
public:
    // 窗口框架边界
    enum class BoundType {
        UNBOUNDED_PRECEDING,
        VALUE_PRECEDING,
        CURRENT_ROW,
        VALUE_FOLLOWING,
        UNBOUNDED_FOLLOWING
    };
    
    struct WindowFrame {
        enum class FrameType { ROWS, RANGE, GROUPS };
        
        FrameType type;
        BoundType start_bound;
        BoundType end_bound;
        int64_t start_value;  // 用于VALUE_PRECEDING/VALUE_FOLLOWING
        int64_t end_value;    // 用于VALUE_PRECEDING/VALUE_FOLLOWING
    };
    
    WindowSpecificationNode();
    
    void set_partition_by(std::vector<std::unique_ptr<ExpressionNode>> partition_by);
    void set_order_by(std::vector<std::unique_ptr<OrderByElement>> order_by);
    void set_frame(const WindowFrame& frame);
    
    const std::vector<std::unique_ptr<ExpressionNode>>& partition_by() const;
    const std::vector<std::unique_ptr<OrderByElement>>& order_by() const;
    const WindowFrame* frame() const;
    
    void accept(NodeVisitor& visitor) override;
    
private:
    std::vector<std::unique_ptr<ExpressionNode>> partition_by_;
    std::vector<std::unique_ptr<OrderByElement>> order_by_;
    std::optional<WindowFrame> frame_;
};
```

### 2.3 执行引擎设计

#### 2.3.1 窗口函数执行器
```cpp
class WindowFunctionExecutor {
public:
    struct WindowState {
        std::vector<Value> partition;      // 当前分区的数据
        std::vector<Value> sorted_partition; // 排序后的分区数据
        size_t current_row;                // 当前处理的行索引
        std::unordered_map<std::string, Value> function_states; // 各函数的状态
    };
    
    WindowFunctionExecutor(std::shared_ptr<ExecutionContext> context);
    
    // 执行窗口函数
    std::vector<Value> execute(const WindowFunctionNode& window_func,
                               const std::vector<std::vector<Value>>& input_rows);
    
private:
    // 各种窗口函数的实现
    Value execute_row_number(WindowState& state);
    Value execute_rank(WindowState& state);
    Value execute_dense_rank(WindowState& state);
    Value execute_first_value(WindowState& state, const ExpressionNode& expr);
    Value execute_lag(WindowState& state, const ExpressionNode& expr, int offset);
    Value execute_sum(WindowState& state, const ExpressionNode& expr);
    Value execute_avg(WindowState& state, const ExpressionNode& expr);
    
    // 辅助方法
    void partition_rows(const std::vector<std::vector<Value>>& input_rows,
                       const std::vector<ExpressionNode*>& partition_exprs,
                       std::vector<std::vector<std::vector<Value>>>& partitions);
    
    void sort_partition(std::vector<std::vector<Value>>& partition,
                       const std::vector<OrderByElement*>& order_by);
    
    std::shared_ptr<ExecutionContext> context_;
};
```

#### 2.3.2 执行流程
1. **分区阶段**: 根据PARTITION BY表达式将输入行分组
2. **排序阶段**: 在每个分区内根据ORDER BY表达式排序
3. **计算阶段**: 对每个窗口函数，按行计算值
4. **合并阶段**: 将窗口函数结果合并到原始行中

### 2.4 优化策略

#### 2.4.1 内存优化
- 流式处理：逐行处理，避免全量数据加载
- 缓冲区管理：根据窗口大小动态调整缓冲区
- 增量计算：对于滑动窗口，重用部分计算结果

#### 2.4.2 并行优化
- 分区级并行：不同分区可并行处理
- 函数级并行：不同的窗口函数可并行计算

## 3. 公用表表达式(CTE)设计

### 3.1 语法规范

#### 3.1.1 基本语法
```sql
WITH cte_name (column1, column2) AS (
    SELECT col1, col2 FROM source_table WHERE condition
)
SELECT * FROM cte_name;
```

#### 3.1.2 多CTE语法
```sql
WITH
    cte1 AS (SELECT ...),
    cte2 AS (SELECT ... FROM cte1 WHERE ...),
    cte3 AS (SELECT ... FROM cte2 JOIN ...)
SELECT * FROM cte3;
```

### 3.2 AST节点设计

#### 3.2.1 CTE节点
```cpp
class CommonTableExpressionNode : public ASTNode {
public:
    CommonTableExpressionNode(const std::string& name,
                             std::vector<std::string> column_names,
                             std::unique_ptr<QueryNode> query);
    
    const std::string& name() const { return name_; }
    const std::vector<std::string>& column_names() const { return column_names_; }
    const QueryNode* query() const { return query_.get(); }
    
    void accept(NodeVisitor& visitor) override;
    
private:
    std::string name_;
    std::vector<std::string> column_names_;
    std::unique_ptr<QueryNode> query_;
};
```

#### 3.2.2 WITH子句节点
```cpp
class WithClauseNode : public ASTNode {
public:
    WithClauseNode();
    
    void add_cte(std::unique_ptr<CommonTableExpressionNode> cte);
    const std::vector<std::unique_ptr<CommonTableExpressionNode>>& ctes() const;
    
    // 查找CTE定义
    const CommonTableExpressionNode* find_cte(const std::string& name) const;
    
    void accept(NodeVisitor& visitor) override;
    
private:
    std::vector<std::unique_ptr<CommonTableExpressionNode>> ctes_;
    std::unordered_map<std::string, CommonTableExpressionNode*> cte_map_;
};
```

### 3.3 执行引擎设计

#### 3.3.1 CTE执行器
```cpp
class CTEExecutor {
public:
    struct CTEContext {
        std::string name;
        std::vector<std::string> column_names;
        std::shared_ptr<TemporaryTable> result_table;
        bool is_materialized;
        bool is_recursive;
    };
    
    CTEExecutor(std::shared_ptr<ExecutionContext> context);
    
    // 执行WITH子句
    std::unordered_map<std::string, CTEContext> execute_with_clause(
        const WithClauseNode& with_clause);
    
    // 执行单个CTE
    CTEContext execute_cte(const CommonTableExpressionNode& cte,
                          const std::unordered_map<std::string, CTEContext>& cte_contexts);
    
private:
    // 执行策略选择
    enum class ExecutionStrategy {
        MATERIALIZED,   // 物化执行
        INLINE,         // 内联执行
        RECURSIVE       // 递归执行
    };
    
    ExecutionStrategy choose_strategy(const CommonTableExpressionNode& cte);
    
    // 物化执行
    CTEContext execute_materialized(const CommonTableExpressionNode& cte);
    
    // 内联执行
    CTEContext execute_inline(const CommonTableExpressionNode& cte,
                             const std::unordered_map<std::string, CTEContext>& cte_contexts);
    
    std::shared_ptr<ExecutionContext> context_;
};
```

#### 3.3.2 执行策略

1. **物化策略**
   - 执行CTE查询，结果存储在临时表
   - 适用于多次引用的CTE
   - 需要额外存储空间

2. **内联策略**
   - 将CTE查询文本替换到引用位置
   - 适用于单次引用的简单CTE
   - 可能重复执行计算

3. **递归策略**（见第4节）

### 3.4 优化策略

#### 3.4.1 CTE共享
- 多个查询引用同一CTE时共享结果
- 基于CTE内容的哈希值进行识别

#### 3.4.2 惰性求值
- 仅在需要时才执行CTE
- 支持CTE结果缓存

## 4. 递归查询设计

### 4.1 语法规范

#### 4.1.1 递归CTE语法
```sql
WITH RECURSIVE cte_name (col1, col2) AS (
    -- 初始部分 (Anchor Member)
    SELECT non_recursive_expression
    FROM source_table
    
    UNION ALL
    
    -- 递归部分 (Recursive Member)
    SELECT recursive_expression
    FROM cte_name
    WHERE recursion_condition
)
SELECT * FROM cte_name;
```

#### 4.1.2 递归查询示例
```sql
-- 组织架构层次查询
WITH RECURSIVE org_hierarchy AS (
    SELECT id, name, manager_id, 1 as level
    FROM employees
    WHERE manager_id IS NULL
    
    UNION ALL
    
    SELECT e.id, e.name, e.manager_id, oh.level + 1
    FROM employees e
    JOIN org_hierarchy oh ON e.manager_id = oh.id
)
SELECT * FROM org_hierarchy;
```

### 4.2 AST节点设计

#### 4.2.1 递归CTE节点
```cpp
class RecursiveCTENode : public CommonTableExpressionNode {
public:
    RecursiveCTENode(const std::string& name,
                    std::vector<std::string> column_names,
                    std::unique_ptr<QueryNode> anchor_member,
                    std::unique_ptr<QueryNode> recursive_member);
    
    const QueryNode* anchor_member() const { return anchor_member_.get(); }
    const QueryNode* recursive_member() const { return recursive_member_.get(); }
    
    void accept(NodeVisitor& visitor) override;
    
private:
    std::unique_ptr<QueryNode> anchor_member_;
    std::unique_ptr<QueryNode> recursive_member_;
};
```

### 4.3 执行引擎设计

#### 4.3.1 递归查询执行器
```cpp
class RecursiveQueryExecutor {
public:
    struct RecursionState {
        std::shared_ptr<TemporaryTable> working_table;
        std::shared_ptr<TemporaryTable> result_table;
        size_t iteration;
        bool has_changes;
    };
    
    RecursiveQueryExecutor(std::shared_ptr<ExecutionContext> context);
    
    // 执行递归查询
    std::shared_ptr<TemporaryTable> execute_recursive_cte(
        const RecursiveCTENode& recursive_cte,
        const std::unordered_map<std::string, CTEContext>& outer_cte_contexts);
    
private:
    // 迭代执行算法
    RecursionState execute_iteration(const RecursiveCTENode& recursive_cte,
                                    const RecursionState& current_state,
                                    const std::unordered_map<std::string, CTEContext>& cte_contexts);
    
    // 终止条件检查
    bool should_terminate(const RecursionState& state);
    
    // 行去重
    void deduplicate_rows(std::shared_ptr<TemporaryTable> table);
    
    std::shared_ptr<ExecutionContext> context_;
    size_t max_recursion_depth_;
    size_t min_rows_per_iteration_;
};
```

#### 4.3.2 执行算法

1. **迭代算法**
   ```
   初始化: 执行anchor member，结果存入working_table和result_table
   循环:
       执行recursive member，参数为working_table
       如果新结果为空: 退出循环
       将新结果加入result_table
       用新结果替换working_table
       检查终止条件(深度限制、行数变化等)
   ```

2. **优化算法**
   - 增量计算：只处理新增的行
   - 并行迭代：多个递归分支并行处理
   - 深度优先/广度优先：根据数据特点选择遍历策略

### 4.4 终止条件和安全控制

#### 4.4.1 终止条件
1. **结果集稳定**：递归部分不再产生新行
2. **深度限制**：达到最大递归深度（默认100）
3. **行数限制**：结果集超过最大行数限制
4. **时间限制**：执行时间超过阈值

#### 4.4.2 安全控制
```cpp
struct RecursionSafetyConfig {
    size_t max_depth = 100;
    size_t max_total_rows = 1000000;
    size_t max_iteration_time_ms = 5000;
    bool enable_cycle_detection = true;
};
```

## 5. 集合操作设计

### 5.1 语法规范

#### 5.1.1 支持的集合操作
1. **UNION**: 合并结果，去除重复行
   ```sql
   SELECT col1 FROM table1
   UNION
   SELECT col2 FROM table2;
   ```

2. **UNION ALL**: 合并结果，保留所有行
   ```sql
   SELECT col1 FROM table1
   UNION ALL
   SELECT col2 FROM table2;
   ```

3. **INTERSECT**: 交集
   ```sql
   SELECT col1 FROM table1
   INTERSECT
   SELECT col2 FROM table2;
   ```

4. **EXCEPT/MINUS**: 差集
   ```sql
   SELECT col1 FROM table1
   EXCEPT
   SELECT col2 FROM table2;
   ```

#### 5.1.2 复合集合操作
```sql
(SELECT a FROM t1)
UNION
(SELECT b FROM t2)
INTERSECT
(SELECT c FROM t3);
```

### 5.2 AST节点设计

#### 5.2.1 集合操作节点
```cpp
class SetOperationNode : public QueryNode {
public:
    enum class OperationType {
        UNION,
        UNION_ALL,
        INTERSECT,
        EXCEPT
    };
    
    SetOperationNode(OperationType operation,
                    std::unique_ptr<QueryNode> left,
                    std::unique_ptr<QueryNode> right);
    
    OperationType operation() const { return operation_; }
    const QueryNode* left() const { return left_.get(); }
    const QueryNode* right() const { return right_.get(); }
    
    void accept(NodeVisitor& visitor) override;
    
private:
    OperationType operation_;
    std::unique_ptr<QueryNode> left_;
    std::unique_ptr<QueryNode> right_;
};
```

### 5.3 执行引擎设计

#### 5.3.1 集合操作执行器
```cpp
class SetOperationExecutor {
public:
    SetOperationExecutor(std::shared_ptr<ExecutionContext> context);
    
    // 执行集合操作
    std::shared_ptr<TemporaryTable> execute_set_operation(
        const SetOperationNode& set_op);
    
private:
    // 各种集合操作的实现
    std::shared_ptr<TemporaryTable> execute_union(
        const QueryNode& left, const QueryNode& right);
    
    std::shared_ptr<TemporaryTable> execute_union_all(
        const QueryNode& left, const QueryNode& right);
    
    std::shared_ptr<TemporaryTable> execute_intersect(
        const QueryNode& left, const QueryNode& right);
    
    std::shared_ptr<TemporaryTable> execute_except(
        const QueryNode& left, const QueryNode& right);
    
    // 辅助方法
    void ensure_compatible_schemas(const QueryNode& left, const QueryNode& right);
    std::vector<Value> extract_row_key(const std::vector<Value>& row);
    
    std::shared_ptr<ExecutionContext> context_;
};
```

#### 5.3.2 执行算法

1. **UNION ALL**
   - 简单合并两个结果集
   - 保持原始顺序（可选）

2. **UNION**
   - 执行UNION ALL
   - 使用哈希表去重

3. **INTERSECT**
   - 构建左结果的哈希表
   - 遍历右结果，检查是否在哈希表中

4. **EXCEPT**
   - 构建右结果的哈希表
   - 遍历左结果，排除在哈希表中的行

### 5.4 优化策略

#### 5.4.1 并行执行
- 左右操作数可并行执行
- 去重操作可并行化

#### 5.4.2 内存优化
- 流式处理：支持大数据集
- 外部排序：内存不足时使用磁盘
- 布隆过滤器：快速排除不可能匹配的行

#### 5.4.3 下推优化
- 将集合操作下推到存储层
- 利用索引加速集合操作

## 6. 集成设计

### 6.1 与统一查询计划集成

#### 6.1.1 查询计划扩展
```cpp
class AdvancedQueryPlan : public UnifiedQueryPlan {
public:
    // 复杂查询特定的计划构建
    bool buildComplexQueryPlan(const WithClauseNode* with_clause,
                              const std::vector<WindowFunctionNode*>& window_funcs,
                              const SetOperationNode* set_operation);
    
    // 复杂查询特定的执行
    ExecutionResult executeComplexQuery();
    
private:
    std::optional<WithClauseNode> with_clause_;
    std::vector<WindowFunctionNode*> window_functions_;
    std::optional<SetOperationNode> set_operation_;
};
```

#### 6.1.2 执行流程集成
1. **解析阶段**: 识别复杂查询语法元素
2. **计划构建**: 创建包含复杂查询元素的查询计划
3. **优化阶段**: 应用复杂查询特定的优化规则
4. **执行阶段**: 按正确顺序执行各个组件

### 6.2 错误处理

#### 6.2.1 语法错误
- 窗口函数语法错误
- CTE循环引用错误
- 递归查询终止条件错误

#### 6.2.2 语义错误
- 窗口函数参数类型不匹配
- CTE列名不匹配
- 递归查询类型不兼容

#### 6.2.3 资源错误
- 递归深度超过限制
- 内存不足无法执行窗口函数
- 临时表空间不足

### 6.3 性能监控

#### 6.3.1 监控指标
- 窗口函数执行时间
- CTE物化开销
- 递归查询迭代次数
- 集合操作内存使用

#### 6.3.2 调优建议
- 基于统计信息的执行策略选择
- 自适应缓冲区大小调整
- 动态并行度调整

## 7. 测试策略

### 7.1 单元测试

#### 7.1.1 窗口函数测试
- 各种窗口函数的基本功能测试
- 分区和排序测试
- 窗口框架测试
- 边界条件测试

#### 7.1.2 CTE测试
- 简单CTE功能测试
- 多CTE依赖测试
- CTE与JOIN组合测试
- CTE结果缓存测试

#### 7.1.3 递归查询测试
- 基本递归功能测试
- 终止条件测试
- 循环检测测试
- 性能边界测试

#### 7.1.4 集合操作测试
- 各种集合操作功能测试
- 空结果集测试
- 重复数据处理测试
- 类型兼容性测试

### 7.2 集成测试

#### 7.2.1 组合功能测试
- 窗口函数与CTE组合
- 递归CTE与集合操作组合
- 复杂查询嵌套测试

#### 7.2.2 性能测试
- 大数据集窗口函数性能
- 深度递归查询性能
- 大规模集合操作性能

#### 7.2.3 并发测试
- 多查询并发执行窗口函数
- 并发CTE物化测试
- 递归查询并发控制

### 7.3 回归测试
- 确保新功能不影响现有查询
- 性能回归测试
- 内存泄漏检测

## 8. 总结

本文档详细设计了SQLCC复杂查询功能的实现方案，包括窗口函数、CTE、递归查询和集合操作。通过模块化设计和优化策略，可以在保持系统稳定性的同时，提供强大的复杂查询能力。

实现建议按以下顺序进行：
1. 先实现集合操作（相对简单）
2. 实现CTE支持（包括递归CTE基础框架）
3. 实现窗口函数
4. 完善递归查询的优化和安全控制
5. 进行性能优化和集成测试
