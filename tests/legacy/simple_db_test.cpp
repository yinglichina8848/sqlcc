#include "database_manager.h"
#include <iostream>
#include <filesystem>

using namespace sqlcc;
namespace fs = std::filesystem;

int main() {
    try {
        std::cout << "=== Simple Database Persistence Test ===" << std::endl;
        
        // 清理之前的测试数据
        fs::remove_all("./test_databases");
        fs::create_directories("./test_databases");
        
        // 第一部分：创建数据库
        std::cout << "Part 1: Creating databases..." << std::endl;
        {
            DatabaseManager db_manager("./test_databases", 32, 8, 32);
            
            // 创建测试数据库
            std::cout << "Creating test database 'mydb'..." << std::endl;
            if (!db_manager.CreateDatabase("mydb")) {
                std::cerr << "Failed to create database 'mydb'" << std::endl;
                return 1;
            }
            std::cout << "Database 'mydb' created successfully!" << std::endl;
            
            // 创建另一个测试数据库
            std::cout << "Creating another test database 'yourdb'..." << std::endl;
            if (!db_manager.CreateDatabase("yourdb")) {
                std::cerr << "Failed to create database 'yourdb'" << std::endl;
                return 1;
            }
            std::cout << "Database 'yourdb' created successfully!" << std::endl;
            
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
            DatabaseManager db_manager("./test_databases", 32, 8, 32);
            
            // 检查数据库是否存在
            std::cout << "Checking if databases exist..." << std::endl;
            if (db_manager.DatabaseExists("mydb")) {
                std::cout << "Database 'mydb' exists!" << std::endl;
            } else {
                std::cout << "Database 'mydb' does not exist!" << std::endl;
            }
            
            if (db_manager.DatabaseExists("yourdb")) {
                std::cout << "Database 'yourdb' exists!" << std::endl;
            } else {
                std::cout << "Database 'yourdb' does not exist!" << std::endl;
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
        if (fs::exists("./test_databases")) {
            for (const auto& entry : fs::directory_iterator("./test_databases")) {
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