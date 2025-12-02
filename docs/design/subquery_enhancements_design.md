# SQLCC 子查询增强设计文档

## 1. 概述

本文档详细设计SQLCC数据库系统的子查询增强功能实现方案，包括：
- 相关子查询（Correlated Subqueries）
- EXISTS/NOT EXISTS子查询
- IN/ANY/ALL子查询
- 标量子查询（Scalar Subqueries）

## 2. 当前子查询支持分析

### 2.1 已实现的子查询功能

#### 2.1.1 基本子查询支持
- ✅ 非相关子查询在WHERE子句中
- ✅ 简单的IN子查询
- ✅ 子查询作为表达式

#### 2.1.2 语法支持现状
```sql
-- 当前支持的基本子查询
SELECT * FROM t1 WHERE id IN (SELECT id FROM t2);
SELECT * FROM t1 WHERE age > (SELECT AVG(age) FROM t2);
```

### 2.2 缺失的高级子查询功能

#### 2.2.1 相关子查询
```sql
-- 当前不支持的相关子查询
SELECT * FROM employees e1
WHERE salary > (
    SELECT AVG(salary) 
    FROM employees e2 
    WHERE e2.department = e1.department  -- 引用外部查询
);
```

#### 2.2.2 EXISTS/NOT EXISTS
```sql
-- 当前不支持的EXISTS子查询
SELECT * FROM customers c
WHERE EXISTS (
    SELECT 1 FROM orders o 
    WHERE o.customer_id = c.id AND o.amount > 1000
);
```

#### 2.2.3 ANY/ALL子查询
```sql
-- 当前不支持的ANY/ALL子查询
SELECT * FROM products p
WHERE price > ALL (
    SELECT price FROM competitor_prices 
    WHERE product_type = p.type
);
```

#### 2.2.4 标量子查询位置
```sql
-- 当前不支持的标量子查询位置
SELECT name, 
       (SELECT COUNT(*) FROM orders WHERE customer_id = c.id) as order_count
FROM customers c;
```

## 3. 相关子查询设计

### 3.1 语法规范

#### 3.1.1 相关子查询语法
```sql
-- WHERE子句中的相关子查询
SELECT * FROM t1 
WHERE column1 OPERATOR (
    SELECT agg_func(column2) 
    FROM t2 
    WHERE t2.key = t1.key  -- 相关性条件
);

-- HAVING子句中的相关子查询
SELECT department, AVG(salary) as avg_salary
FROM employees
GROUP BY department
HAVING AVG(salary) > (
    SELECT AVG(salary) 
    FROM employees e2 
    WHERE e2.department = employees.department
);
```

#### 3.1.2 相关性识别
- 子查询引用外部查询的列
- 相关性条件通常出现在WHERE子句中
- 可能多层嵌套相关性

### 3.2 AST节点设计

#### 3.2.1 相关子查询节点
```cpp
class CorrelatedSubqueryNode : public SubqueryNode {
public:
    CorrelatedSubqueryNode(std::unique_ptr<QueryNode> subquery,
                          const std::vector<Correlation>& correlations);
    
    // 获取相关性信息
    const std::vector<Correlation>& getCorrelations() const { return correlations_; }
    
    // 检查是否包含特定外部列引用
    bool referencesExternalColumn(const std::string& table_name, 
                                 const std::string& column_name) const;
    
    // 获取外部列引用列表
    std::vector<ColumnReference> getExternalReferences() const;
    
    void accept(NodeVisitor& visitor) override;
    
private:
    std::vector<Correlation> correlations_;  // 相关性映射
    
    // 相关性结构
    struct Correlation {
        std::string external_table;    // 外部表名
        std::string external_column;   // 外部列名
        std::string internal_table;    // 内部表名（可能重命名）
        std::string internal_column;   // 内部列名
        Value current_value;          // 当前外部列值（执行时）
    };
};
```

#### 3.2.2 相关性分析器
```cpp
class CorrelationAnalyzer {
public:
    // 分析子查询中的相关性
    std::vector<Correlation> analyzeCorrelations(
        const QueryNode& subquery,
        const std::vector<TableReference>& outer_context);
    
    // 检查表达式中的外部引用
    std::vector<ColumnReference> findExternalReferences(
        const ExpressionNode& expr,
        const std::vector<TableReference>& outer_context);
    
    // 重写相关性条件
    std::unique_ptr<ExpressionNode> rewriteCorrelationConditions(
        const ExpressionNode& condition,
        const std::vector<Correlation>& correlations);
    
private:
    // 表引用解析
    struct TableReference {
        std::string alias;
        std::string table_name;
        std::vector<std::string> columns;
    };
    
    // 构建表引用上下文
    std::vector<TableReference> buildTableContext(const QueryNode& query);
};
```

