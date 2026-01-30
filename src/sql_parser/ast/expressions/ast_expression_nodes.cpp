/**
 * @file ast_expression_nodes.cpp
 * @brief 表达式相关AST节点实现
 *
 * 包含表达式相关的AST节点实现，包括：
 * - BinaryExpression（二元表达式）
 * - UnaryExpression（一元表达式）
 * - LiteralExpression（字面量表达式）
 * - FunctionCallExpression（函数调用表达式）
 */

#include "src/sql_parser/ast/ast_node.h"
#include "src/sql_parser/ast/ast_nodes.h"
#include "src/sql_parser/ast/expressions/ast_expression_nodes.h"
#include <iostream>

namespace sqlcc {
namespace sql_parser {

// ==================== BinaryExpression ====================

BinaryExpression::BinaryExpression(std::unique_ptr<Expression> left,
                                   BinaryOperator op,
                                   std::unique_ptr<Expression> right)
    : Expression(BINARY_EXPRESSION), left_(std::move(left)), op_(op),
      right_(std::move(right)) {}

BinaryExpression::~BinaryExpression() {}

void BinaryExpression::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

const Expression *BinaryExpression::getLeft() const { return left_.get(); }

BinaryExpression::BinaryOperator BinaryExpression::getOperator() const { return op_; }

const Expression *BinaryExpression::getRight() const { return right_.get(); }

// ==================== UnaryExpression ====================

UnaryExpression::UnaryExpression(UnaryOperator op, std::unique_ptr<Expression> operand)
    : Expression(UNARY_EXPRESSION), op_(op), operand_(std::move(operand)) {}

UnaryExpression::~UnaryExpression() {}

void UnaryExpression::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

UnaryExpression::UnaryOperator UnaryExpression::getOperator() const { return op_; }

const Expression *UnaryExpression::getOperand() const { return operand_.get(); }

// ==================== LiteralExpression ====================

LiteralExpression::LiteralExpression(const std::string &value, LiteralType type)
    : Expression(LITERAL_EXPRESSION), value_(value), type_(type) {}

LiteralExpression::~LiteralExpression() {}

void LiteralExpression::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

const std::string &LiteralExpression::getValue() const { return value_; }

LiteralExpression::LiteralType LiteralExpression::getType() const { return type_; }

// ==================== FunctionCallExpression ====================

FunctionCallExpression::FunctionCallExpression(const std::string &functionName)
    : Expression(FUNCTION_CALL_EXPRESSION), functionName_(functionName) {}

FunctionCallExpression::~FunctionCallExpression() {}

void FunctionCallExpression::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

void FunctionCallExpression::addArgument(std::unique_ptr<Expression> argument) {
  arguments_.push_back(std::move(argument));
}

const std::string &FunctionCallExpression::getFunctionName() const {
  return functionName_;
}

const std::vector<std::unique_ptr<Expression>> &
FunctionCallExpression::getArguments() const {
  return arguments_;
}

// ==================== IdentifierExpression ====================

IdentifierExpression::IdentifierExpression(const std::string &identifier)
    : Expression(IDENTIFIER_EXPRESSION), identifier_(identifier) {}

IdentifierExpression::~IdentifierExpression() {}

void IdentifierExpression::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

const std::string &IdentifierExpression::getIdentifier() const {
  return identifier_;
}

} // namespace sql_parser
} // namespace sqlcc