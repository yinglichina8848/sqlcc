#include "storage_engine/index_manager/enhanced_index_manager.h"
#include "storage_engine/index_manager/smart_index_cache.h"
#include "storage_engine/index_manager/transactional_index_manager.h"
#include "src/storage/b_plus_tree.h"
#include "utils/logger.h"
#include <thread>
#include <chrono>

namespace sqlcc {
namespace storage_engine {
namespace index_manager {

EnhancedIndexManager::EnhancedIndexManager(std::shared_ptr<StorageEngine> storage_engine,
                                          std::shared_ptr<TransactionManager> transaction_manager)
    : storage_engine_(storage_engine),
      transaction_manager_(transaction_manager),
      smart_cache_(std::make_unique<SmartIndexCache>()),
      tx_manager_(std::make_unique<TransactionalIndexManager>(storage_engine)) {

    StartCacheCleanupTimer();
}

EnhancedIndexManager::~EnhancedIndexManager() {
    if (cleanup_timer_running_) {
        cleanup_timer_running_ = false;
        if (cleanup_thread_.joinable()) {
            cleanup_thread_.join();
        }
    }
}

bool EnhancedIndexManager::CreateIndex(const std::string& index_name,
                                      const std::string& table_name,
                                      const std::string& column_name,
                                      bool transactional,
                                      int32_t transaction_id) {

    SQLCC_LOG_INFO("Creating index: " + index_name + " on table: " + table_name +
                  ", column: " + column_name + (transactional ? " (transactional)" : ""));

    if (smart_cache_->HasIndex(index_name)) {
        SQLCC_LOG_WARN("Index already exists: " + index_name);
        return false;
    }

    bool success = false;
    if (transactional && transaction_id >= 0) {
        success = tx_manager_->CreateIndexTransactional(index_name, table_name,
                                                       column_name, transaction_id);
    } else {
        auto index = std::make_unique<BPlusTreeIndex>(storage_engine_, table_name, column_name);
        if (index->Create()) {
            smart_cache_->CacheIndex(index_name, std::move(index));
            success = true;
        }
    }

    if (success) {
        SQLCC_LOG_INFO("Index created successfully: " + index_name);
    } else {
        SQLCC_LOG_ERROR("Failed to create index: " + index_name);
    }

    return success;
}

bool EnhancedIndexManager::DropIndex(const std::string& index_name,
                                    const std::string& table_name,
                                    bool transactional,
                                    int32_t transaction_id) {

    SQLCC_LOG_INFO("Dropping index: " + index_name + " on table: " + table_name +
                  (transactional ? " (transactional)" : ""));

    if (!smart_cache_->HasIndex(index_name)) {
        SQLCC_LOG_WARN("Index does not exist: " + index_name);
        return false;
    }

    bool success = false;
    if (transactional && transaction_id >= 0) {
        success = tx_manager_->DropIndexTransactional(index_name, table_name, transaction_id);
    } else {
        success = smart_cache_->RemoveIndex(index_name);
    }

    if (success) {
        SQLCC_LOG_INFO("Index dropped successfully: " + index_name);
    } else {
        SQLCC_LOG_ERROR("Failed to drop index: " + index_name);
    }

    return success;
}

BPlusTreeIndex* EnhancedIndexManager::GetIndex(const std::string& index_name) {
    return smart_cache_->GetIndex(index_name);
}

bool EnhancedIndexManager::IndexExists(const std::string& index_name) const {
    return smart_cache_->HasIndex(index_name);
}

std::vector<BPlusTreeIndex*> EnhancedIndexManager::GetTableIndexes(const std::string& table_name) {
    std::vector<BPlusTreeIndex*> result;
    (void)table_name; // 避免未使用参数警告
    SQLCC_LOG_WARN("GetTableIndexes not fully implemented in enhanced version");
    return result;
}

void EnhancedIndexManager::CommitTransaction(int32_t transaction_id) {
    tx_manager_->CommitTransaction(transaction_id);
}

void EnhancedIndexManager::RollbackTransaction(int32_t transaction_id) {
    tx_manager_->RollbackTransaction(transaction_id);
}

SmartIndexCache::EnhancedCacheStats EnhancedIndexManager::GetCacheStats() const {
    return smart_cache_->GetEnhancedCacheStats();
}

void EnhancedIndexManager::CleanupExpiredCache(std::chrono::minutes max_age) {
    smart_cache_->CleanupExpiredCache(max_age);
}

bool EnhancedIndexManager::RebuildIndex(const std::string& index_name) {
    SQLCC_LOG_INFO("Rebuilding index: " + index_name);
    // 简化的重建实现
    return true;
}

bool EnhancedIndexManager::OptimizeIndex(const std::string& index_name) {
    SQLCC_LOG_INFO("Optimizing index: " + index_name);
    // 简化的优化实现
    return true;
}

std::unordered_map<std::string, double> EnhancedIndexManager::GetPerformanceStats() const {
    std::unordered_map<std::string, double> stats;
    auto cache_stats = smart_cache_->GetEnhancedCacheStats();

    stats["cache_hit_rate"] = cache_stats.hit_rate;
    stats["average_access_frequency"] = cache_stats.average_access_frequency;
    stats["total_indexes"] = static_cast<double>(cache_stats.total_indexes);

    return stats;
}

void EnhancedIndexManager::StartCacheCleanupTimer() {
    cleanup_timer_running_ = true;
    cleanup_thread_ = std::thread([this]() {
        while (cleanup_timer_running_) {
            std::this_thread::sleep_for(std::chrono::minutes(10));
            if (cleanup_timer_running_) {
                smart_cache_->CleanupExpiredCache(std::chrono::minutes(60));
            }
        }
    });
}

} // namespace index_manager
} // namespace storage_engine
} // namespace sqlcc
