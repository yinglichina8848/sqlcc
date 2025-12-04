# SQLCC AST 节点结构设计

本文档定义了SQLCC解析器新的AST（抽象语法树）节点结构设计，采用清晰的层次化架构，支持复杂SQL特性的解析和处理。

## 1. AST 总体架构

### 1.1 核心设计原则

1. **层次化设计**：清晰的继承层次，便于扩展和维护
2. **访问者模式**：统一的操作接口，支持多种处理方式
3. **位置信息保留**：每个节点保留源代码位置，便于错误报告
4. **类型安全**：强类型设计，避免运行时类型错误
5. **内存管理**：智能指针管理，避免内存泄漏

### 1.2 主要层次结构

```
ASTNode (基类)
├── Statement (语句节点)
│   ├── DDLStatement (DDL语句)
│   │   ├── CreateStatement
│   │   ├── AlterStatement
│   │   ├── DropStatement
│   │   └── IndexStatement
│   ├── DMLStatement (DML语句)
│   │   ├── SelectStatement
│   │   ├── InsertStatement
│   │   ├── UpdateStatement
│   │   └── DeleteStatement
│   ├── DCLStatement (权限语句)
│   │   ├── GrantStatement
│   │   └── RevokeStatement
│   └── TCLStatement (事务语句)
│       ├── CommitStatement
│       └── RollbackStatement
├── Expression (表达式节点)
│   ├── LiteralExpression (字面量)
│   ├── ColumnReferenceExpression (列引用)
│   ├── FunctionCallExpression (函数调用)
│   ├── BinaryExpression (二元表达式)
│   ├── UnaryExpression (一元表达式)
│   ├── SubqueryExpression (子查询)
│   └── CaseExpression (CASE表达式)
├── DataType (数据类型节点)
│   ├── PrimitiveType
│   ├── ArrayType
│   ├── MapType
│   └── UserDefinedType
└── Constraint (约束节点)
    ├── PrimaryKeyConstraint
    ├── ForeignKeyConstraint
    ├── UniqueConstraint
    └── CheckConstraint
```

## 2. 核心基类定义

### 2.1 ASTNode 基类

```cpp
class ASTNode {
public:
    // 位置信息
    SourceLocation location_;

    // 构造函数
    ASTNode(const SourceLocation& location = SourceLocation());

    // 虚析构函数
    virtual ~ASTNode() = default;

    // 接受访问者
    virtual void accept(ASTVisitor& visitor) = 0;

    // 获取位置信息
    const SourceLocation& getLocation() const;

    // 克隆接口（用于AST变换）
    virtual std::unique_ptr<ASTNode> clone() const = 0;

    // 调试支持
    virtual std::string toString() const = 0;
};
```

### 2.2 SourceLocation 结构

```cpp
struct SourceLocation {
    size_t line_;      // 行号
    size_t column_;    // 列号
    size_t offset_;    // 字符偏移量
    std::string file_; // 文件名（可选）

    SourceLocation(size_t line = 0, size_t column = 0, size_t offset = 0,
                   const std::string& file = "");

    std::string toString() const;
};
```

## 3. 语句节点层次

### 3.1 Statement 基类

```cpp
class Statement : public ASTNode {
public:
    enum StatementType {
        CREATE,
        SELECT,
        INSERT,
        UPDATE,
        DELETE,
        DROP,
        ALTER,
        GRANT,
        REVOKE,
        COMMIT,
        ROLLBACK,
        USE,
        SHOW
    };

    explicit Statement(StatementType type, const SourceLocation& location = SourceLocation());

    StatementType getStatementType() const;

    void accept(ASTVisitor& visitor) override;

private:
    StatementType type_;
};
```

### 3.2 DDL 语句

#### 3.2.1 CreateStatement

