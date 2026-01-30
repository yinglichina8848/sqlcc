#ifndef SQLCC_SQL_PARSER_WINDOW_FUNCTION_NODE_H
#define SQLCC_SQL_PARSER_WINDOW_FUNCTION_NODE_H

#include "window_function.h"

namespace sqlcc {
namespace sql_parser {

// Backwards-compatibility aliases: some parts of the codebase use
// WindowFunctionNode / WindowSpecificationNode while others use
// WindowFunction / WindowSpecification. Make them the same type so
// both naming styles compile without further changes.
using WindowFunctionNode = WindowFunction;
using WindowSpecificationNode = WindowSpecification;

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_WINDOW_FUNCTION_NODE_H
