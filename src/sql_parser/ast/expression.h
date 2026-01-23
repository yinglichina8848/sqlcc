#pragma once

#include "ast_node.h"
#include "../operator_kind.h"
#include <memory>
#include <string>

namespace sqlcc {
namespace sql_parser {
namespace ast {

enum class ExpressionType {
  NumericLiteral,
  StringLiteral,
  BooleanLiteral,
  NullLiteral,
  Identifier,
  FunctionCall,
  Binary,
};

class NodeVisitor;

class Expression : public ASTNode {
public:
  virtual ~Expression() = default;
  virtual ExpressionType getType() const = 0;
  virtual void accept(NodeVisitor& visitor) = 0;
};

using ExprPtr = std::unique_ptr<Expression>;

class NumericLiteralExpression : public Expression {
public:
  explicit NumericLiteralExpression(double value) : value_(value) {}
  double getValue() const { return value_; }
  ExpressionType getType() const override { return ExpressionType::NumericLiteral; }
  void accept(NodeVisitor& visitor) override;

private:
  double value_;
};

class StringLiteralExpression : public Expression {
public:
  explicit StringLiteralExpression(const std::string& value) : value_(value) {}
  const std::string& getValue() const { return value_; }
  ExpressionType getType() const override { return ExpressionType::StringLiteral; }
  void accept(NodeVisitor& visitor) override;

private:
  std::string value_;
};

class BooleanLiteralExpression : public Expression {
public:
  explicit BooleanLiteralExpression(bool value) : value_(value) {}
  bool getValue() const { return value_; }
  ExpressionType getType() const override { return ExpressionType::BooleanLiteral; }
  void accept(NodeVisitor& visitor) override;

private:
  bool value_;
};

class NullLiteralExpression : public Expression {
public:
  ExpressionType getType() const override { return ExpressionType::NullLiteral; }
  void accept(NodeVisitor& visitor) override;
};

class IdentifierExpression : public Expression {
public:
  explicit IdentifierExpression(const std::string& name) : name_(name) {}
  const std::string& getName() const { return name_; }
  ExpressionType getType() const override { return ExpressionType::Identifier; }
  void accept(NodeVisitor& visitor) override;

private:
  std::string name_;
};

class FunctionCallExpression : public Expression {
public:
  FunctionCallExpression(const std::string& name, std::vector<ExprPtr> arguments)
      : name_(name), arguments_(std::move(arguments)) {}
  const std::string& getName() const { return name_; }
  const std::vector<ExprPtr>& getArguments() const { return arguments_; }
  ExpressionType getType() const override { return ExpressionType::FunctionCall; }
  void accept(NodeVisitor& visitor) override;

private:
  std::string name_;
  std::vector<ExprPtr> arguments_;
};

class BinaryExpression : public Expression {
public:
  BinaryExpression(ExprPtr left, ExprPtr right, OperatorKind op)
      : left_(std::move(left)), right_(std::move(right)), op_(op) {}
  const Expression& getLeft() const { return *left_; }
  const Expression& getRight() const { return *right_; }
  OperatorKind getOperator() const { return op_; }
  ExpressionType getType() const override { return ExpressionType::Binary; }
  void accept(NodeVisitor& visitor) override;

private:
  ExprPtr left_;
  ExprPtr right_;
  OperatorKind op_;
};

} // namespace ast
} // namespace sql_parser
} // namespace sqlcc

// Global namespace aliases for backward compatibility
using Expression = sqlcc::sql_parser::ast::Expression;
using NumericLiteralExpression = sqlcc::sql_parser::ast::NumericLiteralExpression;
using StringLiteralExpression = sqlcc::sql_parser::ast::StringLiteralExpression;
using BooleanLiteralExpression = sqlcc::sql_parser::ast::BooleanLiteralExpression;
using NullLiteralExpression = sqlcc::sql_parser::ast::NullLiteralExpression;
using IdentifierExpression = sqlcc::sql_parser::ast::IdentifierExpression;
using FunctionCallExpression = sqlcc::sql_parser::ast::FunctionCallExpression;
using BinaryExpression = sqlcc::sql_parser::ast::BinaryExpression;
using OperatorKind = sqlcc::sql_parser::OperatorKind;
using ExprPtr = sqlcc::sql_parser::ast::ExprPtr;
