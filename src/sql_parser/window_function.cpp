#include "sql_parser/window_function.h"
#include "sql_parser/ast_node.h"
#include "sql_parser/node_visitor.h"
#include "sql_parser/token.h"

namespace sqlcc {
namespace sql_parser {

WindowFunction::WindowFunction(FunctionType type)
    : functionType_(type) {
}

WindowFunction::~WindowFunction() = default;

WindowSpecification::WindowSpecification() = default;

WindowSpecification::~WindowSpecification() = default;

} // namespace sql_parser
} // namespace sqlcc