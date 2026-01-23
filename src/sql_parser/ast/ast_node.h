#ifndef SQLCC_SQL_PARSER_AST_AST_NODE_H
#define SQLCC_SQL_PARSER_AST_AST_NODE_H

#include <memory>
#include <string>

namespace sqlcc {
namespace sql_parser {

class ASTNode {
public:
  virtual ~ASTNode() = default;
  virtual std::string getTypeName() const { return "ASTNode"; }
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_AST_AST_NODE_H
