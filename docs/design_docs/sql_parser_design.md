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

5. **数据控制语句(DCL)**
   - CREATE USER
   - DROP USER
   - GRANT
   - REVOKE

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
        KEYWORD_USER, KEYWORD_GRANT, KEYWORD_REVOKE, // 新增DCL关键字
        KEYWORD_PRIVILEGES, KEYWORD_TO, KEYWORD_FROM,
        KEYWORD_WITH, KEYWORD_PASSWORD, KEYWORD_IDENTIFIED,
        
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
```

### 2.2 语法分析器(Parser)

#### 2.2.1 设计思路

语法分析器采用递归下降解析方法，根据SQL语法规则将Token流转换为抽象语法树(AST)。每个语句类型都有对应的解析方法。

#### 2.2.2 核心类设计

```cpp
class Parser {
public:
    Parser(const std::string& input);
    ~Parser() = default;
    
    std::vector<std::unique_ptr<Statement>> parseStatements();
    
private:
    Lexer lexer_;
    Token currentToken_;
    
    // 通用解析方法
    void nextToken();
    bool match(Token::Type type);
    void consume(Token::Type type);
    void consume();
    void expect(Token::Type type, const std::string& errorMessage);
    
    // 语句解析方法
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<CreateStatement> parseCreateStatement();
    std::unique_ptr<SelectStatement> parseSelectStatement();
    std::unique_ptr<InsertStatement> parseInsertStatement();
    std::unique_ptr<UpdateStatement> parseUpdateStatement();
    std::unique_ptr<DeleteStatement> parseDeleteStatement();
    std::unique_ptr<DropStatement> parseDropStatement();
    std::unique_ptr<AlterStatement> parseAlterStatement();
    std::unique_ptr<UseStatement> parseUseStatement();
    
    // DCL语句解析方法
    std::unique_ptr<Statement> parseCreateUserStatement();
    std::unique_ptr<Statement> parseDropUserStatement();
    std::unique_ptr<Statement> parseGrantStatement();
    std::unique_ptr<Statement> parseRevokeStatement();
    
    // 辅助解析方法
    void parseColumnDefinitions(CreateStatement& stmt);
    std::string parseDataType();
    void parseColumnConstraint(ColumnDefinition& columnDef);
    void parseSelectList(SelectStatement& stmt);
    void parseFromClause(SelectStatement& stmt);
    void parseWhereClause(SelectStatement& stmt);
    void parseJoinClause(SelectStatement& stmt);
    void parseGroupByClause(SelectStatement& stmt);
    void parseOrderByClause(SelectStatement& stmt);
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseComparisonExpression();
    std::unique_ptr<Expression> parseAdditiveExpression();
    std::unique_ptr<Expression> parseMultiplicativeExpression();
    std::unique_ptr<Expression> parsePrimaryExpression();
};
```

### 2.3 抽象语法树(AST)

#### 2.3.1 设计思路

抽象语法树是SQL语句的结构化表示，每种语句类型都有对应的AST节点类。AST节点形成继承层次结构，基类为Statement。

#### 2.3.2 核心类设计

##### 基类Statement

```cpp
class Statement {
public:
    enum Type {
        SELECT,
        INSERT,
        UPDATE,
        DELETE,
        CREATE,
        DROP,
        ALTER,
        USE,
        CREATE_USER,  // 新增DCL语句类型
        DROP_USER,    // 新增DCL语句类型
        GRANT,        // 新增DCL语句类型
        REVOKE        // 新增DCL语句类型
    };
    
    Statement(Type type);
    virtual ~Statement() = default;
    
    Type getType() const;
    
private:
    Type type_;
};
```

##### DCL语句节点

###### CreateUserStatement

```cpp
class CreateUserStatement : public Statement {
public:
    CreateUserStatement(const std::string& username, const std::string& password);
    
    const std::string& getUsername() const;
    const std::string& getPassword() const;
    bool isWithPassword() const;
    
    void setWithPassword(bool withPassword);
    
private:
    std::string username_;
    std::string password_;
    bool withPassword_;
};
```

###### DropUserStatement

```cpp
class DropUserStatement : public Statement {
public:
    DropUserStatement(const std::string& username);
    
    const std::string& getUsername() const;
    bool isIfExists() const;
    
    void setIfExists(bool ifExists);
    
private:
    std::string username_;
    bool ifExists_;
};
```

###### GrantStatement

```cpp
class GrantStatement : public Statement {
public:
    GrantStatement();
    
    void addPrivilege(const std::string& privilege);
    const std::vector<std::string>& getPrivileges() const;
    
    void setObjectType(const std::string& objectType);
    const std::string& getObjectType() const;
    
    void setObjectName(const std::string& objectName);
    const std::string& getObjectName() const;
    
    void setGrantee(const std::string& grantee);
    const std::string& getGrantee() const;
    
private:
    std::vector<std::string> privileges_;
    std::string objectType_;
    std::string objectName_;
    std::string grantee_;
};
```

###### RevokeStatement

```cpp
class RevokeStatement : public Statement {
public:
    RevokeStatement();
    
    void addPrivilege(const std::string& privilege);
    const std::vector<std::string>& getPrivileges() const;
    
    void setObjectType(const std::string& objectType);
    const std::string& getObjectType() const;
    
    void setObjectName(const std::string& objectName);
    const std::string& getObjectName() const;
    
    void setGrantee(const std::string& grantee);
    const std::string& getGrantee() const;
    
private:
    std::vector<std::string> privileges_;
    std::string objectType_;
    std::string objectName_;
    std::string grantee_;
};
```

## 3. DCL语句语法规范

### 3.1 CREATE USER语句

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

### 3.2 DROP USER语句

#### 语法

```
DROP USER [IF EXISTS] username
```

#### 示例

```sql
DROP USER 'john';
DROP USER IF EXISTS 'jane';
```

### 3.3 GRANT语句

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

### 3.4 REVOKE语句

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

## 4. 扩展性设计

### 4.1 新增语句类型

新增SQL语句类型时，需要进行以下操作：

1. 在Token类中添加新的关键字类型
2. 在Lexer中添加关键字识别逻辑
3. 在Parser中添加新的解析方法
4. 在Statement基类中添加新的语句类型枚举
5. 创建新的AST节点类
6. 在执行引擎中添加对应的执行逻辑

### 4.2 新增关键字

新增关键字时，需要进行以下操作：

1. 在Token::Type枚举中添加新的关键字类型
2. 在Lexer::readIdentifier方法中添加关键字识别逻辑
3. 在Token::getTypeName方法中添加类型名称映射

这种模块化设计使得SQL解析模块易于扩展和维护。