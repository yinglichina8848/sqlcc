#pragma once

#include "ast_node.h"

namespace sqlcc {
namespace sql_parser {

namespace ast { class NodeVisitor; }
using NodeVisitor = ast::NodeVisitor;

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
        CREATE_PROCEDURE,
        DROP_PROCEDURE,
        CREATE_TRIGGER,
        DROP_TRIGGER,
        CREATE,
        DROP,
        WITH_RECURSIVE,
        COMMIT,  // 事务提交语句
        ROLLBACK,  // 事务回滚语句
        BEGIN,  // 事务开始语句
        USE,  // 使用数据库语句
        CREATE_USER,  // 创建用户语句
        DROP_USER,  // 删除用户语句
        GRANT,  // 授权语句
        REVOKE,  // 撤销授权语句
        SHOW,  // 显示语句
        CREATE_VIEW,  // 创建视图语句
        CREATE_ROLE,  // 创建角色语句
        DROP_ROLE,  // 删除角色语句
        GRANT_ROLE,  // 授予角色语句
        REVOKE_ROLE,  // 撤销角色语句
        LOAD_DATA,  // LOAD DATA 语句
    };

    explicit Statement(Type type) : type_(type) {}
    virtual ~Statement() = default;

    virtual void accept(ast::NodeVisitor& visitor) = 0;

    Type getType() const { return type_; }

private:
    Type type_;
};

} // namespace sql_parser
} // namespace sqlcc
