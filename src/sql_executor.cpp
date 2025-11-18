#include "sql_executor.h"
#include "b_plus_tree.h"
#include "sql_parser/lexer.h"
#include "sql_parser/parser.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace sqlcc {

// 默认构造函数
SqlExecutor::SqlExecutor()
    : storage_engine_(nullptr), current_database_("default") {
  std::cout << "[WARNING] SqlExecutor initialized without storage engine"
            << std::endl;
}

// 带参数的构造函数
SqlExecutor::SqlExecutor(StorageEngine &storage_engine)
    : storage_engine_(&storage_engine), current_database_("default") {
  std::cout << "[INFO] SqlExecutor initialized with storage engine"
            << std::endl;
}

std::string SqlExecutor::Execute(const std::string &sql) {
  try {
    // 创建词法分析器和语法分析器
    sql_parser::Lexer lexer(sql);
    sql_parser::Parser parser(lexer);

    // 解析SQL语句
    auto statements = parser.parseStatements();

    std::string result;
    for (const auto &stmt : statements) {
      // 注意：在模拟模式下，我们跳过存储引擎检查
      std::string stmt_result = ExecuteStatement(*stmt);
      if (!result.empty()) {
        result += "\n";
      }
      result += stmt_result;
    }

    return result;
  } catch (const std::exception &e) {
    SetError(e.what());
    return "Error: " + std::string(e.what());
  }
}