### 3.3 执行引擎设计

#### 3.3.1 相关子查询执行器
```cpp
class CorrelatedSubqueryExecutor {
public:
    struct CorrelationContext {
        std::unordered_map<std::string, Value> current_values;  // 当前外部值
        size_t execution_count;      // 执行次数
        size_t cache_hits;           // 缓存命中次数
        std::shared_ptr<ResultCache> cache;  // 结果缓存
    };
    
    CorrelatedSubqueryExecutor(std::shared_ptr<ExecutionContext> context);
    
    // 执行相关子查询
    Value executeCorrelatedSubquery(
        const CorrelatedSubqueryNode& subquery,
        const CorrelationContext& correlation_context);
    
    // 预编译子查询执行计划
    std::shared_ptr<CompiledSubquery> compileSubquery(
        const CorrelatedSubqueryNode& subquery);
    
private:
    // 执行策略
    enum class ExecutionStrategy {
        NESTED_LOOPS,      // 嵌套循环（默认）
        CACHE_BASED,       // 基于缓存
        MATERIALIZED,      // 物化执行
        REWRITTEN          // 重写为JOIN
    };
    
    // 选择执行策略
    ExecutionStrategy chooseExecutionStrategy(
        const CorrelatedSubqueryNode& subquery,
        const CorrelationContext& context);
    
    // 嵌套循环执行
    Value executeNestedLoops(const CorrelatedSubqueryNode& subquery,
                            const CorrelationContext& context);
    
    // 缓存执行
    Value executeWithCache(const CorrelatedSubqueryNode& subquery,
                          CorrelationContext& context);
    
    std::shared_ptr<ExecutionContext> context_;
    std::shared_ptr<SubqueryCacheManager> cache_manager_;
};
```

#### 3.3.2 执行算法

**算法1：嵌套循环执行**
```
对于外部查询的每一行：
    1. 提取相关性列的值
    2. 将值绑定到子查询参数
    3. 执行子查询
    4. 返回结果
```

**算法2：缓存优化**
```
维护相关性值到子查询结果的缓存
对于每个外部行：
    1. 检查缓存中是否有对应结果
    2. 如果命中：返回缓存结果
    3. 如果未命中：执行子查询，缓存结果
```

**算法3：批处理执行**
```
收集一批外部行的相关性值
批量执行子查询（可能重写为JOIN）
将结果映射回各外部行
```

### 3.4 优化策略

#### 3.4.1 子查询重写优化

```cpp
class SubqueryRewritingOptimizer {
public:
    // 将相关子查询重写为JOIN
    std::unique_ptr<QueryNode> rewriteAsJoin(
        const CorrelatedSubqueryNode& subquery,
        const std::vector<TableReference>& outer_tables);
    
    // 检查是否可重写为JOIN
    bool isRewritableToJoin(const CorrelatedSubqueryNode& subquery);
    
    // 重写为SEMI JOIN/ANTI JOIN
    std::unique_ptr<QueryNode> rewriteAsSemiJoin(
        const CorrelatedSubqueryNode& subquery,
        bool is_anti = false);
    
private:
    // 重写条件分析
    struct RewriteAnalysis {
        bool can_rewrite;
        JoinType suggested_join_type;
        std::vector<std::string> join_conditions;
        bool needs_aggregation;
    };
    
    RewriteAnalysis analyzeRewritePotential(
        const CorrelatedSubqueryNode& subquery);
};
```

#### 3.4.2 缓存策略优化

```cpp
class SubqueryCacheManager {
public:
    struct CacheEntry {
        std::vector<Value> correlation_values;
        Value result;
        size_t access_count;
        std::chrono::system_clock::time_point last_access;
        size_t memory_size;
    };
    
    // 获取缓存结果
    std::optional<Value> getCachedResult(
        const std::vector<Value>& correlation_values);
    
    // 设置缓存结果
    void setCachedResult(const std::vector<Value>& correlation_values,
                        const Value& result);
    
    // 缓存淘汰策略
    void evictIfNeeded();
    
private:
    // 缓存存储
    std::unordered_map<size_t, CacheEntry> cache_;  // 哈希值->缓存条目
    
    // 哈希函数
    size_t computeHash(const std::vector<Value>& values);
    
    // 缓存配置
    struct CacheConfig {
        size_t max_entries = 1000;
        size_t max_memory_mb = 100;
        std::chrono::minutes entry_ttl{10};
    };
    
    CacheConfig config_;
};
```

