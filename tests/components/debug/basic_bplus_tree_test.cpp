#include "storage/b_plus_tree.h"
#include "storage_engine.h"
#include "utils/config_manager.h"
#include <iostream>
#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

int main() {
    std::cout << "Starting basic B+Tree test..." << std::endl;
    
    // 创建测试目录
    std::string test_dir = "/tmp/sqlcc_bpt_test";
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
        sqlcc::BPlusTreeIndex index(storage_engine, "test_table", "test_column");
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
        
        // 测试搜索功能
        std::cout << "\n--- Testing Index Search ---" << std::endl;
        std::vector<sqlcc::IndexEntry> search_result = index.Search("key1");
        std::cout << "Index Search result: " << (search_result.size() > 0 ? "SUCCESS" : "FAILED") 
                  << " (found " << search_result.size() << " entries)" << std::endl;
        
        if (search_result.size() > 0) {
            std::cout << "Found entry: key=" << search_result[0].key 
                      << ", page_id=" << search_result[0].page_id 
                      << ", offset=" << search_result[0].offset << std::endl;
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
    
    std::cout << "\nBasic B+Tree test completed." << std::endl;
    return 0;
}