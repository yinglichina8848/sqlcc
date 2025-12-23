#include "core/core_database_manager.h"
#include "storage/index_manager.h"
#include "storage/buffer_pool.h"
#include "storage/buffer_pool_sharded.h"
#include "storage/table_storage.h"
#include "storage_engine.h"
#include "utils/config_manager.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

// 确保正确包含spdlog
#ifdef USE_SPDLOG
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#endif

namespace fs = std::filesystem;

namespace sqlcc {

DatabaseManager::DatabaseManager(const std::string &db_path,
                                 size_t buffer_pool_size, size_t shard_count,
                                 size_t stripe_count)
    : db_path_(db_path), current_database_(""), is_closed_(false),
      config_manager_(nullptr), storage_engine_(nullptr), buffer_pool_(nullptr),
      txn_manager_(nullptr), index_manager_(nullptr) {

  // 确保数据库目录存在
  fs::create_directories(db_path_);

  // 初始化关键组件
  try {
    // 创建配置管理器
    config_manager_ = std::make_shared<ConfigManager>();

    // 创建存储引擎，传递数据库路径
    storage_engine_ = std::make_shared<StorageEngine>(*config_manager_, db_path_);

    // 创建索引管理器
    index_manager_ =
        std::make_shared<IndexManager>(storage_engine_, *config_manager_);

    // TODO: 事务管理器暂未实现
    // txn_manager_ = std::make_shared<TransactionManager>();

  } catch (const std::exception &e) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("Failed to initialize DatabaseManager components: {}",
                 e.what());
#endif
    // 如果初始化失败，确保所有指针为nullptr
    config_manager_ = nullptr;
    storage_engine_ = nullptr;
    index_manager_ = nullptr;
  }

#ifdef USE_SPDLOG
  try {
    // 检查logger是否已经存在，避免重复创建
    auto console_logger = spdlog::get("console");
    if (!console_logger) {
      console_logger = spdlog::stdout_color_mt("console");
      spdlog::set_default_logger(console_logger);
    }

    auto file_logger = spdlog::get("file_logger");
    if (!file_logger) {
      // 确保日志目录存在
      fs::create_directories("logs");
      file_logger =
          spdlog::basic_logger_mt("file_logger", "logs/database_manager.log");
    }

    spdlog::set_level(spdlog::level::info);
    SPDLOG_INFO("DatabaseManager initialized with db_path={}, "
                "buffer_pool_size={}, shard_count={}, stripe_count={}",
                db_path, buffer_pool_size, shard_count, stripe_count);
  } catch (const spdlog::spdlog_ex &ex) {
    std::cerr << "Log initialization failed: " << ex.what() << std::endl;
  }
#endif
}

DatabaseManager::~DatabaseManager() { Close(); }

// 数据库管理方法
bool DatabaseManager::CreateDatabase(const std::string &db_name) {
  std::lock_guard<std::mutex> lock(mutex_);

  // 检查数据库是否已存在
  if (database_tables_.find(db_name) != database_tables_.end()) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("Database {} already exists", db_name);
#endif
    return false;
  }

  try {
    // 创建数据库目录
    std::string db_path = db_path_ + "/" + db_name;
    if (!std::filesystem::exists(db_path)) {
      std::filesystem::create_directories(db_path);
    }

    // 初始化数据库元数据
    database_tables_[db_name] = {};

    // 直接创建系统表（避免递归锁）
    database_tables_[db_name].push_back("__tables__");
    std::string table_file_path = db_path + "/__tables__.table";
    std::ofstream file(table_file_path);
    file.close();

#ifdef USE_SPDLOG
    SPDLOG_INFO("Created database: {}", db_name);
#endif
    return true;
  } catch (const std::exception &e) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("Failed to create database {}: {}", db_name, e.what());
#endif
    return false;
  }
}

bool DatabaseManager::DropDatabase(const std::string &db_name) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = database_tables_.find(db_name);
  if (it == database_tables_.end()) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("Database {} does not exist", db_name);
#endif
    return false;
  }

  // 不能删除当前使用的数据库
  if (current_database_ == db_name) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("Cannot drop current database: {}", db_name);
#endif
    return false;
  }

  try {
    std::string db_full_path = db_path_ + "/" + db_name;
    fs::remove_all(db_full_path);
    database_tables_.erase(it);

    // 清理表存储
    table_storages_.erase(db_name);

#ifdef USE_SPDLOG
    SPDLOG_INFO("Dropped database: {}", db_name);
#endif
    return true;
  } catch (const std::exception &e) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("Failed to drop database {}: {}", db_name, e.what());
#endif
    return false;
  }
}

