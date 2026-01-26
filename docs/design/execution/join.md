# Join 设计文档

## 1. 概述

Join 是 SQLCC 数据库系统中的连接操作类，代表两个或多个表之间的连接操作。它是查询执行计划中的重要操作，用于将多个表中的数据关联起来。

## 2. 核心功能

### 2.1 主要功能

- **连接条件管理**：管理连接操作的连接条件
- **连接类型支持**：支持各种连接类型，如内连接、外连接、交叉连接等
- **连接表管理**：管理参与连接的表
- **统计信息收集**：收集连接操作的统计信息
- **性能优化**：支持各种连接优化技术

### 2.2 设计优势

- **模块化**：将连接操作封装为独立的类，便于维护和扩展
- **多连接类型支持**：支持各种连接类型，满足不同的查询需求
- **优化支持**：支持各种连接优化技术，如哈希连接、排序合并连接等
- **统计信息集成**：集成统计信息，便于代价估算和查询优化
- **可扩展**：可以轻松添加新的连接类型和优化技术

## 3. 类定义

```cpp
class Join {
public:
    enum JoinType {
        INNER,
        LEFT_OUTER,
        RIGHT_OUTER,
        FULL_OUTER,
        CROSS,
        NATURAL,
        SEMI,
        ANTI
    };

    Join(std::shared_ptr<ExecutionPlan> left_plan,
         std::shared_ptr<ExecutionPlan> right_plan,
         std::shared_ptr<Expression> condition,
         JoinType type = INNER);
    virtual ~Join() = default;

    // 获取左表执行计划
    std::shared_ptr<ExecutionPlan> get_left_plan() const;

    // 获取右表执行计划
    std::shared_ptr<ExecutionPlan> get_right_plan() const;

    // 获取连接条件
    std::shared_ptr<Expression> get_condition() const;

    // 设置连接条件
    void set_condition(std::shared_ptr<Expression> condition);

    // 获取连接类型
    JoinType get_join_type() const;

    // 设置连接类型
    void set_join_type(JoinType type);

    // 获取左表统计信息
    const TableStatistics* get_left_statistics() const;

    // 获取右表统计信息
    const TableStatistics* get_right_statistics() const;

    // 设置左表统计信息
    void set_left_statistics(const TableStatistics* stats);

    // 设置右表统计信息
    void set_right_statistics(const TableStatistics* stats);

    // 是否是内连接
    bool is_inner_join() const;

    // 是否是外连接
    bool is_outer_join() const;

    // 是否是左外连接
    bool is_left_outer_join() const;

    // 是否是右外连接
    bool is_right_outer_join() const;

    // 是否是全外连接
    bool is_full_outer_join() const;

    // 是否是交叉连接
    bool is_cross_join() const;

    // 是否是自然连接
    bool is_natural_join() const;

    // 是否是半连接
    bool is_semi_join() const;

    // 是否是反连接
    bool is_anti_join() const;

    // 是否有连接条件
    bool has_condition() const;

protected:
    std::shared_ptr<ExecutionPlan> left_plan_;     // 左表执行计划
    std::shared_ptr<ExecutionPlan> right_plan_;    // 右表执行计划
    std::shared_ptr<Expression> condition_;        // 连接条件
    JoinType join_type_;                           // 连接类型
    const TableStatistics* left_stats_;            // 左表统计信息
    const TableStatistics* right_stats_;           // 右表统计信息
};
```

## 4. 核心组件

### 4.1 构造函数

```cpp
Join(std::shared_ptr<ExecutionPlan> left_plan,
     std::shared_ptr<ExecutionPlan> right_plan,
     std::shared_ptr<Expression> condition,
     JoinType type = INNER);
```

- **功能**：初始化连接操作
- **参数**：
  - `left_plan` - 左表执行计划
  - `right_plan` - 右表执行计划
  - `condition` - 连接条件
  - `type` - 连接类型（默认为内连接）

### 4.2 执行计划访问

#### 4.2.1 获取左表执行计划

```cpp
std::shared_ptr<ExecutionPlan> get_left_plan() const;
```

- **功能**：获取连接操作的左表执行计划
- **返回值**：左表执行计划的智能指针

