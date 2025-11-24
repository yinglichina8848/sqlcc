#pragma once

#include "sql_parser/ast_nodes.h"
#include "storage_engine.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sqlcc {

/**
 * @brief 约束执行器接口
 *
 * 定义约束验证的通用接口，所有具体的约束执行器都需要实现此接口
 */
class ConstraintExecutor {
public:
  virtual ~ConstraintExecutor() = default;

  /**
   * @brief 验证插入操作
   * @param record 要插入的记录
   * @param table_schema 表结构信息
   * @return true if validation passes, false otherwise
   */
  virtual bool validateInsert(
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema) = 0;

  /**
   * @brief 验证更新操作
   * @param old_record 旧记录
   * @param new_record 新记录
   * @param table_schema 表结构信息
   * @return true if validation passes, false otherwise
   */
  virtual bool validateUpdate(
      const std::vector<std::string> &old_record,
      const std::vector<std::string> &new_record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema) = 0;

  /**
   * @brief 验证删除操作
   * @param record 要删除的记录
   * @param table_schema 表结构信息
   * @return true if validation passes, false otherwise
   */
  virtual bool validateDelete(
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema) = 0;

  /**
   * @brief 获取约束名称
   * @return 约束名称
   */
  virtual const std::string &getConstraintName() const = 0;

  /**
   * @brief 获取约束类型
   * @return 约束类型
   */
  virtual sql_parser::TableConstraint::Type getConstraintType() const = 0;
};

/**
 * @brief 外键约束执行器
 *
 * 负责验证外键参照完整性约束
 */
class ForeignKeyConstraintExecutor : public ConstraintExecutor {
public:
  /**
   * @brief 构造函数
   * @param constraint 外键约束定义
   * @param storage_engine 存储引擎引用
   */
  ForeignKeyConstraintExecutor(
      const sql_parser::ForeignKeyConstraint &constraint,
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
  sql_parser::TableConstraint::Type getConstraintType() const override {
    return sql_parser::TableConstraint::FOREIGN_KEY;
  }

private:
  /**
   * @brief 检查父表中是否存在对应的记录
   * @param foreign_key_value 外键值
   * @return true if parent record exists
   */
  bool parentRecordExists(const std::string &foreign_key_value);

  /**
   * @brief 检查是否有子表记录引用此父表记录
   * @param primary_key_value 主键值
   * @return true if has child references
   */
  bool hasChildReferences(const std::string &primary_key_value);

  /**
   * @brief 获取记录中外键列的值
   * @param record 记录数据
   * @param table_schema 表结构
   * @return 外键值
   */
  std::string getForeignKeyValue(
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema);

  /**
   * @brief 获取记录中主键的值
   * @param record 记录数据
   * @param table_schema 表结构
   * @return 主键值
   */
  std::string getPrimaryKeyValue(
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema);

public:
  /**
   * @brief 设置当前表名
   * @param table_name 表名
   */
  void setCurrentTableName(const std::string &table_name);

public:
  /**
   * @brief 字符串转换为小写
   * @param str 输入字符串
   * @return 小写字符串
   */
  std::string toLower(const std::string &str) const;

  sql_parser::ForeignKeyConstraint constraint_;        // 外键约束定义
  StorageEngine &storage_engine_;                      // 存储引擎引用
  std::string current_table_name_;                     // 当前表名
  std::vector<std::string> lower_foreign_key_columns_; // 小写的列名
  std::string lower_referenced_table_;                 // 被引用表名（小写）
  std::string lower_referenced_column_;                // 被引用列名（小写）
};

/**
 * @brief 唯一约束执行器
 *
 * 负责验证唯一性约束，包括主键和UNIQUE约束
 */
class UniqueConstraintExecutor : public ConstraintExecutor {
public:
  /**
   * @brief 构造函数
   * @param constraint 唯一约束定义（主键或UNIQUE）
   * @param storage_engine 存储引擎引用
   * @param table_name 表名
   * @param is_primary_key 是否为主键约束
   */
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
  sql_parser::TableConstraint::Type getConstraintType() const override;

private:
  /**
   * @brief 检查值是否唯一
   * @param values 要检查的值列表
   * @return true if values are unique
   */
  bool checkUniqueness(const std::vector<std::string> &values);

  /**
   * @brief 获取记录中约束列的值
   * @param record 记录数据
   * @param table_schema 表结构
   * @return 约束列值列表
   */
  std::vector<std::string> getConstraintValues(
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema);

  /**
   * @brief 字符串转换为小写
   * @param str 输入字符串
   * @return 小写字符串
   */
  std::string toLower(const std::string &str) const;

  std::reference_wrapper<const sql_parser::TableConstraint>
      constraint_;                                    // 约束定义
  StorageEngine &storage_engine_;                     // 存储引擎引用
  std::string table_name_;                            // 表名
  bool is_primary_key_;                               // 是否为主键
  std::vector<std::string> lower_constraint_columns_; // 小写的列名
};

/**
 * @brief CHECK约束执行器
 *
 * 负责验证CHECK约束条件
 */
class CheckConstraintExecutor : public ConstraintExecutor {
public:
  /**
   * @brief 构造函数
   * @param constraint CHECK约束定义
   * @param table_name 表名
   */
  CheckConstraintExecutor(const sql_parser::CheckConstraint &constraint,
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
  sql_parser::TableConstraint::Type getConstraintType() const override {
    return sql_parser::TableConstraint::CHECK;
  }

private:
  /**
   * @brief 求值CHECK约束表达式
   * @param record 记录数据
   * @param table_schema 表结构
   * @return true if condition evaluates to true
   */
  bool evaluateCheckCondition(
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema);

  // FIXME: CheckConstraint 不能直接拷贝，使用引用避免拷贝
  std::reference_wrapper<const sql_parser::CheckConstraint>
      constraint_;         // CHECK约束定义
  std::string table_name_; // 表名
};

/**
 * @brief 表达式求值器
 *
 * 为CHECK约束提供表达式求值功能
 */
class ExpressionEvaluator {
public:
  /**
   * @brief 求值表达式
   * @param expr 表达式节点
   * @param record 记录数据
   * @param table_schema 表结构
   * @return 求值结果
   */
  static bool
  evaluate(const sql_parser::Expression *expr,
           const std::vector<std::string> &record,
           const std::vector<sql_parser::ColumnDefinition> &table_schema);

private:
  /**
   * @brief 求值二元表达式
   */
  static bool evaluateBinaryExpression(
      const sql_parser::BinaryExpression *expr,
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema);

  /**
   * @brief 求值标识符（列引用）
   */
  static std::string evaluateIdentifier(
      const sql_parser::IdentifierExpression *expr,
      const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema);

  /**
   * @brief 获取列索引
   */
  static int
  getColumnIndex(const std::string &column_name,
                 const std::vector<sql_parser::ColumnDefinition> &table_schema);
};

} // namespace sqlcc
