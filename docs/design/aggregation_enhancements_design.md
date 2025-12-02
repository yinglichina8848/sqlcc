# SQLCC 聚合和分组增强设计文档

## 1. 概述

本文档详细设计SQLCC数据库系统的聚合和分组增强功能实现方案，包括：
- HAVING子句支持
- GROUPING SETS多维分组
- ROLLUP层次分组
- CUBE多维分组
- 窗口聚合函数增强

## 2. 当前聚合功能分析

### 2.1 已实现的聚合功能

#### 2.1.1 基本聚合函数
- ✅ SUM：求和
- ✅ AVG：平均值
- ✅ COUNT：计数
- ✅ MIN：最小值
- ✅ MAX：最大值

#### 2.1.2 基本GROUP BY支持
```sql
-- 当前支持的基本分组
SELECT department, AVG(salary) 
FROM employees 
GROUP BY department;

SELECT YEAR(hiredate), COUNT(*) 
FROM employees 
GROUP BY YEAR(hiredate);
```

#### 2.1.3 限制和缺失功能
1. ❌ HAVING子句：分组后过滤
2. ❌ GROUPING SETS：多维分组
3. ❌ ROLLUP：层次分组
4. ❌ CUBE：多维分组
5. ❌ 高级窗口聚合：滑动窗口聚合

## 3. HAVING子句设计

### 3.1 语法规范

#### 3.1.1 HAVING子句语法
```sql
-- 基本HAVING语法
SELECT column, aggregate_function(column)
FROM table
WHERE condition
GROUP BY column
HAVING aggregate_function(column) > value;

-- 复杂HAVING条件
SELECT department, AVG(salary), COUNT(*)
FROM employees
WHERE hiredate > '2020-01-01'
GROUP BY department
HAVING AVG(salary) > 50000 
   AND COUNT(*) > 5
   AND MAX(salary) < 200000;
```

#### 3.1.2 HAVING子句语义
- 在GROUP BY之后执行
- 对分组结果进行过滤
- 可以使用聚合函数，但不能使用普通列（除非在GROUP BY中）
- 可以引用SELECT列表中的别名

### 3.2 AST节点设计

#### 3.2.1 HAVING子句节点
```cpp
class HavingClauseNode : public ASTNode {
public:
    HavingClauseNode(std::unique_ptr<ExpressionNode> condition);
    
    // 获取HAVING条件
    const ExpressionNode* getCondition() const { return condition_.get(); }
    
    // 验证HAVING条件有效性
    bool validate(const std::vector<std::string>& group_by_columns,
                 const std::vector<std::string>& select_columns) const;
    
    // 提取使用的聚合函数
    std::vector<std::string> extractAggregateFunctions() const;
    
    void accept(NodeVisitor& visitor) override;
    
private:
    std::unique_ptr<ExpressionNode> condition_;
    
    // 验证辅助方法
    bool isValidColumnReference(const std::string& column_name,
                               const std::vector<std::string>& group_by_columns,
                               const std::vector<std::string>& select_columns) const;
};
```

#### 3.2.2 扩展的SELECT语句节点
```cpp
class EnhancedSelectStatementNode : public SelectStatementNode {
public:
    EnhancedSelectStatementNode();
    
    // 设置HAVING子句
    void setHavingClause(std::unique_ptr<HavingClauseNode> having_clause);
    
    // 获取HAVING子句
    const HavingClauseNode* getHavingClause() const { return having_clause_.get(); }
    
    // 检查是否有HAVING子句
    bool hasHavingClause() const { return having_clause_ != nullptr; }
    
    // 设置分组增强（GROUPING SETS/ROLLUP/CUBE）
    void setGroupByEnhancement(std::unique_ptr<GroupByEnhancementNode> enhancement);
    
    const GroupByEnhancementNode* getGroupByEnhancement() const { 
        return group_by_enhancement_.get(); 
    }
    
    void accept(NodeVisitor& visitor) override;
    
private:
    std::unique_ptr<HavingClauseNode> having_clause_;
    std::unique_ptr<GroupByEnhancementNode> group_by_enhancement_;
};
```

### 3.3 执行引擎设计

