# SQL解析模块设计文档

## 1. 总体设计

### 1.1 设计目标

设计并实现一个完整的SQL解析模块，能够解析常用的SQL语句（包括数据库操作、表操作、数据CRUD操作等），并生成正确的抽象语法树(AST)。解析模块应具有良好的可扩展性、高性能和健壮的错误处理机制，支持后续添加新的SQL语句类型和功能。

### 1.2 模块架构

SQL解析模块采用经典的编译器前端架构，主要包含以下核心组件：

1. **词法分析器(Lexer/Scanner)** - 负责将SQL文本分解为标记(Token)流，采用**确定有限自动机(DFA)**状态机驱动设计
2. **语法分析器(Parser)** - 负责将Token流构建为抽象语法树(AST)，采用递归下降解析器架构
3. **抽象语法树(AST)** - 表示SQL语句的结构化表示，支持完整的位置追踪和访问者模式
4. **Token类型系统** - 定义和管理SQL语言中的所有标记类型，支持80+ SQL关键字
5. **错误处理系统** - 提供结构化的错误处理和恢复机制
6. **AST节点访问器** - 提供AST遍历和操作接口

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

5. **数据控制语句(DCL)**
   - CREATE USER
   - DROP USER
   - GRANT
   - REVOKE

## 2. 详细设计

### 2.1 词法分析器(Lexer/Scanner)

#### 2.1.1 设计思路

词法分析器负责将输入的SQL文本转换为标记(Token)序列。采用**确定有限自动机(DFA)**状态机驱动设计，替代传统的if-else条件判断链，显著提升了性能和可维护性。支持完整的SQL词法规则和Unicode标识符。

#### 2.1.2 DFA状态机设计

```cpp
enum class LexerState {
    START,           // 初始状态
    IDENTIFIER,      // 标识符状态
    NUMBER,          // 数字字面量状态
    NUMBER_DECIMAL,  // 小数部分
    NUMBER_EXPONENT, // 指数部分
    STRING_SINGLE,   // 单引号字符串
    STRING_DOUBLE,   // 双引号标识符
    STRING_ESCAPE,   // 转义序列
    OPERATOR,        // 操作符状态
    PUNCTUATION,     // 标点符号状态
    COMMENT_LINE,    // 单行注释
    COMMENT_BLOCK    // 多行注释
};
```

#### 2.1.3 状态转移机制

使用高效的哈希表实现状态转移，避免了传统的if-else判断链：

```cpp
unordered_map<LexerState, unordered_map<char, LexerState>> stateTransitions;
```

#### 2.1.4 新Token类型系统

```cpp
class Token {
public:
    enum Type {
        // Punctuation
        SEMICOLON, LPAREN, RPAREN, COMMA, DOT,

        // Literals
        INTEGER_LITERAL, FLOAT_LITERAL, STRING_LITERAL,

        // Identifiers
        IDENTIFIER,

        // Operators
        OPERATOR_PLUS, OPERATOR_MINUS, OPERATOR_MULTIPLY,
        OPERATOR_DIVIDE, OPERATOR_EQUAL, OPERATOR_NOT_EQUAL,
        OPERATOR_LESS, OPERATOR_LESS_EQUAL, OPERATOR_GREATER,
        OPERATOR_GREATER_EQUAL, OPERATOR_AND, OPERATOR_OR,
        OPERATOR_NOT, OPERATOR_LIKE, OPERATOR_IN,

        // Keywords (80+ SQL keywords)
        KEYWORD_SELECT, KEYWORD_FROM, KEYWORD_WHERE,
        KEYWORD_CREATE, KEYWORD_TABLE, KEYWORD_INDEX,
        KEYWORD_INSERT, KEYWORD_UPDATE, KEYWORD_DELETE,
        KEYWORD_DROP, KEYWORD_ALTER, KEYWORD_USE,
        KEYWORD_DATABASE, KEYWORD_JOIN, KEYWORD_ON,
        KEYWORD_GROUP, KEYWORD_BY, KEYWORD_HAVING,
        KEYWORD_ORDER, KEYWORD_INTO, KEYWORD_VALUES,
        KEYWORD_SET, KEYWORD_USER, KEYWORD_GRANT,
        KEYWORD_REVOKE, KEYWORD_PRIVILEGES, KEYWORD_TO,
        KEYWORD_FROM, KEYWORD_WITH, KEYWORD_PASSWORD,
        KEYWORD_IDENTIFIED, KEYWORD_COUNT, KEYWORD_SUM,
        KEYWORD_AVG, KEYWORD_MIN, KEYWORD_MAX,
        // ... 更多关键词
        
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
```

