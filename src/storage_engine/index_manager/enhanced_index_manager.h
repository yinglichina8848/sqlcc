#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <thread>

#include "smart_index_cache.h"

namespace sqlcc {

class StorageEngine;
class TransactionManager;
class BPlusTreeIndex;

namespace storage_engine {
namespace index_manager {

class TransactionalIndexManager;

class EnhancedIndexManager {
public:
    EnhancedIndexManager(std::shared_ptr<StorageEngine> storage_engine,
                        std::shared_ptr<TransactionManager> transaction_manager);
    ~EnhancedIndexManager();
    
    // Index management
    bool CreateIndex(const std::string& index_name,
                    const std::string& table_name,
                    const std::string& column_name,
                    bool transactional = false,
                    int32_t transaction_id = -1);
    
    bool DropIndex(const std::string& index_name,
                  const std::string& table_name,
                  bool transactional = false,
                  int32_t transaction_id = -1);
    
    BPlusTreeIndex* GetIndex(const std::string& index_name);
    bool IndexExists(const std::string& index_name) const;
    std::vector<BPlusTreeIndex*> GetTableIndexes(const std::string& table_name);
    
    // Transaction support
    void CommitTransaction(int32_t transaction_id);
    void RollbackTransaction(int32_t transaction_id);
    
    // Cache management
    EnhancedCacheStats GetCacheStats() const;
    void CleanupExpiredCache(std::chrono::minutes max_age);
    
    // Maintenance
    bool RebuildIndex(const std::string& index_name);
    bool OptimizeIndex(const std::string& index_name);
    
    // Performance monitoring
    std::unordered_map<std::string, double> GetPerformanceStats() const;

private:
    void StartCacheCleanupTimer();
    
    std::shared_ptr<StorageEngine> storage_engine_;
    std::shared_ptr<TransactionManager> transaction_manager_;
    std::unique_ptr<SmartIndexCache> smart_cache_;
    std::unique_ptr<TransactionalIndexManager> tx_manager_;
    std::thread cleanup_thread_;
    bool cleanup_timer_running_ = false;
};

} // namespace index_manager
} // namespace storage_engine
} // namespace sqlcc
