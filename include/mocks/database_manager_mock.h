#pragma once

#include "core/core_database_manager.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace sqlcc {
namespace mocks {

/**
 * @brief DatabaseManager Mock类，用于单元测试
 *
 * 提供可配置的DatabaseManager接口实现，支持：
 * - 模拟数据库和表操作的成功/失败
 * - 记录方法调用历史
 * - 自定义返回值
 * - 验证调用参数
 */
class DatabaseManagerMock : public DatabaseManager {
public:
    /**
     * @brief 构造函数
     * @param db_path 数据库存储路径
     * @param buffer_pool_size 缓冲池大小
     * @param shard_count 分片数量
     * @param stripe_count 条带数量
     */
    DatabaseManagerMock(const std::string& db_path,
                       size_t buffer_pool_size = 1024 * 1024 * 100,
                       size_t shard_count = 4,
                       size_t stripe_count = 8);

    /**
     * @brief 析构函数
     */
    ~DatabaseManagerMock() override;

    // 禁止拷贝
    DatabaseManagerMock(const DatabaseManagerMock&) = delete;
    DatabaseManagerMock& operator=(const DatabaseManagerMock&) = delete;

    // Mock配置方法
    void SetCreateDatabaseResult(bool success);
    void SetDropDatabaseResult(bool success);
    void SetUseDatabaseResult(bool success);
    void SetListDatabasesResult(const std::vector<std::string>& databases);
    void SetDatabaseExistsResult(bool exists);
    void SetCurrentDatabaseResult(const std::string& db_name);
    void SetCreateTableResult(bool success);
    void SetDropTableResult(bool success);
    void SetTableExistsResult(bool exists);
    void SetListTablesResult(const std::vector<std::string>& tables);
    void SetBeginTransactionResult(TransactionId txn_id);
    void SetCommitTransactionResult(bool success);
    void SetRollbackTransactionResult(bool success);
    void SetReadPageResult(bool success);
    void SetWritePageResult(bool success);
    void SetLockKeyResult(bool success);
    void SetUnlockKeyResult(bool success);
    void SetFlushAllPagesResult(bool success);
    void SetCloseResult(bool success);
    void SetGetTableMetadataResult(std::shared_ptr<TableMetadata> metadata);
    void SetInitializeResult(bool success);
    void SetIsInitializedResult(bool initialized);
    void SetExecuteResult(bool success);
    void SetGetTableNamesResult(const std::vector<std::string>& table_names);
    void SetGetTableSchemaResult(const std::string& schema);

    // 调用历史记录
    struct CallRecord {
        std::string method_name;
        std::vector<std::string> args;
    };

    const std::vector<CallRecord>& GetCallHistory() const { return call_history_; }
    void ClearCallHistory() { call_history_.clear(); }

    // 重写DatabaseManager接口方法
    bool CreateDatabase(const std::string& db_name) override;
    bool DropDatabase(const std::string& db_name) override;
    bool UseDatabase(const std::string& db_name) override;
    std::vector<std::string> ListDatabases() override;
    bool DatabaseExists(const std::string& db_name) override;
    std::string GetCurrentDatabase() const override;
    bool CreateTable(const std::string& db_name,
                    const std::string& table_name,
                    const std::vector<std::pair<std::string, std::string>>& columns) override;
    bool CreateTable(const std::string& table_name,
                    const std::vector<std::pair<std::string, std::string>>& columns) override;
    bool DropTable(const std::string& table_name) override;
    bool TableExists(const std::string& table_name) override;
    std::vector<std::string> ListTables() override;
    TransactionId BeginTransaction(IsolationLevel isolation_level = IsolationLevel::READ_COMMITTED) override;
    bool CommitTransaction(TransactionId txn_id) override;
    bool RollbackTransaction(TransactionId txn_id) override;
    bool ReadPage(TransactionId txn_id, int32_t page_id, Page** page) override;
    bool WritePage(TransactionId txn_id, int32_t page_id, Page* page) override;
    bool LockKey(TransactionId txn_id, const std::string& key) override;
    bool UnlockKey(TransactionId txn_id, const std::string& key) override;
    bool FlushAllPages() override;
    bool Close() override;
    std::shared_ptr<TableMetadata> GetTableMetadata(const std::string& table_name) override;
    std::shared_ptr<IndexManager> GetIndexManager() override;
    std::shared_ptr<ConfigManager> GetConfig() override;
    bool Initialize() override;
    bool IsInitialized() const override;
    bool Execute(const std::string& sql) override;
    std::vector<std::string> GetTableNames() override;
    std::string GetTableSchema(const std::string& table_name) override;

private:
    void RecordCall(const std::string& method, const std::vector<std::string>& args = {});

    // Mock配置
    bool create_database_success_ = true;
    bool drop_database_success_ = true;
    bool use_database_success_ = true;
    std::vector<std::string> list_databases_result_;
    bool database_exists_result_ = false;
    std::string current_database_result_;
    bool create_table_success_ = true;
    bool drop_table_success_ = true;
    bool table_exists_result_ = false;
    std::vector<std::string> list_tables_result_;
    TransactionId begin_transaction_result_ = 1;
    bool commit_transaction_success_ = true;
    bool rollback_transaction_success_ = true;
    bool read_page_success_ = true;
    bool write_page_success_ = true;
    bool lock_key_success_ = true;
    bool unlock_key_success_ = true;
    bool flush_all_pages_success_ = true;
    bool close_success_ = true;
    std::shared_ptr<TableMetadata> table_metadata_result_;
    bool initialize_success_ = true;
    bool is_initialized_result_ = true;
    bool execute_success_ = true;
    std::vector<std::string> get_table_names_result_;
    std::string get_table_schema_result_;

    // 调用历史
    std::vector<CallRecord> call_history_;
};

} // namespace mocks
} // namespace sqlcc

#endif // SQLCC_MOCKS_DATABASE_MANAGER_MOCK_H