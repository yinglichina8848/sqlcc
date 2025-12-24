#include "sql_executor.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

using namespace sqlcc;

// 测试SQL执行器的全面功能
void TestSqlExecutorComprehensive() {
    std::cout << "=== SQL执行器全面测试开始 ===" << std::endl;
    
    // 创建临时数据目录
    std::string data_dir = "./sql_executor_test_data";
    std::filesystem::create_directory(data_dir);
    std::cout << "创建临时数据目录: " << data_dir << std::endl;
    
    // 创建SqlExecutor实例
    SqlExecutor executor;
    
    // 1. 测试基本SQL命令
    std::cout << "\n=== 测试1: 基本SQL命令 ===" << std::endl;
    
    // 测试SELECT命令（即使是空实现，也可以调用以覆盖代码路径）
    std::cout << "\n测试SELECT命令" << std::endl;
    std::string result = executor.Execute("SELECT * FROM test_table");
    std::cout << "结果: " << result << std::endl;
    
    // 测试INSERT命令
    std::cout << "\n测试INSERT命令" << std::endl;
    result = executor.Execute("INSERT INTO test_table VALUES (1, 'test')");
    std::cout << "结果: " << result << std::endl;
    
    // 测试UPDATE命令
    std::cout << "\n测试UPDATE命令" << std::endl;
    result = executor.Execute("UPDATE test_table SET column1 = 'updated' WHERE id = 1");
    std::cout << "结果: " << result << std::endl;
    
    // 测试DELETE命令
    std::cout << "\n测试DELETE命令" << std::endl;
    result = executor.Execute("DELETE FROM test_table WHERE id = 1");
    std::cout << "结果: " << result << std::endl;
    
    // 2. 测试DDL命令（除了之前DCL测试中的部分）
    std::cout << "\n=== 测试2: DDL命令 ===" << std::endl;
    
    // 测试CREATE TABLE
    std::cout << "\n测试CREATE TABLE" << std::endl;
    result = executor.Execute("CREATE TABLE test_create_table (id INT, name VARCHAR(255))");
    std::cout << "结果: " << result << std::endl;
    
    // 测试ALTER TABLE
    std::cout << "\n测试ALTER TABLE" << std::endl;
    result = executor.Execute("ALTER TABLE test_create_table ADD COLUMN age INT");
    std::cout << "结果: " << result << std::endl;
    
    // 测试DROP TABLE
    std::cout << "\n测试DROP TABLE" << std::endl;
    result = executor.Execute("DROP TABLE test_create_table");
    std::cout << "结果: " << result << std::endl;
    
    // 测试CREATE INDEX
    std::cout << "\n测试CREATE INDEX" << std::endl;
    result = executor.Execute("CREATE INDEX idx_test ON test_table (column1)");
    std::cout << "结果: " << result << std::endl;
    
    // 测试DROP INDEX
    std::cout << "\n测试DROP INDEX" << std::endl;
    result = executor.Execute("DROP INDEX idx_test");
    std::cout << "结果: " << result << std::endl;
    
    // 3. 测试SHOW和USE命令
    std::cout << "\n=== 测试3: SHOW和USE命令 ===" << std::endl;
    
    // 测试SHOW DATABASES
    std::cout << "\n测试SHOW DATABASES" << std::endl;
    result = executor.Execute("SHOW DATABASES");
    std::cout << "结果: " << result << std::endl;
    
    // 测试SHOW TABLES
    std::cout << "\n测试SHOW TABLES" << std::endl;
    result = executor.Execute("SHOW TABLES");
    std::cout << "结果: " << result << std::endl;
    
    // 测试USE DATABASE
    std::cout << "\n测试USE DATABASE" << std::endl;
    result = executor.Execute("USE test_database");
    std::cout << "结果: " << result << std::endl;
    
    // 4. 测试事务相关命令
    std::cout << "\n=== 测试4: 事务相关命令 ===" << std::endl;
    
    // 测试BEGIN TRANSACTION
    std::cout << "\n测试BEGIN TRANSACTION" << std::endl;
    result = executor.Execute("BEGIN TRANSACTION");
    std::cout << "结果: " << result << std::endl;
    
    // 测试COMMIT
    std::cout << "\n测试COMMIT" << std::endl;
    result = executor.Execute("COMMIT");
    std::cout << "结果: " << result << std::endl;
    
    // 测试ROLLBACK
    std::cout << "\n测试ROLLBACK" << std::endl;
    result = executor.Execute("ROLLBACK");
    std::cout << "结果: " << result << std::endl;
    
    // 5. 测试边界情况和错误处理
    std::cout << "\n=== 测试5: 边界情况和错误处理 ===" << std::endl;
    
    // 测试空SQL语句
    std::cout << "\n测试空SQL语句" << std::endl;
    result = executor.Execute("");
    std::cout << "结果: " << result << std::endl;
    
    // 测试只有空白字符的SQL
    std::cout << "\n测试只有空白字符的SQL" << std::endl;
    result = executor.Execute("   \t  \n  ");
    std::cout << "结果: " << result << std::endl;
    
    // 测试未知命令
    std::cout << "\n测试未知命令" << std::endl;
    result = executor.Execute("UNKNOWN_COMMAND test");
    std::cout << "结果: " << result << std::endl;
    
    // 6. 测试DCL命令详细功能
    std::cout << "\n=== 测试6: DCL命令详细功能 ===" << std::endl;
    
    // 测试CREATE USER带引号的用户名和密码
    std::cout << "\n测试CREATE USER带引号" << std::endl;
    result = executor.Execute("CREATE USER \"quoted_user\" IDENTIFIED BY \"password\";");
    std::cout << "结果: " << result << std::endl;
    
    // 测试CREATE USER带角色
    std::cout << "\n测试CREATE USER带角色" << std::endl;
    result = executor.Execute("CREATE USER admin_user IDENTIFIED BY 'admin123' ROLE ADMIN;");
    std::cout << "结果: " << result << std::endl;
    
    // 测试GRANT语句不同格式
    std::cout << "\n测试GRANT语句" << std::endl;
    result = executor.Execute("GRANT SELECT, INSERT ON test_table TO quoted_user;");
    std::cout << "结果: " << result << std::endl;
    
    // 测试GRANT语句错误格式
    std::cout << "\n测试GRANT语句错误格式" << std::endl;
    result = executor.Execute("GRANT INVALID PERMISSION ON test_table TO quoted_user;");
    std::cout << "结果: " << result << std::endl;
    
    // 测试REVOKE语句
    std::cout << "\n测试REVOKE语句" << std::endl;
    result = executor.Execute("REVOKE SELECT ON test_table FROM quoted_user;");
    std::cout << "结果: " << result << std::endl;
    
    // 测试DROP USER IF EXISTS
    std::cout << "\n测试DROP USER IF EXISTS" << std::endl;
    result = executor.Execute("DROP USER IF EXISTS non_existent_user;");
    std::cout << "结果: " << result << std::endl;
    
    // 7. 测试ExecuteFile方法（通过创建临时文件）
    std::cout << "\n=== 测试7: ExecuteFile方法 ===" << std::endl;
    
    // 创建测试SQL文件
    std::string test_file = "./test_comprehensive.sql";
    std::ofstream file(test_file);
    if (file.is_open()) {
        file << "-- 测试SQL文件\n";
        file << "\n";
        file << "SELECT * FROM test_table;\n";
        file << "-- 这是一条注释\n";
        file << "INSERT INTO test_table VALUES (1, 'file_test');\n";
        file << "UPDATE test_table SET name = 'updated' WHERE id = 1;\n";
        file << "-- 空行测试\n";
        file << "\n";
        file << "DELETE FROM test_table WHERE id = 1;\n";
        file << "CREATE TABLE file_test_table (id INT, name VARCHAR(100));\n";
        file << "DROP TABLE file_test_table;\n";
        file.close();
        std::cout << "创建测试SQL文件: " << test_file << std::endl;
        
        // 执行SQL文件
        std::string file_result = executor.ExecuteFile(test_file);
        std::cout << "测试ExecuteFile方法: 执行完成" << std::endl;
        std::cout << "文件执行结果长度: " << file_result.length() << " 字符" << std::endl;
        
        // 清理测试文件
        std::filesystem::remove(test_file);
        std::cout << "清理测试SQL文件" << std::endl;
    }
    
    // 测试ExecuteFile方法 - 文件不存在的情况
    std::cout << "\n测试ExecuteFile方法 - 文件不存在" << std::endl;
    std::string non_existent_result = executor.ExecuteFile("./non_existent_file.sql");
    std::cout << "结果: " << non_existent_result << std::endl;
    
    // 获取并显示错误信息
    std::string error_info = executor.GetLastError();
    std::cout << "错误信息: " << error_info << std::endl;
    
    // 8. 测试GetLastError方法
    std::cout << "\n=== 测试8: GetLastError方法 ===" << std::endl;
    
    // 先执行一个可能产生错误的命令
    executor.Execute("INVALID SQL COMMAND");
    
    // 获取最后一个错误
    std::string last_error = executor.GetLastError();
    std::cout << "最后错误: " << last_error << std::endl;
    
    // 清理临时数据目录
    if (std::filesystem::exists(data_dir)) {
        std::filesystem::remove_all(data_dir);
        std::cout << "\n清理临时数据目录" << std::endl;
    }
    
    // 9. 测试更多的SQL命令变体
    std::cout << "\n=== 测试9: SQL命令变体测试 ===" << std::endl;
    
    // 测试CREATE TABLE的IF NOT EXISTS选项
    std::cout << "\n测试CREATE TABLE IF NOT EXISTS" << std::endl;
    result = executor.Execute("CREATE TABLE IF NOT EXISTS test_table2 (id INT PRIMARY KEY, name VARCHAR(255));");
    std::cout << "结果: " << result << std::endl;
    
    // 测试CREATE TABLE语法错误
    std::cout << "\n测试CREATE TABLE语法错误" << std::endl;
    result = executor.Execute("CREATE TABLE invalid_table id INT, name VARCHAR(255);");
    std::cout << "结果: " << result << std::endl;
    
    // 测试各种ALTER TABLE子句
    std::cout << "\n测试ALTER TABLE ADD" << std::endl;
    result = executor.Execute("ALTER TABLE test_table ADD COLUMN age INT;");
    std::cout << "结果: " << result << std::endl;
    
    std::cout << "\n测试ALTER TABLE MODIFY" << std::endl;
    result = executor.Execute("ALTER TABLE test_table MODIFY COLUMN name VARCHAR(200);");
    std::cout << "结果: " << result << std::endl;
    
    std::cout << "\n测试ALTER TABLE DROP" << std::endl;
    result = executor.Execute("ALTER TABLE test_table DROP COLUMN age;");
    std::cout << "结果: " << result << std::endl;
    
    std::cout << "\n测试ALTER TABLE RENAME" << std::endl;
    result = executor.Execute("ALTER TABLE test_table RENAME TO renamed_table;");
    std::cout << "结果: " << result << std::endl;
    
    // 测试SHOW CREATE TABLE
    std::cout << "\n测试SHOW CREATE TABLE" << std::endl;
    result = executor.Execute("SHOW CREATE TABLE test_table2;");
    std::cout << "结果: " << result << std::endl;
    
    std::cout << "\n=== SQL执行器全面测试完成 ===" << std::endl;
}

int main() {
    try {
        TestSqlExecutorComprehensive();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "测试过程中发生异常: " << e.what() << std::endl;
        return 1;
    }
}
