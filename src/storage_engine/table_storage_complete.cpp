#include "storage/table_storage.h"
#include "storage/b_plus_tree.h"
#include "storage_engine.h"
#include "storage/index_manager.h"
#include "utils/logger.h"
#include <algorithm>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>
#include <utility>
#include <limits>
#include <mutex>
#include <cmath>

namespace sqlcc {

// 页面RAII管理器类 - 实现安全的页面生命周期管理
class PageRAII {
public:
    PageRAII(Page* page, std::shared_ptr<StorageEngine> storage_engine, int32_t page_id)
        : page_(page), storage_engine_(std::move(storage_engine)), page_id_(page_id), pinned_(true) {
        if (!page_) {
            throw std::invalid_argument("Page cannot be null");
        }
        if (!storage_engine_) {
            throw std::invalid_argument("StorageEngine cannot be null");
        }
        if (page_id < 0) {
            throw std::invalid_argument("Page ID must be non-negative");
        }
    }

    ~PageRAII() {
        if (pinned_ && storage_engine_ && page_id_ >= 0) {
            try {
                storage_engine_->UnpinPage(page_id_, false);
            } catch (const std::exception& e) {
                SQLCC_LOG_ERROR("Failed to unpin page " + std::to_string(page_id_) + 
                               " during RAII cleanup: " + std::string(e.what()));
            }
        }
    }

    // 禁止拷贝，允许移动
    PageRAII(const PageRAII&) = delete;
    PageRAII& operator=(const PageRAII&) = delete;
    
    PageRAII(PageRAII&& other) noexcept 
        : page_(other.page_), storage_engine_(std::move(other.storage_engine_)), 
          page_id_(other.page_id_), pinned_(other.pinned_) {
        other.page_ = nullptr;
        other.pinned_ = false;
    }
    
    PageRAII& operator=(PageRAII&& other) noexcept {
        if (this != &other) {
            // 清理当前资源
            if (pinned_ && storage_engine_ && page_id_ >= 0) {
                storage_engine_->UnpinPage(page_id_, false);
            }
            
            // 移动新资源
            page_ = other.page_;
            storage_engine_ = std::move(other.storage_engine_);
            page_id_ = other.page_id_;
            pinned_ = other.pinned_;
            
            other.page_ = nullptr;
            other.pinned_ = false;
        }
        return *this;
    }

    Page* Get() const { return page_; }
    Page& operator*() const { return *page_; }
    Page* operator->() const { return page_; }
    int32_t GetPageId() const { return page_id_; }
    
    // 安全的数据访问
    char* GetData() {
        if (!page_) {
            throw std::runtime_error("Page is null");
        }
        return page_->GetData();
    }
    
    const char* GetData() const {
        if (!page_) {
            throw std::runtime_error("Page is null");
        }
        return page_->GetData();
    }
    
    void Unpin(bool is_dirty) {
        if (pinned_ && storage_engine_ && page_id_ >= 0) {
            storage_engine_->UnpinPage(page_id_, is_dirty);
            pinned_ = false;
        }
    }

private:
    Page* page_;
    std::shared_ptr<StorageEngine> storage_engine_;
    int32_t page_id_;
    bool pinned_;
};

// 记录安全验证类
class RecordValidator {
public:
    // 验证记录大小限制
    static bool ValidateRecordSize(size_t record_size, size_t max_record_size = 65536) {
        if (record_size == 0) {
            SQLCC_LOG_ERROR("Record size cannot be zero");
            return false;
        }
        if (record_size > max_record_size) {
            SQLCC_LOG_ERROR("Record size " + std::to_string(record_size) + 
                         " exceeds maximum allowed size " + std::to_string(max_record_size));
            return false;
        }
        return true;
    }
    
    // 验证字段类型边界
    static bool ValidateFieldValue(const std::string& field_name, const std::string& field_type, 
                               const std::string& value) {
        if (field_name.empty()) {
            SQLCC_LOG_ERROR("Field name cannot be empty");
            return false;
        }
        
        // 根据字段类型验证值
        if (field_type == "INT" || field_type == "INTEGER") {
            try {
                // 验证整数值范围
                long long int_value = std::stoll(value);
                if (int_value < INT32_MIN || int_value > INT32_MAX) {
                    SQLCC_LOG_ERROR("Integer value " + value + " out of range for field " + field_name);
                    return false;
                }
            } catch (const std::exception& e) {
                SQLCC_LOG_ERROR("Invalid integer value '" + value + "' for field " + field_name + ": " + e.what());
                return false;
            }
        } else if (field_type == "BIGINT") {
            try {
                // 验证长整数值范围
                long long bigint_value = std::stoll(value);
                if (bigint_value < INT64_MIN || bigint_value > INT64_MAX) {
                    SQLCC_LOG_ERROR("Bigint value " + value + " out of range for field " + field_name);
                    return false;
                }
            } catch (const std::exception& e) {
                SQLCC_LOG_ERROR("Invalid bigint value '" + value + "' for field " + field_name + ": " + e.what());
                return false;
            }
        } else if (field_type == "FLOAT" || field_type == "DOUBLE") {
            try {
                // 验证浮点数值
                double double_value = std::stod(value);
                if (std::isnan(double_value) || std::isinf(double_value)) {
                    SQLCC_LOG_ERROR("Invalid floating point value '" + value + "' for field " + field_name);
                    return false;
                }
            } catch (const std::exception& e) {
                SQLCC_LOG_ERROR("Invalid floating point value '" + value + "' for field " + field_name + ": " + e.what());
                return false;
            }
        } else if (field_type == "VARCHAR" || field_type == "TEXT") {
            // 验证变长字段
            if (value.length() > 65535) {
                SQLCC_LOG_ERROR("VARCHAR/TEXT value length " + std::to_string(value.length()) + 
                             " exceeds maximum for field " + field_name);
                return false;
            }
        }
        
        // 检查空字符和不可打印字符
        for (size_t i = 0; i < value.length(); ++i) {
            char c = value[i];
            if (c == '\0') {
                SQLCC_LOG_ERROR("Null character not allowed in field " + field_name);
                return false;
            }
            // 可以添加更多不可打印字符的检查
        }
        
        return true;
    }
    