## 4. EXISTS/NOT EXISTS子查询设计

### 4.1 语法规范

#### 4.1.1 EXISTS语法
```sql
-- EXISTS子查询
SELECT * FROM t1 
WHERE EXISTS (
    SELECT 1 FROM t2 
    WHERE t2.col = t1.col
);

-- NOT EXISTS子查询  
SELECT * FROM t1
WHERE NOT EXISTS (
    SELECT 1 FROM t2 
    WHERE t2.col = t1.col
);
```

#### 4.1.2 语义定义
- EXISTS：子查询返回至少一行时结果为TRUE
- NOT EXISTS：子查询返回0行时结果为TRUE
- 子查询中的SELECT列表通常为`SELECT 1`或`SELECT *`
- 相关性支持

### 4.2 AST节点设计

#### 4.2.1 EXISTS子查询节点
```cpp
class ExistsSubqueryNode : public SubqueryNode {
public:
    ExistsSubqueryNode(std::unique_ptr<QueryNode> subquery,
                      bool is_not_exists = false);
    
    // 获取子查询
    const QueryNode* getSubquery() const { return subquery_.get(); }
    
    // 是否为NOT EXISTS
    bool isNotExists() const { return is_not_exists_; }
    
    // 检查是否相关
    bool isCorrelated() const;
    
    void accept(NodeVisitor& visitor) override;
    
private:
    std::unique_ptr<QueryNode> subquery_;
    bool is_not_exists_;
    mutable std::optional<bool> correlated_cache_;  // 相关性缓存
};
```

### 4.3 执行引擎设计

#### 4.3.1 EXISTS执行器
```cpp
class ExistsSubqueryExecutor {
public:
    ExistsSubqueryExecutor(std::shared_ptr<ExecutionContext> context);
    
    // 执行EXISTS子查询
    bool executeExistsSubquery(const ExistsSubqueryNode& exists_subquery,
                              const CorrelationContext& correlation_context);
    
    // 优化执行策略
    enum class ExecutionStrategy {
        EARLY_EXIT,      // 早期退出（找到一行即返回）
        COUNT_BASED,     // 基于计数
        JOIN_REWRITE,    // 重写为JOIN
        MATERIALIZED     // 物化执行
    };
    
    ExecutionStrategy chooseExecutionStrategy(
        const ExistsSubqueryNode& exists_subquery,
        const CorrelationContext& context);
    
private:
    // 早期退出执行
    bool executeWithEarlyExit(const ExistsSubqueryNode& exists_subquery,
                             const CorrelationContext& context);
    
    // 基于计数的执行
    bool executeWithCounting(const ExistsSubqueryNode& exists_subquery,
                            const CorrelationContext& context);
    
    // 重写为SEMI/ANTI JOIN
    bool executeAsJoin(const ExistsSubqueryNode& exists_subquery,
                      const CorrelationContext& context);
    
    std::shared_ptr<ExecutionContext> context_;
};
```

#### 4.3.2 优化算法

**算法1：早期退出优化**
```
执行子查询，一旦找到第一行立即返回
对于EXISTS：找到行→返回TRUE
对于NOT EXISTS：找到行→返回FALSE，继续检查
```

**算法2：计数优化**
```
如果子查询有LIMIT 1，使用早期退出
否则使用COUNT(*)并与0比较
```

**算法3：SEMI/ANTI JOIN重写**
```
将EXISTS重写为SEMI JOIN
将NOT EXISTS重写为ANTI JOIN
利用JOIN优化器进行优化
```

### 4.4 EXISTS特定优化

#### 4.4.1 空值处理
```cpp
class ExistsNullHandling {
public:
    // EXISTS的空值语义
    static bool evaluateExistsWithNulls(const std::vector<Value>& subquery_result);
    
    // 三值逻辑处理
    enum class ThreeValuedLogic { TRUE, FALSE, UNKNOWN };
    
    ThreeValuedLogic evaluateThreeValued(const ExistsSubqueryNode& exists_subquery,
                                        const CorrelationContext& context);
    
    // 空值传播规则
    static bool propagateNulls(bool exists_result, 
                              const std::vector<Value>& correlation_values);
};
```