```cpp
class CreateStatement : public Statement {
public:
    enum ObjectType {
        DATABASE,
        TABLE,
        INDEX,
        VIEW,
        TRIGGER,
        FUNCTION,
        PROCEDURE
    };

    CreateStatement(ObjectType objectType,
                   const std::string& objectName,
                   const SourceLocation& location = SourceLocation());

    ObjectType getObjectType() const;
    const std::string& getObjectName() const;

    // 表创建特有方法
    void addColumn(std::unique_ptr<ColumnDefinition> column);
    void addConstraint(std::unique_ptr<TableConstraint> constraint);
    const std::vector<std::unique_ptr<ColumnDefinition>>& getColumns() const;
    const std::vector<std::unique_ptr<TableConstraint>>& getConstraints() const;

    // 索引创建特有方法
    void setTableName(const std::string& tableName);
    void addIndexColumn(std::unique_ptr<IndexColumn> column);
    const std::string& getTableName() const;
    const std::vector<std::unique_ptr<IndexColumn>>& getIndexColumns() const;
    bool isUnique() const;
    void setUnique(bool unique);

    void accept(ASTVisitor& visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
    std::string toString() const override;

private:
    ObjectType objectType_;
    std::string objectName_;

    // 表相关
    std::vector<std::unique_ptr<ColumnDefinition>> columns_;
    std::vector<std::unique_ptr<TableConstraint>> constraints_;

    // 索引相关
    std::string tableName_;
    std::vector<std::unique_ptr<IndexColumn>> indexColumns_;
    bool unique_;
};
```

#### 3.2.2 SelectStatement

```cpp
class SelectStatement : public Statement {
public:
    SelectStatement(const SourceLocation& location = SourceLocation());

    // SELECT 子句
    void setSelectAll(bool selectAll);
    void addSelectItem(std::unique_ptr<SelectItem> item);
    bool isSelectAll() const;
    const std::vector<std::unique_ptr<SelectItem>>& getSelectItems() const;

    // FROM 子句
    void setFromClause(std::unique_ptr<FromClause> fromClause);
    const FromClause* getFromClause() const;

    // WHERE 子句
    void setWhereClause(std::unique_ptr<Expression> whereClause);
    const Expression* getWhereClause() const;

    // GROUP BY 子句
    void addGroupByItem(std::unique_ptr<Expression> item);
    const std::vector<std::unique_ptr<Expression>>& getGroupByItems() const;

    // HAVING 子句
    void setHavingClause(std::unique_ptr<Expression> havingClause);
    const Expression* getHavingClause() const;

    // ORDER BY 子句
    void addOrderByItem(std::unique_ptr<OrderByItem> item);
    const std::vector<std::unique_ptr<OrderByItem>>& getOrderByItems() const;

    // LIMIT/OFFSET 子句
    void setLimit(std::unique_ptr<Expression> limit);
    void setOffset(std::unique_ptr<Expression> offset);
    const Expression* getLimit() const;
    const Expression* getOffset() const;

    // 集合操作
    void addSetOperation(std::unique_ptr<SetOperation> operation);
    const std::vector<std::unique_ptr<SetOperation>>& getSetOperations() const;

    void accept(ASTVisitor& visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
    std::string toString() const override;

private:
    bool selectAll_;
    std::vector<std::unique_ptr<SelectItem>> selectItems_;
    std::unique_ptr<FromClause> fromClause_;
    std::unique_ptr<Expression> whereClause_;
    std::vector<std::unique_ptr<Expression>> groupByItems_;
    std::unique_ptr<Expression> havingClause_;
    std::vector<std::unique_ptr<OrderByItem>> orderByItems_;
    std::unique_ptr<Expression> limit_;
    std::unique_ptr<Expression> offset_;
    std::vector<std::unique_ptr<SetOperation>> setOperations_;
};
```

## 4. 表达式节点层次

### 4.1 Expression 基类

```cpp
class Expression : public ASTNode {
public:
    enum ExpressionType {
        LITERAL,
        COLUMN_REFERENCE,
        FUNCTION_CALL,
        BINARY_OPERATION,
        UNARY_OPERATION,
        SUBQUERY,
        CASE_EXPRESSION,
        CAST_EXPRESSION,
        EXISTS_EXPRESSION,
        IN_EXPRESSION,
        BETWEEN_EXPRESSION
    };

    explicit Expression(ExpressionType type, const SourceLocation& location = SourceLocation());

    ExpressionType getExpressionType() const;

    // 运算符优先级（用于语法分析）
    virtual int getPrecedence() const = 0;

    // 类型检查相关
    virtual DataType getDataType() const = 0;

    void accept(ASTVisitor& visitor) override;

private:
    ExpressionType type_;
};
```

