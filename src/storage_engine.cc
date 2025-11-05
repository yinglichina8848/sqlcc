#include "storage_engine.h"
#include "logger.h"

namespace sqlcc {

StorageEngine::StorageEngine(const std::string& db_filename, size_t pool_size) {
    SQLCC_LOG_INFO("Initializing StorageEngine with database file: " + db_filename + 
                  " and pool size: " + std::to_string(pool_size));
    
    // 创建磁盘管理器
    disk_manager_ = std::make_unique<DiskManager>(db_filename);
    
    // 创建缓冲池管理器
    buffer_pool_ = std::make_unique<BufferPool>(disk_manager_.get(), pool_size);
    
    SQLCC_LOG_INFO("StorageEngine initialized successfully");
}

StorageEngine::~StorageEngine() {
    SQLCC_LOG_INFO("Destroying StorageEngine");
    // 析构函数会自动清理资源
}

Page* StorageEngine::NewPage(int32_t* page_id) {
    SQLCC_LOG_DEBUG("Creating new page");
    Page* page = buffer_pool_->NewPage(page_id);
    if (page != nullptr) {
        SQLCC_LOG_DEBUG("New page created with ID: " + std::to_string(*page_id));
    } else {
        SQLCC_LOG_ERROR("Failed to create new page");
    }
    return page;
}

Page* StorageEngine::FetchPage(int32_t page_id) {
    SQLCC_LOG_DEBUG("Fetching page ID: " + std::to_string(page_id));
    Page* page = buffer_pool_->FetchPage(page_id);
    if (page != nullptr) {
        SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " fetched successfully");
    } else {
        SQLCC_LOG_WARN("Failed to fetch page ID " + std::to_string(page_id));
    }
    return page;
}

bool StorageEngine::UnpinPage(int32_t page_id, bool is_dirty) {
    SQLCC_LOG_DEBUG("Unpinning page ID: " + std::to_string(page_id) + ", is_dirty: " + std::to_string(is_dirty));
    bool result = buffer_pool_->UnpinPage(page_id, is_dirty);
    if (result) {
        SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " unpinned successfully");
    } else {
        SQLCC_LOG_WARN("Failed to unpin page ID " + std::to_string(page_id));
    }
    return result;
}

bool StorageEngine::FlushPage(int32_t page_id) {
    SQLCC_LOG_DEBUG("Flushing page ID: " + std::to_string(page_id));
    bool result = buffer_pool_->FlushPage(page_id);
    if (result) {
        SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " flushed successfully");
    } else {
        SQLCC_LOG_WARN("Failed to flush page ID " + std::to_string(page_id));
    }
    return result;
}

bool StorageEngine::DeletePage(int32_t page_id) {
    SQLCC_LOG_DEBUG("Deleting page ID: " + std::to_string(page_id));
    bool result = buffer_pool_->DeletePage(page_id);
    if (result) {
        SQLCC_LOG_DEBUG("Page ID " + std::to_string(page_id) + " deleted successfully");
    } else {
        SQLCC_LOG_WARN("Failed to delete page ID " + std::to_string(page_id));
    }
    return result;
}

void StorageEngine::FlushAllPages() {
    SQLCC_LOG_INFO("Flushing all pages");
    buffer_pool_->FlushAllPages();
    SQLCC_LOG_INFO("All pages flushed successfully");
}

}  // namespace sqlcc