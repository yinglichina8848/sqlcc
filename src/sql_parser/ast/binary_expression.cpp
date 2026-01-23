#include "binary_expression.h"
#include "node_visitor.h"

#include <string>

namespace sqlcc {
namespace sql_parser {
namespace ast {

BinaryExpression::BinaryExpression(std::unique_ptr<Expression> left,
                                   std::unique_ptr<Expression> right,
                                   OperatorKind op)
    : left_(std::move(left)), right_(std::move(right)), op_(op) {
}

BinaryExpression::~BinaryExpression() = default;

Expression& BinaryExpression::getLeft() const {
  return *left_;
}

Expression& BinaryExpression::getRight() const {
  return *right_;
}

OperatorKind BinaryExpression::getOperator() const {
  return op_;
}

std::string BinaryExpression::getTypeName() const {
  return "BinaryExpression";
}

void BinaryExpression::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

} // namespace ast
} // namespace sql_parser
} // namespace sqlcc