bool DatabaseManager::UseDatabase(const std::string &db_name) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (database_tables_.find(db_name) == database_tables_.end()) {
    // 尝试加载数据库
    std::string db_full_path = db_path_ + "/" + db_name;
    if (!fs::exists(db_full_path)) {
#ifdef USE_SPDLOG
      SPDLOG_ERROR("Database {} does not exist", db_name);
#endif
      return false;
    }
    database_tables_[db_name] = {}; // 初始化空的表列表

    // 直接加载表（避免递归锁）
    try {
      std::string db_path = db_path_ + "/" + db_name;
      if (fs::exists(db_path)) {
        for (const auto &entry : fs::directory_iterator(db_path)) {
          if (entry.is_regular_file() && entry.path().extension() == ".table") {
            std::string table_name = entry.path().stem().string();
            database_tables_[db_name].push_back(table_name);
          }
        }
      }
    } catch (const std::exception &e) {
#ifdef USE_SPDLOG
      SPDLOG_ERROR("Failed to load tables for database {}: {}", db_name,
                   e.what());
#endif
    }
  }

  current_database_ = db_name;

#ifdef USE_SPDLOG
  SPDLOG_INFO("Switched to database: {}", db_name);
#endif
  return true;
}

std::vector<std::string> DatabaseManager::ListDatabases() {
  std::lock_guard<std::mutex> lock(mutex_);
  std::vector<std::string> db_names;
  for (const auto &pair : database_tables_) {
    db_names.push_back(pair.first);
  }
  return db_names;
}

bool DatabaseManager::DatabaseExists(const std::string &db_name) {
  if (is_closed_)
    return false;

  std::string db_dir = db_path_ + "/" + db_name;
  return fs::exists(db_dir);
}

std::string DatabaseManager::GetCurrentDatabase() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return current_database_;
}

// 表管理方法
bool DatabaseManager::CreateTable(
    const std::string &db_name, const std::string &table_name,
    const std::vector<std::pair<std::string, std::string>> &columns) {
  std::lock_guard<std::mutex> lock(mutex_);

  // 检查数据库是否存在
  if (database_tables_.find(db_name) == database_tables_.end()) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("Database {} does not exist", db_name);
#endif
    return false;
  }

  // 检查表是否已存在
  auto &tables = database_tables_[db_name];
  if (std::find(tables.begin(), tables.end(), table_name) != tables.end()) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("Table {} already exists in database {}", table_name, db_name);
#endif
    return false;
  }

  try {
    // 创建表文件并写入元数据
    std::string table_file_path =
        db_path_ + "/" + db_name + "/" + table_name + ".table";
    std::ofstream table_file(table_file_path);
    if (!table_file) {
#ifdef USE_SPDLOG
      SPDLOG_ERROR("Failed to create table file: {}", table_file_path);
#endif
      return false;
    }

    // 写入表元数据（简单的JSON格式）
    table_file << "{\"table_name\":\"" << table_name << "\",";
    table_file << "\"columns\":[";
    for (size_t i = 0; i < columns.size(); ++i) {
      if (i > 0)
        table_file << ",";
      table_file << "{\"name\":\"" << columns[i].first << "\",";
      table_file << "\"type\":\"" << columns[i].second << "\"}";
    }
    table_file << "],\"rows\":[]}" << std::endl;
    table_file.close();

    // 将表添加到数据库表列表中
    tables.push_back(table_name);

#ifdef USE_SPDLOG
    SPDLOG_INFO("Created table: {} in database: {}", table_name, db_name);
#endif
    return true;
  } catch (const std::exception &e) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("Failed to create table {} in database {}: {}", table_name,
                 db_name, e.what());
#endif
    return false;
  }
}

bool DatabaseManager::CreateTable(
    const std::string &table_name,
    const std::vector<std::pair<std::string, std::string>> &columns) {
  return CreateTable(current_database_, table_name, columns);
}

bool DatabaseManager::DropTable(const std::string &table_name) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (current_database_.empty()) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("No database selected");
#endif
    return false;
  }

  auto &tables = database_tables_[current_database_];
  auto it = std::find(tables.begin(), tables.end(), table_name);
  if (it == tables.end()) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("Table {} does not exist in database {}", table_name,
                 current_database_);
#endif
    return false;
  }

  try {
    // 删除表文件
    std::string table_file_path =
        db_path_ + "/" + current_database_ + "/" + table_name + ".table";
    fs::remove(table_file_path);

    // 从列表中移除
    tables.erase(it);

    // 从表存储中移除
    table_storages_[current_database_].erase(table_name);

#ifdef USE_SPDLOG
    SPDLOG_INFO("Dropped table {} from database {}", table_name,
                current_database_);
#endif
    return true;
  } catch (const std::exception &e) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("Failed to drop table {} from database {}: {}", table_name,
                 current_database_, e.what());
