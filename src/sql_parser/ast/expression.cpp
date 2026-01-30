#include "expression.h"
#include "node_visitor.h"

namespace sqlcc {
namespace sql_parser {
namespace ast {

void NumericLiteralExpression::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

void StringLiteralExpression::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

void BooleanLiteralExpression::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

void NullLiteralExpression::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

void IdentifierExpression::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

void FunctionCallExpression::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

void BinaryExpression::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

} // namespace ast
} // namespace sql_parser
} // namespace sqlcc
