#include "../include/sql_parser/ast_dcl_statements.h"
#include "node_visitor.h"

namespace sqlcc::sql_parser {

// CreateRoleStatement 实现
CreateRoleStatement::CreateRoleStatement(const std::string& role_name)
    : Statement(Statement::Type::CREATE_ROLE), role_name_(role_name) {}

CreateRoleStatement::~CreateRoleStatement() = default;

std::string CreateRoleStatement::getTypeName() const {
    return "CreateRoleStatement";
}

void CreateRoleStatement::accept(NodeVisitor& visitor) {
    visitor.visit(*this);
}

// DropRoleStatement 实现
DropRoleStatement::DropRoleStatement(const std::string& role_name)
    : Statement(Statement::Type::DROP_ROLE), role_name_(role_name) {}

DropRoleStatement::~DropRoleStatement() = default;

std::string DropRoleStatement::getTypeName() const {
    return "DropRoleStatement";
}

void DropRoleStatement::accept(NodeVisitor& visitor) {
    visitor.visit(*this);
}

// GrantRoleStatement 实现
GrantRoleStatement::GrantRoleStatement(const std::string& role_name, const std::string& grantee)
    : Statement(Statement::Type::GRANT_ROLE), role_name_(role_name), grantee_(grantee) {}

GrantRoleStatement::~GrantRoleStatement() = default;

std::string GrantRoleStatement::getTypeName() const {
    return "GrantRoleStatement";
}

void GrantRoleStatement::accept(NodeVisitor& visitor) {
    visitor.visit(*this);
}

// RevokeRoleStatement 实现
RevokeRoleStatement::RevokeRoleStatement(const std::string& role_name, const std::string& grantee)
    : Statement(Statement::Type::REVOKE_ROLE), role_name_(role_name), grantee_(grantee) {}

RevokeRoleStatement::~RevokeRoleStatement() = default;

std::string RevokeRoleStatement::getTypeName() const {
    return "RevokeRoleStatement";
}

void RevokeRoleStatement::accept(NodeVisitor& visitor) {
    visitor.visit(*this);
}

} // namespace sqlcc::sql_parser