#### 4.2.2 获取右表执行计划

```cpp
std::shared_ptr<ExecutionPlan> get_right_plan() const;
```

- **功能**：获取连接操作的右表执行计划
- **返回值**：右表执行计划的智能指针

### 4.3 连接条件

#### 4.3.1 获取连接条件

```cpp
std::shared_ptr<Expression> get_condition() const;
```

- **功能**：获取连接操作的连接条件
- **返回值**：连接条件的智能指针

#### 4.3.2 设置连接条件

```cpp
void set_condition(std::shared_ptr<Expression> condition);
```

- **功能**：设置连接操作的连接条件
- **参数**：`condition` - 连接条件的智能指针

### 4.4 连接类型

#### 4.4.1 获取连接类型

```cpp
JoinType get_join_type() const;
```

- **功能**：获取连接操作的连接类型
- **返回值**：连接类型枚举值

#### 4.4.2 设置连接类型

```cpp
void set_join_type(JoinType type);
```

- **功能**：设置连接操作的连接类型
- **参数**：`type` - 连接类型枚举值

### 4.5 统计信息

#### 4.5.1 获取左表统计信息

```cpp
const TableStatistics* get_left_statistics() const;
```

- **功能**：获取连接操作的左表统计信息
- **返回值**：左表统计信息的指针

#### 4.5.2 获取右表统计信息

```cpp
const TableStatistics* get_right_statistics() const;
```

- **功能**：获取连接操作的右表统计信息
- **返回值**：右表统计信息的指针

#### 4.5.3 设置左表统计信息

```cpp
void set_left_statistics(const TableStatistics* stats);
```

- **功能**：设置连接操作的左表统计信息
- **参数**：`stats` - 左表统计信息的指针

#### 4.5.4 设置右表统计信息

```cpp
void set_right_statistics(const TableStatistics* stats);
```

- **功能**：设置连接操作的右表统计信息
- **参数**：`stats` - 右表统计信息的指针

### 4.6 连接类型检查

#### 4.6.1 是否是内连接

```cpp
bool is_inner_join() const;
```

- **功能**：检查是否是内连接
- **返回值**：如果是内连接则返回true，否则返回false

#### 4.6.2 是否是外连接

```cpp
bool is_outer_join() const;
```

- **功能**：检查是否是外连接
- **返回值**：如果是外连接则返回true，否则返回false

#### 4.6.3 是否是左外连接

```cpp
bool is_left_outer_join() const;
```

- **功能**：检查是否是左外连接
- **返回值**：如果是左外连接则返回true，否则返回false

#### 4.6.4 是否是右外连接

```cpp
bool is_right_outer_join() const;
```

- **功能**：检查是否是右外连接
- **返回值**：如果是右外连接则返回true，否则返回false

#### 4.6.5 是否是全外连接

```cpp
bool is_full_outer_join() const;
```

- **功能**：检查是否是全外连接
- **返回值**：如果是全外连接则返回true，否则返回false

#### 4.6.6 是否是交叉连接

```cpp
bool is_cross_join() const;
```

- **功能**：检查是否是交叉连接
- **返回值**：如果是交叉连接则返回true，否则返回false

#### 4.6.7 是否是自然连接

```cpp
bool is_natural_join() const;
```

- **功能**：检查是否是自然连接
- **返回值**：如果是自然连接则返回true，否则返回false

#### 4.6.8 是否是半连接

```cpp
bool is_semi_join() const;
```

- **功能**：检查是否是半连接
- **返回值**：如果是半连接则返回true，否则返回false

#### 4.6.9 是否是反连接

```cpp
bool is_anti_join() const;
```

- **功能**：检查是否是反连接
- **返回值**：如果是反连接则返回true，否则返回false

#### 4.6.10 是否有连接条件

```cpp
bool has_condition() const;
```

- **功能**：检查是否有连接条件
- **返回值**：如果有连接条件则返回true，否则返回false

## 5. 实现细节

### 5.1 构造函数实现

```cpp
Join::Join(std::shared_ptr<ExecutionPlan> left_plan,
          std::shared_ptr<ExecutionPlan> right_plan,
          std::shared_ptr<Expression> condition,
          JoinType type)
    : left_plan_(left_plan),
      right_plan_(right_plan),
      condition_(condition),
      join_type_(type),
      left_stats_(nullptr),
      right_stats_(nullptr) {
    // 初始化连接操作
}
```

