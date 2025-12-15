#include "storage/b_plus_tree.h"
#include "storage/disk_manager.h"
#include "storage_engine.h"
#include <iostream>
#include <memory>
#include <string>

int main() {
    // 创建存储引擎和磁盘管理器
    auto disk_manager = std::make_shared<DiskManager>("test_b_plus_tree.db");
    auto storage_engine = std::make_shared<StorageEngine>(disk_manager);
    
    // 创建B+树索引
    auto b_plus_tree = std::make_unique<BPlusTreeIndex>(storage_engine, 0, 100);
    
    std::cout << "Testing B+Tree Insert and Search..." << std::endl;
    
    // 插入一些测试数据
    std::cout << "Inserting test data..." << std::endl;
    bool insert_result = b_plus_tree->Insert("key1", 1, 0);
    std::cout << "Insert key1: " << (insert_result ? "SUCCESS" : "FAILED") << std::endl;
    
    insert_result = b_plus_tree->Insert("key2", 2, 0);
    std::cout << "Insert key2: " << (insert_result ? "SUCCESS" : "FAILED") << std::endl;
    
    insert_result = b_plus_tree->Insert("key3", 3, 0);
    std::cout << "Insert key3: " << (insert_result ? "SUCCESS" : "FAILED") << std::endl;
    
    // 搜索测试数据
    std::cout << "Searching test data..." << std::endl;
    auto results1 = b_plus_tree->Search("key1");
    std::cout << "Search key1: found " << results1.size() << " entries" << std::endl;
    
    auto results2 = b_plus_tree->Search("key2");
    std::cout << "Search key2: found " << results2.size() << " entries" << std::endl;
    
    auto results3 = b_plus_tree->Search("key3");
    std::cout << "Search key3: found " << results3.size() << " entries" << std::endl;
    
    // 测试查找功能
    std::cout << "Testing Lookup function..." << std::endl;
    int32_t page_id;
    size_t offset;
    bool lookup_result = b_plus_tree->Lookup("key1", page_id, offset);
    std::cout << "Lookup key1: " << (lookup_result ? "FOUND" : "NOT FOUND") 
              << " (page_id=" << page_id << ", offset=" << offset << ")" << std::endl;
    
    // 清理测试文件
    std::remove("test_b_plus_tree.db");
    
    std::cout << "Test completed." << std::endl;
    
    return 0;
}