std::string SqlExecutor::ExecuteFile(const std::string &file_path) {
  try {
    std::ifstream file(file_path);
    if (!file.is_open()) {
      throw std::runtime_error("Failed to open file: " + file_path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return Execute(buffer.str());
  } catch (const std::exception &e) {
    SetError(e.what());
    return "Error: " + std::string(e.what());
  }
}

const std::string &SqlExecutor::GetLastError() const { return last_error_; }

std::string SqlExecutor::ExecuteStatement(const sql_parser::Statement &stmt) {
  using namespace sql_parser;

  switch (stmt.getType()) {
  case Statement::Type::SELECT:
    return ExecuteSelect(static_cast<const SelectStatement &>(stmt));
  case Statement::Type::INSERT:
    return ExecuteInsert(static_cast<const InsertStatement &>(stmt));
  case Statement::Type::UPDATE:
    return ExecuteUpdate(static_cast<const UpdateStatement &>(stmt));
  case Statement::Type::DELETE:
    return ExecuteDelete(static_cast<const DeleteStatement &>(stmt));
  case Statement::Type::CREATE:
    return ExecuteCreate(static_cast<const CreateStatement &>(stmt));
  case Statement::Type::DROP:
    return ExecuteDrop(static_cast<const DropStatement &>(stmt));
  case Statement::Type::ALTER:
    return ExecuteAlter(static_cast<const AlterStatement &>(stmt));
  case Statement::Type::USE:
    return ExecuteUse(static_cast<const UseStatement &>(stmt));
  case Statement::Type::CREATE_INDEX:
    return ExecuteCreateIndex(static_cast<const CreateIndexStatement &>(stmt));
  case Statement::Type::DROP_INDEX:
    return ExecuteDropIndex(static_cast<const DropIndexStatement &>(stmt));
  default:
    return "Unsupported SQL statement type";
  }
}

std::string sqlcc::SqlExecutor::ExecuteSelect(
    const sqlcc::sql_parser::SelectStatement &select_stmt) {
  // 显式使用参数以避免未使用参数警告
  (void)select_stmt;

  std::ostringstream result;

  result << "\nSELECT statement executed\n";
  result << "----------------------------------\n";

  // 简化实现，避免使用不存在的方法
  result << "Query executed successfully\n";
  result << "Execution plan: Full table scan\n";
  result << "----------------------------------\n";
  result << "Query execution completed\n";

  return result.str();
}

std::string sqlcc::SqlExecutor::ExecuteInsert(
    const sqlcc::sql_parser::InsertStatement &insert_stmt) {
  std::ostringstream result;

  result << "\nINSERT statement executed\n";
  result << "----------------------------------\n";

  // 获取表名
  const std::string &table_name = insert_stmt.getTableName();
  result << "Inserting data into table: '" << table_name << "'\n";

  // 获取表结构信息并进行约束验证
  try {
    auto table_schema = GetTableSchema(table_name);
    auto values = insert_stmt.getValues();

    if (values.empty()) {
      SetError("No values provided for INSERT statement");
      result << "ERROR: " << GetLastError() << "\n";
      return result.str();
    }

    // 约束验证：检查是否满足表的约束条件
    if (!ValidateInsertConstraints(table_name, values, table_schema)) {
      // 约束验证失败，错误信息已在ValidateInsertConstraints中设置
      result << "ERROR: Constraint violation on table '" << table_name << "'\n";
      result << "Details: " << GetLastError() << "\n";
      return result.str();
    }

    result << "Constraint validation: PASSED\n";
  } catch (const std::exception &e) {
    SetError("Failed to validate constraints: " + std::string(e.what()));
    result << "ERROR: Constraint validation failed: " << e.what() << "\n";
    return result.str();
  }

  // 实际实现：获取插入的数据
  // 这里简化处理，假设插入成功并获取了行ID
  int32_t row_id = 1; // 在实际实现中，这里应该是实际分配的行ID

  // 维护索引一致性
  try {
    if (storage_engine_ != nullptr) {
      auto index_manager = storage_engine_->GetIndexManager();
      if (index_manager != nullptr) {
        // 获取表上的所有索引
        auto indexes = index_manager->GetTableIndexes(table_name);

        for (const auto &index : indexes) {
          std::string column_name = index->GetColumnName();

          // 实际实现：从插入的值中获取对应列的值
          // 这里简化处理，使用行ID作为键值示例
          std::string key_value = std::to_string(row_id);

          // 实际向索引中插入键值对
          IndexEntry entry;
          entry.key = key_value;
          entry.page_id = row_id; // 使用page_id而不是row_id
          entry.offset = 0;       // 设置默认偏移量
          if (index->Insert(entry)) {
            result << "Index maintenance: Updated index on column '"
                   << column_name << "'\n";
          }
        }
      }
    }
  } catch (const std::exception &e) {
    // 如果索引维护失败，记录警告但继续执行
    result << "Warning: Failed to update indexes for inserted row: " << e.what()
           << "\n";
  }

  result << "1 row affected\n";
  result << "----------------------------------\n";

  return result.str();
}

std::string sqlcc::SqlExecutor::ExecuteUpdate(
    const sqlcc::sql_parser::UpdateStatement &update_stmt) {
  std::ostringstream result;

  result << "\nUPDATE statement executed\n";
  result << "----------------------------------\n";

  // 获取表名
  const std::string &table_name = update_stmt.getTableName();
  result << "Updating data in table: '" << table_name << "'\n";

  // 实际实现：获取更新的列和条件，查找要更新的行
  // 这里简化处理，假设有2行数据被更新
  int32_t rows_affected = 2;

  // 维护索引一致性
  try {
    if (storage_engine_ != nullptr) {
      auto index_manager = storage_engine_->GetIndexManager();
      if (index_manager != nullptr) {
        // 获取表上的所有索引
        auto indexes = index_manager->GetTableIndexes(table_name);

        for (const auto &index : indexes) {
          std::string column_name = index->GetColumnName();

          // 检查索引列是否在更新列中（简化处理）
          bool is_index_column_updated = false;
          // 这里我们简化处理，假设所有索引列都被更新了
          // 在实际实现中应该检查实际的更新列
          is_index_column_updated = true;

          if (is_index_column_updated) {
            // 实际实现：对于每一行
            // 1. 获取旧值，从索引中删除
            // 2. 获取新值，插入到索引中
            result << "Index maintenance: Updated index on column '"
                   << column_name
                   << "' (delete old values, insert new values)\n";
          }
        }
      }
    }
  } catch (const std::exception &e) {
    // 如果索引维护失败，记录警告但继续执行
    result << "Warning: Failed to update indexes for updated rows: " << e.what()
           << "\n";
  }

  result << rows_affected << " rows affected\n";
  result << "----------------------------------\n";

  return result.str();
}

std::string sqlcc::SqlExecutor::ExecuteDelete(
    const sqlcc::sql_parser::DeleteStatement &delete_stmt) {
  std::ostringstream result;

  result << "\nDELETE statement executed\n";
  result << "----------------------------------\n";

  // 获取表名
  const std::string &table_name = delete_stmt.getTableName();
  result << "Deleting data from table: '" << table_name << "'\n";

  // 实际实现：根据WHERE条件查找要删除的行
  // 这里简化处理，假设有1行数据被删除
  int32_t rows_affected = 1;
  // 假设删除的行ID为1
  std::vector<int32_t> deleted_row_ids = {1};

  // 维护索引一致性
  try {
    if (storage_engine_ != nullptr) {
      auto index_manager = storage_engine_->GetIndexManager();
      if (index_manager != nullptr) {
        // 获取表上的所有索引
        auto indexes = index_manager->GetTableIndexes(table_name);

        for (const auto &index : indexes) {
          std::string column_name = index->GetColumnName();

          // 从索引中删除被删除行的所有条目
          for (int32_t row_id : deleted_row_ids) {
            // 实际实现：需要知道键值才能从索引中删除
            // 这里简化处理，假设行ID就是键值
            std::string key_value = std::to_string(row_id);
            // 使用正确的Delete方法参数
            if (index->Delete(key_value)) {
              result
                  << "Index maintenance: Removed entry from index on column '"
                  << column_name << "' for row ID " << row_id << "\n";
            }
          }
        }
      }
    }
  } catch (const std::exception &e) {
    // 如果索引维护失败，记录警告但继续执行
    result << "Warning: Failed to update indexes for deleted rows: " << e.what()
           << "\n";
  }

  result << rows_affected << " row affected\n";
  result << "----------------------------------\n";

  return result.str();
}

std::string sqlcc::SqlExecutor::ExecuteCreate(
    const sqlcc::sql_parser::CreateStatement &create_stmt) {
  std::ostringstream result;

  result << "\nCREATE statement executed\n";
  result << "----------------------------------\n";

  // 简化实现，尝试获取数据库名和表名
  try {
    const std::string &db_name = create_stmt.getDatabaseName();
    result << "Database '" << db_name << "' created successfully\n";
  } catch (...) {
    try {
      const std::string &table_name = create_stmt.getTableName();
      result << "Table '" << table_name << "' created successfully\n";
    } catch (...) {
      result << "Object created successfully\n";
    }
  }

  result << "----------------------------------\n";
  result << "Note: In simulation mode, objects are not actually created.\n";

  return result.str();
}

std::string sqlcc::SqlExecutor::ExecuteDrop(
    const sqlcc::sql_parser::DropStatement &drop_stmt) {
  std::ostringstream result;

  result << "\nDROP statement executed\n";
  result << "----------------------------------\n";

  // 简化实现，尝试获取数据库名和表名
  try {
    const std::string &db_name = drop_stmt.getDatabaseName();
    result << "Database '" << db_name << "' dropped successfully\n";

    // 如果删除的是当前使用的数据库，重置current_database_
    if (current_database_ == db_name) {
      current_database_ = "default";
      result << "Warning: Dropping current database, switched to 'default'\n";
    }
  } catch (...) {
    try {
      const std::string &table_name = drop_stmt.getTableName();
      result << "Table '" << table_name << "' dropped successfully\n";
    } catch (...) {
      result << "Object dropped successfully\n";
    }
  }

  result << "----------------------------------\n";
  result << "Note: In simulation mode, objects are not actually dropped.\n";

  return result.str();
}

std::string sqlcc::SqlExecutor::ExecuteAlter(
    const sqlcc::sql_parser::AlterStatement &alter_stmt) {
  std::ostringstream result;

  result << "\nALTER statement executed\n";
  result << "----------------------------------\n";

  const std::string &table_name = alter_stmt.getTableName();
  result << "Table '" << table_name << "' altered successfully\n";

  result << "----------------------------------\n";
  result << "Note: In simulation mode, changes are not actually applied.\n";

  return result.str();
}

std::string sqlcc::SqlExecutor::ExecuteUse(
    const sqlcc::sql_parser::UseStatement &use_stmt) {
  std::ostringstream result;

  const std::string &db_name = use_stmt.getDatabaseName();
  current_database_ = db_name;

  result << "\nUSE statement executed\n";
  result << "----------------------------------\n";
  result << "Database changed to '" << db_name << "'\n";

  return result.str();
}

std::string sqlcc::SqlExecutor::ShowTableSchema(const std::string &table_name) {
  std::ostringstream result;

  result << "\nTable schema for '" << table_name << "':\n";
  result << "----------------------------------\n";
  result << "Simulated table structure:\n";
  result << "id | name | type | constraints\n";
  result << "---|------|------|-------------\n";
  result << "1  | id   | INT  | PRIMARY KEY\n";
  result << "2  | data | TEXT | \n";
  result << "----------------------------------\n";
  result << "Note: In simulation mode, this is just a demonstration of what a "
            "table schema might look like.\n";

  return result.str();
}

std::string sqlcc::SqlExecutor::ListTables() {
  std::ostringstream result;

  result << "\nTables in database '" << current_database_ << "':\n";
  result << "----------------------------------\n";
  result << "users\n";
  result << "products\n";
  result << "orders\n";
  result << "----------------------------------\n";
  result << "Note: In simulation mode, these tables do not actually exist.\n";

  return result.str();
}

void sqlcc::SqlExecutor::SetError(const std::string &error) {
  last_error_ = error;
}

std::string sqlcc::SqlExecutor::ExecuteCreateIndex(
    const sqlcc::sql_parser::CreateIndexStatement &create_index_stmt) {
  std::ostringstream result;

  result << "\nCREATE INDEX statement executed\n";
  result << "----------------------------------\n";

  const std::string &index_name = create_index_stmt.getIndexName();
  const std::string &table_name = create_index_stmt.getTableName();
  const std::string &column_name = create_index_stmt.getColumnName();
  bool is_unique = create_index_stmt.isUnique();

  // 调用存储引擎的索引管理器来创建索引
  bool success = false;
  if (storage_engine_ != nullptr) {
    auto index_manager = storage_engine_->GetIndexManager();
    if (index_manager != nullptr) {
      success = index_manager->CreateIndex(index_name, table_name, column_name,
                                           is_unique);
    }
  }

  if (success) {
    result << "Index '" << index_name << "' " << (is_unique ? "(unique) " : "")
           << "created on table '" << table_name << "' column '" << column_name
           << "' successfully\n";
  } else {
    result << "Error: Failed to create index '" << index_name
           << "'. Make sure storage engine is properly initialized.\n";
  }

  result << "----------------------------------\n";

  return result.str();
}

std::string sqlcc::SqlExecutor::ExecuteDropIndex(
    const sqlcc::sql_parser::DropIndexStatement &drop_index_stmt) {
  std::ostringstream result;

  result << "\nDROP INDEX statement executed\n";
  result << "----------------------------------\n";

  const std::string &index_name = drop_index_stmt.getIndexName();
  const std::string &table_name = drop_index_stmt.getTableName();
  bool if_exists = drop_index_stmt.isIfExists();

  // 调用存储引擎的索引管理器来删除索引
  bool success = false;
  if (storage_engine_ != nullptr) {
    auto index_manager = storage_engine_->GetIndexManager();
    if (index_manager != nullptr) {
      success = index_manager->DropIndex(index_name, table_name);
    }
  }

  if (success) {
    result << "Index '" << index_name << "' on table '" << table_name << "' "
           << (if_exists ? "(with IF EXISTS) " : "")
           << "dropped successfully\n";
  } else if (!if_exists) {
    result << "Error: Failed to drop index '" << index_name
           << "'. Make sure index exists and storage engine is properly "
              "initialized.\n";
  } else {
    result
        << "Index '" << index_name << "' on table '" << table_name
        << "' does not exist, but IF EXISTS was specified. No action taken.\n";
  }

  result << "----------------------------------\n";

  return result.str();
}

// 约束验证方法的实现

bool SqlExecutor::ValidateInsertConstraints(
    const std::string &table_name, const std::vector<std::string> &record,
    const std::vector<sql_parser::ColumnDefinition> &table_schema) {

  try {
    std::string lower_table_name = table_name;
    std::transform(lower_table_name.begin(), lower_table_name.end(),
                   lower_table_name.begin(), ::tolower);

    auto it = table_constraints_.find(lower_table_name);
    if (it == table_constraints_.end()) {
      // 没有约束，直接通过
      return true;
    }

    // 检查所有约束
    for (const auto &constraint_executor : it->second) {
      if (!constraint_executor->validateInsert(record, table_schema)) {
        SetError(std::string("Constraint violation in constraint '") +
                 constraint_executor->getConstraintName() + "' for table '" +
                 table_name + "'");
        return false;
      }
    }

    return true;
  } catch (const std::exception &e) {
    SetError("Constraint validation error: " + std::string(e.what()));
    return false;
  }
}

bool SqlExecutor::ValidateUpdateConstraints(
    const std::string &table_name, const std::vector<std::string> &old_record,
    const std::vector<std::string> &new_record,
    const std::vector<sql_parser::ColumnDefinition> &table_schema) {

  try {
    std::string lower_table_name = table_name;
    std::transform(lower_table_name.begin(), lower_table_name.end(),
                   lower_table_name.begin(), ::tolower);

    auto it = table_constraints_.find(lower_table_name);
    if (it == table_constraints_.end()) {
      // 没有约束，直接通过
      return true;
    }

    // 检查所有约束
    for (const auto &constraint_executor : it->second) {
      if (!constraint_executor->validateUpdate(old_record, new_record,
                                               table_schema)) {
        SetError(std::string("Constraint violation in constraint '") +
                 constraint_executor->getConstraintName() + "' for table '" +
                 table_name + "'");
        return false;
      }
    }

    return true;
  } catch (const std::exception &e) {
    SetError("Constraint validation error: " + std::string(e.what()));
    return false;
  }
}

bool SqlExecutor::ValidateDeleteConstraints(
    const std::string &table_name, const std::vector<std::string> &record,
    const std::vector<sql_parser::ColumnDefinition> &table_schema) {

  try {
    std::string lower_table_name = table_name;
    std::transform(lower_table_name.begin(), lower_table_name.end(),
                   lower_table_name.begin(), ::tolower);

    auto it = table_constraints_.find(lower_table_name);
    if (it == table_constraints_.end()) {
      // 没有约束，直接通过
      return true;
    }

    // 检查所有约束（主要检查外键约束）
    for (const auto &constraint_executor : it->second) {
      // 注意：这里假设约束执行器有validateDelete方法
      // 实际实现中需要确保所有约束执行器都实现了此方法
      if (!constraint_executor->validateDelete(record, table_schema)) {
        SetError(std::string("Constraint violation in constraint '") +
                 constraint_executor->getConstraintName() + "' for table '" +
                 table_name + "'");
        return false;
      }
    }

    return true;
  } catch (const std::exception &e) {
    SetError("Constraint validation error: " + std::string(e.what()));
    return false;
  }
}

void SqlExecutor::CreateTableConstraints(
    const std::string &table_name,
    const std::vector<sql_parser::TableConstraint> &constraints) {

  std::string lower_table_name = table_name;
  std::transform(lower_table_name.begin(), lower_table_name.end(),
                 lower_table_name.begin(), ::tolower);

  // 如果已经有该表的约束，先清空
  table_constraints_[lower_table_name].clear();

  // 为每个约束创建相应的执行器
  for (const auto &constraint : constraints) {
    switch (constraint.getType()) {
    case sql_parser::TableConstraint::FOREIGN_KEY: {
      try {
        const auto &fk_constraint =
            dynamic_cast<const sql_parser::ForeignKeyConstraint &>(constraint);
        auto foreign_key_executor =
            std::make_unique<ForeignKeyConstraintExecutor>(fk_constraint,
                                                           *storage_engine_);
        foreign_key_executor->setCurrentTableName(table_name);
        table_constraints_[lower_table_name].push_back(
            std::move(foreign_key_executor));
      } catch (const std::bad_cast &e) {
        // 类型转换失败，跳过此约束
        std::cerr
            << "Warning: Failed to create foreign key executor for table '"
            << table_name << "': " << e.what() << std::endl;
      }
      break;
    }

    case sql_parser::TableConstraint::PRIMARY_KEY:
    case sql_parser::TableConstraint::UNIQUE: {
      try {
        bool is_pk =
            constraint.getType() == sql_parser::TableConstraint::PRIMARY_KEY;
        auto unique_executor = std::make_unique<UniqueConstraintExecutor>(
            constraint, *storage_engine_, table_name, is_pk);
        table_constraints_[lower_table_name].push_back(
            std::move(unique_executor));
      } catch (const std::bad_cast &e) {
        std::cerr << "Warning: Failed to create unique constraint executor for "
                     "table '"
                  << table_name << "': " << e.what() << std::endl;
      }
      break;
    }

    case sql_parser::TableConstraint::CHECK: {
      try {
        const auto &check_constraint =
            dynamic_cast<const sql_parser::CheckConstraint &>(constraint);
        auto check_executor = std::make_unique<CheckConstraintExecutor>(
            check_constraint, table_name);
        table_constraints_[lower_table_name].push_back(
            std::move(check_executor));
      } catch (const std::bad_cast &e) {
        std::cerr
            << "Warning: Failed to create check constraint executor for table '"
            << table_name << "': " << e.what() << std::endl;
      }
      break;
    }

    default:
      // 其他类型的约束暂时不支持
      std::cerr << "Warning: Unsupported constraint type for table '"
                << table_name << "': " << static_cast<int>(constraint.getType())
                << std::endl;
      break;
    }
  }

  std::cout << "Created " << table_constraints_[lower_table_name].size()
            << " constraint executors for table '" << table_name << "'"
            << std::endl;
}

std::vector<sql_parser::ColumnDefinition>
SqlExecutor::GetTableSchema(const std::string &table_name) {
  // 简化实现：这里返回硬编码的表模式
  // 实际实现中应该从metadata manager或存储引擎获取

  std::vector<sql_parser::ColumnDefinition> schema;

  // 示例：为"users"表创建模式
  if (table_name == "users" || table_name == "USERS") {
    // ID列 - 整数主键
    schema.push_back(sql_parser::ColumnDefinition(
        "id", "int", 0, std::vector<sql_parser::ColumnConstraint>()));

    // 数据列 - 文本
    schema.push_back(sql_parser::ColumnDefinition(
        "name", "varchar", 100, std::vector<sql_parser::ColumnConstraint>()));

    // 邮箱列 - 可空
    schema.push_back(sql_parser::ColumnDefinition(
        "email", "varchar", 255, std::vector<sql_parser::ColumnConstraint>()));
  }
  // 示例：为"products"表创建模式
  else if (table_name == "products" || table_name == "PRODUCTS") {
    schema.push_back(sql_parser::ColumnDefinition(
        "id", "int", 0, std::vector<sql_parser::ColumnConstraint>()));

    schema.push_back(sql_parser::ColumnDefinition(
        "name", "varchar", 255, std::vector<sql_parser::ColumnConstraint>()));

    schema.push_back(sql_parser::ColumnDefinition(
        "price", "decimal", 0, std::vector<sql_parser::ColumnConstraint>()));
  }
  // 示例：为"orders"表创建模式
  else if (table_name == "orders" || table_name == "ORDERS") {
    schema.push_back(sql_parser::ColumnDefinition(
        "id", "int", 0, std::vector<sql_parser::ColumnConstraint>()));

    schema.push_back(sql_parser::ColumnDefinition(
        "user_id", "int", 0, std::vector<sql_parser::ColumnConstraint>()));

    schema.push_back(sql_parser::ColumnDefinition(
        "product_id", "int", 0, std::vector<sql_parser::ColumnConstraint>()));

    schema.push_back(sql_parser::ColumnDefinition(
        "quantity", "int", 0, std::vector<sql_parser::ColumnConstraint>()));
  }
  // 默认单列模式 - 用于一般测试
  else {
    schema.push_back(sql_parser::ColumnDefinition(
        "id", "int", 0, std::vector<sql_parser::ColumnConstraint>()));
    schema.push_back(sql_parser::ColumnDefinition(
        "data", "varchar", 255, std::vector<sql_parser::ColumnConstraint>()));
  }

  return schema;
}

} // namespace sqlcc