#endif
    return false;
  }
}

bool DatabaseManager::TableExists(const std::string &table_name) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (current_database_.empty()) {
    return false;
  }

  auto &tables = database_tables_[current_database_];
  return std::find(tables.begin(), tables.end(), table_name) != tables.end();
}

std::vector<std::string> DatabaseManager::ListTables() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (current_database_.empty()) {
    return {};
  }

  return database_tables_[current_database_];
}

// 事务相关方法
// TODO: 这些方法需要在database_manager.h中声明
TransactionId
DatabaseManager::BeginTransaction(IsolationLevel isolation_level) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (is_closed_) {
    throw std::runtime_error("DatabaseManager is closed");
  }
  static std::atomic<TransactionId> next_id{1};
  TransactionId id = next_id++;
  // TODO: 集成实际TransactionManager逻辑
  return id;
}

bool DatabaseManager::CommitTransaction(TransactionId txn_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (is_closed_) {
    return false;
  }
  // TODO: 集成实际TransactionManager逻辑
  (void)txn_id; // 标记参数为已使用
  return true;
}

bool DatabaseManager::RollbackTransaction(TransactionId txn_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (is_closed_) {
    return false;
  }
  // TODO: 集成实际TransactionManager逻辑
  (void)txn_id; // 标记参数为已使用
  return true;
}

bool DatabaseManager::ReadPage(TransactionId txn_id, int32_t page_id,
                               Page **page) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (is_closed_ || !buffer_pool_)
    return false;
  (void)txn_id; // 标记参数为已使用
  
  // 检查buffer_pool_是否已初始化
  if (!buffer_pool_) {
    *page = nullptr;
    return false;
  }

  // 获取页面的智能指针
  auto page_ptr = buffer_pool_->FetchPage(page_id);
  if (!page_ptr) {
    *page = nullptr;
    return false;
  }
  
  // 注意：这里我们不能直接返回智能指针拥有的原始指针，
  // 因为当page_ptr销毁时，页面也会被销毁。
  // 在实际实现中，应该重新设计API以正确处理智能指针。
  // 这里为了兼容现有API，我们简单地返回nullptr表示失败。
  *page = nullptr;
  return false;
}

bool DatabaseManager::WritePage(TransactionId txn_id, int32_t page_id,
                                Page *page) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (is_closed_ || !buffer_pool_)
    return false;
  (void)txn_id; // 标记参数为已使用
  (void)page;   // 标记参数为已使用

  // 再次检查buffer_pool_是否有效，避免空指针解引用
  if (!buffer_pool_) {
    return false;
  }

  buffer_pool_->UnpinPage(page_id, true);
  return true;
}

bool DatabaseManager::LockKey(TransactionId txn_id, const std::string &key) {
  // 这里应该实现实际的锁机制
  // 暂时返回true表示成功
  (void)txn_id; // 标记参数为已使用
  (void)key;    // 标记参数为已使用
  return true;
}

bool DatabaseManager::UnlockKey(TransactionId txn_id, const std::string &key) {
  // 这里应该实现实际的解锁机制
  // 暂时返回true表示成功
  (void)txn_id; // 标记参数为已使用
  (void)key;    // 标记参数为已使用
  return true;
}

bool DatabaseManager::FlushAllPages() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (is_closed_) {
    throw std::runtime_error("DatabaseManager is closed");
  }

  // buffer_pool_->FlushAllPages();
  return true;
}

bool DatabaseManager::Close() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (is_closed_) {
    return true;
  }

  try {
    // 关闭所有表存储
    for (auto &db_pair : table_storages_) {
      for (auto &table_pair : db_pair.second) {
        // table_pair.second->Close();
        (void)table_pair; // 标记变量为已使用
      }
    }

    table_storages_.clear();
    database_tables_.clear();

    if (buffer_pool_) {
      // buffer_pool_->Close();
    }

    if (storage_engine_) {
      // storage_engine_->Close();
    }

    is_closed_ = true;

#ifdef USE_SPDLOG
    SPDLOG_INFO("DatabaseManager closed successfully");
#endif
    return true;
  } catch (const std::exception &e) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("Error closing DatabaseManager: {}", e.what());
#endif
    return false;
  }
}

// LoadDatabases方法已按新风格调整，保持与上面一致

bool DatabaseManager::LoadTables(const std::string &db_name) {
  // 注意：此方法现在不再需要锁，因为它只在已加锁的UseDatabase中调用
  // 但为了兼容性保留，如果单独调用需要加锁
  try {
    std::string db_path = db_path_ + "/" + db_name;
    if (fs::exists(db_path)) {
      for (const auto &entry : fs::directory_iterator(db_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".table") {
          std::string table_name = entry.path().stem().string();
          database_tables_[db_name].push_back(table_name);
        }
      }
    }
    return true;
  } catch (const std::exception &e) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("Failed to load tables for database {}: {}", db_name,
                 e.what());
#endif
    return false;
  }
}

