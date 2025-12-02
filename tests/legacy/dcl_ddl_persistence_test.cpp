#include "database_manager.h"
#include "user_manager.h"
#include <iostream>
#include <filesystem>

using namespace sqlcc;
namespace fs = std::filesystem;

int main() {
    try {
        std::cout << "=== DCL and DDL Command Persistence Test ===" << std::endl;
        
        // 清理之前的测试数据
        fs::remove_all("./dcl_ddl_test_data");
        fs::create_directories("./dcl_ddl_test_data");
        
        // 第一部分：创建数据库和用户
        std::cout << "Part 1: Creating databases and users..." << std::endl;
        {
            DatabaseManager db_manager("./dcl_ddl_test_data", 32, 8, 32);
            UserManager user_manager;
            
            // 创建数据库
            std::cout << "Creating database 'testdb'..." << std::endl;
            if (!db_manager.CreateDatabase("testdb")) {
                std::cerr << "Failed to create database 'testdb'" << std::endl;
                return 1;
            }
            std::cout << "Database 'testdb' created successfully!" << std::endl;
            
            // 使用数据库
            std::cout << "Using database 'testdb'..." << std::endl;
            if (!db_manager.UseDatabase("testdb")) {
                std::cerr << "Failed to use database 'testdb'" << std::endl;
                return 1;
            }
            std::cout << "Database 'testdb' is now in use!" << std::endl;
            
            // 创建表
            std::cout << "Creating table 'users'..." << std::endl;
            std::vector<std::pair<std::string, std::string>> columns = {
                {"id", "INT"},
                {"username", "VARCHAR(255)"},
                {"password", "VARCHAR(255)"}
            };
            
            if (!db_manager.CreateTable("users", columns)) {
                std::cerr << "Failed to create table 'users'" << std::endl;
                return 1;
            }
            std::cout << "Table 'users' created successfully!" << std::endl;
            
            // 创建用户（确保角色存在）
            std::cout << "Creating user 'testuser'..." << std::endl;
            // UserManager::CreateUser(username, password, role)
            // 默认角色为"USER"
            if (!user_manager.CreateUser("testuser", "password123")) {
                std::cerr << "Failed to create user 'testuser': " << user_manager.GetLastError() << std::endl;
                return 1;
            }
            std::cout << "User 'testuser' created successfully!" << std::endl;
            
            // 授予权限
            std::cout << "Granting privileges to user 'testuser'..." << std::endl;
            if (!user_manager.GrantPrivilege("testuser", "testdb", "*", "ALL")) {
                std::cerr << "Failed to grant privileges to user 'testuser'" << std::endl;
                return 1;
            }
            std::cout << "Privileges granted to user 'testuser' successfully!" << std::endl;
            
            // 显式关闭
            db_manager.Close();
        } // db_manager在这里被销毁
        
        std::cout << "\nPart 1 completed. Database manager destroyed.\n" << std::endl;
        
        // 第二部分：验证持久化
        std::cout << "Part 2: Verifying persistence..." << std::endl;
        {
            DatabaseManager db_manager("./dcl_ddl_test_data", 32, 8, 32);
            UserManager user_manager;
            
            // 检查数据库是否存在
            std::cout << "Checking if database 'testdb' exists..." << std::endl;
            if (db_manager.DatabaseExists("testdb")) {
                std::cout << "Database 'testdb' exists!" << std::endl;
            } else {
                std::cout << "Database 'testdb' does not exist!" << std::endl;
                return 1;
            }
            
            // 使用数据库
            std::cout << "Using database 'testdb'..." << std::endl;
            if (!db_manager.UseDatabase("testdb")) {
                std::cerr << "Failed to use database 'testdb'" << std::endl;
                return 1;
            }
            std::cout << "Database 'testdb' is now in use!" << std::endl;
            
            // 检查表是否存在
            std::cout << "Checking if table 'users' exists..." << std::endl;
            if (db_manager.TableExists("users")) {
                std::cout << "Table 'users' exists!" << std::endl;
            } else {
                std::cout << "Table 'users' does not exist!" << std::endl;
                return 1;
            }
            
            // 注意：由于UserManager目前没有提供查询用户和权限的方法，
            // 我们无法直接验证用户和权限的持久化。
            // 这将在后续实现中完善。
            
            // 显式关闭
            db_manager.Close();
        } // db_manager在这里被销毁
        
        std::cout << "\nPart 2 completed. Test finished successfully!" << std::endl;
        
        // 检查目录结构
        std::cout << "\nDirectory structure:" << std::endl;
        if (fs::exists("./dcl_ddl_test_data")) {
            for (const auto& entry : fs::directory_iterator("./dcl_ddl_test_data")) {
                if (entry.is_directory()) {
                    std::cout << "Found database directory: " << entry.path().filename().string() << std::endl;
                    
                    // 检查表文件
                    for (const auto& table_entry : fs::directory_iterator(entry.path())) {
                        if (table_entry.is_regular_file()) {
                            std::cout << "  Found table file: " << table_entry.path().filename().string() << std::endl;
                        }
                    }
                }
            }
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}