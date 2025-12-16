#ifndef SQLCC_SQL_PARSER_CONSTRAINT_H
#define SQLCC_SQL_PARSER_CONSTRAINT_H

#include "ast_node.h"
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {
namespace sql_parser {

/**
 * 外键约束类
 */
class ForeignKeyConstraint {
public:
  // 级联操作类型
  enum CascadeAction {
    RESTRICT,
    CASCADE,
    SET_NULL,
    SET_DEFAULT,
    NO_ACTION
  };

  ForeignKeyConstraint(const std::vector<std::string> &columns,
                       const std::string &referenced_table,
                       const std::vector<std::string> &referenced_columns,
                       const std::string &name = "",
                       CascadeAction on_delete = RESTRICT,
                       CascadeAction on_update = RESTRICT);

  const std::vector<std::string> &getColumns() const;
  const std::string &getReferencedTable() const;
  const std::vector<std::string> &getReferencedColumns() const;
  const std::string &getName() const;
  CascadeAction getOnDeleteAction() const;
  CascadeAction getOnUpdateAction() const;

private:
  std::vector<std::string> columns_;
  std::string referenced_table_;
  std::vector<std::string> referenced_columns_;
  std::string name_;
  CascadeAction on_delete_;
  CascadeAction on_update_;
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
 * 空值约束类
 */
class NotNullConstraint {
public:
  NotNullConstraint(const std::string &column, const std::string &name = "");

  const std::string &getColumn() const;
  const std::string &getName() const;

private:
  std::string column_;
  std::string name_;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_CONSTRAINT_H