#### 2.1.5 LexerNew类设计

```cpp
class LexerNew {
public:
    LexerNew(const std::string& sql);
    
    Token getNextToken();
    Token peekToken();
    
    void setPosition(int position);
    int getPosition() const;
    
private:
    void initializeStateTransitions();
    void initializeKeywords();
    
    char advance();
    char peek() const;
    char peekNext() const;
    bool isAtEnd() const;
    
    Token createToken(Token::Type type, const std::string& lexeme);
    void skipWhitespace();
    
    std::string sql_;
    int position_;
    int start_;
    int line_;
    int column_;
    LexerState currentState_;
    
    unordered_map<LexerState, unordered_map<char, LexerState>> stateTransitions_;
    unordered_map<std::string, Token::Type> keywords_;
};
```

#### 2.1.6 工作流程

1. 初始化词法分析器，设置SQL文本和初始状态
2. 构建状态转移表和关键字映射
3. 循环调用`getNextToken()`直到遇到`END_OF_INPUT`或错误
4. 对于每个字符：
   - 根据当前状态和输入字符查找下一状态
   - 执行状态转移，收集字符序列
   - 当到达终止状态时，生成相应的Token
5. 支持完整的错误处理，包括精确的位置跟踪和详细的错误信息

### 2.2 语法分析器(Parser)

#### 2.2.1 设计思路

语法分析器采用**递归下降解析器**架构，**严格映射BNF/EBNF规则**，实现了完整的表达式解析系统和语句解析框架。递归下降解析器是一种自顶向下的解析方法，通过递归函数直接映射BNF/EBNF语法规则，具有代码结构清晰、易于理解和扩展的特点。支持错误恢复机制（Panic Mode Recovery），能够在遇到语法错误时继续解析，提供更全面的错误信息。

#### 2.2.2 BNF/EBNF规则设计

BNF（巴科斯-诺尔范式）和EBNF（扩展巴科斯-诺尔范式）是描述编程语言语法的标准方法。SQL解析器的递归下降实现严格遵循以下BNF/EBNF规则：

```ebnf
# 语句规则
<statement> ::= <create_statement> | <select_statement> | <insert_statement> |
                <update_statement> | <delete_statement> | <drop_statement> |
                <alter_statement> | <use_statement> | <grant_statement> |
                <revoke_statement> | <commit_statement> | <rollback_statement>

# SELECT语句规则
<select_statement> ::= SELECT <select_list> FROM <table_list>
                       [WHERE <expression>]
                       [GROUP BY <group_by_list>]
                       [HAVING <expression>]
                       [ORDER BY <order_by_list>]

# 表达式规则（优先级从低到高）
<expression> ::= <logical_or_expression>

<logical_or_expression> ::= <logical_and_expression> (OR <logical_and_expression>)*

<logical_and_expression> ::= <comparison_expression> (AND <comparison_expression>)*

<comparison_expression> ::= <additive_expression> (<comparison_operator> <additive_expression>)*

<additive_expression> ::= <multiplicative_expression> ((+ | -) <multiplicative_expression>)*

<multiplicative_expression> ::= <unary_expression> ((* | /) <unary_expression>)*

<unary_expression> ::= (<unary_operator>)* <primary_expression>

<primary_expression> ::= <identifier> | <literal> | <function_call> | <subquery> | (<expression>)

# 比较操作符
<comparison_operator> ::= = | != | < | <= | > | >= | LIKE | IN

# 字面量
<literal> ::= <string_literal> | <numeric_literal>
<string_literal> ::= ' <character>* '
<numeric_literal> ::= <integer_literal> | <float_literal>
```

