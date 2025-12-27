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
        
    // 测试插入操作
    std::cout << "Testing insert operations..." << std::endl;

    // 插入测试数据
    std::string key1 = "test_key_1";
    std::string key2 = "test_key_2";
    std::string key3 = "test_key_3";

    if (!b_plus_tree_index->Insert(key1, 1, 100)) {
        std::cerr << "Failed to insert key1: " << key1 << std::endl;
        return 1;
    }
    std::cout << "Inserted key1: " << key1 << std::endl;

    if (!b_plus_tree_index->Insert(key2, 2, 200)) {
        std::cerr << "Failed to insert key2: " << key2 << std::endl;
        return 1;
    }
    std::cout << "Inserted key2: " << key2 << std::endl;

    if (!b_plus_tree_index->Insert(key3, 3, 300)) {
        std::cerr << "Failed to insert key3: " << key3 << std::endl;
        return 1;
    }
    std::cout << "Inserted key3: " << key3 << std::endl;

    // 测试搜索操作
    std::cout << "Testing search operations..." << std::endl;

    auto results1 = b_plus_tree_index->Search(key1);
    if (results1.size() != 1) {
        std::cerr << "Search failed for key1: " << key1 << ", found " << results1.size() << " results" << std::endl;
        return 1;
    }
    std::cout << "Found key1: " << results1[0].key << ", page_id: " << results1[0].page_id << ", offset: " << results1[0].offset << std::endl;

    auto results2 = b_plus_tree_index->Search(key2);
    if (results2.size() != 1) {
        std::cerr << "Search failed for key2: " << key2 << ", found " << results2.size() << " results" << std::endl;
        return 1;
    }
    std::cout << "Found key2: " << results2[0].key << ", page_id: " << results2[0].page_id << ", offset: " << results2[0].offset << std::endl;

    // 测试范围查询
    std::cout << "Testing range search..." << std::endl;
    auto range_results = b_plus_tree_index->SearchRange("test_key_0", "test_key_9");
    if (range_results.size() != 3) {
        std::cerr << "Range search failed, expected 3 results, got " << range_results.size() << std::endl;
        return 1;
    }
    std::cout << "Range search found " << range_results.size() << " results" << std::endl;

    // 测试删除操作
    std::cout << "Testing delete operations..." << std::endl;
    if (!b_plus_tree_index->Delete(key2)) {
        std::cerr << "Failed to delete key2: " << key2 << std::endl;
        return 1;
    }
    std::cout << "Deleted key2: " << key2 << std::endl;

    // 验证删除
    auto results_after_delete = b_plus_tree_index->Search(key2);
    if (results_after_delete.size() != 0) {
        std::cerr << "Key2 still exists after deletion: " << key2 << std::endl;
        return 1;
    }
    std::cout << "Verified key2 deletion" << std::endl;

        std::cout << "All operations completed successfully" << std::endl;
        
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
