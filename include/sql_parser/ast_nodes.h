#ifndef SQLCC_SQL_PARSER_AST_NODES_H
#define SQLCC_SQL_PARSER_AST_NODES_H

// First include the base AST node definitions
#include "sql_parser/ast_node.h"
#include "sql_parser/data_types.h"
#include "set_operation.h"
#include "sql_parser/node_visitor.h"
#include "../storage/table_storage.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Forward declaration for TableMetadata
class TableMetadata;

// All other forward declarations are now handled by the included headers

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
  ColumnDefinition(const std::string &name, const std::string &type);
  ~ColumnDefinition(); // 显式声明析构函数

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

  // DECIMAL相关
  int getPrecision() const { return precision_; }
  int getScale() const { return scale_; }

  // Setters
  void setName(const std::string &name) { name_ = name; }
  void setType(const std::string &type);
  void setPrimaryKey(bool primaryKey = true) { isPrimaryKey_ = primaryKey; }
  void setNullable(bool nullable = true) { isNullable_ = nullable; }
  void setUnique(bool unique = true) { isUnique_ = unique; }
  void setForeignKey(bool foreignKey = true) { isForeignKey_ = foreignKey; }
  void setAutoIncrement(bool autoIncrement = true) {
    isAutoIncrement_ = autoIncrement;
  }
  void setDefaultValue(const std::string &defaultValue);

  // DECIMAL设置
  void setPrecision(int precision) { precision_ = precision; }
  void setScale(int scale) { scale_ = scale; }

  // 兼容旧方法名
  void setIsPrimaryKey(bool isPrimaryKey) { setPrimaryKey(isPrimaryKey); }
  void setIsNullable(bool isNullable) { setNullable(isNullable); }
  void setIsUnique(bool isUnique) { setUnique(isUnique); }
  void setIsAutoIncrement(bool isAutoIncrement) {
    setAutoIncrement(isAutoIncrement);
  }

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

  // DECIMAL相关属性
  int precision_;
  int scale_;
};

// ==================== ConstraintValidator ====================

/**
 * @brief 约束验证器接口
 */
class ConstraintValidator {
public:
  virtual ~ConstraintValidator() = default;

  /**
   * 验证记录是否满足约束条件
   * @param record 要验证的记录
   * @param metadata 表元数据
   * @param table_name 表名
   * @return 验证结果
   */
  virtual bool validate(const std::vector<std::string>& record,
                       std::shared_ptr<TableMetadata> metadata,
                       const std::string& table_name) const = 0;

  /**
   * 获取约束名称
   */
  virtual std::string getConstraintName() const = 0;

  /**
   * 获取约束类型
   */
  virtual std::string getConstraintType() const = 0;
};

/**
 * @brief 主键约束验证器
 */
class PrimaryKeyValidator : public ConstraintValidator {
public:
  PrimaryKeyValidator(const std::vector<std::string>& columns, const std::string& constraint_name = "");
  ~PrimaryKeyValidator() override;

  bool validate(const std::vector<std::string>& record,
               std::shared_ptr<TableMetadata> metadata,
               const std::string& table_name) const override;

  std::string getConstraintName() const override;
  std::string getConstraintType() const override { return "PRIMARY KEY"; }

private:
  std::vector<std::string> columns_;
  std::string constraint_name_;
};

/**
 * @brief 唯一约束验证器
 */
class UniqueKeyValidator : public ConstraintValidator {
public:
  UniqueKeyValidator(const std::vector<std::string>& columns, const std::string& constraint_name = "");
  ~UniqueKeyValidator() override;

  bool validate(const std::vector<std::string>& record,
               std::shared_ptr<TableMetadata> metadata,
               const std::string& table_name) const override;

  std::string getConstraintName() const override;
  std::string getConstraintType() const override { return "UNIQUE"; }

private:
  std::vector<std::string> columns_;
  std::string constraint_name_;

