#include "ast_constraint.h"

namespace sql_parser {

// PrimaryKeyConstraint 实现
PrimaryKeyConstraint::PrimaryKeyConstraint(
    const std::vector<std::string> &columns, const std::string &name)
    : columns_(columns), name_(name) {}

const std::vector<std::string> &PrimaryKeyConstraint::getColumns() const {
  return columns_;
}

const std::string &PrimaryKeyConstraint::getName() const { return name_; }

// UniqueConstraint 实现
UniqueConstraint::UniqueConstraint(const std::vector<std::string> &columns,
                                   const std::string &name)
    : columns_(columns), name_(name) {}

const std::vector<std::string> &UniqueConstraint::getColumns() const {
  return columns_;
}

const std::string &UniqueConstraint::getName() const { return name_; }

// ForeignKeyConstraint 实现
ForeignKeyConstraint::ForeignKeyConstraint(
    const std::vector<std::string> &columns,
    const std::string &referenced_table, const std::string &referenced_column,
    const std::string &name)
    : columns_(columns), referenced_table_(referenced_table),
      referenced_column_(referenced_column), name_(name) {}

const std::vector<std::string> &ForeignKeyConstraint::getColumns() const {
  return columns_;
}

const std::string &ForeignKeyConstraint::getReferencedTable() const {
  return referenced_table_;
}

const std::string &ForeignKeyConstraint::getReferencedColumn() const {
  return referenced_column_;
}

const std::string &ForeignKeyConstraint::getName() const { return name_; }

// CheckConstraint 实现
CheckConstraint::CheckConstraint(std::unique_ptr<Expression> condition,
                                 const std::string &name)
    : condition_(std::move(condition)), name_(name) {}

const Expression *CheckConstraint::getCondition() const {
  return condition_.get();
}

const std::string &CheckConstraint::getName() const { return name_; }

// TableConstraint 实现
TableConstraint::TableConstraint(Type type,
                                 const std::vector<std::string> &columns,
                                 const std::string &name)
    : type_(type), columns_(columns), name_(name) {}

TableConstraint::TableConstraint(
    Type type, std::unique_ptr<ForeignKeyConstraint> fk_constraint,
    const std::string &name)
    : type_(type), fk_constraint_(std::move(fk_constraint)), name_(name) {}

TableConstraint::TableConstraint(
    Type type, std::unique_ptr<CheckConstraint> check_constraint,
    const std::string &name)
    : type_(type), check_constraint_(std::move(check_constraint)), name_(name) {
}

TableConstraint::Type TableConstraint::getType() const { return type_; }

const std::vector<std::string> &TableConstraint::getColumns() const {
  return columns_;
}

const ForeignKeyConstraint *TableConstraint::getForeignKeyConstraint() const {
  return fk_constraint_.get();
}

const CheckConstraint *TableConstraint::getCheckConstraint() const {
  return check_constraint_.get();
}

const std::string &TableConstraint::getName() const { return name_; }

// ColumnDefinition 实现
ColumnDefinition::ColumnDefinition(const std::string &name,
                                   const std::string &type, bool is_nullable,
                                   const std::string &default_value)
    : name_(name), type_(type), is_nullable_(is_nullable),
      default_value_(default_value) {}

const std::string &ColumnDefinition::getName() const { return name_; }

const std::string &ColumnDefinition::getType() const { return type_; }

bool ColumnDefinition::isNullable() const { return is_nullable_; }

const std::string &ColumnDefinition::getDefaultValue() const {
  return default_value_;
}

} // namespace sql_parser