#### 4.4.2 索引优化
```cpp
class ExistsIndexOptimizer {
public:
    // 检查是否可利用索引加速EXISTS
    bool canUseIndexForExists(const ExistsSubqueryNode& exists_subquery,
                             const std::shared_ptr<IndexManager>& index_manager);
    
    // 使用索引执行EXISTS
    bool executeExistsWithIndex(const ExistsSubqueryNode& exists_subquery,
                               const CorrelationContext& context,
                               const std::shared_ptr<IndexManager>& index_manager);
    
private:
    // 索引适用性分析
    struct IndexSuitability {
        bool suitable;
        std::string index_name;
        double estimated_cost;
        size_t estimated_rows;
    };
    
    IndexSuitability analyzeIndexSuitability(
        const ExistsSubqueryNode& exists_subquery,
        const std::shared_ptr<IndexManager>& index_manager);
};
```

## 5. IN/ANY/ALL子查询设计

### 5.1 语法规范

#### 5.1.1 IN子查询语法
```sql
-- IN子查询
SELECT * FROM t1 
WHERE column IN (SELECT column FROM t2);

-- NOT IN子查询
SELECT * FROM t1 
WHERE column NOT IN (SELECT column FROM t2);

-- 多列IN子查询
SELECT * FROM t1 
WHERE (col1, col2) IN (SELECT col1, col2 FROM t2);
```

#### 5.1.2 ANY/SOME子查询语法
```sql
-- ANY子查询
SELECT * FROM t1 
WHERE column > ANY (SELECT column FROM t2);

-- SOME子查询（与ANY同义）
SELECT * FROM t1 
WHERE column > SOME (SELECT column FROM t2);
```

#### 5.1.3 ALL子查询语法
```sql
-- ALL子查询
SELECT * FROM t1 
WHERE column > ALL (SELECT column FROM t2);

-- 与聚合函数比较
SELECT * FROM t1 
WHERE column > ALL (SELECT MAX(column) FROM t2 GROUP BY group_col);
```

### 5.2 AST节点设计

#### 5.2.1 集合比较节点
```cpp
class SetComparisonNode : public ExpressionNode {
public:
    enum class ComparisonType {
        IN, NOT_IN, ANY, ALL
    };
    
    enum class OperatorType {
        EQUAL, NOT_EQUAL,
        LESS, LESS_EQUAL,
        GREATER, GREATER_EQUAL
    };
    
    SetComparisonNode(ComparisonType comparison_type,
                     OperatorType operator_type,
                     std::unique_ptr<ExpressionNode> left_operand,
                     std::unique_ptr<SubqueryNode> right_subquery);
    
    // 获取比较类型
    ComparisonType getComparisonType() const { return comparison_type_; }
    
    // 获取操作符类型
    OperatorType getOperatorType() const { return operator_type_; }
    
    // 获取左操作数
    const ExpressionNode* getLeftOperand() const { return left_operand_.get(); }
    
    // 获取右子查询
    const SubqueryNode* getRightSubquery() const { return right_subquery_.get(); }
    
    // 检查是否相关
    bool isCorrelated() const;
    
    void accept(NodeVisitor& visitor) override;
    
private:
    ComparisonType comparison_type_;
    OperatorType operator_type_;
    std::unique_ptr<ExpressionNode> left_operand_;
    std::unique_ptr<SubqueryNode> right_subquery_;
    mutable std::optional<bool> correlated_cache_;
};
```

### 5.3 执行引擎设计

#### 5.3.1 集合比较执行器
```cpp
class SetComparisonExecutor {
public:
    SetComparisonExecutor(std::shared_ptr<ExecutionContext> context);
    
    // 执行集合比较
    bool executeSetComparison(const SetComparisonNode& set_comparison,
                             const CorrelationContext& correlation_context);
    
    // 执行策略
    enum class ExecutionStrategy {
        HASH_SET,        // 哈希集合
        SORTED_SET,      // 排序集合
        INDEX_LOOKUP,    // 索引查找
        NAIVE_LOOP       // 朴素循环
    };
    
    ExecutionStrategy chooseExecutionStrategy(
        const SetComparisonNode& set_comparison,
        const CorrelationContext& context);
    
private:
    // IN/NOT IN执行
    bool executeInComparison(const SetComparisonNode& set_comparison,
                            const CorrelationContext& context,
                            ExecutionStrategy strategy);
    
    // ANY/ALL执行
    bool executeQuantifiedComparison(const SetComparisonNode& set_comparison,
                                    const CorrelationContext& context,
                                    ExecutionStrategy strategy);
    
    // 哈希集合执行
    bool executeWithHashSet(const SetComparisonNode& set_comparison,
                           const CorrelationContext& context);
    
    // 排序集合执行
    bool executeWithSortedSet(const SetComparisonNode& set_comparison,
                             const CorrelationContext& context);
    
    std::shared_ptr<ExecutionContext> context_;
};
```