### 4.2 具体表达式类型

#### 4.2.1 LiteralExpression

```cpp
class LiteralExpression : public Expression {
public:
    enum LiteralType {
        NULL_LITERAL,
        BOOLEAN_LITERAL,
        INTEGER_LITERAL,
        FLOAT_LITERAL,
        DECIMAL_LITERAL,
        STRING_LITERAL,
        DATE_LITERAL,
        TIME_LITERAL,
        TIMESTAMP_LITERAL,
        ARRAY_LITERAL,
        OBJECT_LITERAL
    };

    LiteralExpression(LiteralType literalType,
                     const std::string& value,
                     const SourceLocation& location = SourceLocation());

    LiteralType getLiteralType() const;
    const std::string& getValue() const;

    // 获取实际值（类型安全）
    template<typename T>
    T getTypedValue() const;

    int getPrecedence() const override;
    DataType getDataType() const override;

    std::unique_ptr<ASTNode> clone() const override;
    std::string toString() const override;

private:
    LiteralType literalType_;
    std::string value_;
    std::any typedValue_;  // 缓存解析后的值
};
```

#### 4.2.2 BinaryExpression

```cpp
class BinaryExpression : public Expression {
public:
    enum Operator {
        // 算术运算符
        ADD, SUBTRACT, MULTIPLY, DIVIDE, MODULO,

        // 比较运算符
        EQUAL, NOT_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,

        // 逻辑运算符
        AND, OR,

        // 字符串运算符
        CONCAT,

        // 其他运算符
        LIKE, IN, BETWEEN
    };

    BinaryExpression(Operator op,
                    std::unique_ptr<Expression> left,
                    std::unique_ptr<Expression> right,
                    const SourceLocation& location = SourceLocation());

    Operator getOperator() const;
    const Expression* getLeft() const;
    const Expression* getRight() const;

    int getPrecedence() const override;
    DataType getDataType() const override;

    std::unique_ptr<ASTNode> clone() const override;
    std::string toString() const override;

private:
    Operator operator_;
    std::unique_ptr<Expression> left_;
    std::unique_ptr<Expression> right_;
};
```

#### 4.2.3 FunctionCallExpression

```cpp
class FunctionCallExpression : public Expression {
public:
    FunctionCallExpression(const std::string& functionName,
                          const SourceLocation& location = SourceLocation());

    const std::string& getFunctionName() const;
    void addArgument(std::unique_ptr<Expression> argument);
    const std::vector<std::unique_ptr<Expression>>& getArguments() const;

    // 函数类型检查
    bool isAggregateFunction() const;
    bool isWindowFunction() const;

    int getPrecedence() const override;
    DataType getDataType() const override;

    std::unique_ptr<ASTNode> clone() const override;
    std::string toString() const override;

private:
    std::string functionName_;
    std::vector<std::unique_ptr<Expression>> arguments_;
};
```

## 5. 数据类型和约束节点

### 5.1 DataType 节点

```cpp
class DataType : public ASTNode {
public:
    enum TypeCategory {
        PRIMITIVE,
        ARRAY,
        MAP,
        USER_DEFINED,
        DECIMAL
    };

    enum PrimitiveType {
        INTEGER,
        FLOAT,
        DOUBLE,
        BOOLEAN,
        STRING,
        DATE,
        TIME,
        TIMESTAMP,
        BLOB,
        TEXT
    };

    DataType(TypeCategory category, const SourceLocation& location = SourceLocation());

    TypeCategory getCategory() const;

    // 原始类型方法
    void setPrimitiveType(PrimitiveType type);
    PrimitiveType getPrimitiveType() const;

    // 数组类型方法
    void setElementType(std::unique_ptr<DataType> elementType);
    const DataType* getElementType() const;

    // 字符串类型方法
    void setLength(int length);
    int getLength() const;

    // 十进制类型方法
    void setPrecision(int precision);
    void setScale(int scale);
    int getPrecision() const;
    int getScale() const;

    void accept(ASTVisitor& visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
    std::string toString() const override;

private:
    TypeCategory category_;
    PrimitiveType primitiveType_;
    std::unique_ptr<DataType> elementType_;
    int length_;
    int precision_;
    int scale_;
};
```

