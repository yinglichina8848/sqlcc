#pragma once

#include "ast_node.h"
#include <string>
#include <vector>

namespace sql::ast {

class FunctionCallExpression : public Expression {
public:
  FunctionCallExpression(std::string name,
                         std::vector<ExprPtr> arguments)
      : name_(std::move(name)),
        arguments_(std::move(arguments)) {}

  const std::string& name() const { return name_; }
  const std::vector<ExprPtr>& arguments() const { return arguments_; }

  void accept(NodeVisitor& visitor) override {
    visitor.visit(*this);
  }

private:
  std::string name_;
  std::vector<ExprPtr> arguments_;
};

} // namespace sql::ast
