/**
 * @file ast_ddl_nodes.cpp
 * @brief DDL相关AST节点实现
 * 
 * 包含数据定义语言（DDL）相关的AST节点实现，包括：
 * - CreateStatement及其子类
 * - DropStatement及其子类
 * - AlterStatement及其子类
 */

#include "../ast_node.h"
#include "../ast_nodes.h"
#include "../../data_types.h"
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

// ==================== CreateStatement ====================

CreateStatement::CreateStatement(ObjectType objectType,
                                 const std::string &objectName)
    : Statement(CREATE), objectType_(objectType), objectName_(objectName) {}

CreateStatement::~CreateStatement() {}

CreateStatement::CreateStatement(ObjectType objectType)
    : Statement(CREATE), objectType_(objectType), objectName_("") {}

void CreateStatement::addColumn(std::unique_ptr<ColumnDefinition> column) {
  columns_.push_back(std::move(column));
}

void CreateStatement::addConstraint(std::unique_ptr<TableConstraint> constraint) {
  constraints_.push_back(std::move(constraint));
}

void CreateStatement::setSelectStatement(std::unique_ptr<SelectStatement> select) {
  selectStatement_ = std::move(select);
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

const std::vector<std::unique_ptr<ColumnDefinition>> &CreateStatement::getColumns() const {
  return columns_;
}

const std::vector<std::unique_ptr<TableConstraint>> &CreateStatement::getConstraints() const {
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

// ==================== DropStatement ====================

DropStatement::DropStatement(ObjectType objectType, const std::string &objectName)
    : Statement(DROP), objectType_(objectType), objectName_(objectName) {}

DropStatement::~DropStatement() {}

void DropStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

DropStatement::ObjectType DropStatement::getObjectType() const {
  return objectType_;
}

const std::string &DropStatement::getObjectName() const {
  return objectName_;
}

// ==================== AlterStatement ====================

AlterStatement::AlterStatement(ObjectType objectType, const std::string &objectName)
    : Statement(ALTER), objectType_(objectType), objectName_(objectName) {}

AlterStatement::~AlterStatement() {}

void AlterStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

AlterStatement::ObjectType AlterStatement::getObjectType() const {
  return objectType_;
}

const std::string &AlterStatement::getObjectName() const {
  return objectName_;
}

// ==================== 其他DDL相关节点 ====================

CreateUserStatement::CreateUserStatement(const std::string &userName, const std::string &password)
    : Statement(CREATE_USER), userName_(userName), password_(password) {}

CreateUserStatement::~CreateUserStatement() {}

void CreateUserStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

DropUserStatement::DropUserStatement(const std::string &userName)
    : Statement(DROP_USER), userName_(userName) {}

DropUserStatement::~DropUserStatement() {}

void DropUserStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

CreateProcedureStatement::CreateProcedureStatement(const std::string &procedureName)
    : Statement(CREATE_PROCEDURE), procedureName_(procedureName) {}

CreateProcedureStatement::~CreateProcedureStatement() {}

void CreateProcedureStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

DropProcedureStatement::DropProcedureStatement(const std::string &procedureName)
    : Statement(DROP_PROCEDURE), procedureName_(procedureName) {}

DropProcedureStatement::~DropProcedureStatement() {}

void DropProcedureStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

CreateTriggerStatement::CreateTriggerStatement(const std::string &triggerName)
    : Statement(CREATE_TRIGGER), triggerName_(triggerName) {}

CreateTriggerStatement::~CreateTriggerStatement() {}

void CreateTriggerStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

DropTriggerStatement::DropTriggerStatement(const std::string &triggerName)
    : Statement(DROP_TRIGGER), triggerName_(triggerName) {}

DropTriggerStatement::~DropTriggerStatement() {}

void DropTriggerStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

// ==================== Index Management ====================

CreateIndexStatement::CreateIndexStatement(const std::string &indexName,
                                         const std::string &tableName,
                                         const std::string &columnName)
    : Statement(CREATE_INDEX), indexName_(indexName), tableName_(tableName),
      columnName_(columnName) {
  if (!columnName.empty()) {
    columns_.push_back(columnName);
  }
}

CreateIndexStatement::~CreateIndexStatement() {}

void CreateIndexStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

DropIndexStatement::DropIndexStatement(const std::string &indexName)
    : Statement(DROP_INDEX), indexName_(indexName), ifExists_(false),
      hasTableName_(false) {}

DropIndexStatement::~DropIndexStatement() {}

void DropIndexStatement::accept(NodeVisitor &visitor) {
  visitor.visit(*this);
}

} // namespace sql_parser
} // namespace sqlcc