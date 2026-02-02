/**
 * @file ast_utility_nodes.cpp
 * @brief 工具类AST节点实现
 * 
 * 包含各种辅助性的AST节点实现，包括：
 * - 权限管理相关节点（GrantStatement, RevokeStatement）
 * - 事务控制相关节点（CommitStatement, RollbackStatement, BeginStatement）
 * - 其他辅助节点（UseStatement, ShowStatement, LoadDataStatement）
 */

#include "../ast_node.h"
#include "../ast_nodes.h"
#include <iostream>

namespace sqlcc {
namespace sql_parser {

// ==================== GrantStatement ====================

GrantStatement::GrantStatement() : Statement(GRANT) {}

GrantStatement::~GrantStatement() {}

void GrantStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

void GrantStatement::addPrivilege(const std::string &privilege) {
  privileges_.push_back(privilege);
}

const std::vector<std::string> &GrantStatement::getPrivileges() const {
  return privileges_;
}

void GrantStatement::setObjectType(const std::string &objectType) {
  objectType_ = objectType;
}

const std::string &GrantStatement::getObjectType() const {
  return objectType_;
}

void GrantStatement::setObjectName(const std::string &objectName) {
  objectName_ = objectName;
}

const std::string &GrantStatement::getObjectName() const {
  return objectName_;
}

void GrantStatement::setGrantee(const std::string &grantee) {
  grantee_ = grantee;
}

const std::string &GrantStatement::getGrantee() const {
  return grantee_;
}

std::string GrantStatement::getGrantee() {
  return grantee_;
}

// ==================== RevokeStatement ====================

RevokeStatement::RevokeStatement() : Statement(REVOKE) {}

RevokeStatement::~RevokeStatement() {}

void RevokeStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

void RevokeStatement::addPrivilege(const std::string &privilege) {
  privileges_.push_back(privilege);
}

const std::vector<std::string> &RevokeStatement::getPrivileges() const {
  return privileges_;
}

void RevokeStatement::setObjectType(const std::string &objectType) {
  objectType_ = objectType;
}

const std::string &RevokeStatement::getObjectType() const {
  return objectType_;
}

void RevokeStatement::setObjectName(const std::string &objectName) {
  objectName_ = objectName;
}

const std::string &RevokeStatement::getObjectName() const {
  return objectName_;
}

void RevokeStatement::setGrantee(const std::string &grantee) {
  grantee_ = grantee;
}

const std::string &RevokeStatement::getGrantee() const {
  return grantee_;
}

std::string RevokeStatement::getGrantee() {
  return grantee_;
}

// ==================== CommitStatement ====================

CommitStatement::CommitStatement() : Statement(COMMIT) {}

CommitStatement::~CommitStatement() {}

void CommitStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

// ==================== RollbackStatement ====================

RollbackStatement::RollbackStatement() : Statement(ROLLBACK) {}

RollbackStatement::~RollbackStatement() {}

void RollbackStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

// ==================== BeginStatement ====================

BeginStatement::BeginStatement() : Statement(BEGIN) {}

BeginStatement::~BeginStatement() {}

void BeginStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

// ==================== UseStatement ====================

UseStatement::UseStatement(const std::string &databaseName)
    : Statement(USE), databaseName_(databaseName) {}

UseStatement::~UseStatement() {}

void UseStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

const std::string &UseStatement::getDatabaseName() const {
  return databaseName_;
}

// ==================== ShowStatement ====================

ShowStatement::ShowStatement(ShowType type) : Statement(SHOW), type_(type) {}

ShowStatement::~ShowStatement() {}

void ShowStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

ShowStatement::ShowType ShowStatement::getShowType() const {
  return type_;
}

void ShowStatement::setTargetObject(const std::string &target) {
  targetObject_ = target;
}

const std::string &ShowStatement::getTargetObject() const {
  return targetObject_;
}

void ShowStatement::setFromDatabase(const std::string &dbName) {
  fromDatabase_ = dbName;
  hasFromDb_ = true;
}

const std::string &ShowStatement::getFromDatabase() const {
  return fromDatabase_;
}

bool ShowStatement::hasFromDatabase() const {
  return hasFromDb_;
}


} // namespace sql_parser
} // namespace sqlcc