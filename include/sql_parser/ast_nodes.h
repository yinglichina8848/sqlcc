#ifndef SQLCC_SQL_PARSER_AST_NODES_H
#define SQLCC_SQL_PARSER_AST_NODES_H

#include "ast_node.h"
#include "node_visitor.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace sqlcc {
namespace sql_parser {

// Forward declarations
class Statement;
class CreateStatement;
class SelectStatement;
class InsertStatement;
class UpdateStatement;
class DeleteStatement;
class DropStatement;
class AlterStatement;
class UseStatement;
class CreateIndexStatement;
class DropIndexStatement;
class CreateUserStatement;
class DropUserStatement;
class GrantStatement;
class RevokeStatement;
class ShowStatement;
class NodeVisitor;

// ==================== ColumnDefinition ====================

class ColumnDefinition {
public:
    ColumnDefinition(const std::string& name, const std::string& type);
    ~ColumnDefinition(); // 显式声明析构函数
    
    // Getters
    const std::string& getName() const { return name_; }
    const std::string& getType() const { return type_; }
    bool isPrimaryKey() const { return isPrimaryKey_; }
    bool isNullable() const { return isNullable_; }
    bool isUnique() const { return isUnique_; }
    bool isAutoIncrement() const { return isAutoIncrement_; }
    const std::string& getDefaultValue() const { return defaultValue_; }
    
    // Setters
    void setName(const std::string& name) { name_ = name; }
    void setType(const std::string& type) { type_ = type; }
    void setPrimaryKey(bool primaryKey = true) { isPrimaryKey_ = primaryKey; }
    void setNullable(bool nullable = true) { isNullable_ = nullable; }
    void setUnique(bool unique = true) { isUnique_ = unique; }
    void setAutoIncrement(bool autoIncrement = true) { isAutoIncrement_ = autoIncrement; }
    void setDefaultValue(const std::string& defaultValue);
    
    // 兼容旧方法名
    void setIsPrimaryKey(bool isPrimaryKey) { setPrimaryKey(isPrimaryKey); }
    void setIsNullable(bool isNullable) { setNullable(isNullable); }
    void setIsUnique(bool isUnique) { setUnique(isUnique); }
    void setIsAutoIncrement(bool isAutoIncrement) { setAutoIncrement(isAutoIncrement); }

private:
    std::string name_;
    std::string type_;
    bool isPrimaryKey_;
    bool isNullable_;
    bool isUnique_;
    bool isAutoIncrement_;
    std::string defaultValue_;
};

// ==================== TableConstraint ====================

class TableConstraint {
public:
    enum Type {
        PRIMARY_KEY,
        FOREIGN_KEY,
        UNIQUE,
        CHECK
    };

    TableConstraint(Type type, const std::string& name = "");
    ~TableConstraint();

    Type getType() const;
    const std::string& getConstraintName() const;
    const std::vector<std::string>& getColumns() const;
    const std::string& getReferencedTable() const;
    const std::vector<std::string>& getReferencedColumns() const;
    const std::string& getCheckExpression() const;

    void addColumn(const std::string& column);
    void setReferencedTable(const std::string& table);
    void addReferencedColumn(const std::string& column);
    void setCheckExpression(const std::string& expression);

private:
    Type type_;
    std::string constraintName_;
    std::vector<std::string> columns_;
    std::string referencedTable_;
    std::vector<std::string> referencedColumns_;
    std::string checkExpression_;
};

// ==================== WhereClause ====================

class WhereClause {
public:
    WhereClause(const std::string& columnName, const std::string& op, const std::string& value);
    ~WhereClause();

    const std::string& getColumnName() const;
    const std::string& getOp() const;
    const std::string& getValue() const;

private:
    std::string columnName_;
    std::string op_;
    std::string value_;
};

// ==================== CreateStatement ====================

class CreateStatement : public Statement {
public:
    enum ObjectType {
        DATABASE,
        TABLE,
        INDEX
    };

    CreateStatement(ObjectType objectType, const std::string& objectName);
    CreateStatement(ObjectType objectType); // 兼容旧用法：后续通过setter设置名称
    ~CreateStatement();

    ObjectType getObjectType() const;
    const std::string& getObjectName() const;
    const std::vector<ColumnDefinition>& getColumns() const;
    const std::vector<TableConstraint>& getConstraints() const;

    void addColumn(ColumnDefinition&& column);
    void addConstraint(TableConstraint&& constraint);
    
