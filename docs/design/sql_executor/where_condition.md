# WhereCondition类详细设计

## 概述

WhereCondition类表示SQL查询中的WHERE条件，用于过滤查询结果。它简化了WHERE条件的表示，便于在查询执行过程中进行条件匹配和验证。

## 类定义

```cpp
struct WhereCondition {
  std::string column_name;   ///< 列名
  std::string operator_type; ///< 操作符: "=", ">", "<", "!=", etc.
  std::string value;         ///< 比较值

  WhereCondition(std::string col, std::string op, std::string val)
      : column_name(std::move(col)), operator_type(std::move(op)),
        value(std::move(val)) {}
};
```

## 构造函数

### WhereCondition(std::string col, std::string op, std::string val)

构造函数：

1. 初始化列名
2. 初始化操作符类型
3. 初始化比较值
4. 使用std::move优化字符串传递

## 成员变量

### std::string column_name

列名：

1. 存储WHERE条件中涉及的列名
2. 用于在记录中定位要比较的字段

### std::string operator_type

操作符类型：

1. 存储比较操作符
2. 支持常见的比较操作符如"=", ">", "<", ">=", "<=", "!="等
3. 可扩展支持LIKE、IN等操作符

### std::string value

比较值：

1. 存储与列值进行比较的目标值
2. 以字符串形式存储，便于通用处理
3. 在比较时根据列的数据类型进行适当转换