#pragma once

#include <memory>
#include <string>

namespace sqlcc {

class StorageEngine;
class TransactionManager;

namespace storage_engine {

namespace index_manager {

class IndexManager {
public:
    IndexManager(std::shared_ptr<StorageEngine> storage_engine,
                 std::shared_ptr<TransactionManager> transaction_manager);
    ~IndexManager();

    bool CreateIndex(const std::string& table_name, const std::string& column_name);
    bool DropIndex(const std::string& table_name, const std::string& column_name);

private:
    std::shared_ptr<StorageEngine> storage_engine_;
    std::shared_ptr<TransactionManager> transaction_manager_;
};

} // namespace index_manager
} // namespace storage_engine
} // namespace sqlcc
