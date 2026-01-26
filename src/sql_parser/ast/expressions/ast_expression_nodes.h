/**
 * ASTExpressionNodes - 表达式相关AST节点头文件
 * 
 * 包含表达式相关的AST节点定义，包括：
 * - 二元表达式：BinaryExpression（算术、比较、逻辑运算）
 * - 一元表达式：UnaryExpression（正负、NOT运算）
 * - 字面量表达式：LiteralExpression（数值、字符串、布尔值）
 * - 函数调用表达式：FunctionCallExpression
 * - 标识符表达式：IdentifierExpression（列名、变量名）
 * 
 * 设计原则：
 * - 单一职责：专门处理表达式相关AST节点
 * - 模块化：按表达式类型分类组织节点定义
 * - 类型安全：强类型系统防止运行时错误
 */

#ifndef SQLCC_SQL_PARSER_AST_EXPRESSIONS_AST_EXPRESSION_NODES_H
#define SQLCC_SQL_PARSER_AST_EXPRESSIONS_AST_EXPRESSION_NODES_H

#include "../ast_node.h"
#include "../expression.h"
#include "../operator_kind.h"
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {
namespace sql_parser {

// ==================== BinaryExpression ====================

class BinaryExpression : public Expression {
public:
    BinaryExpression(std::unique_ptr<Expression> left, OperatorKind op, std::unique_ptr<Expression> right);
    ~BinaryExpression() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std::unique_ptr<Expression> &getLeft() const { return left_; }
    OperatorKind getOperator() const { return op_; }
    const std::unique_ptr<Expression> &getRight() const { return right_; }
    
    // Setters
    void setLeft(std::unique_ptr<Expression> left);
    void setOperator(OperatorKind op) { op_ = op; }
    void setRight(std::unique_ptr<Expression> right);

private:
    std::unique_ptr<Expression> left_;
    OperatorKind op_;
    std::unique_ptr<Expression> right_;
};

// ==================== UnaryExpression ====================

class UnaryExpression : public Expression {
public:
    UnaryExpression(OperatorKind op, std::unique_ptr<Expression> operand);
    ~UnaryExpression() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    OperatorKind getOperator() const { return op_; }
    const std::unique_ptr<Expression> &getOperand() const { return operand_; }
    
    // Setters
    void setOperator(OperatorKind op) { op_ = op; }
    void setOperand(std::unique_ptr<Expression> operand);

private:
    OperatorKind op_;
    std::unique_ptr<Expression> operand_;
};

// ==================== LiteralExpression ====================

class LiteralExpression : public Expression {
public:
    LiteralExpression(const std::string &value, LiteralType type);
    ~LiteralExpression() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std::string &getValue() const { return value_; }
    LiteralType getType() const { return type_; }
    
    // Setters
    void setValue(const std::string &value) { value_ = value; }
    void setType(LiteralType type) { type_ = type; }

private:
    std::string value_;
    LiteralType type_;
};

// ==================== FunctionCallExpression ====================

class FunctionCallExpression : public Expression {
public:
    FunctionCallExpression(const std::string &functionName);
    ~FunctionCallExpression() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std::string &getFunctionName() const { return functionName_; }
    const std::vector<std::unique_ptr<Expression>> &getArguments() const { return arguments_; }
    
    // Setters
    void setFunctionName(const std::string &name) { functionName_ = name; }
    void setArguments(std::vector<std::unique_ptr<Expression>> arguments);

private:
    std::string functionName_;
    std::vector<std::unique_ptr<Expression>> arguments_;
};

// ==================== IdentifierExpression ====================

class IdentifierExpression : public Expression {
public:
    IdentifierExpression(const std::string &name);
    ~IdentifierExpression() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std::string &getName() const { return name_; }
    
    // Setters
    void setName(const std::string &name) { name_ = name; }

private:
    std::string name_;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_AST_EXPRESSIONS_AST_EXPRESSION_NODES_H