    // 兼容旧测试API的setter
    void setObjectName(const std::string& name) { objectName_ = name; }
    void setDatabaseName(const std::string& name) { objectName_ = name; }
    void setTableName(const std::string& name) { objectName_ = name; }
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    ObjectType objectType_;
    std::string objectName_;
    std::vector<ColumnDefinition> columns_;
    std::vector<TableConstraint> constraints_;
};

// ==================== SelectStatement ====================

class SelectStatement : public Statement {
public:
    SelectStatement();
    ~SelectStatement();

    void addSelectColumn(const std::string& column);
    void setTableName(const std::string& table);
    void setWhereClause(const WhereClause& where);
    void setGroupByColumn(const std::string& column);
    void setOrderByColumn(const std::string& column);
    void setOrderDirection(const std::string& direction);
    void setSelectAll(bool selectAll);
    void setJoinCondition(const std::string& condition);
    void setLimit(int limit);
    void setOffset(int offset);

    const std::vector<std::string>& getSelectColumns() const;
    const std::string& getTableName() const;
    const WhereClause& getWhereClause() const;
    const std::string& getGroupByColumn() const;
    const std::string& getOrderByColumn() const;
    const std::string& getOrderDirection() const;
    const std::string& getJoinCondition() const;
    int getLimit() const;
    int getOffset() const;
    bool isSelectAll() const;
    bool hasWhereClause() const;
    bool hasGroupBy() const;
    bool hasOrderBy() const;
    bool hasJoinCondition() const;
    bool hasLimit() const;
    bool hasOffset() const;
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    std::vector<std::string> selectColumns_;
    std::string tableName_;
    WhereClause whereClause_{"", "", ""}; // 初始化空的WhereClause
    std::string groupByColumn_;
    std::string orderByColumn_;
    std::string orderDirection_;
    std::string joinCondition_;
    int limit_;
    int offset_;
    bool selectAll_;
    bool hasLimit_;
    bool hasOffset_;
};

// ==================== InsertStatement ====================

class InsertStatement : public Statement {
public:
    InsertStatement(const std::string& tableName);
    ~InsertStatement();

    void addColumn(const std::string& column);
    void addValue(const std::string& value);
    void finishRow();
    void addValueRow(const std::vector<std::unique_ptr<Expression>>& values);

    const std::string& getTableName() const;
    const std::vector<std::string>& getColumns() const;
    const std::vector<std::vector<std::string>>& getValues() const;
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    std::string tableName_;
    std::vector<std::string> columns_;
    std::vector<std::string> currentRow_;
    std::vector<std::vector<std::string>> values_;
};

// ==================== UpdateStatement ====================

class UpdateStatement : public Statement {
public:
    UpdateStatement(const std::string& tableName);
    ~UpdateStatement();

    void addUpdateValue(const std::string& column, const std::string& value);
    void setWhereClause(const WhereClause& where);

    const std::string& getTableName() const;
    const std::unordered_map<std::string, std::string>& getUpdateValues() const;
    const WhereClause& getWhereClause() const;
    bool hasWhereClause() const;
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    std::string tableName_;
    std::unordered_map<std::string, std::string> updateValues_;
    WhereClause whereClause_{"", "", ""}; // 初始化空的WhereClause
};

// ==================== DeleteStatement ====================

class DeleteStatement : public Statement {
public:
    DeleteStatement(const std::string& tableName);
    ~DeleteStatement();

    void setWhereClause(const WhereClause& where);

    const std::string& getTableName() const;
    const WhereClause& getWhereClause() const;
    bool hasWhereClause() const;
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    std::string tableName_;
    WhereClause whereClause_{"", "", ""}; // 初始化空的WhereClause
};

// ==================== DropStatement ====================

class DropStatement : public Statement {
public:
    enum ObjectType {
        DATABASE,
        TABLE,
        INDEX
    };

    DropStatement(ObjectType objectType, const std::string& objectName);
    DropStatement(ObjectType objectType); // 兼容旧用法：后续通过setter设置名称
    ~DropStatement();

    ObjectType getObjectType() const;
    const std::string& getObjectName() const;
    bool isIfExists() const;
    void setIfExists(bool ifExists);
    // 兼容旧测试API的setter
    void setObjectName(const std::string& name) { objectName_ = name; }
    void setDatabaseName(const std::string& name) { objectName_ = name; }
    void setTableName(const std::string& name) { objectName_ = name; }
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    ObjectType objectType_;
    std::string objectName_;
    bool ifExists_;
};

// ==================== AlterStatement ====================

class AlterStatement : public Statement {
public:
    enum ObjectType {
        DATABASE,
        TABLE
    };

