#ifndef SQLCC_CORE_DATABASE_MANAGER_H
#define SQLCC_CORE_DATABASE_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <map>
#include "../storage_engine/buffer_pool/buffer_pool_sharded.h"

namespace sqlcc {

// 前向声明
class DatabaseFileManager;
class ConfigManager;
class StorageEngine;
class BufferPool;
class TransactionManager;
class IndexManager;
class TableStorage;
struct TableMetadata;
class Page;

using TransactionId = uint64_t;

enum class IsolationLevel {
    READ_UNCOMMITTED,
    READ_COMMITTED,
    REPEATABLE_READ,
    SERIALIZABLE
};

/**
 * WHY: 为什么需要核心数据库管理器？
 *
 * 数据库系统需要统一的入口来管理所有数据库操作：
 * - 数据库生命周期：创建、删除、使用数据库
 * - 表操作：表的创建、删除、查询操作
 * - 事务控制：事务的开始、提交、回滚
 * - 并发访问：多事务的并发执行控制
 * - 资源协调：存储引擎、缓冲池等组件协调
 *
 * 🏗️ 设计模式：外观模式(Facade Pattern)
 * - 统一数据库操作接口
 * - 隐藏底层组件复杂性
 * - 简化客户端使用
 *
 * WHAT: 核心数据库管理器 - 数据库系统的统一管理接口
 *
 * 核心功能：
 * - 数据库管理：创建、删除、使用数据库
 * - 表操作：表的CRUD操作
 * - 事务管理：事务生命周期控制
 * - 并发控制：锁管理和隔离级别
 * - 资源管理：存储和缓冲区管理
 *
 * HOW: 实现机制
 * - 组件协调：管理各个底层组件的生命周期
 * - 线程安全：使用互斥锁保护共享状态
 * - 资源管理：智能指针管理组件生命周期
 * - 错误处理：统一的错误处理和状态管理
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

    // 索引管理方法
    bool CreateIndex(const std::string& index_name,
                    const std::string& table_name,
                    const std::vector<std::string>& columns,
                    bool unique = false, const std::string& condition = "");
    bool DropIndex(const std::string& index_name);

    // 视图管理方法
    bool CreateView(const std::string& view_name,
                   const std::string& query,
                   bool with_check_option = false);
    bool DropView(const std::string& view_name);

    // ALTER TABLE方法
    bool AlterTableAddColumn(const std::string& table_name,
                           const std::string& column_name,
                           const std::string& column_type,
                           const std::string& constraints = "");
    bool AlterTableDropColumn(const std::string& table_name,
                             const std::string& column_name);
    bool AlterTableModifyColumn(const std::string& table_name,
                               const std::string& column_name,
                               const std::string& new_type);
    bool AlterTableRenameColumn(const std::string& table_name,
                               const std::string& old_name,
                               const std::string& new_name);

    // 约束管理方法
    bool AddConstraint(const std::string& table_name,
                      const std::string& constraint_name,
                      const std::string& constraint_type,
                      const std::vector<std::string>& columns,
                      const std::string& expression = "");

    // 数据操作方法（基础实现）
    bool InsertRecord(const std::string& table_name,
                     const std::vector<std::string>& values);
    bool UpdateRecords(const std::string& table_name,
                      const std::map<std::string, std::string>& updates,
                      const std::string& condition = "");
    bool DeleteRecords(const std::string& table_name,
                      const std::string& condition = "");

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
    std::shared_ptr<StorageEngine> GetStorageEngine();
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
    std::shared_ptr<DatabaseFileManager> db_file_manager_;  // 数据库文件管理器

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