#### 5.3.2 IN子查询优化

**算法1：哈希IN优化**
```
1. 执行子查询，构建哈希集合
2. 对于每个外部行，检查值是否在哈希集合中
3. NOT IN需要额外处理空值
```

**算法2：排序IN优化**
```
1. 执行子查询，排序结果
2. 对于每个外部行，使用二分查找
3. 适合数据分布均匀的情况
```

**算法3：索引IN优化**
```
1. 如果子查询列有索引，使用索引查找
2. 对于每个外部行值，在索引中查找
3. 适合选择性高的查询
```

#### 5.3.3 ANY/ALL子查询优化

**ANY子查询优化**
```
ANY等价于：存在至少一个满足条件的值
可优化为：MAX/MIN比较或早期退出
```

**ALL子查询优化**
```
ALL等价于：所有值都满足条件
可优化为：与极值比较或计数验证
```

### 5.4 空值处理

#### 5.4.1 IN空值语义
```cpp
class InNullHandling {
public:
    // IN的空值处理规则
    static bool evaluateInWithNulls(const Value& left_value,
                                   const std::vector<Value>& right_values,
                                   bool is_not_in);
    
    // 三值逻辑结果
    enum class ThreeValuedResult { TRUE, FALSE, UNKNOWN };
    
    ThreeValuedResult evaluateThreeValuedIn(const Value& left_value,
                                           const std::vector<Value>& right_values,
                                           bool is_not_in);
    
    // 空值传播
    static bool propagateNulls(bool in_result,
                              const Value& left_value,
                              const std::vector<Value>& right_values);
};
```

#### 5.4.2 ANY/ALL空值处理
```cpp
class QuantifiedNullHandling {
public:
    // ANY的空值处理
    static bool evaluateAnyWithNulls(const Value& left_value,
                                    const std::vector<Value>& right_values,
                                    const std::string& op);
    
    // ALL的空值处理
    static bool evaluateAllWithNulls(const Value& left_value,
                                    const std::vector<Value>& right_values,
                                    const std::string& op);
    
    // 空值优化：提前检测
    static std::optional<bool> earlyNullDetection(
        const Value& left_value,
        const std::vector<Value>& right_values,
        const std::string& op,
        bool is_all);
};
```

## 6. 标量子查询设计

### 6.1 语法规范

#### 6.1.1 标量子查询位置
```sql
-- SELECT列表中的标量子查询
SELECT name,
       (SELECT COUNT(*) FROM orders WHERE customer_id = c.id) as order_count
FROM customers c;

-- WHERE子句中的标量子查询
SELECT * FROM products
WHERE price > (SELECT AVG(price) FROM products);

-- HAVING子句中的标量子查询
SELECT category, AVG(price) as avg_price
FROM products
GROUP BY category
HAVING AVG(price) > (SELECT AVG(price) FROM products);

-- ORDER BY子句中的标量子查询
SELECT * FROM employees
ORDER BY (SELECT salary FROM salaries WHERE employee_id = employees.id);
```

#### 6.1.2 标量子查询限制
- 必须返回单行单列
- 如果返回多行：运行时错误
- 如果返回多列：语法错误
- 如果返回零行：结果为NULL

### 6.2 AST节点设计

#### 6.2.1 标量子查询节点
```cpp
class ScalarSubqueryNode : public ExpressionNode {
public:
    ScalarSubqueryNode(std::unique_ptr<QueryNode> subquery);
    
    // 获取子查询
    const QueryNode* getSubquery() const { return subquery_.get(); }
    
    // 检查是否相关
    bool isCorrelated() const;
    
    // 验证标量子查询约束
    bool validateScalarConstraints() const;
    
    void accept(NodeVisitor& visitor) override;
    
private:
    std::unique_ptr<QueryNode> subquery_;
    mutable std::optional<bool> correlated_cache_;
    mutable std::optional<bool> valid_cache_;
};
```

### 6.3 执行引擎设计

