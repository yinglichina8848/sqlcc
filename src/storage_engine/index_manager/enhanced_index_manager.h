#pragma once

#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

class StorageEngine;
class TransactionManager;

namespace storage_engine {

namespace index_manager {

class EnhancedIndexManager {
public:
    EnhancedIndexManager(std::shared_ptr<StorageEngine> storage_engine,
                        std::shared_ptr<TransactionManager> transaction_manager);
    ~EnhancedIndexManager();

private:
    std::shared_ptr<StorageEngine> storage_engine_;
    std::shared_ptr<TransactionManager> transaction_manager_;
};

} // namespace index_manager
} // namespace storage_engine
} // namespace sqlcc
