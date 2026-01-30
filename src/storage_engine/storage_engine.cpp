/**
 * Storage Engine Implementation - Clang 20 + C++20
 *
 * This file provides the implementation for the StorageEngine class
 * defined in storage_engine.h.
 */

#include "src/storage_engine/storage_engine.h"
#include "src/storage_engine/index_manager/index_manager.h"

namespace sqlcc {

StorageEngine::StorageEngine(ConfigManager &config_manager, const std::string& db_path)
    : config_manager_(config_manager),
      db_path_(db_path),
      disk_manager_(std::make_unique<DiskManager>(db_path)),
      buffer_pool_(std::make_unique<BufferPoolSharded>(config_manager)),
      index_manager_(nullptr) {
    // Initialize disk manager and buffer pool
    disk_manager_->Initialize();
    buffer_pool_->Initialize();
}

StorageEngine::~StorageEngine() {
    // Flush all pages to disk before shutdown
    FlushAllPages();
    
    // Cleanup resources
    if (buffer_pool_) {
        buffer_pool_->Shutdown();
    }
    
    if (disk_manager_) {
        disk_manager_->Shutdown();
    }
}

void StorageEngine::InitializeIndexManager() {
    if (!index_manager_) {
        index_manager_ = std::make_unique<IndexManager>(*this);
    }
}

std::unique_ptr<Page> StorageEngine::NewPage(int32_t *page_id) {
    return buffer_pool_->NewPage(page_id);
}

std::shared_ptr<Page> StorageEngine::FetchPage(int32_t page_id) {
    return buffer_pool_->FetchPage(page_id);
}

bool StorageEngine::UnpinPage(int32_t page_id, bool is_dirty) {
    return buffer_pool_->UnpinPage(page_id, is_dirty);
}

bool StorageEngine::FlushPage(int32_t page_id) {
    return buffer_pool_->FlushPage(page_id);
}

bool StorageEngine::DeletePage(int32_t page_id) {
    return buffer_pool_->DeletePage(page_id);
}

void StorageEngine::FlushAllPages() {
    if (buffer_pool_) {
        buffer_pool_->FlushAllPages();
    }
}

std::string StorageEngine::GetStats() const {
    std::string stats = "Storage Engine Stats:\n";
    stats += disk_manager_->GetStats();
    stats += buffer_pool_->GetStats();
    return stats;
}

} // namespace sqlcc