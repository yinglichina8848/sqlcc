# SQL解析模块设计文档

## 1. 总体设计

### 1.1 设计目标

设计并实现一个完整的SQL解析模块，能够解析常用的SQL语句（包括数据库操作、表操作、数据CRUD操作等），并生成正确的抽象语法树(AST)。解析模块应具有良好的可扩展性，支持后续添加新的SQL语句类型和功能。

### 1.2 模块架构

SQL解析模块采用经典的编译器前端架构，主要包含以下核心组件：

1. **词法分析器(Lexer/Scanner)** - 负责将SQL文本分解为标记(Token)流
2. **语法分析器(Parser)** - 负责将Token流构建为抽象语法树(AST)
3. **抽象语法树(AST)** - 表示SQL语句的结构化表示
4. **Token管理器** - 定义和管理SQL语言中的所有标记类型
5. **AST节点访问器** - 提供AST遍历和操作接口

### 1.3 数据流向

```
SQL文本 -> 词法分析器 -> Token流 -> 语法分析器 -> 抽象语法树(AST)
```

### 1.4 支持的SQL语句类型

1. **数据库操作语句**
   - CREATE DATABASE
   - ALTER DATABASE
   - DROP DATABASE
   - USE DATABASE

2. **表操作语句**
   - CREATE TABLE
   - ALTER TABLE
   - DROP TABLE
   - TRUNCATE TABLE

3. **数据查询语句**
   - SELECT (包含WHERE, JOIN, GROUP BY, HAVING, ORDER BY等子句)

4. **数据操作语句**
   - INSERT
   - UPDATE
   - DELETE

## 2. 详细设计

### 2.1 词法分析器(Lexer/Scanner)

#### 2.1.1 设计思路

词法分析器负责将输入的SQL文本转换为标记(Token)序列。它通过扫描输入字符流，根据预定义的规则识别关键字、标识符、字面量、运算符和标点符号等。

#### 2.1.2 核心类设计

```cpp
class Token {
public:
    enum Type {
        // 关键字
        KEYWORD_CREATE, KEYWORD_SELECT, KEYWORD_INSERT,
        KEYWORD_UPDATE, KEYWORD_DELETE, KEYWORD_DROP,
        KEYWORD_ALTER, KEYWORD_USE, KEYWORD_DATABASE,
        KEYWORD_TABLE, KEYWORD_WHERE, KEYWORD_JOIN,
        KEYWORD_ON, KEYWORD_GROUP, KEYWORD_BY,
        KEYWORD_HAVING, KEYWORD_ORDER, KEYWORD_INTO,
        KEYWORD_VALUES, KEYWORD_SET,
        
        // 标识符
        IDENTIFIER,
        
        // 字面量
        STRING_LITERAL,
        NUMERIC_LITERAL,
        
        // 运算符
        OPERATOR_EQUAL, OPERATOR_NOT_EQUAL,
        OPERATOR_LESS, OPERATOR_LESS_EQUAL,
        OPERATOR_GREATER, OPERATOR_GREATER_EQUAL,
        OPERATOR_PLUS, OPERATOR_MINUS,
        OPERATOR_MULTIPLY, OPERATOR_DIVIDE,
        OPERATOR_AND, OPERATOR_OR, OPERATOR_NOT,
        OPERATOR_LIKE, OPERATOR_IN,
        
        // 标点符号
        PUNCTUATION_LEFT_PAREN, PUNCTUATION_RIGHT_PAREN,
        PUNCTUATION_COMMA, PUNCTUATION_SEMICOLON,
        PUNCTUATION_DOT,
        
        // 其他
        END_OF_INPUT,
        INVALID_TOKEN
    };
    
    Token(Type type, const std::string& lexeme, int line, int column);
    
    Type getType() const;
    const std::string& getLexeme() const;
    int getLine() const;
    int getColumn() const;
    
    std::string getTypeName() const;
    std::string toString() const;
    
private:
    Type type_;
    std::string lexeme_;
    int line_;
    int column_;
};

class Lexer {
public:
    Lexer(const std::string& sql);
    
    Token getNextToken();
    Token peekToken();
    
    void setPosition(int position);
    int getPosition() const;
    
private:
    char advance();
    char peek() const;
    bool isAtEnd() const;
    
    Token scanIdentifierOrKeyword();
    Token scanStringLiteral();
    Token scanNumericLiteral();
    Token scanOperator();
    
    void skipWhitespace();
    void skipComment();
    
    std::string sql_;
    int position_;
    int line_;
    int column_;
    std::unordered_map<std::string, Token::Type> keywords_;
};
```

