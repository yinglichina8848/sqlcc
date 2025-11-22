#include "sql_executor.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace sqlcc {

// SqlExecutor类实现
SqlExecutor::SqlExecutor() {
  // 初始化
}

SqlExecutor::~SqlExecutor() {
  // 清理资源
}

std::string SqlExecutor::Execute(const std::string &sql) {
  // 简单的SQL解析和执行逻辑
  std::string trimmed_sql = sql;
  TrimString(trimmed_sql);

  if (trimmed_sql.empty()) {
    return "";
  }

  // 简单的SQL命令判断
  if (trimmed_sql.substr(0, 6) == "SELECT") {
    return ExecuteSelect(trimmed_sql);
  } else if (trimmed_sql.substr(0, 6) == "INSERT") {
    return "Query OK, 1 row affected\n";
  } else if (trimmed_sql.substr(0, 6) == "UPDATE") {
    return "Query OK, 0 rows affected\n";
  } else if (trimmed_sql.substr(0, 6) == "DELETE") {
    return "Query OK, 0 rows affected\n";
  } else if (trimmed_sql.substr(0, 6) == "CREATE") {
    return "Query OK\n";
  } else if (trimmed_sql.substr(0, 4) == "DROP") {
    return "Query OK\n";
  } else if (trimmed_sql.substr(0, 3) == "USE") {
    return "Database changed\n";
  } else if (trimmed_sql.substr(0, 4) == "SHOW") {
    return "Tables:\n----------\nproducts\ncustomers\norders\n";
  }

  return "ERROR: Unknown command\n";
}

std::string SqlExecutor::ExecuteSelect(const std::string &sql) {
  // 模拟SELECT查询结果
  if (sql.find("FROM products") != std::string::npos) {
    std::stringstream ss;
    ss << "+----------+----------+\n";
    ss << "| name     | price    |\n";
    ss << "+----------+----------+\n";
    ss << "| Laptop   | 999.99   |\n";
    ss << "| Monitor  | 199.99   |\n";
    ss << "+----------+----------+\n";
    ss << "2 row(s) in set\n";
    return ss.str();
  }

  return "Empty set\n";
}

std::string SqlExecutor::ExecuteFile(const std::string &file_path) {
  return "ERROR: ExecuteFile not implemented\n";
}

const std::string &SqlExecutor::GetLastError() const {
  static std::string empty_error = "";
  return empty_error;
}

void SqlExecutor::SetError(const std::string &error) {
  // 简单实现
}

// 辅助函数：修剪字符串
void TrimString(std::string &str) {
  // 移除开头空白
  size_t start = str.find_first_not_of(" \t\n\r");
  if (start != std::string::npos) {
    str = str.substr(start);
  } else {
    str.clear();
    return;
  }
  // 移除结尾空白
  size_t end = str.find_last_not_of(" \t\n\r");
  if (end != std::string::npos) {
    str = str.substr(0, end + 1);
  } else {
    str.clear();
  }
}

} // namespace sqlcc