/**
 * ASTDMLNodes - DML相关AST节点头文件
 * 
 * 包含数据操作语言（DML）相关的AST节点定义，包括：
 * - DML语句节点：SelectStatement, InsertStatement, UpdateStatement, DeleteStatement等
 * - 子句节点：WhereClause, JoinClause, SetOperation等
 * 
 * 设计原则：
 * - 单一职责：专门处理DML相关AST节点
 * - 模块化：按功能分类组织节点定义
 * - 类型安全：强类型系统防止运行时错误
 */

#ifndef SQLCC_SQL_PARSER_AST_DML_AST_DML_NODES_H
#define SQLCC_SQL_PARSER_AST_DML_AST_DML_NODES_H

#include "../ast_node.h"
#include "../statement.h"
#include "../expression.h"
#include "../set_operation.h"
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {
namespace sql_parser {

// ==================== WhereClause ====================

class WhereClause {
public:
    WhereClause(std::unique_ptr<Expression> condition);
    ~WhereClause();

    // Getters
    const std::unique_ptr<Expression> &getCondition() const { return condition_; }
    
    // Setters
    void setCondition(std::unique_ptr<Expression> condition);

private:
    std::unique_ptr<Expression> condition_;
};

// ==================== JoinClause ====================

class JoinClause {
public:
    // Join类型枚举
    enum JoinType {
        INNER_JOIN,
        LEFT_JOIN,
        RIGHT_JOIN,
        FULL_JOIN
    };
    
    JoinClause(JoinType type, const std::string &tableName, std::unique_ptr<Expression> condition = nullptr);
    ~JoinClause();

    // Getters
    const std::string &getTableName() const { return tableName_; }
    JoinType getType() const { return type_; }
    const std::unique_ptr<Expression> &getCondition() const { return condition_; }
    
    // Setters
    void setTableName(const std::string &name) { tableName_ = name; }
    void setType(JoinType type) { type_ = type; }
    void setCondition(std::unique_ptr<Expression> condition);

private:
    std::string tableName_;
    JoinType type_;
    std::unique_ptr<Expression> condition_;
};

// ==================== SelectStatement ====================

class SelectStatement : public Statement {
public:
  SelectStatement();
  ~SelectStatement() override;
  
  void accept(ast::NodeVisitor& visitor) override;
  
  // Getters
  const std::string &getTableName() const { return tableName_; }
  const std::vector<std::unique_ptr<Expression>> &getSelectList() const { return selectList_; }
  const std::vector<std::string> &getFromTables() const { return fromTables_; }
  const std::unique_ptr<WhereClause> &getWhereClause() const { return whereClause_; }
  const std::vector<std::unique_ptr<JoinClause>> &getJoinClauses() const { return joinClauses_; }
  const std::vector<std::unique_ptr<Expression>> &getGroupBy() const { return groupBy_; }
  const std::unique_ptr<Expression> &getHaving() const { return having_; }
  const std::vector<std::unique_ptr<Expression>> &getOrderBy() const { return orderBy_; }
  bool isDistinct() const { return distinct_; }
  bool isSelectAll() const { return selectAll_; }
  
  // Setters
  void setSelectList(std::vector<std::unique_ptr<Expression>> selectList);
  void setFromTables(const std::vector<std::string> &tables) { fromTables_ = tables; }
  void setWhereClause(std::unique_ptr<WhereClause> where);
  void setJoinClauses(std::vector<std::unique_ptr<JoinClause>> joins);
  void setGroupBy(std::vector<std::unique_ptr<Expression>> groupBy);
  void setHaving(std::unique_ptr<Expression> having);
  void setOrderBy(std::vector<std::unique_ptr<Expression>> orderBy);
  void setDistinct(bool distinct) { distinct_ = distinct; }
  void setSelectAll(bool selectAll) { selectAll_ = selectAll; }
  
  // Additional methods used by parser
  void setTableName(const std::string &name) { tableName_ = name; }
  void addFromTable(const std::string &table) { fromTables_.push_back(table); }
  void addJoinClause(std::unique_ptr<JoinClause> join) { joinClauses_.push_back(std::move(join)); }
  void addSelectColumn(std::unique_ptr<Expression> column) { selectList_.push_back(std::move(column)); }
  void addGroupByColumn(const std::string &column);
  void setOrderByColumn(const std::string &column);
  void setOrderDirection(const std::string &direction);

private:
  std::string tableName_; // 表名
  std::vector<std::unique_ptr<Expression>> selectList_;
  std::vector<std::string> fromTables_;
  std::unique_ptr<WhereClause> whereClause_;
  std::vector<std::unique_ptr<JoinClause>> joinClauses_;
  std::vector<std::unique_ptr<Expression>> groupBy_;
  std::unique_ptr<Expression> having_;
  std::vector<std::unique_ptr<Expression>> orderBy_;
  std::string orderDirection_; // 排序方向: ASC或DESC
  bool distinct_;
  bool selectAll_;
};

// ==================== InsertStatement ====================

class InsertStatement : public Statement {
public:
    InsertStatement(const std::string &tableName);
    ~InsertStatement() override;
    
    void accept(ast::NodeVisitor& visitor) override;
    
    // Getters
    const std::string &getTableName() const { return tableName_; }
    const std::vector<std::string> &getColumnNames() const { return columnNames_; }
    const std::vector<std::vector<std::unique_ptr<Expression>>> &getValues() const { return values_; }
    const std::unique_ptr<SelectStatement> &getSelectStatement() const { return selectStatement_; }
    
    // Setters
    void setTableName(const std::string &name) { tableName_ = name; }
    void setColumnNames(const std::vector<std::string> &columns) { columnNames_ = columns; }
    void setValues(std::vector<std::vector<std::unique_ptr<Expression>>> values);
    void setSelectStatement(std::unique_ptr<SelectStatement> select);
    
    // Additional methods used by parser
    void addColumn(const std::string &column);
    void addValue(const std::string &value);
    void finishRow();

private:
    std::string tableName_;
    std::vector<std::string> columnNames_;
    std::vector<std::vector<std::unique_ptr<Expression>>> values_;
    std::unique_ptr<SelectStatement> selectStatement_;
};

// ==================== UpdateStatement ====================

class UpdateStatement : public Statement {
public:
    UpdateStatement(const std::string &tableName);
    ~UpdateStatement() override;
    
    void accept(ast::NodeVisitor& visitor) override;
    
    // Getters
    const std::string &getTableName() const { return tableName_; }
    const std::vector<std::pair<std::string, std::unique_ptr<Expression>>> &getAssignments() const { return assignments_; }
    const std::unique_ptr<WhereClause> &getWhereClause() const { return whereClause_; }
    
    // Setters
    void setTableName(const std::string &name) { tableName_ = name; }
    void setAssignments(std::vector<std::pair<std::string, std::unique_ptr<Expression>>> assignments);
    void setWhereClause(std::unique_ptr<WhereClause> where);

private:
    std::string tableName_;
    std::vector<std::pair<std::string, std::unique_ptr<Expression>>> assignments_;
    std::unique_ptr<WhereClause> whereClause_;
};

// ==================== DeleteStatement ====================

class DeleteStatement : public Statement {
public:
    DeleteStatement(const std::vector<std::string> &tableNames);
    ~DeleteStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    const std::vector<std::string> &getTableNames() const { return tableNames_; }
    const std::unique_ptr<WhereClause> &getWhereClause() const { return whereClause_; }
    
    // Setters
    void setTableNames(const std::vector<std::string> &tables) { tableNames_ = tables; }
    void setWhereClause(std::unique_ptr<WhereClause> where);

private:
    std::vector<std::string> tableNames_;
    std::unique_ptr<WhereClause> whereClause_;
};

// SetOperation类已经在set_operation.h中定义，此处不再重复定义

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_AST_DML_AST_DML_NODES_H