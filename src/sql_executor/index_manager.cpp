#include "b_plus_tree.h"
#include "logger.h"
#include "config_manager.h"

namespace sqlcc {

IndexManager::IndexManager(StorageEngine* storage_engine, ConfigManager&) 
    : storage_engine_(storage_engine) {
    SQLCC_LOG_INFO("Initializing IndexManager");
    LoadAllIndexes();
}

IndexManager::~IndexManager() {
    SQLCC_LOG_INFO("Destroying IndexManager");
    // 索引对象会通过unique_ptr自动清理
    indexes_.clear();
}

bool IndexManager::CreateIndex(const std::string& index_name, const std::string& table_name, const std::string& column_name, bool) {
    SQLCC_LOG_INFO("Creating index: " + index_name + " on table: " + table_name + ", column: " + column_name);
    
    // 检查索引是否已存在
    if (IndexExists(index_name, table_name)) {
        SQLCC_LOG_WARN("Index already exists: " + index_name);
        return false;
    }
    
    // 创建新的B+树索引
    auto index = std::make_unique<BPlusTreeIndex>(storage_engine_, table_name, column_name);
    if (!index->Create()) {
        SQLCC_LOG_ERROR("Failed to create index: " + index_name);
        return false;
    }
    
    // 将索引添加到索引映射表
    indexes_[index_name] = std::move(index);
    SQLCC_LOG_INFO("Index created successfully: " + index_name);
    return true;
}

bool IndexManager::DropIndex(const std::string& index_name, const std::string& table_name) {
    SQLCC_LOG_INFO("Dropping index: " + index_name + " on table: " + table_name);
    
    // 查找索引
    auto it = indexes_.find(index_name);
    if (it == indexes_.end() || it->second->GetTableName() != table_name) {
        SQLCC_LOG_WARN("Index not found: " + index_name);
        return false;
    }
    
    // 删除索引
    if (!it->second->Drop()) {
        SQLCC_LOG_ERROR("Failed to drop index: " + index_name);
        return false;
    }
    
    // 从映射表中移除索引
    indexes_.erase(it);
    SQLCC_LOG_INFO("Index dropped successfully: " + index_name);
    return true;
}

bool IndexManager::IndexExists(const std::string& index_name, const std::string& table_name) const {
    auto it = indexes_.find(index_name);
    if (it != indexes_.end()) {
        return it->second->GetTableName() == table_name;
    }
    return false;
}

BPlusTreeIndex* IndexManager::GetIndex(const std::string& index_name, const std::string& table_name) {
    auto it = indexes_.find(index_name);
    if (it != indexes_.end() && it->second->GetTableName() == table_name) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<BPlusTreeIndex*> IndexManager::GetTableIndexes(const std::string& table_name) const {
    std::vector<BPlusTreeIndex*> result;
    
    for (const auto& [index_name, index] : indexes_) {
        if (index->GetTableName() == table_name) {
            result.push_back(index.get());
        }
    }
    
    return result;
}

std::string IndexManager::GetIndexName(const std::string& table_name, const std::string& column_name) const {
    return table_name + "_" + column_name + "_idx";
}

void IndexManager::LoadAllIndexes() {
    SQLCC_LOG_INFO("Loading all indexes from storage");
    // 实现从存储加载所有索引的功能
    // 目前只是一个基本实现
}

} // namespace sqlcc