#ifndef SQLCC_SQL_PARSER_AST_AST_NODE_H
#define SQLCC_SQL_PARSER_AST_AST_NODE_H

#include <memory>
#include "node_visitor.h"

namespace sqlcc {
namespace sql_parser {

class NodeVisitor;

class ASTNode {
public:
  virtual ~ASTNode() = default;
  virtual void accept(NodeVisitor& visitor) = 0;
};

class Expression : public ASTNode {
public:
  enum Type {
    IDENTIFIER,
    STRING_LITERAL,
    NUMERIC_LITERAL,
    BOOLEAN_LITERAL,
    NULL_LITERAL,
    FUNCTION
  };

  virtual ~Expression() = default;
  virtual Type getType() const = 0;
  virtual std::string getTypeName() const = 0;
};

using ExprPtr = std::unique_ptr<Expression>;

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_AST_AST_NODE_H
