#include "sql_executor.h"
#include <iostream>
#include <string>

namespace sqlcc {

void TestSqlExecutorUnit() {
  std::cout << "=== SQL执行器单元测试开始 ===" << std::endl;

  // 创建SqlExecutor实例
  SqlExecutor executor;

  // 测试1: 字符串处理功能
  std::cout << "\n测试1: 字符串处理功能" << std::endl;
  std::string test_str1 = "  hello world  ";
  std::string test_str2 = "\t\n  trimmed string \r\n";
  std::string test_str3 = "   ";
  
  std::cout << "原始字符串1: '" << test_str1 << "'" << std::endl;
  std::cout << "原始字符串2: '" << test_str2 << "'" << std::endl;
  std::cout << "原始字符串3: '" << test_str3 << "'" << std::endl;

  // 测试2: 错误处理功能
  std::cout << "\n测试2: 错误处理功能" << std::endl;
  std::string last_error = executor.GetLastError();
  std::cout << "初始错误信息: " << last_error << std::endl;

  // 测试3: 执行空SQL语句
  std::cout << "\n测试3: 执行空SQL语句" << std::endl;
  std::string result = executor.Execute("");
  std::cout << "空语句执行结果: " << result << std::endl;
  last_error = executor.GetLastError();
  std::cout << "错误信息: " << last_error << std::endl;

  // 测试4: 执行无效SQL语句
  std::cout << "\n测试4: 执行无效SQL语句" << std::endl;
  result = executor.Execute("INVALID SQL STATEMENT");
  std::cout << "无效语句执行结果: " << result << std::endl;
  last_error = executor.GetLastError();
  std::cout << "错误信息: " << last_error << std::endl;

  // 测试5: 执行各种类型的SQL语句
  std::cout << "\n测试5: 执行各种类型的SQL语句" << std::endl;
  
  // SELECT语句
  std::cout << "\n  5.1: SELECT语句" << std::endl;
  result = executor.Execute("SELECT * FROM users");
  std::cout << "结果: " << result << std::endl;

  // INSERT语句
  std::cout << "\n  5.2: INSERT语句" << std::endl;
  result = executor.Execute("INSERT INTO users (id, name) VALUES (1, 'Alice')");
  std::cout << "结果: " << result << std::endl;

  // UPDATE语句
  std::cout << "\n  5.3: UPDATE语句" << std::endl;
  result = executor.Execute("UPDATE users SET name = 'Bob' WHERE id = 1");
  std::cout << "结果: " << result << std::endl;

  // DELETE语句
  std::cout << "\n  5.4: DELETE语句" << std::endl;
  result = executor.Execute("DELETE FROM users WHERE id = 1");
  std::cout << "结果: " << result << std::endl;

  // CREATE语句
  std::cout << "\n  5.5: CREATE语句" << std::endl;
  result = executor.Execute("CREATE TABLE products (id INT, name VARCHAR(100))");
  std::cout << "结果: " << result << std::endl;

  // DROP语句
  std::cout << "\n  5.6: DROP语句" << std::endl;
  result = executor.Execute("DROP TABLE products");
  std::cout << "结果: " << result << std::endl;

  // DCL语句
  std::cout << "\n  5.7: DCL语句" << std::endl;
  result = executor.Execute("CREATE USER test_user IDENTIFIED BY 'password'");
  std::cout << "结果: " << result << std::endl;

  std::cout << "\n=== SQL执行器单元测试完成 ===" << std::endl;
}

} // namespace sqlcc

int main() {
  sqlcc::TestSqlExecutorUnit();
  return 0;
}
