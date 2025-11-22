# Record类详细设计

## 概述

Record类表示数据库表中的一行数据记录。它提供了数据记录的存储、访问和操作接口，是SQL执行引擎中处理数据的基本单位。

## 类定义

```cpp
struct Record {
  std::vector<std::string> column_values; ///< 列值数组（按表定义的列顺序）
  uint64_t record_id;                     ///< 记录全局唯一ID
  uint64_t txn_id = 0;                    ///< 事务ID
  std::string table_name;                 ///< 所属表名

  Record() = default;
  Record(std::vector<std::string> values, uint64_t rid = 0)
      : column_values(std::move(values)), record_id(rid) {}
};
```

## 构造函数

### Record()

默认构造函数：

1. 创建空记录
2. 初始化所有成员变量为默认值

### Record(std::vector<std::string> values, uint64_t rid = 0)

构造函数，使用字段值列表初始化记录：

1. 使用给定的字段值列表初始化[column_values](file:///home/liying/sqlcc/include/sql_executor.h#L26-L26)
2. 设置记录ID
3. 使用默认值初始化其他成员变量

## 成员变量

### std::vector<std::string> column_values

列值数组：

1. 按表定义的列顺序存储各列的值
2. 使用字符串类型存储所有数据类型，便于通用处理

### uint64_t record_id

记录全局唯一ID：

1. 标识记录的唯一性
2. 用于记录定位和操作

### uint64_t txn_id

事务ID：

1. 标识记录所属的事务
2. 用于事务管理和并发控制
3. 默认值为0，表示不属于任何事务

### std::string table_name

所属表名：

1. 标识记录所属的表
2. 用于元数据查找和验证
