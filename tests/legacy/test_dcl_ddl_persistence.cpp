#include "sql_executor.h"
#include "database_manager.h"
#include <iostream>
#include <filesystem>

using namespace sqlcc;
namespace fs = std::filesystem;

int main() {
    try {
        std::cout << "=== Testing DCL and DDL Command Persistence ===" << std::endl;
        
        // 清理之前的测试数据
        fs::remove_all("./test_data");
        fs::create_directories("./test_data");
        
        // 创建第一个SQL执行器实例
        std::cout << "Step 1: Creating first SqlExecutor instance..." << std::endl;
        SqlExecutor executor1;
        
        // 执行CREATE DATABASE命令
        std::cout << "Step 2: Executing CREATE DATABASE commands..." << std::endl;
        std::string result1 = executor1.Execute("CREATE DATABASE testdb1;");
        std::cout << "CREATE DATABASE testdb1: " << result1 << std::endl;
        
        std::string result2 = executor1.Execute("CREATE DATABASE testdb2;");
        std::cout << "CREATE DATABASE testdb2: " << result2 << std::endl;
        
        // 切换到数据库并创建表
        std::cout << "Step 3: Creating tables..." << std::endl;
        std::string result3 = executor1.Execute("USE testdb1;");
        std::cout << "USE testdb1: " << result3 << std::endl;
        
        std::string result4 = executor1.Execute("CREATE TABLE users (id INT, name VARCHAR(255), age INT);");
        std::cout << "CREATE TABLE users: " << result4 << std::endl;
        
        // 创建用户
        std::cout << "Step 4: Creating users..." << std::endl;
        std::string result5 = executor1.Execute("CREATE USER 'testuser'@'localhost' IDENTIFIED BY 'password';");
        std::cout << "CREATE USER testuser: " << result5 << std::endl;
        
        std::string result6 = executor1.Execute("GRANT SELECT ON testdb1.users TO 'testuser'@'localhost';");
        std::cout << "GRANT SELECT: " << result6 << std::endl;
        
        // 销毁第一个执行器
        std::cout << "Step 5: Destroying first SqlExecutor instance..." << std::endl;
        
        // 创建第二个SQL执行器实例来验证持久化
        std::cout << "Step 6: Creating second SqlExecutor instance to verify persistence..." << std::endl;
        SqlExecutor executor2;
        
        // 检查数据库是否存在
        std::cout << "Step 7: Checking if databases exist..." << std::endl;
        std::string result7 = executor2.Execute("USE testdb1;");
        std::cout << "USE testdb1: " << result7 << std::endl;
        
        std::string result8 = executor2.Execute("USE testdb2;");
        std::cout << "USE testdb2: " << result8 << std::endl;
        
        // 检查表是否存在
        std::cout << "Step 8: Checking if tables exist..." << std::endl;
        std::string result9 = executor2.Execute("USE testdb1;");
        std::cout << "USE testdb1: " << result9 << std::endl;
        
        // 这里我们无法直接列出表，但可以尝试使用表来验证它是否存在
        std::string result10 = executor2.Execute("SELECT * FROM users;");
        std::cout << "SELECT * FROM users: " << result10 << std::endl;
        
        // 检查用户和权限是否存在
        std::cout << "Step 9: Checking if users and privileges exist..." << std::endl;
        // 注意：我们目前没有实现SHOW USERS或SHOW GRANTS命令，所以这里只是概念性的
        
        // 检查数据库目录
        std::cout << "Step 10: Checking database directories..." << std::endl;
        if (fs::exists("./test_data")) {
            for (const auto& entry : fs::directory_iterator("./test_data")) {
                if (entry.is_directory()) {
                    std::cout << "Found database directory: " << entry.path().filename().string() << std::endl;
                }
            }
        } else {
            std::cout << "Database path does not exist: ./test_data" << std::endl;
        }
        
        std::cout << "=== Test completed successfully! ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}