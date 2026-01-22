#include "ast/ast_node.h"
#include "function/function_definition.h"
#include "function/function_call.h"
#include "function/function_ddl.h"
#include <algorithm>
#include <sstream>

namespace sqlcc {
namespace sql_parser {



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
