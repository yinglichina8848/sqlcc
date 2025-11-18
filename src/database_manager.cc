#include "database_manager.h"

#include <sstream>

namespace sqlcc {

DatabaseManager::DatabaseManager(const std::string& db_path,
                               size_t buffer_pool_size,
                               size_t shard_count,
                               size_t stripe_count)
    : db_path_(db_path), is_closed_(false) {
    // 初始化磁盘管理器
    auto disk_manager = std::make_shared<DiskManager>(db_path);
    
    // 初始化配置管理器
    auto config_manager = std::make_shared<ConfigManager>();
    config_manager->SetBufferPoolSize(buffer_pool_size);
    
    // 初始化shard化缓冲池
    buffer_pool_ = std::make_shared<ShardedBufferPool>(
        disk_manager,
        config_manager,
        buffer_pool_size,
        shard_count
    );
    
    // 初始化事务管理器（使用striped key锁）
    txn_manager_ = std::make_shared<TransactionManager>(stripe_count);
}

DatabaseManager::~DatabaseManager() {
    if (!is_closed_) {
        Close();
    }
}

TransactionId DatabaseManager::BeginTransaction(IsolationLevel isolation_level) {
    if (is_closed_) {
        throw Exception("Database is closed");
    }
    
    return txn_manager_->BeginTransaction(isolation_level);
}

bool DatabaseManager::CommitTransaction(TransactionId txn_id) {
    if (is_closed_) {
        return false;
    }
    
    return txn_manager_->CommitTransaction(txn_id);
}

bool DatabaseManager::RollbackTransaction(TransactionId txn_id) {
    if (is_closed_) {
        return false;
    }
    
    return txn_manager_->RollbackTransaction(txn_id);
}

bool DatabaseManager::ReadPage(TransactionId txn_id, page_id_t page_id, Page** page) {
    if (is_closed_) {
        return false;
    }
    
    // 检查事务状态
    try {
        TransactionStatus status = txn_manager_->GetTransactionStatus(txn_id);
        if (status != TransactionStatus::ACTIVE) {
            return false;
        }
    } catch (const Exception& e) {
        return false;
    }
    
    // 从缓冲池读取页面（读事务无需锁，通过快照机制保证一致性）
    return buffer_pool_->FetchPage(page_id, page);
}

bool DatabaseManager::WritePage(TransactionId txn_id, page_id_t page_id, Page* page) {
    if (is_closed_) {
        return false;
    }
    
    // 检查事务状态
    try {
        TransactionStatus status = txn_manager_->GetTransactionStatus(txn_id);
        if (status != TransactionStatus::ACTIVE) {
            return false;
        }
    } catch (const Exception& e) {
        return false;
    }
    
    // 构造页面ID对应的键
    std::stringstream ss;
    ss << "page_" << page_id;
    std::string page_key = ss.str();
    
    // 先锁定对应的键
    if (!txn_manager_->LockForWrite(txn_id, page_key)) {
        return false;
    }
    
    // 设置页面为脏页
    page->is_dirty_ = true;
    
    // 由于缓冲池已经持有页面，不需要额外操作
    // 页面会在适当的时候被刷新到磁盘
    
    return true;
}

bool DatabaseManager::LockKey(TransactionId txn_id, const std::string& key) {
    if (is_closed_) {
        return false;
    }
    
    return txn_manager_->LockForWrite(txn_id, key);
}

bool DatabaseManager::UnlockKey(TransactionId txn_id, const std::string& key) {
    if (is_closed_) {
        return false;
    }
    
    return txn_manager_->UnlockForWrite(txn_id, key);
}

bool DatabaseManager::FlushAllPages() {
    if (is_closed_) {
        return false;
    }
    
    // 刷新所有脏页到磁盘
    return buffer_pool_->FlushAllPages();
}

bool DatabaseManager::Close() {
    if (is_closed_) {
        return true;
    }
    
    // 刷新所有脏页
    if (!FlushAllPages()) {
        return false;
    }
    
    // 关闭缓冲池
    buffer_pool_.reset();
    
    // 关闭事务管理器
    txn_manager_.reset();
    
    is_closed_ = true;
    return true;
}

}  // namespace sqlcc