    AlterStatement(ObjectType objectType, const std::string& objectName);
    AlterStatement(ObjectType objectType); // 兼容旧用法：后续通过setter设置名称
    ~AlterStatement();

    ObjectType getObjectType() const;
    const std::string& getObjectName() const;
    // 兼容旧测试API的setter
    void setObjectName(const std::string& name) { objectName_ = name; }
    void setDatabaseName(const std::string& name) { objectName_ = name; }
    void setTableName(const std::string& name) { objectName_ = name; }
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    ObjectType objectType_;
    std::string objectName_;
};

// ==================== UseStatement ====================

class UseStatement : public Statement {
public:
    UseStatement(const std::string& databaseName);
    ~UseStatement();

    const std::string& getDatabaseName() const;
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    std::string databaseName_;
};

// ==================== CreateIndexStatement ====================

class CreateIndexStatement : public Statement {
public:
    CreateIndexStatement(const std::string& indexName, const std::string& tableName, const std::string& columnName);
    ~CreateIndexStatement();

    const std::string& getIndexName() const;
    const std::string& getTableName() const;
    const std::string& getColumnName() const;
    void addColumn(const std::string& column);
    const std::vector<std::string>& getColumns() const;
    
    void setUnique(bool unique);  // 设置UNIQUE标记
    bool isUnique() const;         // 获取UNIQUE标记
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    std::string indexName_;
    std::string tableName_;
    std::vector<std::string> columns_;
    bool unique_;  // 是否为UNIQUE索引
};

// ==================== DropIndexStatement ====================

class DropIndexStatement : public Statement {
public:
    DropIndexStatement(const std::string& indexName);
    ~DropIndexStatement();

    const std::string& getIndexName() const;
    
    void setTableName(const std::string& tableName);  // 设置表名
    const std::string& getTableName() const;          // 获取表名
    bool hasTableName() const;                        // 是否指定了表名
    
    void setIfExists(bool ifExists);  // 设置IF EXISTS标记
    bool isIfExists() const;          // 获取IF EXISTS标记
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    std::string indexName_;
    std::string tableName_;  // 表名（可选）
    bool ifExists_;          // IF EXISTS标记
    bool hasTableName_;      // 是否指定表名
};

// ==================== CreateUserStatement ====================

class CreateUserStatement : public Statement {
public:
    CreateUserStatement(const std::string& username, const std::string& password);
    ~CreateUserStatement();

    const std::string& getUsername() const;
    const std::string& getPassword() const;
    bool isWithPassword() const;
    void setWithPassword(bool withPassword);
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    std::string username_;
    std::string password_;
    bool withPassword_;
};

// ==================== DropUserStatement ====================

class DropUserStatement : public Statement {
public:
    DropUserStatement(const std::string& username);
    ~DropUserStatement();

    const std::string& getUsername() const;
    bool isIfExists() const;
    void setIfExists(bool ifExists);
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    std::string username_;
    bool ifExists_;
};

// ==================== GrantStatement ====================

class GrantStatement : public Statement {
public:
    GrantStatement();
    ~GrantStatement();

    void addPrivilege(const std::string& privilege);
    const std::vector<std::string>& getPrivileges() const;

    void setObjectType(const std::string& objectType);
    const std::string& getObjectType() const;

    void setObjectName(const std::string& objectName);
    const std::string& getObjectName() const;

    void setGrantee(const std::string& grantee);
    const std::string& getGrantee() const;
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    std::vector<std::string> privileges_;
    std::string objectType_;
    std::string objectName_;
    std::string grantee_;
};

// ==================== RevokeStatement ====================

class RevokeStatement : public Statement {
public:
    RevokeStatement();
    ~RevokeStatement();

    void addPrivilege(const std::string& privilege);
    const std::vector<std::string>& getPrivileges() const;

    void setObjectType(const std::string& objectType);
    const std::string& getObjectType() const;

    void setObjectName(const std::string& objectName);
    const std::string& getObjectName() const;

    void setGrantee(const std::string& grantee);
    const std::string& getGrantee() const;
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    std::vector<std::string> privileges_;
    std::string objectType_;
    std::string objectName_;
    std::string grantee_;
};

// ==================== ShowStatement ====================

class ShowStatement : public Statement {
public:
    enum ShowType {
        DATABASES,       // SHOW DATABASES
        TABLES,          // SHOW TABLES [FROM db]
        CREATE_TABLE,    // SHOW CREATE TABLE table
        COLUMNS,         // SHOW COLUMNS FROM table
        INDEXES,         // SHOW INDEXES FROM table
        GRANTS           // SHOW GRANTS FOR user
    };