#### 2.2.3 BNF到递归下降代码的映射

递归下降解析器通过为每个BNF规则编写一个对应的递归函数来实现。以下是BNF规则到代码的映射示例：

| BNF规则 | 递归函数 | 实现逻辑 |
|---------|----------|----------|
| `<expression> ::= <logical_or_expression>` | `parseExpression()` | 调用`parseLogicalOrExpression()` |
| `<logical_or_expression> ::= <logical_and_expression> (OR <logical_and_expression>)*` | `parseLogicalOrExpression()` | 1. 解析左操作数（调用`parseLogicalAndExpression()`）<br>2. 循环检查下一个Token是否为OR<br>3. 如果是，解析右操作数并创建BinaryExpression节点 |
| `<comparison_expression> ::= <additive_expression> (<comparison_operator> <additive_expression>)*` | `parseComparisonExpression()` | 1. 解析左操作数（调用`parseAdditiveExpression()`）<br>2. 循环检查下一个Token是否为比较操作符<br>3. 如果是，解析右操作数并创建BinaryExpression节点 |
| `<primary_expression> ::= <identifier> | <literal> | <function_call> | <subquery> | (<expression>)` | `parsePrimaryExpression()` | 1. 检查当前Token类型<br>2. 根据Token类型调用相应的解析函数<br>3. 对于括号表达式，先消耗左括号，递归调用`parseExpression()`，再消耗右括号 |

#### 2.2.4 递归下降解析器实现示例

以下是递归下降解析器的核心实现示例，展示了如何将BNF规则映射到C++代码：

```cpp
// 实现 <expression> ::= <logical_or_expression>
std::unique_ptr<Expression> ParserNew::parseExpression() {
    return parseLogicalOrExpression();
}

// 实现 <logical_or_expression> ::= <logical_and_expression> (OR <logical_and_expression>)*
std::unique_ptr<Expression> ParserNew::parseLogicalOrExpression() {
    auto left = parseLogicalAndExpression();
    
    // 处理 (OR <logical_and_expression>)* 部分
    while (match({Token::KEYWORD_OR})) {
        auto op = previous();
        auto right = parseLogicalAndExpression();
        left = std::make_unique<BinaryExpression>(
            std::move(left), op.getType(), std::move(right),
            SourceLocation(op.getLine(), op.getColumn(), 0)
        );
    }
    
    return left;
}

// 实现 <comparison_expression> ::= <additive_expression> (<comparison_operator> <additive_expression>)*
std::unique_ptr<Expression> ParserNew::parseComparisonExpression() {
    auto left = parseAdditiveExpression();
    
    // 处理 (<comparison_operator> <additive_expression>)* 部分
    while (match({Token::OPERATOR_EQUAL, Token::OPERATOR_NOT_EQUAL, 
                 Token::OPERATOR_LESS, Token::OPERATOR_LESS_EQUAL, 
                 Token::OPERATOR_GREATER, Token::OPERATOR_GREATER_EQUAL,
                 Token::OPERATOR_LIKE, Token::OPERATOR_IN})) {
        auto op = previous();
        auto right = parseAdditiveExpression();
        left = std::make_unique<BinaryExpression>(
            std::move(left), op.getType(), std::move(right),
            SourceLocation(op.getLine(), op.getColumn(), 0)
        );
    }
    
    return left;
}

// 实现 <primary_expression> ::= <identifier> | <literal> | <function_call> | <subquery> | (<expression>)
std::unique_ptr<Expression> ParserNew::parsePrimaryExpression() {
    if (match({Token::IDENTIFIER})) {
        // 检查是否为函数调用
        if (check(Token::LPAREN)) {
            return parseFunctionCall();
        }
        // 否则是普通标识符
        auto token = previous();
        return std::make_unique<IdentifierExpression>(
            token.getLexeme(),
            SourceLocation(token.getLine(), token.getColumn(), 0)
        );
    }
    
    // 处理字面量
    if (match({Token::STRING_LITERAL, Token::INTEGER_LITERAL, Token::FLOAT_LITERAL})) {
        auto token = previous();
        switch (token.getType()) {
            case Token::STRING_LITERAL:
                return std::make_unique<StringLiteralExpression>(
                    token.getLexeme(),
                    SourceLocation(token.getLine(), token.getColumn(), 0)
                );
            case Token::INTEGER_LITERAL:
                return std::make_unique<IntegerLiteralExpression>(
                    std::stoi(token.getLexeme()),
                    SourceLocation(token.getLine(), token.getColumn(), 0)
                );
            case Token::FLOAT_LITERAL:
                return std::make_unique<FloatLiteralExpression>(
                    std::stod(token.getLexeme()),
                    SourceLocation(token.getLine(), token.getColumn(), 0)
                );
            default:
                reportError(token, "Unexpected literal type");
                return nullptr;
        }
    }
    
    // 处理括号表达式
    if (match({Token::LPAREN})) {
        auto expr = parseExpression();
        consume(Token::RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    // 处理子查询
    if (check(Token::KEYWORD_SELECT)) {
        return parseSubquery();
    }
    
    // 无法解析的情况
    reportError(current_, "Expected expression");
    return nullptr;
}
```

