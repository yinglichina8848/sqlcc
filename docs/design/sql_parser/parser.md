# Parser类详细设计

## 概述

Parser类是SQL解析器的核心组件，负责将词法分析器(Lexer)生成的Token流解析为抽象语法树(AST)。它实现了递归下降解析算法，支持多种SQL语句的解析。

## 类定义

```cpp
class Parser {
public:
    explicit Parser(Lexer &lexer);
    explicit Parser(const std::string &sql);
    ~Parser();
    
    std::unique_ptr<Statement> parseStatement();
    std::vector<std::unique_ptr<Statement>> parseStatements();
    std::unique_ptr<Statement> parseSingleStatement();
    void setStrictMode(bool strict);
    const std::string &getErrorMessage() const;

private:
    bool match(Token::Type expected);
    bool peek(Token::Type expected) const;
    void consume();
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseCondition();
    std::unique_ptr<Expression> parseLogical();
    std::unique_ptr<Expression> parseComparison();
    std::unique_ptr<Expression> parseAdditive();
    std::unique_ptr<Expression> parseMultiplicative();
    std::unique_ptr<Expression> parseUnary();
    std::unique_ptr<Expression> parseAtomic();
    std::unique_ptr<Expression> parseFunctionCall();
    std::unique_ptr<Expression> parsePrimaryExpression();
    std::unique_ptr<SelectStatement> parseSelectStatement();
    std::unique_ptr<Statement> parseSelect();
    std::unique_ptr<Statement> parseInsert();
    std::unique_ptr<Statement> parseUpdate();
    std::unique_ptr<Statement> parseDelete();
    std::unique_ptr<Statement> parseCreate();
    std::unique_ptr<Statement> parseDrop();
    std::unique_ptr<Statement> parseAlter();
    std::unique_ptr<Statement> parseUse();
    std::unique_ptr<CreateIndexStatement> parseCreateIndex();
    std::unique_ptr<DropIndexStatement> parseDropIndex();
    std::unique_ptr<Statement> parseTransactionStatement();
    std::unique_ptr<BeginTransactionStatement> parseBeginTransaction();
    std::unique_ptr<CommitStatement> parseCommit();
    std::unique_ptr<RollbackStatement> parseRollback();
    std::unique_ptr<SavepointStatement> parseSavepoint();
    std::unique_ptr<SetTransactionStatement> parseSetTransaction();
    std::unique_ptr<CreateStatement> parseCreateDatabase();
    std::unique_ptr<CreateStatement> parseCreateTable();
    TableReference parseTableReference();
    std::unique_ptr<WhereClause> parseWhereClause();
    std::unique_ptr<GroupByClause> parseGroupByClause();
    std::unique_ptr<OrderByClause> parseOrderByClause();
    ColumnDefinition parseColumnDefinition();
    std::unique_ptr<TableConstraint> parseTableConstraint();
    std::unique_ptr<PrimaryKeyConstraint> parsePrimaryKeyConstraint();
    std::unique_ptr<UniqueConstraint> parseUniqueConstraint();
    std::unique_ptr<ForeignKeyConstraint> parseForeignKeyConstraint();
    std::unique_ptr<CheckConstraint> parseCheckConstraint();
    std::vector<SelectItem> parseSelectItems();
    std::unique_ptr<JoinClause> parseJoinClause();
    std::vector<std::unique_ptr<Expression>> parseValueList();
    std::vector<std::vector<std::unique_ptr<Expression>>> parseMultipleValues();
    void reportError(const std::string &message);

private:
    Lexer *lexer_;
    bool ownsLexer_;
    Token currentToken_;
    bool strictMode_;
    std::string errorMessage_;
};
```

## 构造函数

### Parser(Lexer &lexer)

构造函数，接收Lexer引用：

1. 存储词法分析器指针
2. 设置不拥有词法分析器对象
3. 获取第一个Token

### Parser(const std::string &sql)

构造函数，接收SQL字符串：

1. 创建新的词法分析器对象
2. 设置拥有词法分析器对象
3. 获取第一个Token

## 析构函数

### ~Parser()

析构函数：

1. 如果拥有词法分析器对象，则释放它

