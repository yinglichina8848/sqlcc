#include "storage_engine/index_manager/smart_index_factory.h"
#include "storage_engine/index_manager/enhanced_index_manager.h"
#include "storage/b_plus_tree.h"
#include "utils/logger.h"

namespace sqlcc {
namespace storage_engine {
namespace index_manager {

std::unique_ptr<EnhancedIndexManager> SmartIndexFactory::CreateEnhancedIndexManager(
    std::shared_ptr<StorageEngine> storage_engine,
    std::shared_ptr<TransactionManager> transaction_manager) {

    return std::make_unique<EnhancedIndexManager>(storage_engine, transaction_manager);
}

std::unique_ptr<BPlusTreeIndex> SmartIndexFactory::CreateSmartIndex(
    std::shared_ptr<StorageEngine> storage_engine,
    const std::string& table_name,
    const std::string& column_name) {

    return std::make_unique<BPlusTreeIndex>(storage_engine, table_name, column_name);
}

std::unique_ptr<BPlusTreeIndex> SmartIndexFactory::CreateCompositeIndex(
    std::shared_ptr<StorageEngine> storage_engine,
    const std::string& table_name,
    const std::vector<std::string>& columns) {

    if (!columns.empty()) {
        return CreateSmartIndex(storage_engine, table_name, columns[0]);
    }
    return nullptr;
}

std::string SmartIndexFactory::RecommendIndexType(const std::string& table_name,
                                                const std::vector<std::string>& columns,
                                                const std::string& data_characteristics) {

    if (columns.size() > 1) {
        return "composite_index";
    }

    if (data_characteristics.find("unique") != std::string::npos) {
        return "unique_index";
    }

    if (data_characteristics.find("text") != std::string::npos) {
        return "text_index";
    }

    return "btree_index";
}

std::unique_ptr<BPlusTreeIndex> SmartIndexFactory::CreateConfiguredIndex(
    const std::string& config,
    std::shared_ptr<StorageEngine> storage_engine) {

    // 简化的配置解析实现
    // 实际应该使用JSON解析器
    if (config.find("composite") != std::string::npos) {
        return CreateCompositeIndex(storage_engine, "default_table", {"column1", "column2"});
    }

    return CreateSmartIndex(storage_engine, "default_table", "default_column");
}

} // namespace index_manager
} // namespace storage_engine
} // namespace sqlcc
