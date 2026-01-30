#include "ast/ast_node.h"
#include "src/sql_parser/constraint.h"

namespace sqlcc {
namespace sql_parser {

// ForeignKeyConstraint implementation
ForeignKeyConstraint::ForeignKeyConstraint(const std::vector<std::string> &columns,
                                         const std::string &referenced_table,
                                         const std::vector<std::string> &referenced_columns,
                                         const std::string &name,
                                         CascadeAction on_delete,
                                         CascadeAction on_update,
                                         DeferrableMode deferrable)
    : columns_(columns),
      referenced_table_(referenced_table),
      referenced_columns_(referenced_columns),
      name_(name),
      on_delete_(on_delete),
      on_update_(on_update),
      deferrable_(deferrable) {
}

const std::vector<std::string> &ForeignKeyConstraint::getColumns() const {
    return columns_;
}

const std::string &ForeignKeyConstraint::getReferencedTable() const {
    return referenced_table_;
}

const std::vector<std::string> &ForeignKeyConstraint::getReferencedColumns() const {
    return referenced_columns_;
}

const std::string &ForeignKeyConstraint::getName() const {
    return name_;
}

ForeignKeyConstraint::CascadeAction ForeignKeyConstraint::getOnDeleteAction() const {
    return on_delete_;
}

ForeignKeyConstraint::CascadeAction ForeignKeyConstraint::getOnUpdateAction() const {
    return on_update_;
}

ForeignKeyConstraint::DeferrableMode ForeignKeyConstraint::getDeferrableMode() const {
    return deferrable_;
}

// CheckConstraint implementation
CheckConstraint::CheckConstraint(std::unique_ptr<Expression> condition,
                                const std::string &name)
    : condition_(std::move(condition)), name_(name) {
}

const Expression *CheckConstraint::getCondition() const {
    return condition_.get();
}

const std::string &CheckConstraint::getName() const {
    return name_;
}

// PrimaryKeyConstraint implementation
PrimaryKeyConstraint::PrimaryKeyConstraint(const std::vector<std::string> &columns,
                                           const std::string &name)
    : columns_(columns), name_(name) {
}

const std::vector<std::string> &PrimaryKeyConstraint::getColumns() const {
    return columns_;
}

const std::string &PrimaryKeyConstraint::getName() const {
    return name_;
}

// UniqueConstraint implementation
UniqueConstraint::UniqueConstraint(const std::vector<std::string> &columns,
                                  const std::string &name)
    : columns_(columns), name_(name) {
}

const std::vector<std::string> &UniqueConstraint::getColumns() const {
    return columns_;
}

const std::string &UniqueConstraint::getName() const {
    return name_;
}

// NotNullConstraint implementation
NotNullConstraint::NotNullConstraint(const std::string &column, const std::string &name)
    : column_(column), name_(name) {
}

const std::string &NotNullConstraint::getColumn() const {
    return column_;
}

const std::string &NotNullConstraint::getName() const {
    return name_;
}

// AssertionConstraint implementation
AssertionConstraint::AssertionConstraint(std::unique_ptr<Expression> condition,
                                       const std::string &name)
    : condition_(std::move(condition)), name_(name) {
}

const Expression *AssertionConstraint::getCondition() const {
    return condition_.get();
}

const std::string &AssertionConstraint::getName() const {
    return name_;
}

} // namespace sql_parser
} // namespace sqlcc
