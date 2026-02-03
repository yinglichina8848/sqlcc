/**
 * @file system_database.h
 * @brief SQLCC系统数据库 - 数据库元数据与系统目录管理
 *
 * SystemDatabase 是数据库系统的“元数据中心”。它管理着所有系统表（System Tables），
 * 这些表存储了数据库自身的关键信息，包括用户账户、权限配置、统计信息以及
 * 所有用户表的元数据定义。它是数据库自举（Bootstrap）和运行的基础。
 *
 * 📚 配套教材参考：
 * - [第3章：SQL数据定义](../../textbook/《数据库系统原理与开发实践》.md#第三章sql数据定义)
 * - [3.5 数据字典与系统目录](../../textbook/《数据库系统原理与开发实践》.md#35-数据字典与系统目录)
 * - [10.2 存取控制与用户管理](../../textbook/《数据库系统原理与开发实践》.md#102-存取控制与用户管理)
 *
 * WHY层 - 设计意图：
 *   1. **自描述性**：数据库系统必须能够描述自身。通过将元数据存储在特殊的系统表中，
 *      可以使用标准的 SQL 查询来访问和管理数据库结构。
 *   2. **集中管理**：避免元数据散落在文件头或硬编码中，提供统一的持久化和事务支持。
 *   3. **安全性基础**：用户认证和授权信息必须安全、可靠地存储。
 *
 * WHAT层 - 核心系统表：
 *   - `sqlcc_users`: 存储用户信息（用户名、密码哈希、创建时间）。
 *   - `sqlcc_privileges`: 存储用户/角色的权限映射。
 *   - `sqlcc_tables`: 存储所有用户表的定义（表名、Schema、行数估计）。
 *   - `sqlcc_columns`: 存储列定义（列名、类型、约束、默认值）。
 *   - `sqlcc_indexes`: 存储索引元数据（索引名、目标表、索引列）。
 *   - `sqlcc_statistics`: 存储优化器所需的统计信息（直方图、基数）。
 *
 * HOW层 - 实现机制：
 *   - **引导加载（Bootstrap）**：在 DatabaseManager 初始化时，首先加载 SystemDatabase。
 *     如果系统表不存在，会自动创建并初始化默认数据（如 'admin' 用户）。
 *   - **特权访问**：系统表的读写受严格控制，通常只有超级用户或内部执行器可操作。
 *   - **内存缓存**：为了性能，常驻元数据（如用户权限）会被加载到内存缓存中。
 *
 * @author SQLCC技术委员会
 * @version 1.2.6
 * @date 2026-02-02
 */

#ifndef SQLCC_CORE_SYSTEM_DATABASE_H
#define SQLCC_CORE_SYSTEM_DATABASE_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

namespace sqlcc {

class DatabaseManager;

/**
 * @class SystemDatabase
 * @brief 系统数据库管理类 - 封装了对系统目录表的访问
 */
class SystemDatabase {
public:
    /**
     * @brief 构造函数
     * @param db_manager 数据库管理器实例，用于底层表操作
     */
    explicit SystemDatabase(std::shared_ptr<DatabaseManager> db_manager);

    /**
     * @brief 析构函数
     */
    ~SystemDatabase();

    /**
     * @brief 初始化系统数据库
     * 
     * WHY: 确保所有必要的系统表都已存在且结构正确。
     * WHAT: 检查 `sqlcc_users`, `sqlcc_tables` 等表。如果缺失，
     * 执行 CREATE TABLE 语句进行创建，并插入默认的管理员账户。
     * 
     * @return 初始化是否成功
     */
    bool Initialize();

    /**
     * @brief 获取底层数据库管理器
     */
    std::shared_ptr<DatabaseManager> GetDatabaseManager();

    /**
     * @brief 检查系统数据库状态
     */
    bool IsInitialized() const;

    /**
     * @brief 获取最后一次错误信息
     */
    std::string GetLastError() const;

private:
    std::shared_ptr<DatabaseManager> db_manager_;
    bool is_initialized_;
    std::string last_error_;
};

} // namespace sqlcc

#endif // SQLCC_CORE_SYSTEM_DATABASE_H