  bool checkUniqueness(const std::vector<std::string>& key_values) const;
};

/**
 * @brief 外键约束验证器
 */
class ForeignKeyValidator : public ConstraintValidator {
public:
  ForeignKeyValidator(const std::vector<std::string>& columns,
                     const std::string& referenced_table,
                     const std::vector<std::string>& referenced_columns,
                     const std::string& constraint_name = "");
  ~ForeignKeyValidator() override;

  bool validate(const std::vector<std::string>& record,
               std::shared_ptr<TableMetadata> metadata,
               const std::string& table_name) const override;

  std::string getConstraintName() const override;
  std::string getConstraintType() const override { return "FOREIGN KEY"; }

  // 级联操作支持
  enum CascadeAction { RESTRICT, CASCADE, SET_NULL };
  void setOnDeleteAction(CascadeAction action);
  void setOnUpdateAction(CascadeAction action);
  CascadeAction getOnDeleteAction() const;
  CascadeAction getOnUpdateAction() const;

private:
  std::vector<std::string> columns_;
  std::string referenced_table_;
  std::vector<std::string> referenced_columns_;
  std::string constraint_name_;
  CascadeAction on_delete_action_;
  CascadeAction on_update_action_;

  bool checkReferenceExists(const std::vector<std::string>& values,
                           const std::string& ref_table) const;
};

/**
 * @brief 检查约束验证器
 */
class CheckConstraintValidator : public ConstraintValidator {
public:
  CheckConstraintValidator(const std::string& expression, const std::string& constraint_name = "");
  ~CheckConstraintValidator() override;

  bool validate(const std::vector<std::string>& record,
               std::shared_ptr<TableMetadata> metadata,
               const std::string& table_name) const override;

  std::string getConstraintName() const override;
  std::string getConstraintType() const override { return "CHECK"; }

private:
  std::string expression_;
  std::string constraint_name_;

  bool evaluateExpression(const std::string& expression,
                         const std::vector<std::string>& record,
                         std::shared_ptr<TableMetadata> metadata) const;
};

/**
 * @brief 约束管理器
 */
class ConstraintManager {
public:
  static ConstraintManager& getInstance();

  // 添加约束验证器
  void addValidator(const std::string& table_name,
                   std::unique_ptr<ConstraintValidator> validator);

  // 移除约束验证器
  void removeValidator(const std::string& table_name,
                      const std::string& constraint_name);

  // 验证记录
  bool validateRecord(const std::vector<std::string>& record,
                     std::shared_ptr<TableMetadata> metadata,
                     const std::string& table_name) const;

  // 获取表的所有约束
  std::vector<const ConstraintValidator*> getValidators(const std::string& table_name) const;

  // 清空表的所有约束
  void clearValidators(const std::string& table_name);

private:
  ConstraintManager();
  std::unordered_map<std::string, std::vector<std::unique_ptr<ConstraintValidator>>> validators_;
};

// ==================== TableConstraint ====================

class TableConstraint {
public:
  enum Type { PRIMARY_KEY, FOREIGN_KEY, UNIQUE, CHECK };

  TableConstraint(Type type, const std::string &name = "");
  ~TableConstraint();

  Type getType() const;
  const std::string &getConstraintName() const;
  const std::vector<std::string> &getColumns() const;
  const std::string &getReferencedTable() const;
  const std::vector<std::string> &getReferencedColumns() const;
  const std::string &getCheckExpression() const;

  void addColumn(const std::string &column);
  void setReferencedTable(const std::string &table);
  void addReferencedColumn(const std::string &column);
  void setCheckExpression(const std::string &expression);

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
  WhereClause(const std::string &columnName, const std::string &op,
              const std::string &value);
  ~WhereClause();

  const std::string &getColumnName() const;
  const std::string &getOp() const;
  const std::string &getValue() const;

private:
  std::string columnName_;
  std::string op_;
  std::string value_;
};

// ==================== CreateStatement ====================

