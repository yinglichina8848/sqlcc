#ifndef SQLCC_SQL_PARSER_AST_NODE_H
#define SQLCC_SQL_PARSER_AST_NODE_H

#include <memory>
#include <string>
#include "token_new.h"
#include "node_visitor.h"

namespace sqlcc {
namespace sql_parser {

/**
 * AST节点基类
 */
class Node {
public:
    virtual ~Node() = default;
    virtual void accept(NodeVisitor &visitor) = 0;
};

/**
 * 表达式节点基类
 */
class Expression : public Node {
public:
    Expression();
    virtual ~Expression();
    
    virtual std::string getTypeName() const;
    virtual void accept(NodeVisitor &visitor) = 0;
};

/**
 * 语句节点基类
 */
class Statement : public Node {
public:
    enum Type {
        CREATE,
        SELECT,
        INSERT,
        UPDATE,
        DELETE,
        DROP,
        ALTER,
        USE,
        CREATE_INDEX,
        DROP_INDEX,
        CREATE_USER,
        DROP_USER,
        GRANT,
        REVOKE,
        SHOW,
        COMMIT,
        ROLLBACK,
        CREATE_PROCEDURE,
        DROP_PROCEDURE,
        CALL_PROCEDURE,
        CREATE_TRIGGER,
        DROP_TRIGGER,
        ALTER_TRIGGER
    };

    Statement(Type type);
    virtual ~Statement();

    Type getType() const;
    std::string getTypeName() const;
    virtual void accept(NodeVisitor &visitor) = 0;

private:
    Type type_;
};


} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_AST_NODE_H
