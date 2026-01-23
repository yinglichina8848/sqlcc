#pragma once

namespace sql::ast {

class Expression;
class NumericLiteralExpression;
class StringLiteralExpression;
class BooleanLiteralExpression;
class NullLiteralExpression;
class IdentifierExpression;
class FunctionCallExpression;

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
};

} // namespace sql::ast
