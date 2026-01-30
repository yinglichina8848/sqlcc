#pragma once

#include <string>
#include <memory>

namespace sqlcc {

class Transaction {
public:
    virtual ~Transaction() = default;

    virtual bool Begin() = 0;
    virtual bool Commit() = 0;
    virtual bool Rollback() = 0;
    virtual bool IsActive() const = 0;
    virtual std::string GetId() const = 0;
};

class TransactionFactory {
public:
    virtual ~TransactionFactory() = default;
    virtual std::unique_ptr<Transaction> CreateTransaction() = 0;
};

} // namespace sqlcc
