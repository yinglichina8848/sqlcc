#include "storage/b_plus_tree.h"
#include "utils/config_manager.h"
#include "storage_engine.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    try {
        // 创建临时测试目录
        fs::path test_dir = fs::temp_directory_path() / "sqlcc_simple_bplus_tree_test";
        fs::create_directories(test_dir);
        
        // 设置配置管理器
        sqlcc::ConfigManager config_manager;
        
        // 创建StorageEngine实例
        auto storage_engine = std::make_shared<sqlcc::StorageEngine>(config_manager, test_dir.string());
        
        // 创建B+树索引
        auto bplus_tree = std::make_unique<sqlcc::BPlusTreeIndex>(
            storage_engine.get(), "test_table", "test_column");
        
        // 创建索引
        std::cout << "Creating index..." << std::endl;
        bool create_result = bplus_tree->Create();
        std::cout << "Create result: " << (create_result ? "SUCCESS" : "FAILED") << std::endl;
        
        if (!create_result) {
            std::cerr << "Failed to create index" << std::endl;
            return 1;
        }
        
        // 插入一些测试数据
        std::cout << "Inserting test data..." << std::endl;
        bool insert_result1 = bplus_tree->Insert("key1", 1, 0);
        bool insert_result2 = bplus_tree->Insert("key2", 2, 0);
        bool insert_result3 = bplus_tree->Insert("key3", 3, 0);
        
        std::cout << "Insert results: " << insert_result1 << ", " << insert_result2 << ", " << insert_result3 << std::endl;
        
        // 搜索测试数据
        std::cout << "Searching for key2..." << std::endl;
        int32_t page_id;
        size_t offset;
        bool lookup_result = bplus_tree->Lookup("key2", page_id, offset);
        std::cout << "Lookup result: " << (lookup_result ? "FOUND" : "NOT FOUND") << std::endl;
        if (lookup_result) {
            std::cout << "Page ID: " << page_id << ", Offset: " << offset << std::endl;
        }
        
        // 清理
        fs::remove_all(test_dir);
        
        std::cout << "Test completed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}