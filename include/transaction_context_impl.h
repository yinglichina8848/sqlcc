#pragma once

#include "transaction_context.h"
#include "transaction_manager.h"

namespace sqlcc {

/**
 * @brief TransactionContext接口的具体实现
 *
 * 基于TransactionManager实现事务上下文接口，
 * 通过组合模式将TransactionManager的功能适配为TransactionContext接口。
 */
class TransactionContextImpl : public TransactionContext {
public:
    /**
     * @brief 构造函数
     * @param transaction_manager TransactionManager的引用
     */
    explicit TransactionContextImpl(TransactionManager& transaction_manager);

    /**
     * @brief 开始事务
     * @return 事务ID，失败返回空字符串
     */
    std::string beginTransaction() override;

    /**
     * @brief 提交事务
     * @param transaction_id 事务ID
     * @return 是否成功
     */
    bool commitTransaction(const std::string& transaction_id) override;

    /**
     * @brief 回滚事务
     * @param transaction_id 事务ID
     * @return 是否成功
     */
    bool rollbackTransaction(const std::string& transaction_id) override;

    /**
     * @brief 检查事务是否活跃
     * @param transaction_id 事务ID
     * @return 是否活跃
     */
    bool isTransactionActive(const std::string& transaction_id) override;

private:
    /// TransactionManager的引用
    TransactionManager& transaction_manager_;

    /// 将字符串ID转换为TransactionId
    TransactionId stringToTxnId(const std::string& id) const;

    /// 将TransactionId转换为字符串
    std::string txnIdToString(TransactionId id) const;
};

} // namespace sqlcc
