/**
 * @file i_database_manager.h
 * @brief SQLCC核心数据库管理器接口
 * @author SQLCC Team
 * @date 2026-02-11
 * @copyright Copyright (c) 2026
 *
 * 文件用途说明：
 * 本文件定义了核心数据库管理器的抽象接口，用于解耦 Core 模块与具体实现。
 * 采用纯虚接口设计，支持依赖注入和 Mock 测试。
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace sqlcc {
namespace core {
namespace interfaces {

/**
 * WHY: 为什么需要数据库管理器接口？
 *
 * 1. **依赖解耦**: ExecutionContext 等组件不应该直接依赖 DatabaseManager 的具体实现
 * 2. **可测试性**: 通过接口可以轻松地 Mock 数据库管理器进行单元测试
 * 3. **扩展性**: 未来可以支持不同类型的数据库管理器实现（内存版、分布式版等）
 * 4. **编译优化**: 使用接口可以减少头文件依赖，提高编译速度
 *
 * WHAT: 数据库管理器的抽象接口定义
 *
 * 核心能力：
 * - 数据库生命周期管理：创建、删除、使用数据库
 * - 表管理：表的 CRUD 操作
 * - 事务控制：事务的开始、提交、回滚
 * - SQL 执行：执行 SQL 语句
 * - 元数据查询：获取表结构、索引等信息
 *
 * HOW: 接口设计原则
 *
 * 1. 纯虚函数：所有方法都是纯虚函数，强制实现类提供具体实现
 * 2. 返回值：使用 bool 表示成功/失败，避免异常（与 SQLCC 整体风格一致）
 * 3. 智能指针：返回 shared_ptr 用于资源管理
 * 4. 常量正确性：查询方法标记为 const
 */

// 前向声明
class IStorageEngine;
class IIndexManager;
class ITransactionManager;
class IConfigManager;

// 事务隔离级别
enum class IsolationLevel {
    READ_UNCOMMITTED,
    READ_COMMITTED,
    REPEATABLE_READ,
    SERIALIZABLE
};

// 事务 ID 类型
using TransactionId = uint64_t;

/**
 * @brief 表元数据接口
 *
 * WHY: 为什么需要表元数据接口？
 * 数据库系统需要统一的表结构描述方式，用于：
 * - 查询优化器获取表结构信息
 * - 执行器验证列存在性和类型
 * - 存储引擎管理表的生命周期
 *
 * WHAT: 表元数据的核心信息
 * - 表名标识
 * - 列定义（名称、类型）
 * - 主键信息
 *
 * HOW: 使用接口而非具体实现
 * - 支持不同类型的表（普通表、视图、临时表）
 * - 便于 Mock 测试
 * - 支持未来扩展（分区信息、约束等）
 */
class ITableMetadata {
public:
    virtual ~ITableMetadata() = default;

    /**
     * @brief 获取表名
     * @return 表名
     */
    virtual std::string GetTableName() const = 0;

    /**
     * @brief 获取列定义
     * @return 列定义列表，格式为 [(列名, 类型), ...]
     */
    virtual std::vector<std::pair<std::string, std::string>> GetColumns() const = 0;

    /**
     * @brief 获取主键列名
     * @return 主键列名，如果没有主键返回空字符串
     */
    virtual std::string GetPrimaryKey() const = 0;
};

/**
 * @brief 数据库管理器接口
 * 
 * 这是 Core 模块的核心接口，定义了数据库系统的统一管理能力。
 */
class IDatabaseManager {
public:
    virtual ~IDatabaseManager() = default;
    
    // ========================================================================
    // 生命周期管理
    // ========================================================================
    
    /**
     * @brief 初始化数据库管理器
     * @return true 初始化成功，false 失败
     */
    virtual bool Initialize() = 0;
    
    /**
     * @brief 关闭数据库管理器
     * @return true 关闭成功，false 失败
     */
    virtual bool Close() = 0;
    
    /**
     * @brief 检查是否已初始化
     * @return true 已初始化，false 未初始化
     */
    virtual bool IsInitialized() const = 0;
    
    // ========================================================================
    // 数据库管理
    // ========================================================================
    
    /**
     * @brief 创建数据库
     * @param db_name 数据库名称
     * @return true 创建成功，false 失败
     */
    virtual bool CreateDatabase(const std::string& db_name) = 0;
    
    /**
     * @brief 删除数据库
     * @param db_name 数据库名称
     * @return true 删除成功，false 失败
     */
    virtual bool DropDatabase(const std::string& db_name) = 0;
    
    /**
     * @brief 切换当前数据库
     * @param db_name 数据库名称
     * @return true 切换成功，false 失败
     */
    virtual bool UseDatabase(const std::string& db_name) = 0;
    