#### 3.3.1 HAVING子句执行器
```cpp
class HavingClauseExecutor {
public:
    HavingClauseExecutor(std::shared_ptr<ExecutionContext> context);
    
    // 应用HAVING过滤
    std::vector<std::vector<Value>> applyHavingFilter(
        const std::vector<std::vector<Value>>& grouped_rows,
        const HavingClauseNode& having_clause,
        const std::vector<std::string>& group_by_columns,
        const std::vector<AggregateInfo>& aggregate_infos);
    
    // 编译HAVING条件
    std::function<bool(const std::vector<Value>&)> compileHavingCondition(
        const HavingClauseNode& having_clause,
        const std::vector<std::string>& group_by_columns,
        const std::vector<AggregateInfo>& aggregate_infos);
    
private:
    // HAVING条件评估上下文
    struct HavingEvaluationContext {
        const std::vector<Value>& group_key;
        const std::vector<Value>& aggregate_values;
        const std::vector<std::string>& group_by_columns;
        const std::vector<AggregateInfo>& aggregate_infos;
    };
    
    // 创建评估函数
    std::function<bool(const HavingEvaluationContext&)> createEvaluator(
        const ExpressionNode& condition);
    
    std::shared_ptr<ExecutionContext> context_;
};
```

#### 3.3.2 执行流程
```
1. 执行WHERE条件过滤
2. 执行GROUP BY分组
3. 计算聚合函数值
4. 应用HAVING条件过滤
5. 返回最终结果
```

### 3.4 优化策略

#### 3.4.1 HAVING条件下推
```cpp
class HavingPushdownOptimizer {
public:
    // 将HAVING条件下推到WHERE（如果可能）
    std::unique_ptr<ExpressionNode> pushDownToWhere(
        const HavingClauseNode& having_clause,
        const std::vector<std::string>& group_by_columns);
    
    // 分析条件可下推性
    struct PushdownAnalysis {
        bool can_push_partially;
        bool can_push_fully;
        std::unique_ptr<ExpressionNode> where_condition;
        std::unique_ptr<ExpressionNode> remaining_having_condition;
    };
    
    PushdownAnalysis analyzePushdownPotential(
        const HavingClauseNode& having_clause,
        const std::vector<std::string>& group_by_columns);
    
private:
    // 检查条件是否只依赖分组列
    bool dependsOnlyOnGroupColumns(const ExpressionNode& condition,
                                  const std::vector<std::string>& group_by_columns);
};
```

#### 3.4.2 聚合结果缓存
```cpp
class HavingResultCache {
public:
    struct CacheKey {
        std::string query_hash;
        std::vector<Value> group_key;
        size_t hash() const;
    };
    
    // 缓存HAVING评估结果
    void cacheResult(const CacheKey& key, bool result);
    
    // 获取缓存结果
    std::optional<bool> getCachedResult(const CacheKey& key);
    
    // 清空缓存
    void clear();
    
private:
    std::unordered_map<size_t, bool> cache_;
    std::mutex cache_mutex_;
};
```

## 4. GROUPING SETS设计

### 4.1 语法规范

#### 4.1.1 GROUPING SETS语法
```sql
-- 基本GROUPING SETS
SELECT year, quarter, product, SUM(sales)
FROM sales_data
GROUP BY GROUPING SETS (
    (year, quarter, product),
    (year, quarter),
    (year),
    ()
);

-- 混合分组
SELECT year, quarter, product, region, SUM(sales)
FROM sales_data
GROUP BY GROUPING SETS (
    (year, quarter, product, region),
    (year, quarter),
    (product, region),
    ()
);
```

#### 4.1.2 GROUPING函数
```sql
-- 使用GROUPING函数识别分组级别
SELECT year, quarter, product, 
       SUM(sales),
       GROUPING(year) as g_year,
       GROUPING(quarter) as g_quarter,
       GROUPING(product) as g_product
FROM sales_data
GROUP BY GROUPING SETS (
    (year, quarter, product),
    (year, quarter),
    (year),
    ()
);
```

### 4.2 AST节点设计

#### 4.2.1 GROUPING SETS节点
```cpp
class GroupingSetsNode : public GroupByEnhancementNode {
public:
    GroupingSetsNode();
    
    // 添加分组集
    void addGroupingSet(std::vector<std::string> grouping_set);
    
    // 获取所有分组集
    const std::vector<std::vector<std::string>>& getGroupingSets() const { 
        return grouping_sets_; 
    }
    
    // 生成所有分组组合
    std::vector<std::vector<std::string>> generateAllGroupingCombinations() const;
    
    // 计算需要的GROUPING函数
    std::vector<std::string> getGroupingFunctionColumns() const;
    
    void accept(NodeVisitor& visitor) override;
    
private:
    std::vector<std::vector<std::string>> grouping_sets_;
};
```

