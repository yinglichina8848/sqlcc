#pragma once

#include "transaction_manager.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace sqlcc {
namespace mocks {

/**
 * @brief TransactionManager Mock类，用于单元测试
 *
 * 提供可配置的TransactionManager接口实现，支持：
 * - 模拟事务操作的成功/失败
 * - 记录方法调用历史
 * - 自定义返回值
 * - 验证调用参数
 */
class TransactionManagerMock : public TransactionManager {
public:
    /**
     * @brief 构造函数
     */
    TransactionManagerMock();

    /**
     * @brief 析构函数
     */
    ~TransactionManagerMock() override;

    // 禁止拷贝
    TransactionManagerMock(const TransactionManagerMock&) = delete;
    TransactionManagerMock& operator=(const TransactionManagerMock&) = delete;

    // Mock配置方法
    void SetBeginTransactionResult(TransactionId txn_id);
    void SetCommitTransactionResult(bool success);
    void SetRollbackTransactionResult(bool success);
    void SetCreateSavepointResult(bool success);
    void SetRollbackToSavepointResult(bool success);
    void SetAcquireLockResult(bool success);
    void SetDetectDeadlockResult(bool has_deadlock);
    void SetGetTransactionStateResult(TransactionState state);
    void SetGetActiveTransactionsResult(const std::vector<TransactionId>& txn_ids);
    void SetNextTransactionIdResult(TransactionId txn_id);

    // 调用历史记录
    struct CallRecord {
        std::string method_name;
        std::vector<std::string> args;
    };

    const std::vector<CallRecord>& GetCallHistory() const { return call_history_; }
    void ClearCallHistory() { call_history_.clear(); }

    // 重写TransactionManager接口方法
    TransactionId begin_transaction(IsolationLevel isolation_level = IsolationLevel::READ_COMMITTED) override;
    bool commit_transaction(TransactionId txn_id) override;
    bool rollback_transaction(TransactionId txn_id) override;
    bool create_savepoint(TransactionId txn_id, const std::string &savepoint_name) override;
    bool rollback_to_savepoint(TransactionId txn_id, const std::string &savepoint_name) override;
    bool acquire_lock(TransactionId txn_id, const std::string &resource, LockType lock_type, bool wait = true) override;
    void release_lock(TransactionId txn_id, const std::string &resource) override;
    bool detect_deadlock(TransactionId txn_id) override;
    TransactionState get_transaction_state(TransactionId txn_id) const override;
    std::vector<TransactionId> get_active_transactions() const override;
    void log_operation(TransactionId txn_id, const LogEntry &entry) override;
    TransactionId next_transaction_id() override;

private:
    void RecordCall(const std::string& method, const std::vector<std::string>& args = {});

    // Mock配置
    TransactionId begin_transaction_result_ = 1;
    bool commit_transaction_success_ = true;
    bool rollback_transaction_success_ = true;
    bool create_savepoint_success_ = true;
    bool rollback_to_savepoint_success_ = true;
    bool acquire_lock_success_ = true;
    bool detect_deadlock_result_ = false;
    TransactionState get_transaction_state_result_ = TransactionState::ACTIVE;
    std::vector<TransactionId> get_active_transactions_result_;
    TransactionId next_transaction_id_result_ = 100;

    // 调用历史
    std::vector<CallRecord> call_history_;
};

} // namespace mocks
} // namespace sqlcc

#endif // SQLCC_MOCKS_TRANSACTION_MANAGER_MOCK_H