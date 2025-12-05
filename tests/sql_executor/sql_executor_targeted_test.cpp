#include "sql_executor.h"
#include <iostream>
#include <string>
#include <cassert>
#include <fstream>

using namespace sqlcc;

// 专门测试sql_executor.cpp中未覆盖的代码部分
void TestSqlExecutorUncoveredParts() {
    std::cout << "=== 开始测试sql_executor.cpp未覆盖的代码部分 ===\n";
    SqlExecutor executor;
    
    // 1. 测试用户管理相关功能的详细逻辑
    std::cout << "\n=== 测试1: 用户管理详细功能 ===" << std::endl;
    
    // 测试CREATE USER命令的不同格式
    std::cout << "\n测试CREATE USER命令 - 带密码和角色" << std::endl;
    std::string result1 = executor.Execute("CREATE USER test_user1 IDENTIFIED BY 'password123' ROLE ADMIN;");
    std::cout << "结果: " << result1 << std::endl;
    
    std::cout << "\n测试CREATE USER命令 - 只有密码" << std::endl;
    std::string result2 = executor.Execute("CREATE USER test_user2 IDENTIFIED BY 'password456';");
    std::cout << "结果: " << result2 << std::endl;
    
    // 测试GRANT命令的详细逻辑
    std::cout << "\n测试GRANT命令 - 授予SELECT权限" << std::endl;
    std::string result3 = executor.Execute("GRANT SELECT ON products TO test_user1;");
    std::cout << "结果: " << result3 << std::endl;
    
    std::cout << "\n测试GRANT命令 - 错误格式" << std::endl;
    std::string result4 = executor.Execute("GRANT INVALID ON products TO test_user1;");
    std::cout << "结果: " << result4 << std::endl;
    
    // 测试REVOKE命令的详细逻辑
    std::cout << "\n测试REVOKE命令 - 撤销权限" << std::endl;
    std::string result5 = executor.Execute("REVOKE SELECT ON products FROM test_user1;");
    std::cout << "结果: " << result5 << std::endl;
    
    std::cout << "\n测试REVOKE命令 - 错误格式" << std::endl;
    std::string result6 = executor.Execute("REVOKE INVALID ON products FROM test_user1;");
    std::cout << "结果: " << result6 << std::endl;
    
    // 测试DROP USER的IF EXISTS选项
    std::cout << "\n测试DROP USER命令 - 带IF EXISTS" << std::endl;
    std::string result7 = executor.Execute("DROP USER IF EXISTS non_existent_user;");
    std::cout << "结果: " << result7 << std::endl;
    
    // 2. 测试DDL命令的详细逻辑
    std::cout << "\n=== 测试2: DDL命令详细功能 ===" << std::endl;
    
    // 测试CREATE TABLE的不同选项
    std::cout << "\n测试CREATE TABLE - 标准格式" << std::endl;
    std::string result8 = executor.Execute("CREATE TABLE test_table (id INT, name VARCHAR(100));");
    std::cout << "结果: " << result8 << std::endl;
    
    std::cout << "\n测试CREATE TABLE - 带IF NOT EXISTS" << std::endl;
    std::string result9 = executor.Execute("CREATE TABLE IF NOT EXISTS test_table2 (id INT, name VARCHAR(100));");
    std::cout << "结果: " << result9 << std::endl;
    
    std::cout << "\n测试CREATE TABLE - 语法错误（缺少括号）" << std::endl;
    std::string result10 = executor.Execute("CREATE TABLE invalid_table id INT, name VARCHAR(100);");
    std::cout << "结果: " << result10 << std::endl;
    
    // 测试ALTER TABLE的不同动作
    std::cout << "\n测试ALTER TABLE - ADD列" << std::endl;
    std::string result11 = executor.Execute("ALTER TABLE test_table ADD COLUMN age INT;");
    std::cout << "结果: " << result11 << std::endl;
    
    std::cout << "\n测试ALTER TABLE - MODIFY列" << std::endl;
    std::string result12 = executor.Execute("ALTER TABLE test_table MODIFY COLUMN name VARCHAR(200);");
    std::cout << "结果: " << result12 << std::endl;
    
    std::cout << "\n测试ALTER TABLE - DROP列" << std::endl;
    std::string result13 = executor.Execute("ALTER TABLE test_table DROP COLUMN age;");
    std::cout << "结果: " << result13 << std::endl;
    
    std::cout << "\n测试ALTER TABLE - RENAME" << std::endl;
    std::string result14 = executor.Execute("ALTER TABLE test_table RENAME TO test_table_renamed;");
    std::cout << "结果: " << result14 << std::endl;
    
    std::cout << "\n测试ALTER TABLE - 无效语法" << std::endl;
    std::string result15 = executor.Execute("ALTER TABLE test_table INVALID ACTION;");
    std::cout << "结果: " << result15 << std::endl;
    
    // 3. 测试SHOW命令的不同变体
    std::cout << "\n=== 测试3: SHOW命令变体 ===" << std::endl;
    
    std::cout << "\n测试SHOW TABLES" << std::endl;
    std::string result16 = executor.Execute("SHOW TABLES;");
    std::cout << "结果: " << result16 << std::endl;
    
    std::cout << "\n测试SHOW CREATE TABLE" << std::endl;
    std::string result17 = executor.Execute("SHOW CREATE TABLE test_table;");
    std::cout << "结果: " << result17 << std::endl;
    
    std::cout << "\n测试SHOW DATABASES" << std::endl;
    std::string result18 = executor.Execute("SHOW DATABASES;");
    std::cout << "结果: " << result18 << std::endl;
    
    // 4. 测试错误处理路径
    std::cout << "\n=== 测试4: 错误处理路径 ===" << std::endl;
    
    // 测试无效命令
    std::cout << "\n测试无效命令" << std::endl;
    std::string result19 = executor.Execute("INVALID SQL COMMAND;");
    std::cout << "结果: " << result19 << std::endl;
    
    // 测试获取最后错误
    std::cout << "\n测试GetLastError方法" << std::endl;
    std::string last_error = executor.GetLastError();
    std::cout << "最后错误: " << last_error << std::endl;
    
    // 5. 测试ExecuteFile方法的错误处理
    std::cout << "\n=== 测试5: ExecuteFile错误处理 ===" << std::endl;
    
    std::cout << "\n测试ExecuteFile - 不存在的文件" << std::endl;
    std::string result20 = executor.ExecuteFile("non_existent_file.sql");
    std::cout << "结果: " << result20 << std::endl;
    
    // 清理创建的用户
    std::cout << "\n清理测试用户" << std::endl;
    executor.Execute("DROP USER IF EXISTS test_user1;");
    executor.Execute("DROP USER IF EXISTS test_user2;");
    
    std::cout << "\n=== sql_executor.cpp未覆盖代码部分测试完成 ===\n";
}

int main() {
    try {
        TestSqlExecutorUncoveredParts();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "测试过程中发生异常: " << e.what() << std::endl;
        return 1;
    }
}
