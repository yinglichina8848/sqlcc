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
class SelectStatement;
class InsertStatement;
class UpdateStatement;
class DeleteStatement;
class CreateStatement;
class DropStatement;
class AlterStatement;
class CreateIndexStatement;
class DropIndexStatement;
class CreateUserStatement;
class DropUserStatement;
class CreateRoleStatement;
class DropRoleStatement;
class GrantRoleStatement;
class RevokeRoleStatement;
class CreateProcedureStatement;
class CallProcedureStatement;
class DropProcedureStatement;
class CreateTriggerStatement;
class AlterTriggerStatement;
class DropTriggerStatement;

namespace ast {

class NodeVisitor {
public:
  virtual ~NodeVisitor() = default;

  // Expression visitor methods
  virtual void visit(const Expression&) {}
  virtual void visit(const NumericLiteralExpression&) {}
  virtual void visit(const StringLiteralExpression&) {}
  virtual void visit(const BooleanLiteralExpression&) {}
  virtual void visit(const NullLiteralExpression&) {}
  virtual void visit(const IdentifierExpression&) {}
  virtual void visit(const FunctionCallExpression&) {}
  virtual void visit(const BinaryExpression&) {}
  virtual void visit(const WindowFunction&) {}

  // Statement visitor methods
  virtual void visit(const WithRecursiveClause&) {}
  virtual void visit(const WindowSpecification&) {}  // 添加WindowSpecification的visit方法

  // Utility statement visitor methods
  virtual void visit(const CommitStatement&) {}
  virtual void visit(const RollbackStatement&) {}
  virtual void visit(const BeginStatement&) {}
  virtual void visit(const UseStatement&) {}
  virtual void visit(const ShowStatement&) {}
  virtual void visit(const LoadDataStatement&) {}
  virtual void visit(const GrantStatement&) {}
  virtual void visit(const RevokeStatement&) {}

  // View statement visitor methods
  virtual void visit(const CreateViewStatement&) {}
  virtual void visit(const AlterViewStatement&) {}
  virtual void visit(const DropViewStatement&) {}

  // DML statement visitor methods
  virtual void visit(const SelectStatement&) {}
  virtual void visit(const InsertStatement&) {}
  virtual void visit(const UpdateStatement&) {}
  virtual void visit(const DeleteStatement&) {}

  // DDL statement visitor methods
  virtual void visit(const CreateStatement&) {}
  virtual void visit(const DropStatement&) {}
  virtual void visit(const AlterStatement&) {}
  virtual void visit(const CreateIndexStatement&) {}
  virtual void visit(const DropIndexStatement&) {}
  virtual void visit(const CreateUserStatement&) {}
  virtual void visit(const DropUserStatement&) {}
  virtual void visit(const CreateRoleStatement&) {}
  virtual void visit(const DropRoleStatement&) {}
  virtual void visit(const GrantRoleStatement&) {}
  virtual void visit(const RevokeRoleStatement&) {}
  virtual void visit(const CreateProcedureStatement&) {}
  virtual void visit(const CallProcedureStatement&) {}
  virtual void visit(const DropProcedureStatement&) {}
  virtual void visit(const CreateTriggerStatement&) {}
  virtual void visit(const AlterTriggerStatement&) {}
  virtual void visit(const DropTriggerStatement&) {}
};

} // namespace ast
} // namespace sql_parser
} // namespace sqlcc