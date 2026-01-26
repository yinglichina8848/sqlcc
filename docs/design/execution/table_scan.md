# TableScan 设计文档

## 1. 概述

TableScan 是 SQLCC 数据库系统中的表扫描操作类，代表对数据库表的全表扫描或范围扫描操作。它是查询执行计划中的基础操作，用于从表中读取数据。

## 2. 核心功能

### 2.1 主要功能

- **表数据读取**：从表中读取数据行
- **过滤条件应用**：应用过滤条件筛选符合条件的行
- **列投影**：只读取查询所需的列，减少数据传输
- **统计信息收集**：收集表扫描操作的统计信息
- **性能优化**：支持各种表扫描优化技术

### 2.2 设计优势

- **模块化**：将表扫描操作封装为独立的类，便于维护和扩展
- **优化支持**：支持各种表扫描优化技术，如索引扫描、范围扫描等
- **统计信息集成**：集成统计信息，便于代价估算和查询优化
- **类型安全**：确保表扫描操作的类型安全
- **可扩展**：可以轻松添加新的表扫描优化技术

## 3. 类定义

```cpp
class TableScan {
public:
    TableScan() = default;
    TableScan(const std::string& table_name,
              const std::vector<std::string>& columns,
              std::shared_ptr<Expression> filter = nullptr);
    virtual ~TableScan() = default;

    // 获取表名
    const std::string& get_table_name() const;

    // 获取扫描的列列表
    const std::vector<std::string>& get_columns() const;

    // 获取过滤条件
    std::shared_ptr<Expression> get_filter() const;

    // 设置过滤条件
    void set_filter(std::shared_ptr<Expression> filter);

    // 获取表统计信息
    const TableStatistics* get_statistics() const;

    // 设置表统计信息
    void set_statistics(const TableStatistics* stats);

    // 是否是全表扫描
    bool is_full_scan() const;

    // 是否是索引扫描
    bool is_index_scan() const;

    // 是否有过滤条件
    bool has_filter() const;

protected:
    std::string table_name_;              // 表名
    std::vector<std::string> columns_;    // 扫描的列列表
    std::shared_ptr<Expression> filter_;  // 过滤条件
    const TableStatistics* statistics_;   // 表统计信息
    bool is_full_scan_;                   // 是否是全表扫描
};
```

## 4. 核心组件

### 4.1 构造函数

```cpp
TableScan(const std::string& table_name,
          const std::vector<std::string>& columns,
          std::shared_ptr<Expression> filter = nullptr);
```

- **功能**：初始化表扫描操作
- **参数**：
  - `table_name` - 表名
  - `columns` - 扫描的列列表
  - `filter` - 过滤条件（可选）

### 4.2 表信息访问

#### 4.2.1 获取表名

```cpp
const std::string& get_table_name() const;
```

- **功能**：获取表扫描的表名
- **返回值**：表名的常量引用

#### 4.2.2 获取扫描的列列表

```cpp
const std::vector<std::string>& get_columns() const;
```

- **功能**：获取表扫描操作读取的列列表
- **返回值**：列列表的常量引用

### 4.3 过滤条件

#### 4.3.1 获取过滤条件

```cpp
std::shared_ptr<Expression> get_filter() const;
```

- **功能**：获取表扫描操作的过滤条件
- **返回值**：过滤条件的智能指针

#### 4.3.2 设置过滤条件

```cpp
void set_filter(std::shared_ptr<Expression> filter);
```

- **功能**：设置表扫描操作的过滤条件
- **参数**：`filter` - 过滤条件的智能指针

### 4.4 统计信息

#### 4.4.1 获取表统计信息

```cpp
const TableStatistics* get_statistics() const;
```

- **功能**：获取表扫描操作的统计信息
- **返回值**：表统计信息的指针

#### 4.4.2 设置表统计信息

```cpp
void set_statistics(const TableStatistics* stats);
```

- **功能**：设置表扫描操作的统计信息
- **参数**：`stats` - 表统计信息的指针

### 4.5 扫描类型

#### 4.5.1 是否是全表扫描

```cpp
bool is_full_scan() const;
```

- **功能**：检查是否是全表扫描操作
- **返回值**：如果是全表扫描则返回true，否则返回false

#### 4.5.2 是否是索引扫描

```cpp
bool is_index_scan() const;
```

- **功能**：检查是否是索引扫描操作
- **返回值**：如果是索引扫描则返回true，否则返回false

#### 4.5.3 是否有过滤条件

```cpp
bool has_filter() const;
```

- **功能**：检查是否有过滤条件
- **返回值**：如果有过滤条件则返回true，否则返回false

## 5. 实现细节

### 5.1 构造函数实现

```cpp
TableScan::TableScan(const std::string& table_name,
                   const std::vector<std::string>& columns,
                   std::shared_ptr<Expression> filter) 
    : table_name_(table_name),
      columns_(columns),
      filter_(filter),
      statistics_(nullptr),
      is_full_scan_(true) {
    // 如果有索引条件，设置为索引扫描
    // 这里可以根据过滤条件判断是否可以使用索引
}
```

### 5.2 扫描类型检查实现

```cpp
bool TableScan::is_full_scan() const {
    return is_full_scan_;
}

bool TableScan::is_index_scan() const {
    return !is_full_scan_;
}

bool TableScan::has_filter() const {
    return filter_ != nullptr;
}
```

## 6. 扩展类

### 6.1 IndexScan

IndexScan 是 TableScan 的派生类，用于实现索引扫描操作：

