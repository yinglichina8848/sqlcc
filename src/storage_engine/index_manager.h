#ifndef SQLCC_INDEX_MANAGER_H
#define SQLCC_INDEX_MANAGER_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// 前向声明解决循环依赖
namespace sqlcc {
class StorageEngine;
class BPlusTreeIndex;
class ConfigManager;
} // namespace sqlcc

namespace sqlcc {

/**
 * @class IndexManager
 * @brief 索引管理器 - 负责 B+ 树索引的生命周期与查找路由
 *
 * WHY层 - 设计意图：
 *   全表扫描在大规模数据下极其低效。索引通过维护有序的键值对，将查找复杂度从 O(N) 降低到 O(log N)。
 *   IndexManager 统一管理系统中所有的索引对象，确保查询优化器能够快速发现可用索引，
 *   并在数据更新时维护索引与主表的一致性。
 *
 * WHAT层 - 功能说明：
 *   提供索引创建（CreateIndex）和删除（DropIndex）。
 *   支持单列索引和复合索引（Composite Index）。
 *   维护表名到索引对象的映射，支持索引存在性检查。
 *   自动生成标准化的索引名称规范。
 *
 * HOW层 - 实现机制：
 *   1. 内存索引映射：使用 unordered_map 以索引全名（Table.Index）为键存储 unique_ptr<BPlusTreeIndex>。
 *   2. 物理绑定：每个索引对象内部关联一个 PageID，作为 B+ 树的根节点。
 *   3. 延迟加载：LoadAllIndexes 在系统启动时扫描元数据表并重建内存句柄。
 *   4. 一致性协调：通过 StorageEngine 提供的页面接口，确保索引节点的持久化。
 */
class IndexManager {
public:
  IndexManager(std::shared_ptr<StorageEngine> storage_engine, ConfigManager &config_manager);
  ~IndexManager();

  // 索引管理
  bool CreateIndex(const std::string &index_name, const std::string &table_name,
                   const std::string &column_name, bool unique = false);
  bool CreateCompositeIndex(const std::string &index_name,
                           const std::string &table_name,
                           const std::vector<std::string> &columns,
                           bool unique = false);
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
  std::vector<std::vector<std::string>>
  GetCompositeIndexedColumns(const std::string &table_name) const;

  // 索引名称生成
  std::string GetIndexName(const std::string &table_name,
                           const std::string &column_name) const;
  std::string GetCompositeIndexName(const std::string &table_name,
                                   const std::vector<std::string> &columns) const;

private:
  std::shared_ptr<StorageEngine> storage_engine_; // 存储引擎智能指针
  std::unordered_map<std::string, std::unique_ptr<BPlusTreeIndex>>
      indexes_; // 索引映射表

  // 内部方法
  void LoadAllIndexes();
};

} // namespace sqlcc

#endif // SQLCC_INDEX_MANAGER_H
