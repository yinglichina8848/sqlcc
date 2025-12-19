#include "execution/function_executor.h"
#include "procedure/procedure_vm.h"
#include "sql_parser/function_ast.h"
#include <algorithm>
#include <regex>
#include <sstream>

namespace sqlcc {

// FunctionExecutionContext implementation
FunctionExecutionContext::FunctionExecutionContext(const std::string& name,
                                                 const std::vector<Value>& args,
                                                 std::shared_ptr<SqlExecutor> executor)
    : function_name(name), arguments(args), sql_executor(executor) {}

// UserDefinedFunction implementation
UserDefinedFunction::UserDefinedFunction(std::unique_ptr<FunctionDefinition> def)
    : definition_(std::move(def)) {}

const std::string& UserDefinedFunction::getName() const {
    return definition_->getName();
}

const FunctionDefinition& UserDefinedFunction::getDefinition() const {
    return *definition_;
}

FunctionReturnType UserDefinedFunction::getReturnType() const {
    // Simple heuristic based on return type
    const std::string& return_type = definition_->getReturnType();
    if (return_type.find("TABLE") != std::string::npos) {
        return FunctionReturnType::TABLE;
    }
    return FunctionReturnType::SCALAR;
}

bool UserDefinedFunction::isDeterministic() const {
    return definition_->isDeterministic();
}

bool UserDefinedFunction::containsSql() const {
    return definition_->containsSql();
}

bool UserDefinedFunction::readsSqlData() const {
    return definition_->readsSqlData();
}

bool UserDefinedFunction::modifiesSqlData() const {
    return definition_->modifiesSqlData();
}

// SqlUserDefinedFunction implementation
SqlUserDefinedFunction::SqlUserDefinedFunction(std::unique_ptr<FunctionDefinition> definition)
    : UserDefinedFunction(std::move(definition)) {}

Value SqlUserDefinedFunction::executeScalar(const std::vector<Value>& arguments,
                                          std::shared_ptr<SqlExecutor> executor) {
    const std::string& body = getDefinition().getBody();

    // Substitute parameters in the SQL body
    std::string sql = substituteParameters(body, arguments);

    // Execute the SQL query
    return executeSqlQuery(sql, executor);
}

std::vector<std::unordered_map<std::string, Value>>
SqlUserDefinedFunction::executeTable(const std::vector<Value>& arguments,
                                   std::shared_ptr<SqlExecutor> executor) {
    // For table-valued functions, we need to execute the query and return result set
    // This is a simplified implementation - in a real system, this would need
    // proper result set handling
    std::vector<std::unordered_map<std::string, Value>> result;

    const std::string& body = getDefinition().getBody();
    std::string sql = substituteParameters(body, arguments);

    // Execute query and convert to result format
    Value query_result = executeSqlQuery(sql, executor);

    // Convert single result to table format (simplified)
    std::unordered_map<std::string, Value> row;
    row["result"] = query_result;
    result.push_back(std::move(row));

    return result;
}

std::string SqlUserDefinedFunction::substituteParameters(const std::string& body,
                                                       const std::vector<Value>& arguments) {
    std::string result = body;
    const auto& params = getDefinition().getParameters();

    // Simple parameter substitution: $1, $2, etc.
    for (size_t i = 0; i < arguments.size() && i < params.size(); ++i) {
        std::string placeholder = "$" + std::to_string(i + 1);
        std::string value_str = arguments[i].toString();

        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), value_str);
            pos += value_str.length();
        }
    }

    return result;
}

Value SqlUserDefinedFunction::executeSqlQuery(const std::string& sql,
                                            std::shared_ptr<SqlExecutor> executor) {
    // This is a simplified implementation
    // In a real system, this would execute the SQL and return appropriate results

    // For now, return a dummy value
    // TODO: Implement proper SQL execution integration
    return Value("function_result");
}

// FunctionExecutor implementation
FunctionExecutor& FunctionExecutor::getInstance() {
    static FunctionExecutor instance;
    return instance;
}

