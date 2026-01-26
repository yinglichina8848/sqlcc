# ExpressionParser 设计文档

## 1. 概述

ExpressionParser 是 SQLCC 数据库系统中专门负责解析 SQL 表达式的组件。它实现了单一职责原则，将复杂的 SQL 表达式解析逻辑与主解析器分离，提高了代码的可维护性、可测试性和可扩展性。

## 2. 核心功能

### 2.1 主要功能

- **解析算术表达式**：处理加减乘除等基本算术运算
- **解析逻辑表达式**：处理 AND、OR、NOT 等逻辑运算
- **解析比较表达式**：处理 =、!=、<、>、<=、>=、LIKE、BETWEEN 等比较运算
- **解析函数调用**：处理内置函数和用户自定义函数调用
- **解析列引用**：处理表列的引用（如 table.column）
- **解析字面量**：处理数字、字符串、NULL、布尔值等字面量
- **解析子查询**：处理嵌套的 SELECT 语句

### 2.2 设计优势

- **单一职责**：仅负责 SQL 表达式解析，职责明确
- **模块化**：与主解析器分离，降低耦合度
- **可测试性**：表达式解析逻辑可独立测试
- **可扩展性**：新表达式类型可轻松添加
- **清晰的优先级处理**：通过递归下降算法正确处理运算符优先级

## 3. 类定义

```cpp
class ExpressionParser {
public:
  explicit ExpressionParser(TokenStream& tokens);
  std::unique_ptr<Expression> parseExpression();
  std::unique_ptr<Expression> parseLogicalOr();
  std::unique_ptr<Expression> parseLogicalAnd();
  std::unique_ptr<Expression> parseEquality();
  std::unique_ptr<Expression> parseComparison();
  std::unique_ptr<Expression> parseTerm();
  std::unique_ptr<Expression> parseFactor();
  std::unique_ptr<Expression> parseUnary();
  std::unique_ptr<Expression> parsePrimary();
  std::unique_ptr<Expression> parseIdentifierExpression();

private:
  TokenStream& tokens_;
  std::unique_ptr<Expression> parseBinaryExpression(int minPrecedence);
  int getPrecedence(OperatorKind op) const;
  OperatorKind tokenToOperatorKind(Type tokenType) const;
  std::unique_ptr<Expression> parseBinaryOp(
      std::function<std::unique_ptr<Expression>()> parseNextLevel,
      const std::vector<Type>& operators);
};
```

## 4. 核心组件

### 4.1 构造函数

```cpp
explicit ExpressionParser(TokenStream& tokens);
```

- **功能**：通过依赖注入接收 TokenStream 对象
- **参数**：
  - `tokens` - Token 流引用，提供 token 访问接口
- **设计意图**：确保松耦合和高可测试性

### 4.2 解析方法

#### parseExpression

```cpp
std::unique_ptr<Expression> parseExpression();
```

- **功能**：表达式解析的主入口点
- **返回值**：解析后的表达式 AST 节点
- **设计意图**：作为递归下降解析器的起始点，从最低优先级的 OR 运算符开始解析

#### parseLogicalOr

```cpp
std::unique_ptr<Expression> parseLogicalOr();
```

- **功能**：解析逻辑或表达式（OR 运算符）
- **返回值**：逻辑或表达式 AST 节点
- **优先级**：最低优先级

#### parseLogicalAnd

```cpp
std::unique_ptr<Expression> parseLogicalAnd();
```

- **功能**：解析逻辑与表达式（AND 运算符）
- **返回值**：逻辑与表达式 AST 节点
- **优先级**：高于 OR

#### parseEquality

```cpp
std::unique_ptr<Expression> parseEquality();
```

- **功能**：解析等式表达式（=、!= 等运算符）
- **返回值**：等式表达式 AST 节点
- **优先级**：高于 AND

#### parseComparison

```cpp
std::unique_ptr<Expression> parseComparison();
```

- **功能**：解析比较表达式（<、>、<=、>= 等运算符）
- **返回值**：比较表达式 AST 节点
- **优先级**：高于等式运算符

#### parseTerm

```cpp
std::unique_ptr<Expression> parseTerm();
```

