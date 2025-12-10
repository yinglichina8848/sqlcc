#include "../../include/sql_parser/window_function_node.h"

namespace sqlcc {
namespace sql_parser {

WindowFunctionNode::WindowFunctionNode(FunctionType type)
    : functionType_(type) {}

WindowFunctionNode::~WindowFunctionNode() {}

WindowSpecificationNode::WindowSpecificationNode() {}

WindowSpecificationNode::~WindowSpecificationNode() {}

} // namespace sql_parser
} // namespace sqlcc