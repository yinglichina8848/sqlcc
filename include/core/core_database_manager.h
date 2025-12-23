#ifndef SQLCC_CORE_DATABASE_MANAGER_H
#define SQLCC_CORE_DATABASE_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace sqlcc {

// 前向声明
class ConfigManager;
class StorageEngine;
class BufferPool;
class TransactionManager;
class IndexManager;
class TableStorage;
class TableMetadata;
struct Page;

using TransactionId = uint64_t;

enum class IsolationLevel {
    READ_UNCOMMITTED,
    READ_COMMITTED,
    REPEATABLE_READ,
    SERIALIZABLE
};

/**
 * @brief 数据库管理器 - 核心服务层组件
 *
 * 负责数据库的创建、删除、管理，以及表的管理操作。
 * 提供高层数据库操作接口，协调各个底层组件。
 */
class DatabaseManager {
public:
    /**
     * @brief 构造函数
     * @param db_path 数据库存储路径
     * @param buffer_pool_size 缓冲池大小
     * @param shard_count 分片数量
     * @param stripe_count 条带数量
     */
    DatabaseManager(const std::string& db_path,
                   size_t buffer_pool_size = 1024 * 1024 * 100, // 100MB
                   size_t shard_count = 4,
                   size_t stripe_count = 8);

    /**
     * @brief 析构函数
     */
    ~DatabaseManager();

    // 数据库管理方法
    bool CreateDatabase(const std::string& db_name);
    bool DropDatabase(const std::string& db_name);
    bool UseDatabase(const std::string& db_name);
    std::vector<std::string> ListDatabases();
    bool DatabaseExists(const std::string& db_name);
    std::string GetCurrentDatabase() const;

    // 表管理方法
    bool CreateTable(const std::string& db_name,
                    const std::string& table_name,
                    const std::vector<std::pair<std::string, std::string>>& columns);
    bool CreateTable(const std::string& table_name,
                    const std::vector<std::pair<std::string, std::string>>& columns);
    bool DropTable(const std::string& table_name);
    bool TableExists(const std::string& table_name);
    std::vector<std::string> ListTables();

    // 事务相关方法
    TransactionId BeginTransaction(IsolationLevel isolation_level = IsolationLevel::READ_COMMITTED);
    bool CommitTransaction(TransactionId txn_id);
    bool RollbackTransaction(TransactionId txn_id);

    // 页面操作方法
    bool ReadPage(TransactionId txn_id, int32_t page_id, Page** page);
    bool WritePage(TransactionId txn_id, int32_t page_id, Page* page);

    // 锁管理方法
    bool LockKey(TransactionId txn_id, const std::string& key);
    bool UnlockKey(TransactionId txn_id, const std::string& key);

    // 系统维护方法
    bool FlushAllPages();
    bool Close();

    // 元数据查询方法
    std::shared_ptr<TableMetadata> GetTableMetadata(const std::string& table_name);

    // 组件访问方法
    std::shared_ptr<IndexManager> GetIndexManager();
    std::shared_ptr<ConfigManager> GetConfig();

    // 初始化和状态检查方法
    bool Initialize();
    bool IsInitialized() const;

    // 测试辅助方法
    bool Execute(const std::string& sql);
    std::vector<std::string> GetTableNames();
    std::string GetTableSchema(const std::string& table_name);

private:
    // 私有辅助方法
    bool LoadTables(const std::string& db_name);

    // 成员变量
    std::string db_path_;                                    // 数据库存储路径
    std::string current_database_;                          // 当前使用的数据库
    bool is_closed_;                                        // 是否已关闭

    // 组件指针
    std::shared_ptr<ConfigManager> config_manager_;         // 配置管理器
    std::shared_ptr<StorageEngine> storage_engine_;         // 存储引擎
    std::shared_ptr<BufferPool> buffer_pool_;               // 缓冲池
    std::shared_ptr<TransactionManager> txn_manager_;       // 事务管理器
    std::shared_ptr<IndexManager> index_manager_;           // 索引管理器

    // 数据结构
    std::unordered_map<std::string, std::vector<std::string>> database_tables_;  // 数据库->表映射
    std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<TableStorage>>> table_storages_;  // 数据库->(表名->存储)

    // 线程安全
    mutable std::mutex mutex_;                              // 保护所有操作的互斥锁

    // 禁用拷贝构造和赋值
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
};

} // namespace sqlcc

#endif // SQLCC_CORE_DATABASE_MANAGER_H
