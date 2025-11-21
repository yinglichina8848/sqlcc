#include "constraint_executor.h"
#include <algorithm>
#include <string>
#include <vector>

namespace sqlcc {

// ForeignKeyConstraintExecutor 实现

ForeignKeyConstraintExecutor::ForeignKeyConstraintExecutor(
    const sql_parser::ForeignKeyConstraint &constraint,
    StorageEngine &storage_engine)
    : constraint_(constraint), storage_engine_(storage_engine),
      current_table_name_("unknown") {
  // 将外键列名转换为小写，以便匹配
  for (const auto &col : constraint_.getColumns()) {
    lower_foreign_key_columns_.push_back(toLower(col));
  }
  lower_referenced_table_ = toLower(constraint_.getReferencedTable());
  lower_referenced_column_ = toLower(constraint_.getReferencedColumn());
}

bool ForeignKeyConstraintExecutor::validateInsert(
    const std::vector<std::string> &record,
    const std::vector<sql_parser::ColumnDefinition> &table_schema) {

  // 检查外键列是否在record中，如果不在或为NULL，允许插入
  std::string foreign_key_value = getForeignKeyValue(record, table_schema);
  if (foreign_key_value.empty()) {
    return true; // NULL值允许插入
  }

  // 检查被引用表中是否存在对应的记录
  return parentRecordExists(foreign_key_value);
}

bool ForeignKeyConstraintExecutor::validateUpdate(
    const std::vector<std::string> &old_record,
    const std::vector<std::string> &new_record,
    const std::vector<sql_parser::ColumnDefinition> &table_schema) {

  // 只有当外键列被修改时才需要验证
  std::string old_fk_value = getForeignKeyValue(old_record, table_schema);
  std::string new_fk_value = getForeignKeyValue(new_record, table_schema);

  if (old_fk_value == new_fk_value) {
    return true; // 外键值没有变化，不需要验证
  }

  // 新值为空(NULL)或在父表中存在
  if (new_fk_value.empty() || parentRecordExists(new_fk_value)) {
    return true;
  }

  return false;
}

bool ForeignKeyConstraintExecutor::validateDelete(
    const std::vector<std::string> &record,
    const std::vector<sql_parser::ColumnDefinition> &table_schema) {

  // 对于删除操作，需要检查是否存在子表记录引用此父表记录
  std::string primary_key_value = getPrimaryKeyValue(record, table_schema);
  if (primary_key_value.empty()) {
    return true; // 主键为空，无法被引用
  }

  // 检查是否有子表记录引用此主键值
  return !hasChildReferences(primary_key_value);
}

const std::string &ForeignKeyConstraintExecutor::getConstraintName() const {
  return constraint_.getName();
}

bool ForeignKeyConstraintExecutor::parentRecordExists(
    const std::string &foreign_key_value) {

  try {
    // 查询被引用表中的记录
    std::string query =
        "SELECT COUNT(*) FROM " + constraint_.getReferencedTable() + " WHERE " +
        constraint_.getReferencedColumn() + " = '" + foreign_key_value + "'";

    // 简化的查询执行（实际实现需要通过StorageEngine的查询接口）
    // 这里返回true表示允许通过，生产环境需要实际的表查询
    return true; // 占位符实现

  } catch (const std::exception &e) {
    // 记录错误但不抛出异常，返回false表示验证失败
    return false;
  }
}

bool ForeignKeyConstraintExecutor::hasChildReferences(
    const std::string &primary_key_value) {

  try {
    // 查询当前表中是否有记录引用此主键值
    std::string query = "SELECT COUNT(*) FROM " + current_table_name_ +
                        " WHERE " + constraint_.getColumns()[0] + " = '" +
                        primary_key_value + "'";

    // 简化的查询执行（实际实现需要通过StorageEngine的查询接口）
    // 这里返回false表示没有引用，允许删除
    return false; // 占位符实现

  } catch (const std::exception &e) {
    // 记录错误但不抛出异常，返回true表示有引用（保险起见）
    return true;
  }
}

std::string ForeignKeyConstraintExecutor::getForeignKeyValue(
    const std::vector<std::string> &record,
    const std::vector<sql_parser::ColumnDefinition> &table_schema) {

  // 找到外键列在表结构中的索引
  for (size_t i = 0; i < table_schema.size(); ++i) {
    std::string col_name_lower = toLower(table_schema[i].getName());
    if (std::find(lower_foreign_key_columns_.begin(),
                  lower_foreign_key_columns_.end(),
                  col_name_lower) != lower_foreign_key_columns_.end()) {
      if (i < record.size()) {
        return record[i];
      }
    }
  }

  // 如果找不到外键列，返回空字符串
  return "";
}

std::string ForeignKeyConstraintExecutor::getPrimaryKeyValue(
    const std::vector<std::string> &record,
    const std::vector<sql_parser::ColumnDefinition> &table_schema) {

  // 找到被引用列（实际上应该是主键）对应的值
  // 这里简化处理，当只有一个外键列时返回该列的值
  if (constraint_.getColumns().size() == 1) {
    return getForeignKeyValue(record, table_schema);
  }

  // 多列外键情况，返回空字符串（需要更复杂的主键查找逻辑）
  return "";
}

void ForeignKeyConstraintExecutor::setCurrentTableName(
    const std::string &table_name) {
  current_table_name_ = table_name;
  // 重置列名的小写版本
  lower_foreign_key_columns_.clear();
  for (const auto &col : constraint_.getColumns()) {
    lower_foreign_key_columns_.push_back(toLower(col));
  }
}

std::string
ForeignKeyConstraintExecutor::toLower(const std::string &str) const {
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

// UniqueConstraintExecutor 实现

UniqueConstraintExecutor::UniqueConstraintExecutor(
    const sql_parser::TableConstraint &constraint,
    StorageEngine &storage_engine, const std::string &table_name,
    bool is_primary_key)
    : constraint_(constraint), storage_engine_(storage_engine),
      table_name_(table_name), is_primary_key_(is_primary_key) {
  // 初始化列名的小写版本
  if (is_primary_key) {
    // 主键约束
    const auto &pk_constraint =
        dynamic_cast<const sql_parser::PrimaryKeyConstraint &>(
            constraint_.get());
    for (const auto &col : pk_constraint.getColumns()) {
      lower_constraint_columns_.push_back(toLower(col));
    }
  } else {
    // 唯一约束
    const auto &unique_constraint =
        dynamic_cast<const sql_parser::UniqueConstraint &>(constraint_.get());
    for (const auto &col : unique_constraint.getColumns()) {
      lower_constraint_columns_.push_back(toLower(col));
    }
  }
}

bool UniqueConstraintExecutor::validateInsert(
    const std::vector<std::string> &record,
    const std::vector<sql_parser::ColumnDefinition> &table_schema) {

  // 获取约束列的值
  std::vector<std::string> constraint_values =
      getConstraintValues(record, table_schema);

  // 检查值列表中是否有空值（对于主键，不允许NULL；对于唯一索引，允许NULL但需要特殊处理）
  bool has_null = false;
  for (const auto &value : constraint_values) {
    if (value.empty()) {
      has_null = true;
      break;
    }
  }

  // 如果是主键约束，NULL值不允许
  if (is_primary_key_ && has_null) {
    return false;
  }

  // 如果有NULL值且不是主键约束，允许插入（SQL标准：NULL值不参与唯一性检查）
  if (!is_primary_key_ && has_null) {
    return true;
  }

  // 检查唯一性
  return checkUniqueness(constraint_values);
}

bool UniqueConstraintExecutor::validateUpdate(
    const std::vector<std::string> &old_record,
    const std::vector<std::string> &new_record,
    const std::vector<sql_parser::ColumnDefinition> &table_schema) {

  // 获取新旧记录的约束列值
  std::vector<std::string> old_values =
      getConstraintValues(old_record, table_schema);
  std::vector<std::string> new_values =
      getConstraintValues(new_record, table_schema);

  // 如果新旧值相同，不需要验证
  if (old_values == new_values) {
    return true;
  }

  // 检查新值是否包含NULL
  bool has_null = false;
  for (const auto &value : new_values) {
    if (value.empty()) {
      has_null = true;
      break;
    }
  }

  // 主键不允许NULL
  if (is_primary_key_ && has_null) {
    return false;
  }

  // 非主键约束的NULL值允许
  if (!is_primary_key_ && has_null) {
    return true;
  }

  // 检查新值的唯一性
  return checkUniqueness(new_values);
}

bool UniqueConstraintExecutor::validateDelete(
    [[maybe_unused]] const std::vector<std::string> &record,
    [[maybe_unused]] const std::vector<sql_parser::ColumnDefinition> &table_schema) {
  // 删除操作不影响唯一性约束
  return true;
}

const std::string &UniqueConstraintExecutor::getConstraintName() const {
  return constraint_.get().getName();
}

sql_parser::TableConstraint::Type
UniqueConstraintExecutor::getConstraintType() const {
  return is_primary_key_ ? sql_parser::TableConstraint::PRIMARY_KEY
                         : sql_parser::TableConstraint::UNIQUE;
}

bool UniqueConstraintExecutor::checkUniqueness(
    const std::vector<std::string> &values) {

  try {
    // 构建SELECT查询来检查是否存在重复值
    std::string query = "SELECT COUNT(*) FROM " + table_name_ + " WHERE ";

    std::vector<std::string> conditions;
    if (is_primary_key_) {
      const auto &pk_constraint =
          dynamic_cast<const sql_parser::PrimaryKeyConstraint &>(
              constraint_.get());
      const auto &columns = pk_constraint.getColumns();
      for (size_t i = 0; i < columns.size(); ++i) {
        std::string condition = columns[i] + " = '" + values[i] + "'";
        conditions.push_back(condition);
      }
    } else {
      const auto &unique_constraint =
          dynamic_cast<const sql_parser::UniqueConstraint &>(constraint_.get());
      const auto &columns = unique_constraint.getColumns();
      for (size_t i = 0; i < columns.size(); ++i) {
        std::string condition = columns[i] + " = '" + values[i] + "'";
        conditions.push_back(condition);
      }
    }

    // 连接所有条件
    for (size_t i = 0; i < conditions.size(); ++i) {
      if (i > 0)
        query += " AND ";
      query += conditions[i];
    }

    // 实际的查询执行应当返回是否存在记录
    // 这里返回true表示没有重复记录，允许插入
    return true; // 占位符实现

  } catch (const std::exception &e) {
    // 记录错误但不抛出异常，返回false表示验证失败
    return false;
  }
}

std::vector<std::string> UniqueConstraintExecutor::getConstraintValues(
    const std::vector<std::string> &record,
    const std::vector<sql_parser::ColumnDefinition> &table_schema) {

  std::vector<std::string> constraint_values;

  // 获取约束涉及的所有列名
  std::vector<std::string> constraint_columns;
  if (is_primary_key_) {
    const auto &pk_constraint =
        dynamic_cast<const sql_parser::PrimaryKeyConstraint &>(
            constraint_.get());
    constraint_columns = pk_constraint.getColumns();
  } else {
    const auto &unique_constraint =
        dynamic_cast<const sql_parser::UniqueConstraint &>(constraint_.get());
    constraint_columns = unique_constraint.getColumns();
  }

  // 将列名转换为小写
  std::vector<std::string> lower_columns;
  for (const auto &col : constraint_columns) {
    lower_columns.push_back(toLower(col));
  }

  // 按照列的顺序提取值
  for (const auto &constraint_col : lower_columns) {
    // 在表结构中找到对应列的索引
    for (size_t i = 0; i < table_schema.size(); ++i) {
      if (toLower(table_schema[i].getName()) == constraint_col) {
        if (i < record.size()) {
          constraint_values.push_back(record[i]);
        } else {
          constraint_values.push_back(""); // 缺少值的情况
        }
        break;
      }
    }
  }

  return constraint_values;
}

std::string UniqueConstraintExecutor::toLower(const std::string &str) const {
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

// CheckConstraintExecutor 实现

// FIXME: CheckConstraint 不能直接拷贝，这里使用引用
CheckConstraintExecutor::CheckConstraintExecutor(
    const sql_parser::CheckConstraint &constraint,
    const std::string &table_name)
    : constraint_(constraint), table_name_(table_name) {
  // 使用引用包装器避免拷贝问题
}

bool CheckConstraintExecutor::validateInsert(
    const std::vector<std::string> &record,
    const std::vector<sql_parser::ColumnDefinition> &table_schema) {

  // 求值CHECK约束表达式
  return evaluateCheckCondition(record, table_schema);
}

bool CheckConstraintExecutor::validateUpdate(
    [[maybe_unused]] const std::vector<std::string> &old_record,
    const std::vector<std::string> &new_record,
    const std::vector<sql_parser::ColumnDefinition> &table_schema) {

  // 只对新记录进行验证
  return evaluateCheckCondition(new_record, table_schema);
}

bool CheckConstraintExecutor::validateDelete(
    [[maybe_unused]] const std::vector<std::string> &record,
    [[maybe_unused]] const std::vector<sql_parser::ColumnDefinition> &table_schema) {
  // 删除操作不影响CHECK约束
  return true;
}

const std::string &CheckConstraintExecutor::getConstraintName() const {
  return constraint_.get().getName();
}

bool CheckConstraintExecutor::evaluateCheckCondition(
    const std::vector<std::string> &record,
    const std::vector<sql_parser::ColumnDefinition> &table_schema) {

  try {
    // 简化的CHECK约束验证
    // 实际实现需要完整的表达式求值引擎

    const auto &condition = constraint_.get().getCondition();
    if (!condition) {
      // 没有条件表达式，默认通过
      return true;
    }

    // 这里应该调用ExpressionEvaluator来求值约束表达式
    // 目前返回true作为占位符
    return ExpressionEvaluator::evaluate(condition.get(), record, table_schema);

  } catch (const std::exception &e) {
    // CHECK约束验证失败
    return false;
  }
}

// ExpressionEvaluator 辅助函数

bool ExpressionEvaluator::evaluate(
    const sql_parser::Expression *expr, const std::vector<std::string> &record,
    const std::vector<sql_parser::ColumnDefinition> &table_schema) {

  if (!expr)
    return true;

  switch (expr->getType()) {
  case sql_parser::Expression::BINARY: {
    const auto *binary_expr =
        dynamic_cast<const sql_parser::BinaryExpression *>(expr);
    return evaluateBinaryExpression(binary_expr, record, table_schema);
  }

  case sql_parser::Expression::IDENTIFIER: {
    // 标识符通常为真（简化处理）
    return true;
  }

  case sql_parser::Expression::STRING_LITERAL:
  case sql_parser::Expression::NUMERIC_LITERAL: {
    // 字面量通常为真（除非是布尔字面量）
    return true;
  }

  default: {
    // 其他表达式类型默认通过
    return true;
  }
  }
}

bool ExpressionEvaluator::evaluateBinaryExpression(
    const sql_parser::BinaryExpression *expr,
    [[maybe_unused]] const std::vector<std::string> &record,
    [[maybe_unused]] const std::vector<sql_parser::ColumnDefinition> &table_schema) {

  if (!expr)
    return true;

  // 简化的二元表达式求值
  // 这里只处理最基本的比较操作
  auto op = expr->getOperator();

  switch (op) {
  case sql_parser::Token::Type::OPERATOR_GREATER:
  case sql_parser::Token::Type::OPERATOR_LESS:
  case sql_parser::Token::Type::OPERATOR_EQUAL: {
    // 比较操作的简化验证
    return true; // 占位符实现
  }

  default: {
    return true;
  }
  }
}

} // namespace sqlcc
