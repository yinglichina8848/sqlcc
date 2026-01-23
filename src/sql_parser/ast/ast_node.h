#pragma once

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
  virtual ~Expression() = default;
};

using ExprPtr = std::unique_ptr<Expression>;

} // namespace sql_parser
} // namespace sqlcc
#endif // SQLCC_SQL_PARSER_AST_AST_NODE_H
