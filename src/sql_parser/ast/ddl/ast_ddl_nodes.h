/**
 * ASTDDLNodes - DDL相关AST节点头文件
 * 
 * 包含数据定义语言（DDL）相关的AST节点定义，包括：
 * - 基础结构类：ColumnDefinition, TableConstraint等
 * - DDL语句节点：CreateStatement, DropStatement, AlterStatement等
 * 
 * 设计原则：
 * - 单一职责：专门处理DDL相关AST节点
 * - 模块化：按功能分类组织节点定义
 * - 类型安全：强类型系统防止运行时错误
 */

#ifndef SQLCC_SQL_PARSER_AST_DDL_AST_DDL_NODES_H
#define SQLCC_SQL_PARSER_AST_DDL_AST_DDL_NODES_H

#include "../ast_node.h"
#include "../statement.h"
#include "../../data_types.h"
#include <memory>
#include <string>
#include <vector>

// Forward declarations
class SelectStatement;

namespace sqlcc {
namespace sql_parser {

// ==================== ColumnDefinition ====================

class ColumnDefinition {
public:
    ColumnDefinition(const std::string &name, const std::string &type);
    ~ColumnDefinition();

    // Getters
    const std::string &getName() const { return name_; }
    DataType getDataType() const { return dataType_; }
    const std::string &getTypeString() const { return type_; }
    bool isPrimaryKey() const { return isPrimaryKey_; }
    bool isNullable() const { return isNullable_; }
    bool isUnique() const { return isUnique_; }
    bool isAutoIncrement() const { return isAutoIncrement_; }
    bool isForeignKey() const { return isForeignKey_; }
    const std::string &getDefaultValue() const { return defaultValue_; }
    int getPrecision() const { return precision_; }
    int getScale() const { return scale_; }

    // Setters
    void setName(const std::string &name) { name_ = name; }
    void setType(const std::string &type);
    void setPrimaryKey(bool primaryKey = true) { isPrimaryKey_ = primaryKey; }
    void setNullable(bool nullable = true) { isNullable_ = nullable; }
    void setUnique(bool unique = true) { isUnique_ = unique; }
    void setForeignKey(bool foreignKey = true) { isForeignKey_ = foreignKey; }
    void setAutoIncrement(bool autoIncrement = true) { isAutoIncrement_ = autoIncrement; }
    void setDefaultValue(const std::string &defaultValue);
    void setPrecision(int precision) { precision_ = precision; }
    void setScale(int scale) { scale_ = scale; }

private:
    std::string name_;
    std::string type_;
    DataType dataType_;
    bool isPrimaryKey_;
    bool isNullable_;
    bool isUnique_;
    bool isForeignKey_;
    bool isAutoIncrement_;
    std::string defaultValue_;
    int precision_;
    int scale_;
};

// ==================== TableConstraint ====================

class TableConstraint {
public:
    enum Type { PRIMARY_KEY, FOREIGN_KEY, UNIQUE, CHECK };

    TableConstraint(Type type, const std::string &name = "");
    ~TableConstraint();

    // Getters
    Type getType() const { return type_; }
    const std::string &getConstraintName() const { return constraintName_; }
    const std::vector<std::string> &getColumns() const { return columns_; }
    const std::string &getReferencedTable() const { return referencedTable_; }
    const std::vector<std::string> &getReferencedColumns() const { return referencedColumns_; }
    const std::string &getCheckExpression() const { return checkExpression_; }

    // Setters
    void setConstraintName(const std::string &name) { constraintName_ = name; }
    void setType(Type type) { type_ = type; }
    void addColumn(const std::string &column) { columns_.push_back(column); }
    void setReferencedTable(const std::string &table) { referencedTable_ = table; }
    void addReferencedColumn(const std::string &column) { referencedColumns_.push_back(column); }
    void setCheckExpression(const std::string &expression) { checkExpression_ = expression; }

private:
    Type type_;
    std::string constraintName_;
    std::vector<std::string> columns_;
    std::string referencedTable_;
    std::vector<std::string> referencedColumns_;
    std::string checkExpression_;
};

// ==================== CreateStatement ====================

class CreateStatement : public Statement {
public:
    enum ObjectType { DATABASE, TABLE, INDEX, VIEW, PROCEDURE, TRIGGER };