#### 4.2.2 GROUPING函数节点
```cpp
class GroupingFunctionNode : public ExpressionNode {
public:
    GroupingFunctionNode(const std::vector<std::string>& columns);
    
    // 获取GROUPING函数作用的列
    const std::vector<std::string>& getColumns() const { return columns_; }
    
    // 计算GROUPING值
    Value computeGroupingValue(const std::vector<bool>& column_in_group) const;
    
    // 生成GROUPING掩码
    static uint64_t generateGroupingMask(const std::vector<bool>& column_in_group);
    
    void accept(NodeVisitor& visitor) override;
    
private:
    std::vector<std::string> columns_;
};
```

### 4.3 执行引擎设计

#### 4.3.1 GROUPING SETS执行器
```cpp
class GroupingSetsExecutor {
public:
    struct GroupingSetContext {
        std::vector<std::string> grouping_columns;
        std::vector<size_t> column_indices;
        std::vector<bool> grouping_mask;  // 用于GROUPING函数
    };
    
    GroupingSetsExecutor(std::shared_ptr<ExecutionContext> context);
    
    // 执行GROUPING SETS
    std::vector<std::vector<Value>> executeGroupingSets(
        const std::vector<std::vector<Value>>& input_rows,
        const GroupingSetsNode& grouping_sets,
        const std::vector<AggregateInfo>& aggregate_infos);
    
    // 执行单个分组集
    std::vector<std::vector<Value>> executeSingleGroupingSet(
        const std::vector<std::vector<Value>>& input_rows,
        const GroupingSetContext& context,
        const std::vector<AggregateInfo>& aggregate_infos);
    
    // 合并分组结果
    std::vector<std::vector<Value>> mergeGroupingResults(
        const std::vector<std::vector<std::vector<Value>>>& all_results);
    
private:
    // 分组集执行策略
    enum class ExecutionStrategy {
        MULTI_PASS,      // 多次执行（默认）
        SINGLE_PASS,     // 单次执行（使用哈希表）
        HYBRID           // 混合策略
    };
    
    ExecutionStrategy chooseExecutionStrategy(
        const GroupingSetsNode& grouping_sets,
        size_t input_row_count);
    
    std::shared_ptr<ExecutionContext> context_;
};
```

#### 4.3.2 执行算法

**算法1：多次执行算法**
```
对于每个分组集：
    1. 按分组集列进行分组
    2. 计算聚合函数
    3. 添加GROUPING函数值
    4. 添加到结果集
最后合并所有结果
```

**算法2：单次执行算法**
```
1. 构建所有可能的分组键组合
2. 使用多维哈希表同时计算所有分组
3. 一次性生成所有结果
```

**算法3：混合算法**
```
小分组集使用多次执行
大分组集使用单次执行
根据数据特征动态选择
```

### 4.4 优化策略

#### 4.4.1 公共子表达式共享
```cpp
class GroupingSetsOptimizer {
public:
    // 识别可共享的计算
    std::vector<SharedComputation> findSharedComputations(
        const GroupingSetsNode& grouping_sets);
    
    // 应用共享优化
    std::unique_ptr<ExecutionPlan> applySharingOptimization(
        std::unique_ptr<ExecutionPlan> original_plan,
        const std::vector<SharedComputation>& shared_computations);
    
private:
    struct SharedComputation {
        std::vector<std::string> base_columns;
        std::vector<std::vector<std::string>> dependent_sets;
        bool can_be_shared;
        double sharing_benefit;  // 共享收益估计
    };
};
```

#### 4.4.2 内存优化
```cpp
class GroupingSetsMemoryManager {
public:
    // 估算内存需求
    size_t estimateMemoryRequirements(
        const GroupingSetsNode& grouping_sets,
        size_t input_row_count,
        size_t avg_row_size);
    
    // 选择内存策略
    enum class MemoryStrategy {
        IN_MEMORY,      // 全内存
        SPILL_TO_DISK,  // 溢出到磁盘
        STREAMING       // 流式处理
    };
    
    MemoryStrategy chooseMemoryStrategy(
        size_t required_memory,
        size_t available_memory);
    
    // 内存监控和调整
    void monitorAndAdjust(const GroupingSetsExecutor& executor);
    
private:
    struct MemoryMetrics {
        size_t peak_usage;
        size_t current_usage;
        size_t spill_count;
        size_t spill_size;
    };
    
    MemoryMetrics metrics_;
};
```

