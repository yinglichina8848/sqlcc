#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include "../b_plus_tree.h"
#include "../../logger/logger.h"

namespace sqlcc {
namespace storage_engine {
namespace index_manager {

class SmartPtrLifetimeManager {
public:
    SmartPtrLifetimeManager() = default;
    ~SmartPtrLifetimeManager();
    
    // RAII guard for index lifetime management
    class IndexLifetimeGuard {
    public:
        IndexLifetimeGuard(std::unique_ptr<BPlusTreeIndex> index,
                          std::function<void(BPlusTreeIndex*)> cleanup = nullptr);
        ~IndexLifetimeGuard();
        
        BPlusTreeIndex* Get() const;
        BPlusTreeIndex* operator->() const;
        
        IndexLifetimeGuard(const IndexLifetimeGuard&) = delete;
        IndexLifetimeGuard& operator=(const IndexLifetimeGuard&) = delete;
        IndexLifetimeGuard(IndexLifetimeGuard&&) = default;
        IndexLifetimeGuard& operator=(IndexLifetimeGuard&&) = default;
        
    private:
        std::unique_ptr<BPlusTreeIndex> index_;
        std::function<void(BPlusTreeIndex*)> cleanup_;
    };
    
    // Ownership management
    std::unique_ptr<BPlusTreeIndex> TransferOwnership(std::shared_ptr<BPlusTreeIndex> shared_index);
    
    // Safe release methods
    void SafeRelease(std::unique_ptr<BPlusTreeIndex>& index);
    void BatchRelease(std::vector<std::unique_ptr<BPlusTreeIndex>>& indexes);
    
    // Prevent copying
    SmartPtrLifetimeManager(const SmartPtrLifetimeManager&) = delete;
    SmartPtrLifetimeManager& operator=(const SmartPtrLifetimeManager&) = delete;
};

} // namespace index_manager
} // namespace storage_engine
} // namespace sqlcc