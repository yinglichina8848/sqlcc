#ifndef SQLCC_SQL_PARSER_AST_DCL_STATEMENTS_H
#define SQLCC_SQL_PARSER_AST_DCL_STATEMENTS_H

#include "src/sql_parser/ast/ast_node.h"
#include <string>
#include <vector>

namespace sqlcc::sql_parser {

/**
 * CREATE ROLE 语句节点
 */
class CreateRoleStatement : public Statement {
public:
    CreateRoleStatement(const std::string& role_name);
    virtual ~CreateRoleStatement();

    std::string getTypeName() const;
    virtual void accept(NodeVisitor& visitor) override;

    const std::string& getRoleName() const { return role_name_; }

private:
    std::string role_name_;
};

/**
 * DROP ROLE 语句节点
 */
class DropRoleStatement : public Statement {
public:
    DropRoleStatement(const std::string& role_name);
    virtual ~DropRoleStatement();

    std::string getTypeName() const;
    virtual void accept(NodeVisitor& visitor) override;

    const std::string& getRoleName() const { return role_name_; }

private:
    std::string role_name_;
};

/**
 * GRANT ROLE 语句节点
 */
class GrantRoleStatement : public Statement {
public:
    GrantRoleStatement(const std::string& role_name, const std::string& grantee);
    virtual ~GrantRoleStatement();

    std::string getTypeName() const;
    virtual void accept(NodeVisitor& visitor) override;

    const std::string& getRoleName() const { return role_name_; }
    const std::string& getGrantee() const { return grantee_; }

private:
    std::string role_name_;
    std::string grantee_;
};

/**
 * REVOKE ROLE 语句节点
 */
class RevokeRoleStatement : public Statement {
public:
    RevokeRoleStatement(const std::string& role_name, const std::string& grantee);
    virtual ~RevokeRoleStatement();

    std::string getTypeName() const;
    virtual void accept(NodeVisitor& visitor) override;

    const std::string& getRoleName() const { return role_name_; }
    const std::string& getGrantee() const { return grantee_; }

private:
    std::string role_name_;
    std::string grantee_;
};

} // namespace sqlcc::sql_parser

#endif // SQLCC_SQL_PARSER_AST_DCL_STATEMENTS_H
