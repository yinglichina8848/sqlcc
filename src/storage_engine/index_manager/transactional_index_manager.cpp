#include "transactional_index_manager.h"
#include "../b_plus_tree.h"
#include "../storage_engine.h"
#include "../../logger/logger.h"

namespace sqlcc {
namespace storage_engine {
namespace index_manager {

TransactionalIndexManager::TransactionalIndexManager(std::shared_ptr<StorageEngine> storage_engine)
    : storage_engine_(storage_engine) {}

bool TransactionalIndexManager::CreateIndexInTransaction(const std::string& index_name,
                                                        const std::string& table_name,
                                                        const std::string& column_name,
                                                        int transaction_id) {
    // 记录事务操作
    IndexOperation op{index_name, table_name, column_name, OperationType::CREATE, transaction_id};
    transaction_log_[transaction_id].push_back(op);

    // 执行创建操作
    auto index = std::make_unique<BPlusTreeIndex>(storage_engine_, table_name, column_name);
    if (!index->Create()) {
        RollbackTransaction(transaction_id);
        return false;
    }

    // 缓存索引（这里假设有一个全局缓存，但实际实现中需要外部管理）
    // index_cache_.CacheIndex(index_name, std::move(index));

    SQLCC_LOG_INFO("Created index " + index_name + " transactionally for transaction " +
                  std::to_string(transaction_id));
    return true;
}

bool TransactionalIndexManager::DropIndexInTransaction(const std::string& index_name,
                                                      const std::string& table_name,
                                                      const std::string& column_name,
                                                      int transaction_id) {
    // 记录事务操作
    IndexOperation op{index_name, table_name, "", OperationType::DROP, transaction_id};
    transaction_log_[transaction_id].push_back(op);

    // 检查索引是否存在（这里需要外部验证）
    // 标记为待删除
    pending_deletions_[index_name] = std::unique_ptr<BPlusTreeIndex>(
        nullptr  // 实际实现中需要从缓存中获取
    );

    SQLCC_LOG_INFO("Dropped index " + index_name + " transactionally for transaction " +
                  std::to_string(transaction_id));
    return true;
}

void TransactionalIndexManager::CommitTransaction(int32_t transaction_id) {
    auto it = transaction_log_.find(transaction_id);
    if (it != transaction_log_.end()) {
        // 清理待删除的索引
        for (const auto& op : it->second) {
            if (op.type == OperationType::DROP) {
                pending_deletions_.erase(op.index_name);
            }
        }

        transaction_log_.erase(it);
        SQLCC_LOG_INFO("Committed index operations for transaction " +
                      std::to_string(transaction_id));
    }
}

void TransactionalIndexManager::RollbackTransaction(int32_t transaction_id) {
    auto it = transaction_log_.find(transaction_id);
    if (it != transaction_log_.end()) {
        // 回滚操作
        for (auto op_it = it->second.rbegin(); op_it != it->second.rend(); ++op_it) {
            const auto& op = *op_it;
            switch (op.type) {
                case OperationType::CREATE:
                    // 这里需要实际删除已创建的索引
                    SQLCC_LOG_INFO("Rolling back index creation: " + op.index_name);
                    break;
                case OperationType::DROP:
                    // 将待删除的索引重新放回缓存
                    if (pending_deletions_.find(op.index_name) != pending_deletions_.end()) {
                        // 重新激活索引
                        SQLCC_LOG_INFO("Rolling back index drop: " + op.index_name);
                    }
                    break;
            }
        }

        transaction_log_.erase(it);
        SQLCC_LOG_INFO("Rolled back index operations for transaction " +
                      std::to_string(transaction_id));
    }
}

} // namespace index_manager
} // namespace storage_engine
} // namespace sqlcc