    CreateStatement(ObjectType objectType);
    CreateStatement(ObjectType objectType, const std::string &objectName);
    ~CreateStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    ObjectType getObjectType() const { return objectType_; }
    const std::string &getObjectName() const { return objectName_; }
    const std::vector<std::unique_ptr<ColumnDefinition>> &getColumns() const { return columns_; }
    const std::vector<std::unique_ptr<TableConstraint>> &getConstraints() const { return constraints_; }
    const std::unique_ptr<SelectStatement> &getSelectStatement() const { return selectStatement_; }
    
    // Setters
    void setObjectType(ObjectType type) { objectType_ = type; }
    void setObjectName(const std::string &name) { objectName_ = name; }
    void addColumn(std::unique_ptr<ColumnDefinition> column) { columns_.push_back(std::move(column)); }
    void addConstraint(std::unique_ptr<TableConstraint> constraint) { constraints_.push_back(std::move(constraint)); }
    void setSelectStatement(std::unique_ptr<SelectStatement> select);

private:
    ObjectType objectType_;
    std::string objectName_;
    std::vector<std::unique_ptr<ColumnDefinition>> columns_;
    std::vector<std::unique_ptr<TableConstraint>> constraints_;
    std::unique_ptr<SelectStatement> selectStatement_;
};

// ==================== DropStatement ====================

class DropStatement : public Statement {
public:
    enum ObjectType { DATABASE, TABLE, INDEX, VIEW, PROCEDURE, TRIGGER, USER };

    DropStatement(ObjectType objectType, const std::string &objectName = "");
    ~DropStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    ObjectType getObjectType() const { return objectType_; }
    const std::string &getObjectName() const { return objectName_; }
    bool isIfExists() const { return ifExists_; }
    
    // Setters
    void setObjectType(ObjectType type) { objectType_ = type; }
    void setObjectName(const std::string &name) { objectName_ = name; }
    void setIfExists(bool ifExists) { ifExists_ = ifExists; }

private:
    ObjectType objectType_;
    std::string objectName_;
    bool ifExists_;
};

// ==================== AlterStatement ====================

class AlterStatement : public Statement {
public:
    enum ObjectType { DATABASE, TABLE };

    AlterStatement(ObjectType objectType);
    AlterStatement(ObjectType objectType, const std::string &objectName);
    ~AlterStatement() override;
    
    void accept(NodeVisitor &visitor) override;
    
    // Getters
    ObjectType getObjectType() const { return objectType_; }
    const std::string &getObjectName() const { return objectName_; }
    const std::string &getAlterType() const { return alterType_; }
    const std::unique_ptr<ColumnDefinition> &getColumnDefinition() const { return columnDefinition_; }
    const std::unique_ptr<TableConstraint> &getTableConstraint() const { return tableConstraint_; }
    
    // Setters
    void setObjectType(ObjectType type) { objectType_ = type; }
    void setObjectName(const std::string &name) { objectName_ = name; }
    void setTableName(const std::string &name) { setObjectName(name); }
    void setAlterType(const std::string &type) { alterType_ = type; }
    void setColumnDefinition(std::unique_ptr<ColumnDefinition> column) { columnDefinition_ = std::move(column); }
    void setTableConstraint(std::unique_ptr<TableConstraint> constraint) { tableConstraint_ = std::move(constraint); }

private:
    ObjectType objectType_;
    std::string objectName_;
    std::string alterType_;
    std::unique_ptr<ColumnDefinition> columnDefinition_;
    std::unique_ptr<TableConstraint> tableConstraint_;
};

// ==================== CreateViewStatement ====================

class CreateViewStatement : public Statement {
public:
    CreateViewStatement(const std::string &viewName);
    ~CreateViewStatement() override;

    void accept(NodeVisitor &visitor) override;

