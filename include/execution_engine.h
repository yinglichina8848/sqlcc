#ifndef SQLCC_EXECUTION_ENGINE_H
#define SQLCC_EXECUTION_ENGINE_H

#include "sql_parser/ast_nodes.h"
#include "user_manager.h"
#include "system_database.h"
#include "table_storage.h"
#include "storage_engine.h"
#include <memory>
#include <string>

namespace sqlcc {

class DatabaseManager;

/**
 * @brief 执行结果类
 */
struct ExecutionResult {
    enum Status { SUCCESS, FAILURE };

    bool success;
    std::string message;
    
    ExecutionResult(bool success = true, const std::string& message = "");

    Status getStatus() const { return success ? SUCCESS : FAILURE; }
    const std::string& getMessage() const { return message; }
};

/**
 * @brief 执行引擎接口
 */
class ExecutionEngine {
protected:
    std::shared_ptr<DatabaseManager> db_manager_;

public:
    ExecutionEngine(std::shared_ptr<DatabaseManager> db_manager);
    virtual ~ExecutionEngine() = default;
    
    /**
     * 执行SQL语句
     */
    virtual ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt) = 0;
};

/**
 * @brief DDL执行器 - 处理数据定义语言
 */
class DDLExecutor : public ExecutionEngine {
public:
    DDLExecutor(std::shared_ptr<DatabaseManager> db_manager);

    ExecutionResult execute(std::unique_ptr<sqlcc::sql_parser::Statement> stmt) override;

private:
    ExecutionResult executeCreate(sqlcc::sql_parser::CreateStatement* stmt);
    ExecutionResult executeDrop(sqlcc::sql_parser::DropStatement* stmt);
    ExecutionResult executeAlter(sqlcc::sql_parser::AlterStatement* stmt);
    ExecutionResult executeCreateIndex(sqlcc::sql_parser::CreateIndexStatement* stmt);
    ExecutionResult executeDropIndex(sqlcc::sql_parser::DropIndexStatement* stmt);
};

/**
 * @brief DML执行器 - 处理数据操作语言
 */
class DMLExecutor : public ExecutionEngine {
public:
    DMLExecutor(std::shared_ptr<DatabaseManager> db_manager);

    ExecutionResult execute(std::unique_ptr<sqlcc::sql_parser::Statement> stmt) override;
    
    // 公开的辅助方法（用于WHERE条件评估，可以被外部访问）
    bool compareValues(const std::string& left, const std::string& right, const std::string& op);

private:
    ExecutionResult executeInsert(sqlcc::sql_parser::InsertStatement* stmt);
    ExecutionResult executeUpdate(sqlcc::sql_parser::UpdateStatement* stmt);
    ExecutionResult executeDelete(sqlcc::sql_parser::DeleteStatement* stmt);
    
    // 辅助方法
    bool matchesWhereClause(const std::vector<std::string>& record,
                           const sqlcc::sql_parser::WhereClause& where_clause,
                           std::shared_ptr<TableMetadata> metadata);
    std::string getColumnValue(const std::vector<std::string>& record,
                              const std::string& column_name,
                              std::shared_ptr<TableMetadata> metadata);
    
    // WHERE条件评估辅助方法
    // TODO: 支持AND/OR组合条件
    // TODO: 支持IN操作符
    // TODO: 支持BETWEEN操作符
    // TODO: 支持LIKE模式匹配
    
    // 约束验证方法
    bool validateColumnConstraints(const std::vector<std::string>& record,
                                  std::shared_ptr<TableMetadata> metadata,
                                  const std::string& table_name);
    bool checkUniqueConstraints(const std::vector<std::string>& record,
                               std::shared_ptr<TableMetadata> metadata,
                               const std::string& table_name);
    bool checkPrimaryKeyConstraints(const std::vector<std::string>& record,
                                   std::shared_ptr<TableMetadata> metadata,
                                   const std::string& table_name);
    bool checkUniqueKeyConstraints(const std::vector<std::string>& record,
                                  std::shared_ptr<TableMetadata> metadata,
                                  const std::string& table_name);
        
    // 索引维护方法
    void maintainIndexesOnInsert(const std::vector<std::string>& record,
                                const std::string& table_name,
                                int32_t page_id, size_t offset);
    void maintainIndexesOnUpdate(const std::vector<std::string>& old_record,
                                const std::vector<std::string>& new_record,
                                const std::string& table_name,
                                int32_t page_id, size_t offset);
    void maintainIndexesOnDelete(const std::vector<std::string>& record,
                                const std::string& table_name,
                                int32_t page_id, size_t offset);
};


/**
 * @brief DCL执行器 - 处理数据控制语言
 */
class DCLExecutor : public ExecutionEngine {
public:
    DCLExecutor(std::shared_ptr<DatabaseManager> db_manager, std::shared_ptr<UserManager> user_manager);
    
    ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt) override;
    
private:
    ExecutionResult executeCreateUser(sql_parser::CreateUserStatement* stmt);
    ExecutionResult executeDropUser(sql_parser::DropUserStatement* stmt);
    ExecutionResult executeGrant(sql_parser::GrantStatement* stmt);
    ExecutionResult executeRevoke(sql_parser::RevokeStatement* stmt);
    
    std::shared_ptr<UserManager> user_manager_;
};

/**
 * @brief 工具执行器 - 处理USE, SHOW等语句
 */
class UtilityExecutor : public ExecutionEngine {
public:
    UtilityExecutor(std::shared_ptr<DatabaseManager> db_manager);
    UtilityExecutor(std::shared_ptr<DatabaseManager> db_manager, std::shared_ptr<SystemDatabase> system_db);
    
    ExecutionResult execute(std::unique_ptr<sql_parser::Statement> stmt) override;
    
private:
    ExecutionResult executeShow(sql_parser::ShowStatement* stmt);
    std::string formatDatabases(const std::vector<std::string>& databases);
    std::string formatTables(const std::vector<std::string>& tables);
    
    std::shared_ptr<SystemDatabase> system_db_;
};

} // namespace sqlcc

#endif // SQLCC_EXECUTION_ENGINE_H