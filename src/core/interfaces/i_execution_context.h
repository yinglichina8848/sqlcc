/**
 * @file i_execution_context.h
 * @brief SQLCC执行上下文接口
 * @author SQLCC Team
 * @date 2026-02-11
 * @copyright Copyright (c) 2026
 *
 * 文件用途说明：
 * 本文件定义了执行上下文的抽象接口，用于解耦 SQL 执行模块与 Core 模块。
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace sqlcc {
namespace core {
namespace interfaces {

// 前向声明
class IDatabaseManager;
class IUserManager;

// 事务 ID 类型定义
using TransactionId = uint64_t;

/**
 * WHY: 为什么需要执行上下文接口？
 *
 * 1. **状态管理**: SQL 执行需要维护复杂的上下文状态
 * 2. **依赖解耦**: 执行器不应该直接依赖 ExecutionContext 的具体实现
 * 3. **可测试性**: 便于 Mock 执行上下文进行单元测试
 * 4. **多租户**: 支持不同租户的执行上下文隔离
 *
 * WHAT: 执行上下文的核心能力
 *
 * - 用户信息：当前用户、数据库
 * - 事务状态：事务 ID、隔离级别
 * - 执行统计：影响行数、执行时间
 * - 错误处理：错误状态、错误信息
 */

/**
 * @brief 执行上下文接口
 */
class IExecutionContext {
public:
    virtual ~IExecutionContext() = default;
    
    // ========================================================================
    // 用户和数据库上下文
    // ========================================================================
    
    /**
     * @brief 获取当前用户名
     * @return 用户名
     */
    virtual std::string GetCurrentUser() const = 0;
    
    /**
     * @brief 设置当前用户名
     * @param user 用户名
     */
    virtual void SetCurrentUser(const std::string& user) = 0;
    
    /**
     * @brief 获取当前数据库名
     * @return 数据库名
     */
    virtual std::string GetCurrentDatabase() const = 0;
    
    /**
     * @brief 设置当前数据库名
     * @param database 数据库名
     */
    virtual void SetCurrentDatabase(const std::string& database) = 0;
    
    // ========================================================================
    // 事务状态
    // ========================================================================
    
    /**
     * @brief 检查是否处于事务中
     * @return true 处于事务中，false 不在事务中
     */
    virtual bool IsTransactional() const = 0;
    
    /**
     * @brief 设置事务状态
     * @param is_transactional 是否处于事务中
     */
    virtual void SetTransactional(bool is_transactional) = 0;
    
    /**
     * @brief 获取事务 ID
     * @return 事务 ID，如果不在事务中返回 0
     */
    virtual TransactionId GetTransactionId() const = 0;

    /**
     * @brief 设置事务 ID
     * @param transaction_id 事务 ID
     */
    virtual void SetTransactionId(TransactionId transaction_id) = 0;
    
    /**
     * @brief 检查是否为只读执行
     * @return true 只读，false 可写
     */
    virtual bool IsReadOnly() const = 0;
    
    /**
     * @brief 设置只读状态
     * @param read_only 是否为只读
     */
    virtual void SetReadOnly(bool read_only) = 0;
    
    // ========================================================================
    // 执行统计
    // ========================================================================
    
    /**
     * @brief 获取影响的行数
     * @return 影响的行数
     */
    virtual size_t GetRowsAffected() const = 0;
    
    /**
     * @brief 设置影响的行数
     * @param rows 行数
     */
    virtual void SetRowsAffected(size_t rows) = 0;
    
    /**
     * @brief 增加影响的行数
     * @param rows 要增加的行数，默认为 1
     */
    virtual void IncrementRowsAffected(size_t rows = 1) = 0;
    
    /**
     * @brief 获取返回的行数
     * @return 返回的行数
     */
    virtual size_t GetRowsReturned() const = 0;
    
    /**
     * @brief 设置返回的行数
     * @param rows 行数
     */
    virtual void SetRowsReturned(size_t rows) = 0;
    
    /**
     * @brief 获取执行时间（毫秒）
     * @return 执行时间
     */
    virtual size_t GetExecutionTimeMs() const = 0;
    
    /**
     * @brief 设置执行时间
     * @param time_ms 执行时间（毫秒）
     */
    virtual void SetExecutionTimeMs(size_t time_ms) = 0;
    
    // ========================================================================
    // 执行计划
    // ========================================================================
    
    /**
     * @brief 检查是否使用了索引
     * @return true 使用了索引，false 未使用
     */
    virtual bool IsUsedIndex() const = 0;
    
    /**
     * @brief 设置是否使用了索引
     * @param used_index 是否使用了索引
     */
    virtual void SetUsedIndex(bool used_index) = 0;
    
    /**
     * @brief 获取执行计划
     * @return 执行计划描述
     */
    virtual std::string GetExecutionPlan() const = 0;
    
    /**
     * @brief 设置执行计划
     * @param execution_plan 执行计划描述
     */
    virtual void SetExecutionPlan(const std::string& execution_plan) = 0;
    
    // ========================================================================
    // 错误处理
    // ========================================================================
    
    /**
     * @brief 检查是否有错误
     * @return true 有错误，false 无错误
     */
    virtual bool HasError() const = 0;
    
    /**
     * @brief 设置错误状态
     * @param has_error 是否有错误
     * @param error_message 错误信息
     */
    virtual void SetError(bool has_error, const std::string& error_message = "") = 0;
    
    /**
     * @brief 获取错误信息
     * @return 错误信息
     */
    virtual std::string GetErrorMessage() const = 0;
    
    /**
     * @brief 清除错误状态
     */
    virtual void ClearError() = 0;
    
    // ========================================================================
    // 管理器访问
    // ========================================================================
    
    /**
     * @brief 获取数据库管理器接口
     * @return 数据库管理器接口指针
     */
    virtual std::shared_ptr<IDatabaseManager> GetDbManager() const = 0;
    
    /**
     * @brief 设置数据库管理器接口
     * @param db_manager 数据库管理器接口指针
     */
    virtual void SetDbManager(std::shared_ptr<IDatabaseManager> db_manager) = 0;
    
    /**
     * @brief 获取用户管理器接口
     * @return 用户管理器接口指针
     */
    virtual std::shared_ptr<IUserManager> GetUserManager() const = 0;
    
    /**
     * @brief 设置用户管理器接口
     * @param user_manager 用户管理器接口指针
     */
    virtual void SetUserManager(std::shared_ptr<IUserManager> user_manager) = 0;
    
    // ========================================================================
    // 上下文操作
    // ========================================================================
    
    /**
     * @brief 重置上下文状态
     */
    virtual void Reset() = 0;
    
    /**
     * @brief 克隆上下文
     * @return 上下文的深拷贝
     */
    virtual std::shared_ptr<IExecutionContext> Clone() const = 0;
    
    /**
     * @brief 将上下文转换为字符串（用于调试）
     * @return 上下文的字符串表示
     */
    virtual std::string ToString() const = 0;
};

} // namespace interfaces
} // namespace core
} // namespace sqlcc
