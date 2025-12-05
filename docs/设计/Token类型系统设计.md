# SQLCC Token 类型系统设计

本文档定义了SQLCC解析器新的Token类型系统设计，采用更清晰、更系统化的分类方式。

## 1. Token 分类体系

### 1.1 按功能分类

#### 1.1.1 标点符号 (Punctuation)
```cpp
enum class PunctuationType {
    // 括号
    LPAREN,        // (
    RPAREN,        // )
    LBRACE,        // {
    RBRACE,        // }
    LBRACKET,      // [
    RBRACKET,      // ]

    // 分隔符
    COMMA,         // ,
    SEMICOLON,     // ;
    DOT,           // .
    COLON,         // :
    DOUBLE_COLON,  // ::

    // 其他
    QUESTION,      // ?
    AT,            // @
    HASH,          // #
    DOLLAR,        // $
};
```

#### 1.1.2 操作符 (Operators)
```cpp
enum class OperatorType {
    // 算术运算符
    PLUS,          // +
    MINUS,         // -
    MULTIPLY,      // *
    DIVIDE,        // /
    MODULO,        // %
    POWER,         // ^

    // 比较运算符
    EQUAL,         // =
    NOT_EQUAL,     // != 或 <>
    LESS,          // <
    LESS_EQUAL,    // <=
    GREATER,       // >
    GREATER_EQUAL, // >=

    // 逻辑运算符
    AND,           // AND 或 &&
    OR,            // OR 或 ||
    NOT,           // NOT 或 !

    // 位运算符
    BIT_AND,       // &
    BIT_OR,        // |
    BIT_XOR,       // ^
    BIT_NOT,       // ~
    SHIFT_LEFT,    // <<
    SHIFT_RIGHT,   // >>

    // 字符串运算符
    CONCAT,        // ||

    // 赋值运算符
    ASSIGN,        // =

    // 其他运算符
    LIKE,          // LIKE
    IN,            // IN
    IS,            // IS
    BETWEEN,       // BETWEEN
    EXISTS,        // EXISTS
};
```

#### 1.1.3 字面量 (Literals)
```cpp
enum class LiteralType {
    // 数值字面量
    INTEGER,       // 整数，如 123
    FLOAT,         // 浮点数，如 123.45
    DECIMAL,       // 十进制数，如 123.45

    // 字符串字面量
    STRING,        // 字符串，如 'hello' 或 "hello"

    // 布尔字面量
    TRUE,          // TRUE
    FALSE,         // FALSE

    // 空值字面量
    NULL,          // NULL

    // 日期时间字面量
    DATE,          // DATE '2023-01-01'
    TIME,          // TIME '12:30:45'
    TIMESTAMP,     // TIMESTAMP '2023-01-01 12:30:45'

    // 数组字面量
    ARRAY,         // [1, 2, 3]

    // JSON字面量
    JSON,          // {"key": "value"}
};
```

#### 1.1.4 关键字 (Keywords)
```cpp
enum class KeywordType {
    // DDL 关键字
    CREATE,
    ALTER,
    DROP,
    TABLE,
    INDEX,
    VIEW,
    TRIGGER,
    PROCEDURE,
    FUNCTION,
    DATABASE,
    SCHEMA,

    // DML 关键字
    SELECT,
    INSERT,
    UPDATE,
    DELETE,
    FROM,
    INTO,
    VALUES,
    SET,

    // 聚合函数
    COUNT,
    SUM,
    AVG,
    MIN,
    MAX,

    // 子句关键字
    WHERE,
    GROUP,
    BY,
    HAVING,
    ORDER,
    LIMIT,
    OFFSET,

    // JOIN 关键字
    JOIN,
    INNER,
    LEFT,
    RIGHT,
    FULL,
    OUTER,
    ON,
    USING,

    // 约束关键字
    PRIMARY,
    KEY,
    FOREIGN,
    REFERENCES,
    UNIQUE,
    CHECK,
    DEFAULT,
    NOT,
    NULL,
    AUTO_INCREMENT,

    // 逻辑关键字
    AND,
    OR,
    IN,
    EXISTS,
    BETWEEN,
    LIKE,
    AS,
    DISTINCT,
    ALL,
    ANY,
    SOME,

    // 集合操作
    UNION,
    INTERSECT,
    EXCEPT,

    // 控制流
    CASE,
    WHEN,
    THEN,
    ELSE,
    END,
    IF,

    // 事务控制
    BEGIN,
    COMMIT,
    ROLLBACK,
    TRANSACTION,

    // SHOW 命令
    SHOW,
    DESCRIBE,
    EXPLAIN,

    // 权限管理
    GRANT,
    REVOKE,
    TO,
    FROM,
    WITH,
    PASSWORD,
    IDENTIFIED,
    USER,

    // 其他
    ASC,
    DESC,
    TRUE,
    FALSE,
};
```

