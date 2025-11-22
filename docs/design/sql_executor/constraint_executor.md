# ConstraintExecutor类详细设计

## 概述

ConstraintExecutor类是SQL执行引擎中负责处理表约束验证的组件。它提供了统一的接口来验证各种类型的表约束，包括主键约束、唯一性约束、外键约束和检查约束等。

## 类定义

```cpp
class ConstraintExecutor {
public:
    virtual ~ConstraintExecutor() = default;

    /**
     * @brief 验证插入操作是否满足约束
     * @param record 插入的记录
     * @param table_schema 表结构定义
     * @return 是否满足约束
     */
    virtual bool validateInsert(
        const std::vector<std::string> &record,
        const std::vector<sql_parser::ColumnDefinition> &table_schema) = 0;

    /**
     * @brief 验证更新操作是否满足约束
     * @param old_record 更新前的记录
     * @param new_record 更新后的记录
     * @param table_schema 表结构定义
     * @return 是否满足约束
     */
    virtual bool validateUpdate(
        const std::vector<std::string> &old_record,
        const std::vector<std::string> &new_record,
        const std::vector<sql_parser::ColumnDefinition> &table_schema) = 0;

    /**
     * @brief 验证删除操作是否满足约束
     * @param record 删除的记录
     * @param table_schema 表结构定义
     * @return 是否满足约束
     */
    virtual bool validateDelete(
        const std::vector<std::string> &record,
        const std::vector<sql_parser::ColumnDefinition> &table_schema) = 0;
};
```

## 派生类

### PrimaryKeyConstraintExecutor类

主键约束执行器：

```cpp
class PrimaryKeyConstraintExecutor : public ConstraintExecutor {
public:
    explicit PrimaryKeyConstraintExecutor(const sql_parser::PrimaryKeyConstraint& constraint);
    
    bool ValidateInsert(const std::vector<std::string>& record,
                       const std::vector<sql_parser::ColumnDefinition>& table_schema) override;
    
    bool ValidateUpdate(const std::vector<std::string>& old_record,
                       const std::vector<std::string>& new_record,
                       const std::vector<sql_parser::ColumnDefinition>& table_schema) override;
    
    bool ValidateDelete(const std::vector<std::string>& record,
                       const std::vector<sql_parser::ColumnDefinition>& table_schema) override;

private:
    std::vector<size_t> key_columns_;  ///< 主键列索引列表
};
```

### UniqueConstraintExecutor类

唯一性约束执行器：

```cpp
class UniqueConstraintExecutor : public ConstraintExecutor {
public:
    explicit UniqueConstraintExecutor(const sql_parser::UniqueConstraint& constraint);
    
    bool ValidateInsert(const std::vector<std::string>& record,
                       const std::vector<sql_parser::ColumnDefinition>& table_schema) override;
    
    bool ValidateUpdate(const std::vector<std::string>& old_record,
                       const std::vector<std::string>& new_record,
                       const std::vector<sql_parser::ColumnDefinition>& table_schema) override;
    
    bool ValidateDelete(const std::vector<std::string>& record,
                       const std::vector<sql_parser::ColumnDefinition>& table_schema) override;

private:
    std::vector<size_t> unique_columns_;  ///< 唯一性列索引列表
};
```

### ForeignKeyConstraintExecutor类

外键约束执行器：

```cpp
class ForeignKeyConstraintExecutor : public ConstraintExecutor {
public:
    explicit ForeignKeyConstraintExecutor(const sql_parser::ForeignKeyConstraint& constraint);
    
    bool ValidateInsert(const std::vector<std::string>& record,
                       const std::vector<sql_parser::ColumnDefinition>& table_schema) override;
    
    bool ValidateUpdate(const std::vector<std::string>& old_record,
                       const std::vector<std::string>& new_record,
                       const std::vector<sql_parser::ColumnDefinition>& table_schema) override;
    
    bool ValidateDelete(const std::vector<std::string>& record,
                       const std::vector<sql_parser::ColumnDefinition>& table_schema) override;

private:
    std::vector<size_t> foreign_columns_;      ///< 外键列索引列表
    std::string referenced_table_;             ///< 引用的表名
    std::vector<size_t> referenced_columns_;   ///< 引用的列索引列表
};
```

### CheckConstraintExecutor类

检查约束执行器：

```cpp
class CheckConstraintExecutor : public ConstraintExecutor {
public:
    explicit CheckConstraintExecutor(const sql_parser::CheckConstraint& constraint);
    
    bool ValidateInsert(const std::vector<std::string>& record,
                       const std::vector<sql_parser::ColumnDefinition>& table_schema) override;
    
    bool ValidateUpdate(const std::vector<std::string>& old_record,
                       const std::vector<std::string>& new_record,
                       const std::vector<sql_parser::ColumnDefinition>& table_schema) override;
    
    bool ValidateDelete(const std::vector<std::string>& record,
                       const std::vector<sql_parser::ColumnDefinition>& table_schema) override;

private:
    std::unique_ptr<sql_parser::Expression> condition_;  ///< 检查条件表达式
};
```

## ConstraintExecutor类

### virtual ~ConstraintExecutor()

虚析构函数：

1. 确保派生类能正确析构
2. 使用默认虚析构函数

### virtual bool validateInsert(const std::vector<std::string>& record, const std::vector<sql_parser::ColumnDefinition>& table_schema) = 0

验证插入操作是否满足约束：