    const std::string &getViewName() const;
    const std::vector<std::string> &getColumnNames() const;
    const SelectStatement &getSelectStatement() const;

    void addColumnName(const std::string &columnName);
    void setSelectStatement(std::unique_ptr<SelectStatement> selectStmt);

    bool hasColumnNames() const;

private:
    std::string viewName_;
    std::vector<std::string> columnNames_;
    std::unique_ptr<SelectStatement> selectStatement_;
};

// ==================== User Management ====================

class CreateUserStatement : public Statement {
public:
    CreateUserStatement(const std::string &userName, const std::string &password);
    ~CreateUserStatement() override;
    void accept(NodeVisitor &visitor) override;

    const std::string &getUserName() const { return userName_; }
    const std::string &getPassword() const { return password_; }

private:
    std::string userName_;
    std::string password_;
};

class DropUserStatement : public Statement {
public:
    DropUserStatement(const std::string &userName);
    ~DropUserStatement() override;
    void accept(NodeVisitor &visitor) override;

    const std::string &getUserName() const { return userName_; }
    bool isIfExists() const { return ifExists_; }
    
    void setIfExists(bool ifExists) { ifExists_ = ifExists; }

private:
    std::string userName_;
    bool ifExists_ = false;
};

// ==================== Procedure Management ====================

class CreateProcedureStatement : public Statement {
public:
    CreateProcedureStatement(const std::string &procedureName);
    ~CreateProcedureStatement() override;
    void accept(NodeVisitor &visitor) override;

    const std::string &getProcedureName() const { return procedureName_; }

private:
    std::string procedureName_;
};

class DropProcedureStatement : public Statement {
public:
    DropProcedureStatement(const std::string &procedureName);
    ~DropProcedureStatement() override;
    void accept(NodeVisitor &visitor) override;

    const std::string &getProcedureName() const { return procedureName_; }

private:
    std::string procedureName_;
};

// ==================== Trigger Management ====================

class CreateTriggerStatement : public Statement {
public:
    CreateTriggerStatement(const std::string &triggerName);
    ~CreateTriggerStatement() override;
    void accept(NodeVisitor &visitor) override;

    const std::string &getTriggerName() const { return triggerName_; }

private:
    std::string triggerName_;
};

class DropTriggerStatement : public Statement {
public:
    DropTriggerStatement(const std::string &triggerName);
    ~DropTriggerStatement() override;
    void accept(NodeVisitor &visitor) override;

    const std::string &getTriggerName() const { return triggerName_; }

private:
    std::string triggerName_;
};

// ==================== Index Management ====================

class CreateIndexStatement : public Statement {
public:
    CreateIndexStatement(const std::string &indexName,
                       const std::string &tableName,
                       const std::string &columnName);
    ~CreateIndexStatement() override;

    void accept(NodeVisitor &visitor) override;

    const std::string &getIndexName() const { return indexName_; }
    const std::string &getTableName() const { return tableName_; }
    const std::string &getColumnName() const { return columnName_; }
    void addColumn(const std::string &column) { columns_.push_back(column); }
    const std::vector<std::string> &getColumns() const { return columns_; }

    void setUnique(bool unique) { unique_ = unique; }
    bool isUnique() const { return unique_; }

private:
    std::string indexName_;
    std::string tableName_;
    std::string columnName_;
    std::vector<std::string> columns_;
    bool unique_ = false;
};

class DropIndexStatement : public Statement {
public:
    DropIndexStatement(const std::string &indexName);
    ~DropIndexStatement() override;

    void accept(NodeVisitor &visitor) override;

    const std::string &getIndexName() const { return indexName_; }
    void setTableName(const std::string &tableName) { tableName_ = tableName; hasTableName_ = true; }
    const std::string &getTableName() const { return tableName_; }
    bool hasTableName() const { return hasTableName_; }

    void setIfExists(bool ifExists) { ifExists_ = ifExists; }
    bool isIfExists() const { return ifExists_; }

private:
    std::string indexName_;
    std::string tableName_;
    bool ifExists_ = false;
    bool hasTableName_ = false;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_AST_DDL_AST_DDL_NODES_H