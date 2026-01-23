#pragma once

#include "ast_nodes.h"
#include <iostream>

namespace sql::ast {

class DebugPrintVisitor : public NodeVisitor {
public:
  explicit DebugPrintVisitor(std::ostream& os = std::cout)
      : os_(os) {}

  void visit(NumericLiteralExpression& expr) override {
    indent();
    os_ << "NumericLiteral(" << expr.value() << ")\n";
  }

  void visit(StringLiteralExpression& expr) override {
    indent();
    os_ << "StringLiteral(\"" << expr.value() << "\")\n";
  }

  void visit(BooleanLiteralExpression& expr) override {
    indent();
    os_ << "BooleanLiteral(" << (expr.value() ? "true" : "false") << ")\n";
  }

  void visit(NullLiteralExpression&) override {
    indent();
    os_ << "NullLiteral\n";
  }

  void visit(IdentifierExpression& expr) override {
    indent();
    os_ << "Identifier(" << expr.name() << ")\n";
  }

  void visit(FunctionCallExpression& expr) override {
    indent();
    os_ << "FunctionCall(" << expr.name() << ")\n";
    ++indent_;
    for (auto& arg : expr.arguments()) {
      arg->accept(*this);
    }
    --indent_;
  }

private:
  void indent() {
    for (int i = 0; i < indent_; ++i) {
      os_ << "  ";
    }
  }

  int indent_ = 0;
  std::ostream& os_;
};

} // namespace sql::ast
