# Lexer类详细设计

## 概述

Lexer类是SQL解析器的词法分析组件，负责将输入的SQL文本分解为标记(Token)流。它识别关键字、标识符、字面量、运算符和标点符号等，并为每个标记提供位置信息。

## 类定义

```cpp
class Lexer {
public:
    explicit Lexer(const std::string& input);
    ~Lexer() = default;
    
    Token nextToken();

private:
    Token scanNextToken();
    Token peekToken() const;
    struct Position {
        int line;
        int column;
    };
    Position getPosition() const;
    
private:
    void skipWhitespace();
    void skipComment();
    Token readIdentifier();
    Token readNumber();
    Token readString();
    bool isIdentifierStart(char c) const;
    bool isIdentifierPart(char c) const;
    bool isDigit(char c) const;
    bool isLetter(char c) const;
    bool isWhitespace(char c) const;
    char currentChar() const;
    void advance();
    void advance(int count);
    bool isEOF() const;
    bool match(char c) const;
    bool match(const std::string& str) const;
    Token::Type getKeywordType(const std::string& identifier) const;

private:
    const std::string input_;
    size_t position_;
    int line_;
    int column_;
    Token currentToken_;
    bool hasCachedToken_;
};
```

## 构造函数

### Lexer(const std::string& input)

构造函数：

1. 存储输入的SQL字符串
2. 初始化位置信息（position_, line_, column_）
3. 初始化Token缓存状态

## 析构函数

### ~Lexer()

默认析构函数，无需特殊处理。

## 公共方法

### Token nextToken()

获取下一个Token：

1. 如果有缓存的Token，则返回缓存的Token
2. 否则调用scanNextToken扫描下一个Token
3. 更新位置信息

## 私有方法

### Token scanNextToken()

扫描下一个Token（内部实现）：

1. 跳过空白字符
2. 根据当前字符类型确定Token类型
3. 调用相应的读取方法
4. 返回解析出的Token

### Token peekToken() const

查看当前Token但不消费：

1. 返回当前Token对象，不改变Lexer状态

### Position getPosition() const

获取当前位置：

1. 返回行号和列号结构体

### void skipWhitespace()

跳过空白字符：

1. 跳过空格、制表符、换行符等空白字符
2. 更新行号和列号

### void skipComment()

跳过注释：

1. 识别并跳过SQL注释（--开头的行注释）
2. 更新位置信息

### Token readIdentifier()

读取标识符或关键字：

1. 读取连续的标识符字符
2. 检查是否为关键字
3. 返回相应的Token

### Token readNumber()

读取数值字面量：

1. 读取连续的数字字符
2. 处理小数点和科学计数法
3. 返回数值字面量Token

### Token readString()

读取字符串字面量：

1. 读取单引号或双引号包围的字符串
2. 处理转义字符
3. 返回字符串字面量Token

### bool isIdentifierStart(char c) const

检查当前字符是否为标识符开始：

1. 检查字符是否为字母或下划线

### bool isIdentifierPart(char c) const

检查当前字符是否为标识符部分：

1. 检查字符是否为字母、数字或下划线

### bool isDigit(char c) const

检查当前字符是否为数字：

1. 检查字符是否为0-9

### bool isLetter(char c) const

检查当前字符是否为字母：

1. 检查字符是否为a-z或A-Z

### bool isWhitespace(char c) const

检查当前字符是否为空白：

1. 检查字符是否为空格、制表符或换行符

### char currentChar() const

获取当前字符：

1. 返回input_中position_位置的字符

### void advance()

向前移动一个字符：

1. 增加position_
2. 更新行号和列号

### void advance(int count)

向前移动多个字符：

1. 增加position_ count个位置
2. 更新行号和列号

### bool isEOF() const

检查是否到达输入末尾：

1. 检查position_是否超出input_长度

### bool match(char c) const

检查当前字符是否为特定字符：

1. 比较当前字符与指定字符

### bool match(const std::string& str) const

检查当前字符序列是否匹配特定字符串：

1. 比较当前位置开始的字符序列与指定字符串

### Token::Type getKeywordType(const std::string& identifier) const

获取关键字对应的Token类型：

1. 查找标识符是否为关键字
2. 如果是关键字则返回相应Token类型，否则返回IDENTIFIER

## 成员变量

### const std::string input_

输入的SQL字符串，用于词法分析。

### size_t position_

当前扫描位置，指向input_中的字符索引。

### int line_, column_

当前行号和列号，用于错误报告和调试。

### Token currentToken_

当前Token（用于peek），缓存下一个Token。

### bool hasCachedToken_

是否有缓存的Token，用于peekToken方法。