#### 6.3.1 标量子查询执行器
```cpp
class ScalarSubqueryExecutor {
public:
    ScalarSubqueryExecutor(std::shared_ptr<ExecutionContext> context);
    
    // 执行标量子查询
    Value executeScalarSubquery(const ScalarSubqueryNode& scalar_subquery,
                               const CorrelationContext& correlation_context);
    
    // 执行策略
    enum class ExecutionStrategy {
        CACHED,          // 缓存执行（非相关）
        NESTED,          // 嵌套执行（相关）
        REWRITTEN,       // 重写为JOIN
        MATERIALIZED     // 物化执行
    };
    
    ExecutionStrategy chooseExecutionStrategy(
        const ScalarSubqueryNode& scalar_subquery,
        const CorrelationContext& context);
    
    // 验证标量结果
    bool validateScalarResult(const std::vector<std::vector<Value>>& result);
    
private:
    // 缓存执行
    Value executeCached(const ScalarSubqueryNode& scalar_subquery);
    
    // 嵌套执行
    Value executeNested(const ScalarSubqueryNode& scalar_subquery,
                       const CorrelationContext& context);
    
    // 结果验证
    void validateResultCardinality(const std::vector<std::vector<Value>>& result);
    
    std::shared_ptr<ExecutionContext> context_;
    std::shared_ptr<ScalarSubqueryCache> cache_;
};
```

#### 6.3.2 缓存管理

```cpp
class ScalarSubqueryCache {
public:
    struct CacheEntry {
        std::string subquery_hash;
        Value result;
        std::chrono::system_clock::time_point timestamp;
        size_t execution_count;
        bool is_correlated;
    };
    
    // 获取缓存结果
    std::optional<Value> getCachedResult(const std::string& subquery_hash,
                                        const std::vector<Value>& correlation_values = {});
    
    // 设置缓存结果
    void setCachedResult(const std::string& subquery_hash,
                        const Value& result,
                        const std::vector<Value>& correlation_values = {});
    
    // 缓存键生成
    std::string generateCacheKey(const ScalarSubqueryNode& subquery,
                                const std::vector<Value>& correlation_values = {});
    
private:
    // 缓存存储
    std::unordered_map<std::string, CacheEntry> uncorrelated_cache_;
    std::unordered_map<std::string, std::unordered_map<size_t, CacheEntry>> correlated_cache_;
    
    // 哈希函数
    size_t hashCorrelationValues(const std::vector<Value>& values);
};
```

### 6.4 优化策略

#### 6.4.1 非相关标量子查询优化

**优化1：常量折叠**
```
如果子查询不依赖外部上下文且结果稳定
在查询编译时执行并缓存结果
```

**优化2：结果共享**
```
同一标量子查询在查询中多次出现
共享执行结果，避免重复计算
```

**优化3：延迟执行**
```
如果可能返回NULL，延迟到需要时执行
避免不必要的计算
```

#### 6.4.2 相关标量子查询优化

**优化1：批处理执行**
```
收集一批相关性值
批量执行子查询，减少上下文切换
```

**优化2：结果预测**
```
基于历史结果预测可能的结果
选择性使用缓存
```

**优化3：渐进式执行**
```
先执行快速路径（如索引查找）
失败时回退到完整执行
```

## 7. 查询优化器集成

### 7.1 子查询重写框架

#### 7.1.1 重写规则引擎
```cpp
class SubqueryRewriteEngine {
public:
    // 注册重写规则
    void registerRewriteRule(std::unique_ptr<SubqueryRewriteRule> rule);
    
    // 应用重写规则
    std::unique_ptr<QueryNode> rewriteSubqueries(
        std::unique_ptr<QueryNode> query);
    
    // 规则优先级管理
    void setRulePriority(const std::string& rule_name, int priority);
    
private:
    // 重写规则接口
    class SubqueryRewriteRule {
    public:
        virtual ~SubqueryRewriteRule() = default;
        
        virtual std::string getName() const = 0;
        
        virtual bool isApplicable(const QueryNode& query) const = 0;
        
        virtual std::unique_ptr<QueryNode> apply(
            std::unique_ptr<QueryNode> query) = 0;
        
        virtual int getPriority() const { return 0; }
    };
    
    std::vector<std::unique_ptr<SubqueryRewriteRule>> rules_;
};
```

#### 7.1.2 常见重写规则

**规则1：EXISTS转SEMI JOIN**
```cpp
class ExistsToSemiJoinRule : public SubqueryRewriteRule {
public:
    std::string getName() const override { return "ExistsToSemiJoin"; }
    
    bool isApplicable(const QueryNode& query) const override;
    
    std::unique_ptr<QueryNode> apply(
        std::unique_ptr<QueryNode> query) override;
};
```

