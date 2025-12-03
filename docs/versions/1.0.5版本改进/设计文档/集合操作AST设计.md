# 集合操作AST设计文档

## 阶段概述
- **阶段名称**: 第一阶段 - 语法扩展和AST设计
- **目标版本**: v1.0.6
- **时间安排**: 预计2周
- **主要任务**: 扩展SQL解析器支持集合操作语法，设计AST节点结构

## 当前状态分析

### 现有AST结构分析
通过对`/home/liying/sqlcc_qoder/include/sql_parser/ast_nodes.h`的分析，当前AST结构包含：

1. **基础语句类**: Statement（抽象基类）
2. **查询语句类**: SelectStatement（已实现基础功能）
3. **其他语句类**: CreateStatement、InsertStatement、UpdateStatement等

### 缺失功能
1. **集合操作节点**: 缺少表示UNION、INTERSECT、EXCEPT的AST节点
2. **语法解析**: 解析器不支持集合操作语法
3. **类型定义**: 缺少集合操作相关的枚举和类型定义

## 技术设计方案

### 1. AST节点扩展设计

#### SetOperationStatement类设计
```cpp
class SetOperationStatement : public Statement {
public:
    enum OperationType {
        UNION,      // UNION操作
        UNION_ALL,  // UNION ALL操作
        INTERSECT,  // INTERSECT操作
        EXCEPT      // EXCEPT操作
    };
    
    SetOperationStatement(OperationType opType, 
                         std::unique_ptr<SelectStatement> left,
                         std::unique_ptr<SelectStatement> right);
    
    ~SetOperationStatement();
    
    // Getter方法
    OperationType getOperationType() const;
    const SelectStatement& getLeftOperand() const;
    const SelectStatement& getRightOperand() const;
    
    // Visitor模式支持
    void accept(NodeVisitor &visitor) override;
    
private:
    OperationType operationType_;
    std::unique_ptr<SelectStatement> leftOperand_;
    std::unique_ptr<SelectStatement> rightOperand_;
};
```

#### SelectStatement扩展设计
修改现有的SelectStatement类，使其能够作为集合操作的操作数：
```cpp
class SelectStatement : public Statement {
public:
    // 现有方法保持不变
    // ...
    
    // 新增方法：支持嵌套集合操作
    bool isSetOperation() const;
    void setSetOperation(std::unique_ptr<SetOperationStatement> setOp);
    const SetOperationStatement* getSetOperation() const;
    
private:
    // 新增成员：支持嵌套集合操作
    std::unique_ptr<SetOperationStatement> setOperation_;
};
```

### 2. 语法规则设计

#### BNF语法规则（概念性）
```
select_statement ::= simple_select [set_operation select_statement]

set_operation ::= UNION [ALL] | INTERSECT | EXCEPT

simple_select ::= SELECT [DISTINCT] select_list
                 FROM table_reference
                 [WHERE search_condition]
                 [GROUP BY grouping_columns]
                 [ORDER BY order_columns]
```

#### 解析优先级
1. **最高优先级**: UNION ALL
2. **次高优先级**: UNION、INTERSECT、EXCEPT
3. **左结合性**: 所有集合操作符都是左结合的

### 3. 解析器扩展设计

#### Parser类扩展
在`/home/liying/sqlcc_qoder/src/sql_parser/parser.cpp`中新增方法：

```cpp
// 解析SELECT语句（支持集合操作）
std::unique_ptr<SelectStatement> Parser::parseSelectStatement() {
    // 解析第一个SELECT语句
    auto left = parseSimpleSelectStatement();
    
    // 检查是否有集合操作符
    while (isSetOperation()) {
        auto opType = parseSetOperation();
        auto right = parseSimpleSelectStatement();
        
        // 创建集合操作节点
        auto setOp = std::make_unique<SetOperationStatement>(opType, 
                                                           std::move(left), 
                                                           std::move(right));
        
        // 将集合操作设置为左操作数的子节点
        left->setSetOperation(std::move(setOp));
    }
    
    return left;
}

// 判断当前token是否为集合操作符
bool Parser::isSetOperation() {
    return match(Token::KEYWORD_UNION) || 
           match(Token::KEYWORD_INTERSECT) || 
           match(Token::KEYWORD_EXCEPT);
}

// 解析集合操作符
SetOperationStatement::OperationType Parser::parseSetOperation() {
    if (match(Token::KEYWORD_UNION)) {
        consume();
        if (match(Token::KEYWORD_ALL)) {
            consume();
            return SetOperationStatement::UNION_ALL;
        }
        return SetOperationStatement::UNION;
    } else if (match(Token::KEYWORD_INTERSECT)) {
        consume();
        return SetOperationStatement::INTERSECT;
    } else if (match(Token::KEYWORD_EXCEPT)) {
        consume();
        return SetOperationStatement::EXCEPT;
    }
    
    reportError("Expected set operation keyword");
    return SetOperationStatement::UNION; // 默认值
}
```