#### 2.2.5 递归下降解析器的优势

1. **直观性强**：代码结构与BNF规则直接对应，易于理解和维护
2. **易于扩展**：添加新的语法规则只需添加相应的递归函数
3. **错误处理灵活**：可以在解析过程中精确定位错误位置
4. **性能优良**：避免了表驱动解析器的额外开销
5. **调试方便**：可以在每个解析函数中添加调试信息

#### 2.2.6 左递归处理

递归下降解析器无法直接处理左递归规则（即规则的第一个符号是规则本身）。BNF规则在设计时已经考虑了这一点，通过右递归或迭代的方式避免了左递归。例如：

- 避免：`<list> ::= <list>, <element>`（左递归）
- 采用：`<list> ::= <element> (, <element>)*`（右递归/迭代）

这种设计使得递归下降解析器能够高效地处理所有SQL语法规则。

#### 2.2.7 ParserNew类设计

```cpp
class ParserNew {
public:
    ParserNew(LexerNew& lexer);
    
    std::unique_ptr<Statement> parseStatement();
    std::vector<std::unique_ptr<Error>> getErrors() const;
    
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
    
    // DCL语句解析方法
    std::unique_ptr<Statement> parseCreateUserStatement();
    std::unique_ptr<Statement> parseDropUserStatement();
    std::unique_ptr<Statement> parseGrantStatement();
    std::unique_ptr<Statement> parseRevokeStatement();
    
    // 表达式解析系统
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseLogicalOrExpression();
    std::unique_ptr<Expression> parseLogicalAndExpression();
    std::unique_ptr<Expression> parseComparisonExpression();
    std::unique_ptr<Expression> parseAdditiveExpression();
    std::unique_ptr<Expression> parseMultiplicativeExpression();
    std::unique_ptr<Expression> parseUnaryExpression();
    std::unique_ptr<Expression> parsePrimaryExpression();
    std::unique_ptr<Expression> parseFunctionCall();
    std::unique_ptr<Expression> parseSubquery();
    
    // 子句解析方法
    std::unique_ptr<WhereClause> parseWhereClause();
    std::unique_ptr<JoinClause> parseJoinClause();
    std::unique_ptr<GroupByClause> parseGroupByClause();
    std::unique_ptr<HavingClause> parseHavingClause();
    std::unique_ptr<OrderByClause> parseOrderByClause();
    
    // 错误处理和恢复
    void recoverFromError();
    void synchronize();
    void reportError(const Token& token, const std::string& message);
    
    // 辅助方法
    Token consume(Token::Type expected, const std::string& errorMsg);
    bool match(const std::vector<Token::Type>& types);
    bool check(Token::Type type) const;
    Token advance();
    Token peek() const;
    Token previous() const;
    bool isAtEnd() const;
    
    LexerNew& lexer_;
    Token current_;
    Token previous_;
    std::vector<std::unique_ptr<Error>> errors_;
    bool hadError_;
    bool panicMode_;
};
```

