#pragma once

#include "expression.h"
#include "statement.h"

namespace sqlcc {
namespace sql_parser {

class WindowFunction;
class WithRecursiveClause;
class WindowSpecification;  // 添加WindowSpecification前向声明

class CommitStatement;
class RollbackStatement;
class BeginStatement;
class UseStatement;
class ShowStatement;
class LoadDataStatement;
class GrantStatement;
class RevokeStatement;
class CreateViewStatement;
class AlterViewStatement;
class DropViewStatement;

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

  // Utility statement visitor methods
  virtual void visit(CommitStatement&) {}
  virtual void visit(RollbackStatement&) {}
  virtual void visit(BeginStatement&) {}
  virtual void visit(UseStatement&) {}
  virtual void visit(ShowStatement&) {}
  virtual void visit(LoadDataStatement&) {}
  virtual void visit(GrantStatement&) {}
  virtual void visit(RevokeStatement&) {}

  // View statement visitor methods
  virtual void visit(CreateViewStatement&) {}
  virtual void visit(AlterViewStatement&) {}
  virtual void visit(DropViewStatement&) {}
};

} // namespace ast
} // namespace sql_parser
} // namespace sqlcc