#### 1.1.5 标识符 (Identifiers)
```cpp
enum class IdentifierType {
    // 基本标识符
    IDENTIFIER,        // 普通标识符，如 table_name, column_name

    // 限定标识符
    QUALIFIED_NAME,    // 限定名，如 table.column 或 db.table.column

    // 特殊标识符
    QUOTED_IDENTIFIER, // 引号标识符，如 "SELECT" 或 `SELECT`
};
```

#### 1.1.6 注释 (Comments)
```cpp
enum class CommentType {
    SINGLE_LINE,     // -- 单行注释
    MULTI_LINE,      // /* 多行注释 */
    INLINE,          // # 行内注释（如果支持）
};
```

### 1.2 统一 Token 类型枚举

基于上述分类，我们设计统一的Token类型枚举：

```cpp
class Token {
public:
    enum Type {
        // 特殊标记
        END_OF_INPUT = 0,
        UNKNOWN,
        ERROR,

        // 标点符号 (1000-1999)
        PUNCTUATION_START = 1000,
        LPAREN = PUNCTUATION_START,
        RPAREN,
        COMMA,
        SEMICOLON,
        DOT,
        PUNCTUATION_END,

        // 操作符 (2000-2999)
        OPERATOR_START = 2000,
        OP_PLUS = OPERATOR_START,
        OP_MINUS,
        OP_MULTIPLY,
        OP_DIVIDE,
        OP_EQUAL,
        OP_NOT_EQUAL,
        OP_LESS,
        OP_LESS_EQUAL,
        OP_GREATER,
        OP_GREATER_EQUAL,
        OP_AND,
        OP_OR,
        OP_NOT,
        OP_LIKE,
        OP_IN,
        OP_IS,
        OPERATOR_END,

        // 字面量 (3000-3999)
        LITERAL_START = 3000,
        INTEGER_LITERAL = LITERAL_START,
        FLOAT_LITERAL,
        STRING_LITERAL,
        BOOLEAN_LITERAL,
        NULL_LITERAL,
        LITERAL_END,

        // 关键字 (4000-4999)
        KEYWORD_START = 4000,
        KW_SELECT = KEYWORD_START,
        KW_FROM,
        KW_WHERE,
        KW_INSERT,
        KW_UPDATE,
        KW_DELETE,
        KW_CREATE,
        KW_DROP,
        KW_ALTER,
        KW_TABLE,
        KW_INDEX,
        // ... 更多关键字
        KEYWORD_END,

        // 标识符 (5000-5999)
        IDENTIFIER_START = 5000,
        IDENTIFIER = IDENTIFIER_START,
        QUALIFIED_IDENTIFIER,
        QUOTED_IDENTIFIER,
        IDENTIFIER_END,

        // 注释 (6000-6999)
        COMMENT_START = 6000,
        SINGLE_LINE_COMMENT = COMMENT_START,
        MULTI_LINE_COMMENT,
        COMMENT_END,
    };

    // 构造函数和方法...
};
```

## 2. Token 结构设计

