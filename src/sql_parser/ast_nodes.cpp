#include "sql_parser/ast_nodes.h"
#include <iostream>
#include <algorithm>
#include <cctype>

namespace sqlcc {
namespace sql_parser {

// ==================== ColumnDefinition ====================

ColumnDefinition::ColumnDefinition(const std::string& name, const std::string& type)
    : name_(name), type_(type), isPrimaryKey_(false), isNullable_(true), 
      isUnique_(false), isAutoIncrement_(false) {
}

ColumnDefinition::~ColumnDefinition() {
}

void ColumnDefinition::setDefaultValue(const std::string& defaultValue) {
    defaultValue_ = defaultValue;
}

// ==================== TableConstraint ====================

TableConstraint::TableConstraint(Type type, const std::string& name)
    : type_(type), constraintName_(name) {
}

TableConstraint::~TableConstraint() {
}

void TableConstraint::addColumn(const std::string& column) {
    columns_.push_back(column);
}

void TableConstraint::setReferencedTable(const std::string& table) {
    referencedTable_ = table;
}

void TableConstraint::addReferencedColumn(const std::string& column) {
    referencedColumns_.push_back(column);
}

void TableConstraint::setCheckExpression(const std::string& expression) {
    checkExpression_ = expression;
}

// ==================== WhereClause ====================

WhereClause::WhereClause(const std::string& columnName, const std::string& op, const std::string& value)
    : columnName_(columnName), op_(op), value_(value) {
}

WhereClause::~WhereClause() {
}

const std::string& WhereClause::getColumnName() const {
    return columnName_;
}

const std::string& WhereClause::getOp() const {
    return op_;
}

const std::string& WhereClause::getValue() const {
    return value_;
}

// ==================== CreateStatement ====================

CreateStatement::CreateStatement(ObjectType objectType, const std::string& objectName)
    : Statement(CREATE), objectType_(objectType), objectName_(objectName) {
}

CreateStatement::~CreateStatement() {
}

CreateStatement::CreateStatement(ObjectType objectType)
    : Statement(CREATE), objectType_(objectType), objectName_("") {}

void CreateStatement::addColumn(ColumnDefinition&& column) {
    columns_.push_back(std::move(column));
}

void CreateStatement::addConstraint(TableConstraint&& constraint) {
    constraints_.push_back(std::move(constraint));
}

CreateStatement::ObjectType CreateStatement::getObjectType() const { return objectType_; }
const std::string& CreateStatement::getObjectName() const { return objectName_; }
const std::vector<ColumnDefinition>& CreateStatement::getColumns() const { return columns_; }
const std::vector<TableConstraint>& CreateStatement::getConstraints() const { return constraints_; }


SelectStatement::SelectStatement()
    : Statement(SELECT), selectAll_(false), joinCondition_(""), limit_(0), offset_(0), hasLimit_(false), hasOffset_(false) {
}

SelectStatement::~SelectStatement() {
}

void SelectStatement::addSelectColumn(const std::string& column) {
    selectColumns_.push_back(column);
}

void SelectStatement::setTableName(const std::string& table) {
    tableName_ = table;
}

void SelectStatement::setWhereClause(const WhereClause& where) {
    whereClause_ = where;
}

void SelectStatement::setGroupByColumn(const std::string& column) {
    groupByColumn_ = column;
}

void SelectStatement::setOrderByColumn(const std::string& column) {
    orderByColumn_ = column;
}

void SelectStatement::setOrderDirection(const std::string& direction) {
    orderDirection_ = direction;
}

void SelectStatement::setSelectAll(bool selectAll) {
    selectAll_ = selectAll;
}

void SelectStatement::setJoinCondition(const std::string& condition) {
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

const std::vector<std::string>& SelectStatement::getSelectColumns() const {
    return selectColumns_;
}

const std::string& SelectStatement::getTableName() const {
    return tableName_;
}

const WhereClause& SelectStatement::getWhereClause() const {
    return whereClause_;
}

const std::string& SelectStatement::getGroupByColumn() const {
    return groupByColumn_;
}

const std::string& SelectStatement::getOrderByColumn() const {
    return orderByColumn_;
}

const std::string& SelectStatement::getOrderDirection() const {
    return orderDirection_;
}

const std::string& SelectStatement::getJoinCondition() const {
    return joinCondition_;
}

int SelectStatement::getLimit() const {
    return limit_;
}

int SelectStatement::getOffset() const {
    return offset_;
}

bool SelectStatement::isSelectAll() const {
    return selectAll_;
}

bool SelectStatement::hasWhereClause() const {
    return !whereClause_.getColumnName().empty();
}

bool SelectStatement::hasGroupBy() const {
    return !groupByColumn_.empty();
}

bool SelectStatement::hasOrderBy() const {
    return !orderByColumn_.empty();
}

bool SelectStatement::hasJoinCondition() const {
    return !joinCondition_.empty();
}

bool SelectStatement::hasLimit() const {
    return hasLimit_;
}

bool SelectStatement::hasOffset() const {
    return hasOffset_;
}

// ==================== InsertStatement ====================

InsertStatement::InsertStatement(const std::string& tableName)
    : Statement(INSERT), tableName_(tableName) {
}

InsertStatement::~InsertStatement() {
}

void InsertStatement::addColumn(const std::string& column) {
    columns_.push_back(column);
}

void InsertStatement::addValue(const std::string& value) {
    currentRow_.push_back(value);
}

void InsertStatement::finishRow() {
    if (!currentRow_.empty()) {
        values_.push_back(currentRow_);
        currentRow_.clear();
    }
}

void InsertStatement::addValueRow(const std::vector<std::unique_ptr<Expression>>& values) {
    // 这里仅作演示，实际实现可能需要更复杂的处理
    for (const auto &value : values) {
        // 忽略未使用的变量警告
        (void)value;
    }
}

const std::string& InsertStatement::getTableName() const {
    return tableName_;
}

const std::vector<std::string>& InsertStatement::getColumns() const {
    return columns_;
}

const std::vector<std::vector<std::string>>& InsertStatement::getValues() const {
    return values_;
}

// ==================== UpdateStatement ====================

UpdateStatement::UpdateStatement(const std::string& tableName)
    : Statement(UPDATE), tableName_(tableName) {
}

UpdateStatement::~UpdateStatement() {
}

void UpdateStatement::addUpdateValue(const std::string& column, const std::string& value) {
    updateValues_[column] = value;
}

void UpdateStatement::setWhereClause(const WhereClause& where) {
    whereClause_ = where;
}

const std::string& UpdateStatement::getTableName() const {
    return tableName_;
}

const std::unordered_map<std::string, std::string>& UpdateStatement::getUpdateValues() const {
    return updateValues_;
}

const WhereClause& UpdateStatement::getWhereClause() const {
    return whereClause_;
}

bool UpdateStatement::hasWhereClause() const {
    return !whereClause_.getColumnName().empty();
}

// ==================== DeleteStatement ====================

DeleteStatement::DeleteStatement(const std::string& tableName)
    : Statement(DELETE), tableName_(tableName) {
}

DeleteStatement::~DeleteStatement() {
}

void DeleteStatement::setWhereClause(const WhereClause& where) {
    whereClause_ = where;
}

const std::string& DeleteStatement::getTableName() const {
    return tableName_;
}

const WhereClause& DeleteStatement::getWhereClause() const {
    return whereClause_;
}

bool DeleteStatement::hasWhereClause() const {
    return !whereClause_.getColumnName().empty();
}

// ==================== DropStatement ====================

DropStatement::DropStatement(ObjectType objectType, const std::string& objectName)
    : Statement(DROP), objectType_(objectType), objectName_(objectName), ifExists_(false) {
}

DropStatement::~DropStatement() {
}

DropStatement::DropStatement(ObjectType objectType)
    : Statement(DROP), objectType_(objectType), objectName_(""), ifExists_(false) {}

void DropStatement::setIfExists(bool ifExists) {
    ifExists_ = ifExists;
}

DropStatement::ObjectType DropStatement::getObjectType() const { return objectType_; }
const std::string& DropStatement::getObjectName() const { return objectName_; }
bool DropStatement::isIfExists() const { return ifExists_; }


AlterStatement::AlterStatement(ObjectType objectType, const std::string& objectName)
    : Statement(ALTER), objectType_(objectType), objectName_(objectName) {
}

AlterStatement::~AlterStatement() {
}

AlterStatement::AlterStatement(ObjectType objectType)
    : Statement(ALTER), objectType_(objectType), objectName_("") {}

AlterStatement::ObjectType AlterStatement::getObjectType() const { return objectType_; }
const std::string& AlterStatement::getObjectName() const { return objectName_; }


UseStatement::UseStatement(const std::string& databaseName)
    : Statement(USE), databaseName_(databaseName) {
}

UseStatement::~UseStatement() {
}

const std::string& UseStatement::getDatabaseName() const { return databaseName_; }


CreateIndexStatement::CreateIndexStatement(const std::string& indexName, const std::string& tableName, const std::string& columnName)
    : Statement(CREATE_INDEX), indexName_(indexName), tableName_(tableName), unique_(false) {
    columns_.push_back(columnName);
}

CreateIndexStatement::~CreateIndexStatement() {
}

void CreateIndexStatement::addColumn(const std::string& column) {
    columns_.push_back(column);
}

const std::string& CreateIndexStatement::getIndexName() const {
    return indexName_;
}

const std::string& CreateIndexStatement::getTableName() const {
    return tableName_;
}

const std::string& CreateIndexStatement::getColumnName() const {
    return columns_.empty() ? "" : columns_[0];
}

const std::vector<std::string>& CreateIndexStatement::getColumns() const {
    return columns_;
}

void CreateIndexStatement::setUnique(bool unique) {
    unique_ = unique;
}

bool CreateIndexStatement::isUnique() const {
    return unique_;
}

// ==================== DropIndexStatement ====================

DropIndexStatement::DropIndexStatement(const std::string& indexName)
    : Statement(DROP_INDEX), indexName_(indexName), ifExists_(false), hasTableName_(false) {
}

DropIndexStatement::~DropIndexStatement() {
}

const std::string& DropIndexStatement::getIndexName() const {
    return indexName_;
}

void DropIndexStatement::setTableName(const std::string& tableName) {
    tableName_ = tableName;
    hasTableName_ = true;
}

const std::string& DropIndexStatement::getTableName() const {
    return tableName_;
}

bool DropIndexStatement::hasTableName() const {
    return hasTableName_;
}

void DropIndexStatement::setIfExists(bool ifExists) {
    ifExists_ = ifExists;
}

bool DropIndexStatement::isIfExists() const {
    return ifExists_;
}

// ==================== CreateUserStatement ====================

CreateUserStatement::CreateUserStatement(const std::string& username, const std::string& password)
    : Statement(CREATE_USER), username_(username), password_(password), withPassword_(false) {
}

CreateUserStatement::~CreateUserStatement() {
}

const std::string& CreateUserStatement::getUsername() const {
    return username_;
}

const std::string& CreateUserStatement::getPassword() const {
    return password_;
}

bool CreateUserStatement::isWithPassword() const {
    return withPassword_;
}

void CreateUserStatement::setWithPassword(bool withPassword) {
    withPassword_ = withPassword;
}

// ==================== DropUserStatement ====================

DropUserStatement::DropUserStatement(const std::string& username)
    : Statement(DROP_USER), username_(username), ifExists_(false) {
}

DropUserStatement::~DropUserStatement() {
}

const std::string& DropUserStatement::getUsername() const {
    return username_;
}

bool DropUserStatement::isIfExists() const {
    return ifExists_;
}

void DropUserStatement::setIfExists(bool ifExists) {
    ifExists_ = ifExists;
}

// ==================== GrantStatement ====================

GrantStatement::GrantStatement()
    : Statement(GRANT) {
}

GrantStatement::~GrantStatement() {
}

void GrantStatement::addPrivilege(const std::string& privilege) {
    privileges_.push_back(privilege);
}

const std::vector<std::string>& GrantStatement::getPrivileges() const {
    return privileges_;
}

void GrantStatement::setObjectType(const std::string& objectType) {
    objectType_ = objectType;
}

const std::string& GrantStatement::getObjectType() const {
    return objectType_;
}

void GrantStatement::setObjectName(const std::string& objectName) {
    objectName_ = objectName;
}

const std::string& GrantStatement::getObjectName() const {
    return objectName_;
}

void GrantStatement::setGrantee(const std::string& grantee) {
    grantee_ = grantee;
}

const std::string& GrantStatement::getGrantee() const {
    return grantee_;
}

// ==================== RevokeStatement ====================

RevokeStatement::RevokeStatement()
    : Statement(REVOKE) {
}

RevokeStatement::~RevokeStatement() {
}

void RevokeStatement::addPrivilege(const std::string& privilege) {
    privileges_.push_back(privilege);
}

const std::vector<std::string>& RevokeStatement::getPrivileges() const {
    return privileges_;
}

void RevokeStatement::setObjectType(const std::string& objectType) {
    objectType_ = objectType;
}

const std::string& RevokeStatement::getObjectType() const {
    return objectType_;
}

void RevokeStatement::setObjectName(const std::string& objectName) {
    objectName_ = objectName;
}

const std::string& RevokeStatement::getObjectName() const {
    return objectName_;
}

void RevokeStatement::setGrantee(const std::string& grantee) {
    grantee_ = grantee;
}

const std::string& RevokeStatement::getGrantee() const {
    return grantee_;
}

// ==================== ShowStatement ====================

ShowStatement::ShowStatement(ShowType type)
    : Statement(SHOW), type_(type), hasFromDb_(false) {
}

ShowStatement::~ShowStatement() {
}

ShowStatement::ShowType ShowStatement::getShowType() const {
    return type_;
}

void ShowStatement::setTargetObject(const std::string& target) {
    targetObject_ = target;
}

const std::string& ShowStatement::getTargetObject() const {
    return targetObject_;
}

void ShowStatement::setFromDatabase(const std::string& dbName) {
    fromDatabase_ = dbName;
    hasFromDb_ = true;
}

const std::string& ShowStatement::getFromDatabase() const {
    return fromDatabase_;
}

bool ShowStatement::hasFromDatabase() const {
    return hasFromDb_;
}

} // namespace sql_parser
} // namespace sqlcc