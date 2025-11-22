# Token类详细设计

## 概述

Token类表示SQL语言中的词法单元，是词法分析器(Lexer)的输出结果。每个Token包含类型、文本内容和位置信息，用于语法分析器(Parser)进行语法分析。

## 类定义

```cpp
class Token {
public:
    enum Type {
        // 关键字
        KEYWORD_CREATE,
        KEYWORD_SELECT,
        KEYWORD_INSERT,
        KEYWORD_UPDATE,
        KEYWORD_DELETE,
        KEYWORD_DROP,
        KEYWORD_ALTER,
        KEYWORD_USE,
        KEYWORD_DATABASE,
        KEYWORD_TABLE,
        KEYWORD_WHERE,
        KEYWORD_JOIN,
        KEYWORD_ON,
        KEYWORD_GROUP,
        KEYWORD_BY,
        KEYWORD_HAVING,
        KEYWORD_ORDER,
        KEYWORD_INTO,
        KEYWORD_VALUES,
        KEYWORD_SET,
        KEYWORD_FROM,
        KEYWORD_AS,
        KEYWORD_DISTINCT,
        KEYWORD_AND,
        KEYWORD_OR,
        KEYWORD_NOT,
        KEYWORD_NULL,
        KEYWORD_IS,
        KEYWORD_LIKE,
        KEYWORD_IN,
        KEYWORD_EXISTS,
        KEYWORD_BETWEEN,
        KEYWORD_LIMIT,
        KEYWORD_OFFSET,
        KEYWORD_ASC,
        KEYWORD_DESC,
        KEYWORD_INDEX,
        KEYWORD_PRIMARY,
        KEYWORD_KEY,
        KEYWORD_UNIQUE,
        KEYWORD_IF,
        KEYWORD_NOT_NULL,
        KEYWORD_NULLABLE,
        KEYWORD_DEFAULT,
        KEYWORD_CONSTRAINT,
        KEYWORD_FOREIGN,
        KEYWORD_REFERENCES,
        KEYWORD_CHECK,
        KEYWORD_DECIMAL,
        KEYWORD_DATE,
        KEYWORD_TIME,
        KEYWORD_TIMESTAMP,
        KEYWORD_CHAR,
        KEYWORD_VARCHAR,
        KEYWORD_SMALLINT,
        KEYWORD_DOUBLE,
        KEYWORD_BOOLEAN,
        // 事务相关关键字
        KEYWORD_BEGIN,
        KEYWORD_TRANSACTION,
        KEYWORD_START,
        KEYWORD_COMMIT,
        KEYWORD_ROLLBACK,
        KEYWORD_SAVEPOINT,
        KEYWORD_AUTOCOMMIT,
        KEYWORD_ISOLATION,
        KEYWORD_LEVEL,
        KEYWORD_READ,
        KEYWORD_WRITE,
        KEYWORD_UNCOMMITTED,
        KEYWORD_COMMITTED,
        KEYWORD_REPEATABLE,
        KEYWORD_SERIALIZABLE,

        // 标识符
        IDENTIFIER,

        // 字面量
        STRING_LITERAL,
        NUMERIC_LITERAL,

        // 运算符
        OPERATOR_EQUAL,
        OPERATOR_NOT_EQUAL,
        OPERATOR_LESS,
        OPERATOR_LESS_EQUAL,
        OPERATOR_GREATER,
        OPERATOR_GREATER_EQUAL,
        OPERATOR_PLUS,
        OPERATOR_MINUS,
        OPERATOR_MULTIPLY,
        OPERATOR_DIVIDE,
        OPERATOR_MODULO,

        // 标点符号
        PUNCTUATION_LEFT_PAREN,
        PUNCTUATION_RIGHT_PAREN,
        PUNCTUATION_COMMA,
        PUNCTUATION_SEMICOLON,
        PUNCTUATION_DOT,
        PUNCTUATION_COLON,
        PUNCTUATION_DOUBLE_COLON,

        // 其他
        END_OF_INPUT,
        INVALID_TOKEN
    };

    Token(Type type, const std::string &lexeme, int line, int column);
    
    Type getType() const;
    const std::string &getLexeme() const;
    int getLine() const;
    int getColumn() const;
    std::string getTypeName() const;
    std::string toString() const;

private:
    Type type_;
    std::string lexeme_;
    int line_;
    int column_;
    static std::unordered_map<Type, std::string> typeNames_;
};
```

## 构造函数

### Token(Type type, const std::string &lexeme, int line, int column)

构造函数：

1. 初始化Token类型
2. 初始化Token的文本内容
3. 初始化行号和列号

## 公共方法

### Type getType() const

获取Token类型：

1. 返回type_成员变量

### const std::string &getLexeme() const

获取Token的文本内容：

1. 返回lexeme_成员变量

### int getLine() const

获取行号：

1. 返回line_成员变量

### int getColumn() const

获取列号：

1. 返回column_成员变量

### std::string getTypeName() const

获取Token类型的名称（用于调试）：

1. 通过typeNames_映射表查找类型名称

### std::string toString() const

将Token转换为字符串表示（用于调试）：

1. 格式化输出Token信息，包括类型、文本内容和位置

## 成员变量

### Type type_

Token类型，表示Token的语义类别。

### std::string lexeme_

Token的文本内容，表示Token在源代码中的实际字符串。

### int line_, column_

行号和列号，表示Token在源代码中的位置，用于错误报告。

### static std::unordered_map<Type, std::string> typeNames_

类型名称映射表，用于将Token类型映射到可读的字符串名称，便于调试和日志输出。