### 2.1 Token 类定义

```cpp
class Token {
public:
    // 类型
    Type type_;

    // 词素（原始文本）
    std::string lexeme_;

    // 位置信息
    size_t line_;
    size_t column_;
    size_t offset_;  // 在输入流中的偏移量

    // 语义信息（可选，用于语义分析）
    std::shared_ptr<SemanticInfo> semantic_info_;

    // 构造函数
    Token(Type type, const std::string& lexeme,
          size_t line = 0, size_t column = 0, size_t offset = 0);

    // 获取类型信息
    Type getType() const;
    std::string getLexeme() const;
    size_t getLine() const;
    size_t getColumn() const;
    size_t getOffset() const;

    // 类型检查
    bool isPunctuation() const;
    bool isOperator() const;
    bool isLiteral() const;
    bool isKeyword() const;
    bool isIdentifier() const;
    bool isComment() const;

    // 类型转换
    static std::string typeToString(Type type);
    static Type stringToType(const std::string& str);

    // 调试支持
    std::string toString() const;
};
```

### 2.2 语义信息结构

```cpp
struct SemanticInfo {
    // 对于标识符
    std::string resolved_name;    // 解析后的名称
    SymbolType symbol_type;       // 符号类型（表、列、函数等）

    // 对于字面量
    ValueType value_type;         // 值类型
    std::any value;               // 实际值

    // 对于操作符
    int precedence;               // 优先级
    Associativity associativity;  // 结合性

    // 作用域信息
    std::shared_ptr<Scope> scope;
};
```

## 3. Token 工厂模式

为了更好地管理Token的创建，我们使用工厂模式：

```cpp
class TokenFactory {
public:
    static Token createPunctuation(PunctuationType type, size_t line, size_t column);
    static Token createOperator(OperatorType type, size_t line, size_t column);
    static Token createLiteral(LiteralType type, const std::string& value, size_t line, size_t column);
    static Token createKeyword(KeywordType type, size_t line, size_t column);
    static Token createIdentifier(const std::string& name, size_t line, size_t column);
    static Token createComment(CommentType type, const std::string& content, size_t line, size_t column);

    // 批量创建
    static std::vector<Token> createTokens(const std::string& input);
};
```

## 4. Token 流管理

```cpp
class TokenStream {
public:
    // 构造函数
    TokenStream(const std::vector<Token>& tokens);

    // 导航方法
    const Token& current() const;
    const Token& peek(int offset = 1) const;
    void advance();

    // 位置管理
    void mark();
    void reset();
    void rewind(size_t positions);

    // 查询方法
    bool isAtEnd() const;
    size_t position() const;
    size_t size() const;

    // 错误恢复
    void skipToNextStatement();
    void skipToToken(Token::Type type);

    // 调试支持
    std::string toString() const;
    std::vector<Token> getTokensInRange(size_t start, size_t end) const;
};
```

## 5. 实现注意事项

### 5.1 内存管理
- Token对象应该设计为轻量级，避免不必要的拷贝
- 使用对象池模式复用Token对象
- 支持Token的延迟初始化

### 5.2 线程安全
- Token类应该是线程安全的
- TokenFactory可以设计为线程安全的单例
- TokenStream需要考虑并发访问

### 5.3 扩展性
- 新的Token类型可以通过继承扩展
- 支持自定义Token类型的注册机制
- 插件架构支持第三方Token类型

### 5.4 调试支持
- Token的字符串表示
- Token流的文本重建
- 位置信息的可视化

## 6. 与现有代码的兼容性

### 6.1 API 兼容性
- 保持现有的Token接口不变
- 提供适配器模式支持旧的Token使用方式
- 渐进式迁移到新的Token系统

### 6.2 性能兼容性
- 新的Token系统不应该显著降低性能
- 提供性能基准测试
- 支持性能监控和调优

---

*本文档定义了SQLCC解析器新的Token类型系统，为后续的词法分析器和语法分析器实现奠定基础。*
