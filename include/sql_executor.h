#pragma once

#include "constraint_executor.h"
#include "data_type.h"
#include "sql_parser/ast_nodes.h"
#include "sql_parser/parser.h"
#include "storage_engine.h"
#include "transaction_manager.h"
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace sqlcc {

// 前向声明
class IndexManager;

/**
 * @brief 数据记录结构
 * SQL执行的核心数据单元
 */
struct Record {
  std::vector<std::string> column_values; ///< 列值数组（按表定义的列顺序）
  uint64_t record_id;                     ///< 记录全局唯一ID
  uint64_t txn_id = 0;                    ///< 事务ID
  std::string table_name;                 ///< 所属表名

  Record() = default;
  Record(std::vector<std::string> values, uint64_t rid = 0)
      : column_values(std::move(values)), record_id(rid) {}
};

/**
 * @brief 表元数据结构
 * 描述表的完整信息，包括列定义、约束等
 */
struct TableMetadata {
  std::string table_name;                                 ///< 表名
  std::vector<sql_parser::ColumnDefinition> columns;      ///< 列定义
  std::unordered_map<std::string, size_t> column_indexes; ///< 列名到列索引映射
  std::vector<sql_parser::TableConstraint> constraints;   ///< 表级约束
  uint64_t record_count = 0;                              ///< 记录数量
  uint32_t root_page_id = 0;                              ///< 数据页面根节点

  /**
   * @brief 获取列定义
   * @param column_name 列名
   * @return 列定义，如果不存在返回nullptr
   */
  const sql_parser::ColumnDefinition *
  GetColumnDef(const std::string &column_name) const {
    auto it = column_indexes.find(column_name);
    if (it == column_indexes.end())
      return nullptr;
    return &columns[it->second];
  }
};

/**
 * @brief WHERE条件结构
 * 简化版WHERE条件表示，用于查询过滤
 */
struct WhereCondition {
  std::string column_name;   ///< 列名
  std::string operator_type; ///< 操作符: "=", ">", "<", "!=", etc.
  std::string value;         ///< 比较值

  WhereCondition(std::string col, std::string op, std::string val)
      : column_name(std::move(col)), operator_type(std::move(op)),
        value(std::move(val)) {}
};

/**
 * @brief SQL执行异常类
 */
class SqlExecutionException : public std::runtime_error {
public:
  explicit SqlExecutionException(const std::string &message)
      : std::runtime_error(message) {}
};

/**
 * @brief SQL执行器类，负责执行SQL语句
 *
 * 设计实现SQL语句实际执行，从简单的DDL/DML操作到复杂的查询处理
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

  /**
   * @brief 执行单个SQL语句
   * @param stmt SQL语句节点
   * @return 执行结果字符串
   */
  // 执行单个SQL语句
  std::string ExecuteStatement(const sql_parser::Statement *stmt);

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

private:
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
   * @brief 表元数据管理器
   * 存储所有表的元数据信息
   */
  std::unordered_map<std::string, TableMetadata> table_catalog_;

  /**
   * @brief 内存记录存储
   * 用于在内存中存储表记录数据
   */
  std::unordered_map<std::string, std::vector<Record>> records_;

  /**
   * @brief 记录管理器
   * 负责记录的CRUD操作
   */
  std::unique_ptr<Record> record_manager_; // TODO: 需要实现RecordManager类

  /**
   * @brief 索引执行器
   * 管理表的索引操作
   */
  IndexManager *index_executor_ =
      nullptr; // 从StorageEngine获取，使用原始指针避免类型不完整问题

  /**
   * @brief 事务管理器
   * 处理事务ACID保证
   */
  std::unique_ptr<TransactionManager> transaction_manager_; // TODO: 需要集成

  /**
   * @brief 表约束管理器，存储每个表的约束执行器列表
   * 键：表名（小写），值：约束执行器列表
   */
  std::unordered_map<std::string,
                     std::vector<std::unique_ptr<ConstraintExecutor>>>
      table_constraints_;

  /**
   * @brief 并发控制锁
   */
  std::mutex execution_mutex_;

  // ============== 私有辅助方法 ==============

  /**
   * @brief 创建表管理器方法
   */
  bool CreateTable(const std::string &table_name,
                   const std::vector<sql_parser::ColumnDefinition> &columns,
                   const std::vector<sql_parser::TableConstraint> &constraints);

  bool DropTable(const std::string &table_name);

  const TableMetadata *GetTableMetadata(const std::string &table_name) const;

  /**
   * @brief 记录管理器方法
   */
  bool InsertRecord(const std::string &table_name, const Record &record,
                    uint64_t &rid);

  bool UpdateRecord(const std::string &table_name, uint64_t rid,
                    const Record &new_record);

  bool DeleteRecord(const std::string &table_name, uint64_t rid);

  Record GetRecord(const std::string &table_name, uint64_t rid) const;

  std::vector<Record> GetAllRecords(const std::string &table_name) const;

  std::vector<Record> QueryRecords(const std::string &table_name,
                                   const WhereCondition &condition) const;

public:
  /**
   * @brief 约束验证方法
   */
  bool ValidateInsertConstraints(
      const std::string &table_name, const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema);

  bool ValidateUpdateConstraints(
      const std::string &table_name, const std::vector<std::string> &old_record,
      const std::vector<std::string> &new_record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema);

  bool ValidateDeleteConstraints(
      const std::string &table_name, const std::vector<std::string> &record,
      const std::vector<sql_parser::ColumnDefinition> &table_schema);

  bool ValidateConstraintDefinition(
      const sql_parser::TableConstraint &constraint,
      const std::vector<sql_parser::ColumnDefinition> &columns);

  /**
   * @brief 为表创建约束执行器（当表被创建或变更时调用）
   * @param table_name 表名
   * @param constraints 表的约束列表
   */
  void CreateTableConstraints(
      const std::string &table_name,
      const std::vector<sql_parser::TableConstraint> &constraints);

private:
  /**
   * @brief 获取表架构信息
   * @param table_name 表名
   * @return 表列定义列表
   */
  std::vector<sql_parser::ColumnDefinition>
  GetTableSchema(const std::string &table_name) const;

  /**
   * @brief 结果格式化方法
   */
  std::string FormatQueryResults(const std::vector<Record> &results,
                                 const std::vector<size_t> &column_indices,
                                 const TableMetadata &meta) const;

  std::string FormatTableSchema(const TableMetadata &meta) const;

  std::string FormatTableList(const std::vector<std::string> &tables) const;

  /**
   * @brief 表名标准化
   */
  std::string NormalizeTableName(const std::string &name) const {
    std::string lower_name;
    lower_name.reserve(name.size());
    for (char c : name) {
      lower_name += std::tolower(static_cast<unsigned char>(c));
    }
    return lower_name;
  }

  /**
   * @brief 解析WHERE条件为WhereCondition
   */
  std::vector<WhereCondition>
  ParseWhereClause(const sql_parser::WhereClause &where_clause) const;

  /**
   * @brief 评估WHERE条件
   */
  bool EvaluateWhereCondition(
      const sql_parser::WhereClause &where_clause, const Record &record,
      const std::vector<sql_parser::ColumnDefinition> &columns) const;
};

} // namespace sqlcc