### 5.2 约束节点

```cpp
class Constraint : public ASTNode {
public:
    enum ConstraintType {
        PRIMARY_KEY,
        FOREIGN_KEY,
        UNIQUE,
        CHECK,
        NOT_NULL,
        DEFAULT,
        AUTO_INCREMENT
    };

    Constraint(ConstraintType type,
              const std::string& name = "",
              const SourceLocation& location = SourceLocation());

    ConstraintType getConstraintType() const;
    const std::string& getConstraintName() const;

    void accept(ASTVisitor& visitor) override;

private:
    ConstraintType type_;
    std::string name_;
};
```

## 6. 访问者模式

### 6.1 ASTVisitor 基类

```cpp
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    // 语句访问者
    virtual void visit(Statement& node) = 0;
    virtual void visit(CreateStatement& node) = 0;
    virtual void visit(SelectStatement& node) = 0;
    virtual void visit(InsertStatement& node) = 0;
    virtual void visit(UpdateStatement& node) = 0;
    virtual void visit(DeleteStatement& node) = 0;

    // 表达式访问者
    virtual void visit(Expression& node) = 0;
    virtual void visit(LiteralExpression& node) = 0;
    virtual void visit(BinaryExpression& node) = 0;
    virtual void visit(FunctionCallExpression& node) = 0;

    // 数据类型访问者
    virtual void visit(DataType& node) = 0;

    // 约束访问者
    virtual void visit(Constraint& node) = 0;
};
```

### 6.2 具体访问者示例

```cpp
// 语义分析访问者
class SemanticAnalyzer : public ASTVisitor {
public:
    void analyze(std::unique_ptr<ASTNode>& ast);

    // 实现所有visit方法...
    void visit(Statement& node) override;
    void visit(CreateStatement& node) override;
    // ...
};

// 代码生成访问者
class CodeGenerator : public ASTVisitor {
public:
    std::string generate(std::unique_ptr<ASTNode>& ast);

    // 实现所有visit方法...
    void visit(Statement& node) override;
    void visit(CreateStatement& node) override;
    // ...
};
```

## 7. 辅助结构

### 7.1 SelectItem

```cpp
class SelectItem : public ASTNode {
public:
    SelectItem(std::unique_ptr<Expression> expression,
              const std::string& alias = "",
              const SourceLocation& location = SourceLocation());

    const Expression* getExpression() const;
    const std::string& getAlias() const;
    bool hasAlias() const;

    void accept(ASTVisitor& visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
    std::string toString() const override;

private:
    std::unique_ptr<Expression> expression_;
    std::string alias_;
};
```

### 7.2 FromClause

```cpp
class FromClause : public ASTNode {
public:
    FromClause(std::unique_ptr<TableReference> table,
              const SourceLocation& location = SourceLocation());

    void addJoin(std::unique_ptr<JoinClause> join);
    const TableReference* getTable() const;
    const std::vector<std::unique_ptr<JoinClause>>& getJoins() const;

    void accept(ASTVisitor& visitor) override;
    std::unique_ptr<ASTNode> clone() const override;
    std::string toString() const override;

private:
    std::unique_ptr<TableReference> table_;
    std::vector<std::unique_ptr<JoinClause>> joins_;
};
```

## 8. 实现注意事项

### 8.1 内存管理
- 使用智能指针管理所有AST节点
- 避免循环引用（使用弱引用或父指针）
- 实现深拷贝克隆方法

### 8.2 位置信息
- 每个节点必须保留完整的源代码位置信息
- 支持位置信息的合并和比较
- 便于错误报告和调试

### 8.3 类型系统
- 实现完整的数据类型系统
- 支持类型检查和类型推导
- 提供类型转换机制

### 8.4 扩展性
- 设计插件架构支持新的语句类型
- 支持自定义函数和操作符
- 便于添加新的SQL方言支持

---

*本文档定义了SQLCC解析器新的AST节点结构，为后续的语义分析、代码生成和优化奠定基础。*
