#pragma once

#include <memory>
#include <string>

namespace sqlcc {

class StorageEngine;
class TransactionManager;

namespace storage_engine {

namespace index_manager {

class TransactionalIndexManager {
public:
    TransactionalIndexManager(std::shared_ptr<StorageEngine> storage_engine);
    ~TransactionalIndexManager();

    bool CreateIndexInTransaction(const std::string& table_name,
                                   const std::string& column_name,
                                   int transaction_id);
    bool DropIndexInTransaction(const std::string& table_name,
                                 const std::string& column_name,
                                 int transaction_id);
    bool CommitTransaction(int transaction_id);
    bool RollbackTransaction(int transaction_id);

private:
    std::shared_ptr<StorageEngine> storage_engine_;
};

} // namespace index_manager
} // namespace storage_engine
} // namespace sqlcc
