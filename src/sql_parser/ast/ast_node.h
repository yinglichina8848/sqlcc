#pragma once

#include <memory>

namespace sql::ast {

class NodeVisitor;

class ASTNode {
public:
  virtual ~ASTNode() = default;
  virtual void accept(NodeVisitor& visitor) = 0;
};

class Expression : public ASTNode {
public:
  ~Expression() override = default;
};

using ExprPtr = std::unique_ptr<Expression>;

} // namespace sql::ast