class CreateStatement : public Statement {
public:
  enum ObjectType { DATABASE, TABLE, INDEX, VIEW, PROCEDURE, TRIGGER };

  CreateStatement(ObjectType objectType, const std::string &objectName);
  CreateStatement(ObjectType objectType); // 兼容旧用法：后续通过setter设置名称
  ~CreateStatement();

  ObjectType getObjectType() const;
  const std::string &getObjectName() const;
  const std::vector<ColumnDefinition> &getColumns() const;
  const std::vector<TableConstraint> &getConstraints() const;

  void addColumn(ColumnDefinition &&column);
  void addConstraint(TableConstraint &&constraint);

  // 兼容旧测试API的setter
  void setObjectName(const std::string &name) { objectName_ = name; }
  void setDatabaseName(const std::string &name) { objectName_ = name; }
  void setTableName(const std::string &name) { objectName_ = name; }

  // 兼容旧API的非const getter
  // (removed non-const compatibility getters to keep a single canonical API)
  // 兼容旧API的非const getter（保留以兼容已编译的对象文件）
  std::string getObjectName();
  CreateStatement::ObjectType getObjectType();

  void accept(NodeVisitor &visitor);

private:
  ObjectType objectType_;
  std::string objectName_;
  std::vector<ColumnDefinition> columns_;
  std::vector<TableConstraint> constraints_;
};

// ==================== CreateViewStatement ====================

class CreateViewStatement : public Statement {
public:
  CreateViewStatement(const std::string &viewName);
  ~CreateViewStatement();

  const std::string &getViewName() const;
  const std::vector<std::string> &getColumnNames() const;
  const SelectStatement &getSelectStatement() const;

  void addColumnName(const std::string &columnName);
  void setSelectStatement(std::unique_ptr<SelectStatement> selectStmt);

  bool hasColumnNames() const;

  void accept(NodeVisitor &visitor);

private:
  std::string viewName_;
  std::vector<std::string> columnNames_;
  std::unique_ptr<SelectStatement> selectStatement_;
};

// ==================== AlterViewStatement ====================

class AlterViewStatement : public Statement {
public:
  AlterViewStatement(const std::string &viewName);
  ~AlterViewStatement();

  const std::string &getViewName() const;
  const std::vector<std::string> &getColumnNames() const;
  const SelectStatement &getSelectStatement() const;

  void addColumnName(const std::string &columnName);
  void setSelectStatement(std::unique_ptr<SelectStatement> selectStmt);

  bool hasColumnNames() const;

  void accept(NodeVisitor &visitor);

private:
  std::string viewName_;
  std::vector<std::string> columnNames_;
  std::unique_ptr<SelectStatement> selectStatement_;
};

// ==================== DropViewStatement ====================

class DropViewStatement : public Statement {
public:
  enum DropBehavior { RESTRICT, CASCADE };

  DropViewStatement(const std::string &viewName);
  ~DropViewStatement();

  const std::string &getViewName() const;
  DropBehavior getDropBehavior() const;
  void setDropBehavior(DropBehavior behavior);

  bool isIfExists() const;
  void setIfExists(bool ifExists);

  void accept(NodeVisitor &visitor);

private:
  std::string viewName_;
  DropBehavior dropBehavior_;
  bool ifExists_;
};

// ==================== SelectStatement ====================

// ==================== JoinClause ====================

class JoinClause {
public:
  enum JoinType {
    INNER_JOIN,
    LEFT_JOIN,
    RIGHT_JOIN,
    FULL_JOIN,
    CROSS_JOIN
  };

  JoinClause(JoinType type, const std::string &tableName,
             std::unique_ptr<Expression> condition = nullptr);
  ~JoinClause();

  JoinType getJoinType() const;
  void setJoinType(JoinType type);
  const std::string &getTableName() const;
  const Expression *getCondition() const;
  std::unique_ptr<Expression> takeCondition(); // Transfer ownership

