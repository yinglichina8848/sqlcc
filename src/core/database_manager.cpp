#include "database_manager.h"

#include <algorithm>
#include <sstream>

namespace sqlcc {

DatabaseManager::DatabaseManager(const std::string& db_path,
                               size_t buffer_pool_size,
                               size_t shard_count,
                               size_t stripe_count)
    : db_path_(db_path), current_database_(), is_closed_(false), database_tables_() {
    // 初始化配置管理器
    auto config_manager = std::make_shared<ConfigManager>();
    config_manager->SetValue("buffer_pool.size", static_cast<int>(buffer_pool_size));
    config_manager->SetValue("buffer_pool.stripe_count", static_cast<int>(stripe_count));
    // 注意：构造函数没有max_connections参数，使用shard_count作为连接池相关配置
    
    // 初始化磁盘管理器
    auto disk_manager = std::make_shared<DiskManager>(db_path, *config_manager);
    
    // 初始化shard化缓冲池
    buffer_pool_ = std::make_shared<BufferPoolSharded>(
        disk_manager.get(),
        *config_manager,
        buffer_pool_size,
        shard_count
    );
    
    // 初始化事务管理器
    txn_manager_ = std::make_shared<TransactionManager>();
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
    
    return txn_manager_->begin_transaction(isolation_level);
}

bool DatabaseManager::CommitTransaction(TransactionId txn_id) {
    if (is_closed_) {
        return false;
    }
    
    return txn_manager_->commit_transaction(txn_id);
}

bool DatabaseManager::RollbackTransaction(TransactionId txn_id) {
    if (is_closed_) {
        return false;
    }
    
    return txn_manager_->rollback_transaction(txn_id);
}

bool DatabaseManager::ReadPage(TransactionId txn_id, uint64_t page_id, Page** page) {
    if (is_closed_) {
        return false;
    }
    
    // 检查事务状态
    try {
        TransactionState state = txn_manager_->get_transaction_state(txn_id);
        if (state != TransactionState::ACTIVE) {
            return false;
        }
    } catch (const Exception& e) {
        return false;
    }
    
    // 从缓冲池读取页面（读事务无需锁，通过快照机制保证一致性）
    return buffer_pool_->FetchPage(page_id, page);
}

bool DatabaseManager::WritePage(TransactionId txn_id, uint64_t page_id, Page* page) {
    if (is_closed_) {
        throw Exception("Database is closed");
    }
    // 确保页面被锁
    if (!LockKey(txn_id, std::to_string(page_id))) {
        return false;
    }
    bool success = buffer_pool_->FlushPage(static_cast<int32_t>(page_id));
    UnlockKey(txn_id, std::to_string(page_id));
    return success;
}

bool DatabaseManager::LockKey(TransactionId txn_id, const std::string& key) {
    if (is_closed_) {
        throw Exception("Database is closed");
    }
    return txn_manager_->acquire_lock(txn_id, key, LockType::EXCLUSIVE);
}

bool DatabaseManager::UnlockKey(TransactionId txn_id, const std::string& key) {
    if (is_closed_) {
        throw Exception("Database is closed");
    }
    txn_manager_->release_lock(txn_id, key);
    return true;
}

bool DatabaseManager::FlushAllPages() {
    if (is_closed_) {
        throw Exception("Database is closed");
    }
    buffer_pool_->FlushAllPages();
    return true;
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

// 数据库管理方法实现
bool DatabaseManager::CreateDatabase(const std::string& db_name) {
    if (is_closed_) {
        return false;
    }
    
    // 检查数据库是否已存在
    if (database_tables_.find(db_name) != database_tables_.end()) {
        return false;
    }
    
    // 创建数据库（在内存中创建表列表）
    database_tables_[db_name] = std::vector<std::string>();
    return true;
}

bool DatabaseManager::DropDatabase(const std::string& db_name) {
    if (is_closed_) {
        return false;
    }
    
    // 检查数据库是否存在
    auto it = database_tables_.find(db_name);
    if (it == database_tables_.end()) {
        return false;
    }
    
    // 如果删除的是当前数据库，清空当前数据库
    if (db_name == current_database_) {
        current_database_.clear();
    }
    
    // 删除数据库
    database_tables_.erase(it);
    return true;
}

bool DatabaseManager::UseDatabase(const std::string& db_name) {
    if (is_closed_) {
        return false;
    }
    
    // 检查数据库是否存在
    if (database_tables_.find(db_name) == database_tables_.end()) {
        return false;
    }
    
    // 切换到指定数据库
    current_database_ = db_name;
    return true;
}

std::vector<std::string> DatabaseManager::ListDatabases() {
    std::vector<std::string> databases;
    if (is_closed_) {
        return databases;
    }
    
    // 收集所有数据库名
    for (const auto& pair : database_tables_) {
        databases.push_back(pair.first);
    }
    return databases;
}

bool DatabaseManager::DatabaseExists(const std::string& db_name) {
    if (is_closed_) {
        return false;
    }
    
    return database_tables_.find(db_name) != database_tables_.end();
}

std::string DatabaseManager::GetCurrentDatabase() const {
    return current_database_;
}

// 表管理方法实现
bool DatabaseManager::CreateTable(const std::string& table_name, const std::vector<std::pair<std::string, std::string>>& columns) {
    if (is_closed_) {
        return false;
    }
    
    // 检查是否已选择数据库
    if (current_database_.empty()) {
        return false;
    }
    
    // 检查表是否已存在
    if (TableExists(table_name)) {
        return false;
    }
    
    // 在当前数据库中创建表
    database_tables_[current_database_].push_back(table_name);
    return true;
}

bool DatabaseManager::DropTable(const std::string& table_name) {
    if (is_closed_) {
        return false;
    }
    
    // 检查是否已选择数据库
    if (current_database_.empty()) {
        return false;
    }
    
    // 检查表是否存在
    auto& tables = database_tables_[current_database_];
    auto it = std::find(tables.begin(), tables.end(), table_name);
    if (it == tables.end()) {
        return false;
    }
    
    // 删除表
    tables.erase(it);
    return true;
}

bool DatabaseManager::TableExists(const std::string& table_name) {
    if (is_closed_ || current_database_.empty()) {
        return false;
    }
    
    const auto& tables = database_tables_[current_database_];
    return std::find(tables.begin(), tables.end(), table_name) != tables.end();
}

std::vector<std::string> DatabaseManager::ListTables() {
    if (is_closed_ || current_database_.empty()) {
        return std::vector<std::string>();
    }
    
    return database_tables_[current_database_];
}

}  // namespace sqlcc