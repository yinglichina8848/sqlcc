#pragma once

#include "ast_node.h"

namespace sqlcc {
namespace sql_parser {

class NodeVisitor;

class Statement : public ASTNode {
public:
    enum Type {
        SELECT,
        INSERT,
        UPDATE,
        DELETE,
        CREATE_TABLE,
        DROP_TABLE,
        ALTER_TABLE,
        CREATE_INDEX,
        DROP_INDEX,
        COMPOSITE_SELECT,  // Added for CompositeSelectStatement
        SAVEPOINT,
        RELEASE_SAVEPOINT,
        SET_TRANSACTION,
        ALTER,
        CREATE_DOMAIN,
        ALTER_DOMAIN,
        DROP_DOMAIN,
        ALTER_TABLE_ENHANCED,
        CALL_PROCEDURE,
        CREATE,
        DROP,
        WITH_RECURSIVE,
    };

    explicit Statement(Type type) : type_(type) {}
    virtual ~Statement() = default;

    Type getType() const { return type_; }

private:
    Type type_;
};

} // namespace sql_parser
} // namespace sqlcc
