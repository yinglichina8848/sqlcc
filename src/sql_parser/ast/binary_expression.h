#pragma once

#include "ast_node.h"
#include "../operator_kind.h"
#include <memory>
#include <string>

namespace sqlcc {
namespace sql_parser {
namespace ast {

class BinaryExpression : public Expression {
public:
  BinaryExpression(std::unique_ptr<Expression> left,
                   std::unique_ptr<Expression> right,
                   OperatorKind op);

  ~BinaryExpression() override;

  Expression& getLeft() const;

  Expression& getRight() const;

  OperatorKind getOperator() const;

  std::string getTypeName() const;

  void accept(NodeVisitor& visitor) override;

private:
  std::unique_ptr<Expression> left_;
  std::unique_ptr<Expression> right_;
  OperatorKind op_;
};

} // namespace ast
} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_AST_BINARY_EXPRESSION_H
