#include "sql_parser/ast_nodes.h"
#include "storage_engine.h"
#include <functional>
#include <string>
#include <vector>

namespace sqlcc {

/**
 * 约束执行器基类
 */
class ConstraintExecutor {
public:
  virtual ~ConstraintExecutor() = default;

  virtual bool validateInsert(
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema) = 0;

  virtual bool validateUpdate(
      const std::vector<std::string> &old_record,
      const std::vector<std::string> &new_record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema) = 0;

  virtual bool validateDelete(
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema) = 0;

  virtual const std::string &getConstraintName() const = 0;
};

/**
 * 外键约束执行器
 */
class ForeignKeyConstraintExecutor : public ConstraintExecutor {
public:
  ForeignKeyConstraintExecutor(const sql_parser::TableConstraint &constraint,
                               StorageEngine &storage_engine);

  bool validateInsert(
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema) override;

  bool validateUpdate(
      const std::vector<std::string> &old_record,
      const std::vector<std::string> &new_record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema) override;

  bool validateDelete(
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema) override;

  const std::string &getConstraintName() const override;

  void setCurrentTableName(const std::string &table_name);

private:
  bool parentRecordExists(const std::string &foreign_key_value);
  std::string getForeignKeyValue(
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema);
  std::string getPrimaryKeyValue(
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema);

  std::reference_wrapper<const sql_parser::TableConstraint> constraint_;
  StorageEngine &storage_engine_;
  std::string current_table_name_;
  std::vector<std::string> lower_foreign_key_columns_;
  std::string lower_referenced_table_;
  std::string lower_referenced_column_;
};

/**
 * 唯一约束执行器
 */
class UniqueConstraintExecutor : public ConstraintExecutor {
public:
  UniqueConstraintExecutor(const sql_parser::TableConstraint &constraint,
                           StorageEngine &storage_engine,
                           const std::string &table_name,
                           bool is_primary_key = false);

  bool validateInsert(
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema) override;

  bool validateUpdate(
      const std::vector<std::string> &old_record,
      const std::vector<std::string> &new_record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema) override;

  bool validateDelete(
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema) override;

  const std::string &getConstraintName() const override;

private:
  bool checkUniqueness(const std::vector<std::string> &constraint_values);
  std::vector<std::string> getConstraintValues(
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema);

  std::reference_wrapper<const sql_parser::TableConstraint> constraint_;
  StorageEngine &storage_engine_;
  std::string table_name_;
  bool is_primary_key_;
  std::vector<std::string> lower_constraint_columns_;
};

/**
 * 检查约束执行器
 */
class CheckConstraintExecutor : public ConstraintExecutor {
public:
  CheckConstraintExecutor(const sql_parser::TableConstraint &constraint,
                          const std::string &table_name);

  bool validateInsert(
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema) override;

  bool validateUpdate(
      const std::vector<std::string> &old_record,
      const std::vector<std::string> &new_record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema) override;

  bool validateDelete(
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema) override;

  const std::string &getConstraintName() const override;

private:
  bool evaluateCheckCondition(
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema);

  std::reference_wrapper<const sql_parser::TableConstraint> constraint_;
  std::string table_name_;
};

/**
 * 表达式求值器
 */
class ExpressionEvaluator {
public:
  static bool
  evaluate(const sql_parser::Expression *expr,
           const std::vector<std::string> &record,
           const std::vector<sql_parser::ColumnDefinition> &table_schema);

private:
  static bool evaluateBinaryExpression(
      const sql_parser::BinaryExpression *binary_expr,
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema);
};

} // namespace sqlcc