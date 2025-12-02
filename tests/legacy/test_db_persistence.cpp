#include "database_manager.h"
#include "config_manager.h"
#include <iostream>
#include <filesystem>

using namespace sqlcc;
namespace fs = std::filesystem;

int main() {
    try {
        std::cout << "=== Testing Database Persistence ===" << std::endl;
        
        // 清理之前的测试数据
        fs::remove_all("./databases");
        fs::create_directories("./databases");
        
        // 创建第一个数据库管理器实例
        std::cout << "Step 1: Creating first DatabaseManager instance..." << std::endl;
        auto db_manager1 = std::make_shared<DatabaseManager>("./databases", 64, 16, 64);
        
        // 创建测试数据库
        std::cout << "Step 2: Creating test database 'testdb'..." << std::endl;
        if (!db_manager1->CreateDatabase("testdb")) {
            std::cerr << "Failed to create test database" << std::endl;
            return 1;
        }
        std::cout << "Database 'testdb' created successfully!" << std::endl;
        
        // 创建另一个测试数据库
        std::cout << "Step 3: Creating another test database 'anotherdb'..." << std::endl;
        if (!db_manager1->CreateDatabase("anotherdb")) {
            std::cerr << "Failed to create another test database" << std::endl;
            return 1;
        }
        std::cout << "Database 'anotherdb' created successfully!" << std::endl;
        
        // 列出所有数据库
        std::cout << "Step 4: Listing all databases..." << std::endl;
        auto databases = db_manager1->ListDatabases();
        for (const auto& db : databases) {
            std::cout << "  - " << db << std::endl;
        }
        
        // 关闭第一个数据库管理器
        std::cout << "Step 5: Closing first DatabaseManager instance..." << std::endl;
        db_manager1->Close(); // 显式调用Close而不是依赖析构函数
        db_manager1.reset();
        
        // 创建第二个数据库管理器实例来验证持久化
        std::cout << "Step 6: Creating second DatabaseManager instance to verify persistence..." << std::endl;
        auto db_manager2 = std::make_shared<DatabaseManager>("./databases", 64, 16, 64);
        
        // 检查测试数据库是否存在
        std::cout << "Step 7: Checking if test databases exist..." << std::endl;
        if (db_manager2->DatabaseExists("testdb")) {
            std::cout << "Test database 'testdb' exists!" << std::endl;
        } else {
            std::cout << "Test database 'testdb' does not exist!" << std::endl;
        }
        
        if (db_manager2->DatabaseExists("anotherdb")) {
            std::cout << "Test database 'anotherdb' exists!" << std::endl;
        } else {
            std::cout << "Test database 'anotherdb' does not exist!" << std::endl;
        }
        
        // 列出所有数据库（第二次检查）
        std::cout << "Step 8: Listing all databases again..." << std::endl;
        auto databases2 = db_manager2->ListDatabases();
        for (const auto& db : databases2) {
            std::cout << "  - " << db << std::endl;
        }
        
        // 检查数据库目录
        std::cout << "Step 9: Checking database directories..." << std::endl;
        std::string db_path = "./databases";
        if (fs::exists(db_path)) {
            for (const auto& entry : fs::directory_iterator(db_path)) {
                if (entry.is_directory()) {
                    std::cout << "Found database directory: " << entry.path().filename().string() << std::endl;
                }
            }
        } else {
            std::cout << "Database path does not exist: " << db_path << std::endl;
        }
        
        // 关闭第二个数据库管理器
        db_manager2->Close();
        db_manager2.reset();
        
        std::cout << "=== Test completed successfully! ===" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}