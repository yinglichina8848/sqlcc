#include "ast/ast_node.h"
#ifndef SQLCC_SQL_PARSER_ADVANCED_SQL92_FEATURES_H
#define SQLCC_SQL_PARSER_ADVANCED_SQL92_FEATURES_H

#include "ast_nodes.h"
#include "node_visitor.h"
#include <memory>
#include <string>
#include <vector>

namespace sqlcc {
namespace sql_parser {

// ==================== Advanced Transaction Control ====================

/**
 * SAVEPOINT语句
 */
class SavepointStatement : public Statement {
public:
  SavepointStatement(const std::string &savepointName);
  ~SavepointStatement();

  const std::string &getSavepointName() const;
  void accept(NodeVisitor &visitor);

private:
  std::string savepointName_;
};

/**
 * RELEASE SAVEPOINT语句
 */
class ReleaseSavepointStatement : public Statement {
public:
  ReleaseSavepointStatement(const std::string &savepointName);
  ~ReleaseSavepointStatement();

  const std::string &getSavepointName() const;
  void accept(NodeVisitor &visitor);

private:
  std::string savepointName_;
};

/**
 * SET TRANSACTION语句
 */
class SetTransactionStatement : public Statement {
public:
  enum IsolationLevel {
    READ_UNCOMMITTED,
    READ_COMMITTED,
    REPEATABLE_READ,
    SERIALIZABLE
  };

  enum AccessMode { READ_ONLY, READ_WRITE };

  SetTransactionStatement();
  ~SetTransactionStatement();

  void setIsolationLevel(IsolationLevel level);
  IsolationLevel getIsolationLevel() const;
  bool hasIsolationLevel() const;

  void setAccessMode(AccessMode mode);
  AccessMode getAccessMode() const;
  bool hasAccessMode() const;

  void setWork(bool work);
  bool isWork() const;

  void accept(NodeVisitor &visitor);

private:
  IsolationLevel isolationLevel_;
  AccessMode accessMode_;
  bool work_;
  bool hasIsolationLevel_;
  bool hasAccessMode_;
  bool hasWork_;
};

// ==================== User Defined Types (DOMAIN) ====================

/**
 * 域定义
 */
class DomainDefinition {
public:
  enum BaseType {
    CHARACTER,
    DECIMAL,
    NUMERIC,
    INTEGER,
    BIGINT,
    SMALLINT,
    FLOAT,
    REAL,
    DOUBLE_PRECISION,
    BOOLEAN,
    DATE,
    TIME,
    TIMESTAMP
  };

  DomainDefinition(const std::string &name, BaseType baseType);
  ~DomainDefinition();

  const std::string &getName() const;
  BaseType getBaseType() const;
  std::string getBaseTypeString() const;

  void setCharacterLength(int length);
  int getCharacterLength() const;
  bool hasCharacterLength() const;

  void setPrecision(int precision);
  int getPrecision() const;
  bool hasPrecision() const;

  void setScale(int scale);
  int getScale() const;
  bool hasScale() const;

  void setDefaultValue(const std::string &defaultValue);
  const std::string &getDefaultValue() const;
  bool hasDefaultValue() const;

  void setCheckConstraint(const std::string &checkConstraint);
  const std::string &getCheckConstraint() const;
  bool hasCheckConstraint() const;

  void setNotNull(bool notNull);
  bool isNotNull() const;
  bool hasNotNull() const;

private:
  std::string name_;
  BaseType baseType_;
  int characterLength_;
  int precision_;
  int scale_;
  std::string defaultValue_;
  std::string checkConstraint_;
  bool notNull_;
  bool hasCharacterLength_;
  bool hasPrecision_;
  bool hasScale_;
  bool hasDefaultValue_;
  bool hasCheckConstraint_;
  bool hasNotNull_;
};

/**
 * CREATE DOMAIN语句
 */
class CreateDomainStatement : public Statement {
public:
  CreateDomainStatement(std::unique_ptr<DomainDefinition> domainDef);
  ~CreateDomainStatement();

  const DomainDefinition &getDomainDefinition() const;
  std::unique_ptr<DomainDefinition> takeDomainDefinition();