1. 纯虚函数，由派生类实现具体验证逻辑
2. 参数record表示要插入的记录数据
3. 参数table_schema表示表的结构定义

### virtual bool validateUpdate(const std::vector<std::string>& old_record, const std::vector<std::string>& new_record, const std::vector<sql_parser::ColumnDefinition>& table_schema) = 0

验证更新操作是否满足约束：

1. 纯虚函数，由派生类实现具体验证逻辑
2. 参数old_record表示更新前的记录数据
3. 参数new_record表示更新后的记录数据
4. 参数table_schema表示表的结构定义

### virtual bool validateDelete(const std::vector<std::string>& record, const std::vector<sql_parser::ColumnDefinition>& table_schema) = 0

验证删除操作是否满足约束：

1. 纯虚函数，由派生类实现具体验证逻辑
2. 参数record表示要删除的记录数据
3. 参数table_schema表示表的结构定义

## PrimaryKeyConstraintExecutor类

### PrimaryKeyConstraintExecutor(const sql_parser::PrimaryKeyConstraint& constraint)

构造函数：

1. 接收主键约束定义
2. 初始化主键约束执行器

### bool ValidateInsert(const std::vector<std::string>& record, const std::vector<sql_parser::ColumnDefinition>& table_schema)

验证插入操作是否满足主键约束：

1. 检查主键列是否为空
2. 检查主键值是否唯一

### bool ValidateUpdate(const std::vector<std::string>& old_record, const std::vector<std::string>& new_record, const std::vector<sql_parser::ColumnDefinition>& table_schema)

验证更新操作是否满足主键约束：

1. 检查主键列是否被修改
2. 如果修改，检查新主键值是否唯一

### bool ValidateDelete(const std::vector<std::string>& record, const std::vector<sql_parser::ColumnDefinition>& table_schema)

验证删除操作是否满足主键约束：

1. 主键约束对删除操作无特殊要求

### private成员变量

#### std::vector<size_t> key_columns_

主键列索引列表：

1. 存储主键列在表中的索引位置

## UniqueConstraintExecutor类

### UniqueConstraintExecutor(const sql_parser::UniqueConstraint& constraint)

构造函数：

1. 接收唯一性约束定义
2. 初始化唯一性约束执行器

### bool ValidateInsert(const std::vector<std::string>& record, const std::vector<sql_parser::ColumnDefinition>& table_schema)

验证插入操作是否满足唯一性约束：

1. 检查唯一性列的值是否已存在

### bool ValidateUpdate(const std::vector<std::string>& old_record, const std::vector<std::string>& new_record, const std::vector<sql_parser::ColumnDefinition>& table_schema)

验证更新操作是否满足唯一性约束：

1. 如果唯一性列被修改，检查新值是否已存在

### bool ValidateDelete(const std::vector<std::string>& record, const std::vector<sql_parser::ColumnDefinition>& table_schema)

验证删除操作是否满足唯一性约束：

1. 唯一性约束对删除操作无特殊要求

### private成员变量

#### std::vector<size_t> unique_columns_

唯一性列索引列表：

1. 存储唯一性列在表中的索引位置

## ForeignKeyConstraintExecutor类

### ForeignKeyConstraintExecutor(const sql_parser::ForeignKeyConstraint& constraint)

构造函数：

1. 接收外键约束定义
2. 初始化外键约束执行器

### bool ValidateInsert(const std::vector<std::string>& record, const std::vector<sql_parser::ColumnDefinition>& table_schema)

验证插入操作是否满足外键约束：

1. 检查外键列的值在引用表中是否存在

### bool ValidateUpdate(const std::vector<std::string>& old_record, const std::vector<std::string>& new_record, const std::vector<sql_parser::ColumnDefinition>& table_schema)

验证更新操作是否满足外键约束：

1. 如果外键列被修改，检查新值在引用表中是否存在

### bool ValidateDelete(const std::vector<std::string>& record, const std::vector<sql_parser::ColumnDefinition>& table_schema)

验证删除操作是否满足外键约束：

1. 检查是否有其他表的外键引用要删除的记录

### private成员变量

#### std::vector<size_t> foreign_columns_

外键列索引列表：

1. 存储外键列在表中的索引位置

#### std::string referenced_table_

引用的表名：

1. 存储外键引用的表名

#### std::vector<size_t> referenced_columns_

引用的列索引列表：

1. 存储外键引用的列在引用表中的索引位置

## CheckConstraintExecutor类

### CheckConstraintExecutor(const sql_parser::CheckConstraint& constraint)

构造函数：

1. 接收检查约束定义
2. 初始化检查约束执行器

### bool ValidateInsert(const std::vector<std::string>& record, const std::vector<sql_parser::ColumnDefinition>& table_schema)

验证插入操作是否满足检查约束：

1. 计算检查条件表达式
2. 验证结果是否为真

### bool ValidateUpdate(const std::vector<std::string>& old_record, const std::vector<std::string>& new_record, const std::vector<sql_parser::ColumnDefinition>& table_schema)

验证更新操作是否满足检查约束：

1. 计算检查条件表达式（使用新记录）
2. 验证结果是否为真

### bool ValidateDelete(const std::vector<std::string>& record, const std::vector<sql_parser::ColumnDefinition>& table_schema)

验证删除操作是否满足检查约束：

1. 检查约束对删除操作无特殊要求

### private成员变量

#### std::unique_ptr<sql_parser::Expression> condition_

检查条件表达式：

1. 存储检查约束的条件表达式
2. 用于验证记录是否满足约束条件