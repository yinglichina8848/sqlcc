#include "../include/sql_executor.h"
#include <iostream>
#include <string>

using namespace sqlcc;

// 最小化测试，直接针对sql_executor.cpp中的核心函数
void TestSqlExecutorMinimal() {
    std::cout << "=== 开始SQL执行器最小化测试 ===\n";
    
    // 测试1: 直接测试TrimString函数
    std::cout << "\n=== 测试1: TrimString函数 ===" << std::endl;
    std::string test_str = "  SELECT * FROM users  ";
    TrimString(test_str);
    std::cout << "修剪后: \"" << test_str << "\"" << std::endl;
    
    // 测试2: 直接测试Execute方法的各种分支
    std::cout << "\n=== 测试2: Execute方法核心分支 ===" << std::endl;
    SqlExecutor executor;
    
    // 确保测试覆盖Execute方法中的每个if-else分支
    std::string result1 = executor.Execute("SELECT * FROM users");
    std::cout << "SELECT命令: " << (result1.find("id") != std::string::npos ? "成功" : "失败") << std::endl;
    
    std::string result2 = executor.Execute("INSERT INTO users VALUES (1, 'test')");
    std::cout << "INSERT命令: " << (result2.find("Query OK") != std::string::npos ? "成功" : "失败") << std::endl;
    
    std::string result3 = executor.Execute("UPDATE users SET name='updated' WHERE id=1");
    std::cout << "UPDATE命令: " << (result3.find("Query OK") != std::string::npos ? "成功" : "失败") << std::endl;
    
    std::string result4 = executor.Execute("DELETE FROM users WHERE id=1");
    std::cout << "DELETE命令: " << (result4.find("Query OK") != std::string::npos ? "成功" : "失败") << std::endl;
    
    std::string result5 = executor.Execute("CREATE TABLE test_minimal (id INT)");
    std::cout << "CREATE TABLE命令: " << (result5.find("Query OK") != std::string::npos ? "成功" : "失败") << std::endl;
    
    std::string result6 = executor.Execute("DROP TABLE test_minimal");
    std::cout << "DROP TABLE命令: " << (result6.find("Query OK") != std::string::npos ? "成功" : "失败") << std::endl;
    
    std::string result7 = executor.Execute("SHOW TABLES");
    std::cout << "SHOW TABLES命令: " << (result7.find("users") != std::string::npos ? "成功" : "失败") << std::endl;
    
    std::string result8 = executor.Execute("USE test_db");
    std::cout << "USE命令: " << (result8.find("Database changed") != std::string::npos ? "成功" : "失败") << std::endl;
    
    std::string result9 = executor.Execute("UNKNOWN COMMAND");
    std::cout << "未知命令: " << (result9.find("Unknown command") != std::string::npos ? "成功" : "失败") << std::endl;
    
    // 测试3: 错误处理函数
    std::cout << "\n=== 测试3: 错误处理函数 ===" << std::endl;
    executor.SetError("Test error message");
    std::string error = executor.GetLastError();
    std::cout << "设置和获取错误: " << (error == "Test error message" ? "成功" : "失败") << std::endl;
    
    std::cout << "\n=== SQL执行器最小化测试完成 ===\n";
}

int main() {
    TestSqlExecutorMinimal();
    return 0;
}