### 4. 词法分析器扩展

#### Token类型扩展
在`/home/liying/sqlcc_qoder/include/sql_parser/token.h`中新增token类型：
```cpp
class Token {
public:
    enum Type {
        // 现有类型...
        KEYWORD_UNION,      // UNION关键字
        KEYWORD_INTERSECT,  // INTERSECT关键字
        KEYWORD_EXCEPT,     // EXCEPT关键字
        KEYWORD_ALL         // ALL关键字（用于UNION ALL）
    };
};
```

#### Lexer关键字映射扩展
在`/home/liying/sqlcc_qoder/src/sql_parser/lexer.cpp`中更新关键字映射：
```cpp
void Lexer::initKeywordMap() {
    keywordMap_["UNION"] = Token::KEYWORD_UNION;
    keywordMap_["INTERSECT"] = Token::KEYWORD_INTERSECT;
    keywordMap_["EXCEPT"] = Token::KEYWORD_EXCEPT;
    keywordMap_["ALL"] = Token::KEYWORD_ALL;
    // 现有映射...
}
```

## 实现步骤

### 步骤1：扩展Token定义
1. 修改`token.h`添加新的token类型
2. 更新`lexer.cpp`的关键字映射
3. 编译测试词法分析器

### 步骤2：设计AST节点
1. 在`ast_nodes.h`中定义SetOperationStatement类
2. 扩展SelectStatement类支持集合操作
3. 实现相关的方法和构造函数

### 步骤3：扩展解析器
1. 在`parser.cpp`中添加集合操作解析方法
2. 修改现有的select语句解析逻辑
3. 实现语法错误处理

### 步骤4：测试验证
1. 编写单元测试验证语法解析
2. 测试AST构建的正确性
3. 验证错误处理机制

## 技术挑战和解决方案

### 挑战1：左递归解析
**问题**: SELECT语句可能包含嵌套的集合操作，形成左递归结构

**解决方案**: 使用迭代方式解析，避免递归深度过大

### 挑战2：操作符优先级
**问题**: 不同集合操作符可能有不同的优先级

**解决方案**: 按照SQL标准实现标准的优先级规则

### 挑战3：错误恢复
**问题**: 语法错误时需要能够恢复并继续解析

**解决方案**: 实现健壮的错误恢复机制

## 测试计划

### 单元测试用例
1. **基础语法测试**: 验证UNION、INTERSECT、EXCEPT的解析
2. **嵌套操作测试**: 测试多层嵌套的集合操作
3. **错误语法测试**: 验证语法错误的正确处理
4. **边界条件测试**: 测试空查询、单表查询等边界情况

### 集成测试用例
1. **AST构建测试**: 验证AST节点构建的正确性
2. **Visitor模式测试**: 测试AST遍历功能
3. **序列化测试**: 测试AST的序列化和反序列化

## 风险评估

### 技术风险
1. **解析复杂度**: 集合操作语法相对复杂，可能增加解析器复杂度
2. **内存管理**: unique_ptr的使用需要谨慎处理所有权转移

### 缓解措施
1. 分步骤实现，逐步验证
2. 使用智能指针避免内存泄漏
3. 充分的单元测试覆盖

## 下一步行动

1. **立即开始**: 实现Token和Lexer的扩展
2. **代码实现**: 按照设计文档实现AST节点
3. **测试验证**: 编写和运行测试用例
4. **文档更新**: 更新API文档和设计文档

---

**文档版本**: 1.0  
**创建日期**: 2024-01-20  
**最后更新**: 2024-01-20  
**负责人**: SQLCC开发团队