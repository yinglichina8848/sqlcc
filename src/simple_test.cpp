#include "../include/sql_executor.h"
#include <iostream>
#include <sstream>
#include <string>

int main() {
  sqlcc::SqlExecutor executor;
  std::string line;
  std::string current_sql;

  std::cout << "SQLCC Simple Executor - Reading commands from stdin\n";
  std::cout << "---------------------------------------------\n";

  // 从标准输入读取SQL命令
  while (std::getline(std::cin, line)) {
    // 跳过注释行和空行
    if (line.empty() || line.substr(0, 2) == "--") {
      continue;
    }

    // 将当前行添加到SQL语句
    current_sql += line;

    // 检查是否是完整的SQL语句（以分号结尾）
    size_t semicolon_pos = current_sql.find_last_of(';');
    if (semicolon_pos != std::string::npos) {
      // 提取完整的SQL语句
      std::string sql_statement = current_sql.substr(0, semicolon_pos + 1);
      // 移除末尾的分号和空白字符
      size_t end_pos = sql_statement.find_last_not_of("; \t\r\n");
      if (end_pos != std::string::npos) {
        sql_statement = sql_statement.substr(0, end_pos + 1);
      }

      // 执行SQL语句
      std::cout << "Executing: " << sql_statement << "\n";
      std::string result = executor.Execute(sql_statement);
      std::cout << result << "\n\n";

      // 清空当前SQL，保留剩余部分（如果有）
      current_sql = current_sql.substr(semicolon_pos + 1);
    }
  }

  // 执行最后一条没有分号结尾的SQL语句（如果有）
  current_sql.erase(0, current_sql.find_first_not_of(" \t\r\n"));
  if (!current_sql.empty()) {
    std::cout << "Executing: " << current_sql << "\n";
    std::string result = executor.Execute(current_sql);
    std::cout << result << "\n\n";
  }

  std::cout << "---------------------------------------------\n";
  std::cout << "Execution completed\n";
  return 0;
}