#pragma once

#include <memory>
#include <unordered_map>
#include <string>

namespace sqlcc {
namespace sql_executor {

// Forward declarations
namespace sql_parser {
class SetTransactionStatement;
} // namespace sql_parser

/**
 * Transaction Control Manager - 事务控制管理器
 * 处理SAVEPOINT、SET TRANSACTION等事务控制语句
 */
class TransactionControlManager {
public:
    static TransactionControlManager& getInstance();

    // SAVEPOINT管理
    bool createSavepoint(const std::string& savepointName);
    bool releaseSavepoint(const std::string& savepointName);
    bool rollbackToSavepoint(const std::string& savepointName);
    bool savepointExists(const std::string& savepointName) const;

    // SET TRANSACTION
    bool setTransactionIsolation(sql_parser::SetTransactionStatement::IsolationLevel level);
    bool setTransactionAccessMode(sql_parser::SetTransactionStatement::AccessMode mode);
    sql_parser::SetTransactionStatement::IsolationLevel getCurrentIsolationLevel() const;
    sql_parser::SetTransactionStatement::AccessMode getCurrentAccessMode() const;

    // 事务统计
    std::string getTransactionInfo() const;

private:
    TransactionControlManager() = default;

    struct SavepointInfo {
        std::string name;
        long transaction_id;
        std::string created_by;
        long created_time;
    };

    std::unordered_map<std::string, SavepointInfo> savepoints_;
    sql_parser::SetTransactionStatement::IsolationLevel current_isolation_level_;
    sql_parser::SetTransactionStatement::AccessMode current_access_mode_;
    long current_transaction_id_ = 0;
    long next_savepoint_id_ = 1;
};

} // namespace sql_executor
} // namespace sqlcc
