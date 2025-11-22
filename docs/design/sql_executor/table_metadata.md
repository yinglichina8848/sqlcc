# TableMetadata类详细设计

## 概述

TableMetadata类用于存储和管理数据库表的元数据信息，包括表名、列定义、约束信息等。它是SQL执行引擎中重要的数据结构，为SQL语句执行提供必要的表结构信息。

## 类定义

```cpp
struct TableMetadata {
  std::string table_name;                                      ///< 表名
  std::vector<sql_parser::ColumnDefinition> columns;           ///< 列定义列表
  std::unordered_map<std::string, size_t> column_indexes;      ///< 列名到列索引映射
  std::vector<sql_parser::TableConstraint> constraints;        ///< 表级约束列表
  uint64_t record_count = 0;                                   ///< 记录数量
  uint32_t root_page_id = 0;                                   ///< 数据页面根节点

  /**
   * @brief 获取指定列的定义
   * @param column_name 列名
   * @return 列定义指针，如果不存在则返回nullptr
   */
  const sql_parser::ColumnDefinition *
  GetColumnDef(const std::string &column_name) const;
};
```


## 公共方法

### const sql_parser::ColumnDefinition *GetColumnDef(const std::string &column_name) const

获取指定列的定义：

1. 在column_indexes中查找列名
2. 如果找到则返回对应索引的列定义
3. 如果未找到则返回nullptr

## 公共方法

### const sql_parser::ColumnDefinition *GetColumnDef(const std::string &column_name) const

获取指定列的定义：

1. 在[column_indexes](file:///home/liying/sqlcc/include/sql_executor.h#L54-L54)中查找列名
2. 如果找到则返回对应索引的列定义
3. 如果未找到则返回nullptr

## 成员变量

### std::string table_name

表名：

1. 存储表的名称
2. 用于标识表的唯一性

### std::vector<sql_parser::ColumnDefinition> columns

列定义列表：

1. 存储表中所有列的定义信息
2. 包括列名、数据类型、约束等

### std::unordered_map<std::string, size_t> column_indexes

列名到列索引的映射：

1. 提供从列名到列定义在[columns](file:///home/liying/sqlcc/include/sql_executor.h#L53-L53)中索引的快速查找
2. 提高列查找效率
3. 键为列名，值为在[columns](file:///home/liying/sqlcc/include/sql_executor.h#L53-L53)中的索引

### std::vector<sql_parser::TableConstraint> constraints

表级约束列表：

1. 存储表级约束定义
2. 包括主键、唯一性、外键、检查约束等

### uint64_t record_count

记录数量：

1. 存储表中记录的数量
2. 用于统计和性能优化

### uint32_t root_page_id

数据页面根节点：

1. 存储表数据的根页面ID
2. 用于定位表数据在存储引擎中的位置