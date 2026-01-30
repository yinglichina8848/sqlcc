#include "advanced_sql92_features.h"

namespace sqlcc {
namespace sql_parser {

// ==================== Advanced Transaction Control ====================

SavepointStatement::SavepointStatement(const std::string &savepointName)
    : Statement(Type::SAVEPOINT), savepointName_(savepointName) {}

SavepointStatement::~SavepointStatement() {}

const std::string &SavepointStatement::getSavepointName() const {
    return savepointName_;
}

void SavepointStatement::accept(NodeVisitor &visitor) {
    // Default implementation - can be overridden by specific visitors
}

// ==================== Release Savepoint ====================

ReleaseSavepointStatement::ReleaseSavepointStatement(const std::string &savepointName)
    : Statement(Type::RELEASE_SAVEPOINT), savepointName_(savepointName) {}

ReleaseSavepointStatement::~ReleaseSavepointStatement() {}

const std::string &ReleaseSavepointStatement::getSavepointName() const {
    return savepointName_;
}

void ReleaseSavepointStatement::accept(NodeVisitor &visitor) {
    // Default implementation
}

// ==================== Set Transaction ====================

SetTransactionStatement::SetTransactionStatement()
    : Statement(Type::SET_TRANSACTION),
      isolationLevel_(READ_COMMITTED),
      accessMode_(READ_WRITE),
      work_(false),
      hasIsolationLevel_(false),
      hasAccessMode_(false),
      hasWork_(false) {}

SetTransactionStatement::~SetTransactionStatement() {}

void SetTransactionStatement::setIsolationLevel(IsolationLevel level) {
    isolationLevel_ = level;
    hasIsolationLevel_ = true;
}

SetTransactionStatement::IsolationLevel SetTransactionStatement::getIsolationLevel() const {
    return isolationLevel_;
}

bool SetTransactionStatement::hasIsolationLevel() const {
    return hasIsolationLevel_;
}

void SetTransactionStatement::setAccessMode(AccessMode mode) {
    accessMode_ = mode;
    hasAccessMode_ = true;
}

SetTransactionStatement::AccessMode SetTransactionStatement::getAccessMode() const {
    return accessMode_;
}

bool SetTransactionStatement::hasAccessMode() const {
    return hasAccessMode_;
}

void SetTransactionStatement::setWork(bool work) {
    work_ = work;
    hasWork_ = true;
}

bool SetTransactionStatement::isWork() const {
    return work_;
}

void SetTransactionStatement::accept(NodeVisitor &visitor) {
    // Default implementation
}

// ==================== Domain Definition ====================

DomainDefinition::DomainDefinition(const std::string &name, BaseType baseType)
    : name_(name),
      baseType_(baseType),
      characterLength_(0),
      precision_(0),
      scale_(0),
      notNull_(false),
      hasCharacterLength_(false),
      hasPrecision_(false),
      hasScale_(false),
      hasDefaultValue_(false),
      hasCheckConstraint_(false),
      hasNotNull_(false) {}

DomainDefinition::~DomainDefinition() {}

const std::string &DomainDefinition::getName() const {
    return name_;
}

DomainDefinition::BaseType DomainDefinition::getBaseType() const {
    return baseType_;
}

std::string DomainDefinition::getBaseTypeString() const {
    switch (baseType_) {
        case CHARACTER: return "CHARACTER";
        case DECIMAL: return "DECIMAL";
        case NUMERIC: return "NUMERIC";
        case INTEGER: return "INTEGER";
        case BIGINT: return "BIGINT";
        case SMALLINT: return "SMALLINT";
        case FLOAT: return "FLOAT";
        case REAL: return "REAL";
        case DOUBLE_PRECISION: return "DOUBLE_PRECISION";
        case BOOLEAN: return "BOOLEAN";
        case DATE: return "DATE";
        case TIME: return "TIME";
        case TIMESTAMP: return "TIMESTAMP";
        default: return "UNKNOWN";
    }
}

void DomainDefinition::setCharacterLength(int length) {
    characterLength_ = length;
    hasCharacterLength_ = true;
}

int DomainDefinition::getCharacterLength() const {
    return characterLength_;
}

bool DomainDefinition::hasCharacterLength() const {
    return hasCharacterLength_;
}

void DomainDefinition::setPrecision(int precision) {
    precision_ = precision;
    hasPrecision_ = true;
}

int DomainDefinition::getPrecision() const {
    return precision_;
}

bool DomainDefinition::hasPrecision() const {
    return hasPrecision_;
}

void DomainDefinition::setScale(int scale) {
    scale_ = scale;
    hasScale_ = true;
}

int DomainDefinition::getScale() const {
    return scale_;
}

bool DomainDefinition::hasScale() const {
    return hasScale_;
}

void DomainDefinition::setDefaultValue(const std::string &defaultValue) {
    defaultValue_ = defaultValue;
    hasDefaultValue_ = true;
}

const std::string &DomainDefinition::getDefaultValue() const {
    return defaultValue_;
}

bool DomainDefinition::hasDefaultValue() const {
    return hasDefaultValue_;
}

void DomainDefinition::setCheckConstraint(const std::string &checkConstraint) {
    checkConstraint_ = checkConstraint;
    hasCheckConstraint_ = true;
}

const std::string &DomainDefinition::getCheckConstraint() const {
    return checkConstraint_;
}

bool DomainDefinition::hasCheckConstraint() const {
    return hasCheckConstraint_;
}

void DomainDefinition::setNotNull(bool notNull) {
    notNull_ = notNull;
    hasNotNull_ = true;
}

bool DomainDefinition::isNotNull() const {
    return notNull_;
}

bool DomainDefinition::hasNotNull() const {
    return hasNotNull_;
}

// ==================== Create Domain ====================

CreateDomainStatement::CreateDomainStatement(std::unique_ptr<DomainDefinition> domainDef)
    : Statement(Type::CREATE_DOMAIN), domainDef_(std::move(domainDef)) {}

CreateDomainStatement::~CreateDomainStatement() {}

const DomainDefinition &CreateDomainStatement::getDomainDefinition() const {
    return *domainDef_;
}

std::unique_ptr<DomainDefinition> CreateDomainStatement::takeDomainDefinition() {
    return std::move(domainDef_);
}

void CreateDomainStatement::accept(NodeVisitor &visitor) {
    // Default implementation
}

// ==================== Alter Domain ====================

AlterDomainStatement::AlterDomainStatement(const std::string &domainName, Action action)
    : Statement(Type::ALTER_DOMAIN),
      domainName_(domainName),
      action_(action),
      hasDefaultValue_(false),
      hasConstraintName_(false),
      hasConstraintDefinition_(false) {}

AlterDomainStatement::~AlterDomainStatement() {}

const std::string &AlterDomainStatement::getDomainName() const {
    return domainName_;
}

AlterDomainStatement::Action AlterDomainStatement::getAction() const {
    return action_;
}

std::string AlterDomainStatement::getActionString() const {
    switch (action_) {
        case SET_DEFAULT: return "SET_DEFAULT";
        case DROP_DEFAULT: return "DROP_DEFAULT";
        case ADD_CONSTRAINT: return "ADD_CONSTRAINT";
        case DROP_CONSTRAINT: return "DROP_CONSTRAINT";
        default: return "UNKNOWN";
    }
}

void AlterDomainStatement::setDefaultValue(const std::string &defaultValue) {
    defaultValue_ = defaultValue;
    hasDefaultValue_ = true;
}

const std::string &AlterDomainStatement::getDefaultValue() const {
    return defaultValue_;
}

bool AlterDomainStatement::hasDefaultValue() const {
    return hasDefaultValue_;
}

void AlterDomainStatement::setConstraintName(const std::string &constraintName) {
    constraintName_ = constraintName;
    hasConstraintName_ = true;
}

const std::string &AlterDomainStatement::getConstraintName() const {
    return constraintName_;
}

bool AlterDomainStatement::hasConstraintName() const {
    return hasConstraintName_;
}

void AlterDomainStatement::setConstraintDefinition(const std::string &constraintDef) {
    constraintDefinition_ = constraintDef;
    hasConstraintDefinition_ = true;
}

const std::string &AlterDomainStatement::getConstraintDefinition() const {
    return constraintDefinition_;
}

bool AlterDomainStatement::hasConstraintDefinition() const {
    return hasConstraintDefinition_;
}

void AlterDomainStatement::accept(NodeVisitor &visitor) {
    // Default implementation
}

// ==================== Drop Domain ====================

DropDomainStatement::DropDomainStatement(const std::string &domainName)
    : Statement(Type::DROP_DOMAIN),
      domainName_(domainName),
      dropBehavior_(RESTRICT),
      ifExists_(false) {}

DropDomainStatement::~DropDomainStatement() {}

const std::string &DropDomainStatement::getDomainName() const {
    return domainName_;
}

DropDomainStatement::DropBehavior DropDomainStatement::getDropBehavior() const {
    return dropBehavior_;
}

void DropDomainStatement::setDropBehavior(DropBehavior behavior) {
    dropBehavior_ = behavior;
}

bool DropDomainStatement::isIfExists() const {
    return ifExists_;
}

void DropDomainStatement::setIfExists(bool ifExists) {
    ifExists_ = ifExists;
}

void DropDomainStatement::accept(NodeVisitor &visitor) {
    // Default implementation
}



// ==================== Enhanced Trigger ====================

EnhancedTriggerDefinition::EnhancedTriggerDefinition(const std::string &name, Timing timing, Event event,
                                                   Level level, const std::string &tableName)
    : TriggerDefinition(name, timing, event, level, tableName),
      hasOldTableName_(false),
      hasNewTableName_(false),
      hasWhenCondition_(false) {}

EnhancedTriggerDefinition::~EnhancedTriggerDefinition() {}

void EnhancedTriggerDefinition::setOldTableName(const std::string &oldTableName) {
    oldTableName_ = oldTableName;
    hasOldTableName_ = true;
}

const std::string &EnhancedTriggerDefinition::getOldTableName() const {
    return oldTableName_;
}

bool EnhancedTriggerDefinition::hasOldTableName() const {
    return hasOldTableName_;
}

void EnhancedTriggerDefinition::setNewTableName(const std::string &newTableName) {
    newTableName_ = newTableName;
    hasNewTableName_ = true;
}

const std::string &EnhancedTriggerDefinition::getNewTableName() const {
    return newTableName_;
}

bool EnhancedTriggerDefinition::hasNewTableName() const {
    return hasNewTableName_;
}

void EnhancedTriggerDefinition::addVariable(const std::string &name, const std::string &type) {
    variables_.emplace_back(name, type);
}

const std::vector<std::pair<std::string, std::string>> &EnhancedTriggerDefinition::getVariables() const {
    return variables_;
}

void EnhancedTriggerDefinition::setWhenCondition(const std::string &whenCondition) {
    whenCondition_ = whenCondition;
    hasWhenCondition_ = true;
}

const std::string &EnhancedTriggerDefinition::getWhenCondition() const {
    return whenCondition_;
}

bool EnhancedTriggerDefinition::hasWhenCondition() const {
    return hasWhenCondition_;
}

// ==================== Alter Table Action ====================

AlterTableAction::AlterTableAction(ActionType type)
    : actionType_(type),
      columnName_(),
      oldColumnName_(),
      columnDef_("", ""),
      newColumnDef_("", ""),
      constraint_(TableConstraint::PRIMARY_KEY),
      constraintName_(),
      triggerName_(),
      hasColumnDefinition_(false),
      hasColumnName_(false),
      hasNewColumnDefinition_(false),
      hasOldColumnName_(false),
      hasConstraint_(false),
      hasConstraintName_(false),
      hasTriggerName_(false) {}

AlterTableAction::~AlterTableAction() {}

AlterTableAction::ActionType AlterTableAction::getActionType() const {
    return actionType_;
}

std::string AlterTableAction::getActionTypeString() const {
    switch (actionType_) {
        case ADD_COLUMN: return "ADD_COLUMN";
        case DROP_COLUMN: return "DROP_COLUMN";
        case ALTER_COLUMN: return "ALTER_COLUMN";
        case RENAME_COLUMN: return "RENAME_COLUMN";
        case ADD_CONSTRAINT: return "ADD_CONSTRAINT";
        case DROP_CONSTRAINT: return "DROP_CONSTRAINT";
        case DISABLE_TRIGGER: return "DISABLE_TRIGGER";
        case ENABLE_TRIGGER: return "ENABLE_TRIGGER";
        default: return "UNKNOWN";
    }
}

void AlterTableAction::setColumnDefinition(ColumnDefinition &&columnDef) {
    columnDef_ = std::move(columnDef);
    hasColumnDefinition_ = true;
}

const ColumnDefinition &AlterTableAction::getColumnDefinition() const {
    return columnDef_;
}

bool AlterTableAction::hasColumnDefinition() const {
    return hasColumnDefinition_;
}

void AlterTableAction::setColumnName(const std::string &columnName) {
    columnName_ = columnName;
    hasColumnName_ = true;
}

const std::string &AlterTableAction::getColumnName() const {
    return columnName_;
}

bool AlterTableAction::hasColumnName() const {
    return hasColumnName_;
}

void AlterTableAction::setNewColumnDefinition(ColumnDefinition &&columnDef) {
    newColumnDef_ = std::move(columnDef);
    hasNewColumnDefinition_ = true;
}

const ColumnDefinition &AlterTableAction::getNewColumnDefinition() const {
    return newColumnDef_;
}

bool AlterTableAction::hasNewColumnDefinition() const {
    return hasNewColumnDefinition_;
}

void AlterTableAction::setOldColumnName(const std::string &oldColumnName) {
    oldColumnName_ = oldColumnName;
    hasOldColumnName_ = true;
}

const std::string &AlterTableAction::getOldColumnName() const {
    return oldColumnName_;
}

bool AlterTableAction::hasOldColumnName() const {
    return hasOldColumnName_;
}

void AlterTableAction::setConstraint(TableConstraint &&constraint) {
    constraint_ = std::move(constraint);
    hasConstraint_ = true;
}

const TableConstraint &AlterTableAction::getConstraint() const {
    return constraint_;
}

bool AlterTableAction::hasConstraint() const {
    return hasConstraint_;
}

void AlterTableAction::setConstraintName(const std::string &constraintName) {
    constraintName_ = constraintName;
    hasConstraintName_ = true;
}

const std::string &AlterTableAction::getConstraintName() const {
    return constraintName_;
}

bool AlterTableAction::hasConstraintName() const {
    return hasConstraintName_;
}

void AlterTableAction::setTriggerName(const std::string &triggerName) {
    triggerName_ = triggerName;
    hasTriggerName_ = true;
}

const std::string &AlterTableAction::getTriggerName() const {
    return triggerName_;
}

bool AlterTableAction::hasTriggerName() const {
    return hasTriggerName_;
}

// ==================== Enhanced Alter Table ====================

EnhancedAlterTableStatement::EnhancedAlterTableStatement(const std::string &tableName)
    : Statement(Type::ALTER_TABLE_ENHANCED), tableName_(tableName) {}

EnhancedAlterTableStatement::~EnhancedAlterTableStatement() {}

const std::string &EnhancedAlterTableStatement::getTableName() const {
    return tableName_;
}

void EnhancedAlterTableStatement::addAction(std::unique_ptr<AlterTableAction> action) {
    actions_.push_back(std::move(action));
}

const std::vector<std::unique_ptr<AlterTableAction>> &EnhancedAlterTableStatement::getActions() const {
    return actions_;
}

void EnhancedAlterTableStatement::accept(NodeVisitor &visitor) {
    // Default implementation
}

} // namespace sql_parser
} // namespace sqlcc
