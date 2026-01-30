#include "src/transaction_context_impl.h"
#include <sstream>
#include <stdexcept>

namespace sqlcc {

TransactionContextImpl::TransactionContextImpl(TransactionManager& transaction_manager)
    : transaction_manager_(transaction_manager) {
}

std::string TransactionContextImpl::beginTransaction() {
    try {
        TransactionId txn_id = transaction_manager_.begin_transaction();
        return txnIdToString(txn_id);
    } catch (const std::exception& e) {
        // 记录错误日志
        return "";
    }
}

bool TransactionContextImpl::commitTransaction(const std::string& transaction_id) {
    try {
        TransactionId txn_id = stringToTxnId(transaction_id);
        return transaction_manager_.commit_transaction(txn_id);
    } catch (const std::exception& e) {
        // 记录错误日志
        return false;
    }
}

bool TransactionContextImpl::rollbackTransaction(const std::string& transaction_id) {
    try {
        TransactionId txn_id = stringToTxnId(transaction_id);
        return transaction_manager_.rollback_transaction(txn_id);
    } catch (const std::exception& e) {
        // 记录错误日志
        return false;
    }
}

bool TransactionContextImpl::isTransactionActive(const std::string& transaction_id) {
    try {
        TransactionId txn_id = stringToTxnId(transaction_id);
        TransactionState state = transaction_manager_.get_transaction_state(txn_id);
        return state == TransactionState::ACTIVE;
    } catch (const std::exception& e) {
        // 记录错误日志
        return false;
    }
}

TransactionId TransactionContextImpl::stringToTxnId(const std::string& id) const {
    try {
        return std::stoull(id);
    } catch (const std::exception& e) {
        throw std::invalid_argument("Invalid transaction ID format: " + id);
    }
}

std::string TransactionContextImpl::txnIdToString(TransactionId id) const {
    return std::to_string(id);
}

} // namespace sqlcc
