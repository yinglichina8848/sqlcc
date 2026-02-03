/**
 * @file index_manager.h
 * @brief SQLCC索引管理器 - 数据库索引生命周期与访问控制中心
 *
 * IndexManager 是存储引擎中负责管理所有 B+ 树索引的顶层组件。
 * 它维护了表、列与物理索引文件之间的映射关系，并协调索引的创建、
 * 删除、加载和查询操作。它是查询优化器（Query Optimizer）获取
 * 访问路径（Access Path）的主要来源。
 *
 * 📚 配套教材参考：
 * - [第5章：索引与散列](../../textbook/《数据库系统原理与开发实践》.md#第五章索引与散列)
 * - [5.1 索引的基本概念](../../textbook/《数据库系统原理与开发实践》.md#51-索引的基本概念)
 * - [5.2 B+树索引](../../textbook/《数据库系统原理与开发实践》.md#52-b树索引)
 * - [5.4 索引的管理与维护](../../textbook/《数据库系统原理与开发实践》.md#54-索引的管理与维护)
 *
 * WHY层 - 设计意图：
 *   1. **统一管理入口**：避免散落在各处的索引操作，提供集中的 CRUD 接口。
 *   2. **生命周期绑定**：确保索引与主表的生命周期一致（表删则索引删）。
 *   3. **元数据一致性**：维护内存中的索引对象与磁盘上的元数据表同步。
 *   4. **并发控制**：在创建/删除索引时提供必要的锁保护，防止并发访问异常。
 *
 * WHAT层 - 功能说明：
 *   - 索引注册表：维护 `Map<TableName, List<Index>>` 的映射关系。
 *   - 物理管理：负责分配索引根页（Root Page），调用 B+ 树底层接口。
 *   - 启动加载：系统启动时，自动扫描元数据表，重建所有索引的内存句柄。
 *   - 复合索引支持：处理多列联合索引的创建逻辑。
 *
 * HOW层 - 实现机制：
 *   - **延迟加载**：构造时不立即加载所有索引，而是根据元数据按需或在 `LoadAllIndexes` 中加载。
 *   - **命名规范**：默认使用 `idx_table_column` 格式，也支持用户自定义名称。
 *   - **所有权管理**：使用 `std::unique_ptr` 管理 `BPlusTreeIndex` 对象，确保内存安全。
 *   - **原子操作**：创建索引时，先持久化元数据，再初始化 B+ 树结构，保证崩溃一致性。
 *
 * @author SQLCC技术委员会
 * @version 1.2.6
 * @date 2026-02-02
 */

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
 */
class IndexManager {
public:
  /**
   * @brief 构造函数
   * @param storage_engine 存储引擎实例，用于底层页面操作
   * @param config_manager 配置管理器
   */
  IndexManager(std::shared_ptr<StorageEngine> storage_engine, ConfigManager &config_manager);
  
  ~IndexManager();

  /**
   * @brief 创建单列索引
   * 
   * WHY: 加速特定列的查询性能。
   * HOW: 
   * 1. 检查索引是否已存在。
   * 2. 在元数据表中注册新索引。
   * 3. 分配根页面并初始化 B+ 树结构。
   * 4. (可选) 扫描全表构建初始索引数据。
   * 
   * @param index_name 索引名称
   * @param table_name 目标表名
   * @param column_name 目标列名
   * @param unique 是否唯一索引
   * @return 是否创建成功
   */
  bool CreateIndex(const std::string &index_name, const std::string &table_name,
                   const std::string &column_name, bool unique = false);

  /**
   * @brief 创建复合索引
   * 
   * WHY: 支持多列联合查询（如 WHERE a=1 AND b=2）。
   * WHAT: 将多个列的值组合成复合键（Key）。
   */
  bool CreateCompositeIndex(const std::string &index_name,
                           const std::string &table_name,
                           const std::vector<std::string> &columns,
                           bool unique = false);

  /**
   * @brief 删除索引
   * 
   * HOW:
   * 1. 从内存映射中移除索引对象。
   * 2. 更新元数据表，标记索引为删除。
   * 3. 释放 B+ 树占用的所有物理页面（需存储引擎支持页面回收）。
   */
  bool DropIndex(const std::string &index_name, const std::string &table_name);

  /**
   * @brief 检查索引是否存在
   */
  bool IndexExists(const std::string &index_name,
                   const std::string &table_name) const;

  /**
   * @brief 获取索引对象指针
   * 
   * 用于执行器（Executor）进行实际的插入、删除或查找操作。
   * @return BPlusTreeIndex* 指针，若不存在返回 nullptr
   */
  BPlusTreeIndex *GetIndex(const std::string &index_name,
                           const std::string &table_name);

  /**
   * @brief 获取表的所有索引
   * 
   * 用于 DML 操作（Insert/Update/Delete）时，同步更新该表的所有索引。
   */
  std::vector<BPlusTreeIndex *>
  GetTableIndexes(const std::string &table_name) const;

  // 获取表的索引列元数据
  std::vector<std::string>
  GetIndexedColumns(const std::string &table_name) const;
  
  std::vector<std::vector<std::string>>
  GetCompositeIndexedColumns(const std::string &table_name) const;

  // 索引命名工具方法
  std::string GetIndexName(const std::string &table_name,
                           const std::string &column_name) const;
  std::string GetCompositeIndexName(const std::string &table_name,
                                   const std::vector<std::string> &columns) const;

private:
  std::shared_ptr<StorageEngine> storage_engine_;
  
  /**
   * @brief 内存索引注册表
   * Key: 索引全名 (通常是 table_name + "." + index_name)
   * Value: BPlusTreeIndex 对象的唯一指针
   */
  std::unordered_map<std::string, std::unique_ptr<BPlusTreeIndex>> indexes_;

  /**
   * @brief 从持久化存储加载所有索引定义
   * 通常在系统启动时调用。
   */
  void LoadAllIndexes();
};

} // namespace sqlcc

#endif // SQLCC_INDEX_MANAGER_H
