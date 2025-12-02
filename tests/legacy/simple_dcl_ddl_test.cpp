#include "database_manager.h"
#include <iostream>
#include <filesystem>

using namespace sqlcc;
namespace fs = std::filesystem;

int main() {
    try {
        std::cout << "=== Simple DCL and DDL Command Persistence Test ===" << std::endl;
        
        // 清理之前的测试数据
        fs::remove_all("./simple_test_data");
        fs::create_directories("./simple_test_data");
        
        // 第一部分：创建数据库
        std::cout << "Part 1: Creating databases..." << std::endl;
        {
            DatabaseManager db_manager("./simple_test_data", 32, 8, 32);
            
            // 执行CREATE DATABASE命令
            std::cout << "Executing CREATE DATABASE testdb1..." << std::endl;
            if (!db_manager.CreateDatabase("testdb1")) {
                std::cerr << "Failed to create database 'testdb1'" << std::endl;
                return 1;
            }
            std::cout << "Database 'testdb1' created successfully!" << std::endl;
            
            std::cout << "Executing CREATE DATABASE testdb2..." << std::endl;
            if (!db_manager.CreateDatabase("testdb2")) {
                std::cerr << "Failed to create database 'testdb2'" << std::endl;
                return 1;
            }
            std::cout << "Database 'testdb2' created successfully!" << std::endl;
            
            // 列出所有数据库
            std::cout << "Current databases:" << std::endl;
            auto databases = db_manager.ListDatabases();
            for (const auto& db : databases) {
                std::cout << "  - " << db << std::endl;
            }
            
            // 显式关闭
            db_manager.Close();
        } // db_manager在这里被销毁
        
        std::cout << "\nPart 1 completed. Database manager destroyed.\n" << std::endl;
        
        // 第二部分：验证持久化
        std::cout << "Part 2: Verifying persistence..." << std::endl;
        {
            DatabaseManager db_manager("./simple_test_data", 32, 8, 32);
            
            // 检查数据库是否存在
            std::cout << "Checking if databases exist..." << std::endl;
            if (db_manager.DatabaseExists("testdb1")) {
                std::cout << "Database 'testdb1' exists!" << std::endl;
            } else {
                std::cout << "Database 'testdb1' does not exist!" << std::endl;
            }
            
            if (db_manager.DatabaseExists("testdb2")) {
                std::cout << "Database 'testdb2' exists!" << std::endl;
            } else {
                std::cout << "Database 'testdb2' does not exist!" << std::endl;
            }
            
            // 列出所有数据库
            std::cout << "Current databases:" << std::endl;
            auto databases = db_manager.ListDatabases();
            for (const auto& db : databases) {
                std::cout << "  - " << db << std::endl;
            }
            
            // 显式关闭
            db_manager.Close();
        } // db_manager在这里被销毁
        
        std::cout << "\nPart 2 completed. Test finished successfully!" << std::endl;
        
        // 检查目录结构
        std::cout << "\nDirectory structure:" << std::endl;
        if (fs::exists("./simple_test_data")) {
            for (const auto& entry : fs::directory_iterator("./simple_test_data")) {
                if (entry.is_directory()) {
                    std::cout << "Found database directory: " << entry.path().filename().string() << std::endl;
                }
            }
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}