  void setCondition(std::unique_ptr<Expression> condition);

private:
  JoinType joinType_;
  std::string tableName_;
  std::unique_ptr<Expression> condition_;
};

class SelectStatement : public Statement {
public:
  SelectStatement();
  ~SelectStatement();

  void addSelectColumn(const std::string &column);
  void addSelectItem(const std::string &column);
  void setTableName(const std::string &table);
  void addFromTable(const std::string &table);
  void addJoinClause(std::unique_ptr<JoinClause> join);
  void setWhereClause(const WhereClause &where);
  void setWhereExpression(std::unique_ptr<Expression> expr);
  void setGroupByColumn(const std::string &column);
  void addGroupByColumn(const std::string &column); // New method for multiple GROUP BY columns
  void setHavingClause(std::unique_ptr<Expression> expr); // New method for HAVING clause
  void setDistinct(bool distinct = true);
  bool isDistinct() const;
  void setOrderByColumn(const std::string &column);
  void setOrderDirection(const std::string &direction);
  void setSelectAll(bool selectAll);
  void setJoinCondition(const std::string &condition);
  void setLimit(int limit);
  void setOffset(int offset);

  const std::vector<std::string> &getSelectColumns() const;
  const std::vector<std::string> &getSelectItems() const {
    return selectColumns_;
  }
  const std::vector<std::string> &getFromTables() const { return fromTables_; }
  const std::vector<std::unique_ptr<JoinClause>> &getJoinClauses() const { return joinClauses_; }
  const std::string &getTableName() const;
  const WhereClause &getWhereClause() const;
  const Expression *getWhereExpression() const;
  const std::string &getGroupByColumn() const;
  const std::vector<std::string> &getGroupByColumns() const; // New method for multiple GROUP BY columns
  const Expression *getHavingClause() const; // New method for HAVING clause
  const std::string &getOrderByColumn() const;
  const std::string &getOrderDirection() const;
  const std::string &getJoinCondition() const;
  int getLimit() const;
  int getOffset() const;
  bool isSelectAll() const;
  bool hasWhereClause() const;
  bool hasWhereExpression() const;
  bool hasGroupBy() const;
  bool hasHavingClause() const; // New method
  bool hasOrderBy() const;
  bool hasJoinCondition() const;
  bool hasJoins() const { return !joinClauses_.empty(); }
  bool hasLimit() const;
  bool hasOffset() const;

  void accept(NodeVisitor &visitor);

private:
  std::vector<std::string> selectColumns_;
  std::vector<std::string> fromTables_;
  std::vector<std::unique_ptr<JoinClause>> joinClauses_;
  std::string tableName_;
  WhereClause whereClause_{"", "", ""}; // 初始化空的WhereClause
  std::unique_ptr<Expression> whereExpression_; // 完整的WHERE表达式
  std::string groupByColumn_;
  std::vector<std::string> groupByColumns_; // Support for multiple GROUP BY columns
  std::unique_ptr<Expression> havingClause_; // HAVING clause expression
  std::string orderByColumn_;
  std::string orderDirection_;
  std::string joinCondition_;
  int limit_;
  int offset_;
  bool selectAll_;
  bool distinct_; // 是否使用DISTINCT
  bool hasLimit_;
  bool hasOffset_;
};

// ==================== CompositeSelectStatement (复合SELECT，包含集合操作) ====================

class CompositeSelectStatement : public Statement {
public:
  CompositeSelectStatement() : Statement(Statement::COMPOSITE_SELECT) {}
  ~CompositeSelectStatement() override = default;

  void addSelectStatement(std::unique_ptr<SelectStatement> stmt) {
    selectStatements_.push_back(std::move(stmt));
  }

  void addSetOperation(std::unique_ptr<SetOperation> op) {
    operations_.push_back(std::move(op));
  }

  const std::vector<std::unique_ptr<SelectStatement>> &getSelectStatements() const {
    return selectStatements_;
  }

