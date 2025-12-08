#include "database_manager.h"
#include "sql_executor.h"
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

namespace sqlcc {

void TestSqlExecutorMinimal() {
  std::cout << "=== SQL执行器最小测试开始 ===" << std::endl;

  // 创建独立的测试目录
  std::string test_dir = "./sql_executor_minimal_test";
  if (fs::exists(test_dir)) {
    fs::remove_all(test_dir);
  }
  fs::create_directories(test_dir);

  // 创建DatabaseManager实例，使用独立的测试目录
  auto db_manager = std::make_shared<DatabaseManager>(test_dir);

  // 创建SqlExecutor实例，传入DatabaseManager
  SqlExecutor executor(db_manager);

  // 先创建测试表
  std::cout << "\n准备阶段: 创建测试表" << std::endl;
  std::string create_result = executor.Execute(
      "CREATE TABLE test_table (id INT, column1 VARCHAR(255))");
  std::cout << "创建表结果: " << create_result << std::endl;

  // 检查是否有错误
  std::string error = executor.GetLastError();
  if (!error.empty()) {
    std::cout << "错误信息: " << error << std::endl;
  }

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
  result = executor.Execute(
      "UPDATE test_table SET column1 = 'updated' WHERE id = 1");
  std::cout << "结果: " << result << std::endl;

  // 测试4: 执行简单DELETE语句
  std::cout << "\n测试4: 执行简单DELETE语句" << std::endl;
  result = executor.Execute("DELETE FROM test_table WHERE id = 1");
  std::cout << "结果: " << result << std::endl;

  // 测试5: 执行DDL语句
  std::cout << "\n测试5: 执行DDL语句" << std::endl;
  result =
      executor.Execute("CREATE TABLE test_ddl (id INT, name VARCHAR(255))");
  std::cout << "结果: " << result << std::endl;

  // 测试6: 执行DCL语句
  std::cout << "\n测试6: 执行DCL语句" << std::endl;
  result = executor.Execute("CREATE USER test_user IDENTIFIED BY 'password'");
  std::cout << "结果: " << result << std::endl;

  std::cout << "\n=== SQL执行器最小测试完成 ===" << std::endl;

  // 清理测试目录
  if (fs::exists(test_dir)) {
    fs::remove_all(test_dir);
  }
}

} // namespace sqlcc

int main() {
  sqlcc::TestSqlExecutorMinimal();
  return 0;
}