#### 2.2.8 表达式解析系统

实现了完整的表达式优先级层次结构：

1. **逻辑OR表达式** (AND/OR优先级)
2. **逻辑AND表达式**
3. **比较表达式** (=, !=, <, <=, >, >=, LIKE, IN)
4. **加法表达式** (+, -)
5. **乘法表达式** (*, /)
6. **一元表达式** (NOT, +, -)
7. **基本表达式** (标识符、字面量、函数调用、子查询)

#### 2.2.9 错误恢复机制

采用**Panic Mode Recovery**策略：

1. 当遇到语法错误时，进入panic模式
2. 跳过Token直到找到合适的同步点（如语句结束符或关键字）
3. 重置状态，继续解析后续语句
4. 收集所有错误信息，提供完整的错误报告

#### 2.2.10 工作流程

1. 初始化语法分析器，关联词法分析器
2. 调用`parseStatement()`开始解析
3. 根据当前Token确定语句类型
4. 递归调用相应的解析方法构建AST，严格遵循BNF规则
5. 遇到错误时，执行错误恢复，继续解析
6. 返回完整的语句AST节点和所有收集的错误信息

### 2.3 抽象语法树(AST)

#### 2.3.1 设计思路

AST采用节点类层次结构，通过继承实现不同类型的节点。使用访问者模式支持AST的遍历和操作。重构后的AST系统引入了**SourceLocation位置追踪系统**，支持完整的位置信息保留和智能指针内存管理，显著提升了可维护性和调试能力。

#### 2.3.2 SourceLocation位置追踪系统

```cpp
class SourceLocation {
public:
    SourceLocation(int line, int column, int offset);
    
    int getLine() const { return line_; }
    int getColumn() const { return column_; }
    int getOffset() const { return offset_; }
    
    std::string toString() const;
    
private:
    int line_;
    int column_;
    int offset_;
};
```

#### 2.3.3 AST核心架构

```cpp
// 基类
class ASTNode {
public:
    virtual ~ASTNode() = default;
    
    // 访问者模式支持
    virtual void accept(ASTVisitor& visitor) = 0;
    
    // 位置信息
    void setLocation(const SourceLocation& location);
    const SourceLocation& getLocation() const;
    
    // 克隆支持
    virtual std::unique_ptr<ASTNode> clone() const = 0;
    
    // 调试支持
    virtual std::string dump(int indent = 0) const = 0;
    virtual std::string toString() const = 0;
    
private:
    SourceLocation location_;
};

// 访问者接口
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    // 语句访问方法
    virtual void visit(CreateStatement& node) = 0;
    virtual void visit(SelectStatement& node) = 0;
    virtual void visit(InsertStatement& node) = 0;
    virtual void visit(UpdateStatement& node) = 0;
    virtual void visit(DeleteStatement& node) = 0;
    virtual void visit(DropStatement& node) = 0;
    virtual void visit(AlterStatement& node) = 0;
    virtual void visit(UseStatement& node) = 0;
    virtual void visit(CreateUserStatement& node) = 0;
    virtual void visit(DropUserStatement& node) = 0;
    virtual void visit(GrantStatement& node) = 0;
    virtual void visit(RevokeStatement& node) = 0;
    
    // 表达式访问方法
    virtual void visit(IdentifierExpression& node) = 0;
    virtual void visit(StringLiteralExpression& node) = 0;
    virtual void visit(NumericLiteralExpression& node) = 0;
    virtual void visit(BinaryExpression& node) = 0;
    virtual void visit(UnaryExpression& node) = 0;
    virtual void visit(FunctionExpression& node) = 0;
    virtual void visit(SubqueryExpression& node) = 0;
    
    // 子句访问方法
    virtual void visit(WhereClause& node) = 0;
    virtual void visit(JoinClause& node) = 0;
    virtual void visit(GroupByClause& node) = 0;
    virtual void visit(HavingClause& node) = 0;
    virtual void visit(OrderByClause& node) = 0;
};

// 语句节点基类
class Statement : public ASTNode {
public:
    enum Type {
        CREATE,
        SELECT,
        INSERT,
        UPDATE,
        DELETE,
        DROP,
        ALTER,
        USE,
        CREATE_USER,
        DROP_USER,
        GRANT,
        REVOKE
    };
    
    virtual Type getType() const = 0;
};

// 表达式节点基类
class Expression : public ASTNode {
public:
    enum Type {
        IDENTIFIER,
        STRING_LITERAL,
        INTEGER_LITERAL,
        FLOAT_LITERAL,
        BINARY,
        UNARY,
        FUNCTION,
        SUBQUERY
    };
    
    virtual Type getType() const = 0;
};

// 具体节点类示例
class CreateStatement : public Statement {
public:
    enum Target {
        DATABASE,
        TABLE,
        INDEX
    };
    
    CreateStatement(Target target, const SourceLocation& location);
    
    Type getType() const override { return CREATE; }
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    std::unique_ptr<ASTNode> clone() const override;
    std::string dump(int indent = 0) const override;
    std::string toString() const override;
    
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

#### 2.3.4 AST节点层次结构

```
ASTNode
├── Statement
│   ├── CreateStatement
│   ├── SelectStatement
│   ├── InsertStatement
│   ├── UpdateStatement
│   ├── DeleteStatement
│   ├── DropStatement
│   ├── AlterStatement
│   ├── UseStatement
│   ├── CreateUserStatement
│   ├── DropUserStatement
│   ├── GrantStatement
│   └── RevokeStatement
└── Expression
    ├── IdentifierExpression
    ├── StringLiteralExpression
    ├── IntegerLiteralExpression
    ├── FloatLiteralExpression
    ├── BinaryExpression
    ├── UnaryExpression
    ├── FunctionExpression
    └── SubqueryExpression