  const std::vector<std::unique_ptr<SetOperation>> &getSetOperations() const {
    return operations_;
  }

  size_t getStatementCount() const { return selectStatements_.size(); }
  size_t getOperationCount() const { return operations_.size(); }
  bool hasSetOperations() const { return !operations_.empty(); }

  void accept(NodeVisitor &visitor);

private:
  std::vector<std::unique_ptr<SelectStatement>> selectStatements_;
  std::vector<std::unique_ptr<SetOperation>> operations_;
};

// ==================== InsertStatement ====================

class InsertStatement : public Statement {
public:
  InsertStatement();
  InsertStatement(const std::string &tableName);
  ~InsertStatement();

  void setTableName(const std::string &tableName) { tableName_ = tableName; }
  const std::string &getTableName() const;
  const std::vector<std::string> &getColumns() const;
  const std::vector<std::vector<std::string>> &getValues() const;

  void addColumn(const std::string &column);
  void addValue(const std::string &value);
  void finishRow();
  void addValueRow(const std::vector<std::unique_ptr<Expression>> &values);

  void accept(NodeVisitor &visitor);

private:
  std::string tableName_;
  std::vector<std::string> columns_;
  std::vector<std::string> currentRow_;
  std::vector<std::vector<std::string>> values_;
};

// ==================== UpdateStatement ====================

class UpdateStatement : public Statement {
public:
  UpdateStatement();
  UpdateStatement(const std::string &tableName);
  ~UpdateStatement();

  void addUpdateValue(const std::string &column, const std::string &value);
  void setWhereClause(const WhereClause &where);
  void setTableName(const std::string &tableName) { tableName_ = tableName; }

  const std::string &getTableName() const;
  std::string getTableName();
  const std::unordered_map<std::string, std::string> &getUpdateValues() const;
  const WhereClause &getWhereClause() const;
  bool hasWhereClause() const;

  void accept(NodeVisitor &visitor);

private:
  std::string tableName_;
  std::unordered_map<std::string, std::string> updateValues_;
  WhereClause whereClause_{"", "", ""}; // 初始化空的WhereClause
};

// ==================== DeleteStatement ====================

class DeleteStatement : public Statement {
public:
  DeleteStatement();
  DeleteStatement(const std::string &tableName);
  ~DeleteStatement();

  void setWhereClause(const WhereClause &where);
  void setTableName(const std::string &tableName) { tableName_ = tableName; }

  const std::string &getTableName() const;
  std::string getTableName();
  const WhereClause &getWhereClause() const;
  bool hasWhereClause() const;

  void accept(NodeVisitor &visitor);

private:
  std::string tableName_;
  WhereClause whereClause_{"", "", ""}; // 初始化空的WhereClause
};

// ==================== DropStatement ====================

class DropStatement : public Statement {
public:
  enum ObjectType { DATABASE, TABLE, INDEX };

  DropStatement(ObjectType objectType, const std::string &objectName);
  DropStatement(ObjectType objectType); // 兼容旧用法：后续通过setter设置名称
  ~DropStatement();

  ObjectType getObjectType() const;
  const std::string &getObjectName() const;
  bool isIfExists() const;
  void setIfExists(bool ifExists);
  // 兼容旧测试API的setter
  void setObjectName(const std::string &name) { objectName_ = name; }
  void setDatabaseName(const std::string &name) { objectName_ = name; }
  void setTableName(const std::string &name) { objectName_ = name; }

  // 兼容旧API的非const getter
  std::string getObjectName();
  DropStatement::ObjectType getObjectType();

  void accept(NodeVisitor &visitor);

private:
  ObjectType objectType_;
  std::string objectName_;
  bool ifExists_;
};

// ==================== AlterStatement ====================

class AlterStatement : public Statement {
public:
  enum Target { DATABASE, TABLE };
  
  enum Action {
    ADD_COLUMN,
    DROP_COLUMN,
    MODIFY_COLUMN,
    RENAME_TABLE,
    ADD_INDEX,
    DROP_INDEX
  };

