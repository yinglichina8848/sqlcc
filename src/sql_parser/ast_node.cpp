#include "sql_parser/ast_node.h"

namespace sqlcc {
namespace sql_parser {

Node::~Node() = default;

Expression::~Expression() = default;

Statement::~Statement() = default;

void Expression::accept(NodeVisitor &visitor) {
    // Expression是抽象基类，不应该被直接访问
    // 具体的表达式类型会重写这个方法
}

Expression::Type Expression::getType() const {
    return IDENTIFIER; // 默认类型
}

void Statement::accept(NodeVisitor &visitor) {
    // Statement是抽象基类，不应该被直接访问
    // 具体的语句类型会重写这个方法
}

} // namespace sql_parser
} // namespace sqlcc