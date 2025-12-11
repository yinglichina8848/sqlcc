#include "storage/b_plus_tree.h"
#include "utils/config_manager.h"
#include "storage_engine.h"
#include <iostream>
#include <memory>

int main() {
    try {
        // 创建ConfigManager和StorageEngine实例
        auto config_manager = std::make_unique<sqlcc::ConfigManager>();
        auto storage_engine = std::make_shared<sqlcc::StorageEngine>(*config_manager);
        
        // 创建BPlusTreeIndex实例 - 直接使用shared_ptr
        auto b_plus_tree_index = std::make_unique<sqlcc::BPlusTreeIndex>(
            storage_engine, "test_table", "test_column");
        
        // 创建索引
        if (b_plus_tree_index->Create()) {
            std::cout << "SUCCESS: BPlusTreeIndex created successfully!" << std::endl;
            
            // 测试插入
            sqlcc::IndexEntry entry("1", 1, 0);
            if (b_plus_tree_index->Insert(entry)) {
                std::cout << "SUCCESS: Index entry inserted successfully!" << std::endl;
                
                // 测试搜索
                auto results = b_plus_tree_index->Search("1");
                if (!results.empty() && results[0].key == "1") {
                    std::cout << "SUCCESS: Index entry found successfully!" << std::endl;
                    return 0;
                } else {
                    std::cout << "ERROR: Index entry not found!" << std::endl;
                    return 1;
                }
            } else {
                std::cout << "ERROR: Failed to insert index entry!" << std::endl;
                return 1;
            }
        } else {
            std::cout << "ERROR: Failed to create BPlusTreeIndex!" << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cout << "ERROR: Exception caught: " << e.what() << std::endl;
        return 1;
    }
}