  AlterStatement(Target target);
  ~AlterStatement();

  Type getType() const { return ALTER; }
  void accept(NodeVisitor &visitor);

  // Getters
  Target getTarget() const;
  Action getAction() const;
  const std::string& getDatabaseName() const;
  const std::string& getTableName() const;
  const std::string& getColumnName() const;
  const ColumnDefinition& getColumnDefinition() const;
  const std::string& getIndexName() const;
  const std::string& getNewTableName() const;

  // Setters
  void setDatabaseName(const std::string& name);
  void setTableName(const std::string& name);
  void setAction(Action action);
  void setColumnName(const std::string& name);
  void setColumnDefinition(ColumnDefinition&& columnDef);
  void setIndexName(const std::string& name);
  void setNewTableName(const std::string& newName);

private:
  Target target_;
  Action action_;
  std::string databaseName_;
  std::string tableName_;
  std::string columnName_;
  ColumnDefinition columnDef_;
  std::string indexName_;
  std::string newTableName_;
};

// ==================== UseStatement ====================

class UseStatement : public Statement {
public:
  UseStatement(const std::string &databaseName);
  ~UseStatement();

  const std::string &getDatabaseName() const;
  std::string getDatabaseName();

  void accept(NodeVisitor &visitor);

private:
  std::string databaseName_;
};

// ==================== CreateIndexStatement ====================

class CreateIndexStatement : public Statement {
public:
  CreateIndexStatement(const std::string &indexName,
                       const std::string &tableName,
                       const std::string &columnName);
  ~CreateIndexStatement();

  const std::string &getIndexName() const;
  const std::string &getTableName() const;
  std::string getTableName();
  const std::string &getColumnName() const;
  void addColumn(const std::string &column);
  const std::vector<std::string> &getColumns() const;

  void setUnique(bool unique); // 设置UNIQUE标记
  bool isUnique() const;       // 获取UNIQUE标记

  void accept(NodeVisitor &visitor);

private:
  std::string indexName_;
  std::string tableName_;
  std::vector<std::string> columns_;
  bool unique_; // 是否为UNIQUE索引
};

// ==================== DropIndexStatement ====================

class DropIndexStatement : public Statement {
public:
  DropIndexStatement(const std::string &indexName);
  ~DropIndexStatement();

  const std::string &getIndexName() const;

  void setTableName(const std::string &tableName); // 设置表名
  const std::string &getTableName() const;         // 获取表名
  std::string getTableName();
  bool hasTableName() const;                       // 是否指定了表名

  void setIfExists(bool ifExists); // 设置IF EXISTS标记
  bool isIfExists() const;         // 获取IF EXISTS标记

  void accept(NodeVisitor &visitor);

private:
  std::string indexName_;
  std::string tableName_; // 表名（可选）
  bool ifExists_;         // IF EXISTS标记
  bool hasTableName_;     // 是否指定表名
};

// ==================== CreateUserStatement ====================

class CreateUserStatement : public Statement {
public:
  CreateUserStatement(const std::string &username, const std::string &password);
  ~CreateUserStatement();

  const std::string &getUsername() const;
  std::string getUsername();
  const std::string &getPassword() const;
  bool isWithPassword() const;
  void setWithPassword(bool withPassword);

  void accept(NodeVisitor &visitor);

private:
  std::string username_;
  std::string password_;
  bool withPassword_;
};

// ==================== DropUserStatement ====================

class DropUserStatement : public Statement {
public:
  DropUserStatement(const std::string &username);
  ~DropUserStatement();

  const std::string &getUsername() const;
  std::string getUsername();
  bool isIfExists() const;
  void setIfExists(bool ifExists);

  void accept(NodeVisitor &visitor);

private:
  std::string username_;
  bool ifExists_;
};

// ==================== GrantStatement ====================

class GrantStatement : public Statement {
public:
  GrantStatement();
  ~GrantStatement();