## 5. ROLLUP设计

### 5.1 语法规范

#### 5.1.1 ROLLUP语法
```sql
-- 基本ROLLUP
SELECT year, quarter, month, SUM(sales)
FROM sales_data
GROUP BY ROLLUP (year, quarter, month);

-- 等价于
SELECT year, quarter, month, SUM(sales)
FROM sales_data
GROUP BY GROUPING SETS (
    (year, quarter, month),
    (year, quarter),
    (year),
    ()
);

-- 多列ROLLUP
SELECT country, region, city, department, SUM(sales)
FROM sales_data
GROUP BY ROLLUP (country, region, city), department;
```

#### 5.1.2 ROLLUP语义
- 生成层次分组：从最详细到最汇总
- 为每个分组级别生成小计
- 最后生成总计

### 5.2 AST节点设计

#### 5.2.1 ROLLUP节点
```cpp
class RollupNode : public GroupByEnhancementNode {
public:
    RollupNode(const std::vector<std::vector<std::string>>& rollup_columns);
    
    // 获取ROLLUP列
    const std::vector<std::vector<std::string>>& getRollupColumns() const { 
        return rollup_columns_; 
    }
    
    // 转换为GROUPING SETS
    std::unique_ptr<GroupingSetsNode> toGroupingSets() const;
    
    // 生成所有分组组合
    std::vector<std::vector<std::string>> generateAllGroupingCombinations() const override;
    
    void accept(NodeVisitor& visitor) override;
    
private:
    std::vector<std::vector<std::string>> rollup_columns_;
    
    // 递归生成分组组合
    void generateCombinationsRecursive(
        size_t level,
        const std::vector<std::string>& current_prefix,
        std::vector<std::vector<std::string>>& result) const;
};
```

### 5.3 执行引擎设计

#### 5.3.1 ROLLUP执行器
```cpp
class RollupExecutor {
public:
    RollupExecutor(std::shared_ptr<ExecutionContext> context);
    
    // 执行ROLLUP
    std::vector<std::vector<Value>> executeRollup(
        const std::vector<std::vector<Value>>& input_rows,
        const RollupNode& rollup,
        const std::vector<AggregateInfo>& aggregate_infos);
    
    // 层次聚合算法
    std::vector<std::vector<Value>> executeHierarchicalAggregation(
        const std::vector<std::vector<Value>>& input_rows,
        const RollupNode& rollup,
        const std::vector<AggregateInfo>& aggregate_infos);
    
    // 增量计算优化
    std::vector<std::vector<Value>> executeIncrementalRollup(
        const std::vector<std::vector<Value>>& input_rows,
        const RollupNode& rollup,
        const std::vector<AggregateInfo>& aggregate_infos);
    
private:
    // 层次聚合节点
    struct HierarchyNode {
        std::vector<Value> key;
        std::vector<Value> aggregates;
        std::vector<std::shared_ptr<HierarchyNode>> children;
        std::shared_ptr<HierarchyNode> parent;
        
        void addChild(std::shared_ptr<HierarchyNode> child);
        void propagateAggregatesUpwards();
    };
    
    // 构建层次树
    std::shared_ptr<HierarchyNode> buildHierarchyTree(
        const std::vector<std::vector<Value>>& grouped_data,
        const RollupNode& rollup);
    
    std::shared_ptr<ExecutionContext> context_;
};
```

#### 5.3.2 优化算法

**算法1：层次聚合树**
```
1. 构建层次树结构
2. 从叶子节点开始计算聚合
3. 向上传播聚合结果
4. 遍历树收集所有级别结果
```

**算法2：排序合并**
```
1. 按ROLLUP列排序数据
2. 在一次扫描中计算所有层次
3. 维护各级别的运行聚合
```

**算法3：哈希ROLLUP**
```
1. 为每个层次维护哈希表
2. 同时更新所有层次的聚合
3. 适合大数据集
```

## 6. CUBE设计

