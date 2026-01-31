# Parser 类文档

## 类概述

`Parser` 是 SQLCC 的核心 SQL 解析器，采用 **递归下降解析器（Recursive Descent Parser）** 架构，实现完整的 SQL-92 标准语法分析。

## WHY: 为什么需要专门的 SQL 解析器？

**设计动机**：数据库系统需要将用户输入的 SQL 字符串转换为可执行的内部表示，传统解析方案存在诸多问题：

1. **语法复杂**：SQL 语法包含递归嵌套结构、运算符优先级、多重子句
2. **错误定位**：解析错误时需要提供精确的位置和原因信息
3. **性能要求**：解析过程需要高效，避免成为系统瓶颈
4. **扩展性**：需要支持新 SQL 特性的快速添加
5. **标准兼容**：必须完全符合 SQL 标准语法规范

**SQL解析器核心价值**：
- 语法正确性：严格验证 SQL 语句的语法和语义正确性
- 结构化表示：将 SQL 字符串转换为 AST 树结构
- 错误恢复：提供友好的错误信息和解析继续能力
- 性能优化：高效的解析算法和内存管理
- 标准兼容：完全支持 SQL 标准的各种语法特性

**为什么选择递归下降解析器**：
- 代码结构清晰，每个非终结符对应一个函数
- 错误恢复能力强，便于提供精确的错误信息
- 易于维护和扩展，支持新语法规则的添加
- 执行效率高，无需复杂的解析表

## WHAT: 核心功能

### 词法分析（Lexer）
- **Token 生成**：将 SQL 字符串转换为 Token 流
- **关键字识别**：识别 SQL 关键字和标识符
- **字面量解析**：解析字符串、数字、日期等字面量

### 语法分析（Parser）
- **DDL 语句**：`CREATE`, `ALTER`, `DROP` 等
- **DML 语句**：`SELECT`, `INSERT`, `UPDATE`, `DELETE` 等
- **DCL 语句**：`GRANT`, `REVOKE` 等
- **TCL 语句**：`COMMIT`, `ROLLBACK`, `SAVEPOINT` 等

### 语义验证
- **类型检查**：验证操作数的类型兼容性
- **表/列存在性**：验证引用的表和列是否存在
- **约束验证**：检查约束条件是否满足

### 错误处理
- **语法错误**：检测不符合 SQL 语法规则的情况
- **语义错误**：检测语义不正确但语法正确的情况
- **位置信息**：提供错误的精确位置信息
- **错误恢复**：尝试从错误中恢复继续解析

## 模块化架构

```
Parser (主解析器)
├── Lexer (词法分析器)
├── ParserDDL (DDL 解析器)
├── ParserDML (DML 解析器)
├── ParserDCL (DCL 解析器)
├── ParserTCL (TCL 解析器)
└── ExpressionParser (表达式解析器)
```

## HOW: 实现机制

### 解析器初始化流程
1. 创建 Lexer 对象处理输入字符串
2. 设置当前位置和前瞻 Token
3. 初始化用于错误恢复的同步 Token 集合
4. 准备收集解析错误的容器

### 主解析循环流程
1. 语句识别：检查当前 Token 是否表示语句开始
2. 语句解析：根据语句类型调用相应的解析函数
3. 语句分隔：处理语句间的分号分隔符
4. 错误处理：记录解析失败时的错误信息
5. 继续循环：继续解析下一个语句直到输入结束

### 表达式解析实现
1. 优先级驱动：使用运算符优先级控制解析顺序
2. 递归下降：从低优先级到高优先级逐层解析
3. 括号处理：正确处理括号表达式和嵌套结构
4. 函数调用：识别和解析函数调用表达式
5. 类型转换：构建相应的 AST 节点表示表达式结构

### 错误恢复机制（恐慌模式）
1. 检测到错误时进入恐慌模式
2. 跳过错误 Token 直到遇到同步 Token
3. 重置解析状态准备继续解析
4. 记录错误信息但不中断解析过程
5. 尝试从下一个有效位置继续解析

## 使用示例

```cpp
#include "sql_parser/parser.h"
#include "sql_parser/ast/ast_nodes.h"

// 创建解析器
sqlcc::sql_parser::Parser parser("SELECT id, name FROM users WHERE age > 18");

// 执行解析
auto statements = parser.parse();

// 检查错误
if (parser.hadError()) {
    auto errors = parser.getDetailedErrors();
    for (const auto& error : errors) {
        std::cerr << error << std::endl;
    }
    return;
}

// 处理解析结果
for (auto& stmt : statements) {
    // 根据语句类型处理
    switch (stmt->getType()) {
        case sql_parser::Statement::Type::SELECT:
            // 处理 SELECT 语句
            break;
        case sql_parser::Statement::Type::INSERT:
            // 处理 INSERT 语句
            break;
        // ...
    }
}
```

## 支持的 SQL 语句

### DDL 语句
- `CREATE TABLE`, `CREATE INDEX`, `CREATE DATABASE`
- `ALTER TABLE`
- `DROP TABLE`, `DROP INDEX`, `DROP DATABASE`

### DML 语句
- `SELECT`, `SELECT ... FROM ... WHERE ...`
- `INSERT INTO ... VALUES ...`
- `UPDATE ... SET ... WHERE ...`
- `DELETE FROM ... WHERE ...`

### DCL 语句
- `CREATE USER`, `DROP USER`
- `GRANT`, `REVOKE`

### TCL 语句
- `BEGIN`, `COMMIT`, `ROLLBACK`
- `SAVEPOINT`, `ROLLBACK TO`

## 设计模式

**递归下降解析器（Recursive Descent Parser）**：
- 自顶向下：从语法树的根节点开始逐步构建
- 函数映射：每个非终结符对应一个解析函数
- 预测分析：根据当前 Token 预测解析路径
- 错误恢复：遇到错误时尝试恢复继续解析
- 状态管理：维护解析状态和错误信息

## 版本信息

- **版本**: v1.3.9
- **最后更新**: 2026-01-31
- **C++标准**: C++20
- **编译器**: Clang 18+
- **SQL标准**: SQL-92 完整支持