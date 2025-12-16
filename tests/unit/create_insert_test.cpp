#include "storage/b_plus_tree.h"
#include "storage_engine.h"
#include "utils/config_manager.h"
#include <iostream>
#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

int main() {
    std::cout << "Starting create and insert test..." << std::endl;
    
    // 创建测试目录
    std::string test_dir = "/tmp/sqlcc_ci_test";
    fs::remove_all(test_dir);
    fs::create_directories(test_dir);
    
    try {
        // 创建配置管理器
        sqlcc::ConfigManager config_manager;
        
        // 创建存储引擎（使用智能指针）
        auto storage_engine = std::make_shared<sqlcc::StorageEngine>(config_manager, test_dir);
        std::cout << "Storage engine created successfully" << std::endl;
        
        // 创建B+树索引
        std::cout << "\n--- Creating B+Tree Index ---" << std::endl;
        sqlcc::BPlusTreeIndex index(storage_engine.get(), "test_table", "test_column");
        std::cout << "BPlusTreeIndex object created" << std::endl;
        
        // 测试创建索引
        bool create_result = index.Create();
        std::cout << "B+Tree Index Create result: " << (create_result ? "SUCCESS" : "FAILED") << std::endl;
        
        if (!create_result) {
            std::cout << "Failed to create index, exiting..." << std::endl;
            return 1;
        }
        
        // 测试插入功能
        std::cout << "\n--- Testing Index Insert ---" << std::endl;
        bool insert_result = index.Insert("key1", 1, 100);
        std::cout << "Index Insert result: " << (insert_result ? "SUCCESS" : "FAILED") << std::endl;
        
        if (insert_result) {
            std::cout << "Create and insert test PASSED!" << std::endl;
        } else {
            std::cout << "Create and insert test FAILED!" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }
    
    // 清理测试目录
    fs::remove_all(test_dir);
    
    std::cout << "\nCreate and insert test completed successfully." << std::endl;
    return 0;
}