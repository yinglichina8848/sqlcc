#include "sql_parser/ast_nodes.h"

namespace sqlcc {
namespace sql_parser {

// ================ CreateStatement ================
CreateStatement::CreateStatement(Target target)
    : target_(target) {
}

void CreateStatement::setDatabaseName(const std::string& name) {
    databaseName_ = name;
}

void CreateStatement::setTableName(const std::string& name) {
    tableName_ = name;
}

void CreateStatement::addColumn(ColumnDefinition&& column) {
    columns_.push_back(std::move(column));
}

void CreateStatement::addTableConstraint(std::unique_ptr<TableConstraint> constraint) {
    tableConstraints_.push_back(std::move(constraint));
}

CreateStatement::Target CreateStatement::getTarget() const {
    return target_;
}

const std::string& CreateStatement::getDatabaseName() const {
    return databaseName_;
}

const std::string& CreateStatement::getTableName() const {
    return tableName_;
}

const std::vector<ColumnDefinition>& CreateStatement::getColumns() const {
    return columns_;
}

const std::vector<std::unique_ptr<TableConstraint>>& CreateStatement::getTableConstraints() const {
    return tableConstraints_;
}

// ================ SelectStatement ================
SelectStatement::SelectStatement()
    : distinct_(false), limit_(-1), offset_(-1) {
}

void SelectStatement::setDistinct(bool distinct) {
    distinct_ = distinct;
}

void SelectStatement::addSelectItem(SelectItem&& item) {
    selectItems_.push_back(std::move(item));
}

void SelectStatement::addFromTable(const TableReference& table) {
    fromTables_.push_back(table);
}

void SelectStatement::setWhereClause(std::unique_ptr<WhereClause> clause) {
    whereClause_ = std::move(clause);
}

void SelectStatement::addJoinClause(std::unique_ptr<JoinClause> clause) {
    joinClauses_.push_back(std::move(clause));
}

void SelectStatement::setGroupByClause(std::unique_ptr<GroupByClause> clause) {
    groupByClause_ = std::move(clause);
}

void SelectStatement::setOrderByClause(std::unique_ptr<OrderByClause> clause) {
    orderByClause_ = std::move(clause);
}

void SelectStatement::setLimit(int limit) {
    limit_ = limit;
}

void SelectStatement::setOffset(int offset) {
    offset_ = offset;
}

bool SelectStatement::isDistinct() const {
    return distinct_;
}

const std::vector<SelectItem>& SelectStatement::getSelectItems() const {
    return selectItems_;
}

const std::vector<TableReference>& SelectStatement::getFromTables() const {
    return fromTables_;
}

const std::unique_ptr<WhereClause>& SelectStatement::getWhereClause() const {
    return whereClause_;
}

const std::vector<std::unique_ptr<JoinClause>>& SelectStatement::getJoinClauses() const {
    return joinClauses_;
}

const std::unique_ptr<GroupByClause>& SelectStatement::getGroupByClause() const {
    return groupByClause_;
}

const std::unique_ptr<OrderByClause>& SelectStatement::getOrderByClause() const {
    return orderByClause_;
}

int SelectStatement::getLimit() const {
    return limit_;
}

int SelectStatement::getOffset() const {
    return offset_;
}

// ================ InsertStatement ================
InsertStatement::InsertStatement() {
}

void InsertStatement::setTableName(const std::string& name) {
    tableName_ = name;
}

void InsertStatement::addColumn(const std::string& column) {
    columns_.push_back(column);
}

void InsertStatement::addValueRow(const std::vector<std::unique_ptr<Expression>>& values) {
    std::vector<std::unique_ptr<Expression>> newRow;
    // 注意：这里暂时跳过值的复制操作
    // 但我们仍然使用values参数来避免未使用警告
    (void)values; // 显式使用参数以避免警告
    valueRows_.push_back(std::move(newRow));
}

const std::string& InsertStatement::getTableName() const {
    return tableName_;
}

const std::vector<std::string>& InsertStatement::getColumns() const {
    return columns_;
}

const std::vector<std::vector<std::unique_ptr<Expression>>>& InsertStatement::getValueRows() const {
    return valueRows_;
}

// ================ UpdateStatement ================
UpdateStatement::UpdateStatement() {
}

void UpdateStatement::setTableName(const std::string& name) {
    tableName_ = name;
}

void UpdateStatement::addSetItem(const std::string& column, std::unique_ptr<Expression> value) {
    setItems_.emplace_back(column, std::move(value));
}

void UpdateStatement::setWhereClause(std::unique_ptr<WhereClause> clause) {
    whereClause_ = std::move(clause);
}

const std::string& UpdateStatement::getTableName() const {
    return tableName_;
}

const std::vector<std::pair<std::string, std::unique_ptr<Expression>>>& UpdateStatement::getSetItems() const {
    return setItems_;
}

const std::unique_ptr<WhereClause>& UpdateStatement::getWhereClause() const {
    return whereClause_;
}

// ================ DeleteStatement ================
DeleteStatement::DeleteStatement() {
}

void DeleteStatement::setTableName(const std::string& name) {
    tableName_ = name;
}

void DeleteStatement::setWhereClause(std::unique_ptr<WhereClause> clause) {
    whereClause_ = std::move(clause);
}

const std::string& DeleteStatement::getTableName() const {
    return tableName_;
}

const std::unique_ptr<WhereClause>& DeleteStatement::getWhereClause() const {
    return whereClause_;
}

// ================ DropStatement ================
DropStatement::DropStatement(Target target)
    : target_(target), ifExists_(false) {
}

void DropStatement::setDatabaseName(const std::string& name) {
    databaseName_ = name;
}

void DropStatement::setTableName(const std::string& name) {
    tableName_ = name;
}

void DropStatement::setIfExists(bool ifExists) {
    ifExists_ = ifExists;
}

DropStatement::Target DropStatement::getTarget() const {
    return target_;
}

const std::string& DropStatement::getDatabaseName() const {
    return databaseName_;
}

const std::string& DropStatement::getTableName() const {
    return tableName_;
}

bool DropStatement::isIfExists() const {
    return ifExists_;
}

// ================ AlterStatement ================
AlterStatement::AlterStatement(Target target)
    : target_(target),
      columnDef_("", "") {
}

void AlterStatement::setDatabaseName(const std::string& name) {
    databaseName_ = name;
}

void AlterStatement::setTableName(const std::string& name) {
    tableName_ = name;
}

void AlterStatement::setAction(Action action) {
    action_ = action;
}

void AlterStatement::setColumnName(const std::string& name) {
    columnName_ = name;
}

void AlterStatement::setColumnDefinition(ColumnDefinition&& columnDef) {
    columnDef_ = std::move(columnDef);
}

void AlterStatement::setNewTableName(const std::string& newName) {
    newTableName_ = newName;
}

AlterStatement::Target AlterStatement::getTarget() const {
    return target_;
}

AlterStatement::Action AlterStatement::getAction() const {
    return action_;
}

const std::string& AlterStatement::getDatabaseName() const {
    return databaseName_;
}

const std::string& AlterStatement::getTableName() const {
    return tableName_;
}

const std::string& AlterStatement::getColumnName() const {
    return columnName_;
}

const ColumnDefinition& AlterStatement::getColumnDefinition() const {
    return columnDef_;
}

const std::string& AlterStatement::getNewTableName() const {
    return newTableName_;
}

// ================ UseStatement ================
UseStatement::UseStatement() {
}

void UseStatement::setDatabaseName(const std::string& name) {
    databaseName_ = name;
}

const std::string& UseStatement::getDatabaseName() const {
    return databaseName_;
}

// ================ IdentifierExpression ================
IdentifierExpression::IdentifierExpression(const std::string& name)
    : name_(name) {
}

const std::string& IdentifierExpression::getName() const {
    return name_;
}

void IdentifierExpression::setName(const std::string& name) {
    name_ = name;
}

// ================ StringLiteralExpression ================
StringLiteralExpression::StringLiteralExpression(const std::string& value)
    : value_(value) {
}

const std::string& StringLiteralExpression::getValue() const {
    return value_;
}

// ================ NumericLiteralExpression ================
NumericLiteralExpression::NumericLiteralExpression(double value, bool isInteger)
    : value_(value), isInteger_(isInteger) {
}

double NumericLiteralExpression::getValue() const {
    return value_;
}

bool NumericLiteralExpression::isInteger() const {
    return isInteger_;
}

// ================ BinaryExpression ================
BinaryExpression::BinaryExpression(Token::Type op, 
                                   std::unique_ptr<Expression> left, 
                                   std::unique_ptr<Expression> right)
    : op_(op), left_(std::move(left)), right_(std::move(right)) {
}

Token::Type BinaryExpression::getOperator() const {
    return op_;
}

const std::unique_ptr<Expression>& BinaryExpression::getLeft() const {
    return left_;
}

const std::unique_ptr<Expression>& BinaryExpression::getRight() const {
    return right_;
}

// ================ UnaryExpression ================
UnaryExpression::UnaryExpression(Token::Type op, std::unique_ptr<Expression> operand)
    : op_(op), operand_(std::move(operand)) {
}

Token::Type UnaryExpression::getOperator() const {
    return op_;
}

const std::unique_ptr<Expression>& UnaryExpression::getOperand() const {
    return operand_;
}

// ================ FunctionExpression ================
FunctionExpression::FunctionExpression(const std::string& name)
    : name_(name) {
}

void FunctionExpression::addArgument(std::unique_ptr<Expression> arg) {
    arguments_.push_back(std::move(arg));
}

const std::string& FunctionExpression::getName() const {
    return name_;
}

const std::vector<std::unique_ptr<Expression>>& FunctionExpression::getArguments() const {
    return arguments_;
}

// ================ 子查询表达式实现 ================

/**
 * SubqueryExpression 实现
 */
SubqueryExpression::SubqueryExpression(SubqueryType type)
    : type_(type) {
}

SubqueryExpression::SubqueryExpression(SubqueryType type, std::unique_ptr<SelectStatement> subquery)
    : type_(type), subquery_(std::move(subquery)) {
}

void SubqueryExpression::setSubquery(std::unique_ptr<SelectStatement> subquery) {
    subquery_ = std::move(subquery);
}

const std::unique_ptr<SelectStatement>& SubqueryExpression::getSubquery() const {
    return subquery_;
}

SubqueryExpression::SubqueryType SubqueryExpression::getSubqueryType() const {
    return type_;
}

/**
 * ExistsExpression 实现
 */
ExistsExpression::ExistsExpression(std::unique_ptr<SelectStatement> subquery)
    : SubqueryExpression(SubqueryExpression::SubqueryType::EXISTS, std::move(subquery)) {
}

/**
 * InExpression 实现
 */
InExpression::InExpression(std::unique_ptr<Expression> leftExpr,
                          std::unique_ptr<SelectStatement> subquery,
                          bool isNotIn)
    : SubqueryExpression(isNotIn ? SubqueryExpression::SubqueryType::NOT_IN :
                                  SubqueryExpression::SubqueryType::IN,
                          std::move(subquery)),
      leftExpr_(std::move(leftExpr)) {
}

const std::unique_ptr<Expression>& InExpression::getLeftExpression() const {
    return leftExpr_;
}

// ================ WhereClause ================
WhereClause::WhereClause(std::unique_ptr<Expression> condition)
    : condition_(std::move(condition)) {
}

const std::unique_ptr<Expression>& WhereClause::getCondition() const {
    return condition_;
}

void WhereClause::setCondition(std::unique_ptr<Expression> condition) {
    condition_ = std::move(condition);
}

// ================ JoinClause ================
JoinClause::JoinClause(Type type, 
                       const TableReference& table, 
                       std::unique_ptr<Expression> condition)
    : type_(type), table_(table), condition_(std::move(condition)) {
}

JoinClause::Type JoinClause::getType() const {
    return type_;
}

const TableReference& JoinClause::getTable() const {
    return table_;
}

const std::unique_ptr<Expression>& JoinClause::getCondition() const {
    return condition_;
}

// ================ GroupByClause ================
GroupByClause::GroupByClause() {
}

void GroupByClause::addGroupByItem(std::unique_ptr<Expression> item) {
    groupByItems_.push_back(std::move(item));
}

void GroupByClause::setHavingCondition(std::unique_ptr<Expression> condition) {
    havingCondition_ = std::move(condition);
}

const std::vector<std::unique_ptr<Expression>>& GroupByClause::getGroupByItems() const {
    return groupByItems_;
}

const std::unique_ptr<Expression>& GroupByClause::getHavingCondition() const {
    return havingCondition_;
}

bool GroupByClause::hasHaving() const {
    return havingCondition_ != nullptr;
}

// ================ OrderByClause ================
OrderByClause::OrderByClause() {
}

void OrderByClause::addOrderByItem(std::unique_ptr<Expression> expr, Direction direction) {
    orderByItems_.emplace_back(std::move(expr), direction);
}

const std::vector<std::pair<std::unique_ptr<Expression>, OrderByClause::Direction>>& OrderByClause::getOrderByItems() const {
    return orderByItems_;
}

// ================ ColumnDefinition ================
ColumnDefinition::ColumnDefinition(const std::string& name, const std::string& type)
    : name_(name),
      type_(type),
      nullable_(true),
      primaryKey_(false),
      unique_(false) {
    // Foreign key and check constraint members are initialized by default
}

const std::string& ColumnDefinition::getName() const {
    return name_;
}

const std::string& ColumnDefinition::getType() const {
    return type_;
}

void ColumnDefinition::setNullable(bool nullable) {
    nullable_ = nullable;
}

void ColumnDefinition::setDefaultValue(std::unique_ptr<Expression> defaultValue) {
    defaultValue_ = std::move(defaultValue);
}

void ColumnDefinition::setPrimaryKey(bool primaryKey) {
    primaryKey_ = primaryKey;
}

void ColumnDefinition::setUnique(bool unique) {
    unique_ = unique;
}

void ColumnDefinition::setForeignKey(const std::string& refTable, const std::string& refColumn) {
    referencedTable_ = refTable;
    referencedColumn_ = refColumn;
}

void ColumnDefinition::setCheckConstraint(std::unique_ptr<Expression> checkExpr) {
    checkConstraint_ = std::move(checkExpr);
}

bool ColumnDefinition::isNullable() const {
    return nullable_;
}

bool ColumnDefinition::hasDefaultValue() const {
    return defaultValue_ != nullptr;
}

const std::unique_ptr<Expression>& ColumnDefinition::getDefaultValue() const {
    return defaultValue_;
}

bool ColumnDefinition::isPrimaryKey() const {
    return primaryKey_;
}

bool ColumnDefinition::isUnique() const {
    return unique_;
}

bool ColumnDefinition::isForeignKey() const {
    return !referencedTable_.empty();
}

const std::string& ColumnDefinition::getReferencedTable() const {
    return referencedTable_;
}

const std::string& ColumnDefinition::getReferencedColumn() const {
    return referencedColumn_;
}

bool ColumnDefinition::hasCheckConstraint() const {
    return checkConstraint_ != nullptr;
}

const std::unique_ptr<Expression>& ColumnDefinition::getCheckConstraint() const {
    return checkConstraint_;
}

// ================ TableReference ================
TableReference::TableReference(const std::string& name)
    : name_(name) {
}

const std::string& TableReference::getName() const {
    return name_;
}

const std::string& TableReference::getAlias() const {
    return alias_;
}

void TableReference::setAlias(const std::string& alias) {
    alias_ = alias;
}

bool TableReference::hasAlias() const {
    return !alias_.empty();
}

// ================ SelectItem ================
SelectItem::SelectItem(std::unique_ptr<Expression> expr)
    : expr_(std::move(expr)) {
}

void SelectItem::setAlias(const std::string& alias) {
    alias_ = alias;
}

const std::unique_ptr<Expression>& SelectItem::getExpression() const {
    return expr_;
}

const std::string& SelectItem::getAlias() const {
    return alias_;
}

bool SelectItem::hasAlias() const {
    return !alias_.empty();
}

// ================ 表级约束实现 ================

/**
 * TableConstraint 实现
 */
TableConstraint::TableConstraint(Type type) : type_(type) {
}

TableConstraint::Type TableConstraint::getType() const {
    return type_;
}

void TableConstraint::setName(const std::string& name) {
    name_ = name;
}

const std::string& TableConstraint::getName() const {
    return name_;
}

/**
 * PrimaryKeyConstraint 实现
 */
PrimaryKeyConstraint::PrimaryKeyConstraint() : TableConstraint(TableConstraint::PRIMARY_KEY) {
}

void PrimaryKeyConstraint::addColumn(const std::string& columnName) {
    columns_.push_back(columnName);
}

const std::vector<std::string>& PrimaryKeyConstraint::getColumns() const {
    return columns_;
}

/**
 * UniqueConstraint 实现
 */
UniqueConstraint::UniqueConstraint() : TableConstraint(TableConstraint::UNIQUE) {
}

void UniqueConstraint::addColumn(const std::string& columnName) {
    columns_.push_back(columnName);
}

const std::vector<std::string>& UniqueConstraint::getColumns() const {
    return columns_;
}

/**
 * ForeignKeyConstraint 实现
 */
ForeignKeyConstraint::ForeignKeyConstraint() : TableConstraint(TableConstraint::FOREIGN_KEY) {
}

void ForeignKeyConstraint::addColumn(const std::string& columnName) {
    columns_.push_back(columnName);
}

void ForeignKeyConstraint::setReferencedTable(const std::string& tableName) {
    referencedTable_ = tableName;
}

void ForeignKeyConstraint::setReferencedColumn(const std::string& columnName) {
    referencedColumn_ = columnName;
}

const std::vector<std::string>& ForeignKeyConstraint::getColumns() const {
    return columns_;
}

const std::string& ForeignKeyConstraint::getReferencedTable() const {
    return referencedTable_;
}

const std::string& ForeignKeyConstraint::getReferencedColumn() const {
    return referencedColumn_;
}

/**
 * CheckConstraint 实现
 */
CheckConstraint::CheckConstraint() : TableConstraint(TableConstraint::CHECK) {
}

void CheckConstraint::setCondition(std::unique_ptr<Expression> condition) {
    condition_ = std::move(condition);
}

const std::unique_ptr<Expression>& CheckConstraint::getCondition() const {
    return condition_;
}

// ================ CreateIndexStatement ================
CreateIndexStatement::CreateIndexStatement()
    : unique_(false) {
}

void CreateIndexStatement::setIndexName(const std::string& name) {
    indexName_ = name;
}

void CreateIndexStatement::setTableName(const std::string& name) {
    tableName_ = name;
}

void CreateIndexStatement::addColumnName(const std::string& column) {
    columns_.push_back(column);
}

void CreateIndexStatement::setUnique(bool unique) {
    unique_ = unique;
}

const std::string& CreateIndexStatement::getIndexName() const {
    return indexName_;
}

const std::string& CreateIndexStatement::getTableName() const {
    return tableName_;
}

const std::vector<std::string>& CreateIndexStatement::getColumnNames() const {
    return columns_;
}

// 向后兼容的方法：返回第一个列名（如果有的话）
const std::string& CreateIndexStatement::getColumnName() const {
    static const std::string emptyString = "";
    if (!columns_.empty()) {
        return columns_[0];
    }
    return emptyString;
}

bool CreateIndexStatement::isUnique() const {
    return unique_;
}

// ================ DropIndexStatement ================
DropIndexStatement::DropIndexStatement()
    : ifExists_(false) {
}

void DropIndexStatement::setIndexName(const std::string& name) {
    indexName_ = name;
}

void DropIndexStatement::setTableName(const std::string& name) {
    tableName_ = name;
}

void DropIndexStatement::setIfExists(bool ifExists) {
    ifExists_ = ifExists;
}

const std::string& DropIndexStatement::getIndexName() const {
    return indexName_;
}

const std::string& DropIndexStatement::getTableName() const {
    return tableName_;
}

bool DropIndexStatement::isIfExists() const {
    return ifExists_;
}

} // namespace sql_parser
} // namespace sqlcc
