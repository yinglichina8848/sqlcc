/**
 * @file schema_manager.h
 * @brief SQLCC模式管理器 - 逻辑命名空间与多租户隔离核心
 *
 * SchemaManager 负责管理数据库中的 Schema（模式/命名空间）。在 SQL 标准中，
 * Schema 是数据库对象的容器，用于组织表、视图、索引等对象。它提供了逻辑隔离
 * 和访问控制边界，是多租户架构（Multi-Tenancy）的基础组件。
 *
 * 📚 配套教材参考：
 * - [第3章：SQL数据定义](../../textbook/《数据库系统原理与开发实践》.md#第三章sql数据定义)
 * - [3.1 模式定义与管理](../../textbook/《数据库系统原理与开发实践》.md#31-模式定义与管理)
 * - [10.2 存取控制与用户管理](../../textbook/《数据库系统原理与开发实践》.md#102-存取控制与用户管理)
 *
 * WHY层 - 设计意图：
 *   1. **逻辑隔离**：允许不同用户在同一个物理数据库中创建同名表而不冲突（如 user1.table1 和 user2.table1）。
 *   2. **权限边界**：基于 Schema 进行授权（GRANT ALL ON SCHEMA ...），简化权限管理。
 *   3. **组织结构**：将相关的数据库对象分组管理，提高可维护性。
 *
 * WHAT层 - 功能说明：
 *   - Schema CRUD：创建（CreateSchema）、删除（DropSchema）、修改（AlterSchema）。
 *   - 元数据持久化：将 Schema 定义存储在系统表（如 `sqlcc_schemas`）中。
 *   - 存在性检查：快速验证 Schema 是否存在，防止无效引用。
 *   - 列表查询：获取系统内所有 Schema 的清单。
 *
 * HOW层 - 实现机制：
 *   - **内存映射**：使用 `unordered_map<name, Schema>` 缓存所有 Schema 元数据，
 *     加速查找（通常 Schema 数量有限，全量缓存可行）。
 *   - **持久化同步**：任何修改操作（Create/Drop/Alter）都必须先写入系统表，再更新内存缓存。
 *   - **并发控制**：使用互斥锁（Mutex）保护内存缓存的并发访问。
 *   - **默认 Schema**：系统初始化时自动创建 'public' 模式，作为默认命名空间。
 *
 * @author SQLCC技术委员会
 * @version 1.2.6
 * @date 2026-02-02
 */

#ifndef SCHEMA_MANAGER_H
#define SCHEMA_MANAGER_H

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace sqlcc {

/**
 * @struct Schema
 * @brief 模式元数据结构
 */
struct Schema {
  std::string schema_name;  ///< 模式名称（唯一标识）
  std::string owner;        ///< 拥有者用户名
  std::string created_at;   ///< 创建时间戳
};

/**
 * @class SchemaManager
 * @brief 模式管理器 - 负责 Schema 对象的全生命周期管理
 */
class SchemaManager {
public:
  SchemaManager();
  ~SchemaManager();

  /**
   * @brief 创建新模式
   * 
   * WHY: 为用户或应用分配独立的命名空间。
   * @param schema_name 模式名称
   * @param owner 拥有者
   * @return 是否创建成功
   */
  bool CreateSchema(const std::string &schema_name, const std::string &owner);

  /**
   * @brief 删除模式
   * 
   * @note 通常需要检查模式是否为空（CASCADE/RESTRICT 语义由执行器处理）。
   */
  bool DropSchema(const std::string &schema_name);

  /**
   * @brief 修改模式属性（如重命名）
   */
  bool AlterSchema(const std::string &schema_name,
                   const std::string &new_schema_name);

  /**
   * @brief 获取模式详细信息
   * @return Schema 结构体副本
   */
  Schema GetSchema(const std::string &schema_name) const;

  /**
   * @brief 列出所有模式
   * 用于 `SHOW SCHEMAS` 命令。
   */
  std::vector<Schema> ListSchemas() const;

  /**
   * @brief 快速检查模式是否存在
   */
  bool SchemaExists(const std::string &schema_name) const;

  const std::string &GetLastError() const;

private:
  std::string GetCurrentTimeString();

  std::unordered_map<std::string, Schema> schemas_;
  mutable std::mutex mutex_;
  std::string last_error_;
};

} // namespace sqlcc

#endif // SCHEMA_MANAGER_H