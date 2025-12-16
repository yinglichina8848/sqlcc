#include "storage/b_plus_tree.h"
#include "storage_engine.h"
#include "utils/config_manager.h"
#include <iostream>
#include <filesystem>
#include <memory>

namespace fs = std::filesystem;

int main() {
    std::cout << "Starting index insert test..." << std::endl;
    
    // 创建测试目录
    std::string test_dir = "/tmp/sqlcc_insert_test";
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
        
        if (!exists_result) {
            std::cout << "Index does not exist, cannot continue with insert test" << std::endl;
            return 1;
        }
        
        // 创建索引条目
        std::cout << "\n--- Creating Index Entry ---" << std::endl;
        sqlcc::IndexEntry entry("key1", 1, 100);
        std::cout << "IndexEntry created: key=" << entry.key << ", page_id=" << entry.page_id << ", offset=" << entry.offset << std::endl;
        
        // 尝试插入条目到索引
        std::cout << "\n--- Testing Index Insert ---" << std::endl;
        std::cout << "Calling index.Insert()..." << std::endl;
        bool insert_result = index.Insert(entry.key, entry.page_id, entry.offset);
        std::cout << "Index Insert result: " << (insert_result ? "SUCCESS" : "FAILED") << std::endl;
        
        // 测试搜索条目
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
    
    std::cout << "\nIndex insert test completed." << std::endl;
    return 0;
}
