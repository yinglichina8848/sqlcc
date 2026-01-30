#include "src/sql_parser/ast/ast_node.h"
#include "src/sql_parser/ast/ast_nodes.h"
#include "src/sql_parser/ast/node_visitor.h"
#include "src/sql_parser/ast/ast_fwd.h"
#include <memory>

namespace sqlcc {
namespace sql_parser {

// ==================== AlterViewStatement Implementation ====================

AlterViewStatement::AlterViewStatement(const std::string &viewName)
    : Statement(Statement::ALTER), viewName_(viewName) {}

AlterViewStatement::~AlterViewStatement() {}

const std::string &AlterViewStatement::getViewName() const { return viewName_; }

const std::vector<std::string> &AlterViewStatement::getColumnNames() const {
  return columnNames_;
}

const SelectStatement &AlterViewStatement::getSelectStatement() const {
  return *selectStatement_;
}

void AlterViewStatement::addColumnName(const std::string &columnName) {
  columnNames_.push_back(columnName);
}

void AlterViewStatement::setSelectStatement(std::unique_ptr<SelectStatement> selectStmt) {
  selectStatement_ = std::move(selectStmt);
}

bool AlterViewStatement::hasColumnNames() const {
  return !columnNames_.empty();
}

void AlterViewStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

// ==================== DropViewStatement Implementation ====================

DropViewStatement::DropViewStatement(const std::string &viewName)
    : Statement(Statement::DROP), viewName_(viewName), dropBehavior_(RESTRICT), ifExists_(false) {}

DropViewStatement::~DropViewStatement() {}

const std::string &DropViewStatement::getViewName() const { return viewName_; }

DropViewStatement::DropBehavior DropViewStatement::getDropBehavior() const {
  return dropBehavior_;
}

void DropViewStatement::setDropBehavior(DropBehavior behavior) {
  dropBehavior_ = behavior;
}

bool DropViewStatement::isIfExists() const { return ifExists_; }

void DropViewStatement::setIfExists(bool ifExists) { ifExists_ = ifExists; }

void DropViewStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

} // namespace sql_parser
} // namespace sqlcc