### 6.1 语法规范

#### 6.1.1 CUBE语法
```sql
-- 基本CUBE
SELECT year, quarter, product, SUM(sales)
FROM sales_data
GROUP BY CUBE (year, quarter, product);

-- 等价于
SELECT year, quarter, product, SUM(sales)
FROM sales_data
GROUP BY GROUPING SETS (
    (year, quarter, product),
    (year, quarter),
    (year, product),
    (quarter, product),
    (year),
    (quarter),
    (product),
    ()
);

-- 部分CUBE
SELECT year, quarter, month, product, SUM(sales)
FROM sales_data
GROUP BY CUBE (year, quarter, month), product;
```

#### 6.1.2 CUBE语义
- 生成所有可能的分组组合
- 包括所有维度的交叉分组
- 生成完整的OLAP立方体

### 6.2 AST节点设计

#### 6.2.1 CUBE节点
```cpp
class CubeNode : public GroupByEnhancementNode {
public:
    CubeNode(const std::vector<std::vector<std::string>>& cube_columns);
    
    // 获取CUBE列
    const std::vector<std::vector<std::string>>& getCubeColumns() const { 
        return cube_columns_; 
    }
    
    // 转换为GROUPING SETS
    std::unique_ptr<GroupingSetsNode> toGroupingSets() const;
    
    // 生成所有分组组合（幂集）
    std::vector<std::vector<std::string>> generateAllGroupingCombinations() const override;
    
    void accept(NodeVisitor& visitor) override;
    
private:
    std::vector<std::vector<std::string>> cube_columns_;
    
    // 生成幂集
    std::vector<std::vector<std::string>> generatePowerSet(
        const std::vector<std::string>& columns) const;
};
```

### 6.3 执行引擎设计

#### 6.3.1 CUBE执行器
```cpp
class CubeExecutor {
public:
    CubeExecutor(std::shared_ptr<ExecutionContext> context);
    
    // 执行CUBE
    std::vector<std::vector<Value>> executeCube(
        const std::vector<std::vector<Value>>& input_rows,
        const CubeNode& cube,
        const std::vector<AggregateInfo>& aggregate_infos);
    
    // 位图CUBE算法
    std::vector<std::vector<Value>> executeBitmapCube(
        const std::vector<std::vector<Value>>& input_rows,
        const CubeNode& cube,
        const std::vector<AggregateInfo>& aggregate_infos);
    
    // 并行CUBE计算
    std::vector<std::vector<Value>> executeParallelCube(
        const std::vector<std::vector<Value>>& input_rows,
        const CubeNode& cube,
        const std::vector<AggregateInfo>& aggregate_infos,
        size_t parallelism);
    
private:
    // 位图表示分组组合
    struct BitmapGroupKey {
        uint64_t bitmap;
        std::vector<Value> values;
        
        bool operator==(const BitmapGroupKey& other) const;
        size_t hash() const;
    };
    
    // 位图哈希表
    class BitmapHashMap {
    public:
        void insert(const BitmapGroupKey& key, const std::vector<Value>& aggregates);
        std::optional<std::vector<Value>> find(const BitmapGroupKey& key) const;
        void clear();
        
    private:
        std::unordered_map<size_t, std::vector<Value>> map_;
    };
    
    std::shared_ptr<ExecutionContext> context_;
};
```

#### 6.3.2 优化算法

**算法1：位图CUBE算法**
```
1. 为每个维度值分配位图位置
2. 使用位图表示分组组合
3. 在位图上进行聚合计算
4. 利用位运算加速
```

**算法2：并行CUBE**
```
1. 将数据分区
2. 每个分区计算局部CUBE
3. 合并局部结果
4. 利用多核并行
```

**算法3：增量CUBE**
```
1. 维护CUBE结果缓存
2. 增量更新受影响的分组
3. 适合频繁更新的场景
```

## 7. 窗口聚合增强

### 7.1 语法规范

#### 7.1.1 窗口聚合语法
```sql
-- 滑动窗口聚合
SELECT time, value,
       AVG(value) OVER (
           ORDER BY time
           ROWS BETWEEN 2 PRECEDING AND 2 FOLLOWING
       ) as moving_avg,
       SUM(value) OVER (
           ORDER BY time
           RANGE BETWEEN INTERVAL '1' HOUR PRECEDING AND CURRENT ROW
       ) as hourly_sum
FROM sensor_data;

-- 分区窗口聚合
SELECT department, employee, salary,
       AVG(salary) OVER (PARTITION BY department) as dept_avg,
       RANK() OVER (PARTITION BY department ORDER BY salary DESC) as dept_rank
FROM employees;
```