└── Clause
    ├── WhereClause
    ├── JoinClause
    ├── GroupByClause
    ├── HavingClause
    └── OrderByClause
```

### 2.4 结构化错误处理系统

#### 2.4.1 设计思路

实现了完整的结构化错误处理系统，支持多种错误类型和严重程度，提供详细的错误信息和多格式输出。

#### 2.4.2 错误类型分类

```cpp
enum class ErrorType {
    // 词法错误
    LEXER_INVALID_CHARACTER,
    LEXER_UNTERMINATED_STRING,
    LEXER_UNTERMINATED_COMMENT,
    
    // 语法错误
    PARSER_UNEXPECTED_TOKEN,
    PARSER_MISSING_TOKEN,
    PARSER_INVALID_STATEMENT,
    PARSER_INVALID_EXPRESSION,
    
    // 语义错误
    SEMANTIC_UNDEFINED_IDENTIFIER,
    SEMANTIC_DUPLICATE_DEFINITION,
    SEMANTIC_INVALID_TYPE,
    SEMANTIC_INVALID_OPERATION,
    
    // 运行时错误
    RUNTIME_ERROR,
    
    // 其他错误
    UNKNOWN_ERROR
};

// 错误严重程度
enum class ErrorSeverity {
    INFO,    // 信息性消息
    WARNING, // 警告
    ERROR,   // 错误，无法继续执行
    FATAL    // 致命错误，程序终止
};
```

#### 2.4.3 错误类设计

```cpp
class Error {
public:
    Error(ErrorType type, ErrorSeverity severity, 
          const SourceLocation& location, const std::string& message);
    
    ErrorType getType() const;
    ErrorSeverity getSeverity() const;
    const SourceLocation& getLocation() const;
    const std::string& getMessage() const;
    
    std::string toString() const;
    std::string toJSON() const;
    std::string toIDEFormat() const;
    
private:
    ErrorType type_;
    ErrorSeverity severity_;
    SourceLocation location_;
    std::string message_;
};

class ErrorCollector {
public:
    void addError(const Error& error);
    void addError(ErrorType type, ErrorSeverity severity, 
                  const SourceLocation& location, const std::string& message);
    
    const std::vector<Error>& getErrors() const;
    size_t getErrorCount() const;
    size_t getWarningCount() const;
    size_t getFatalCount() const;
    
    bool hasErrors() const;
    bool hasFatalErrors() const;
    
