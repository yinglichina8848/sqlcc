#include "sql_parser/ast_node.h"
#include "function_ast.h"
#include "ast_nodes.h"
#include <algorithm>
#include <sstream>

namespace sqlcc {
namespace sql_parser {

// FunctionDefinition implementation
FunctionDefinition::FunctionDefinition(const std::string& name, const std::string& return_type)
    : name_(name), return_type_(return_type), language_("SQL") {}

FunctionDefinition::~FunctionDefinition() = default;

void FunctionDefinition::addParameter(const FunctionParameter& param) {
    parameters_.push_back(param);
}

void FunctionDefinition::addCharacteristic(const std::string& characteristic) {
    characteristics_.push_back(characteristic);
}

void FunctionDefinition::setBody(const std::string& body) {
    body_ = body;
}

void FunctionDefinition::setLanguage(const std::string& language) {
    language_ = language;
}

bool FunctionDefinition::isDeterministic() const {
    return std::find(characteristics_.begin(), characteristics_.end(), "DETERMINISTIC") != characteristics_.end();
}

bool FunctionDefinition::containsSql() const {
    return std::find(characteristics_.begin(), characteristics_.end(), "CONTAINS SQL") != characteristics_.end();
}

bool FunctionDefinition::readsSqlData() const {
    return std::find(characteristics_.begin(), characteristics_.end(), "READS SQL DATA") != characteristics_.end();
}

bool FunctionDefinition::modifiesSqlData() const {
    return std::find(characteristics_.begin(), characteristics_.end(), "MODIFIES SQL DATA") != characteristics_.end();
}

std::string FunctionDefinition::characteristicToString(FunctionCharacteristic characteristic) {
    switch (characteristic) {
        case FunctionCharacteristic::DETERMINISTIC: return "DETERMINISTIC";
        case FunctionCharacteristic::NOT_DETERMINISTIC: return "NOT DETERMINISTIC";
        case FunctionCharacteristic::CONTAINS_SQL: return "CONTAINS SQL";
        case FunctionCharacteristic::READS_SQL_DATA: return "READS SQL DATA";
        case FunctionCharacteristic::MODIFIES_SQL_DATA: return "MODIFIES SQL DATA";
        default: return "UNKNOWN";
    }
}

FunctionCharacteristic FunctionDefinition::stringToCharacteristic(const std::string& str) {
    if (str == "DETERMINISTIC") return FunctionCharacteristic::DETERMINISTIC;
    if (str == "NOT DETERMINISTIC") return FunctionCharacteristic::NOT_DETERMINISTIC;
    if (str == "CONTAINS SQL") return FunctionCharacteristic::CONTAINS_SQL;
    if (str == "READS SQL DATA") return FunctionCharacteristic::READS_SQL_DATA;
    if (str == "MODIFIES SQL DATA") return FunctionCharacteristic::MODIFIES_SQL_DATA;
    return FunctionCharacteristic::DETERMINISTIC; // default
}

// FunctionCallExpression implementation
FunctionCallExpression::FunctionCallExpression(const std::string& function_name)
    : function_name_(function_name) {}

FunctionCallExpression::~FunctionCallExpression() = default;

void FunctionCallExpression::addArgument(std::unique_ptr<Expression> argument) {
    arguments_.push_back(std::move(argument));
}

// FunctionCallStatement implementation
FunctionCallStatement::FunctionCallStatement(const std::string& function_name)
    : Statement(Statement::Type::CALL_PROCEDURE), function_name_(function_name) {}

FunctionCallStatement::~FunctionCallStatement() = default;

void FunctionCallStatement::addArgument(std::unique_ptr<Expression> argument) {
    arguments_.push_back(std::move(argument));
}

// CreateFunctionStatement implementation
CreateFunctionStatement::CreateFunctionStatement(std::unique_ptr<FunctionDefinition> function_def)
    : Statement(Statement::Type::CREATE), function_def_(std::move(function_def)) {}

CreateFunctionStatement::~CreateFunctionStatement() = default;

bool CreateFunctionStatement::isValid() const {
    if (!function_def_) return false;
    if (function_def_->getName().empty()) return false;
    if (function_def_->getReturnType().empty()) return false;
    if (function_def_->getBody().empty()) return false;
    return true;
}

// DropFunctionStatement implementation
DropFunctionStatement::DropFunctionStatement(const std::string& function_name)
    : Statement(Statement::Type::DROP), function_name_(function_name), drop_behavior_(RESTRICT), if_exists_(false) {}

DropFunctionStatement::~DropFunctionStatement() = default;

// AlterFunctionStatement implementation
AlterFunctionStatement::AlterFunctionStatement(const std::string& function_name)
    : Statement(Statement::Type::ALTER), function_name_(function_name), action_(RENAME_TO) {}

AlterFunctionStatement::~AlterFunctionStatement() = default;

} // namespace sql_parser
} // namespace sqlcc