  void addPrivilege(const std::string &privilege);
  const std::vector<std::string> &getPrivileges() const;

  void setObjectType(const std::string &objectType);
  const std::string &getObjectType() const;

  void setObjectName(const std::string &objectName);
  const std::string &getObjectName() const;

  void setGrantee(const std::string &grantee);
  const std::string &getGrantee() const;
  std::string getGrantee();

  void accept(NodeVisitor &visitor);

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

  void addPrivilege(const std::string &privilege);
  const std::vector<std::string> &getPrivileges() const;

  void setObjectType(const std::string &objectType);
  const std::string &getObjectType() const;

  void setObjectName(const std::string &objectName);
  const std::string &getObjectName() const;

  void setGrantee(const std::string &grantee);
  const std::string &getGrantee() const;
  std::string getGrantee();

  void accept(NodeVisitor &visitor);

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
    DATABASES,    // SHOW DATABASES
    TABLES,       // SHOW TABLES [FROM db]
    CREATE_TABLE, // SHOW CREATE TABLE table
    COLUMNS,      // SHOW COLUMNS FROM table
    INDEXES,      // SHOW INDEXES FROM table
    GRANTS        // SHOW GRANTS FOR user
  };

  ShowStatement(ShowType type);
  ~ShowStatement();

  ShowType getShowType() const;

  // 设置目标对象（表名、用户名、数据库名）
  void setTargetObject(const std::string &target);
  const std::string &getTargetObject() const;

  // 设置FROM子句（数据库名）
  void setFromDatabase(const std::string &dbName);
  const std::string &getFromDatabase() const;
  bool hasFromDatabase() const;

  void accept(NodeVisitor &visitor);

private:
  ShowType type_;
  std::string targetObject_; // 目标对象（表名、用户名）
  std::string fromDatabase_; // FROM子句指定的数据库
  bool hasFromDb_;           // 是否有FROM子句
};

// ==================== Expression Classes ====================

class IdentifierExpression : public Expression {
public:
  IdentifierExpression(const std::string &name);
  ~IdentifierExpression();

  const std::string &getName() const;
  virtual std::string getTypeName() const override;
  virtual void accept(NodeVisitor &visitor);
  virtual Type getType() const override { return IDENTIFIER; }

private:
  std::string name_;
};

class StringLiteralExpression : public Expression {
public:
  StringLiteralExpression(const std::string &value);
  ~StringLiteralExpression();

  const std::string &getValue() const;
  virtual std::string getTypeName() const override;
  virtual void accept(NodeVisitor &visitor);
  virtual Type getType() const override { return STRING_LITERAL; }

private:
  std::string value_;
};

class NumericLiteralExpression : public Expression {
public:
  NumericLiteralExpression(double value);
  ~NumericLiteralExpression();

  double getValue() const;
  virtual std::string getTypeName() const override;
  virtual void accept(NodeVisitor &visitor);
  virtual Type getType() const override { return NUMERIC_LITERAL; }

private:
  double value_;
};

class BooleanLiteralExpression : public Expression {
public:
  BooleanLiteralExpression(bool value);
  ~BooleanLiteralExpression();

  bool getValue() const;
  virtual std::string getTypeName() const override;
  virtual void accept(NodeVisitor &visitor);
  virtual Type getType() const override { return BOOLEAN_LITERAL; }

private:
  bool value_;
};

class NullLiteralExpression : public Expression {
public:
  NullLiteralExpression();
  ~NullLiteralExpression();

  virtual std::string getTypeName() const override;
  virtual void accept(NodeVisitor &visitor);
  virtual Type getType() const override { return NULL_LITERAL; }
};

// ==================== CommitStatement ====================

class CommitStatement : public Statement {
public:
  CommitStatement();
  ~CommitStatement();

  void accept(NodeVisitor &visitor);

private:
};

// ==================== RollbackStatement ====================

