#ifndef SQLCC_SQL_PARSER_NODE_VISITOR_H
#define SQLCC_SQL_PARSER_NODE_VISITOR_H

#include "ast_node.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sqlcc {
namespace sql_parser {

// Forward declarations for all AST node classes
class CreateStatement;
class CreateViewStatement;
class SelectStatement;
class InsertStatement;
class UpdateStatement;
class DeleteStatement;
class DropStatement;
class AlterStatement;
class UseStatement;
class CreateIndexStatement;
class DropIndexStatement;
class CreateUserStatement;
class DropUserStatement;
class GrantStatement;
class RevokeStatement;
class ShowStatement;
class CommitStatement;
class RollbackStatement;
class BinaryExpression;
class IdentifierExpression;
class StringLiteralExpression;
class NumericLiteralExpression;
class BooleanLiteralExpression;
class NullLiteralExpression;
class CreateProcedureStatement;
class CallProcedureStatement;
class DropProcedureStatement;
class CreateTriggerStatement;
class DropTriggerStatement;
class AlterTriggerStatement;
class Expression;
class SetOperation;
class CompositeSelectStatement;
class WindowFunction;
class WindowSpecification;
class WithRecursiveClause;

class NodeVisitor {
public:
  virtual ~NodeVisitor() = default;

  // 为每种具体的AST节点类型提供visit方法
  virtual void visit(CreateStatement &node) = 0;
  virtual void visit(CreateViewStatement &node) = 0;
  virtual void visit(SelectStatement &node) = 0;
  virtual void visit(InsertStatement &node) = 0;
  virtual void visit(UpdateStatement &node) = 0;
  virtual void visit(DeleteStatement &node) = 0;
  virtual void visit(DropStatement &node) = 0;
  virtual void visit(AlterStatement &node) = 0;
  virtual void visit(UseStatement &node) = 0;
  virtual void visit(CreateIndexStatement &node) = 0;
  virtual void visit(DropIndexStatement &node) = 0;
  virtual void visit(CreateUserStatement &node) = 0;
  virtual void visit(DropUserStatement &node) = 0;
  virtual void visit(GrantStatement &node) = 0;
  virtual void visit(RevokeStatement &node) = 0;
  virtual void visit(ShowStatement &node) = 0;
  virtual void visit(CommitStatement &node) = 0;
  virtual void visit(RollbackStatement &node) = 0;
  virtual void visit(CreateProcedureStatement &node) = 0;
  virtual void visit(CallProcedureStatement &node) = 0;
  virtual void visit(DropProcedureStatement &node) = 0;
  virtual void visit(CreateTriggerStatement &node) = 0;
  virtual void visit(DropTriggerStatement &node) = 0;
  virtual void visit(AlterTriggerStatement &node) = 0;

  // 表达式访问方法
  virtual void visit(BinaryExpression &node) = 0;
  virtual void visit(IdentifierExpression &node) = 0;
  virtual void visit(StringLiteralExpression &node) = 0;
  virtual void visit(NumericLiteralExpression &node) = 0;
  virtual void visit(BooleanLiteralExpression &node) = 0;
  virtual void visit(NullLiteralExpression &node) = 0;

  // 集合操作访问方法
  virtual void visit(SetOperation &node) = 0;
  virtual void visit(CompositeSelectStatement &node) = 0;
  
  // 窗口函数访问方法
  virtual void visit(WindowFunction &node) = 0;
  virtual void visit(WindowSpecification &node) = 0;
  virtual void visit(WithRecursiveClause &node) = 0;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_NODE_VISITOR_H
