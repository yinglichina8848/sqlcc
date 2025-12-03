#include "table_storage.h"
#include "storage_engine.h"
#include "b_plus_tree.h"
#include "logger.h"
#include <cstring>
#include <algorithm>

namespace sqlcc {

TableStorageManager::TableStorageManager(std::shared_ptr<StorageEngine> storage_engine)
    : storage_engine_(storage_engine) {
    // TODO: 需要实现IndexManager类
    // 临时注释掉索引管理器初始化
    // index_manager_ = std::make_shared<IndexManager>(storage_engine_.get(), storage_engine->GetConfigManager());
}

TableStorageManager::~TableStorageManager() {
}

bool TableStorageManager::CreateTable(const std::string& table_name, const std::vector<TableColumn>& columns) {
    // 检查表是否已存在
    if (TableExists(table_name)) {
        SQLCC_LOG_WARN("Table already exists: " + table_name);
        return false;
    }

    // 创建表元数据
    auto metadata = std::make_shared<TableMetadata>();
    metadata->table_name = table_name;
    metadata->columns = columns;
    
    // 计算记录大小
    metadata->record_size = 0;
    metadata->is_fixed_length = true;
    
    for (size_t i = 0; i < columns.size(); i++) {
        const auto& column = columns[i];
        metadata->column_index_map[column.name] = static_cast<int>(i);
        
        // 检查是否为变长字段
        if (column.type == "VARCHAR" || column.type == "TEXT") {
            metadata->is_fixed_length = false;
            metadata->record_size += sizeof(uint32_t); // 变长字段长度前缀
        } else if (column.type == "INT" || column.type == "INTEGER") {
            metadata->record_size += sizeof(int32_t);
        } else if (column.type == "BIGINT") {
            metadata->record_size += sizeof(int64_t);
        } else if (column.type == "FLOAT") {
            metadata->record_size += sizeof(float);
        } else if (column.type == "DOUBLE") {
            metadata->record_size += sizeof(double);
        } else {
            // 默认当作固定大小处理
            metadata->record_size += column.size;
        }
    }
    
    // 添加记录头部大小
    metadata->record_size += sizeof(RecordHeader);
    
    // 存储元数据
    table_metadata_[table_name] = metadata;
    
    SQLCC_LOG_INFO("Created table: " + table_name + " with " + std::to_string(columns.size()) + " columns");
    return true;
}

bool TableStorageManager::DropTable(const std::string& table_name) {
    // 检查表是否存在
    if (!TableExists(table_name)) {
        SQLCC_LOG_WARN("Table does not exist: " + table_name);
        return false;
    }

    // 移除表元数据
    table_metadata_.erase(table_name);
    
    SQLCC_LOG_INFO("Dropped table: " + table_name);
    return true;
}

bool TableStorageManager::TableExists(const std::string& table_name) const {
    return table_metadata_.find(table_name) != table_metadata_.end();
}

std::shared_ptr<TableMetadata> TableStorageManager::GetTableMetadata(const std::string& table_name) const {
    auto it = table_metadata_.find(table_name);
    if (it != table_metadata_.end()) {
        return it->second;
    }
    return nullptr;
}

bool TableStorageManager::InsertRecord(const std::string& table_name, const std::vector<std::string>& values, 
                                     int32_t& page_id, size_t& offset) {
    // 检查表是否存在
    auto metadata = GetTableMetadata(table_name);
    if (!metadata) {
        SQLCC_LOG_ERROR("Table does not exist: " + table_name);
        return false;
    }

    // 检查列数是否匹配
    if (values.size() != metadata->columns.size()) {
        SQLCC_LOG_ERROR("Column count mismatch for table: " + table_name);
        return false;
    }

    // 分配新页面（简化实现，实际应查找有足够空间的页面）
    Page* page = AllocateNewPage(table_name);
    if (!page) {
        SQLCC_LOG_ERROR("Failed to allocate new page for table: " + table_name);
        return false;
    }

    // 插入记录到页面
    if (!InsertRecordToPage(page, values, offset)) {
        SQLCC_LOG_ERROR("Failed to insert record to page for table: " + table_name);
        return false;
    }

    page_id = page->GetPageId();
    return true;
}

bool TableStorageManager::UpdateRecord(const std::string& table_name, int32_t page_id, size_t offset, 
                                     const std::vector<std::string>& new_values) {
    // 检查表是否存在
    auto metadata = GetTableMetadata(table_name);
    if (!metadata) {
        SQLCC_LOG_ERROR("Table does not exist: " + table_name);
        return false;
    }

    // 获取页面
    Page* page = storage_engine_->FetchPage(page_id);
    if (!page) {
        SQLCC_LOG_ERROR("Failed to fetch page: " + std::to_string(page_id));
        return false;
    }

    // 更新记录
    bool result = UpdateRecordInPage(page, offset, new_values);
    
    // 解除页面固定
    storage_engine_->UnpinPage(page_id, result); // 如果更新成功，则标记为脏页
    
    return result;
}

bool TableStorageManager::DeleteRecord(const std::string& table_name, int32_t page_id, size_t offset) {
    // 检查表是否存在
    auto metadata = GetTableMetadata(table_name);
    if (!metadata) {
        SQLCC_LOG_ERROR("Table does not exist: " + table_name);
        return false;
    }

    // 获取页面
    Page* page = storage_engine_->FetchPage(page_id);
    if (!page) {
        SQLCC_LOG_ERROR("Failed to fetch page: " + std::to_string(page_id));
        return false;
    }

    // 删除记录
    bool result = DeleteRecordInPage(page, offset);
    
    // 解除页面固定
    storage_engine_->UnpinPage(page_id, result); // 如果删除成功，则标记为脏页
    
    return result;
}

std::vector<std::string> TableStorageManager::GetRecord(const std::string& table_name, int32_t page_id, size_t offset) const {
    // 检查表是否存在
    auto metadata = GetTableMetadata(table_name);
    if (!metadata) {
        SQLCC_LOG_ERROR("Table does not exist: " + table_name);
        return {};
    }

    // 获取页面
    Page* page = storage_engine_->FetchPage(page_id);
    if (!page) {
        SQLCC_LOG_ERROR("Failed to fetch page: " + std::to_string(page_id));
        return {};
    }

    // 获取记录
    std::vector<std::string> record = GetRecordFromPage(page, offset);
    
    // 解除页面固定
    storage_engine_->UnpinPage(page_id, false);
    
    return record;
}

std::vector<std::pair<int32_t, size_t>> TableStorageManager::ScanTable(const std::string& table_name) const {
    // 检查表是否存在
    auto metadata = GetTableMetadata(table_name);
    if (!metadata) {
        SQLCC_LOG_ERROR("Table does not exist: " + table_name);
        return {};
    }

    // 简化实现：返回空结果，表示扫描表
    // TODO: 实现完整的表扫描逻辑
    SQLCC_LOG_WARN("ScanTable simplified implementation: returning empty result");
    return {};
}

std::vector<std::vector<std::string>> TableStorageManager::GetRecords(const std::string& table_name, 
                                                                     const std::vector<std::pair<int32_t, size_t>>& locations) const {
    // 检查表是否存在
    auto metadata = GetTableMetadata(table_name);
    if (!metadata) {
        SQLCC_LOG_ERROR("Table does not exist: " + table_name);
        return {};
    }

    std::vector<std::vector<std::string>> records;
    
    // 遍历所有位置，获取记录
    for (const auto& location : locations) {
        int32_t page_id = location.first;
        size_t offset = location.second;
        
        // 获取页面
        Page* page = storage_engine_->FetchPage(page_id);
        if (!page) {
            SQLCC_LOG_ERROR("Failed to fetch page: " + std::to_string(page_id));
            continue;
        }
        
        // 获取记录
        std::vector<std::string> record = GetRecordFromPage(page, offset);
        if (!record.empty()) {
            records.push_back(record);
        }
        
        // 解除页面固定
        storage_engine_->UnpinPage(page_id, false);
    }
    
    return records;
}

Page* TableStorageManager::AllocateNewPage(const std::string& table_name) {
    int32_t page_id;
    Page* page = storage_engine_->NewPage(&page_id);
    if (!page) {
        return nullptr;
    }

    // 初始化页面
    InitializePage(page, table_name);
    
    return page;
}

bool TableStorageManager::InitializePage(Page* page, const std::string& table_name) {
    // 初始化页面头部
    PageHeader header{};
    header.page_type = PageType::TABLE_PAGE;
    header.page_id = page->GetPageId();
    header.prev_page_id = -1;
    header.next_page_id = -1;
    header.free_space_offset = PAGE_HEADER_SIZE;
    header.free_space_size = PAGE_SIZE - PAGE_HEADER_SIZE;
    header.slot_count = 0;
    header.tuple_count = 0;
    
    WritePageHeader(page, header);
    return true;
}

bool TableStorageManager::InsertRecordToPage(Page* page, const std::vector<std::string>& values, size_t& offset) {
    char* data = page->GetData();
    
    // 读取页面头部
    PageHeader header = ReadPageHeader(page);
    
    // 计算记录大小
    // 注意：这里简化处理，实际应根据表元数据计算
    size_t record_size = sizeof(RecordHeader);
    for (const auto& value : values) {
        record_size += sizeof(uint32_t) + value.length(); // 长度前缀 + 数据
    }
    
    // 检查是否有足够空间
    if (header.free_space_size < record_size + SLOT_ARRAY_ENTRY_SIZE) {
        SQLCC_LOG_WARN("Not enough space in page for record insertion");
        return false;
    }
    
    // 计算记录插入位置
    offset = header.free_space_offset;
    
    // 写入记录头部
    RecordHeader record_header{};
    record_header.size = record_size;
    record_header.is_deleted = false;
    record_header.next_free_offset = 0;
    
    memcpy(data + offset, &record_header, sizeof(RecordHeader));
    
    // 写入记录数据
    size_t data_offset = offset + sizeof(RecordHeader);
    for (const auto& value : values) {
        uint32_t len = value.length();
        memcpy(data + data_offset, &len, sizeof(uint32_t));
        data_offset += sizeof(uint32_t);
        
        memcpy(data + data_offset, value.c_str(), len);
        data_offset += len;
    }
    
    // 更新页面头部
    header.free_space_offset += record_size;
    header.free_space_size -= record_size;
    header.slot_count++;
    header.tuple_count++;
    
    WritePageHeader(page, header);
    
    return true;
}

bool TableStorageManager::UpdateRecordInPage(Page* page, size_t offset, const std::vector<std::string>& new_values) {
    char* data = page->GetData();
    
    // 读取记录头部
    RecordHeader record_header;
    memcpy(&record_header, data + offset, sizeof(RecordHeader));
    
    // 标记为删除
    record_header.is_deleted = true;
    memcpy(data + offset, &record_header, sizeof(RecordHeader));
    
    // 插入新记录（简化实现，实际应尝试原地更新）
    size_t new_offset;
    return InsertRecordToPage(page, new_values, new_offset);
}

bool TableStorageManager::DeleteRecordInPage(Page* page, size_t offset) {
    char* data = page->GetData();
    
    // 读取记录头部
    RecordHeader record_header;
    memcpy(&record_header, data + offset, sizeof(RecordHeader));
    
    // 标记为删除
    record_header.is_deleted = true;
    memcpy(data + offset, &record_header, sizeof(RecordHeader));
    
    // 更新页面头部
    PageHeader header = ReadPageHeader(page);
    header.tuple_count--;
    WritePageHeader(page, header);
    
    return true;
}

std::vector<std::string> TableStorageManager::GetRecordFromPage(Page* page, size_t offset) const {
    char* data = page->GetData();
    
    // 读取记录头部
    RecordHeader record_header;
    memcpy(&record_header, data + offset, sizeof(RecordHeader));
    
    // 检查记录是否已被删除
    if (record_header.is_deleted) {
        return {};
    }
    
    // 读取记录数据（简化实现）
    std::vector<std::string> values;
    size_t data_offset = offset + sizeof(RecordHeader);
    
    // 注意：这里需要根据表的实际元数据来解析数据
    // 当前仅为演示目的，简单处理
    while (data_offset < offset + record_header.size) {
        uint32_t len;
        memcpy(&len, data + data_offset, sizeof(uint32_t));
        data_offset += sizeof(uint32_t);
        
        std::string value(data + data_offset, len);
        values.push_back(value);
        data_offset += len;
    }
    
    return values;
}

PageHeader TableStorageManager::ReadPageHeader(Page* page) const {
    char* data = page->GetData();
    PageHeader header;
    
    // 从页面数据中读取头部信息
    memcpy(&header.page_type, data, sizeof(PageType));
    memcpy(&header.page_id, data + sizeof(PageType), sizeof(int32_t));
    memcpy(&header.prev_page_id, data + sizeof(PageType) + sizeof(int32_t), sizeof(int32_t));
    memcpy(&header.next_page_id, data + sizeof(PageType) + 2 * sizeof(int32_t), sizeof(int32_t));
    memcpy(&header.free_space_offset, data + sizeof(PageType) + 3 * sizeof(int32_t), sizeof(uint16_t));
    memcpy(&header.free_space_size, data + sizeof(PageType) + 3 * sizeof(int32_t) + sizeof(uint16_t), sizeof(uint16_t));
    memcpy(&header.slot_count, data + sizeof(PageType) + 3 * sizeof(int32_t) + 2 * sizeof(uint16_t), sizeof(uint16_t));
    memcpy(&header.tuple_count, data + sizeof(PageType) + 3 * sizeof(int32_t) + 3 * sizeof(uint16_t), sizeof(uint16_t));
    
    return header;
}

void TableStorageManager::WritePageHeader(Page* page, const PageHeader& header) const {
    char* data = page->GetData();
    
    // 将头部信息写入页面数据
    memcpy(data, &header.page_type, sizeof(PageType));
    memcpy(data + sizeof(PageType), &header.page_id, sizeof(int32_t));
    memcpy(data + sizeof(PageType) + sizeof(int32_t), &header.prev_page_id, sizeof(int32_t));
    memcpy(data + sizeof(PageType) + 2 * sizeof(int32_t), &header.next_page_id, sizeof(int32_t));
    memcpy(data + sizeof(PageType) + 3 * sizeof(int32_t), &header.free_space_offset, sizeof(uint16_t));
    memcpy(data + sizeof(PageType) + 3 * sizeof(int32_t) + sizeof(uint16_t), &header.free_space_size, sizeof(uint16_t));
    memcpy(data + sizeof(PageType) + 3 * sizeof(int32_t) + 2 * sizeof(uint16_t), &header.slot_count, sizeof(uint16_t));
    memcpy(data + sizeof(PageType) + 3 * sizeof(int32_t) + 3 * sizeof(uint16_t), &header.tuple_count, sizeof(uint16_t));
}

bool TableStorageManager::CreateIndex(const std::string& table_name, const std::string& column_name) {
    // TODO: 需要实现IndexManager类
    SQLCC_LOG_WARN("CreateIndex not implemented: IndexManager class is missing");
    return false;
}

bool TableStorageManager::DropIndex(const std::string& table_name, const std::string& column_name) {
    // TODO: 需要实现IndexManager类
    SQLCC_LOG_WARN("DropIndex not implemented: IndexManager class is missing");
    return false;
}

bool TableStorageManager::IndexExists(const std::string& table_name, const std::string& column_name) const {
    // TODO: 需要实现IndexManager类
    SQLCC_LOG_WARN("IndexExists not implemented: IndexManager class is missing");
    return false;
}

std::shared_ptr<BPlusTreeIndex> TableStorageManager::GetIndex(const std::string& table_name, const std::string& column_name) {
    // TODO: 需要实现IndexManager类
    SQLCC_LOG_WARN("GetIndex not implemented: IndexManager class is missing");
    return nullptr;
}

} // namespace sqlcc