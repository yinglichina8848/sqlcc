#pragma once

#include "ast_nodes.h"
#include <iostream>

namespace sqlcc {
namespace sql_parser {

class DebugPrintVisitor : public NodeVisitor {
public:
  explicit DebugPrintVisitor(std::ostream& os = std::cout)
      : os_(os) {}

  void visit(const NumericLiteralExpression& expr) override {
    indent();
    os_ << "NumericLiteral(" << expr.getValue() << ")\n";
  }

  void visit(const StringLiteralExpression& expr) override {
    indent();
    os_ << "StringLiteral(\"" << expr.getValue() << "\")\n";
  }

  void visit(const BooleanLiteralExpression& expr) override {
    indent();
    os_ << "BooleanLiteral(" << (expr.getValue() ? "true" : "false") << ")\n";
  }

  void visit(const NullLiteralExpression&) override {
    indent();
    os_ << "NullLiteral\n";
  }

  void visit(const IdentifierExpression& expr) override {
    indent();
    os_ << "Identifier(" << expr.getName() << ")\n";
  }

  void visit(const FunctionCallExpression& expr) override {
    indent();
    os_ << "FunctionCall(" << expr.getName() << ")\n";
    ++indent_;
    for (const auto& arg : expr.getArguments()) {
      arg->accept(*this);
    }
    --indent_;
  }

  void visit(const BinaryExpression& expr) override {
    indent();
    os_ << "BinaryExpression(";
    switch (expr.getOperator()) {
      case OperatorKind::Add: os_ << "Add"; break;
      case OperatorKind::Subtract: os_ << "Subtract"; break;
      case OperatorKind::Multiply: os_ << "Multiply"; break;
      case OperatorKind::Divide: os_ << "Divide"; break;
      case OperatorKind::Equal: os_ << "Equal"; break;
      case OperatorKind::NotEqual: os_ << "NotEqual"; break;
      case OperatorKind::Less: os_ << "Less"; break;
      case OperatorKind::LessEqual: os_ << "LessEqual"; break;
      case OperatorKind::Greater: os_ << "Greater"; break;
      case OperatorKind::GreaterEqual: os_ << "GreaterEqual"; break;
      case OperatorKind::And: os_ << "And"; break;
      case OperatorKind::Or: os_ << "Or"; break;
      case OperatorKind::Not: os_ << "Not"; break;
      case OperatorKind::Negate: os_ << "Negate"; break;
    }
    os_ << ")\n";
    ++indent_;
    expr.getLeft().accept(*this);
    expr.getRight().accept(*this);
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

} // namespace sql_parser
} // namespace sqlcc
