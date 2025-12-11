#include "table_storage.h"
#include "storage_engine.h"
#include "config_manager.h"
#include <iostream>
#include <vector>
#include <memory>
#include <filesystem>

using namespace sqlcc;
namespace fs = std::filesystem;

int main() {
    // 创建测试目录
    std::string test_dir = "/tmp/sqlcc_table_test";
    fs::remove_all(test_dir);
    fs::create_directories(test_dir);

    try {
        // 创建配置管理器（使用智能指针）
        auto config_manager = std::make_shared<ConfigManager>();
        config_manager->SetValue("database.file", test_dir + "/test.db");
        config_manager->SetValue("buffer.pool.size", 64);
        config_manager->SetValue("buffer.shard.count", 16);

        // 创建存储引擎（使用智能指针）
        auto storage_engine = std::make_shared<StorageEngine>(*config_manager, test_dir);

        // 创建表存储管理器（使用智能指针）
        auto table_storage = std::make_shared<TableStorageManager>(storage_engine);
        
        // 定义表列
        std::vector<TableColumn> columns = {
            {"id", "INT", sizeof(int32_t), false, ""},
            {"name", "VARCHAR", 255, false, ""},
            {"age", "INT", sizeof(int32_t), true, ""}
        };
        
        // 创建表
        std::cout << "Creating table 'users'..." << std::endl;
        if (table_storage->CreateTable("users", columns)) {
            std::cout << "Table 'users' created successfully!" << std::endl;
        } else {
            std::cout << "Failed to create table 'users'" << std::endl;
            return 1;
        }
        
        // 插入记录
        std::cout << "Inserting records..." << std::endl;
        std::vector<std::string> record1 = {"1", "Alice", "25"};
        std::vector<std::string> record2 = {"2", "Bob", "30"};
        std::vector<std::string> record3 = {"3", "Charlie", ""}; // age为NULL
        
        int32_t page_id1, page_id2, page_id3;
        size_t offset1, offset2, offset3;
        
        if (table_storage->InsertRecord("users", record1, page_id1, offset1)) {
            std::cout << "Record 1 inserted at page " << page_id1 << ", offset " << offset1 << std::endl;
        }
        
        if (table_storage->InsertRecord("users", record2, page_id2, offset2)) {
            std::cout << "Record 2 inserted at page " << page_id2 << ", offset " << offset2 << std::endl;
        }
        
        if (table_storage->InsertRecord("users", record3, page_id3, offset3)) {
            std::cout << "Record 3 inserted at page " << page_id3 << ", offset " << offset3 << std::endl;
        }
        
        // 创建索引
        std::cout << "Creating index on 'name' column..." << std::endl;
        if (table_storage->CreateIndex("users", "name")) {
            std::cout << "Index on 'name' column created successfully!" << std::endl;
        } else {
            std::cout << "Failed to create index on 'name' column" << std::endl;
        }
        
        // 查询记录
        std::cout << "Retrieving records..." << std::endl;
        auto retrieved_record1 = table_storage->GetRecord("users", page_id1, offset1);
        if (!retrieved_record1.empty()) {
            std::cout << "Retrieved record 1: ";
            for (const auto& field : retrieved_record1) {
                std::cout << field << " ";
            }
            std::cout << std::endl;
        }
        
        std::cout << "Test completed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
