#include "storage/b_plus_tree.h"
#include "storage_engine.h"
#include "utils/config_manager.h"
#include <iostream>
#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

int main() {
    std::cout << "Starting debug index test..." << std::endl;
    
    // 创建测试目录
    std::string test_dir = "/tmp/sqlcc_debug_test";
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
        
        // 测试索引是否存在
        std::cout << "\n--- Testing Index Exists ---" << std::endl;
        bool exists_result = index.Exists();
        std::cout << "Index Exists result: " << (exists_result ? "SUCCESS" : "FAILED") << std::endl;
        
        // 测试插入条目
        std::cout << "\n--- Testing Index Entry Insertion ---" << std::endl;
        std::cout << "Creating IndexEntry..." << std::endl;
        sqlcc::IndexEntry entry("key1", 1, 100);
        std::cout << "IndexEntry created: key=" << entry.key << ", page_id=" << entry.page_id << ", offset=" << entry.offset << std::endl;
        
        std::cout << "Calling index.Insert()..." << std::endl;
        bool insert_result = index.Insert(entry.key, entry.page_id, entry.offset);
        std::cout << "Index Entry Insert result: " << (insert_result ? "SUCCESS" : "FAILED") << std::endl;
        
        // 测试搜索条目
        std::cout << "\n--- Testing Index Entry Search ---" << std::endl;
        std::vector<sqlcc::IndexEntry> search_result = index.Search("key1");
        std::cout << "Index Entry Search result: " << (search_result.size() > 0 ? "SUCCESS" : "FAILED") 
                  << " (found " << search_result.size() << " entries)" << std::endl;
        
        // 测试删除索引
        std::cout << "\n--- Testing Index Drop ---" << std::endl;
        bool drop_result = index.Drop();
        std::cout << "Index Drop result: " << (drop_result ? "SUCCESS" : "FAILED") << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }
    
    // 清理测试目录
    fs::remove_all(test_dir);
    
    std::cout << "\nDebug test completed." << std::endl;
    return 0;
}