#### 7.1.2 窗口框架类型
1. **ROWS**：基于行数的窗口
2. **RANGE**：基于值的范围的窗口
3. **GROUPS**：基于分组数的窗口

### 7.2 AST节点设计

#### 7.2.1 窗口聚合节点
```cpp
class WindowAggregateNode : public ExpressionNode {
public:
    struct WindowFrame {
        enum class FrameType { ROWS, RANGE, GROUPS };
        enum class BoundType {
            UNBOUNDED_PRECEDING,
            VALUE_PRECEDING,
            CURRENT_ROW,
            VALUE_FOLLOWING,
            UNBOUNDED_FOLLOWING
        };
        
        FrameType type;
        BoundType start_bound;
        BoundType end_bound;
        std::optional<Value> start_value;
        std::optional<Value> end_value;
    };
    
    WindowAggregateNode(const std::string& function_name,
                       std::vector<std::unique_ptr<ExpressionNode>> arguments,
                       std::vector<std::unique_ptr<ExpressionNode>> partition_by,
                       std::vector<std::unique_ptr<ExpressionNode>> order_by,
                       std::optional<WindowFrame> frame);
    
    // 获取窗口规范
    const std::vector<std::unique_ptr<ExpressionNode>>& getPartitionBy() const { 
        return partition_by_; 
    }
    
    const std::vector<std::unique_ptr<ExpressionNode>>& getOrderBy() const { 
        return order_by_; 
    }
    
    const std::optional<WindowFrame>& getFrame() const { return frame_; }
    
    void accept(NodeVisitor& visitor) override;
    
private:
    std::string function_name_;
    std::vector<std::unique_ptr<ExpressionNode>> arguments_;
    std::vector<std::unique_ptr<ExpressionNode>> partition_by_;
    std::vector<std::unique_ptr<ExpressionNode>> order_by_;
    std::optional<WindowFrame> frame_;
};
```

### 7.3 执行引擎设计

#### 7.3.1 窗口聚合执行器
```cpp
class WindowAggregateExecutor {
public:
    struct WindowState {
        std::vector<std::vector<Value>> current_partition;
        size_t current_row;
        std::deque<size_t> window_rows;  // 滑动窗口中的行索引
        
        // 维护聚合状态
        struct AggregateState {
            Value sum;
            Value min;
            Value max;
            size_t count;
            std::vector<Value> values;  // 用于中位数等
        };
        
        AggregateState aggregate_state;
    };
    
    WindowAggregateExecutor(std::shared_ptr<ExecutionContext> context);
    
    // 执行窗口聚合
    std::vector<Value> executeWindowAggregate(
        const WindowAggregateNode& window_aggregate,
        const std::vector<std::vector<Value>>& input_rows);
    
    // 流式窗口聚合
    std::vector<Value> executeStreamingWindowAggregate(
        const WindowAggregateNode& window_aggregate,
        const std::vector<std::vector<Value>>& input_rows);
    
private:
    // 分区数据
    std::vector<std::vector<std::vector<Value>>> partitionRows(
        const std::vector<std::vector<Value>>& input_rows,
        const std::vector<std::unique_ptr<ExpressionNode>>& partition_by);
    
    // 排序分区
    void sortPartition(std::vector<std::vector<Value>>& partition,
                      const std::vector<std::unique_ptr<ExpressionNode>>& order_by);
    
    // 计算窗口聚合
    Value computeWindowAggregate(const WindowAggregateNode& window_aggregate,
                                const WindowState& state,
                                const std::vector<std::vector<Value>>& partition);
    
    std::shared_ptr<ExecutionContext> context_;
};
```

#### 7.3.2 优化算法

**算法1：增量窗口聚合**
```
维护滑动窗口的聚合状态
增加新行时更新状态
移除旧行时反向更新
避免重新计算整个窗口
```

**算法2：并行窗口聚合**
```
按分区并行处理
每个分区独立计算窗口聚合
合并结果
```

**算法3：分段窗口聚合**
```
将大窗口分解为小段
分别计算各段聚合
合并段结果
```

