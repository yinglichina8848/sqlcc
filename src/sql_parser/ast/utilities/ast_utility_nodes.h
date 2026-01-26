x/**
 * ASTUtilityNodes - 工具类AST节点头文件
 * 
 * 包含各种工具类AST节点定义，包括：
 * - 权限管理语句：GrantStatement, RevokeStatement
 * - 事务控制语句：CommitStatement, RollbackStatement, BeginStatement
 * - 其他辅助语句：UseStatement, ShowStatement, LoadDataStatement
 * 
 * 设计原则：
 * - 单一职责：专门处理工具类AST节点
 * - 模块化：按功能分类组织节点定义
 * - 类型安全：强类型系统防止运行时错误
 */

#ifndef SQLCC_SQL_PARSER_AST_UTILITIES_AST_UTILITY_NODES_H
#define SQLCC_SQL_PARSER_AST_UTILITIES_AST_UTILITY_NODES_H

#include "../ast_node.h"
#include "../statement.h"
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {
namespace sql_parser {

// ==================== GrantStatement ====================

class GrantStatement : public Statement {
public:
    GrantStatement();
    ~GrantStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std::vector<std::string> &getPrivileges() const { return privileges_; }
    const std::string &getObjectType() const { return objectType_; }
    const std::string &getObjectName() const { return objectName_; }
    const std::vector<std::string> &getUsers() const { return users_; }
    
    // Setters
    void setPrivileges(const std::vector<std::string> &privileges) { privileges_ = privileges; }
    void setObjectType(const std::string &type) { objectType_ = type; }
    void setObjectName(const std::string &name) { objectName_ = name; }
    void setUsers(const std::vector<std::string> &users) { users_ = users; }

private:
    std::vector<std::string> privileges_;
    std::string objectType_;
    std::string objectName_;
    std::vector<std::string> users_;
};

// ==================== RevokeStatement ====================

class RevokeStatement : public Statement {
public:
    RevokeStatement();
    ~RevokeStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std::vector<std::string> &getPrivileges() const { return privileges_; }
    const std::string &getObjectType() const { return objectType_; }
    const std::string &getObjectName() const { return objectName_; }
    const std::vector<std::string> &getUsers() const { return users_; }
    
    // Setters
    void setPrivileges(const std::vector<std::string> &privileges) { privileges_ = privileges; }
    void setObjectType(const std::string &type) { objectType_ = type; }
    void setObjectName(const std::string &name) { objectName_ = name; }
    void setUsers(const std::vector<std::string> &users) { users_ = users; }

private:
    std::vector<std::string> privileges_;
    std::string objectType_;
    std::string objectName_;
    std::vector<std::string> users_;
};

// ==================== CommitStatement ====================

class CommitStatement : public Statement {
public:
    CommitStatement();
    ~CommitStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std::string &getTransactionName() const { return transactionName_; }
    
    // Setters
    void setTransactionName(const std::string &name) { transactionName_ = name; }

private:
    std::string transactionName_;
};

// ==================== RollbackStatement ====================

class RollbackStatement : public Statement {
public:
    RollbackStatement();
    ~RollbackStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std::string &getTransactionName() const { return transactionName_; }
    
    // Setters
    void setTransactionName(const std::string &name) { transactionName_ = name; }

private:
    std::string transactionName_;
};

// ==================== BeginStatement ====================

class BeginStatement : public Statement {
public:
    BeginStatement();
    ~BeginStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std::string &getTransactionName() const { return transactionName_; }
    const std::string &getIsolationLevel() const { return isolationLevel_; }
    
    // Setters
    void setTransactionName(const std::string &name) { transactionName_ = name; }
    void setIsolationLevel(const std::string &level) { isolationLevel_ = level; }

private:
    std::string transactionName_;
    std::string isolationLevel_;
};

// ==================== UseStatement ====================

class UseStatement : public Statement {
public:
    UseStatement(const std::string &databaseName);
    ~UseStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std::string &getDatabaseName() const { return databaseName_; }
    
    // Setters
    void setDatabaseName(const std::string &name) { databaseName_ = name; }

private:
    std::string databaseName_;
};

// ==================== ShowStatement ====================

class ShowStatement : public Statement {
public:
    ShowStatement(const std::string &showType);
    ~ShowStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std::string &getShowType() const { return showType_; }
    const std::string &getObjectName() const { return objectName_; }
    
    // Setters
    void setShowType(const std::string &type) { showType_ = type; }
    void setObjectName(const std::string &name) { objectName_ = name; }

private:
    std::string showType_;
    std::string objectName_;
};

// ==================== LoadDataStatement ====================

class LoadDataStatement : public Statement {
public:
    LoadDataStatement(const std::string &fileName, const std::string &tableName);
    ~LoadDataStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std::string &getFileName() const { return fileName_; }
    const std::string &getTableName() const { return tableName_; }
    const std::vector<std::string> &getColumnNames() const { return columnNames_; }
    
    // Setters
    void setFileName(const std::string &name) { fileName_ = name; }
    void setTableName(const std::string &name) { tableName_ = name; }
    void setColumnNames(const std::vector<std::string> &columns) { columnNames_ = columns; }

private:
    std::string fileName_;
    std::string tableName_;
    std::vector<std::string> columnNames_;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_AST_UTILITIES_AST_UTILITY_NODES_H