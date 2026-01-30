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

#include "src/sql_parser/ast/ast_node.h"
#include "src/sql_parser/ast/ast_nodes.h"
#include <iostream>

namespace sqlcc {
namespace sql_parser {

// ==================== WhereClause ====================

WhereClause::WhereClause(std::unique_ptr<Expression> condition)
    : condition_(std::move(condition)) {}

WhereClause::~WhereClause() {}

void WhereClause::setCondition(std::unique_ptr<Expression> condition) {
    condition_ = std::move(condition);
}

// ==================== SelectStatement ====================

SelectStatement::SelectStatement()
    : Statement(SELECT), distinct_(false), selectAll_(false) {}

SelectStatement::~SelectStatement() {}

void SelectStatement::setSelectList(std::vector<std::unique_ptr<Expression>> selectList) {
    selectList_ = std::move(selectList);
}

void SelectStatement::setWhereClause(std::unique_ptr<WhereClause> where) {
    whereClause_ = std::move(where);
}

void SelectStatement::setJoinClauses(std::vector<std::unique_ptr<JoinClause>> joins) {
    joinClauses_ = std::move(joins);
}

void SelectStatement::setGroupBy(std::vector<std::unique_ptr<Expression>> groupBy) {
    groupBy_ = std::move(groupBy);
}

void SelectStatement::setHaving(std::unique_ptr<Expression> having) {
    having_ = std::move(having);
}

void SelectStatement::setOrderBy(std::vector<std::unique_ptr<Expression>> orderBy) {
    orderBy_ = std::move(orderBy);
}

void SelectStatement::addJoinClause(std::unique_ptr<JoinClause> join) {
  joinClauses_.push_back(std::move(join));
}

void SelectStatement::accept(ast::NodeVisitor &visitor) {
  visitor.visit(*this);
}

void SelectStatement::addGroupByColumn(const std::string &column) {
  groupBy_.push_back(std::make_unique<IdentifierExpression>(column));
}

void SelectStatement::setOrderByColumn(const std::string &column) {
  orderBy_.clear();
  orderBy_.push_back(std::make_unique<IdentifierExpression>(column));
}

void SelectStatement::setOrderDirection(const std::string &direction) {
  orderDirection_ = direction;
}

// ==================== InsertStatement ====================

InsertStatement::InsertStatement(const std::string &tableName)
    : Statement(INSERT), tableName_(tableName) {}

InsertStatement::~InsertStatement() {}

void InsertStatement::accept(ast::NodeVisitor &visitor) {
  visitor.visit(*this);
}

void InsertStatement::addValue(const std::string &value) {
  // 确保至少有一行
  if (values_.empty()) {
    values_.emplace_back();
  }
  // 添加值到当前行
  values_.back().push_back(std::make_unique<StringLiteralExpression>(value));
}

void InsertStatement::finishRow() {
  // 准备新的一行
  values_.emplace_back();
}

void InsertStatement::setValues(std::vector<std::vector<std::unique_ptr<Expression>>> values) {
  values_ = std::move(values);
}

void InsertStatement::setSelectStatement(std::unique_ptr<SelectStatement> select) {
    selectStatement_ = std::move(select);
}

void InsertStatement::addColumn(const std::string &column) {
    columnNames_.push_back(column);
}

// ==================== UpdateStatement ====================

UpdateStatement::UpdateStatement(const std::string &tableName)
    : Statement(UPDATE), tableName_(tableName) {}

UpdateStatement::~UpdateStatement() {}

void UpdateStatement::accept(ast::NodeVisitor &visitor) {
  visitor.visit(*this);
}

void UpdateStatement::setAssignments(std::vector<std::pair<std::string, std::unique_ptr<Expression>>> assignments) {
    assignments_ = std::move(assignments);
}

void UpdateStatement::setWhereClause(std::unique_ptr<WhereClause> where) {
    whereClause_ = std::move(where);
}

// ==================== DeleteStatement ====================

DeleteStatement::DeleteStatement(const std::vector<std::string> &tableNames)
    : Statement(DELETE), tableNames_(tableNames) {}

DeleteStatement::~DeleteStatement() {}

void DeleteStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

void DeleteStatement::setWhereClause(std::unique_ptr<WhereClause> whereClause) {
  whereClause_ = std::move(whereClause);
}

// ==================== JoinClause ====================

JoinClause::JoinClause(JoinType type, const std::string &tableName,
                       std::unique_ptr<Expression> condition)
    : tableName_(tableName), type_(type), condition_(std::move(condition)) {}

JoinClause::~JoinClause() {}

void JoinClause::setCondition(std::unique_ptr<Expression> condition) {
    condition_ = std::move(condition);
}

} // namespace sql_parser
} // namespace sqlcc