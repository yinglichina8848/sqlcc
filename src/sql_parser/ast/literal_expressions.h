#pragma once
#include "ast_node.h"
#include <string>

namespace sqlcc::sql_parser {

// ---------- Numeric ----------
class NumericLiteralExpression : public Expression {
public:
  explicit NumericLiteralExpression(double v) : value(v) {}
  double getValue() const { return value; }

  virtual std::string getTypeName() const override { return "NumericLiteralExpression"; }
  virtual void accept(NodeVisitor &visitor) override;
  virtual Type getType() const override { return NUMERIC_LITERAL; }

private:
  double value;
};

// ---------- String ----------
class StringLiteralExpression : public Expression {
public:
  explicit StringLiteralExpression(std::string v) : value(std::move(v)) {}
  const std::string& getValue() const { return value; }

  virtual std::string getTypeName() const override { return "StringLiteralExpression"; }
  virtual void accept(NodeVisitor &visitor) override;
  virtual Type getType() const override { return STRING_LITERAL; }

private:
  std::string value;
};

// ---------- Boolean ----------
class BooleanLiteralExpression : public Expression {
public:
  explicit BooleanLiteralExpression(bool v) : value(v) {}
  bool getValue() const { return value; }

  virtual std::string getTypeName() const override { return "BooleanLiteralExpression"; }
  virtual void accept(NodeVisitor &visitor) override;
  virtual Type getType() const override { return BOOLEAN_LITERAL; }

private:
  bool value;
};

// ---------- Null ----------
class NullLiteralExpression : public Expression {
public:
  NullLiteralExpression() = default;

  virtual std::string getTypeName() const override { return "NullLiteralExpression"; }
  virtual void accept(NodeVisitor &visitor) override;
  virtual Type getType() const override { return NULL_LITERAL; }
};

} // namespace sqlcc::sql_parser
