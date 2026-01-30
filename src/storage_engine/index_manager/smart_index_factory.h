#pragma once

#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

class StorageEngine;
class TransactionManager;
class BPlusTreeIndex;

namespace storage_engine {

namespace index_manager {

class EnhancedIndexManager;

class SmartIndexFactory {
public:
    SmartIndexFactory();
    ~SmartIndexFactory();

    std::unique_ptr<EnhancedIndexManager> CreateEnhancedIndexManager(
        std::shared_ptr<StorageEngine> storage_engine,
        std::shared_ptr<TransactionManager> transaction_manager);

    std::unique_ptr<BPlusTreeIndex> CreateSmartIndex(
        std::shared_ptr<StorageEngine> storage_engine,
        const std::string& table_name,
        const std::string& column_name);

    std::unique_ptr<BPlusTreeIndex> CreateCompositeIndex(
        std::shared_ptr<StorageEngine> storage_engine,
        const std::string& table_name,
        const std::vector<std::string>& columns);

    std::string RecommendIndexType(const std::string& table_name,
                                    const std::vector<std::string>& columns,
                                    const std::string& data_characteristics);

    std::unique_ptr<BPlusTreeIndex> CreateConfiguredIndex(
        const std::string& config,
        std::shared_ptr<StorageEngine> storage_engine);
};

} // namespace index_manager
} // namespace storage_engine
} // namespace sqlcc
