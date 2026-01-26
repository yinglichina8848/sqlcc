/**
 * @file ast_dml_nodes.cpp
 * @brief DML相关AST节点实现
 * 
 * 包含数据操作语言（DML）相关的AST节点实现，包括：
 * - SelectStatement及其相关类
 * - InsertStatement
 * - UpdateStatement
 * - DeleteStatement
 */

#include "ast_node.h"
#include "ast_nodes.h"
#include <iostream>

namespace sqlcc {
namespace sql_parser {

// ==================== WhereClause ====================

WhereClause::WhereClause(const std::string &columnName, const std::string &op,
                         const std::string &value)
    : columnName_(columnName), op_(op), value_(value) {}

WhereClause::~WhereClause() {}

const std::string &WhereClause::getColumnName() const { return columnName_; }

const std::string &WhereClause::getOp() const { return op_; }

const std::string &WhereClause::getValue() const { return value_; }

// ==================== SelectStatement ====================

SelectStatement::SelectStatement()
    : Statement(SELECT), joinCondition_(""), limit_(-1), offset_(0),
      selectAll_(false), distinct_(false), hasLimit_(false), hasOffset_(false) {}

SelectStatement::~SelectStatement() {}

void SelectStatement::addJoinClause(std::unique_ptr<JoinClause> join) {
  joinClauses_.push_back(std::move(join));
}

void SelectStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

// ==================== InsertStatement ====================

InsertStatement::InsertStatement(const std::string &tableName)
    : Statement(INSERT), tableName_(tableName) {}

InsertStatement::~InsertStatement() {}

void InsertStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

void InsertStatement::setColumnNames(const std::vector<std::string> &columnNames) {
  columnNames_ = columnNames;
}

void InsertStatement::setValues(const std::vector<std::string> &values) {
  values_ = values;
}

const std::string &InsertStatement::getTableName() const {
  return tableName_;
}

const std::vector<std::string> &InsertStatement::getColumnNames() const {
  return columnNames_;
}

const std::vector<std::string> &InsertStatement::getValues() const {
  return values_;
}

// ==================== UpdateStatement ====================

UpdateStatement::UpdateStatement(const std::string &tableName)
    : Statement(UPDATE), tableName_(tableName) {}

UpdateStatement::~UpdateStatement() {}

void UpdateStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

// ==================== DeleteStatement ====================

DeleteStatement::DeleteStatement(const std::string &tableName)
    : Statement(DELETE), tableName_(tableName) {}

DeleteStatement::~DeleteStatement() {}

void DeleteStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

void DeleteStatement::setWhereClause(std::unique_ptr<WhereClause> whereClause) {
  whereClause_ = std::move(whereClause);
}

const std::string &DeleteStatement::getTableName() const {
  return tableName_;
}

const WhereClause *DeleteStatement::getWhereClause() const {
  return whereClause_.get();
}

// ==================== JoinClause ====================

JoinClause::JoinClause(JoinType type, const std::string &tableName,
                       std::unique_ptr<Expression> condition)
    : type_(type), tableName_(tableName), condition_(std::move(condition)) {}

JoinClause::~JoinClause() {}

JoinClause::JoinType JoinClause::getType() const { return type_; }

const std::string &JoinClause::getTableName() const { return tableName_; }

const Expression *JoinClause::getCondition() const { return condition_.get(); }

// ==================== SetOperation ====================

SetOperation::SetOperation(SetOperationType type,
                           std::unique_ptr<SelectStatement> left,
                           std::unique_ptr<SelectStatement> right)
    : Statement(SET_OPERATION), type_(type), left_(std::move(left)),
      right_(std::move(right)) {}

SetOperation::~SetOperation() {}

void SetOperation::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

SetOperationType SetOperation::getType() const { return type_; }

const SelectStatement *SetOperation::getLeft() const { return left_.get(); }

const SelectStatement *SetOperation::getRight() const { return right_.get(); }

} // namespace sql_parser
} // namespace sqlcc