- **功能**：解析项表达式（+、- 运算符）
- **返回值**：项表达式 AST 节点
- **优先级**：高于比较运算符

#### parseFactor

```cpp
std::unique_ptr<Expression> parseFactor();
```

- **功能**：解析因子表达式（*、/、% 运算符）
- **返回值**：因子表达式 AST 节点
- **优先级**：高于加减运算符

#### parseUnary

```cpp
std::unique_ptr<Expression> parseUnary();
```

- **功能**：解析一元表达式（NOT、+、-、~ 运算符）
- **返回值**：一元表达式 AST 节点
- **优先级**：高于乘除模运算符

#### parsePrimary

```cpp
std::unique_ptr<Expression> parsePrimary();
```

- **功能**：解析基本表达式（原子表达式）
- **返回值**：基本表达式 AST 节点
- **处理内容**：字面量、列引用、函数调用、括号表达式、子查询
- **优先级**：最高优先级

#### parseIdentifierExpression

```cpp
std::unique_ptr<Expression> parseIdentifierExpression();
```

- **功能**：解析标识符表达式
- **返回值**：标识符表达式 AST 节点
- **处理内容**：列引用（table.column）和函数调用（func(args)）

### 4.3 私有辅助方法

#### parseBinaryExpression

```cpp
std::unique_ptr<Expression> parseBinaryExpression(int minPrecedence);
```

- **功能**：通用的二元表达式解析器
- **参数**：
  - `minPrecedence` - 最小优先级
- **返回值**：解析后的二元表达式 AST 节点
- **设计意图**：实现优先级驱动的解析

#### getPrecedence

```cpp
int getPrecedence(OperatorKind op) const;
```

- **功能**：获取运算符的优先级
- **参数**：
  - `op` - 运算符类型
- **返回值**：优先级值（数值越大，优先级越高）

#### tokenToOperatorKind

```cpp
OperatorKind tokenToOperatorKind(Type tokenType) const;
```

- **功能**：将 Token 类型转换为 OperatorKind
- **参数**：
  - `tokenType` - Token 类型
- **返回值**：对应的 OperatorKind

#### parseBinaryOp

```cpp
std::unique_ptr<Expression> parseBinaryOp(
    std::function<std::unique_ptr<Expression>()> parseNextLevel,
    const std::vector<Type>& operators);
```

- **功能**：解析特定的二元运算符
- **参数**：
  - `parseNextLevel` - 解析下一级表达式的函数
  - `operators` - 要解析的运算符列表
- **返回值**：解析后的二元表达式 AST 节点
- **状态**：遗留方法，将被移除

### 4.4 私有成员

```cpp
TokenStream& tokens_;
```

- **功能**：Token 流引用，提供 token 访问接口
- **设计意图**：通过依赖注入获得，确保松耦合

## 5. 实现细节

### 5.1 递归下降解析算法

ExpressionParser 使用经典的递归下降解析算法，具有以下特点：

- **优先级驱动**：从低优先级到高优先级逐层解析
- **左结合**：大多数运算符使用左结合解析
- **预测分析**：根据当前 token 预测解析路径
- **错误恢复**：遇到错误时提供有意义的错误信息

### 5.2 解析顺序

解析顺序从低优先级到高优先级：

1. OR (逻辑或) - 最低优先级
2. AND (逻辑与)
3. =, !=, <, >, <=, >=, LIKE, IN, BETWEEN (比较)
4. +, - (加减)
5. *, /, % (乘除模)
6. NOT, unary +/-/~ (一元运算符)
7. 函数调用、列引用、字面量、括号表达式 (原子表达式) - 最高优先级

### 5.3 优先级处理

使用 `parseBinaryExpression` 方法实现优先级驱动的解析：

1. 首先解析左侧的高优先级表达式
2. 检查当前 token 是否为运算符
3. 如果是运算符，获取其优先级
4. 如果优先级大于等于 `minPrecedence`，则继续解析右侧的高优先级表达式
5. 创建二元表达式节点并返回

## 6. 性能优化

### 6.1 预分配内存

使用 `std::unique_ptr` 管理 AST 节点内存，避免内存泄漏并提高内存访问效率：

