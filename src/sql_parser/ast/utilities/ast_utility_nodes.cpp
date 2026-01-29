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

GrantStatement::GrantStatement(const std::vector<std::string> &privileges,
                               const std::string &objectType,
                               const std::string &objectName,
                               const std::string &userName)
    : Statement(GRANT), privileges_(privileges), objectType_(objectType),
      objectName_(objectName), userName_(userName) {}

GrantStatement::~GrantStatement() {}

void GrantStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

const std::vector<std::string> &GrantStatement::getPrivileges() const {
  return privileges_;
}

const std::string &GrantStatement::getObjectType() const {
  return objectType_;
}

const std::string &GrantStatement::getObjectName() const {
  return objectName_;
}

const std::string &GrantStatement::getUserName() const {
  return userName_;
}

// ==================== RevokeStatement ====================

RevokeStatement::RevokeStatement(const std::vector<std::string> &privileges,
                                 const std::string &objectType,
                                 const std::string &objectName,
                                 const std::string &userName)
    : Statement(REVOKE), privileges_(privileges), objectType_(objectType),
      objectName_(objectName), userName_(userName) {}

RevokeStatement::~RevokeStatement() {}

void RevokeStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

const std::vector<std::string> &RevokeStatement::getPrivileges() const {
  return privileges_;
}

const std::string &RevokeStatement::getObjectType() const {
  return objectType_;
}

const std::string &RevokeStatement::getObjectName() const {
  return objectName_;
}

const std::string &RevokeStatement::getUserName() const {
  return userName_;
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

ShowStatement::ShowStatement() : Statement(SHOW) {}

ShowStatement::~ShowStatement() {}

void ShowStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

// ==================== LoadDataStatement ====================

LoadDataStatement::LoadDataStatement() : Statement(LOAD_DATA) {}

LoadDataStatement::~LoadDataStatement() {}

void LoadDataStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

} // namespace sql_parser
} // namespace sqlcc