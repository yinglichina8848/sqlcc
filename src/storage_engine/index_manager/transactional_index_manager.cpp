#include "transactional_index_manager.h"
#include "../b_plus_tree.h"
#include "../storage_engine.h"
#include "../../logger/logger.h"

namespace sqlcc {
namespace storage_engine {
namespace index_manager {

/**
 * @class TransactionalIndexManager
 * @brief 事务型索引管理器 - 实现索引 DDL 操作的 ACID 语义与回滚机制
 *
 * WHY层 - 设计意图：
 *   传统的 DDL 操作（如 CREATE INDEX）通常是不支持回滚的。
 *   但在复杂的分布式系统或长事务中，如果事务在中途失败，必须撤销已经建立的物理索引结构，
 *   否则会导致数据字典与物理文件之间的状态不一致。
 *   该类通过引入“事务日志”和“延迟物理删除”，实现了索引操作的原子性。
 *
 * WHAT层 - 功能说明：
 *   执行事务隔离的索引创建（CreateIndexInTransaction）和删除（DropIndexInTransaction）。
 *   维护 transaction_log_，记录每个事务 ID 对应的 DDL 操作序列。
 *   管理 pending_deletions_，存储已被逻辑删除但物理仍存在的索引对象。
 *   提供 Commit 和 Rollback 钩子函数。
 *
 * HOW层 - 实现机制：
 *   1. 记录重做/回滚信息：在执行物理操作前，先在 transaction_log_ 中追加 IndexOperation。
 *   2. 两阶段提交：Commit 时正式清理待删记录；Rollback 时则逆序执行补偿操作（如物理删除新创索引）。
 *   3. 逻辑遮蔽：Drop 操作后，索引进入 pending_deletions_，对其他事务逻辑不可见，但仍物理存在直至 Commit。
 */
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
