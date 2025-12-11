#include "storage/b_plus_tree.h"
#include "storage_engine.h"
#include "utils/config_manager.h"
#include <iostream>
#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

int main() {
    std::cout << "Starting simple index test..." << std::endl;
    
    // 创建测试目录
    std::string test_dir = "/tmp/sqlcc_simple_test";
    fs::remove_all(test_dir);
    fs::create_directories(test_dir);
    
    try {
        // 创建配置管理器
        sqlcc::ConfigManager config_manager;
        
        // 创建存储引擎
        sqlcc::StorageEngine storage_engine(config_manager, test_dir);
        std::cout << "Storage engine created successfully" << std::endl;
        
        // 创建B+树索引
        std::cout << "\n--- Creating B+Tree Index ---" << std::endl;
        sqlcc::BPlusTreeIndex index(&storage_engine, "test_table", "test_column");
        std::cout << "BPlusTreeIndex object created" << std::endl;
        
        bool create_result = index.Create();
        std::cout << "B+Tree Index Create result: " << (create_result ? "SUCCESS" : "FAILED") << std::endl;
        
        // 假设创建成功，继续测试插入功能
        std::cout << "Assuming index creation worked and continuing with insert test" << std::endl;
        
        // 尝试插入条目到索引
        std::cout << "\n--- Testing Index Insert ---" << std::endl;
        std::cout << "Calling index.Insert()..." << std::endl;
        bool insert_result = index.Insert("key1", 1, 100);
        std::cout << "Index Insert result: " << (insert_result ? "SUCCESS" : "FAILED") << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }
    
    // 清理测试目录
    fs::remove_all(test_dir);
    
    std::cout << "\nSimple index test completed." << std::endl;
    return 0;
}