#### 2.1.3 工作流程

1. 初始化词法分析器，设置SQL文本和初始位置
2. 循环调用`getNextToken()`直到遇到`END_OF_INPUT`或错误
3. 对于每个字符：
   - 跳过空白字符和注释
   - 根据当前字符决定扫描的Token类型
   - 扫描完整的Token并返回

### 2.2 语法分析器(Parser)

#### 2.2.1 设计思路

语法分析器使用递归下降法实现，根据SQL语法规则，将Token流构建为抽象语法树。它会检查语法正确性，并处理操作符优先级、表达式嵌套等复杂情况。

#### 2.2.2 核心类设计

```cpp
class Parser {
public:
    Parser(Lexer& lexer);
    
    std::unique_ptr<Statement> parseStatement();
    
private:
    // 语句级解析方法
    std::unique_ptr<Statement> parseCreateStatement();
    std::unique_ptr<Statement> parseSelectStatement();
    std::unique_ptr<Statement> parseInsertStatement();
    std::unique_ptr<Statement> parseUpdateStatement();
    std::unique_ptr<Statement> parseDeleteStatement();
    std::unique_ptr<Statement> parseDropStatement();
    std::unique_ptr<Statement> parseAlterStatement();
    std::unique_ptr<Statement> parseUseStatement();
    
    // 表达式解析方法
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseTerm();
    std::unique_ptr<Expression> parseFactor();
    std::unique_ptr<Expression> parsePrimary();
    
    // 子句解析方法
    std::unique_ptr<WhereClause> parseWhereClause();
    std::unique_ptr<JoinClause> parseJoinClause();
    std::unique_ptr<GroupByClause> parseGroupByClause();
    std::unique_ptr<OrderByClause> parseOrderByClause();
    
    // 辅助方法
    Token consume(Token::Type expected, const std::string& errorMsg);
    bool match(const std::vector<Token::Type>& types);
    bool check(Token::Type type) const;
    Token advance();
    Token peek() const;
    bool isAtEnd() const;
    
    std::runtime_error error(const Token& token, const std::string& message);
    
    Lexer& lexer_;
    Token current_;
};
```

#### 2.2.3 工作流程

1. 初始化语法分析器，关联词法分析器
2. 调用`parseStatement()`开始解析
3. 根据当前Token确定语句类型
4. 递归调用相应的解析方法构建AST
5. 返回完整的语句AST节点

### 2.3 抽象语法树(AST)

#### 2.3.1 设计思路

AST采用节点类层次结构，通过继承实现不同类型的节点。使用访问者模式支持AST的遍历和操作。

#### 2.3.2 核心类设计

```cpp
// 基类
class Node {
public:
    virtual ~Node() = default;
    virtual void accept(NodeVisitor& visitor) = 0;
};

// 访问者接口
class NodeVisitor {
public:
    virtual ~NodeVisitor() = default;
    
    // 语句访问方法
    virtual void visit(CreateStatement& node) = 0;
    virtual void visit(SelectStatement& node) = 0;
    virtual void visit(InsertStatement& node) = 0;
    virtual void visit(UpdateStatement& node) = 0;
    virtual void visit(DeleteStatement& node) = 0;
    virtual void visit(DropStatement& node) = 0;
    virtual void visit(AlterStatement& node) = 0;
    virtual void visit(UseStatement& node) = 0;
    
    // 表达式访问方法
    virtual void visit(IdentifierExpression& node) = 0;
    virtual void visit(StringLiteralExpression& node) = 0;
    virtual void visit(NumericLiteralExpression& node) = 0;
    virtual void visit(BinaryExpression& node) = 0;
    virtual void visit(UnaryExpression& node) = 0;
    virtual void visit(FunctionExpression& node) = 0;
    
    // 子句访问方法
    virtual void visit(WhereClause& node) = 0;
    virtual void visit(JoinClause& node) = 0;
    virtual void visit(GroupByClause& node) = 0;
    virtual void visit(OrderByClause& node) = 0;
};

// 语句节点基类
class Statement : public Node {
public:
    enum Type {
        CREATE,
        SELECT,
        INSERT,
        UPDATE,
        DELETE,
        DROP,
        ALTER,
        USE
    };
    
    virtual Type getType() const = 0;
};

// 表达式节点基类
class Expression : public Node {
public:
    enum Type {
        IDENTIFIER,
        STRING_LITERAL,
        NUMERIC_LITERAL,
        BINARY,
        UNARY,
        FUNCTION
    };
    
    virtual Type getType() const = 0;
};

// 具体节点类示例
class CreateStatement : public Statement {
public:
    enum Target {
        DATABASE,
        TABLE
    };
    
    CreateStatement(Target target);
    
    Type getType() const override { return CREATE; }
    void accept(NodeVisitor& visitor) override { visitor.visit(*this); }
    
    void setDatabaseName(const std::string& name);
    void setTableName(const std::string& name);
    void addColumn(const ColumnDefinition& column);
    
    Target getTarget() const;
    const std::string& getDatabaseName() const;
    const std::string& getTableName() const;
    const std::vector<ColumnDefinition>& getColumns() const;
    
private:
    Target target_;
    std::string databaseName_;
    std::string tableName_;
    std::vector<ColumnDefinition> columns_;
};

// 其他具体节点类（SelectStatement, InsertStatement等）的定义类似
```

