#ifndef SQLCC_DATABASE_EXCEPTIONS_H
#define SQLCC_DATABASE_EXCEPTIONS_H

#include <exception>
#include <string>
#include <stdexcept>

namespace sqlcc {

// 数据库基础异常类
class DatabaseException : public std::runtime_error {
public:
    explicit DatabaseException(const std::string& message)
        : std::runtime_error(message) {}
};

// 数据库不存在异常
class DatabaseNotFoundException : public DatabaseException {
public:
    explicit DatabaseNotFoundException(const std::string& db_name)
        : DatabaseException("Database '" + db_name + "' does not exist") {}
};

// 数据库已存在异常
class DatabaseAlreadyExistsException : public DatabaseException {
public:
    explicit DatabaseAlreadyExistsException(const std::string& db_name)
        : DatabaseException("Database '" + db_name + "' already exists") {}
};

// 表不存在异常
class TableNotFoundException : public DatabaseException {
public:
    explicit TableNotFoundException(const std::string& table_name)
        : DatabaseException("Table '" + table_name + "' does not exist") {}
};

// 表已存在异常
class TableAlreadyExistsException : public DatabaseException {
public:
    explicit TableAlreadyExistsException(const std::string& table_name)
        : DatabaseException("Table '" + table_name + "' already exists") {}
};

// 数据库未选择异常
class NoDatabaseSelectedException : public DatabaseException {
public:
    NoDatabaseSelectedException()
        : DatabaseException("No database selected") {}
};

// 文件操作异常
class FileOperationException : public DatabaseException {
public:
    explicit FileOperationException(const std::string& operation, const std::string& file_path)
        : DatabaseException("File operation '" + operation + "' failed for: " + file_path) {}
};

// 页操作异常
class PageOperationException : public DatabaseException {
public:
    explicit PageOperationException(const std::string& operation, uint32_t page_id)
        : DatabaseException("Page operation '" + operation + "' failed for page " + std::to_string(page_id)) {}
};

// 事务异常
class TransactionException : public DatabaseException {
public:
    explicit TransactionException(const std::string& message)
        : DatabaseException("Transaction error: " + message) {}
};

// WAL异常
class WALException : public DatabaseException {
public:
    explicit WALException(const std::string& message)
        : DatabaseException("WAL error: " + message) {}
};

// 校验和异常
class ChecksumException : public DatabaseException {
public:
    explicit ChecksumException(uint32_t page_id)
        : DatabaseException("Checksum mismatch for page " + std::to_string(page_id)) {}
};

// 并发控制异常
class ConcurrencyException : public DatabaseException {
public:
    explicit ConcurrencyException(const std::string& message)
        : DatabaseException("Concurrency control error: " + message) {}
};

// 索引异常
class IndexException : public DatabaseException {
public:
    explicit IndexException(const std::string& message)
        : DatabaseException("Index error: " + message) {}
};

// 约束违反异常
class ConstraintViolationException : public DatabaseException {
public:
    explicit ConstraintViolationException(const std::string& constraint_type, const std::string& details)
        : DatabaseException("Constraint violation (" + constraint_type + "): " + details) {}
};

// SQL语法异常
class SQLSyntaxException : public DatabaseException {
public:
    explicit SQLSyntaxException(const std::string& sql, const std::string& reason)
        : DatabaseException("SQL syntax error in '" + sql + "': " + reason) {}
};

// 权限异常
class PermissionException : public DatabaseException {
public:
    explicit PermissionException(const std::string& operation, const std::string& resource)
        : DatabaseException("Permission denied for operation '" + operation + "' on '" + resource + "'") {}
};

} // namespace sqlcc

#endif // SQLCC_DATABASE_EXCEPTIONS_H