**规则2：IN转JOIN**
```cpp
class InToJoinRule : public SubqueryRewriteRule {
public:
    std::string getName() const override { return "InToJoin"; }
    
    bool isApplicable(const QueryNode& query) const override;
    
    std::unique_ptr<QueryNode> apply(
        std::unique_ptr<QueryNode> query) override;
};
```

**规则3：标量子查询提升**
```cpp
class ScalarSubqueryLiftingRule : public SubqueryRewriteRule {
public:
    std::string getName() const override { return "ScalarSubqueryLifting"; }
    
    bool isApplicable(const QueryNode& query) const override;
    
    std::unique_ptr<QueryNode> apply(
        std::unique_ptr<QueryNode> query) override;
};
```

### 7.2 成本估算增强

#### 7.2.1 子查询成本模型
```cpp
class SubqueryCostEstimator {
public:
    struct SubqueryCost {
        double cpu_cost;
        double io_cost;
        double memory_cost;
        double correlation_factor;  // 相关性影响因子
        double cardinality;
    };
    
    // 估算子查询成本
    SubqueryCost estimateSubqueryCost(const SubqueryNode& subquery,
                                     const QueryContext& context);
    
    // 估算相关性影响
    double estimateCorrelationImpact(const SubqueryNode& subquery,
                                    const QueryContext& context);
    
    // 比较执行策略成本
    std::vector<std::pair<std::string, SubqueryCost>> compareStrategies(
        const SubqueryNode& subquery,
        const QueryContext& context);
    
private:
    // 统计信息收集
    struct Statistics {
        size_t outer_cardinality;
        size_t inner_cardinality;
        double selectivity;
        bool has_index;
        size_t avg_row_size;
    };
    
    Statistics collectStatistics(const SubqueryNode& subquery,
                                const QueryContext& context);
};
```

### 7.3 执行计划生成

#### 7.3.1 子查询执行计划
```cpp
class SubqueryExecutionPlan {
public:
    struct PlanNode {
        enum class NodeType {
            SUBQUERY_EXECUTION,
            CORRELATION_BINDING,
            RESULT_CACHING,
            JOIN_REWRITE
        };
        
        NodeType type;
        std::unique_ptr<SubqueryNode> subquery;
        std::vector<PlanNode> children;
        CostEstimate cost;
        std::string strategy;
    };
    
    // 构建执行计划
    std::unique_ptr<PlanNode> buildExecutionPlan(
        const SubqueryNode& subquery,
        const QueryContext& context);
    
    // 优化执行计划
    std::unique_ptr<PlanNode> optimizePlan(
        std::unique_ptr<PlanNode> plan,
        const QueryContext& context);
    
    // 执行计划
    Value executePlan(const PlanNode& plan,
                     const CorrelationContext& correlation_context);
    
private:
    // 计划优化规则
    void applyOptimizationRules(PlanNode& plan,
                               const QueryContext& context);
};
```

## 8. 错误处理和兼容性

### 8.1 错误类型定义

#### 8.1.1 语法错误
```cpp
enum class SubquerySyntaxError {
    INVALID_SUBQUERY_POSITION,      // 无效的子查询位置
    MULTIPLE_COLUMNS,               // 标量子查询返回多列
    MISSING_PARENTHESES,            // 缺少括号
    INVALID_CORRELATION_REFERENCE,  // 无效的相关性引用
    UNSUPPORTED_SUBQUERY_TYPE       // 不支持的子查询类型
};
```

#### 8.1.2 语义错误
```cpp
enum class SubquerySemanticError {
    CARDINALITY_VIOLATION,          // 基数违规（标量子查询返回多行）
    TYPE_MISMATCH,                  // 类型不匹配
    AMBIGUOUS_COLUMN_REFERENCE,     // 列引用歧义
    CIRCULAR_CORRELATION,           // 循环相关性
    INVALID_AGGREGATION_CONTEXT     // 无效的聚合上下文
};
```

#### 8.1.3 运行时错误
```cpp
enum class SubqueryRuntimeError {
    SUBQUERY_TIMEOUT,               // 子查询超时
    MEMORY_EXHAUSTED,               // 内存耗尽
    CACHE_OVERFLOW,                 // 缓存溢出
    RECURSION_DEPTH_EXCEEDED,       // 递归深度超限
    INTERNAL_ERROR                  // 内部错误
};
```

### 8.2 错误恢复策略