// 获取表元数据（用于索引优化）
std::shared_ptr<sqlcc::TableMetadata>
sqlcc::DatabaseManager::GetTableMetadata(const std::string &table_name) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (current_database_.empty()) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("No database selected");
#endif
    return nullptr;
  }

  // 检查表是否存在
  if (!TableExists(table_name)) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("Table {} does not exist in database {}", table_name,
                 current_database_);
#endif
    return nullptr;
  }

  // 简化实现：返回一个空的TableMetadata对象
  // 在实际实现中，这里应该从表文件中加载元数据
  auto metadata = std::make_shared<TableMetadata>();
  metadata->table_name = table_name;
  metadata->database_name = current_database_;

  // 添加一些示例列信息
  metadata->columns.push_back({"id", "INT", sizeof(int), false, ""});
  metadata->columns.push_back({"name", "VARCHAR(50)", 50, true, ""});

  return metadata;
}

// 获取索引管理器（用于索引优化）
std::shared_ptr<sqlcc::IndexManager> sqlcc::DatabaseManager::GetIndexManager() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (is_closed_) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("DatabaseManager is closed");
#endif
    return nullptr;
  }

  // 如果索引管理器尚未初始化，则创建它
  if (!index_manager_) {
    // 如果配置管理器不存在，创建一个
    if (!config_manager_) {
      config_manager_ = std::make_shared<ConfigManager>();
    }

    // 如果存储引擎不存在，创建一个存储引擎（需要ConfigManager引用）
    if (!storage_engine_) {
      storage_engine_ = std::make_shared<StorageEngine>(*config_manager_);
    }

    // 创建索引管理器
    index_manager_ =
        std::make_shared<IndexManager>(storage_engine_, *config_manager_);

#ifdef USE_SPDLOG
    SPDLOG_INFO("IndexManager initialized");
#endif
  }

  return index_manager_;
}

// 初始化方法（用于测试）
bool sqlcc::DatabaseManager::Initialize() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (is_closed_) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("DatabaseManager is closed, cannot initialize");
#endif
    return false;
  }

  // 确保关键组件已初始化
  if (!config_manager_) {
    config_manager_ = std::make_shared<ConfigManager>();
  }

  if (!storage_engine_) {
    storage_engine_ = std::make_shared<StorageEngine>(*config_manager_);
  }

  if (!index_manager_) {
    index_manager_ =
        std::make_shared<IndexManager>(storage_engine_, *config_manager_);
  }

#ifdef USE_SPDLOG
  SPDLOG_INFO("DatabaseManager initialized successfully");
#endif
  return true;
}

// 获取配置管理器（用于测试）
std::shared_ptr<sqlcc::ConfigManager> sqlcc::DatabaseManager::GetConfig() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (!config_manager_) {
    config_manager_ = std::make_shared<ConfigManager>();
  }

  return config_manager_;
}

// 检查是否已初始化（用于测试）
bool sqlcc::DatabaseManager::IsInitialized() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return !is_closed_ && config_manager_ != nullptr;
}

// 执行SQL语句的方法（用于测试）
bool sqlcc::DatabaseManager::Execute([[maybe_unused]] const std::string &sql) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (is_closed_) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("DatabaseManager is closed, cannot execute SQL");
#endif
    return false;
  }

  // 简化实现：仅记录SQL语句，不实际执行
#ifdef USE_SPDLOG
  SPDLOG_INFO("Executing SQL: {}", sql);
#endif

  // TODO: 这里应该集成实际的SQL执行逻辑
  return true;
}

// 获取表名列表的方法（用于测试）
std::vector<std::string> sqlcc::DatabaseManager::GetTableNames() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (current_database_.empty()) {
    return {};
  }

  return database_tables_[current_database_];
}

// 获取表结构的方法（用于测试）
std::string
sqlcc::DatabaseManager::GetTableSchema(const std::string &table_name) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (current_database_.empty()) {
    return "";
  }

  // 检查表是否存在
  if (!TableExists(table_name)) {
#ifdef USE_SPDLOG
    SPDLOG_ERROR("Table {} does not exist in database {}", table_name,
                 current_database_);
#endif
    return "";
  }

  // 简化实现：返回示例表结构
  // 在实际实现中，这里应该从表文件中读取真实的表结构
  return "CREATE TABLE " + table_name +
         " (id INT PRIMARY KEY, name VARCHAR(50))";
}

} // namespace sqlcc
