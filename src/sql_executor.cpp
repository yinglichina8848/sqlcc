#include "sql_executor.h"
#include "database_manager.h"
#include "config_manager.h"
#include <algorithm>
#include <cctype>
#include <string>
#include <iostream>

namespace sqlcc {

// 构造函数实现
SqlExecutor::SqlExecutor()
    : last_error_(""),
      user_manager_(std::make_shared<UserManager>()),
      parser_(nullptr),
      ddl_executor_(nullptr),
      dml_executor_(nullptr),
      query_executor_(nullptr),
      dcl_executor_(nullptr) {

    try {
        // 初始化配置管理器
        auto config_manager = std::make_shared<ConfigManager>();

        // 初始化数据库管理器
        db_manager_ = std::make_shared<DatabaseManager>("./test.db", 1024, 4, 2);

        // 初始化执行器
        ddl_executor_ = std::make_unique<DDLExecutor>(db_manager_);
        dml_executor_ = std::make_unique<DMLExecutor>(db_manager_);
        query_executor_ = std::make_unique<QueryExecutor>(db_manager_);
        dcl_executor_ = std::make_unique<DCLExecutor>(db_manager_);

    } catch (const std::exception& e) {
        last_error_ = std::string("Failed to initialize SqlExecutor: ") + e.what();
        std::cerr << "SqlExecutor initialization error: " << e.what() << std::endl;
    }
}

// 析构函数实现
SqlExecutor::~SqlExecutor() {
    // 智能指针会自动释放资源
}

// 执行SQL语句
std::string SqlExecutor::Execute(const std::string &sql) {
  std::string trimmed_sql = sql;
  TrimString(trimmed_sql);

  if (trimmed_sql.empty()) {
    SetError("Empty SQL statement");
    return "错误：空的SQL语句";
  }

  try {
    // 创建新的Parser实例来解析SQL
    sqlcc::sql_parser::Parser temp_parser(trimmed_sql);
    auto statements = temp_parser.parseStatements();

    if (statements.empty()) {
      SetError("Failed to parse SQL statement");
      return "错误：SQL语句解析失败";
    }

    // 执行第一条语句（简化实现）
    auto& stmt_ptr = statements[0];
    if (!stmt_ptr) {
        SetError("Parsed statement is null");
        return "错误：解析出的SQL语句为空";
    }
    auto& stmt = *stmt_ptr;

    // 根据语句类型分发到对应的执行器
    ExecutionResult result;
    switch (stmt.getType()) {
      case sqlcc::sql_parser::Statement::SELECT:
        result = query_executor_->execute(std::move(stmt));
        break;
      case sqlcc::sql_parser::Statement::INSERT:
      case sqlcc::sql_parser::Statement::UPDATE:
      case sqlcc::sql_parser::Statement::DELETE:
        result = dml_executor_->execute(std::move(stmt));
        break;
      case sqlcc::sql_parser::Statement::CREATE:
      case sqlcc::sql_parser::Statement::DROP:
      case sqlcc::sql_parser::Statement::ALTER:
      case sqlcc::sql_parser::Statement::CREATE_INDEX:
      case sqlcc::sql_parser::Statement::DROP_INDEX:
        result = ddl_executor_->execute(std::move(stmt));
        break;
      case sqlcc::sql_parser::Statement::CREATE_USER:
      case sqlcc::sql_parser::Statement::DROP_USER:
      case sqlcc::sql_parser::Statement::GRANT:
      case sqlcc::sql_parser::Statement::REVOKE:
        result = dcl_executor_->execute(std::move(stmt));
        break;
      default:
        SetError("Unsupported SQL statement type");
        return "错误：不支持的SQL语句类型";
    }

    // 返回执行结果
    if (result.getStatus() == ExecutionResult::SUCCESS) {
      return result.getMessage();
    } else {
      SetError(result.getMessage());
      return "错误：" + result.getMessage();
    }
  } catch (const std::exception& e) {
    SetError(e.what());
    return "错误：" + std::string(e.what());
  }
}

// 获取最后一次执行的错误信息
std::string SqlExecutor::GetLastError() const {
    return last_error_;
}

// 私有辅助函数实现
void SqlExecutor::SetError(const std::string &error) { last_error_ = error; }

void SqlExecutor::TrimString(std::string &str) {
  str.erase(str.begin(),
            std::find_if(str.begin(), str.end(),
                         [](unsigned char ch) { return !std::isspace(ch); }));
  str.erase(std::find_if(str.rbegin(), str.rend(),
                         [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
            str.end());
}

} // namespace sqlcc