## 公共方法

### std::unique_ptr<Statement> parseStatement()

解析SQL语句：

1. 根据当前Token类型确定语句类型
2. 调用相应的解析方法
3. 返回解析得到的语句节点

### std::vector<std::unique_ptr<Statement>> parseStatements()

解析多个SQL语句：

1. 循环解析语句直到遇到文件结束符
2. 返回解析得到的语句节点列表

### std::unique_ptr<Statement> parseSingleStatement()

解析单个SQL语句：

1. 解析一个语句
2. 检查是否还有剩余Token

### void setStrictMode(bool strict)

设置严格模式：

1. 设置strictMode_成员变量
2. 在严格模式下对语法错误更敏感

### const std::string &getErrorMessage() const

获取错误信息：

1. 返回errorMessage_成员变量

## 私有方法

### bool match(Token::Type expected)

匹配并消费Token：

1. 检查当前Token类型是否与期望类型匹配
2. 如果匹配则消费当前Token并获取下一个Token
3. 返回是否匹配成功

### bool peek(Token::Type expected) const

尝试匹配Token但不消费：

1. 检查当前Token类型是否与期望类型匹配
2. 不消费当前Token
3. 返回是否匹配成功

### void consume()

消费当前Token：

1. 获取下一个Token并更新currentToken_

### std::unique_ptr<Expression> parseExpression()

解析表达式：

1. 调用parseLogical方法解析逻辑表达式

### std::unique_ptr<Expression> parseCondition()

解析条件表达式：

1. 调用parseLogical方法解析逻辑表达式

### std::unique_ptr<Expression> parseLogical()

解析逻辑表达式（AND/OR）：

1. 解析比较表达式
2. 处理AND和OR操作符

### std::unique_ptr<Expression> parseComparison()

解析比较表达式：

1. 解析加减表达式
2. 处理比较操作符（=, !=, <, >等）

### std::unique_ptr<Expression> parseAdditive()

解析加减表达式：

1. 解析乘除表达式
2. 处理加减操作符

### std::unique_ptr<Expression> parseMultiplicative()

解析乘除表达式：

1. 解析一元表达式
2. 处理乘除操作符

### std::unique_ptr<Expression> parseUnary()

解析一元表达式：

1. 处理一元操作符（+,-,NOT等）
2. 解析原子表达式

### std::unique_ptr<Expression> parseAtomic()

解析原子表达式：

1. 处理标识符、字面量、括号表达式等

### std::unique_ptr<Expression> parseFunctionCall()

解析函数调用：

1. 解析函数名
2. 解析参数列表

### std::unique_ptr<Expression> parsePrimaryExpression()

解析主键表达式：

1. 解析主键相关的表达式

### std::unique_ptr<SelectStatement> parseSelectStatement()

解析SELECT语句（用于子查询）：

1. 解析完整的SELECT语句结构

### std::unique_ptr<Statement> parseSelect()

解析SELECT语句：

1. 解析SELECT子句
2. 解析FROM子句
3. 解析WHERE、GROUP BY、ORDER BY等子句

### std::unique_ptr<Statement> parseInsert()

解析INSERT语句：

1. 解析INSERT INTO子句
2. 解析列列表（如果有）
3. 解析VALUES子句或SELECT子句

### std::unique_ptr<Statement> parseUpdate()

解析UPDATE语句：

1. 解析UPDATE表名
2. 解析SET子句
3. 解析WHERE子句

### std::unique_ptr<Statement> parseDelete()

解析DELETE语句：

1. 解析DELETE FROM子句
2. 解析WHERE子句

### std::unique_ptr<Statement> parseCreate()

解析CREATE语句：

1. 根据下一个Token确定是CREATE DATABASE还是CREATE TABLE
2. 调用相应的解析方法

### std::unique_ptr<Statement> parseDrop()

解析DROP语句：

1. 根据下一个Token确定是DROP DATABASE还是DROP TABLE
2. 调用相应的解析方法

### std::unique_ptr<Statement> parseAlter()

解析ALTER语句：

1. 解析ALTER TABLE语句
2. 处理ADD、DROP、MODIFY等操作

### std::unique_ptr<Statement> parseUse()

