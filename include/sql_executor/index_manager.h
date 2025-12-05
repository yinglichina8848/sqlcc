#ifndef SQLCC_INDEX_MANAGER_H
#define SQLCC_INDEX_MANAGER_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// 前向声明
namespace sqlcc {
class StorageEngine;
class ConfigManager;
class BPlusTreeIndex;
} // namespace sqlcc

namespace sqlcc {

class IndexManager {
public:
  IndexManager(StorageEngine *storage_engine, ConfigManager &config_manager);
  ~IndexManager();

  // 索引管理
  bool CreateIndex(const std::string &index_name, const std::string &table_name,
                   const std::string &column_name, bool unique = false);
  bool DropIndex(const std::string &index_name, const std::string &table_name);
  bool IndexExists(const std::string &index_name,
                   const std::string &table_name) const;

  // 索引查询
  BPlusTreeIndex *GetIndex(const std::string &index_name,
                           const std::string &table_name);
  std::vector<BPlusTreeIndex *>
  GetTableIndexes(const std::string &table_name) const;

  // 获取表的索引列
  std::vector<std::string>
  GetIndexedColumns(const std::string &table_name) const;

  // 索引名称生成
  std::string GetIndexName(const std::string &table_name,
                           const std::string &column_name) const;

private:
  StorageEngine *storage_engine_; // 存储引擎指针
  std::unordered_map<std::string, std::unique_ptr<BPlusTreeIndex>>
      indexes_; // 索引映射表

  // 内部方法
  void LoadAllIndexes();
};

} // namespace sqlcc

#endif // SQLCC_INDEX_MANAGER_H