    // 验证数据完整性约束
    static bool ValidateDataIntegrity(const std::vector<std::string>& field_names,
                                  const std::vector<std::string>& field_types,
                                  const std::vector<std::string>& values) {
        if (field_names.size() != field_types.size() || field_types.size() != values.size()) {
            SQLCC_LOG_ERROR("Field count mismatch: names=" + std::to_string(field_names.size()) +
                         ", types=" + std::to_string(field_types.size()) +
                         ", values=" + std::to_string(values.size()));
            return false;
        }
        
        // 验证每个字段
        for (size_t i = 0; i < values.size(); ++i) {
            if (!ValidateFieldValue(field_names[i], field_types[i], values[i])) {
                return false;
            }
        }
        
        return true;
    }
};

TableStorageManager::TableStorageManager(std::shared_ptr<StorageEngine> storage_engine)
    : storage_engine_(std::move(storage_engine)) {
    // 增强的参数验证
    if (!storage_engine_) {
        throw std::invalid_argument("StorageEngine cannot be null");
    }
    
    try {
        // 初始化索引管理器
        ConfigManager config_manager;
        index_manager_ = std::make_shared<IndexManager>(storage_engine_, config_manager);
        SQLCC_LOG_INFO("TableStorageManager initialized successfully");
    } catch (const std::exception& e) {
        SQLCC_LOG_ERROR("Failed to initialize TableStorageManager: " + std::string(e.what()));
        throw;
    }
}

TableStorageManager::~TableStorageManager() {}

bool TableStorageManager::CreateTable(const std::string &table_name,
                                      const std::vector<TableColumn> &columns) {
    // 增强的参数验证
    if (table_name.empty()) {
        SQLCC_LOG_ERROR("Table name cannot be empty");
        return false;
    }
    
    if (columns.empty()) {
        SQLCC_LOG_ERROR("Table must have at least one column: " + table_name);
        return false;
    }
    
    // 检查表是否已存在
    if (TableExists(table_name)) {
        SQLCC_LOG_WARN("Table already exists: " + table_name);
        return false;
    }

    // 创建表元数据
    auto metadata = std::make_shared<TableMetadata>();
    metadata->table_name = table_name;
    metadata->columns = columns;

    // 计算记录大小并验证
    metadata->record_size = 0;
    metadata->is_fixed_length = true;

    for (size_t i = 0; i < columns.size(); i++) {
        const auto &column = columns[i];
        
        // 验证列名
        if (column.name.empty()) {
            SQLCC_LOG_ERROR("Column name cannot be empty at index " + std::to_string(i));
            return false;
        }
        
        // 检查重复列名
        if (metadata->column_index_map.find(column.name) != metadata->column_index_map.end()) {
            SQLCC_LOG_ERROR("Duplicate column name: " + column.name);
            return false;
        }
        
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
            // 默认当作固定大小处理，但需要验证大小
            if (column.size == 0) {
                SQLCC_LOG_ERROR("Invalid column size for column: " + column.name);
                return false;
            }
            metadata->record_size += column.size;
        }
        
        // 验证总记录大小不会溢出
        if (metadata->record_size > std::numeric_limits<uint32_t>::max()) {
            SQLCC_LOG_ERROR("Record size exceeds maximum for table: " + table_name);
            return false;
        }
    }

    // 添加记录头部大小
    metadata->record_size += sizeof(RecordHeader);

    // 存储元数据
    table_metadata_[table_name] = metadata;

    SQLCC_LOG_INFO("Created table: " + table_name + " with " +
                   std::to_string(columns.size()) + " columns");
    return true;
}