## 8. 查询优化器集成

### 8.1 聚合优化规则

#### 8.1.1 聚合下推优化
```cpp
class AggregatePushdownOptimizer {
public:
    // 将聚合下推到子查询
    std::unique_ptr<QueryNode> pushDownAggregates(
        std::unique_ptr<QueryNode> query);
    
    // 识别可下推的聚合
    struct PushdownOpportunity {
        bool can_push_sum;
        bool can_push_count;
        bool can_push_avg;
        bool can_push_min_max;
        std::vector<std::string> group_by_columns;
    };
    
    PushdownOpportunity analyzePushdownOpportunity(
        const QueryNode& query);
    
private:
    // 检查聚合是否可下推
    bool isAggregatePushable(const std::string& aggregate_func,
                            const ExpressionNode& argument);
};
```

#### 8.1.2 聚合合并优化
```cpp
class AggregateMergingOptimizer {
public:
    // 合并相同分组的聚合
    std::unique_ptr<QueryNode> mergeAggregates(
        std::unique_ptr<QueryNode> query);
    
    // 识别可合并的聚合
    struct MergeCandidate {
        std::vector<std::string> group_by_columns;
        std::vector<std::string> aggregate_functions;
        std::vector<ExpressionNode*> aggregate_arguments;
        bool can_be_merged;
    };
    
    std::vector<MergeCandidate> findMergeCandidates(
        const QueryNode& query);
};
```

### 8.2 统计信息收集

#### 8.2.1 聚合统计信息
```cpp
class AggregateStatisticsCollector {
public:
    struct AggregateStats {
        size_t input_row_count;
        size_t output_row_count;
        size_t distinct_group_count;
        std::vector<double> aggregate_values;
        std::chrono::milliseconds execution_time;
        size_t memory_used;
    };
    
    // 收集聚合统计信息
    AggregateStats collectAggregateStatistics(
        const QueryNode& query,
        const std::vector<std::vector<Value>>& input_rows,
        const std::vector<std::vector<Value>>& output_rows);
    
    // 更新统计信息
    void updateStatistics(const std::string& query_hash,
                         const AggregateStats& stats);
    
    // 预测聚合基数
    size_t estimateAggregateCardinality(
        const QueryNode& query,
        size_t input_cardinality);
    
private:
    // 统计信息存储
    std::unordered_map<std::string, AggregateStats> stats_cache_;
    
    // 基数预测模型
    std::unique_ptr<CardinalityModel> cardinality_model_;
};
```

### 8.3 执行计划生成

#### 8.3.1 聚合执行计划
```cpp
class AggregateExecutionPlan {
public:
    struct PlanNode {
        enum class NodeType {
            HASH_AGGREGATE,
            SORT_AGGREGATE,
            STREAMING_AGGREGATE,
            WINDOW_AGGREGATE,
            GROUPING_SETS,
            ROLLUP,
            CUBE
        };
        
        NodeType type;
        std::vector<std::string> group_by_columns;
        std::vector<AggregateInfo> aggregates;
        std::vector<PlanNode> children;
        CostEstimate cost;
        size_t estimated_output_rows;
    };
    
    // 构建聚合执行计划
    std::unique_ptr<PlanNode> buildAggregatePlan(
        const QueryNode& query,
        const QueryContext& context);
    
    // 选择聚合算法
    enum class AggregateAlgorithm {
        HASH_AGGREGATE,    // 哈希聚合
        SORT_AGGREGATE,    // 排序聚合
        HYBRID_AGGREGATE   // 混合聚合
    };
    
    AggregateAlgorithm chooseAggregateAlgorithm(
        const PlanNode& plan,
        const QueryContext& context);
    
private:
    // 成本估算
    CostEstimate estimateCost(const PlanNode& plan,
                             const QueryContext& context);
    
    // 算法选择器
    std::unique_ptr<AlgorithmSelector> algorithm_selector_;
};
```

## 9. 错误处理和兼容性

### 9.1 错误类型定义

#### 9.1.1 语法错误
```cpp
enum class AggregateSyntaxError {
    INVALID_HAVING_POSITION,       // HAVING位置错误
    INVALID_GROUPING_SETS,         // GROUPING SETS语法错误
    INVALID_ROLLUP_SYNTAX,         // ROLLUP语法错误
    INVALID_CUBE_SYNTAX,           // CUBE语法错误
    INVALID_WINDOW_FRAME           // 窗口框架语法错误
};
```

