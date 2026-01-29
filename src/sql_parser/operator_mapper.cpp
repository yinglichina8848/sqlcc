#include "operator_mapper.h"
#include <stdexcept>

namespace sqlcc {
namespace sql_parser {

OperatorKind tokenToOperatorKind(Type type) {
  switch (type) {
    // Arithmetic operators
    case Type::OPERATOR_PLUS:
      return OperatorKind::Add;
    case Type::OPERATOR_MINUS:
      return OperatorKind::Subtract;
    case Type::OPERATOR_MULTIPLY:
      return OperatorKind::Multiply;
    case Type::OPERATOR_DIVIDE:
      return OperatorKind::Divide;

    // Comparison operators
    case Type::OPERATOR_EQUAL:
      return OperatorKind::Equal;
    case Type::OPERATOR_NOT_EQUAL:
      return OperatorKind::NotEqual;
    case Type::OPERATOR_LESS_THAN:
      return OperatorKind::Less;
    case Type::OPERATOR_LESS_EQUAL:
      return OperatorKind::LessEqual;
    case Type::OPERATOR_GREATER_THAN:
      return OperatorKind::Greater;
    case Type::OPERATOR_GREATER_EQUAL:
      return OperatorKind::GreaterEqual;

    // Logical operators
    case Type::KEYWORD_AND:
      return OperatorKind::And;
    case Type::KEYWORD_OR:
      return OperatorKind::Or;

    // Unary operators
    case Type::KEYWORD_NOT:
      return OperatorKind::Not;

    default:
      throw std::runtime_error("Unsupported token type for OperatorKind mapping");
  }
}

int getOperatorPrecedence(OperatorKind op) {
  switch (op) {
    // Highest precedence: unary operators
    case OperatorKind::Not:
    case OperatorKind::Negate:
      return 4;

    // Multiplicative operators
    case OperatorKind::Multiply:
    case OperatorKind::Divide:
      return 3;

    // Additive operators
    case OperatorKind::Add:
    case OperatorKind::Subtract:
      return 2;

    // Comparison operators
    case OperatorKind::Equal:
    case OperatorKind::NotEqual:
    case OperatorKind::Less:
    case OperatorKind::LessEqual:
    case OperatorKind::Greater:
    case OperatorKind::GreaterEqual:
      return 1;

    // Lowest precedence: logical operators
    case OperatorKind::And:
      return 0;
    case OperatorKind::Or:
      return -1;

    default:
      return -1;  // Unknown operator
  }
}

} // namespace sql_parser
} // namespace sqlcc