```cpp
class IndexScan : public TableScan {
public:
    IndexScan(const std::string& table_name,
             const std::string& index_name,
             const std::vector<std::string>& columns,
             std::shared_ptr<Expression> filter = nullptr);

    // 获取索引名
    const std::string& get_index_name() const;

    // 获取索引范围
    const IndexRange& get_index_range() const;

private:
    std::string index_name_;    // 索引名
    IndexRange index_range_;    // 索引范围
};
```

### 6.2 RangeScan

RangeScan 是 TableScan 的派生类，用于实现范围扫描操作：

```cpp
class RangeScan : public TableScan {
public:
    RangeScan(const std::string& table_name,
             const std::vector<std::string>& columns,
             std::shared_ptr<Expression> lower_bound,
             std::shared_ptr<Expression> upper_bound,
             bool include_lower = true,
             bool include_upper = true);

    // 获取下界
    std::shared_ptr<Expression> get_lower_bound() const;

    // 获取上界
    std::shared_ptr<Expression> get_upper_bound() const;

    // 是否包含下界
    bool include_lower() const;

    // 是否包含上界
    bool include_upper() const;

private:
    std::shared_ptr<Expression> lower_bound_;  // 下界
    std::shared_ptr<Expression> upper_bound_;  // 上界
    bool include_lower_;                       // 是否包含下界
    bool include_upper_;                       // 是否包含上界
};
```

## 7. 性能优化

### 7.1 列投影优化

只读取查询所需的列，减少数据传输和内存使用：

```cpp
// 设置投影列
std::vector<std::string> projection_columns = {"id", "name", "age"};
TableScan scan("users", projection_columns);
```

### 7.2 过滤条件下推

将过滤条件下推到表扫描操作，减少读取的数据量：

```cpp
// 创建过滤条件
auto filter = std::make_shared<BinaryExpression>(
    "age",
    BinaryOperator::GREATER_THAN,
    "18"
);

// 创建表扫描并应用过滤条件
TableScan scan("users", {"id", "name", "age"}, filter);
```

### 7.3 索引扫描优化

对于有索引的列，使用索引扫描替代全表扫描：

```cpp
// 创建索引扫描
IndexScan scan("users", "idx_age", {"id", "name", "age"}, filter);
```

### 7.4 范围扫描优化

对于范围查询，使用范围扫描减少读取的数据量：

```cpp
// 创建范围扫描
RangeScan scan("users", {"id", "name", "age"},
              std::make_shared<BinaryExpression>("age", BinaryOperator::GREATER_THAN, "18"),
              std::make_shared<BinaryExpression>("age", BinaryOperator::LESS_THAN, "30"));
```

## 8. 扩展点

### 8.1 新扫描类型支持

可以通过继承 TableScan 类来实现新类型的表扫描操作：

```cpp
class PartitionScan : public TableScan {
public:
    PartitionScan(const std::string& table_name,
                 const std::vector<std::string>& partitions,
                 const std::vector<std::string>& columns,
                 std::shared_ptr<Expression> filter = nullptr);

    // 获取分区列表
    const std::vector<std::string>& get_partitions() const;

private:
    std::vector<std::string> partitions_;  // 分区列表
};
```

### 8.2 新优化技术

可以扩展 TableScan 类来支持新的表扫描优化技术：

```cpp
class ColumnStoreScan : public TableScan {
public:
    ColumnStoreScan(const std::string& table_name,
                   const std::vector<std::string>& columns,
                   std::shared_ptr<Expression> filter = nullptr);

    // 列存扫描特定方法
    void enable_vectorization();
    void set_batch_size(size_t batch_size);

private:
    bool vectorization_enabled_;  // 是否启用向量化
    size_t batch_size_;            // 批量大小
};
```

## 9. 错误处理

TableScan 类通过抛出异常来处理执行错误。当表扫描操作失败时，会抛出相应的异常，包含错误信息。

## 10. 测试支持

TableScan 类提供了全面的单元测试和集成测试支持，确保其功能的正确性和稳定性。测试覆盖了所有主要的表扫描类型和操作功能。

## 11. 使用示例

### 11.1 创建表扫描

```cpp
// 创建全表扫描
TableScan full_scan("users", {"id", "name", "age"});

// 创建带过滤条件的表扫描
auto filter = std::make_shared<BinaryExpression>(
    "age",
    BinaryOperator::GREATER_THAN,
    "18"
);
TableScan filtered_scan("users", {"id", "name", "age"}, filter);

// 创建索引扫描
IndexScan index_scan("users", "idx_age", {"id", "name", "age"}, filter);

// 创建范围扫描
RangeScan range_scan("users", {"id", "name", "age"},
                     std::make_shared<BinaryExpression>("age", BinaryOperator::GREATER_THAN, "18"),
                     std::make_shared<BinaryExpression>("age", BinaryOperator::LESS_THAN, "30"));
```

### 11.2 使用表扫描进行代价估算

```cpp
// 创建代价估算器
CostEstimator estimator;

// 创建表扫描
TableScan scan("users", {"id", "name", "age"});

// 估算表扫描代价
QueryCost cost = estimator.estimate_scan_cost(&scan);

// 输出代价信息
std::cout << "Table scan cost: " << cost.total_cost << std::endl;
std::cout << "CPU cost: " << cost.cpu_cost << std::endl;
std::cout << "I/O cost: " << cost.io_cost << std::endl;
```

## 12. 总结

TableScan 是 SQLCC 数据库系统中表扫描操作的核心类，代表对数据库表的全表扫描或范围扫描操作。它提供了丰富的功能和灵活的扩展机制，支持各种表扫描优化技术。TableScan 类的设计模块化、可扩展，便于维护和扩展，为 SQLCC 数据库系统提供了高效的数据读取能力。