解析USE语句：

1. 解析USE DATABASE语句

### std::unique_ptr<CreateIndexStatement> parseCreateIndex()

解析CREATE INDEX语句：

1. 解析索引名
2. 解析表名和列列表

### std::unique_ptr<DropIndexStatement> parseDropIndex()

解析DROP INDEX语句：

1. 解析索引名

### std::unique_ptr<Statement> parseTransactionStatement()

解析事务相关语句：

1. 根据当前Token确定事务语句类型
2. 调用相应的解析方法

### std::unique_ptr<BeginTransactionStatement> parseBeginTransaction()

解析BEGIN TRANSACTION语句：

1. 解析BEGIN TRANSACTION语句

### std::unique_ptr<CommitStatement> parseCommit()

解析COMMIT语句：

1. 解析COMMIT语句

### std::unique_ptr<RollbackStatement> parseRollback()

解析ROLLBACK语句：

1. 解析ROLLBACK语句

### std::unique_ptr<SavepointStatement> parseSavepoint()

解析SAVEPOINT语句：

1. 解析SAVEPOINT语句

### std::unique_ptr<SetTransactionStatement> parseSetTransaction()

解析SET TRANSACTION语句：

1. 解析SET TRANSACTION语句

### std::unique_ptr<CreateStatement> parseCreateDatabase()

解析CREATE DATABASE语句：

1. 解析数据库名
2. 解析可选的数据库属性

### std::unique_ptr<CreateStatement> parseCreateTable()

解析CREATE TABLE语句：

1. 解析表名
2. 解析列定义列表
3. 解析表级约束

### TableReference parseTableReference()

解析表引用：

1. 解析表名和可选的别名

### std::unique_ptr<WhereClause> parseWhereClause()

解析WHERE子句：

1. 解析WHERE关键字后的条件表达式

### std::unique_ptr<GroupByClause> parseGroupByClause()

解析GROUP BY子句：

1. 解析GROUP BY关键字后的列列表

### std::unique_ptr<OrderByClause> parseOrderByClause()

解析ORDER BY子句：

1. 解析ORDER BY关键字后的列列表和排序方向

### ColumnDefinition parseColumnDefinition()

解析列定义：

1. 解析列名
2. 解析数据类型
3. 解析列级约束

### std::unique_ptr<TableConstraint> parseTableConstraint()

解析表级约束：

1. 根据约束类型调用相应的解析方法

### std::unique_ptr<PrimaryKeyConstraint> parsePrimaryKeyConstraint()

解析PRIMARY KEY表级约束：

1. 解析主键列列表

### std::unique_ptr<UniqueConstraint> parseUniqueConstraint()

解析UNIQUE表级约束：

1. 解析唯一约束列列表

### std::unique_ptr<ForeignKeyConstraint> parseForeignKeyConstraint()

解析FOREIGN KEY表级约束：

1. 解析外键列列表
2. 解析引用的表和列

### std::unique_ptr<CheckConstraint> parseCheckConstraint()

解析CHECK表级约束：

1. 解析检查条件表达式

### std::vector<SelectItem> parseSelectItems()

解析选择项列表：

1. 解析SELECT子句中的列或表达式列表

### std::unique_ptr<JoinClause> parseJoinClause()

解析JOIN子句：

1. 解析JOIN类型
2. 解析连接表和ON条件

### std::vector<std::unique_ptr<Expression>> parseValueList()

解析值列表：

1. 解析括号内的值列表

### std::vector<std::vector<std::unique_ptr<Expression>>> parseMultipleValues()

解析多行值：

1. 解析多组VALUES括号内的值

### void reportError(const std::string &message)

报告语法错误：

1. 设置errorMessage_成员变量
2. 在非严格模式下尝试恢复解析

## 成员变量

### Lexer *lexer_

词法分析器指针，用于获取Token。

### bool ownsLexer_

是否拥有词法分析器，决定析构时是否需要释放lexer_。

### Token currentToken_

当前Token，用于语法分析。

### bool strictMode_

严格模式标志，在严格模式下对语法错误更敏感。

### std::string errorMessage_

错误信息，存储解析过程中遇到的错误。