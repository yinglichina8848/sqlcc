#include "ast_node.h"
#include "../data_types.h"
#include "../token.h"
#include "ast_nodes.h"
#include <algorithm>
#include <cctype>
#include <iostream>

namespace sqlcc {
namespace sql_parser {

// ==================== ColumnDefinition ====================

ColumnDefinition::ColumnDefinition(const std::string &name,
                                   const std::string &type)
    : name_(name), type_(type), dataType_(DataType::UNKNOWN),
      isPrimaryKey_(false), isNullable_(true), isUnique_(false),
      isAutoIncrement_(false), precision_(18), scale_(0) {
  setType(type);  // 解析类型字符串并设置dataType_
}

ColumnDefinition::~ColumnDefinition() {}

void ColumnDefinition::setType(const std::string &type) {
  type_ = type;
  DataTypeManager& manager = DataTypeManager::getInstance();

  // 解析DECIMAL类型
  int precision, scale;
  if (manager.parseDecimalType(type, precision, scale)) {
    dataType_ = DataType::DECIMAL;
    precision_ = precision;
    scale_ = scale;
  } else {
    dataType_ = manager.getTypeFromName(type);
  }
}

void ColumnDefinition::setDefaultValue(const std::string &defaultValue) {
  defaultValue_ = defaultValue;
}

// ==================== TableConstraint ====================

TableConstraint::TableConstraint(Type type, const std::string &name)
    : type_(type), constraintName_(name) {}

TableConstraint::~TableConstraint() {}

void TableConstraint::addColumn(const std::string &column) {
  columns_.push_back(column);
}

void TableConstraint::setReferencedTable(const std::string &table) {
  referencedTable_ = table;
}

void TableConstraint::addReferencedColumn(const std::string &column) {
  referencedColumns_.push_back(column);
}

void TableConstraint::setCheckExpression(const std::string &expression) {
  checkExpression_ = expression;
}

TableConstraint::Type TableConstraint::getType() const { return type_; }

const std::string &TableConstraint::getConstraintName() const {
  return constraintName_;
}

const std::vector<std::string> &TableConstraint::getColumns() const {
  return columns_;
}

const std::string &TableConstraint::getReferencedTable() const {
  return referencedTable_;
}

const std::vector<std::string> &TableConstraint::getReferencedColumns() const {
  return referencedColumns_;
}

const std::string &TableConstraint::getCheckExpression() const {
  return checkExpression_;
}

// ==================== WhereClause ====================

WhereClause::WhereClause(const std::string &columnName, const std::string &op,
                         const std::string &value)
    : columnName_(columnName), op_(op), value_(value) {}

WhereClause::~WhereClause() {}

const std::string &WhereClause::getColumnName() const { return columnName_; }

const std::string &WhereClause::getOp() const { return op_; }

const std::string &WhereClause::getValue() const { return value_; }

// ==================== CreateStatement ====================

CreateStatement::CreateStatement(ObjectType objectType,
                                 const std::string &objectName)
    : Statement(CREATE), objectType_(objectType), objectName_(objectName) {}

CreateStatement::~CreateStatement() {}

CreateStatement::CreateStatement(ObjectType objectType)
    : Statement(CREATE), objectType_(objectType), objectName_("") {}

void CreateStatement::addColumn(ColumnDefinition &&column) {
  columns_.push_back(std::move(column));
}

void CreateStatement::addConstraint(TableConstraint &&constraint) {
  constraints_.push_back(std::move(constraint));
}

void CreateStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

CreateStatement::ObjectType CreateStatement::getObjectType() const {
  return objectType_;
}

// Non-const version for compatibility (remove duplicate signature)
const std::string &CreateStatement::getObjectName() const {
  return objectName_;
}

// Non-const compatibility getters (restore symbols expected by other TUs)
CreateStatement::ObjectType CreateStatement::getObjectType() {
  return objectType_;
}

std::string CreateStatement::getObjectName() { return objectName_; }

const std::vector<ColumnDefinition> &CreateStatement::getColumns() const {
  return columns_;
}
const std::vector<TableConstraint> &CreateStatement::getConstraints() const {
  return constraints_;
}

// ==================== CreateViewStatement ====================

CreateViewStatement::CreateViewStatement(const std::string &viewName)
    : Statement(CREATE_VIEW), viewName_(viewName) {}

CreateViewStatement::~CreateViewStatement() {}

void CreateViewStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

const std::string &CreateViewStatement::getViewName() const {
  return viewName_;
}

const std::vector<std::string> &CreateViewStatement::getColumnNames() const {
  return columnNames_;
}

const SelectStatement &CreateViewStatement::getSelectStatement() const {
  return *selectStatement_;
}

void CreateViewStatement::addColumnName(const std::string &columnName) {
  columnNames_.push_back(columnName);
}

void CreateViewStatement::setSelectStatement(std::unique_ptr<SelectStatement> selectStmt) {
  selectStatement_ = std::move(selectStmt);
}

bool CreateViewStatement::hasColumnNames() const {
  return !columnNames_.empty();
}

SelectStatement::SelectStatement()
    : Statement(SELECT), joinCondition_(""), limit_(-1), offset_(0),
      selectAll_(false), distinct_(false), hasLimit_(false), hasOffset_(false) {}

SelectStatement::~SelectStatement() {}

void SelectStatement::addJoinClause(std::unique_ptr<JoinClause> join) {
  joinClauses_.push_back(std::move(join));
}

void SelectStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

// ==================== JoinClause ====================

JoinClause::JoinClause(JoinType type, const std::string &tableName,
                       std::unique_ptr<Expression> condition)
    : joinType_(type), tableName_(tableName), condition_(std::move(condition)) {}

JoinClause::~JoinClause() {}

JoinClause::JoinType JoinClause::getJoinType() const {
  return joinType_;
}

void JoinClause::setJoinType(JoinType type) {
  joinType_ = type;
}

const std::string &JoinClause::getTableName() const {
  return tableName_;
}

const Expression *JoinClause::getCondition() const {
  return condition_.get();
}

std::unique_ptr<Expression> JoinClause::takeCondition() {
  return std::move(condition_);
}

void JoinClause::setCondition(std::unique_ptr<Expression> condition) {
  condition_ = std::move(condition);
}

void SelectStatement::addSelectColumn(const std::string &column) {
  selectColumns_.push_back(column);
}

void SelectStatement::addSelectItem(const std::string &column) {
  selectColumns_.push_back(column);
}

void SelectStatement::addFromTable(const std::string &table) {
  fromTables_.push_back(table);
}

void SelectStatement::setTableName(const std::string &table) {
  tableName_ = table;
}

void SelectStatement::setWhereClause(const WhereClause &where) {
  whereClause_ = where;
}

void SelectStatement::setWhereExpression(std::unique_ptr<Expression> expr) {
  whereExpression_ = std::move(expr);
}

void SelectStatement::setGroupByColumn(const std::string &column) {
  groupByColumn_ = column;
}

void SelectStatement::addGroupByColumn(const std::string &column) {
  groupByColumns_.push_back(column);
}

void SelectStatement::setHavingClause(std::unique_ptr<Expression> expr) {
  havingClause_ = std::move(expr);
}

void SelectStatement::setDistinct(bool distinct) {
  distinct_ = distinct;
}

bool SelectStatement::isDistinct() const {
  return distinct_;
}

void SelectStatement::setOrderByColumn(const std::string &column) {
  orderByColumn_ = column;
}

void SelectStatement::setOrderDirection(const std::string &direction) {
  orderDirection_ = direction;
}

void SelectStatement::setSelectAll(bool selectAll) { selectAll_ = selectAll; }

void SelectStatement::setJoinCondition(const std::string &condition) {
  joinCondition_ = condition;
}

void SelectStatement::setLimit(int limit) {
  limit_ = limit;
  hasLimit_ = true;
}

void SelectStatement::setOffset(int offset) {
  offset_ = offset;
  hasOffset_ = true;
}

const std::vector<std::string> &SelectStatement::getSelectColumns() const {
  return selectColumns_;
}

const std::string &SelectStatement::getTableName() const { return tableName_; }

const WhereClause &SelectStatement::getWhereClause() const {
  return whereClause_;
}

const Expression *SelectStatement::getWhereExpression() const {
  return whereExpression_.get();
}

const std::string &SelectStatement::getGroupByColumn() const {
  return groupByColumn_;
}

const std::vector<std::string> &SelectStatement::getGroupByColumns() const {
  return groupByColumns_;
}

const Expression *SelectStatement::getHavingClause() const {
  return havingClause_.get();
}

const std::string &SelectStatement::getOrderByColumn() const {
  return orderByColumn_;
}

const std::string &SelectStatement::getOrderDirection() const {
  return orderDirection_;
}

const std::string &SelectStatement::getJoinCondition() const {
  return joinCondition_;
}

int SelectStatement::getLimit() const { return limit_; }

int SelectStatement::getOffset() const { return offset_; }

bool SelectStatement::isSelectAll() const { return selectAll_; }

bool SelectStatement::hasWhereClause() const {
  return !whereClause_.getColumnName().empty();
}

bool SelectStatement::hasWhereExpression() const {
  return whereExpression_ != nullptr;
}

bool SelectStatement::hasGroupBy() const { return !groupByColumn_.empty() || !groupByColumns_.empty(); }

bool SelectStatement::hasHavingClause() const { return havingClause_ != nullptr; }

bool SelectStatement::hasOrderBy() const { return !orderByColumn_.empty(); }

bool SelectStatement::hasJoinCondition() const {
  return !joinCondition_.empty();
}

bool SelectStatement::hasLimit() const { return hasLimit_; }

bool SelectStatement::hasOffset() const { return hasOffset_; }

// ==================== CompositeSelectStatement ====================

// Implement the missing accept method
void CompositeSelectStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

// ==================== InsertStatement ====================

InsertStatement::InsertStatement() : Statement(INSERT), tableName_("") {}

InsertStatement::InsertStatement(const std::string &tableName)
    : Statement(INSERT), tableName_(tableName) {}

InsertStatement::~InsertStatement() {}

void InsertStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

void InsertStatement::addColumn(const std::string &column) {
  columns_.push_back(column);
}

void InsertStatement::addValue(const std::string &value) {
  currentRow_.push_back(value);
}

void InsertStatement::finishRow() {
  if (!currentRow_.empty()) {
    values_.push_back(std::move(currentRow_));
    currentRow_.clear();
  }
}

void InsertStatement::addValueRow(
    const std::vector<std::unique_ptr<Expression>> &values) {
  // 这里仅作演示，实际实现可能需要更复杂的处理
  for (const auto &value : values) {
    // 忽略未使用的变量警告
    (void)value;
  }
}

const std::string &InsertStatement::getTableName() const { return tableName_; }

const std::vector<std::string> &InsertStatement::getColumns() const {
  return columns_;
}

const std::vector<std::vector<std::string>> &
InsertStatement::getValues() const {
  return values_;
}

// ==================== UpdateStatement ====================

UpdateStatement::UpdateStatement() : Statement(UPDATE), tableName_("") {}

UpdateStatement::UpdateStatement(const std::string &tableName)
    : Statement(UPDATE), tableName_(tableName) {}

UpdateStatement::~UpdateStatement() {}

void UpdateStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

void UpdateStatement::addUpdateValue(const std::string &column,
                                     const std::string &value) {
  updateValues_[column] = value;
}

void UpdateStatement::setWhereClause(const WhereClause &where) {
  whereClause_ = where;
}

const std::string &UpdateStatement::getTableName() const { return tableName_; }

// Non-const version for compatibility
std::string UpdateStatement::getTableName() { return tableName_; }

const std::unordered_map<std::string, std::string> &
UpdateStatement::getUpdateValues() const {
  return updateValues_;
}

const WhereClause &UpdateStatement::getWhereClause() const {
  return whereClause_;
}

bool UpdateStatement::hasWhereClause() const {
  return !whereClause_.getColumnName().empty();
}

// ==================== DeleteStatement ====================

DeleteStatement::DeleteStatement() : Statement(DELETE), tableName_("") {}

DeleteStatement::DeleteStatement(const std::string &tableName)
    : Statement(DELETE), tableName_(tableName) {}

DeleteStatement::~DeleteStatement() {}

void DeleteStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

void DeleteStatement::setWhereClause(const WhereClause &where) {
  whereClause_ = where;
}

const std::string &DeleteStatement::getTableName() const { return tableName_; }

// Non-const version for compatibility
std::string DeleteStatement::getTableName() { return tableName_; }

const WhereClause &DeleteStatement::getWhereClause() const {
  return whereClause_;
}

bool DeleteStatement::hasWhereClause() const {
  return !whereClause_.getColumnName().empty();
}

// ==================== DropStatement ====================

DropStatement::DropStatement(ObjectType objectType,
                             const std::string &objectName)
    : Statement(DROP), objectType_(objectType), objectName_(objectName),
      ifExists_(false) {}

DropStatement::~DropStatement() {}

DropStatement::DropStatement(ObjectType objectType)
    : Statement(DROP), objectType_(objectType), objectName_(""),
      ifExists_(false) {}

void DropStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

void DropStatement::setIfExists(bool ifExists) { ifExists_ = ifExists; }

DropStatement::ObjectType DropStatement::getObjectType() const {
  return objectType_;
}

// Non-const version for compatibility
DropStatement::ObjectType DropStatement::getObjectType() {
  return objectType_;
}

const std::string &DropStatement::getObjectName() const { return objectName_; }

// Non-const version for compatibility
std::string DropStatement::getObjectName() {
  return objectName_;
}
bool DropStatement::isIfExists() const { return ifExists_; }

// ==================== AlterStatement ====================

AlterStatement::AlterStatement(Target target)
    : Statement(ALTER), target_(target), action_(ADD_COLUMN), 
      columnDef_("temp", "temp") {}  // 提供默认的ColumnDefinition构造参数

AlterStatement::~AlterStatement() {}

// Getters
AlterStatement::Target AlterStatement::getTarget() const { return target_; }

AlterStatement::Action AlterStatement::getAction() const { return action_; }

const std::string& AlterStatement::getDatabaseName() const { return databaseName_; }

const std::string& AlterStatement::getTableName() const { return tableName_; }

const std::string& AlterStatement::getColumnName() const { return columnName_; }

const ColumnDefinition& AlterStatement::getColumnDefinition() const { return columnDef_; }

const std::string& AlterStatement::getIndexName() const { return indexName_; }

const std::string& AlterStatement::getNewTableName() const { return newTableName_; }

// Setters
void AlterStatement::setDatabaseName(const std::string& name) { databaseName_ = name; }

void AlterStatement::setTableName(const std::string& name) { tableName_ = name; }

void AlterStatement::setAction(Action action) { action_ = action; }

void AlterStatement::setColumnName(const std::string& name) { columnName_ = name; }

void AlterStatement::setColumnDefinition(ColumnDefinition&& columnDef) { columnDef_ = std::move(columnDef); }

void AlterStatement::setIndexName(const std::string& name) { indexName_ = name; }

void AlterStatement::setNewTableName(const std::string& newName) { newTableName_ = newName; }

void AlterStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

UseStatement::UseStatement(const std::string &databaseName)
    : Statement(USE), databaseName_(databaseName) {}

UseStatement::~UseStatement() {}

void UseStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

const std::string &UseStatement::getDatabaseName() const {
  return databaseName_;
}

// Non-const version for compatibility
std::string UseStatement::getDatabaseName() {
  return databaseName_;
}

CreateIndexStatement::CreateIndexStatement(const std::string &indexName,
                                           const std::string &tableName,
                                           const std::string &columnName)
    : Statement(CREATE_INDEX), indexName_(indexName), tableName_(tableName),
      unique_(false) {
  columns_.push_back(columnName);
}

CreateIndexStatement::~CreateIndexStatement() {}

void CreateIndexStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

void CreateIndexStatement::addColumn(const std::string &column) {
  columns_.push_back(column);
}

const std::string &CreateIndexStatement::getIndexName() const {
  return indexName_;
}

const std::string &CreateIndexStatement::getTableName() const {
  return tableName_;
}

// Non-const version for compatibility
std::string CreateIndexStatement::getTableName() {
  return tableName_;
}

const std::string &CreateIndexStatement::getColumnName() const {
  static const std::string emptyString = "";
  return columns_.empty() ? emptyString : columns_[0];
}

const std::vector<std::string> &CreateIndexStatement::getColumns() const {
  return columns_;
}

void CreateIndexStatement::setUnique(bool unique) { unique_ = unique; }

bool CreateIndexStatement::isUnique() const { return unique_; }

// ==================== DropIndexStatement ====================

DropIndexStatement::DropIndexStatement(const std::string &indexName)
    : Statement(DROP_INDEX), indexName_(indexName), ifExists_(false),
      hasTableName_(false) {}

DropIndexStatement::~DropIndexStatement() {}

void DropIndexStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

const std::string &DropIndexStatement::getIndexName() const {
  return indexName_;
}

void DropIndexStatement::setTableName(const std::string &tableName) {
  tableName_ = tableName;
  hasTableName_ = true;
}

const std::string &DropIndexStatement::getTableName() const {
  return tableName_;
}

bool DropIndexStatement::hasTableName() const { return hasTableName_; }

void DropIndexStatement::setIfExists(bool ifExists) { ifExists_ = ifExists; }

bool DropIndexStatement::isIfExists() const { return ifExists_; }

// ==================== CreateUserStatement ====================

CreateUserStatement::CreateUserStatement(const std::string &username,
                                         const std::string &password)
    : Statement(CREATE_USER), username_(username), password_(password),
      withPassword_(false) {}

CreateUserStatement::~CreateUserStatement() {}

void CreateUserStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

const std::string &CreateUserStatement::getUsername() const {
  return username_;
}

// Non-const version for compatibility
std::string CreateUserStatement::getUsername() {
  return username_;
}

const std::string &CreateUserStatement::getPassword() const {
  return password_;
}

bool CreateUserStatement::isWithPassword() const { return withPassword_; }

void CreateUserStatement::setWithPassword(bool withPassword) {
  withPassword_ = withPassword;
}

// ==================== DropUserStatement ====================

DropUserStatement::DropUserStatement(const std::string &username)
    : Statement(DROP_USER), username_(username), ifExists_(false) {}

DropUserStatement::~DropUserStatement() {}

void DropUserStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

const std::string &DropUserStatement::getUsername() const { return username_; }

// Non-const version for compatibility
std::string DropUserStatement::getUsername() {
  return username_;
}

bool DropUserStatement::isIfExists() const { return ifExists_; }

void DropUserStatement::setIfExists(bool ifExists) { ifExists_ = ifExists; }

// ==================== GrantStatement ====================

GrantStatement::GrantStatement() : Statement(GRANT) {}

GrantStatement::~GrantStatement() {}

void GrantStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

void GrantStatement::addPrivilege(const std::string &privilege) {
  privileges_.push_back(privilege);
}

const std::vector<std::string> &GrantStatement::getPrivileges() const {
  return privileges_;
}

void GrantStatement::setObjectType(const std::string &objectType) {
  objectType_ = objectType;
}

const std::string &GrantStatement::getObjectType() const { return objectType_; }

void GrantStatement::setObjectName(const std::string &objectName) {
  objectName_ = objectName;
}

const std::string &GrantStatement::getObjectName() const { return objectName_; }

void GrantStatement::setGrantee(const std::string &grantee) {
  grantee_ = grantee;
}

const std::string &GrantStatement::getGrantee() const { return grantee_; }

// Non-const version for compatibility
std::string GrantStatement::getGrantee() {
  return grantee_;
}

// ==================== RevokeStatement ====================

RevokeStatement::RevokeStatement() : Statement(REVOKE) {}

RevokeStatement::~RevokeStatement() {}

void RevokeStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

void RevokeStatement::addPrivilege(const std::string &privilege) {
  privileges_.push_back(privilege);
}

const std::vector<std::string> &RevokeStatement::getPrivileges() const {
  return privileges_;
}

void RevokeStatement::setObjectType(const std::string &objectType) {
  objectType_ = objectType;
}

const std::string &RevokeStatement::getObjectType() const {
  return objectType_;
}

void RevokeStatement::setObjectName(const std::string &objectName) {
  objectName_ = objectName;
}

const std::string &RevokeStatement::getObjectName() const {
  return objectName_;
}

void RevokeStatement::setGrantee(const std::string &grantee) {
  grantee_ = grantee;
}

const std::string &RevokeStatement::getGrantee() const { return grantee_; }

// Non-const version for compatibility
std::string RevokeStatement::getGrantee() {
  return grantee_;
}

// ==================== ShowStatement ====================

ShowStatement::ShowStatement(ShowType type)
    : Statement(SHOW), type_(type), hasFromDb_(false) {}

ShowStatement::~ShowStatement() {}

void ShowStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

ShowStatement::ShowType ShowStatement::getShowType() const { return type_; }

void ShowStatement::setTargetObject(const std::string &target) {
  targetObject_ = target;
}

const std::string &ShowStatement::getTargetObject() const {
  return targetObject_;
}

void ShowStatement::setFromDatabase(const std::string &dbName) {
  fromDatabase_ = dbName;
  hasFromDb_ = true;
}

const std::string &ShowStatement::getFromDatabase() const {
  return fromDatabase_;
}

bool ShowStatement::hasFromDatabase() const { return hasFromDb_; }

// ==================== Expression Classes ====================

IdentifierExpression::IdentifierExpression(const std::string &name) : name_(name) {}

IdentifierExpression::~IdentifierExpression() {}

const std::string &IdentifierExpression::getName() const { return name_; }

std::string IdentifierExpression::getTypeName() const { return "IdentifierExpression"; }

void IdentifierExpression::accept(NodeVisitor &visitor) { visitor.visit(*this); }

StringLiteralExpression::StringLiteralExpression(const std::string &value) : value_(value) {}

StringLiteralExpression::~StringLiteralExpression() {}

const std::string &StringLiteralExpression::getValue() const { return value_; }

std::string StringLiteralExpression::getTypeName() const { return "StringLiteralExpression"; }

void StringLiteralExpression::accept(NodeVisitor &visitor) { visitor.visit(*this); }

NumericLiteralExpression::NumericLiteralExpression(double value) : value_(value) {}

NumericLiteralExpression::~NumericLiteralExpression() {}

double NumericLiteralExpression::getValue() const { return value_; }

std::string NumericLiteralExpression::getTypeName() const { return "NumericLiteralExpression"; }

void NumericLiteralExpression::accept(NodeVisitor &visitor) { visitor.visit(*this); }

BooleanLiteralExpression::BooleanLiteralExpression(bool value) : value_(value) {}

BooleanLiteralExpression::~BooleanLiteralExpression() {}

bool BooleanLiteralExpression::getValue() const { return value_; }

std::string BooleanLiteralExpression::getTypeName() const { return "BooleanLiteralExpression"; }

void BooleanLiteralExpression::accept(NodeVisitor &visitor) { visitor.visit(*this); }

NullLiteralExpression::NullLiteralExpression() {}

NullLiteralExpression::~NullLiteralExpression() {}

std::string NullLiteralExpression::getTypeName() const { return "NullLiteralExpression"; }

void NullLiteralExpression::accept(NodeVisitor &visitor) { visitor.visit(*this); }

// ==================== CommitStatement ====================

CommitStatement::CommitStatement() : Statement(COMMIT) {}

CommitStatement::~CommitStatement() {}

void CommitStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

// ==================== RollbackStatement ====================

RollbackStatement::RollbackStatement() : Statement(ROLLBACK) {}

RollbackStatement::~RollbackStatement() {}

void RollbackStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

// ==================== ProcedureParameter ====================

ProcedureParameter::ProcedureParameter(const std::string &name,
                                       const std::string &type, Mode mode)
    : name_(name), type_(type), mode_(mode) {}

ProcedureParameter::~ProcedureParameter() {}

const std::string &ProcedureParameter::getName() const { return name_; }

const std::string &ProcedureParameter::getType() const { return type_; }

ProcedureParameter::Mode ProcedureParameter::getMode() const { return mode_; }

std::string ProcedureParameter::getModeString() const {
  switch (mode_) {
  case IN:
    return "IN";
  case OUT:
    return "OUT";
  case INOUT:
    return "INOUT";
  default:
    return "UNKNOWN";
  }
}

// ==================== CreateProcedureStatement ====================

CreateProcedureStatement::CreateProcedureStatement(const std::string &name)
    : CreateStatement(CreateStatement::PROCEDURE, name), name_(name) {}

CreateProcedureStatement::~CreateProcedureStatement() {}

void CreateProcedureStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

void CreateProcedureStatement::addParameter(const ProcedureParameter &param) {
  parameters_.push_back(param);
}

const std::vector<ProcedureParameter> &
CreateProcedureStatement::getParameters() const {
  return parameters_;
}

void CreateProcedureStatement::setBody(const std::string &body) {
  body_ = body;
}

const std::string &CreateProcedureStatement::getBody() const { return body_; }

const std::string &CreateProcedureStatement::getName() const { return name_; }

// ==================== CallProcedureStatement ====================

CallProcedureStatement::CallProcedureStatement(const std::string &name)
    : Statement(CALL_PROCEDURE), name_(name) {}

CallProcedureStatement::~CallProcedureStatement() {}

void CallProcedureStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

void CallProcedureStatement::addArgument(std::unique_ptr<Expression> arg) {
  arguments_.push_back(std::move(arg));
}

const std::vector<std::unique_ptr<Expression>> &
CallProcedureStatement::getArguments() const {
  return arguments_;
}

const std::string &CallProcedureStatement::getName() const { return name_; }

// ==================== DropProcedureStatement ====================

DropProcedureStatement::DropProcedureStatement(const std::string &name)
    : Statement(DROP_PROCEDURE), name_(name) {}

DropProcedureStatement::~DropProcedureStatement() {}

void DropProcedureStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

const std::string &DropProcedureStatement::getName() const { return name_; }

// ==================== TriggerDefinition ====================

TriggerDefinition::TriggerDefinition(const std::string &name, Timing timing,
                                     Event event, Level level,
                                     const std::string &tableName)
    : name_(name), timing_(timing), event_(event), level_(level),
      tableName_(tableName), hasCondition_(false) {}

TriggerDefinition::~TriggerDefinition() {}

const std::string &TriggerDefinition::getName() const { return name_; }

TriggerDefinition::Timing TriggerDefinition::getTiming() const {
  return timing_;
}

std::string TriggerDefinition::getTimingString() const {
  switch (timing_) {
  case BEFORE:
    return "BEFORE";
  case AFTER:
    return "AFTER";
  case INSTEAD_OF:
    return "INSTEAD OF";
  default:
    return "UNKNOWN";
  }
}

TriggerDefinition::Event TriggerDefinition::getEvent() const { return event_; }

std::string TriggerDefinition::getEventString() const {
  switch (event_) {
  case INSERT:
    return "INSERT";
  case UPDATE:
    return "UPDATE";
  case DELETE:
    return "DELETE";
  default:
    return "UNKNOWN";
  }
}

TriggerDefinition::Level TriggerDefinition::getLevel() const { return level_; }

std::string TriggerDefinition::getLevelString() const {
  switch (level_) {
  case ROW:
    return "ROW";
  case STATEMENT:
    return "STATEMENT";
  default:
    return "UNKNOWN";
  }
}

const std::string &TriggerDefinition::getTableName() const {
  return tableName_;
}

void TriggerDefinition::setCondition(const std::string &condition) {
  condition_ = condition;
  hasCondition_ = true;
}

const std::string &TriggerDefinition::getCondition() const {
  return condition_;
}

// Accept methods are already defined inline in the class implementations above


bool TriggerDefinition::hasCondition() const { return hasCondition_; }

void TriggerDefinition::setBody(const std::string &body) { body_ = body; }

const std::string &TriggerDefinition::getBody() const { return body_; }

// ==================== CreateTriggerStatement ====================

CreateTriggerStatement::CreateTriggerStatement(
    const TriggerDefinition &triggerDef)
    : CreateStatement(CreateStatement::TRIGGER, triggerDef.getName()), triggerDef_(triggerDef) {}

CreateTriggerStatement::~CreateTriggerStatement() {}

void CreateTriggerStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

const TriggerDefinition &CreateTriggerStatement::getTriggerDefinition() const {
  return triggerDef_;
}

// ==================== DropTriggerStatement ====================

DropTriggerStatement::DropTriggerStatement(const std::string &name)
    : Statement(DROP_TRIGGER), name_(name) {}

DropTriggerStatement::~DropTriggerStatement() {}

void DropTriggerStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

const std::string &DropTriggerStatement::getName() const { return name_; }

// ==================== AlterTriggerStatement ====================

AlterTriggerStatement::AlterTriggerStatement(const std::string &name,
                                             Action action)
    : Statement(ALTER_TRIGGER), name_(name), action_(action) {}

AlterTriggerStatement::~AlterTriggerStatement() {}

void AlterTriggerStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

const std::string &AlterTriggerStatement::getName() const { return name_; }

AlterTriggerStatement::Action AlterTriggerStatement::getAction() const {
  return action_;
}

std::string AlterTriggerStatement::getActionString() const {
  switch (action_) {
  case ENABLE:
    return "ENABLE";
  case DISABLE:
    return "DISABLE";
  default:
    return "UNKNOWN";
  }
}

// ==================== CreateUserStatement ====================

// CreateUserStatement methods are already defined above (around line 450)

// ==================== DropUserStatement ====================

// DropUserStatement methods are already defined above (around line 450)

// ==================== GrantStatement ====================

// GrantStatement methods are already defined above (around line 450)

// ==================== RevokeStatement ====================

// RevokeStatement methods are already defined above (around line 450)

// Force generation of typeinfo for all classes
// This ensures the linker can find the typeinfo by actually instantiating classes
namespace {
    // Expression classes
    sqlcc::sql_parser::Expression dummy_expression;
    sqlcc::sql_parser::BinaryExpression dummy_binary_expr(nullptr, nullptr, sqlcc::sql_parser::TokenType::OPERATOR_PLUS);
    sqlcc::sql_parser::IdentifierExpression dummy_identifier("");
    sqlcc::sql_parser::StringLiteralExpression dummy_string("");
    sqlcc::sql_parser::NumericLiteralExpression dummy_numeric(0.0);
    sqlcc::sql_parser::BooleanLiteralExpression dummy_boolean(true);
    sqlcc::sql_parser::NullLiteralExpression dummy_null;