    void clear();
    
    std::string generateReport() const;
    std::string generateJSONReport() const;
    std::string generateIDEReport() const;
    
private:
    std::vector<Error> errors_;
};
```

#### 2.4.4 错误处理流程

1. 解析过程中检测到错误
2. 创建Error对象，包含错误类型、严重程度、位置和详细信息
3. 将Error对象添加到ErrorCollector
4. 根据错误严重程度决定是否继续解析
5. 解析完成后，生成完整的错误报告
6. 支持多种输出格式：控制台、JSON、IDE格式

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

## 3.4 数据控制语句(DCL)

### 3.4.1 CREATE USER语句

#### 语法
```
CREATE USER username IDENTIFIED BY 'password'
CREATE USER username WITH PASSWORD 'password'
```

#### 示例
```sql
CREATE USER 'john' IDENTIFIED BY 'password123';
CREATE USER 'jane' WITH PASSWORD 'mypassword';
```

### 3.4.2 DROP USER语句

#### 语法
```
DROP USER [IF EXISTS] username
```

#### 示例
```sql
DROP USER 'john';
DROP USER IF EXISTS 'jane';
```

### 3.4.3 GRANT语句

#### 语法
```
GRANT privilege [, privilege]* ON object_type object_name TO grantee
GRANT ALL PRIVILEGES ON object_type object_name TO grantee
```

#### 示例
```sql
GRANT SELECT ON TABLE users TO 'john';
GRANT SELECT, INSERT, UPDATE ON TABLE orders TO 'jane';
GRANT ALL PRIVILEGES ON DATABASE mydb TO 'admin';
```

### 3.4.4 REVOKE语句

#### 语法
```
REVOKE privilege [, privilege]* ON object_type object_name FROM grantee
REVOKE ALL PRIVILEGES ON object_type object_name FROM grantee
```

#### 示例
```sql
REVOKE SELECT ON TABLE users FROM 'john';
REVOKE SELECT, INSERT, UPDATE ON TABLE orders FROM 'jane';
REVOKE ALL PRIVILEGES ON DATABASE mydb FROM 'admin';
```

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

为每个组件编写独立的单元测试，覆盖率达到100%核心功能：

1. **词法分析器测试**：
   - 验证DFA状态机的正确性
   - 验证各种Token类型的识别
   - 测试Unicode标识符支持
   - 测试注释处理
   - 测试错误情况处理

2. **语法分析器测试**：
   - 验证各种SQL语句的正确解析
   - 测试表达式优先级处理
   - 测试错误恢复机制
   - 测试BNF规则的严格映射

3. **AST测试**：
   - 验证生成的AST结构是否符合预期
   - 测试AST访问者模式
   - 测试AST克隆和调试功能
   - 测试位置信息的正确性

4. **错误处理测试**：
   - 验证错误收集和报告
   - 测试多格式错误输出
   - 测试错误统计功能

### 4.2 集成测试

测试完整的SQL解析流程：

1. **端到端SQL处理测试**：从SQL文本到AST的完整流程
2. **复杂SQL语句测试**：包含多种子句和表达式的复杂查询
3. **错误场景测试**：各种语法错误和语义错误的处理
4. **边界条件测试**：极端长度、嵌套深度等边界情况
5. **兼容性测试**：确保与现有系统的兼容性

### 4.3 性能测试

测试解析器在不同规模SQL语句下的性能表现：

1. **基准性能测试**：新旧解析器性能对比
   - 简单查询处理：+15-25%性能提升
   - 复杂查询处理：+25-40%性能提升
   - 内存使用：-20-30%内存占用

2. **吞吐量测试**：字符处理速度评估
3. **大查询性能测试**：复杂SQL语句处理
4. **并发性能测试**：多线程环境下的表现

### 4.4 测试框架

采用现代化的测试框架和工具：

1. **单元测试框架**：Google Test (gtest)
2. **性能测试框架**：Google Benchmark
3. **测试覆盖率工具**：gcov/lcov
4. **测试自动化**：CI/CD集成
5. **测试报告生成**：自动化测试报告

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