#### 8.2.1 优雅降级
```cpp
class SubqueryErrorRecovery {
public:
    // 尝试恢复子查询错误
    std::optional<Value> tryRecover(
        SubqueryRuntimeError error,
        const SubqueryNode& subquery,
        const CorrelationContext& context);
    
    // 降级执行策略
    enum class FallbackStrategy {
        USE_CACHED_RESULT,          // 使用缓存结果
        SIMPLIFY_SUBQUERY,          // 简化子查询
        USE_APPROXIMATION,          // 使用近似结果
        RETURN_DEFAULT_VALUE        // 返回默认值
    };
    
    FallbackStrategy chooseFallbackStrategy(
        SubqueryRuntimeError error,
        const SubqueryNode& subquery);
    
private:
    // 简化子查询
    std::unique_ptr<QueryNode> simplifySubquery(
        const SubqueryNode& subquery);
    
    // 生成近似结果
    Value generateApproximation(const SubqueryNode& subquery,
                               const CorrelationContext& context);
};
```

### 8.3 兼容性配置

#### 8.3.1 子查询配置
```cpp
struct SubqueryCompatibilityConfig {
    // 功能启用配置
    bool enable_correlated_subqueries = true;
    bool enable_exists_subqueries = true;
    bool enable_in_subqueries = true;
    bool enable_any_all_subqueries = true;
    bool enable_scalar_subqueries = true;
    
    // 限制配置
    size_t max_subquery_depth = 10;
    size_t max_correlation_depth = 5;
    size_t max_subquery_result_rows = 1000000;
    size_t max_subquery_execution_time_ms = 5000;
    
    // 优化配置
    bool enable_subquery_rewriting = true;
    bool enable_subquery_caching = true;
    bool enable_subquery_materialization = false;
    
    // 空值处理配置
    bool strict_null_handling = true;
    bool three_valued_logic = true;
};
```

## 9. 测试策略

### 9.1 单元测试

#### 9.1.1 语法解析测试
- 相关子查询语法解析测试
- EXISTS/NOT EXISTS语法测试
- IN/ANY/ALL语法测试
- 标量子查询位置测试

#### 9.1.2 语义验证测试
- 相关性分析测试
- 类型兼容性测试
- 基数约束测试
- 空值语义测试

#### 9.1.3 执行算法测试
- 嵌套循环执行测试
- 哈希集合执行测试
- 缓存管理测试
- 重写规则测试

### 9.2 集成测试

#### 9.2.1 功能集成测试
- 子查询与JOIN组合测试
- 子查询与聚合组合测试
- 多层嵌套子查询测试
- 子查询与事务集成测试

#### 9.2.2 性能测试
- 相关性性能测试
- 大规模IN子查询测试
- EXISTS优化效果测试
- 缓存命中率测试

#### 9.2.3 并发测试
- 多查询子查询并发测试
- 缓存并发访问测试
- 死锁检测测试

### 9.3 回归测试

#### 9.3.1 兼容性回归测试
- 确保新功能不影响现有子查询
- 性能回归测试
- 内存泄漏检测

#### 9.3.2 边界条件测试
- 空结果集测试
- 单行结果测试
- 大数据集测试
- 极端相关性测试

## 10. 实施路线图

### 10.1 第一阶段：基础框架
1. 扩展AST节点支持高级子查询
2. 实现基础解析器扩展
3. 创建子查询执行框架

### 10.2 第二阶段：核心功能
1. 实现相关子查询支持
2. 实现EXISTS/NOT EXISTS
3. 实现IN子查询增强

### 10.3 第三阶段：高级功能
1. 实现ANY/ALL子查询
2. 实现标量子查询增强
3. 实现子查询重写优化

### 10.4 第四阶段：优化和完善
1. 实现性能优化策略
2. 完善错误处理和兼容性
3. 进行全面的测试和验证

## 11. 总结

本文档详细设计了SQLCC子查询增强功能的实现方案。通过模块化设计和分阶段实施，可以逐步添加相关子查询、EXISTS/NOT EXISTS、IN/ANY/ALL和标量子查询等高级功能。

关键设计要点包括：
1. **AST扩展**：为各类子查询设计专门的AST节点
2. **执行策略**：根据子查询特点选择最优执行算法
3. **优化框架**：实现子查询重写和缓存优化
4. **错误处理**：完善的错误处理和恢复机制
5. **兼容性**：确保与SQL标准和现有系统的兼容性

建议按照实施路线图分阶段开发，每个阶段都进行充分的测试和验证，确保功能的正确性和性能的可靠性。
