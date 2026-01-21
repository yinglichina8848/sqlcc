#include "ast_node.h"
#include "source_location.h"
#include <memory>
#include <string>

namespace sqlcc::sql_parser::ast {

Node::Node(const SourceLocation& location)
    : location_(location) {}

const SourceLocation& Node::getLocation() const {
    return location_;
}

void Node::setLocation(const SourceLocation& location) {
    location_ = location;
}

bool Node::isValid() const {
    return location_.isValid();
}

} // namespace sqlcc::sql_parser::ast
