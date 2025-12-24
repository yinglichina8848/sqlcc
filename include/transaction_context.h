#pragma once

#include <memory>
#include <string>
#include <vector>

namespace sqlcc {

/**
 * @brief 事务上下文接口
 *
 * 定义事务管理的基本操作，用于解耦存储引擎和事务管理器的直接依赖。
 * 通过依赖倒置原则，存储引擎不再直接依赖具体的事务管理器实现，
 * 而是通过此接口进行事务操作。
 */
class TransactionContext {
public:
    virtual ~TransactionContext() = default;

    /**
     * @brief 开始事务
     * @return 事务ID，失败返回空字符串
     */
    virtual std::string beginTransaction() = 0;

    /**
     * @brief 提交事务
     * @param transaction_id 事务ID
     * @return 是否成功
     */
    virtual bool commitTransaction(const std::string& transaction_id) = 0;

    /**
     * @brief 回滚事务
     * @param transaction_id 事务ID
     * @return 是否成功
     */
    virtual bool rollbackTransaction(const std::string& transaction_id) = 0;

    /**
     * @brief 检查事务是否活跃
     * @param transaction_id 事务ID
     * @return 是否活跃
     */
    virtual bool isTransactionActive(const std::string& transaction_id) = 0;
};

} // namespace sqlcc
