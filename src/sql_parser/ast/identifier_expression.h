#pragma once

#include "ast_node.h"
#include <string>

namespace sql::ast {

class IdentifierExpression : public Expression {
public:
  explicit IdentifierExpression(std::string name)
      : name_(std::move(name)) {}

  const std::string& name() const { return name_; }

  void accept(NodeVisitor& visitor) override {
    visitor.visit(*this);
  }

private:
  std::string name_;
};

} // namespace sql::ast