FunctionExecutor::FunctionExecutor() = default;

FunctionExecutor::~FunctionExecutor() = default;

bool FunctionExecutor::registerFunction(std::unique_ptr<UserDefinedFunction> function) {
    if (!function) {
        last_error_ = "Function is null";
        return false;
    }

    const std::string& name = function->getName();
    if (functions_.find(name) != functions_.end()) {
        last_error_ = "Function already exists: " + name;
        return false;
    }

    functions_[name] = std::move(function);
    return true;
}

bool FunctionExecutor::unregisterFunction(const std::string& function_name) {
    auto it = functions_.find(function_name);
    if (it == functions_.end()) {
        last_error_ = "Function not found: " + function_name;
        return false;
    }

    functions_.erase(it);
    return true;
}

bool FunctionExecutor::functionExists(const std::string& function_name) const {
    return functions_.find(function_name) != functions_.end();
}

std::shared_ptr<const UserDefinedFunction>
FunctionExecutor::getFunction(const std::string& function_name) const {
    auto it = functions_.find(function_name);
    if (it != functions_.end()) {
        return std::shared_ptr<const UserDefinedFunction>(it->second.get(),
            [](const UserDefinedFunction*) {}); // Empty deleter for const access
    }
    return nullptr;
}

Value FunctionExecutor::executeScalarFunction(const std::string& function_name,
                                            const std::vector<Value>& arguments,
                                            std::shared_ptr<SqlExecutor> executor) {
    auto func = getFunction(function_name);
    if (!func) {
        last_error_ = "Function not found: " + function_name;
        return Value(); // NULL value
    }

    if (!validateArguments(*func, arguments)) {
        return Value(); // NULL value
    }

    try {
        return func->executeScalar(arguments, executor);
    } catch (const std::exception& e) {
        last_error_ = "Function execution error: " + std::string(e.what());
        return Value(); // NULL value
    }
}

std::vector<std::unordered_map<std::string, Value>>
FunctionExecutor::executeTableFunction(const std::string& function_name,
                                     const std::vector<Value>& arguments,
                                     std::shared_ptr<SqlExecutor> executor) {
    auto func = getFunction(function_name);
    if (!func) {
        last_error_ = "Function not found: " + function_name;
        return {};
    }

    if (!validateArguments(*func, arguments)) {
        return {};
    }

    try {
        return func->executeTable(arguments, executor);
    } catch (const std::exception& e) {
        last_error_ = "Function execution error: " + std::string(e.what());
        return {};
    }
}

std::vector<std::string> FunctionExecutor::getRegisteredFunctions() const {
    std::vector<std::string> names;
    names.reserve(functions_.size());

    for (const auto& pair : functions_) {
        names.push_back(pair.first);
    }

    return names;
}

bool FunctionExecutor::validateArguments(const UserDefinedFunction& function,
                                       const std::vector<Value>& arguments) const {
    const auto& params = function.getDefinition().getParameters();

    if (arguments.size() != params.size()) {
        last_error_ = "Argument count mismatch: expected " +
                     std::to_string(params.size()) + ", got " +
                     std::to_string(arguments.size());
        return false;
    }

    // TODO: Add type validation if needed
    return true;
}

const std::string& FunctionExecutor::getLastError() const {
    return last_error_;
}

// FunctionCaller implementation
Value FunctionCaller::callFunction(const std::string& function_name,
                                 const std::vector<Value>& arguments,
                                 std::shared_ptr<SqlExecutor> executor) {
    return FunctionExecutor::getInstance().executeScalarFunction(
        function_name, arguments, executor);
}

std::vector<std::unordered_map<std::string, Value>>
FunctionCaller::callTableFunction(const std::string& function_name,
                                const std::vector<Value>& arguments,
                                std::shared_ptr<SqlExecutor> executor) {
    return FunctionExecutor::getInstance().executeTableFunction(
        function_name, arguments, executor);
}

} // namespace sqlcc
