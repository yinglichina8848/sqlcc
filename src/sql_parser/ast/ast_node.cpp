#include "../../../include/sql_parser/ast/core/ast_node.h"

namespace sqlcc {
namespace sql_parser {
namespace ast {

ASTNode::ASTNode(const SourceLocation& location)
    : location_(location) {}

const SourceLocation& ASTNode::getLocation() const {
    return location_;
}

void ASTNode::setLocation(const SourceLocation& location) {
    location_ = location;
}

bool ASTNode::isValid() const {
    return location_.isValid();
}

} // namespace ast
} // namespace sql_parser
} // namespace sqlcc
