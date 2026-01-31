#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace sqlcc {

class StorageEngine;
class TransactionManager;

namespace storage_engine {

namespace index_manager {

// 索引操作类型
enum class OperationType {
    CREATE,
    DROP
};

// 索引操作记录
struct IndexOperation {
    std::string index_name;
    std::string table_name;
    std::string column_name;
    OperationType type;
    int32_t transaction_id;
};

#include "../b_plus_tree/index/b_plus_tree_index.h"

using namespace sqlcc;

class TransactionalIndexManager {
public:
    TransactionalIndexManager(std::shared_ptr<StorageEngine> storage_engine);
    ~TransactionalIndexManager();

    bool CreateIndexInTransaction(const std::string& index_name,
                                   const std::string& table_name,
                                   const std::string& column_name,
                                   int transaction_id);
    bool DropIndexInTransaction(const std::string& index_name,
                                 const std::string& table_name,
                                 const std::string& column_name,
                                 int transaction_id);
    void CommitTransaction(int32_t transaction_id);
    void RollbackTransaction(int32_t transaction_id);

private:
    std::shared_ptr<StorageEngine> storage_engine_;
    std::map<int32_t, std::vector<IndexOperation>> transaction_log_;
    std::map<std::string, std::unique_ptr<BPlusTreeIndex>> pending_deletions_;
};

} // namespace index_manager
} // namespace storage_engine
} // namespace sqlcc
