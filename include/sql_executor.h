#pragma once

#include "constraint_executor.h"
#include "data_type.h"
#include "sql_parser/ast_nodes.h"
#include "sql_parser/parser.h"
#include "storage_engine.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace sqlcc {

/**
 * @brief SQL执行器类，负责执行SQL语句
 *
 * Why: 需要一个组件来将SQL解析器生成的AST转换为实际的存储引擎操作
 * What: SqlExecutor类提供SQL语句的执行功能，处理不同类型的SQL语句
 * How: 通过组合SQL解析器和存储引擎，实现SQL语句的执行和结果返回
 */
class SqlExecutor {
public:
  /**
   * @brief 默认构造函数，用于模拟模式
   */
  SqlExecutor();

  /**
   * @brief 构造函数，初始化SQL执行器
   * @param storage_engine 存储引擎实例的引用
   */
  explicit SqlExecutor(StorageEngine &storage_engine);

  /**
   * @brief 析构函数
   */
  ~SqlExecutor() = default;

  /**
   * @brief 禁止拷贝构造
   */
  SqlExecutor(const SqlExecutor &) = delete;

  /**
   * @brief 禁止赋值操作
   */
  SqlExecutor &operator=(const SqlExecutor &) = delete;

  /**
   * @brief 执行SQL语句
   * @param sql SQL语句字符串
   * @return 执行结果字符串
   */
  std::string Execute(const std::string &sql);

  /**
   * @brief 执行SQL脚本文件
   * @param file_path 脚本文件路径
   * @return 执行结果字符串
   */
  std::string ExecuteFile(const std::string &file_path);

  /**
   * @brief 获取最后一次执行的错误信息
   * @return 错误信息字符串
   */
  const std::string &GetLastError() const;

  /**
   * @brief 显示表结构信息
   * @param table_name 表名
   * @return 表结构信息字符串
   */
  std::string ShowTableSchema(const std::string &table_name);

  /**
   * @brief 列出所有表
   * @return 表列表字符串
   */
  std::string ListTables();

private:
  /**
   * @brief 执行单个SQL语句
   * @param stmt SQL语句节点
   * @return 执行结果字符串
   */
  std::string ExecuteStatement(const sql_parser::Statement &stmt);

  /**
   * @brief 执行SELECT语句
   * @param select_stmt SELECT语句节点
   * @return 执行结果字符串
   */
  std::string ExecuteSelect(const sql_parser::SelectStatement &select_stmt);

  /**
   * @brief 执行INSERT语句
   * @param insert_stmt INSERT语句节点
   * @return 执行结果字符串
   */
  std::string ExecuteInsert(const sql_parser::InsertStatement &insert_stmt);

  /**
   * @brief 执行UPDATE语句
   * @param update_stmt UPDATE语句节点
   * @return 执行结果字符串
   */
  std::string ExecuteUpdate(const sql_parser::UpdateStatement &update_stmt);

  /**
   * @brief 执行DELETE语句
   * @param delete_stmt DELETE语句节点
   * @return 执行结果字符串
   */
  std::string ExecuteDelete(const sql_parser::DeleteStatement &delete_stmt);

  /**
   * @brief 执行CREATE语句
   * @param create_stmt CREATE语句节点
   * @return 执行结果字符串
   */
  std::string ExecuteCreate(const sql_parser::CreateStatement &create_stmt);

  /**
   * @brief 执行DROP语句
   * @param drop_stmt DROP语句节点
   * @return 执行结果字符串
   */
  std::string ExecuteDrop(const sql_parser::DropStatement &drop_stmt);

  /**
   * @brief 执行ALTER语句
   * @param alter_stmt ALTER语句节点
   * @return 执行结果字符串
   */
  std::string ExecuteAlter(const sql_parser::AlterStatement &alter_stmt);

  /**
   * @brief 执行USE语句
   * @param use_stmt USE语句节点
   * @return 执行结果字符串
   */
  std::string ExecuteUse(const sql_parser::UseStatement &use_stmt);

  /**
   * @brief 执行CREATE INDEX语句，创建索引
   * @param create_index_stmt CREATE INDEX语句节点
   * @return 执行结果字符串
   */
  std::string
  ExecuteCreateIndex(const sql_parser::CreateIndexStatement &create_index_stmt);

  /**
   * @brief 执行DROP INDEX语句，删除索引
   * @param drop_index_stmt DROP INDEX语句节点
   * @return 执行结果字符串
   */
  std::string
  ExecuteDropIndex(const sql_parser::DropIndexStatement &drop_index_stmt);

  /**
   * @brief 设置错误信息
   * @param error 错误信息字符串
   */
  void SetError(const std::string &error);

private:
  std::shared_ptr<StorageEngine> storage_engine_; ///< 存储引擎智能指针
  std::string last_error_;                        ///< 最后一次错误信息
  std::string current_database_;                  ///< 当前选中的数据库名称

  /**
   * @brief 表约束管理器，存储每个表的约束执行器列表
   * 键：表名（小写），值：约束执行器列表
   */
  std::unordered_map<std::string,
                     std::vector<std::unique_ptr<ConstraintExecutor>>>
      table_constraints_;

  /**
   * @brief 验证记录是否满足INSERT操作的约束条件
   * @param table_name 表名
   * @param record 插入的记录
   * @param table_schema 表结构信息
   * @return true表示通过验证，false表示违反约束
   */
  bool ValidateInsertConstraints(
      const std::string &table_name, const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema);

  /**
   * @brief 验证记录UPDATE是否违反约束条件
   * @param table_name 表名
   * @param old_record 旧记录
   * @param new_record 新记录
   * @param table_schema 表结构信息
   * @return true表示通过验证，false表示违反约束
   */
  bool ValidateUpdateConstraints(
      const std::string &table_name, const std::vector<std::string> &old_record,
      const std::vector<std::string> &new_record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema);

  /**
   * @brief 验证记录DELETE是否违反约束条件（外键约束检查）
   * @param table_name 表名
   * @param record 删除的记录
   * @param table_schema 表结构信息
   * @return true表示可以删除，false表示违反约束
   */
  bool ValidateDeleteConstraints(
      const std::string &table_name, const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema);

  /**
   * @brief 为表创建约束执行器（当表被创建或变更时调用）
   * @param table_name 表名
   * @param constraints 表的约束列表
   */
  void CreateTableConstraints(
      const std::string &table_name,
      const std::vector<sql_parser::TableConstraint> &constraints);

  /**
   * @brief 获取表架构信息
   * @param table_name 表名
   * @return 表列定义列表
   */
  std::vector<sql_parser::ColumnDefinition>
  GetTableSchema(const std::string &table_name);
};

} // namespace sqlcc
