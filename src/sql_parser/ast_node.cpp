#include "sql_parser/ast_node.h"

namespace sqlcc {
namespace sql_parser {

// 虚基类的析构函数已在头文件中默认定义

std::string Expression::getTypeName() const {
    Type type = getType();
    switch (type) {
        case IDENTIFIER: return "IDENTIFIER";
        case STRING_LITERAL: return "STRING_LITERAL";
        case NUMERIC_LITERAL: return "NUMERIC_LITERAL";
        case BINARY: return "BINARY";
        case UNARY: return "UNARY";
        case FUNCTION: return "FUNCTION";
        case EXISTS: return "EXISTS";
        case IN: return "IN";
        default: return "UNKNOWN_EXPRESSION_TYPE";
    }
}

std::string Statement::getTypeName() const {
    Type type = getType();
    switch (type) {
        case CREATE: return "CREATE";
        case SELECT: return "SELECT";
        case INSERT: return "INSERT";
        case UPDATE: return "UPDATE";
        case DELETE: return "DELETE";
        case DROP: return "DROP";
        case ALTER: return "ALTER";
        case USE: return "USE";
        case CREATE_INDEX: return "CREATE_INDEX";
        case DROP_INDEX: return "DROP_INDEX";
        default: return "UNKNOWN_STATEMENT_TYPE";
    }
}

} // namespace sql_parser
} // namespace sqlcc