class RollbackStatement : public Statement {
public:
  RollbackStatement();
  ~RollbackStatement();

  void accept(NodeVisitor &visitor);

private:
};

// ==================== ProcedureParameter ====================

class ProcedureParameter {
public:
  enum Mode { IN, OUT, INOUT };

  ProcedureParameter(const std::string &name, const std::string &type,
                     Mode mode);
  ~ProcedureParameter();

  const std::string &getName() const;
  const std::string &getType() const;
  Mode getMode() const;
  std::string getModeString() const;

private:
  std::string name_;
  std::string type_;
  Mode mode_;
};

// ==================== CreateProcedureStatement ====================

class CreateProcedureStatement : public CreateStatement {
public:
  CreateProcedureStatement(const std::string &name);
  ~CreateProcedureStatement();

  void addParameter(const ProcedureParameter &param);
  const std::vector<ProcedureParameter> &getParameters() const;

  void setBody(const std::string &body);
  const std::string &getBody() const;

  const std::string &getName() const;

  void accept(NodeVisitor &visitor);

private:
  std::string name_;
  std::vector<ProcedureParameter> parameters_;
  std::string body_;
};

// ==================== CallProcedureStatement ====================

class CallProcedureStatement : public Statement {
public:
  CallProcedureStatement(const std::string &name);
  ~CallProcedureStatement();

  void addArgument(std::unique_ptr<Expression> arg);
  const std::vector<std::unique_ptr<Expression>> &getArguments() const;

  const std::string &getName() const;

  void accept(NodeVisitor &visitor);

private:
  std::string name_;
  std::vector<std::unique_ptr<Expression>> arguments_;
};

// ==================== DropProcedureStatement ====================

class DropProcedureStatement : public Statement {
public:
  DropProcedureStatement(const std::string &name);
  ~DropProcedureStatement();

  const std::string &getName() const;

  void accept(NodeVisitor &visitor);

private:
  std::string name_;
};

// ==================== TriggerDefinition ====================

class TriggerDefinition {
public:
  enum Timing { BEFORE, AFTER, INSTEAD_OF };

  enum Event { INSERT, UPDATE, DELETE };

  enum Level { ROW, STATEMENT };

  TriggerDefinition(const std::string &name, Timing timing, Event event,
                    Level level, const std::string &tableName);
  ~TriggerDefinition();

  const std::string &getName() const;
  Timing getTiming() const;
  std::string getTimingString() const;
  Event getEvent() const;
  std::string getEventString() const;
  Level getLevel() const;
  std::string getLevelString() const;
  const std::string &getTableName() const;
  std::string getTableName();

  void setCondition(const std::string &condition);
  const std::string &getCondition() const;
  bool hasCondition() const;

  void setBody(const std::string &body);
  const std::string &getBody() const;

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

class CreateTriggerStatement : public CreateStatement {
public:
  CreateTriggerStatement(const TriggerDefinition &triggerDef);
  ~CreateTriggerStatement();

  const TriggerDefinition &getTriggerDefinition() const;

  void accept(NodeVisitor &visitor);

private:
  TriggerDefinition triggerDef_;
};

// ==================== DropTriggerStatement ====================

class DropTriggerStatement : public Statement {
public:
  DropTriggerStatement(const std::string &name);
  ~DropTriggerStatement();

  const std::string &getName() const;

  void accept(NodeVisitor &visitor);

private:
  std::string name_;
};

// ==================== AlterTriggerStatement ====================

class AlterTriggerStatement : public Statement {
public:
  enum Action { ENABLE, DISABLE };

  AlterTriggerStatement(const std::string &name, Action action);
  ~AlterTriggerStatement();

  const std::string &getName() const;
  Action getAction() const;
  std::string getActionString() const;

  void accept(NodeVisitor &visitor);

private:
  std::string name_;
  Action action_;
};

// ==================== Function AST Nodes ====================
// Function-related classes are defined in function_ast.h

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_AST_NODES_H