### 5.2 连接类型检查实现

```cpp
bool Join::is_inner_join() const {
    return join_type_ == INNER;
}

bool Join::is_outer_join() const {
    return join_type_ == LEFT_OUTER || join_type_ == RIGHT_OUTER || join_type_ == FULL_OUTER;
}

bool Join::is_left_outer_join() const {
    return join_type_ == LEFT_OUTER;
}

bool Join::is_right_outer_join() const {
    return join_type_ == RIGHT_OUTER;
}

bool Join::is_full_outer_join() const {
    return join_type_ == FULL_OUTER;
}

bool Join::is_cross_join() const {
    return join_type_ == CROSS;
}

bool Join::is_natural_join() const {
    return join_type_ == NATURAL;
}

bool Join::is_semi_join() const {
    return join_type_ == SEMI;
}

bool Join::is_anti_join() const {
    return join_type_ == ANTI;
}

bool Join::has_condition() const {
    return condition_ != nullptr;
}
```

## 6. 扩展类

### 6.1 HashJoin

HashJoin 是 Join 的派生类，用于实现哈希连接操作：

```cpp
class HashJoin : public Join {
public:
    HashJoin(std::shared_ptr<ExecutionPlan> left_plan,
            std::shared_ptr<ExecutionPlan> right_plan,
            std::shared_ptr<Expression> condition,
            const std::vector<std::string>& left_key_columns,
            const std::vector<std::string>& right_key_columns,
            JoinType type = INNER);

    // 获取左表连接键列
    const std::vector<std::string>& get_left_key_columns() const;

    // 获取右表连接键列
    const std::vector<std::string>& get_right_key_columns() const;

    // 获取哈希表大小
    size_t get_hash_table_size() const;

    // 设置哈希表大小
    void set_hash_table_size(size_t size);

private:
    std::vector<std::string> left_key_columns_;    // 左表连接键列
    std::vector<std::string> right_key_columns_;   // 右表连接键列
    size_t hash_table_size_;                       // 哈希表大小
};
```

### 6.2 SortMergeJoin

SortMergeJoin 是 Join 的派生类，用于实现排序合并连接操作：

```cpp
class SortMergeJoin : public Join {
public:
    SortMergeJoin(std::shared_ptr<ExecutionPlan> left_plan,
                 std::shared_ptr<ExecutionPlan> right_plan,
                 std::shared_ptr<Expression> condition,
                 const std::vector<std::string>& left_sort_columns,
                 const std::vector<std::string>& right_sort_columns,
                 JoinType type = INNER);

    // 获取左表排序列
    const std::vector<std::string>& get_left_sort_columns() const;

    // 获取右表排序列
    const std::vector<std::string>& get_right_sort_columns() const;

    // 获取排序方向
    SortDirection get_sort_direction() const;

    // 设置排序方向
    void set_sort_direction(SortDirection direction);

private:
    std::vector<std::string> left_sort_columns_;   // 左表排序列
    std::vector<std::string> right_sort_columns_;  // 右表排序列
    SortDirection sort_direction_;                 // 排序方向
};
```

## 7. 性能优化

### 7.1 连接算法选择

根据数据量和分布选择合适的连接算法：

```cpp
// 根据数据量选择连接算法
if (left_rows < 10000 && right_rows < 10000) {
    // 小表使用嵌套循环连接
    auto join = std::make_shared<NestedLoopJoin>(left_plan, right_plan, condition);
} else if (left_rows < right_rows) {
    // 左表小，使用哈希连接（左表作为构建表）
    auto join = std::make_shared<HashJoin>(left_plan, right_plan, condition);
} else {
    // 两表都大，使用排序合并连接
    auto join = std::make_shared<SortMergeJoin>(left_plan, right_plan, condition);
}
```

### 7.2 连接顺序优化

优化连接表的顺序，减少中间结果集的大小：

```cpp
// 重新排序连接表，小表先连接
auto sorted_joins = sort_joins_by_size(joins);
auto optimized_join = create_join_tree(sorted_joins);
```

