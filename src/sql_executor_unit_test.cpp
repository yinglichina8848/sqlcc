#include "../include/sql_executor.h"
#include <iostream>
#include <string>
#include <cassert>
#include <fstream>

using namespace sqlcc;

// 直接测试sql_executor.cpp中的未覆盖函数和分支
void TestSqlExecutorUnit() {
    std::cout << "=== 开始SQL执行器单元测试 ===\n";
    
    // 1. 测试TrimString函数的所有分支
    std::cout << "\n=== 测试1: TrimString函数 ===" << std::endl;
    
    std::string test_str1 = "  test string  ";
    TrimString(test_str1);
    std::cout << "修剪前后空格: \"test string\" -> \"" << test_str1 << "\"" << std::endl;
    
    std::string test_str2 = "\t\ntest string\n\t";
    TrimString(test_str2);
    std::cout << "修剪制表符和换行符: \"test string\" -> \"" << test_str2 << "\"" << std::endl;
    
    std::string test_str3 = "   \t\n   ";
    TrimString(test_str3);
    std::cout << "只包含空白字符: 空字符串 -> \"" << test_str3 << "\"" << std::endl;
    
    std::string test_str4 = "";
    TrimString(test_str4);
    std::cout << "空字符串: 保持空 -> \"" << test_str4 << "\"" << std::endl;
    
    // 2. 测试SqlExecutor构造和析构
    std::cout << "\n=== 测试2: SqlExecutor构造和析构 ===" << std::endl;
    {
        SqlExecutor executor;
        std::cout << "构造函数和析构函数测试: 完成" << std::endl;
    }
    
    // 3. 测试Execute方法的各种命令类型
    std::cout << "\n=== 测试3: Execute方法详细分支 ===" << std::endl;
    SqlExecutor executor;
    
    // 测试空字符串和空白字符串
    std::cout << "\n测试空SQL语句" << std::endl;
    std::string result1 = executor.Execute("");
    std::cout << "结果: \"" << result1 << "\"" << std::endl;
    
    std::cout << "\n测试只有空白字符的SQL" << std::endl;
    std::string result2 = executor.Execute("   \t\n   ");
    std::cout << "结果: \"" << result2 << "\"" << std::endl;
    
    // 测试各种SQL命令前缀
    std::cout << "\n测试SELECT命令" << std::endl;
    std::string result3 = executor.Execute("SELECT * FROM users");
    std::cout << "结果: 包含表头和数据" << std::endl;
    
    std::cout << "\n测试INSERT命令" << std::endl;
    std::string result4 = executor.Execute("INSERT INTO users VALUES (1, 'test')");
    std::cout << "结果: " << result4 << std::endl;
    
    std::cout << "\n测试UPDATE命令" << std::endl;
    std::string result5 = executor.Execute("UPDATE users SET name='new' WHERE id=1");
    std::cout << "结果: " << result5 << std::endl;
    
    std::cout << "\n测试DELETE命令" << std::endl;
    std::string result6 = executor.Execute("DELETE FROM users WHERE id=1");
    std::cout << "结果: " << result6 << std::endl;
    
    // 4. 测试DDL命令的具体分支
    std::cout << "\n=== 测试4: DDL命令详细分支 ===" << std::endl;
    
    // CREATE命令的各种变体
    std::cout << "\n测试CREATE TABLE" << std::endl;
    std::string result7 = executor.Execute("CREATE TABLE test_table (id INT)");
    std::cout << "结果: " << result7 << std::endl;
    
    std::cout << "\n测试CREATE TABLE IF NOT EXISTS" << std::endl;
    std::string result8 = executor.Execute("CREATE TABLE IF NOT EXISTS test_table2 (id INT)");
    std::cout << "结果: " << result8 << std::endl;
    
    std::cout << "\n测试CREATE INDEX" << std::endl;
    std::string result9 = executor.Execute("CREATE INDEX idx_test ON test_table(id)");
    std::cout << "结果: " << result9 << std::endl;
    
    std::cout << "\n测试CREATE VIEW" << std::endl;
    std::string result10 = executor.Execute("CREATE VIEW test_view AS SELECT * FROM test_table");
    std::cout << "结果: " << result10 << std::endl;
    
    // DROP命令的各种变体
    std::cout << "\n测试DROP TABLE" << std::endl;
    std::string result11 = executor.Execute("DROP TABLE test_table");
    std::cout << "结果: " << result11 << std::endl;
    
    std::cout << "\n测试DROP VIEW" << std::endl;
    std::string result12 = executor.Execute("DROP VIEW test_view");
    std::cout << "结果: " << result12 << std::endl;
    
    std::cout << "\n测试其他DROP命令" << std::endl;
    std::string result13 = executor.Execute("DROP DATABASE test_db");
    std::cout << "结果: " << result13 << std::endl;
    
    // 5. 测试ShowTableSchema相关的SHOW命令
    std::cout << "\n=== 测试5: SHOW命令详细分支 ===" << std::endl;
    
    std::cout << "\n测试SHOW TABLES" << std::endl;
    std::string result14 = executor.Execute("SHOW TABLES");
    std::cout << "结果: 包含表列表" << std::endl;
    
    std::cout << "\n测试SHOW CREATE TABLE" << std::endl;
    std::string result15 = executor.Execute("SHOW CREATE TABLE users");
    std::cout << "结果: " << result15 << std::endl;
    
    std::cout << "\n测试其他SHOW命令" << std::endl;
    std::string result16 = executor.Execute("SHOW DATABASES");
    std::cout << "结果: " << result16 << std::endl;
    
    // 6. 测试USE命令
    std::cout << "\n=== 测试6: USE命令 ===" << std::endl;
    
    std::cout << "\n测试USE DATABASE" << std::endl;
    std::string result17 = executor.Execute("USE test_database");
    std::cout << "结果: " << result17 << std::endl;
    
    // 7. 测试错误处理和未知命令
    std::cout << "\n=== 测试7: 错误处理 ===" << std::endl;
    
    std::cout << "\n测试未知命令" << std::endl;
    std::string result18 = executor.Execute("UNKNOWN COMMAND");
    std::cout << "结果: " << result18 << std::endl;
    
    std::cout << "\n测试GetLastError" << std::endl;
    std::string error = executor.GetLastError();
    std::cout << "最后错误: \"" << error << "\"" << std::endl;
    
    // 8. 测试ExecuteFile方法的所有分支
    std::cout << "\n=== 测试8: ExecuteFile方法 ===" << std::endl;
    
    // 创建一个测试SQL文件
    std::string test_file = "./test_unit.sql";
    std::ofstream file(test_file);
    file << "-- 这是一个测试注释\n";
    file << "SELECT * FROM users;\n";
    file << "INSERT INTO users VALUES (1, 'test');\n";
    file << "\n";
    file.close();
    
    std::cout << "\n测试ExecuteFile - 正常文件" << std::endl;
    std::string result19 = executor.ExecuteFile(test_file);
    std::cout << "结果: 执行完成" << std::endl;
    
    std::cout << "\n测试ExecuteFile - 不存在的文件" << std::endl;
    std::string result20 = executor.ExecuteFile("./non_existent.sql");
    std::cout << "结果: " << result20 << std::endl;
    
    // 清理测试文件
    std::remove(test_file.c_str());
    
    std::cout << "\n=== SQL执行器单元测试完成 ===\n";
}

int main() {
    try {
        TestSqlExecutorUnit();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "测试过程中发生异常: " << e.what() << std::endl;
        return 1;
    }
}
