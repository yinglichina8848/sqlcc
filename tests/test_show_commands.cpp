#include "include/sql_executor.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;
using namespace sqlcc;

int main() {
    try {
        // 清理旧数据
        fs::remove_all("./test_show_data");
        
        std::cout << "=== 测试SHOW命令 ===" << std::endl;
        
        // 使用默认构造函数
        SqlExecutor executor;
        
        // 测试1: SHOW DATABASES
        std::cout << "\n【测试1】SHOW DATABASES (空)" << std::endl;
        std::string result = executor.Execute("SHOW DATABASES");
        std::cout << result << std::endl;
        
        // 创建数据库
        std::cout << "\n【测试2】创建数据库" << std::endl;
        result = executor.Execute("CREATE DATABASE db1");
        std::cout << result << std::endl;
        
        result = executor.Execute("CREATE DATABASE db2");
        std::cout << result << std::endl;
        
        // 测试3: SHOW DATABASES (有数据)
        std::cout << "\n【测试3】SHOW DATABASES (有数据)" << std::endl;
        result = executor.Execute("SHOW DATABASES");
        std::cout << result << std::endl;
        
        // 测试4: USE DATABASE并SHOW TABLES
        std::cout << "\n【测试4】USE db1" << std::endl;
        result = executor.Execute("USE db1");
        std::cout << result << std::endl;
        
        std::cout << "\n【测试5】SHOW TABLES (空)" << std::endl;
        result = executor.Execute("SHOW TABLES");
        std::cout << result << std::endl;
        
        // 测试6: 创建表
        std::cout << "\n【测试6】创建表" << std::endl;
        result = executor.Execute("CREATE TABLE users (id INT, name VARCHAR(50))");
        std::cout << result << std::endl;
        
        result = executor.Execute("CREATE TABLE products (id INT, price FLOAT)");
        std::cout << result << std::endl;
        
        // 测试7: SHOW TABLES (有数据)
        std::cout << "\n【测试7】SHOW TABLES (有数据)" << std::endl;
        result = executor.Execute("SHOW TABLES");
        std::cout << result << std::endl;
        
        // 测试8: SHOW TABLES FROM db2
        std::cout << "\n【测试8】SHOW TABLES FROM db2" << std::endl;
        result = executor.Execute("SHOW TABLES FROM db2");
        std::cout << result << std::endl;
        
        // 测试9: SHOW CREATE TABLE
        std::cout << "\n【测试9】SHOW CREATE TABLE users" << std::endl;
        result = executor.Execute("SHOW CREATE TABLE users");
        std::cout << result << std::endl;
        
        // 测试10: SHOW COLUMNS
        std::cout << "\n【测试10】SHOW COLUMNS FROM users" << std::endl;
        result = executor.Execute("SHOW COLUMNS FROM users");
        std::cout << result << std::endl;
        
        // 测试11: SHOW INDEXES
        std::cout << "\n【测试11】SHOW INDEXES FROM users" << std::endl;
        result = executor.Execute("SHOW INDEXES FROM users");
        std::cout << result << std::endl;
        
        std::cout << "\n=== 所有测试完成 ===" << std::endl;
        
        // 清理
        fs::remove_all("./test_show_data");
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
}