### 7.3 连接条件下推

将连接条件下推到表扫描操作，减少读取的数据量：

```cpp
// 将连接条件下推到左表扫描
if (is_push_down_possible(join_condition, left_table)) {
    left_plan->set_filter(join_condition);
    join->set_condition(nullptr);
}
```

### 7.4 索引使用优化

对于等值连接，使用索引来加速连接操作：

```cpp
// 检查右表是否有适合的索引
if (has_index(right_table, join_columns)) {
    // 使用索引扫描加速右表访问
    auto right_index_scan = std::make_shared<IndexScan>(right_table, index_name, columns);
    auto join = std::make_shared<NestedLoopJoin>(left_plan, right_index_scan, condition);
}
```

## 8. 扩展点

### 8.1 新连接算法支持

可以通过继承 Join 类来实现新的连接算法：

```cpp
class BloomFilterJoin : public Join {
public:
    BloomFilterJoin(std::shared_ptr<ExecutionPlan> left_plan,
                   std::shared_ptr<ExecutionPlan> right_plan,
                   std::shared_ptr<Expression> condition,
                   double false_positive_rate,
                   JoinType type = INNER);

    // 获取布隆过滤器的误判率
    double get_false_positive_rate() const;

    // 设置布隆过滤器的误判率
    void set_false_positive_rate(double rate);

private:
    double false_positive_rate_;  // 布隆过滤器的误判率
};
```

### 8.2 新连接类型支持

可以扩展 JoinType 枚举来支持新的连接类型：

```cpp
enum JoinType {
    INNER,
    LEFT_OUTER,
    RIGHT_OUTER,
    FULL_OUTER,
    CROSS,
    NATURAL,
    SEMI,
    ANTI,
    LATERAL,          // 新增：LATERAL连接
    APPLY,            // 新增：APPLY连接
    SORT_MERGE        // 新增：排序合并连接
};
```

## 9. 错误处理

Join 类通过抛出异常来处理执行错误。当连接操作失败时，会抛出相应的异常，包含错误信息。

## 10. 测试支持

Join 类提供了全面的单元测试和集成测试支持，确保其功能的正确性和稳定性。测试覆盖了所有主要的连接类型和算法。

## 11. 使用示例

### 11.1 创建连接操作

```cpp
// 创建表扫描操作
auto left_scan = std::make_shared<TableScan>("users", {"id", "name", "dept_id"});
auto right_scan = std::make_shared<TableScan>("departments", {"dept_id", "dept_name"});

// 创建连接条件
auto condition = std::make_shared<BinaryExpression>(
    "users.dept_id",
    BinaryOperator::EQUAL,
    "departments.dept_id"
);

// 创建内连接
auto inner_join = std::make_shared<Join>(left_scan, right_scan, condition, Join::INNER);

// 创建左外连接
auto left_join = std::make_shared<Join>(left_scan, right_scan, condition, Join::LEFT_OUTER);

// 创建右外连接
auto right_join = std::make_shared<Join>(left_scan, right_scan, condition, Join::RIGHT_OUTER);

// 创建全外连接
auto full_join = std::make_shared<Join>(left_scan, right_scan, condition, Join::FULL_OUTER);
```

### 11.2 使用连接操作进行代价估算

```cpp
// 创建代价估算器
CostEstimator estimator;

// 创建连接操作
auto join = std::make_shared<Join>(left_scan, right_scan, condition, Join::INNER);

// 估算连接代价
QueryCost cost = estimator.estimate_join_cost(join.get());

// 输出代价信息
std::cout << "Join cost: " << cost.total_cost << std::endl;
std::cout << "CPU cost: " << cost.cpu_cost << std::endl;
std::cout << "I/O cost: " << cost.io_cost << std::endl;
std::cout << "Memory cost: " << cost.memory_cost << std::endl;
```

## 12. 总结

Join 是 SQLCC 数据库系统中连接操作的核心类，代表两个或多个表之间的连接操作。它提供了丰富的功能和灵活的扩展机制，支持各种连接类型和优化技术。Join 类的设计模块化、可扩展，便于维护和扩展，为 SQLCC 数据库系统提供了高效的数据关联能力。