```cpp
std::unique_ptr<Expression> parsePrimary() {
    // ...
    return std::make_unique<LiteralExpression>(value);
    // ...
}
```

### 6.2 减少复制

通过引用和指针传递数据，减少不必要的数据复制：

```cpp
explicit ExpressionParser(TokenStream& tokens) : tokens_(tokens) {}
```

### 6.3 高效的优先级查询

使用简单的整数优先级值，实现高效的优先级比较：

```cpp
int getPrecedence(OperatorKind op) const {
    switch(op) {
        case OperatorKind::Or:
            return 1;
        case OperatorKind::And:
            return 2;
        // ...
    }
}
```

## 7. 扩展点

### 7.1 新运算符支持

添加新运算符只需：

1. 在 `OperatorKind` 枚举中添加新运算符
2. 在 `tokenToOperatorKind` 方法中添加 token 到新运算符的映射
3. 在 `getPrecedence` 方法中设置新运算符的优先级

### 7.2 新表达式类型

添加新的表达式类型只需：

1. 创建新的 AST 节点类，继承自 `Expression`
2. 在 `parsePrimary` 或其他合适的解析方法中添加解析逻辑

### 7.3 新的字面量类型

支持新的字面量类型只需在 `parsePrimary` 方法中添加相应的解析逻辑：

```cpp
std::unique_ptr<Expression> parsePrimary() {
    // ...
    if (tokens_.match(Type::NumericLiteral)) {
        return std::make_unique<LiteralExpression>(tokens_.getCurrentToken().value);
    }
    // 添加新的字面量类型解析
    else if (tokens_.match(Type::NewLiteralType)) {
        return std::make_unique<NewLiteralExpression>(tokens_.getCurrentToken().value);
    }
    // ...
}
```

## 8. 错误处理

ExpressionParser 采用以下错误处理策略：

1. **预测性错误恢复**：在解析过程中检查预期的 token，如果不匹配则提供有意义的错误信息
2. **友好的错误提示**：提供详细的错误位置和期望的 token 类型
3. **异常抛出**：遇到无法恢复的错误时抛出异常

## 9. 测试支持

ExpressionParser 设计为易于测试的组件：

1. **独立测试**：可以独立于主解析器进行测试
2. **测试用例丰富**：可以为各种表达式类型编写专门的测试用例
3. **依赖注入**：通过注入 TokenStream 模拟不同的输入场景

## 10. 使用示例

### 10.1 基本使用

```cpp
// 创建 TokenStream
TokenStream tokens(lexer.getTokens());

// 创建 ExpressionParser
ExpressionParser exprParser(tokens);

// 解析表达式
std::unique_ptr<Expression> expr = exprParser.parseExpression();
```

### 10.2 在主解析器中使用

```cpp
class Parser {
public:
    // ...
    std::unique_ptr<Statement> parseSelect() {
        // ...
        // 解析 WHERE 条件
        if (tokens_.match(Type::Where)) {
            ExpressionParser exprParser(tokens_);
            where_clause = exprParser.parseExpression();
        }
        // ...
    }
    // ...
};
```

### 10.3 测试用例

```cpp
void testArithmeticExpression() {
    // 输入：1 + 2 * 3
    std::vector<Token> tokens = {
        {Type::NumericLiteral, "1"},
        {Type::Plus, "+"},
        {Type::NumericLiteral, "2"},
        {Type::Star, "*"},
        {Type::NumericLiteral, "3"},
        {Type::EndOfFile, ""}
    };
    
    TokenStream tokenStream(tokens);
    ExpressionParser exprParser(tokenStream);
    
    std::unique_ptr<Expression> expr = exprParser.parseExpression();
    
    // 验证解析结果
    // ...
}
```

## 11. 总结

ExpressionParser 是 SQLCC 数据库系统中的关键组件，通过单一职责原则和模块化设计，实现了高效、可维护、可扩展的 SQL 表达式解析功能。它采用经典的递归下降解析算法，正确处理运算符优先级，并支持各种 SQL 表达式类型的解析。其设计符合 SOLID 原则，便于测试和扩展，为整个 SQL 解析系统提供了坚实的基础。