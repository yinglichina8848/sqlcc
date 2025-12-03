#include "sql_parser/ast_node.h"
#include <string>

namespace sqlcc {
namespace sql_parser {

// ==================== Statement ====================

Statement::Statement(Type type) : type_(type) {
}

Statement::~Statement() {
}

Statement::Type Statement::getType() const {
    return type_;
}

// ==================== Expression ====================

Expression::Expression() {
}

Expression::~Expression() {
}

std::string Expression::getTypeName() const {
    return "Expression";
}

} // namespace sql_parser
} // namespace sqlcc