    /**
     * @brief 获取当前数据库名称
     * @return 当前数据库名称，如果没有则返回空字符串
     */
    virtual std::string GetCurrentDatabase() const = 0;
    
    /**
     * @brief 列出所有数据库
     * @return 数据库名称列表
     */
    virtual std::vector<std::string> ListDatabases() = 0;
    
    /**
     * @brief 检查数据库是否存在
     * @param db_name 数据库名称
     * @return true 存在，false 不存在
     */
    virtual bool DatabaseExists(const std::string& db_name) const = 0;
    
    // ========================================================================
    // 表管理
    // ========================================================================
    
    /**
     * @brief 创建表
     * @param table_name 表名称
     * @param columns 列定义，格式为 [(列名, 类型), ...]
     * @return true 创建成功，false 失败
     */
    virtual bool CreateTable(const std::string& table_name,
                           const std::vector<std::pair<std::string, std::string>>& columns) = 0;
    
    /**
     * @brief 删除表
     * @param table_name 表名称
     * @return true 删除成功，false 失败
     */
    virtual bool DropTable(const std::string& table_name) = 0;
    
    /**
     * @brief 检查表是否存在
     * @param table_name 表名称
     * @return true 存在，false 不存在
     */
    virtual bool TableExists(const std::string& table_name) const = 0;
    
    /**
     * @brief 列出当前数据库的所有表
     * @return 表名称列表
     */
    virtual std::vector<std::string> ListTables() = 0;
    
    /**
     * @brief 获取表的元数据
     * @param table_name 表名称
     * @return 表元数据指针，如果不存在返回 nullptr
     */
    virtual std::shared_ptr<ITableMetadata> GetTableMetadata(const std::string& table_name) = 0;
    
    // ========================================================================
    // 索引管理
    // ========================================================================
    
    /**
     * @brief 创建索引
     * @param index_name 索引名称
     * @param table_name 表名称
     * @param columns 列名列表
     * @param unique 是否唯一索引
     * @param condition 条件表达式（可选）
     * @return true 创建成功，false 失败
     */
    virtual bool CreateIndex(const std::string& index_name,
                           const std::string& table_name,
                           const std::vector<std::string>& columns,
                           bool unique = false,
                           const std::string& condition = "") = 0;
    
    /**
     * @brief 删除索引
     * @param index_name 索引名称
     * @return true 删除成功，false 失败
     */
    virtual bool DropIndex(const std::string& index_name) = 0;
    
    // ========================================================================
    // 事务管理
    // ========================================================================
    
    /**
     * @brief 开始事务
     * @param isolation_level 隔离级别，默认为 READ_COMMITTED
     * @return 事务 ID，如果失败返回 0
     */
    virtual TransactionId BeginTransaction(IsolationLevel isolation_level = IsolationLevel::READ_COMMITTED) = 0;
    
    /**
     * @brief 提交事务
     * @param txn_id 事务 ID
     * @return true 提交成功，false 失败
     */
    virtual bool CommitTransaction(TransactionId txn_id) = 0;
    
    /**
     * @brief 回滚事务
     * @param txn_id 事务 ID
     * @return true 回滚成功，false 失败
     */
    virtual bool RollbackTransaction(TransactionId txn_id) = 0;
    
    // ========================================================================
    // SQL 执行
    // ========================================================================
    
    /**
     * @brief 执行 SQL 语句
     * @param sql SQL 语句
     * @return true 执行成功，false 失败
     */
    virtual bool Execute(const std::string& sql) = 0;
    
    /**
     * @brief 执行查询并返回结果
     * @param sql SELECT 语句
     * @return 结果集的 JSON 字符串表示
     */
    virtual std::string ExecuteQuery(const std::string& sql) = 0;
    
    // ========================================================================
    // 组件访问接口
    // ========================================================================
    
    /**
     * @brief 获取存储引擎接口
     * @return 存储引擎接口指针
     */
    virtual std::shared_ptr<IStorageEngine> GetStorageEngine() = 0;
    
    /**
     * @brief 获取索引管理器接口
     * @return 索引管理器接口指针
     */
    virtual std::shared_ptr<IIndexManager> GetIndexManager() = 0;
    
    /**
     * @brief 获取事务管理器接口
     * @return 事务管理器接口指针
     */
    virtual std::shared_ptr<ITransactionManager> GetTransactionManager() = 0;
    
    /**
     * @brief 获取配置管理器接口
     * @return 配置管理器接口指针
     */
    virtual std::shared_ptr<IConfigManager> GetConfig() = 0;
};

} // namespace interfaces
} // namespace core
} // namespace sqlcc