  void accept(NodeVisitor &visitor);

private:
  std::unique_ptr<DomainDefinition> domainDef_;
};

/**
 * ALTER DOMAIN语句
 */
class AlterDomainStatement : public Statement {
public:
  enum Action {
    SET_DEFAULT,
    DROP_DEFAULT,
    ADD_CONSTRAINT,
    DROP_CONSTRAINT
  };

  AlterDomainStatement(const std::string &domainName, Action action);
  ~AlterDomainStatement();

  const std::string &getDomainName() const;
  Action getAction() const;
  std::string getActionString() const;

  void setDefaultValue(const std::string &defaultValue);
  const std::string &getDefaultValue() const;
  bool hasDefaultValue() const;

  void setConstraintName(const std::string &constraintName);
  const std::string &getConstraintName() const;
  bool hasConstraintName() const;

  void setConstraintDefinition(const std::string &constraintDef);
  const std::string &getConstraintDefinition() const;
  bool hasConstraintDefinition() const;

  void accept(NodeVisitor &visitor);

private:
  std::string domainName_;
  Action action_;
  std::string defaultValue_;
  std::string constraintName_;
  std::string constraintDefinition_;
  bool hasDefaultValue_;
  bool hasConstraintName_;
  bool hasConstraintDefinition_;
};

/**
 * DROP DOMAIN语句
 */
class DropDomainStatement : public Statement {
public:
  enum DropBehavior { RESTRICT, CASCADE };

  DropDomainStatement(const std::string &domainName);
  ~DropDomainStatement();

  const std::string &getDomainName() const;
  DropBehavior getDropBehavior() const;
  void setDropBehavior(DropBehavior behavior);

  bool isIfExists() const;
  void setIfExists(bool ifExists);

  void accept(NodeVisitor &visitor);

private:
  std::string domainName_;
  DropBehavior dropBehavior_;
  bool ifExists_;
};

// ==================== Enhanced Functions ====================

/**
 * 函数定义
 */
class FunctionDefinition {
public:
  enum ReturnType { SCALAR, TABLE, RECORD };

  FunctionDefinition(const std::string &name);
  ~FunctionDefinition();

  const std::string &getName() const;
  ReturnType getReturnType() const;
  std::string getReturnTypeString() const;

  void addParameter(const ProcedureParameter &param);
  const std::vector<ProcedureParameter> &getParameters() const;

  void setReturnDataType(const std::string &dataType);
  const std::string &getReturnDataType() const;
  bool hasReturnDataType() const;

  void setLanguage(const std::string &language);
  const std::string &getLanguage() const;
  bool hasLanguage() const;

  void setBody(const std::string &body);
  const std::string &getBody() const;

  void setDeterministic(bool deterministic);
  bool isDeterministic() const;
  bool hasDeterministic() const;

private:
  std::string name_;
  ReturnType returnType_;
  std::vector<ProcedureParameter> parameters_;
  std::string returnDataType_;
  std::string language_;
  std::string body_;
  bool deterministic_;
  bool hasReturnDataType_;
  bool hasLanguage_;
  bool hasDeterministic_;
};

/**
 * CREATE FUNCTION语句
 */
class CreateFunctionStatement : public Statement {
public:
  CreateFunctionStatement(std::unique_ptr<FunctionDefinition> functionDef);
  ~CreateFunctionStatement();

  const FunctionDefinition &getFunctionDefinition() const;
  std::unique_ptr<FunctionDefinition> takeFunctionDefinition();

  void accept(NodeVisitor &visitor);

private:
  std::unique_ptr<FunctionDefinition> functionDef_;
};

/**
 * DROP FUNCTION语句
 */
class DropFunctionStatement : public Statement {
public:
  DropFunctionStatement(const std::string &functionName);
  ~DropFunctionStatement();

  const std::string &getFunctionName() const;
  bool isIfExists() const;
  void setIfExists(bool ifExists);

