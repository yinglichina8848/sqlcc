#include "storage/b_plus_tree.h"
#include "storage_engine.h"
#include "utils/config_manager.h"
#include <iostream>
#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

int main() {
    std::cout << "Starting minimal crash test..." << std::endl;
    
    // 创建测试目录
    std::string test_dir = "/tmp/sqlcc_minimal_test";
    fs::remove_all(test_dir);
    fs::create_directories(test_dir);
    
    try {
        // 创建配置管理器
        auto config_manager = std::make_shared<sqlcc::ConfigManager>();
        
        // 创建存储引擎
        auto storage_engine = std::make_shared<sqlcc::StorageEngine>(*config_manager, test_dir);
        std::cout << "Storage engine created successfully" << std::endl;
        
        // 创建B+树索引
        std::cout << "\n--- Creating B+Tree Index ---" << std::endl;
        sqlcc::BPlusTreeIndex index(storage_engine, "test_table", "test_column");
        std::cout << "BPlusTreeIndex object created" << std::endl;
        
        bool create_result = index.Create();
        std::cout << "B+Tree Index Create result: " << (create_result ? "SUCCESS" : "FAILED") << std::endl;
        
        // 测试索引是否存在
        std::cout << "\n--- Testing Index Exists ---" << std::endl;
        bool exists_result = index.Exists();
        std::cout << "Index Exists result: " << (exists_result ? "SUCCESS" : "FAILED") << std::endl;
        
        // 现在尝试直接创建一个叶子节点并插入条目
        std::cout << "\n--- Testing Direct Leaf Node Operations ---" << std::endl;
        int32_t page_id;
        storage_engine->NewPage(&page_id);
        std::cout << "Created page with ID: " << page_id << std::endl;
        
        // 创建叶子节点
        std::cout << "Creating leaf node..." << std::endl;
        auto leaf_node = std::make_unique<sqlcc::BPlusTreeLeafNode>(storage_engine, page_id);
        std::cout << "Leaf node created successfully" << std::endl;
        
        // 创建索引条目
        std::cout << "Creating index entry..." << std::endl;
        sqlcc::IndexEntry entry("key1", 1, 100);
        std::cout << "Index entry created" << std::endl;
        
        // 尝试插入条目
        std::cout << "Inserting entry into leaf node..." << std::endl;
        bool insert_result = leaf_node->Insert(entry);
        std::cout << "Leaf node insert result: " << (insert_result ? "SUCCESS" : "FAILED") << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }
    
    // 清理测试目录
    fs::remove_all(test_dir);
    
    std::cout << "\nMinimal crash test completed." << std::endl;
    return 0;
}