bool TableStorageManager::DropTable(const std::string &table_name) {
    // 增强的参数验证
    if (table_name.empty()) {
        SQLCC_LOG_ERROR("Table name cannot be empty");
        return false;
    }
    
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

bool TableStorageManager::TableExists(const std::string &table_name) const {
    if (table_name.empty()) {
        return false;
    }
    return table_metadata_.find(table_name) != table_metadata_.end();
}

std::shared_ptr<TableMetadata>
TableStorageManager::GetTableMetadata(const std::string &table_name) const {
    if (table_name.empty()) {
        return nullptr;
    }
    auto it = table_metadata_.find(table_name);
    if (it != table_metadata_.end()) {
        return it->second;
    }
    return nullptr;
}

bool TableStorageManager::InsertRecord(const std::string &table_name,
                                       const std::vector<std::string> &values,
                                       int32_t &page_id, size_t &offset) {
    try {
        // 增强的参数验证
        if (table_name.empty()) {
            SQLCC_LOG_ERROR("Table name cannot be empty");
            return false;
        }
        
        if (values.empty()) {
            SQLCC_LOG_ERROR("Values cannot be empty for table: " + table_name);
            return false;
        }

        // 检查表是否存在
        auto metadata = GetTableMetadata(table_name);
        if (!metadata) {
            SQLCC_LOG_ERROR("Table does not exist: " + table_name);
            return false;
        }

        // 检查列数是否匹配
        if (values.size() != metadata->columns.size()) {
            SQLCC_LOG_ERROR("Column count mismatch for table: " + table_name + 
                         " (expected " + std::to_string(metadata->columns.size()) + 
                         ", got " + std::to_string(values.size()) + ")");
            return false;
        }
        
        // 验证数据完整性约束
        std::vector<std::string> field_names;
        std::vector<std::string> field_types;
        for (const auto& column : metadata->columns) {
            field_names.push_back(column.name);
            field_types.push_back(column.type);
        }
        
        if (!RecordValidator::ValidateDataIntegrity(field_names, field_types, values)) {
            return false;
        }

        // 分配新页面（简化实现，实际应查找有足够空间的页面）
        auto page = AllocateNewPage(table_name);
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
        
    } catch (const std::exception& e) {
        SQLCC_LOG_ERROR("Exception in InsertRecord for table " + table_name + ": " + std::string(e.what()));
        return false;
    }
}

bool TableStorageManager::UpdateRecord(
    const std::string &table_name, int32_t page_id, size_t offset,
    const std::vector<std::string> &new_values) {
    try {
        // 增强的参数验证
        if (table_name.empty()) {
            SQLCC_LOG_ERROR("Table name cannot be empty");
            return false;
        }
        
        if (page_id < 0) {
            SQLCC_LOG_ERROR("Invalid page ID: " + std::to_string(page_id));
            return false;
        }
        
        if (offset >= PAGE_SIZE) {
            SQLCC_LOG_ERROR("Invalid offset: " + std::to_string(offset) + " (max: " + std::to_string(PAGE_SIZE) + ")");
            return false;
        }
        
        if (new_values.empty()) {
            SQLCC_LOG_ERROR("New values cannot be empty for table: " + table_name);
            return false;
        }

        // 检查表是否存在
        auto metadata = GetTableMetadata(table_name);
        if (!metadata) {
            SQLCC_LOG_ERROR("Table does not exist: " + table_name);
            return false;
        }
        
        // 检查列数是否匹配
        if (new_values.size() != metadata->columns.size()) {
            SQLCC_LOG_ERROR("Column count mismatch for table: " + table_name);
            return false;
        }

        // 获取页面
        Page *page = storage_engine_->FetchPage(page_id);
        if (!page) {
            SQLCC_LOG_ERROR("Failed to fetch page: " + std::to_string(page_id));
            return false;
        }

        // 更新记录
        bool result = UpdateRecordInPage(page, offset, new_values);

        // 解除页面固定
        storage_engine_->UnpinPage(page_id, result); // 如果更新成功，则标记为脏页

        return result;
        
    } catch (const std::exception& e) {
        SQLCC_LOG_ERROR("Exception in UpdateRecord for table " + table_name + ": " + std::string(e.what()));
        return false;
    }
}

bool TableStorageManager::DeleteRecord(const std::string &table_name,
                                       int32_t page_id, size_t offset) {
    try {
        // 增强的参数验证
        if (table_name.empty()) {
            SQLCC_LOG_ERROR("Table name cannot be empty");
            return false;
        }
        
        if (page_id < 0) {
            SQLCC_LOG_ERROR("Invalid page ID: " + std::to_string(page_id));
            return false;
        }
        
        if (offset >= PAGE_SIZE) {
            SQLCC_LOG_ERROR("Invalid offset: " + std::to_string(offset) + " (max: " + std::to_string(PAGE_SIZE) + ")");
            return false;
        }

        // 检查表是否存在
        auto metadata = GetTableMetadata(table_name);
        if (!metadata) {
            SQLCC_LOG_ERROR("Table does not exist: " + table_name);
            return false;
        }

        // 获取页面
        Page *page = storage_engine_->FetchPage(page_id);
        if (!page) {
            SQLCC_LOG_ERROR("Failed to fetch page: " + std::to_string(page_id));
            return false;
        }

        // 删除记录
        bool result = DeleteRecordInPage(page, offset);

        // 解除页面固定
        storage_engine_->UnpinPage(page_id, result); // 如果删除
