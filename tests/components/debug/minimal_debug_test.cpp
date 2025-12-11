#include "storage/b_plus_tree.h"
#include "storage_engine.h"
#include "utils/config_manager.h"
#include <iostream>
#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

int main() {
    std::cout << "Starting minimal debug test..." << std::endl;
    
    // 创建测试目录
    std::string test_dir = "/tmp/sqlcc_minimal_debug_test";
    fs::remove_all(test_dir);
    fs::create_directories(test_dir);
    
    try {
        // 创建配置管理器
        auto config_manager = std::make_shared<sqlcc::ConfigManager>();
        
        // 创建存储引擎
        auto storage_engine = std::make_shared<sqlcc::StorageEngine>(*config_manager, test_dir);
        std::cout << "Storage engine created successfully" << std::endl;
        
        // 测试B+树索引创建
        std::cout << "\n--- Testing B+Tree Index Creation ---" << std::endl;
        sqlcc::BPlusTreeIndex index(storage_engine, "test_table", "test_column");
        bool create_result = index.Create();
        std::cout << "B+Tree Index Create result: " << (create_result ? "SUCCESS" : "FAILED") << std::endl;
        
        if (!create_result) {
            std::cerr << "Failed to create index" << std::endl;
            return 1;
        }
        
        // 测试索引是否存在
        std::cout << "\n--- Testing Index Exists ---" << std::endl;
        bool exists_result = index.Exists();
        std::cout << "Index Exists result: " << (exists_result ? "SUCCESS" : "FAILED") << std::endl;
        
        if (!exists_result) {
            std::cerr << "Index does not exist" << std::endl;
            return 1;
        }
        
        std::cout << "\nMinimal debug test completed successfully." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }
    
    // 清理测试目录
    fs::remove_all(test_dir);
    
    return 0;
}