#### 9.1.2 语义错误
```cpp
enum class AggregateSemanticError {
    HAVING_WITHOUT_GROUP_BY,       // 没有GROUP BY的HAVING
    INVALID_HAVING_EXPRESSION,     // 无效的HAVING表达式
    GROUPING_SETS_OVERLAP,         // GROUPING SETS重叠
    WINDOW_FRAME_ERROR,            // 窗口框架错误
    AGGREGATE_NESTING              // 聚合嵌套错误
};
```

### 9.2 兼容性配置

#### 9.2.1 聚合兼容性配置
```cpp
struct AggregateCompatibilityConfig {
    // 功能启用配置
    bool enable_having_clause = true;
    bool enable_grouping_sets = true;
    bool enable_rollup = true;
    bool enable_cube = true;
    bool enable_window_aggregates = true;
    
    // 限制配置
    size_t max_grouping_sets = 100;
    size_t max_rollup_levels = 10;
    size_t max_cube_dimensions = 8;
    size_t max_window_size = 1000000;
    
    // 行为配置
    bool strict_group_by = true;           // 严格的GROUP BY语义
    bool allow_rollup_with_grouping_sets = false;
    bool allow_cube_with_rollup = false;
    
    // 窗口配置
    bool enable_sliding_windows = true;
    bool enable_range_windows = true;
    bool enable_groups_windows = false;
};
```

## 10. 测试策略

### 10.1 单元测试

#### 10.1.1 语法解析测试
- HAVING子句语法测试
- GROUPING SETS语法测试
- ROLLUP语法测试
- CUBE语法测试
- 窗口聚合语法测试

#### 10.1.2 语义验证测试
- HAVING条件验证测试
- GROUPING函数测试
- 窗口框架验证测试
- 聚合嵌套规则测试

#### 10.1.3 算法正确性测试
- HAVING过滤正确性测试
- GROUPING SETS结果正确性测试
- ROLLUP层次正确性测试
- CUBE组合正确性测试
- 窗口聚合计算正确性测试

### 10.2 集成测试

#### 10.2.1 功能集成测试
- HAVING与WHERE组合测试
- GROUPING SETS与JOIN组合测试
- ROLLUP与窗口函数组合测试
- CUBE与子查询组合测试

#### 10.2.2 性能测试
- 大规模分组性能测试
- 多层ROLLUP性能测试
- 高维CUBE性能测试
- 滑动窗口性能测试

#### 10.2.3 并发测试
- 并发分组操作测试
- 窗口聚合并发测试
- 死锁检测测试

### 10.3 回归测试

#### 10.3.1 兼容性回归测试
- 确保新功能不影响现有聚合
- 性能回归测试
- 内存泄漏检测

#### 10.3.2 边界条件测试
- 空分组测试
- 单行分组测试
- 大数据集分组测试
- 极端窗口大小测试

## 11. 实施路线图

### 11.1 第一阶段：基础功能
1. 实现HAVING子句支持
2. 扩展AST节点和解析器
3. 创建基础测试框架

### 11.2 第二阶段：分组增强
1. 实现GROUPING SETS支持
2. 实现GROUPING函数
3. 实现ROLLUP支持

### 11.3 第三阶段：高级功能
1. 实现CUBE支持
2. 实现窗口聚合增强
3. 实现优化策略

### 11.4 第四阶段：优化和完善
1. 实现性能优化
2. 完善错误处理和兼容性
3. 进行全面的测试和验证

## 12. 总结

本文档详细设计了SQLCC聚合和分组增强功能的实现方案。通过模块化设计和分阶段实施，可以逐步添加HAVING子句、GROUPING SETS、ROLLUP、CUBE和窗口聚合等高级功能。

关键设计要点包括：
1. **AST扩展**：为各类分组增强设计专门的AST节点
2. **执行算法**：针对不同场景优化的执行算法
3. **优化策略**：聚合下推、合并、共享等优化
4. **窗口聚合**：增量计算和并行处理优化
5. **兼容性**：确保与SQL标准和现有系统的兼容性

建议按照实施路线图分阶段开发，每个阶段都进行充分的测试和验证，确保功能的正确性和性能的可靠性。
