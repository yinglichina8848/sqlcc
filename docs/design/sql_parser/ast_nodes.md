# AST节点类详细设计

## 概述

抽象语法树(AST)节点类表示SQL语句的结构化表示。每个节点对应SQL语句中的一个语法成分，如SELECT语句、WHERE子句、表达式等。这些类构成了SQL语句的树形结构表示，便于后续的语义分析和执行。

## 核心AST节点类

### Statement类

所有SQL语句的基类：

```cpp
class Statement : public AstNode {
public:
    virtual ~Statement() = default;
    virtual void accept(AstVisitor& visitor) = 0;
};
```

### Expression类

所有表达式的基类：

```cpp
class Expression : public AstNode {
public:
    virtual ~Expression() = default;
    virtual void accept(AstVisitor& visitor) = 0;
};
```

### SelectStatement类

SELECT语句节点：

```cpp
class SelectStatement : public Statement {
public:
    std::vector<SelectItem> select_items;
    std::vector<TableReference> from_clauses;
    std::unique_ptr<WhereClause> where_clause;
    std::unique_ptr<GroupByClause> group_by_clause;
    std::unique_ptr<OrderByClause> order_by_clause;
    std::unique_ptr<JoinClause> join_clause;
    
    void accept(AstVisitor& visitor) override;
};
```

### InsertStatement类

INSERT语句节点：

```cpp
class InsertStatement : public Statement {
public:
    std::string table_name;
    std::vector<std::string> columns;
    std::vector<std::vector<std::unique_ptr<Expression>>> values;
    
    void accept(AstVisitor& visitor) override;
};
```

### UpdateStatement类

UPDATE语句节点：

```cpp
class UpdateStatement : public Statement {
public:
    std::string table_name;
    std::vector<UpdateClause> update_clauses;
    std::unique_ptr<WhereClause> where_clause;
    
    void accept(AstVisitor& visitor) override;
};
```

### DeleteStatement类

DELETE语句节点：

```cpp
class DeleteStatement : public Statement {
public:
    std::string table_name;
    std::unique_ptr<WhereClause> where_clause;
    
    void accept(AstVisitor& visitor) override;
};
```

### CreateStatement类

CREATE语句节点：

```cpp
class CreateStatement : public Statement {
public:
    enum Type { DATABASE, TABLE };
    
    Type create_type;
    std::string name;
    std::vector<ColumnDefinition> columns;
    std::vector<std::unique_ptr<TableConstraint>> constraints;
    
    void accept(AstVisitor& visitor) override;
};
```

### DropStatement类

DROP语句节点：

```cpp
class DropStatement : public Statement {
public:
    enum Type { DATABASE, TABLE };
    
    Type drop_type;
    std::string name;
    
    void accept(AstVisitor& visitor) override;
};
```

### AlterStatement类

ALTER语句节点：

```cpp
class AlterStatement : public Statement {
public:
    std::string table_name;
    // ALTER操作的具体内容
    
    void accept(AstVisitor& visitor) override;
};
```

### UseStatement类

USE语句节点：

```cpp
class UseStatement : public Statement {
public:
    std::string database_name;
    
    void accept(AstVisitor& visitor) override;
};
```

### BeginTransactionStatement类

BEGIN TRANSACTION语句节点：

```cpp
class BeginTransactionStatement : public Statement {
public:
    void accept(AstVisitor& visitor) override;
};
```

### CommitStatement类

COMMIT语句节点：

```cpp
class CommitStatement : public Statement {
public:
    void accept(AstVisitor& visitor) override;
};
```

### RollbackStatement类

ROLLBACK语句节点：

```cpp
class RollbackStatement : public Statement {
public:
    void accept(AstVisitor& visitor) override;
};
```

### SavepointStatement类

SAVEPOINT语句节点：

```cpp
class SavepointStatement : public Statement {
public:
    std::string savepoint_name;
    
    void accept(AstVisitor& visitor) override;
};
```

