#include "include/storage/b_plus_tree.h"
#include "include/storage_engine.h"
#include "include/disk_manager.h"
#include "include/utils/config_manager.h"
#include <iostream>
#include <memory>

int main() {
    // 获取配置管理器单例
    sqlcc::ConfigManager& config_manager = sqlcc::ConfigManager::GetInstance();
    
    // 创建存储引擎
    auto storage_engine = std::make_shared<sqlcc::StorageEngine>(config_manager, "/tmp");
    
    // 创建B+树索引
    sqlcc::BPlusTreeIndex index(storage_engine, "test_table", "test_column");
    
    // 尝试创建索引
    std::cout << "Attempting to create index..." << std::endl;
    bool create_result = index.Create();
    
    if (create_result) {
        std::cout << "Index creation successful!" << std::endl;
        
        // 尝试插入一个简单的键值对
        std::cout << "Attempting to insert key-value pair..." << std::endl;
        bool insert_result = index.Insert("key1", 1, 0);
        
        if (insert_result) {
            std::cout << "Insert successful!" << std::endl;
        } else {
            std::cout << "Insert failed!" << std::endl;
        }
    } else {
        std::cout << "Index creation failed!" << std::endl;
    }
    
    return 0;
}