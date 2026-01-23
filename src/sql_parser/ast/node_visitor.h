#pragma once

namespace sqlcc {
namespace sql_parser {

class Expression;
class NumericLiteralExpression;
class StringLiteralExpression;
class BooleanLiteralExpression;
class NullLiteralExpression;
class IdentifierExpression;
class FunctionCallExpression;
class BinaryExpression;

class NodeVisitor {
public:
  virtual ~NodeVisitor() = default;

  virtual void visit(Expression&) {}
  virtual void visit(NumericLiteralExpression&) {}
  virtual void visit(StringLiteralExpression&) {}
  virtual void visit(BooleanLiteralExpression&) {}
  virtual void visit(NullLiteralExpression&) {}
  virtual void visit(IdentifierExpression&) {}
  virtual void visit(FunctionCallExpression&) {}
  virtual void visit(BinaryExpression&) {}
};

} // namespace sql_parser
} // namespace sqlcc
