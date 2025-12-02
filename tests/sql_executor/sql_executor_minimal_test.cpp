#include "../include/sql_executor.h"
#include <iostream>
#include <string>

namespace sqlcc {

void TestSqlExecutorMinimal() {
  std::cout << "=== SQL执行器最小测试开始 ===" << std::endl;

  // 创建SqlExecutor实例
  SqlExecutor executor;

  // 测试1: 执行简单SELECT语句
  std::cout << "\n测试1: 执行简单SELECT语句" << std::endl;
  std::string result = executor.Execute("SELECT * FROM test_table");
  std::cout << "结果: " << result << std::endl;

  // 测试2: 执行简单INSERT语句
  std::cout << "\n测试2: 执行简单INSERT语句" << std::endl;
  result = executor.Execute("INSERT INTO test_table VALUES (1, 'test')");
  std::cout << "结果: " << result << std::endl;

  // 测试3: 执行简单UPDATE语句
  std::cout << "\n测试3: 执行简单UPDATE语句" << std::endl;
  result = executor.Execute("UPDATE test_table SET column1 = 'updated' WHERE id = 1");
  std::cout << "结果: " << result << std::endl;

  // 测试4: 执行简单DELETE语句
  std::cout << "\n测试4: 执行简单DELETE语句" << std::endl;
  result = executor.Execute("DELETE FROM test_table WHERE id = 1");
  std::cout << "结果: " << result << std::endl;

  // 测试5: 执行DDL语句
  std::cout << "\n测试5: 执行DDL语句" << std::endl;
  result = executor.Execute("CREATE TABLE test_ddl (id INT, name VARCHAR(255))");
  std::cout << "结果: " << result << std::endl;

  // 测试6: 执行DCL语句
  std::cout << "\n测试6: 执行DCL语句" << std::endl;
  result = executor.Execute("CREATE USER test_user IDENTIFIED BY 'password'");
  std::cout << "结果: " << result << std::endl;

  std::cout << "\n=== SQL执行器最小测试完成 ===" << std::endl;
}

} // namespace sqlcc

int main() {
  sqlcc::TestSqlExecutorMinimal();
  return 0;
}
