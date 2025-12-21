#pragma once

#include <memory>
#include <string>
#include <vector>

namespace sqlcc {
namespace storage_engine {
namespace index_manager {

// Forward declarations
class StorageEngine;
class TransactionManager;
class EnhancedIndexManager;
class BPlusTreeIndex;

/**
 * @brief 智能索引工厂
 *
 * 提供索引对象的智能创建和管理功能，支持不同类型的索引自动选择和配置化创建。
 */
class SmartIndexFactory {
public:
    /**
     * @brief 创建增强的索引管理器
     * @param storage_engine 存储引擎指针
     * @param transaction_manager 事务管理器指针（可选）
     * @return 增强索引管理器智能指针
     */
    static std::unique_ptr<EnhancedIndexManager> CreateEnhancedIndexManager(
        std::shared_ptr<StorageEngine> storage_engine,
        std::shared_ptr<TransactionManager> transaction_manager = nullptr);

    /**
     * @brief 创建智能指针化的索引对象
     * @param storage_engine 存储引擎指针
     * @param table_name 表名
     * @param column_name 列名
     * @return B+树索引智能指针
     */
    static std::unique_ptr<BPlusTreeIndex> CreateSmartIndex(
        std::shared_ptr<StorageEngine> storage_engine,
        const std::string& table_name,
        const std::string& column_name);

    /**
     * @brief 创建复合索引
     * @param storage_engine 存储引擎指针
     * @param table_name 表名
     * @param columns 列名列表
     * @return B+树索引智能指针
     */
    static std::unique_ptr<BPlusTreeIndex> CreateCompositeIndex(
        std::shared_ptr<StorageEngine> storage_engine,
        const std::string& table_name,
        const std::vector<std::string>& columns);

    /**
     * @brief 智能选择索引类型
     * @param table_name 表名
     * @param columns 列名列表
     * @param data_characteristics 数据特征描述
     * @return 推荐的索引类型
     */
    static std::string RecommendIndexType(const std::string& table_name,
                                        const std::vector<std::string>& columns,
                                        const std::string& data_characteristics);

    /**
     * @brief 创建配置化的索引
     * @param config 索引配置JSON字符串
     * @param storage_engine 存储引擎指针
     * @return B+树索引智能指针
     */
    static std::unique_ptr<BPlusTreeIndex> CreateConfiguredIndex(
        const std::string& config,
        std::shared_ptr<StorageEngine> storage_engine);

private:
    // 禁止实例化
    SmartIndexFactory() = delete;
    SmartIndexFactory(const SmartIndexFactory&) = delete;
    SmartIndexFactory& operator=(const SmartIndexFactory&) = delete;
};

} // namespace index_manager
} // namespace storage_engine
} // namespace sqlcc
