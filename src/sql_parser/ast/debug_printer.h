#pragma once

#include "src/sql_parser/ast/ast_nodes.h"
#include <iostream>

namespace sqlcc {
namespace sql_parser {

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

  void visit(BinaryExpression& expr) override {
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
