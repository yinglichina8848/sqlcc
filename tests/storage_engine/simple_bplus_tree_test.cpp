#include "storage/b_plus_tree.h"
#include "storage_engine.h"
#include "utils/config_manager.h"
#include <iostream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

int main() {
    try {
        std::cout << "Starting B+ Tree test..." << std::endl;
        
        // 创建临时测试目录
        fs::path test_dir = fs::temp_directory_path() / "sqlcc_simple_bplus_tree_test";
        fs::create_directories(test_dir);
        
        std::cout << "Created test directory: " << test_dir << std::endl;
        
        // 设置配置管理器
        auto config_manager = std::make_unique<sqlcc::ConfigManager>();
        
        // 创建StorageEngine实例
        auto storage_engine = std::make_shared<sqlcc::StorageEngine>(*config_manager, test_dir.string());
        
        std::cout << "Created storage engine" << std::endl;
        
        // 创建BPlusTreeIndex实例
        auto b_plus_tree_index = std::make_unique<sqlcc::BPlusTreeIndex>(
            storage_engine, "test_table", "test_column");
            
        std::cout << "Created B+ tree index" << std::endl;
        
        // 创建索引
        if (!b_plus_tree_index->Create()) {
            std::cerr << "Failed to create B+ tree index" << std::endl;
            return 1;
        }
        
        std::cout << "B+ tree index created successfully" << std::endl;
        
        // 插入一些测试数据
        std::cout << "Inserting test data..." << std::endl;
        for (int i = 0; i < 5; ++i) {
            std::string key = std::to_string(i);
            if (!b_plus_tree_index->Insert(key, i, 0)) {
                std::cerr << "Failed to insert key: " << key << std::endl;
                return 1;
            }
            std::cout << "Inserted key: " << key << std::endl;
        }
        
        // 搜索测试数据
        std::cout << "Searching test data..." << std::endl;
        for (int i = 0; i < 5; ++i) {
            std::string key = std::to_string(i);
            auto results = b_plus_tree_index->Search(key);
            if (results.size() != 1) {
                std::cerr << "Search failed for key: " << key << ", found " << results.size() << " results" << std::endl;
                return 1;
            }
            std::cout << "Found key: " << results[0].key << ", page_id: " << results[0].page_id << std::endl;
        }
        
        // 测试不存在的键
        auto results = b_plus_tree_index->Search("nonexistent");
        if (results.size() != 0) {
            std::cerr << "Search for nonexistent key returned results" << std::endl;
            return 1;
        }
        std::cout << "Search for nonexistent key correctly returned empty results" << std::endl;
        
        // 范围查询测试
        std::cout << "Testing range query..." << std::endl;
        auto range_results = b_plus_tree_index->SearchRange("1", "3");
        std::cout << "Range query [1,3] returned " << range_results.size() << " results" << std::endl;
        for (const auto& entry : range_results) {
            std::cout << "  Key: " << entry.key << ", page_id: " << entry.page_id << std::endl;
        }
        
        // 删除测试数据
        std::cout << "Deleting test data..." << std::endl;
        for (int i = 0; i < 5; ++i) {
            std::string key = std::to_string(i);
            if (!b_plus_tree_index->Delete(key)) {
                std::cerr << "Failed to delete key: " << key << std::endl;
                return 1;
            }
            std::cout << "Deleted key: " << key << std::endl;
        }
        
        // 验证删除
        std::cout << "Verifying deletions..." << std::endl;
        for (int i = 0; i < 5; ++i) {
            std::string key = std::to_string(i);
            auto results = b_plus_tree_index->Search(key);
            if (results.size() != 0) {
                std::cerr << "Key still exists after deletion: " << key << std::endl;
                return 1;
            }
        }
        std::cout << "All deletions verified successfully" << std::endl;
        
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }
}