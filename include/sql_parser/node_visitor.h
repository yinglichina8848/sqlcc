#ifndef SQLCC_SQL_PARSER_NODE_VISITOR_H
#define SQLCC_SQL_PARSER_NODE_VISITOR_H

#include "ast_node.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace sqlcc {
namespace sql_parser {

// Forward declarations for all AST node classes
class CreateStatement;
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
class Expression;

class NodeVisitor {
public:
    virtual ~NodeVisitor() = default;
    
    // 为每种具体的AST节点类型提供visit方法
    virtual void visit(CreateStatement& node) = 0;
    virtual void visit(SelectStatement& node) = 0;
    virtual void visit(InsertStatement& node) = 0;
    virtual void visit(UpdateStatement& node) = 0;
    virtual void visit(DeleteStatement& node) = 0;
    virtual void visit(DropStatement& node) = 0;
    virtual void visit(AlterStatement& node) = 0;
    virtual void visit(UseStatement& node) = 0;
    virtual void visit(CreateIndexStatement& node) = 0;
    virtual void visit(DropIndexStatement& node) = 0;
    virtual void visit(CreateUserStatement& node) = 0;
    virtual void visit(DropUserStatement& node) = 0;
    virtual void visit(GrantStatement& node) = 0;
    virtual void visit(RevokeStatement& node) = 0;
    virtual void visit(ShowStatement& node) = 0;
    
    // 表达式访问方法
    virtual void visit(Expression& node) = 0;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_NODE_VISITOR_H