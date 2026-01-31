#include "smart_ptr_lifetime_manager.h"
#include "../b_plus_tree.h"
#include "../../logger/logger.h"

namespace sqlcc {
namespace storage_engine {
namespace index_manager {

SmartPtrLifetimeManager::IndexLifetimeGuard::IndexLifetimeGuard(
    std::unique_ptr<BPlusTreeIndex> index,
    std::function<void(BPlusTreeIndex*)> cleanup)
    : index_(std::move(index)), cleanup_(cleanup) {}

SmartPtrLifetimeManager::IndexLifetimeGuard::~IndexLifetimeGuard() {
    if (cleanup_ && index_) {
        cleanup_(index_.get());
    }
}

BPlusTreeIndex* SmartPtrLifetimeManager::IndexLifetimeGuard::Get() const {
    return index_.get();
}

BPlusTreeIndex* SmartPtrLifetimeManager::IndexLifetimeGuard::operator->() const {
    return index_.get();
}

std::unique_ptr<BPlusTreeIndex> SmartPtrLifetimeManager::TransferOwnership(
    std::shared_ptr<BPlusTreeIndex> shared_index) {

    SQLCC_LOG_WARN("Transferring ownership from shared_ptr to unique_ptr - potential double deletion risk");
    return std::unique_ptr<BPlusTreeIndex>(shared_index.get());
}

void SmartPtrLifetimeManager::SafeRelease(std::unique_ptr<BPlusTreeIndex>& index) {
    if (index) {
        SQLCC_LOG_DEBUG("Safely releasing index: " +
                       (index->GetTableName().empty() ? "unknown" : index->GetTableName()) +
                       "." +
                       (index->GetColumnName().empty() ? "unknown" : index->GetColumnName()));
        index.reset();
    }
}

void SmartPtrLifetimeManager::BatchRelease(std::vector<std::unique_ptr<BPlusTreeIndex>>& indexes) {
    SQLCC_LOG_DEBUG("Batch releasing " + std::to_string(indexes.size()) + " indexes");
    for (auto& index : indexes) {
        SafeRelease(index);
    }
    indexes.clear();
}

} // namespace index_manager
} // namespace storage_engine
} // namespace sqlcc
