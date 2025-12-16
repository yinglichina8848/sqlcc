#include "storage_engine.h"
#include "storage/index_manager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace sqlcc {

StorageEngine::StorageEngine(ConfigManager& config_manager, const std::string& db_path) 
    : config_manager_(config_manager), db_path_(db_path) {
    // 创建磁盘管理器
    std::string db_file = db_path_ + "/sqlcc.db";
    disk_manager_ = std::make_unique<DiskManager>(db_file, config_manager_);
    
    // 获取缓冲池配置
    size_t buffer_pool_size = config_manager_.GetInt("buffer.pool.size", 64);
    size_t shard_count = config_manager_.GetInt("buffer.shard.count", 16);
    
    // 创建缓冲池
    buffer_pool_ = std::make_unique<BufferPoolSharded>(std::move(disk_manager_), config_manager_, buffer_pool_size, shard_count);
    
    // 注意：不能在这里创建索引管理器，因为shared_from_this()只能在对象完全构造后使用
    // 索引管理器的创建将延迟到首次访问时
    index_manager_ = nullptr;
}

StorageEngine::~StorageEngine() {
    // 析构函数实现 - 刷新所有页面到磁盘
    if (buffer_pool_) {
        buffer_pool_->FlushAllPages();
    } else {
        // 确保buffer_pool_为空时的安全处理
    }
}

// 添加一个方法来初始化索引管理器
void StorageEngine::InitializeIndexManager() {
    if (!index_manager_) {
        index_manager_ = std::make_unique<IndexManager>(shared_from_this(), config_manager_);
    }
}

std::unique_ptr<Page> StorageEngine::NewPage(int32_t *page_id) {
    // 创建新页面的实现
    if (!buffer_pool_) {
        return nullptr;
    }
    
    int32_t new_page_id;
    auto page_ptr = buffer_pool_->NewPage(&new_page_id);
    
    if (page_id != nullptr) {
        *page_id = new_page_id;
    }
    
    return page_ptr;
}

Page* StorageEngine::FetchPage(int32_t page_id) {
    // 获取页面的实现
    if (!buffer_pool_) {
        return nullptr;
    }
    
    std::unique_ptr<Page> page = buffer_pool_->FetchPage(page_id);
    return page.release();
}

bool StorageEngine::UnpinPage(int32_t page_id, bool is_dirty) {
    // 取消固定页面的实现
    if (!buffer_pool_) {
        return false;
    }
    
    return buffer_pool_->UnpinPage(page_id, is_dirty);
}

bool StorageEngine::FlushPage(int32_t page_id) {
    // 刷新页面到磁盘的实现
    if (!buffer_pool_) {
        return false;
    }
    
    return buffer_pool_->FlushPage(page_id);
}

bool StorageEngine::DeletePage(int32_t page_id) {
    // 删除页面的实现
    if (!buffer_pool_) {
        return false;
    }
    
    return buffer_pool_->DeletePage(page_id);
}

void StorageEngine::FlushAllPages() {
    // 刷新所有页面到磁盘的实现
    if (buffer_pool_) {
        buffer_pool_->FlushAllPages();
    }
}

std::string StorageEngine::GetStats() const {
    // 获取统计信息的实现
    if (!buffer_pool_) {
        return "Storage Engine Stats: Not initialized";
    }
    
    auto stats = buffer_pool_->GetStats();
    std::ostringstream oss;
    oss << "Storage Engine Stats:\n";
    oss << "  Total Accesses: " << stats.at("total_accesses") << "\n";
    oss << "  Total Hits: " << stats.at("total_hits") << "\n";
    oss << "  Hit Rate: " << stats.at("hit_rate") << "\n";
    oss << "  Used Pages: " << stats.at("used_pages") << "\n";
    oss << "  Pool Size: " << stats.at("pool_size");
    
    return oss.str();
}

} // namespace sqlcc
