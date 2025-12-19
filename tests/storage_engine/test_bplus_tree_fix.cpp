#include "storage/b_plus_tree.h"
#include "storage_engine.h"
#include "utils/config_manager.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    try {
        // 创建临时测试目录
        fs::path test_dir = fs::temp_directory_path() / "sqlcc_bplus_tree_test_fix";
        fs::create_directories(test_dir);
        
        // 设置配置管理器
        auto config_manager = std::make_unique<sqlcc::ConfigManager>();
        
        // 创建StorageEngine实例，传入临时目录作为数据库路径
        auto storage_engine = std::make_shared<sqlcc::StorageEngine>(*config_manager, test_dir.string());
        
        // 创建BPlusTreeIndex实例
        auto b_plus_tree_index = std::make_unique<sqlcc::BPlusTreeIndex>(
            storage_engine, "test_table", "test_column");
            
        // 创建索引
        if (!b_plus_tree_index->Create()) {
            std::cerr << "Failed to create B+ tree index" << std::endl;
            return 1;
        }
        
        std::cout << "B+ tree index created successfully" << std::endl;
        
        // 插入一些测试数据
        for (int i = 0; i < 10; ++i) {
            std::string key = std::to_string(i);
            if (!b_plus_tree_index->Insert(key, i, 0)) {
                std::cerr << "Failed to insert key: " << key << std::endl;
                return 1;
            }
        }
        
        std::cout << "Inserted 10 test entries" << std::endl;
        
        // 搜索测试数据
        for (int i = 0; i < 10; ++i) {
            std::string key = std::to_string(i);
            auto results = b_plus_tree_index->Search(key);
            if (results.size() != 1) {
                std::cerr << "Search failed for key: " << key << std::endl;
                return 1;
            }
            std::cout << "Found key: " << results[0].key << ", page_id: " << results[0].page_id << std::endl;
        }
        
        std::cout << "All searches successful" << std::endl;
        
        // 范围查询测试
        auto range_results = b_plus_tree_index->SearchRange("2", "7");
        std::cout << "Range query [2,7] returned " << range_results.size() << " results" << std::endl;
        for (const auto& entry : range_results) {
            std::cout << "  Key: " << entry.key << ", page_id: " << entry.page_id << std::endl;
        }
        
        // 删除测试数据
        for (int i = 0; i < 10; ++i) {
            std::string key = std::to_string(i);
            if (!b_plus_tree_index->Delete(key)) {
                std::cerr << "Failed to delete key: " << key << std::endl;
                return 1;
            }
        }
        
        std::cout << "Deleted all test entries" << std::endl;
        
        // 验证删除
        for (int i = 0; i < 10; ++i) {
            std::string key = std::to_string(i);
            auto results = b_plus_tree_index->Search(key);
            if (results.size() != 0) {
                std::cerr << "Key still exists after deletion: " << key << std::endl;
                return 1;
            }
        }
        
        std::cout << "All deletions verified successfully" << std::endl;
        
        // 清理
        b_plus_tree_index.reset();
        storage_engine.reset();
        config_manager.reset();
        
        // 删除临时测试目录
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
        
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }
}