    // CreateViewStatement class
    sqlcc::sql_parser::CreateViewStatement dummy_create_view("");

    // Statement classes
    sqlcc::sql_parser::CreateStatement dummy_create(sqlcc::sql_parser::CreateStatement::TABLE, "");
    sqlcc::sql_parser::SelectStatement dummy_select;
    sqlcc::sql_parser::CompositeSelectStatement dummy_composite_select;
    sqlcc::sql_parser::InsertStatement dummy_insert("");
    sqlcc::sql_parser::UpdateStatement dummy_update("");
    sqlcc::sql_parser::DeleteStatement dummy_delete("");
    sqlcc::sql_parser::DropStatement dummy_drop(sqlcc::sql_parser::DropStatement::TABLE, "");
    sqlcc::sql_parser::AlterStatement dummy_alter(sqlcc::sql_parser::AlterStatement::TABLE);
    sqlcc::sql_parser::UseStatement dummy_use("");
    sqlcc::sql_parser::CreateIndexStatement dummy_create_index("", "", "");
    sqlcc::sql_parser::DropIndexStatement dummy_drop_index("");
    sqlcc::sql_parser::CreateUserStatement dummy_create_user("", "");
    sqlcc::sql_parser::DropUserStatement dummy_drop_user("");
    sqlcc::sql_parser::GrantStatement dummy_grant;
    sqlcc::sql_parser::RevokeStatement dummy_revoke;
    sqlcc::sql_parser::ShowStatement dummy_show(sqlcc::sql_parser::ShowStatement::TABLES);
    sqlcc::sql_parser::CommitStatement dummy_commit;
    sqlcc::sql_parser::RollbackStatement dummy_rollback;
    sqlcc::sql_parser::CreateProcedureStatement dummy_create_proc("");
    sqlcc::sql_parser::CallProcedureStatement dummy_call_proc("");
    sqlcc::sql_parser::DropProcedureStatement dummy_drop_proc("");
    sqlcc::sql_parser::CreateTriggerStatement dummy_create_trigger(sqlcc::sql_parser::TriggerDefinition("", sqlcc::sql_parser::TriggerDefinition::BEFORE, sqlcc::sql_parser::TriggerDefinition::INSERT, sqlcc::sql_parser::TriggerDefinition::ROW, ""));
    sqlcc::sql_parser::DropTriggerStatement dummy_drop_trigger("");
    sqlcc::sql_parser::AlterTriggerStatement dummy_alter_trigger("", sqlcc::sql_parser::AlterTriggerStatement::ENABLE);

