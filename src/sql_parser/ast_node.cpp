#include "../../include/sql_parser/ast_node.h"
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

std::string Statement::getTypeName() const {
    switch (type_) {
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
        case CREATE_USER: return "CREATE_USER";
        case DROP_USER: return "DROP_USER";
        case GRANT: return "GRANT";
        case REVOKE: return "REVOKE";
        case SHOW: return "SHOW";
        case COMMIT: return "COMMIT";
        case ROLLBACK: return "ROLLBACK";
        case CREATE_PROCEDURE: return "CREATE_PROCEDURE";
        case DROP_PROCEDURE: return "DROP_PROCEDURE";
        case CALL_PROCEDURE: return "CALL_PROCEDURE";
        case CREATE_TRIGGER: return "CREATE_TRIGGER";
        case DROP_TRIGGER: return "DROP_TRIGGER";
        case ALTER_TRIGGER: return "ALTER_TRIGGER";
        default: return "UNKNOWN";
    }
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