### SetTransactionStatement类

SET TRANSACTION语句节点：

```cpp
class SetTransactionStatement : public Statement {
public:
    // 事务隔离级别设置
    
    void accept(AstVisitor& visitor) override;
};
```

## 辅助类

### TableReference类

表引用：

```cpp
class TableReference {
public:
    std::string name;
    std::string alias;
};
```

### ColumnDefinition类

列定义：

```cpp
class ColumnDefinition {
public:
    std::string name;
    DataType data_type;
    std::vector<ColumnConstraint> constraints;
};
```

### TableConstraint类

表级约束基类：

```cpp
class TableConstraint {
public:
    enum Type { PRIMARY_KEY, UNIQUE, FOREIGN_KEY, CHECK };
    
    Type type;
    std::string name;
};
```

### PrimaryKeyConstraint类

主键约束：

```cpp
class PrimaryKeyConstraint : public TableConstraint {
public:
    std::vector<std::string> columns;
};
```

### UniqueConstraint类

唯一约束：

```cpp
class UniqueConstraint : public TableConstraint {
public:
    std::vector<std::string> columns;
};
```

### ForeignKeyConstraint类

外键约束：

```cpp
class ForeignKeyConstraint : public TableConstraint {
public:
    std::vector<std::string> columns;
    std::string referenced_table;
    std::vector<std::string> referenced_columns;
};
```

### CheckConstraint类

检查约束：

```cpp
class CheckConstraint : public TableConstraint {
public:
    std::unique_ptr<Expression> condition;
};
```

### WhereClause类

WHERE子句：

```cpp
class WhereClause {
public:
    std::unique_ptr<Expression> condition;
};
```

### GroupByClause类

GROUP BY子句：

```cpp
class GroupByClause {
public:
    std::vector<std::string> columns;
};
```

### OrderByClause类

ORDER BY子句：

```cpp
class OrderByClause {
public:
    std::vector<std::string> columns;
    std::vector<bool> ascending;
};
```

### JoinClause类

JOIN子句：

```cpp
class JoinClause {
public:
    enum Type { INNER, LEFT, RIGHT, FULL };
    
    Type join_type;
    TableReference table;
    std::unique_ptr<Expression> on_condition;
};
```

### SelectItem类

SELECT项：

```cpp
class SelectItem {
public:
    std::unique_ptr<Expression> expression;
    std::string alias;
};
```

## 表达式类

### BinaryExpression类

二元表达式：

```cpp
class BinaryExpression : public Expression {
public:
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    std::string op;
};
```

### UnaryExpression类

一元表达式：

```cpp
class UnaryExpression : public Expression {
public:
    std::unique_ptr<Expression> operand;
    std::string op;
};
```

### LiteralExpression类

字面量表达式：

```cpp
class LiteralExpression : public Expression {
public:
    enum Type { STRING, NUMBER, BOOLEAN };
    
    Type literal_type;
    std::string value;
};
```

### IdentifierExpression类

标识符表达式：

```cpp
class IdentifierExpression : public Expression {
public:
    std::string name;
};
```

### FunctionCallExpression类

函数调用表达式：

```cpp
class FunctionCallExpression : public Expression {
public:
    std::string function_name;
    std::vector<std::unique_ptr<Expression>> arguments;
};
```

## 主要功能

1. **结构化表示**：将SQL语句转换为树形结构，便于处理
2. **类型安全**：通过继承体系确保不同类型节点的正确使用
3. **可扩展性**：易于添加新的语句类型和表达式类型
4. **访问者模式支持**：支持AstVisitor进行遍历和处理
5. **内存管理**：使用智能指针管理节点生命周期

## 设计特点

1. **继承体系**：通过继承组织不同类型的节点
2. **组合模式**：复杂节点由简单节点组合而成
3. **智能指针**：使用std::unique_ptr管理子节点内存
4. **访问者模式**：支持对AST的遍历和处理
5. **类型安全**：通过类型系统确保节点使用的正确性