    // Get type names to ensure typeinfo is generated
    volatile const char* type_names[] = {
        typeid(sqlcc::sql_parser::Expression).name(),
        typeid(sqlcc::sql_parser::BinaryExpression).name(),
        typeid(sqlcc::sql_parser::IdentifierExpression).name(),
        typeid(sqlcc::sql_parser::StringLiteralExpression).name(),
        typeid(sqlcc::sql_parser::NumericLiteralExpression).name(),
        typeid(sqlcc::sql_parser::BooleanLiteralExpression).name(),
        typeid(sqlcc::sql_parser::NullLiteralExpression).name(),
        typeid(sqlcc::sql_parser::CreateStatement).name(),
        typeid(sqlcc::sql_parser::SelectStatement).name(),
        typeid(sqlcc::sql_parser::CompositeSelectStatement).name(),
        typeid(sqlcc::sql_parser::InsertStatement).name(),
        typeid(sqlcc::sql_parser::UpdateStatement).name(),
        typeid(sqlcc::sql_parser::DeleteStatement).name(),
        typeid(sqlcc::sql_parser::DropStatement).name(),
        typeid(sqlcc::sql_parser::AlterStatement).name(),
        typeid(sqlcc::sql_parser::UseStatement).name(),
        typeid(sqlcc::sql_parser::CreateIndexStatement).name(),
        typeid(sqlcc::sql_parser::DropIndexStatement).name(),
        typeid(sqlcc::sql_parser::CreateUserStatement).name(),
        typeid(sqlcc::sql_parser::DropUserStatement).name(),
        typeid(sqlcc::sql_parser::GrantStatement).name(),
        typeid(sqlcc::sql_parser::RevokeStatement).name(),
        typeid(sqlcc::sql_parser::ShowStatement).name(),
        typeid(sqlcc::sql_parser::CommitStatement).name(),
        typeid(sqlcc::sql_parser::RollbackStatement).name(),
        typeid(sqlcc::sql_parser::CreateProcedureStatement).name(),
        typeid(sqlcc::sql_parser::CallProcedureStatement).name(),
        typeid(sqlcc::sql_parser::DropProcedureStatement).name(),
        typeid(sqlcc::sql_parser::CreateTriggerStatement).name(),
        typeid(sqlcc::sql_parser::DropTriggerStatement).name(),
        typeid(sqlcc::sql_parser::AlterTriggerStatement).name()
    };
} // anonymous namespace

} // namespace sql_parser
} // namespace sqlcc
