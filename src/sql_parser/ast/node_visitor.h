#pragma once

#include "expression.h"
#include "statement.h"

namespace sqlcc {
namespace sql_parser {

class WindowFunction;
class WithRecursiveClause;
class WindowSpecification;  // 添加WindowSpecification前向声明

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
  virtual void visit(WindowFunction&) {}
  
  // Statement visitor methods
  virtual void visit(WithRecursiveClause&) {}
  virtual void visit(WindowSpecification&) {}  // 添加WindowSpecification的visit方法
};

} // namespace ast
} // namespace sql_parser
} // namespace sqlcc