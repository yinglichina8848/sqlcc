#pragma once

#include "ast_node.h"
#include <memory>
#include <string>
#include <vector>

namespace sql_parser {

/**
 * 外键约束类
 */
class ForeignKeyConstraint {
public:
  ForeignKeyConstraint(const std::vector<std::string> &columns,
                       const std::string &referenced_table,
                       const std::string &referenced_column,
                       const std::string &name = "");

  const std::vector<std::string> &getColumns() const;
  const std::string &getReferencedTable() const;
  const std::string &getReferencedColumn() const;
  const std::string &getName() const;

private:
  std::vector<std::string> columns_;
  std::string referenced_table_;
  std::string referenced_column_;
  std::string name_;
};

/**
 * 检查约束类
 */
class CheckConstraint {
public:
  CheckConstraint(std::unique_ptr<Expression> condition,
                  const std::string &name = "");

  const Expression *getCondition() const;
  const std::string &getName() const;

private:
  std::unique_ptr<Expression> condition_;
  std::string name_;
};

/**
 * 主键约束类
 */
class PrimaryKeyConstraint {
public:
  PrimaryKeyConstraint(const std::vector<std::string> &columns,
                       const std::string &name = "");

  const std::vector<std::string> &getColumns() const;
  const std::string &getName() const;

private:
  std::vector<std::string> columns_;
  std::string name_;
};

/**
 * 唯一约束类
 */
class UniqueConstraint {
public:
  UniqueConstraint(const std::vector<std::string> &columns,
                   const std::string &name = "");

  const std::vector<std::string> &getColumns() const;
  const std::string &getName() const;

private:
  std::vector<std::string> columns_;
  std::string name_;
};

/**
 * 表约束类
 */
class TableConstraint {
public:
  enum Type { PRIMARY_KEY, UNIQUE, FOREIGN_KEY, CHECK };

  TableConstraint(Type type, const std::vector<std::string> &columns,
                  const std::string &name = "");
  TableConstraint(Type type,
                  std::unique_ptr<ForeignKeyConstraint> fk_constraint,
                  const std::string &name = "");
  TableConstraint(Type type, std::unique_ptr<CheckConstraint> check_constraint,
                  const std::string &name = "");

  Type getType() const;
  const std::vector<std::string> &getColumns() const;
  const ForeignKeyConstraint *getForeignKeyConstraint() const;
  const CheckConstraint *getCheckConstraint() const;
  const std::string &getName() const;

private:
  Type type_;
  std::vector<std::string> columns_;
  std::unique_ptr<ForeignKeyConstraint> fk_constraint_;
  std::unique_ptr<CheckConstraint> check_constraint_;
  std::string name_;
};

/**
 * 列定义类
 */
class ColumnDefinition {
public:
  ColumnDefinition(const std::string &name, const std::string &type,
                   bool is_nullable = true,
                   const std::string &default_value = "");

  const std::string &getName() const;
  const std::string &getType() const;
  bool isNullable() const;
  const std::string &getDefaultValue() const;

private:
  std::string name_;
  std::string type_;
  bool is_nullable_;
  std::string default_value_;
};

} // namespace sql_parser