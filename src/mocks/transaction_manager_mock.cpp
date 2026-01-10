#include "mocks/transaction_manager_mock.h"
#include <sstream>

namespace sqlcc {
namespace mocks {

TransactionManagerMock::TransactionManagerMock()
    : TransactionManager() {
    RecordCall("TransactionManagerMock");
}

TransactionManagerMock::~TransactionManagerMock() {
    RecordCall("~TransactionManagerMock");
}

// Mock配置方法实现
void TransactionManagerMock::SetBeginTransactionResult(TransactionId txn_id) {
    begin_transaction_result_ = txn_id;
}

void TransactionManagerMock::SetCommitTransactionResult(bool success) {
    commit_transaction_success_ = success;
}

void TransactionManagerMock::SetRollbackTransactionResult(bool success) {
    rollback_transaction_success_ = success;
}

void TransactionManagerMock::SetCreateSavepointResult(bool success) {
    create_savepoint_success_ = success;
}

void TransactionManagerMock::SetRollbackToSavepointResult(bool success) {
    rollback_to_savepoint_success_ = success;
}

void TransactionManagerMock::SetAcquireLockResult(bool success) {
    acquire_lock_success_ = success;
}

void TransactionManagerMock::SetDetectDeadlockResult(bool has_deadlock) {
    detect_deadlock_result_ = has_deadlock;
}

void TransactionManagerMock::SetGetTransactionStateResult(TransactionState state) {
    get_transaction_state_result_ = state;
}

void TransactionManagerMock::SetGetActiveTransactionsResult(const std::vector<TransactionId>& txn_ids) {
    get_active_transactions_result_ = txn_ids;
}

void TransactionManagerMock::SetNextTransactionIdResult(TransactionId txn_id) {
    next_transaction_id_result_ = txn_id;
}

// 重写TransactionManager接口方法
TransactionId TransactionManagerMock::begin_transaction(IsolationLevel isolation_level) {
    RecordCall("begin_transaction", {std::to_string(static_cast<int>(isolation_level))});
    return begin_transaction_result_;
}

bool TransactionManagerMock::commit_transaction(TransactionId txn_id) {
    RecordCall("commit_transaction", {std::to_string(txn_id)});
    return commit_transaction_success_;
}

bool TransactionManagerMock::rollback_transaction(TransactionId txn_id) {
    RecordCall("rollback_transaction", {std::to_string(txn_id)});
    return rollback_transaction_success_;
}

bool TransactionManagerMock::create_savepoint(TransactionId txn_id, const std::string &savepoint_name) {
    RecordCall("create_savepoint", {std::to_string(txn_id), savepoint_name});
    return create_savepoint_success_;
}

bool TransactionManagerMock::rollback_to_savepoint(TransactionId txn_id, const std::string &savepoint_name) {
    RecordCall("rollback_to_savepoint", {std::to_string(txn_id), savepoint_name});
    return rollback_to_savepoint_success_;
}

bool TransactionManagerMock::acquire_lock(TransactionId txn_id, const std::string &resource, LockType lock_type, bool wait) {
    RecordCall("acquire_lock", {std::to_string(txn_id), resource, std::to_string(static_cast<int>(lock_type)), wait ? "true" : "false"});
    return acquire_lock_success_;
}

void TransactionManagerMock::release_lock(TransactionId txn_id, const std::string &resource) {
    RecordCall("release_lock", {std::to_string(txn_id), resource});
    // Mock实现：记录调用但不执行实际操作
}

bool TransactionManagerMock::detect_deadlock(TransactionId txn_id) {
    RecordCall("detect_deadlock", {std::to_string(txn_id)});
    return detect_deadlock_result_;
}

TransactionState TransactionManagerMock::get_transaction_state(TransactionId txn_id) const {
    RecordCall("get_transaction_state", {std::to_string(txn_id)});
    return get_transaction_state_result_;
}

std::vector<TransactionId> TransactionManagerMock::get_active_transactions() const {
    RecordCall("get_active_transactions");
    return get_active_transactions_result_;
}

void TransactionManagerMock::log_operation(TransactionId txn_id, const LogEntry &entry) {
    RecordCall("log_operation", {std::to_string(txn_id), entry.table_name, entry.operation});
    // Mock实现：记录调用但不执行实际操作
}

TransactionId TransactionManagerMock::next_transaction_id() {
    RecordCall("next_transaction_id");
    return next_transaction_id_result_;
}

void TransactionManagerMock::RecordCall(const std::string& method, const std::vector<std::string>& args) {
    CallRecord record;
    record.method_name = method;

    std::stringstream ss;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << args[i];
    }
    record.args = {ss.str()};

    call_history_.push_back(record);
}

} // namespace mocks
} // namespace sqlcc