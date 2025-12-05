#ifndef SQLCC_DATABASE_MANAGER_H
#define SQLCC_DATABASE_MANAGER_H

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "page.h"
#include "sql_executor/index_manager.h"
#include "storage/table_storage.h"
#include "transaction_manager.h"

// 定义TransactionId类型
using TransactionId = uint64_t;

namespace sqlcc {

// 前向声明
class ConfigManager;
class StorageEngine;
class BufferPoolSharded;
class TransactionManager;
class TableStorage;

/**
 * 数据库管理器
 * 集成了shard化BufferPool和striped key锁，提供高并发的缓存和事务支持
 */
class DatabaseManager {
public:
  /**
   * 构造函数
   * @param db_path 数据库路径
   * @param buffer_pool_size 缓冲池大小
   * @param shard_count 分片数量
   * @param stripe_count 条带数量
   */
  DatabaseManager(const std::string &db_path = "./data",
                  size_t buffer_pool_size = 1024, size_t shard_count = 16,
                  size_t stripe_count = 64);

  // 显式析构函数
  ~DatabaseManager();

  /**
   * 数据库管理方法
   */
  bool CreateDatabase(const std::string &db_name);
  bool DropDatabase(const std::string &db_name);
  bool UseDatabase(const std::string &db_name);
  std::vector<std::string> ListDatabases();
  bool DatabaseExists(const std::string &db_name);
  std::string GetCurrentDatabase() const; // 获取当前使用的数据库

  // 表管理方法
  bool
  CreateTable(const std::string &db_name, const std::string &table_name,
              const std::vector<std::pair<std::string, std::string>> &columns);
  bool
  CreateTable(const std::string &table_name,
              const std::vector<std::pair<std::string, std::string>> &columns);
  bool DropTable(const std::string &table_name);
  bool TableExists(const std::string &table_name);
  std::vector<std::string> ListTables();

  // 事务和锁方法
  TransactionId BeginTransaction(
      IsolationLevel isolation_level = IsolationLevel::READ_COMMITTED);
  bool CommitTransaction(TransactionId txn_id);
  bool RollbackTransaction(TransactionId txn_id);

  bool LockKey(TransactionId txn_id, const std::string &key);
  bool UnlockKey(TransactionId txn_id, const std::string &key);

  // 页面读写（简化实现以通过测试）
  bool ReadPage(TransactionId txn_id, int32_t page_id, Page **page);
  bool WritePage(TransactionId txn_id, int32_t page_id, Page *page);

  // 缓冲池方法
  bool FlushAllPages();

  bool Close();

  // 获取存储引擎（用于DML操作）
  std::shared_ptr<StorageEngine> GetStorageEngine() { return storage_engine_; }

  // 获取表元数据（用于索引优化）
  std::shared_ptr<TableMetadata>
  GetTableMetadata(const std::string &table_name);

  // 获取索引管理器（用于索引优化）
  std::shared_ptr<IndexManager> GetIndexManager();

private:
  std::shared_ptr<ConfigManager> config_manager_;   // 配置管理器
  std::shared_ptr<StorageEngine> storage_engine_;   // 存储引擎
  std::shared_ptr<BufferPoolSharded> buffer_pool_;  // shard化缓冲池
  std::shared_ptr<TransactionManager> txn_manager_; // 事务管理器
  std::shared_ptr<IndexManager> index_manager_;     // 索引管理器
  std::string db_path_;                             // 数据库路径
  std::string current_database_;                    // 当前数据库名
  bool is_closed_;                                  // 是否已关闭
  mutable std::mutex mutex_;                        // 线程同步互斥锁

  // 存储数据库和表的元数据
  std::unordered_map<std::string, std::vector<std::string>> database_tables_;

  // 表存储映射
  std::unordered_map<
      std::string,
      std::unordered_map<std::string, std::shared_ptr<TableStorage>>>
      table_storages_;

  // 私有辅助方法
  bool LoadDatabases();
  bool LoadTables(const std::string &db_name);
};

} // namespace sqlcc

#endif // SQLCC_DATABASE_MANAGER_H