#### 2.3.3 AST节点层次结构

```
Node
├── Statement
│   ├── CreateStatement
│   ├── SelectStatement
│   ├── InsertStatement
│   ├── UpdateStatement
│   ├── DeleteStatement
│   ├── DropStatement
│   ├── AlterStatement
│   └── UseStatement
└── Expression
    ├── IdentifierExpression
    ├── StringLiteralExpression
    ├── NumericLiteralExpression
    ├── BinaryExpression
    ├── UnaryExpression
    └── FunctionExpression
```

### 2.4 支持的SQL语法规则示例

#### 2.4.1 SELECT语句语法

```
SELECT [DISTINCT] column_list
FROM table_list
[JOIN table ON condition]
[WHERE condition]
[GROUP BY column_list]
[HAVING condition]
[ORDER BY column_list [ASC|DESC]]
```

#### 2.4.2 INSERT语句语法

```
INSERT INTO table_name [(column_list)]
VALUES (value_list)
```

#### 2.4.3 UPDATE语句语法

```
UPDATE table_name
SET column_name = value [, column_name = value ...]
[WHERE condition]
```

#### 2.4.4 DELETE语句语法

```
DELETE FROM table_name
[WHERE condition]
```

#### 2.4.5 CREATE TABLE语句语法

```
CREATE TABLE table_name (
    column_name data_type [constraint],
    ...
    [PRIMARY KEY (column_list)]
)
```

## 3. 实现细节

### 3.1 错误处理

解析过程中的错误处理策略：

1. 词法错误：记录错误位置和详细信息，返回`INVALID_TOKEN`
2. 语法错误：抛出详细的`std::runtime_error`异常，包含错误位置和预期的Token类型
3. 错误恢复：提供基本的错误恢复机制，允许在简单错误后继续解析

### 3.2 性能优化考虑

1. 使用预分配内存的容器减少内存分配开销
2. 使用`std::string_view`避免不必要的字符串复制
3. 关键字查找使用哈希表提高效率
4. 对于大型SQL语句，实现流式处理支持

### 3.3 扩展性设计

1. 模块化的设计允许轻松添加新的SQL语句类型
2. 访问者模式使得可以方便地添加新的AST处理逻辑
3. 使用工厂模式创建不同类型的AST节点

## 4. 测试策略

### 4.1 单元测试

为每个组件编写独立的单元测试：

1. **词法分析器测试**：验证能否正确识别各种Token类型
2. **语法分析器测试**：验证能否正确解析各种SQL语句
3. **AST测试**：验证生成的AST结构是否符合预期

### 4.2 集成测试

测试完整的SQL解析流程：

1. 测试各种复杂SQL语句的解析
2. 测试错误情况下的行为
3. 测试边界情况

### 4.3 性能测试

测试解析器在不同规模SQL语句下的性能表现。

## 5. 后续优化计划

1. 添加更高级的SQL特性支持，如子查询、视图、触发器等
2. 实现SQL语句验证功能，检查语法和语义的正确性
3. 优化解析器性能，支持大规模SQL语句的高效解析
4. 添加SQL格式化和重构功能

## 6. 代码规范

遵循项目现有的C++代码规范，包括：

1. 使用智能指针管理动态内存
2. 遵循RAII原则
3. 使用适当的异常处理机制
4. 编写详细的代码注释和文档
5. 确保代码风格一致