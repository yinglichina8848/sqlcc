#include "database_manager_mock.h"
#include <sstream>

namespace sqlcc {
namespace mocks {

DatabaseManagerMock::DatabaseManagerMock(const std::string& db_path,
                                       size_t buffer_pool_size,
                                       size_t shard_count,
                                       size_t stripe_count)
    : DatabaseManager(db_path, buffer_pool_size, shard_count, stripe_count) {
    RecordCall("DatabaseManagerMock", {db_path, std::to_string(buffer_pool_size),
                                      std::to_string(shard_count), std::to_string(stripe_count)});
}

DatabaseManagerMock::~DatabaseManagerMock() {
    RecordCall("~DatabaseManagerMock");
}

// Mock配置方法实现
void DatabaseManagerMock::SetCreateDatabaseResult(bool success) {
    create_database_success_ = success;
}

void DatabaseManagerMock::SetDropDatabaseResult(bool success) {
    drop_database_success_ = success;
}

void DatabaseManagerMock::SetUseDatabaseResult(bool success) {
    use_database_success_ = success;
}

void DatabaseManagerMock::SetListDatabasesResult(const std::vector<std::string>& databases) {
    list_databases_result_ = databases;
}

void DatabaseManagerMock::SetDatabaseExistsResult(bool exists) {
    database_exists_result_ = exists;
}

void DatabaseManagerMock::SetCurrentDatabaseResult(const std::string& db_name) {
    current_database_result_ = db_name;
}

void DatabaseManagerMock::SetCreateTableResult(bool success) {
    create_table_success_ = success;
}

void DatabaseManagerMock::SetDropTableResult(bool success) {
    drop_table_success_ = success;
}

void DatabaseManagerMock::SetTableExistsResult(bool exists) {
    table_exists_result_ = exists;
}

void DatabaseManagerMock::SetListTablesResult(const std::vector<std::string>& tables) {
    list_tables_result_ = tables;
}

void DatabaseManagerMock::SetBeginTransactionResult(TransactionId txn_id) {
    begin_transaction_result_ = txn_id;
}

void DatabaseManagerMock::SetCommitTransactionResult(bool success) {
    commit_transaction_success_ = success;
}

void DatabaseManagerMock::SetRollbackTransactionResult(bool success) {
    rollback_transaction_success_ = success;
}

void DatabaseManagerMock::SetReadPageResult(bool success) {
    read_page_success_ = success;
}

void DatabaseManagerMock::SetWritePageResult(bool success) {
    write_page_success_ = success;
}

void DatabaseManagerMock::SetLockKeyResult(bool success) {
    lock_key_success_ = success;
}

void DatabaseManagerMock::SetUnlockKeyResult(bool success) {
    unlock_key_success_ = success;
}

void DatabaseManagerMock::SetFlushAllPagesResult(bool success) {
    flush_all_pages_success_ = success;
}

void DatabaseManagerMock::SetCloseResult(bool success) {
    close_success_ = success;
}

void DatabaseManagerMock::SetGetTableMetadataResult(std::shared_ptr<TableMetadata> metadata) {
    table_metadata_result_ = metadata;
}

void DatabaseManagerMock::SetInitializeResult(bool success) {
    initialize_success_ = success;
}

void DatabaseManagerMock::SetIsInitializedResult(bool initialized) {
    is_initialized_result_ = initialized;
}

void DatabaseManagerMock::SetExecuteResult(bool success) {
    execute_success_ = success;
}

void DatabaseManagerMock::SetGetTableNamesResult(const std::vector<std::string>& table_names) {
    get_table_names_result_ = table_names;
}

void DatabaseManagerMock::SetGetTableSchemaResult(const std::string& schema) {
    get_table_schema_result_ = schema;
}

// 重写DatabaseManager接口方法
bool DatabaseManagerMock::CreateDatabase(const std::string& db_name) {
    RecordCall("CreateDatabase", {db_name});
    return create_database_success_;
}

bool DatabaseManagerMock::DropDatabase(const std::string& db_name) {
    RecordCall("DropDatabase", {db_name});
    return drop_database_success_;
}

bool DatabaseManagerMock::UseDatabase(const std::string& db_name) {
    RecordCall("UseDatabase", {db_name});
    return use_database_success_;
}

std::vector<std::string> DatabaseManagerMock::ListDatabases() {
    RecordCall("ListDatabases");
    return list_databases_result_;
}

bool DatabaseManagerMock::DatabaseExists(const std::string& db_name) {
    RecordCall("DatabaseExists", {db_name});
    return database_exists_result_;
}

std::string DatabaseManagerMock::GetCurrentDatabase() const {
    RecordCall("GetCurrentDatabase");
    return current_database_result_;
}

bool DatabaseManagerMock::CreateTable(const std::string& db_name,
                                    const std::string& table_name,
                                    const std::vector<std::pair<std::string, std::string>>& columns) {
    RecordCall("CreateTable", {db_name, table_name, std::to_string(columns.size()) + " columns"});
    return create_table_success_;
}

bool DatabaseManagerMock::CreateTable(const std::string& table_name,
                                    const std::vector<std::pair<std::string, std::string>>& columns) {
    RecordCall("CreateTable", {table_name, std::to_string(columns.size()) + " columns"});
    return create_table_success_;
}

bool DatabaseManagerMock::DropTable(const std::string& table_name) {
    RecordCall("DropTable", {table_name});
    return drop_table_success_;
}

bool DatabaseManagerMock::TableExists(const std::string& table_name) {
    RecordCall("TableExists", {table_name});
    return table_exists_result_;
}

std::vector<std::string> DatabaseManagerMock::ListTables() {
    RecordCall("ListTables");
    return list_tables_result_;
}

TransactionId DatabaseManagerMock::BeginTransaction(IsolationLevel isolation_level) {
    RecordCall("BeginTransaction", {std::to_string(static_cast<int>(isolation_level))});
    return begin_transaction_result_;
}

bool DatabaseManagerMock::CommitTransaction(TransactionId txn_id) {
    RecordCall("CommitTransaction", {std::to_string(txn_id)});
    return commit_transaction_success_;
}

bool DatabaseManagerMock::RollbackTransaction(TransactionId txn_id) {
    RecordCall("RollbackTransaction", {std::to_string(txn_id)});
    return rollback_transaction_success_;
}

bool DatabaseManagerMock::ReadPage(TransactionId txn_id, int32_t page_id, Page** page) {
    RecordCall("ReadPage", {std::to_string(txn_id), std::to_string(page_id)});
    if (read_page_success_ && page) {
        *page = nullptr; // Mock实现不创建实际页面
    }
    return read_page_success_;
}

bool DatabaseManagerMock::WritePage(TransactionId txn_id, int32_t page_id, Page* page) {
    RecordCall("WritePage", {std::to_string(txn_id), std::to_string(page_id)});
    (void)page; // 避免未使用参数警告
    return write_page_success_;
}

bool DatabaseManagerMock::LockKey(TransactionId txn_id, const std::string& key) {
    RecordCall("LockKey", {std::to_string(txn_id), key});
    return lock_key_success_;
}

bool DatabaseManagerMock::UnlockKey(TransactionId txn_id, const std::string& key) {
    RecordCall("UnlockKey", {std::to_string(txn_id), key});
    return unlock_key_success_;
}

bool DatabaseManagerMock::FlushAllPages() {
    RecordCall("FlushAllPages");
    return flush_all_pages_success_;
}

bool DatabaseManagerMock::Close() {
    RecordCall("Close");
    return close_success_;
}

std::shared_ptr<TableMetadata> DatabaseManagerMock::GetTableMetadata(const std::string& table_name) {
    RecordCall("GetTableMetadata", {table_name});
    return table_metadata_result_;
}

std::shared_ptr<IndexManager> DatabaseManagerMock::GetIndexManager() {
    RecordCall("GetIndexManager");
    return nullptr; // Mock实现返回nullptr
}

std::shared_ptr<ConfigManager> DatabaseManagerMock::GetConfig() {
    RecordCall("GetConfig");
    return nullptr; // Mock实现返回nullptr
}

bool DatabaseManagerMock::Initialize() {
    RecordCall("Initialize");
    return initialize_success_;
}

bool DatabaseManagerMock::IsInitialized() const {
    RecordCall("IsInitialized");
    return is_initialized_result_;
}

bool DatabaseManagerMock::Execute(const std::string& sql) {
    RecordCall("Execute", {sql.substr(0, 50) + (sql.length() > 50 ? "..." : "")});
    return execute_success_;
}

std::vector<std::string> DatabaseManagerMock::GetTableNames() {
    RecordCall("GetTableNames");
    return get_table_names_result_;
}

std::string DatabaseManagerMock::GetTableSchema(const std::string& table_name) {
    RecordCall("GetTableSchema", {table_name});
    return get_table_schema_result_;
}

void DatabaseManagerMock::RecordCall(const std::string& method, const std::vector<std::string>& args) const {
    CallRecord record;
    record.method_name = method;

    std::stringstream ss;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << args[i];
    }
    record.args = {ss.str()};

    call_history_.push_back(record);
}

} // namespace mocks
} // namespace sqlcc