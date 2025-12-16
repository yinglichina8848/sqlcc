#include "storage/index_manager.h"
#include "storage/b_plus_tree.h"
#include "storage_engine.h"
#include "utils/config_manager.h"
#include "utils/logger.h"

namespace sqlcc {

IndexManager::IndexManager(std::shared_ptr<StorageEngine> storage_engine, ConfigManager &)
    : storage_engine_(storage_engine) {
  SQLCC_LOG_INFO("Initializing IndexManager");
  LoadAllIndexes();
}

IndexManager::~IndexManager() {
  SQLCC_LOG_INFO("Destroying IndexManager");
  // 索引对象会通过unique_ptr自动清理
  indexes_.clear();
}

bool IndexManager::CreateIndex(const std::string &index_name,
                               const std::string &table_name,
                               const std::string &column_name, bool) {
  SQLCC_LOG_INFO("Creating index: " + index_name + " on table: " + table_name +
                 ", column: " + column_name);

  // 检查索引是否已存在
  if (IndexExists(index_name, table_name)) {
    SQLCC_LOG_WARN("Index already exists: " + index_name);
    return false;
  }

  // 创建新的B+树索引 - 使用智能指针
  auto index = std::make_unique<BPlusTreeIndex>(storage_engine_, table_name, column_name);
  if (!index->Create()) {
    SQLCC_LOG_ERROR("Failed to create index: " + index_name);
    return false;
  }

  // 将索引添加到索引映射表
  indexes_[index_name] = std::move(index);
  SQLCC_LOG_INFO("Index created successfully: " + index_name);
  return true;
}

bool IndexManager::DropIndex(const std::string &index_name,
                             const std::string &table_name) {
  SQLCC_LOG_INFO("Dropping index: " + index_name + " on table: " + table_name);

  // 检查索引是否存在
  if (!IndexExists(index_name, table_name)) {
    SQLCC_LOG_WARN("Index does not exist: " + index_name);
    return false;
  }

  // 从索引映射表中移除索引
  indexes_.erase(index_name);
  SQLCC_LOG_INFO("Index dropped successfully: " + index_name);
  return true;
}

bool IndexManager::IndexExists(const std::string &index_name,
                               const std::string &table_name) const {
  return indexes_.find(index_name) != indexes_.end();
}

BPlusTreeIndex *IndexManager::GetIndex(const std::string &index_name,
                                       const std::string &table_name) {
  auto it = indexes_.find(index_name);
  if (it != indexes_.end()) {
    return it->second.get();
  }
  return nullptr;
}

std::vector<BPlusTreeIndex *>
IndexManager::GetTableIndexes(const std::string &table_name) const {
  std::vector<BPlusTreeIndex *> result;

  for (const auto &[index_name, index] : indexes_) {
    if (index->GetTableName() == table_name) {
      result.push_back(index.get());
    }
  }

  return result;
}

std::string IndexManager::GetIndexName(const std::string &table_name,
                                       const std::string &column_name) const {
  return table_name + "_" + column_name + "_idx";
}

bool IndexManager::CreateCompositeIndex(const std::string &index_name,
                                       const std::string &table_name,
                                       const std::vector<std::string> &columns,
                                       bool unique) {
  SQLCC_LOG_INFO("Creating composite index: " + index_name + " on table: " + table_name +
                 " with columns: " + [&columns]() {
                   std::string cols;
                   for (size_t i = 0; i < columns.size(); ++i) {
                     if (i > 0) cols += ",";
                     cols += columns[i];
                   }
                   return cols;
                 }());

  // 检查索引是否已存在
  if (IndexExists(index_name, table_name)) {
    SQLCC_LOG_WARN("Index already exists: " + index_name);
    return false;
  }

  // 对于复合索引，我们创建一个特殊的索引对象
  // 目前简化实现：为第一个列创建索引，后续可扩展为真正的复合索引
  if (!columns.empty()) {
    auto index = std::make_unique<BPlusTreeIndex>(storage_engine_, table_name, columns[0]);
    if (!index->Create()) {
      SQLCC_LOG_ERROR("Failed to create composite index: " + index_name);
      return false;
    }
    indexes_[index_name] = std::move(index);
  }

  SQLCC_LOG_INFO("Composite index created successfully: " + index_name);
  return true;
}

std::string IndexManager::GetCompositeIndexName(const std::string &table_name,
                                               const std::vector<std::string> &columns) const {
  std::string name = table_name + "_composite_";
  for (size_t i = 0; i < columns.size(); ++i) {
    if (i > 0) name += "_";
    name += columns[i];
  }
  name += "_idx";
  return name;
}

std::vector<std::string>
IndexManager::GetIndexedColumns(const std::string &table_name) const {
  std::vector<std::string> result;

  for (const auto &[index_name, index] : indexes_) {
    if (index->GetTableName() == table_name) {
      result.push_back(index->GetColumnName());
    }
  }

  return result;
}

std::vector<std::vector<std::string>>
IndexManager::GetCompositeIndexedColumns(const std::string &table_name) const {
  // 目前简化实现，返回空向量
  // 真正的复合索引实现需要维护复合索引的列信息
  return std::vector<std::vector<std::string>>();
}

void IndexManager::LoadAllIndexes() {
  SQLCC_LOG_INFO("Loading all indexes from storage");

  // 基本实现：从存储加载索引元数据
  // 目前暂时只记录日志，不实际加载索引
  // 这可以防止系统在初始化时卡住

  // 在实际实现中，这里应该：
  // 1. 从系统表或元数据文件中读取索引定义
  // 2. 为每个索引创建对应的BPlusTreeIndex对象
  // 3. 将索引对象添加到索引映射表中

  SQLCC_LOG_INFO("Index loading completed (basic implementation)");
}

} // namespace sqlcc