    ShowStatement(ShowType type);
    ~ShowStatement();

    ShowType getShowType() const;
    
    // 设置目标对象（表名、用户名、数据库名）
    void setTargetObject(const std::string& target);
    const std::string& getTargetObject() const;
    
    // 设置FROM子句（数据库名）
    void setFromDatabase(const std::string& dbName);
    const std::string& getFromDatabase() const;
    bool hasFromDatabase() const;
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    ShowType type_;
    std::string targetObject_;  // 目标对象（表名、用户名）
    std::string fromDatabase_;  // FROM子句指定的数据库
    bool hasFromDb_;           // 是否有FROM子句
};

// ==================== ProcedureParameter ====================

class ProcedureParameter {
public:
    enum Mode {
        IN,
        OUT,
        INOUT
    };

    ProcedureParameter(const std::string& name, const std::string& type, Mode mode);
    ~ProcedureParameter();

    const std::string& getName() const;
    const std::string& getType() const;
    Mode getMode() const;
    std::string getModeString() const;

private:
    std::string name_;
    std::string type_;
    Mode mode_;
};

// ==================== CreateProcedureStatement ====================

class CreateProcedureStatement : public Statement {
public:
    CreateProcedureStatement(const std::string& name);
    ~CreateProcedureStatement();

    void addParameter(const ProcedureParameter& param);
    const std::vector<ProcedureParameter>& getParameters() const;

    void setBody(const std::string& body);
    const std::string& getBody() const;

    const std::string& getName() const;
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    std::string name_;
    std::vector<ProcedureParameter> parameters_;
    std::string body_;
};

// ==================== CallProcedureStatement ====================

class CallProcedureStatement : public Statement {
public:
    CallProcedureStatement(const std::string& name);
    ~CallProcedureStatement();

    void addArgument(std::unique_ptr<Expression> arg);
    const std::vector<std::unique_ptr<Expression>>& getArguments() const;

    const std::string& getName() const;
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    std::string name_;
    std::vector<std::unique_ptr<Expression>> arguments_;
};

// ==================== DropProcedureStatement ====================

class DropProcedureStatement : public Statement {
public:
    DropProcedureStatement(const std::string& name);
    ~DropProcedureStatement();

    const std::string& getName() const;
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    std::string name_;
};

// ==================== TriggerDefinition ====================

class TriggerDefinition {
public:
    enum Timing {
        BEFORE,
        AFTER,
        INSTEAD_OF
    };

    enum Event {
        INSERT,
        UPDATE,
        DELETE
    };

    enum Level {
        ROW,
        STATEMENT
    };

    TriggerDefinition(const std::string& name, Timing timing, Event event, 
                     Level level, const std::string& tableName);
    ~TriggerDefinition();

    const std::string& getName() const;
    Timing getTiming() const;
    std::string getTimingString() const;
    Event getEvent() const;
    std::string getEventString() const;
    Level getLevel() const;
    std::string getLevelString() const;
    const std::string& getTableName() const;

    void setCondition(const std::string& condition);
    const std::string& getCondition() const;
    bool hasCondition() const;

    void setBody(const std::string& body);
    const std::string& getBody() const;

private:
    std::string name_;
    Timing timing_;
    Event event_;
    Level level_;
    std::string tableName_;
    std::string condition_;
    std::string body_;
    bool hasCondition_;
};

// ==================== CreateTriggerStatement ====================

class CreateTriggerStatement : public Statement {
public:
    CreateTriggerStatement(const TriggerDefinition& triggerDef);
    ~CreateTriggerStatement();

    const TriggerDefinition& getTriggerDefinition() const;
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    TriggerDefinition triggerDef_;
};

// ==================== DropTriggerStatement ====================

class DropTriggerStatement : public Statement {
public:
    DropTriggerStatement(const std::string& name);
    ~DropTriggerStatement();

    const std::string& getName() const;
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    std::string name_;
};

// ==================== AlterTriggerStatement ====================

class AlterTriggerStatement : public Statement {
public:
    enum Action {
        ENABLE,
        DISABLE
    };

    AlterTriggerStatement(const std::string& name, Action action);
    ~AlterTriggerStatement();

    const std::string& getName() const;
    Action getAction() const;
    std::string getActionString() const;
    
    void accept(NodeVisitor &visitor) override {
        visitor.visit(*this);
    }

private:
    std::string name_;
    Action action_;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_AST_NODES_H