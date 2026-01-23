#pragma once

#include "expression.h"

namespace sqlcc {
namespace sql_parser {
namespace ast {

class NodeVisitor {
public:
  virtual ~NodeVisitor() = default;

  // Expression visitor methods
  virtual void visit(Expression&) {}
  virtual void visit(NumericLiteralExpression&) {}
  virtual void visit(StringLiteralExpression&) {}
  virtual void visit(BooleanLiteralExpression&) {}
  virtual void visit(NullLiteralExpression&) {}
  virtual void visit(IdentifierExpression&) {}
  virtual void visit(FunctionCallExpression&) {}
  virtual void visit(BinaryExpression&) {}
};

} // namespace ast
} // namespace sql_parser
} // namespace sqlcc