  void accept(NodeVisitor &visitor);

private:
  std::string functionName_;
  bool ifExists_;
};

// ==================== Enhanced Triggers ====================

/**
 * 增强的触发器定义
 */
class EnhancedTriggerDefinition : public TriggerDefinition {
public:
  EnhancedTriggerDefinition(const std::string &name, Timing timing, Event event,
                           Level level, const std::string &tableName);
  ~EnhancedTriggerDefinition();

  // OLD和NEW引用支持
  void setOldTableName(const std::string &oldTableName);
  const std::string &getOldTableName() const;
  bool hasOldTableName() const;

  void setNewTableName(const std::string &newTableName);
  const std::string &getNewTableName() const;
  bool hasNewTableName() const;

  // 触发器变量支持
  void addVariable(const std::string &name, const std::string &type);
  const std::vector<std::pair<std::string, std::string>> &getVariables() const;

  //WHEN条件支持
  void setWhenCondition(const std::string &whenCondition);
  const std::string &getWhenCondition() const;
  bool hasWhenCondition() const;

private:
  std::string oldTableName_;
  std::string newTableName_;
  std::vector<std::pair<std::string, std::string>> variables_;
  std::string whenCondition_;
  bool hasOldTableName_;
  bool hasNewTableName_;
  bool hasWhenCondition_;
};

// ==================== Advanced DDL Operations ====================

/**
 * ALTER TABLE的增强操作
 */
class AlterTableAction {
public:
  enum ActionType {
    ADD_COLUMN,
    DROP_COLUMN,
    ALTER_COLUMN,
    RENAME_COLUMN,
    ADD_CONSTRAINT,
    DROP_CONSTRAINT,
    DISABLE_TRIGGER,
    ENABLE_TRIGGER
  };

  AlterTableAction(ActionType type);
  ~AlterTableAction();

  ActionType getActionType() const;
  std::string getActionTypeString() const;

  // ADD COLUMN支持
  void setColumnDefinition(ColumnDefinition &&columnDef);
  const ColumnDefinition &getColumnDefinition() const;
  bool hasColumnDefinition() const;

  // DROP COLUMN支持
  void setColumnName(const std::string &columnName);
  const std::string &getColumnName() const;
  bool hasColumnName() const;

  // ALTER COLUMN支持
  void setNewColumnDefinition(ColumnDefinition &&columnDef);
  const ColumnDefinition &getNewColumnDefinition() const;
  bool hasNewColumnDefinition() const;

  // RENAME COLUMN支持
  void setOldColumnName(const std::string &oldColumnName);
  const std::string &getOldColumnName() const;
  bool hasOldColumnName() const;

  // CONSTRAINT操作支持
  void setConstraint(TableConstraint &&constraint);
  const TableConstraint &getConstraint() const;
  bool hasConstraint() const;

  void setConstraintName(const std::string &constraintName);
  const std::string &getConstraintName() const;
  bool hasConstraintName() const;

  // TRIGGER操作支持
  void setTriggerName(const std::string &triggerName);
  const std::string &getTriggerName() const;
  bool hasTriggerName() const;

private:
  ActionType actionType_;
  ColumnDefinition columnDef_;
  ColumnDefinition newColumnDef_;
  std::string columnName_;
  std::string oldColumnName_;
  TableConstraint constraint_;
  std::string constraintName_;
  std::string triggerName_;
  bool hasColumnDefinition_;
  bool hasColumnName_;
  bool hasNewColumnDefinition_;
  bool hasOldColumnName_;
  bool hasConstraint_;
  bool hasConstraintName_;
  bool hasTriggerName_;
};

/**
 * 增强的ALTER TABLE语句
 */
class EnhancedAlterTableStatement : public Statement {
public:
  EnhancedAlterTableStatement(const std::string &tableName);
  ~EnhancedAlterTableStatement();

  const std::string &getTableName() const;

  void addAction(std::unique_ptr<AlterTableAction> action);
  const std::vector<std::unique_ptr<AlterTableAction>> &getActions() const;

  void accept(NodeVisitor &visitor);

private:
  std::string tableName_;
  std::vector<std::unique_ptr<AlterTableAction>> actions_;
};

} // namespace sql_parser
} // namespace sqlcc

#endif // SQLCC_SQL_PARSER_ADVANCED_SQL92_FEATURES_H
