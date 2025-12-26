#include "sql_parser/ast_node.h"
#include "procedure/procedure_vm.h"
#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>

namespace sqlcc {
namespace procedure {

// ==================== Value Implementation ====================

int Value::asInteger() const {
    switch (type_) {
        case INTEGER: return int_value_;
        case DOUBLE: return static_cast<int>(double_value_);
        case STRING: return std::stoi(string_value_);
        case BOOLEAN: return bool_value_ ? 1 : 0;
        case NULL_VALUE: return 0;
        default: return 0;
    }
}

double Value::asDouble() const {
    switch (type_) {
        case INTEGER: return static_cast<double>(int_value_);
        case DOUBLE: return double_value_;
        case STRING: return std::stod(string_value_);
        case BOOLEAN: return bool_value_ ? 1.0 : 0.0;
        case NULL_VALUE: return 0.0;
        default: return 0.0;
    }
}

const std::string& Value::asString() const {
    static const std::string empty_string = "";
    switch (type_) {
        case INTEGER: {
            static std::string int_str;
            int_str = std::to_string(int_value_);
            return int_str;
        }
        case DOUBLE: {
            static std::string double_str;
            double_str = std::to_string(double_value_);
            return double_str;
        }
        case STRING: return string_value_;
        case BOOLEAN: return bool_value_ ? "TRUE" : "FALSE";
        case NULL_VALUE: return empty_string;
        default: return empty_string;
    }
}

bool Value::asBoolean() const {
    switch (type_) {
        case INTEGER: return int_value_ != 0;
        case DOUBLE: return double_value_ != 0.0;
        case STRING: return !string_value_.empty() && string_value_ != "0" && string_value_ != "FALSE";
        case BOOLEAN: return bool_value_;
        case NULL_VALUE: return false;
        default: return false;
    }
}

std::string Value::toString() const {
    return asString();
}

// ==================== ProcedureContext Implementation ====================

ProcedureContext::ProcedureContext(std::unique_ptr<sqlcc::core::SqlExecutorInterface> executor)
    : sql_executor_(std::move(executor)) {}

ProcedureContext::~ProcedureContext() {}

void ProcedureContext::setVariable(const std::string& name, const Value& value) {
    variables_[name] = value;
}

Value ProcedureContext::getVariable(const std::string& name) const {
    auto it = variables_.find(name);
    if (it != variables_.end()) {
        return it->second;
    }
    return Value(); // NULL value
}

bool ProcedureContext::hasVariable(const std::string& name) const {
    return variables_.find(name) != variables_.end();
}

void ProcedureContext::setParameter(const std::string& name, const Value& value) {
    parameters_[name] = value;
}

Value ProcedureContext::getParameter(const std::string& name) const {
    auto it = parameters_.find(name);
    if (it != parameters_.end()) {
        return it->second;
    }
    return Value(); // NULL value
}

bool ProcedureContext::hasParameter(const std::string& name) const {
    return parameters_.find(name) != parameters_.end();
}

void ProcedureContext::setReturnValue(const Value& value) {
    return_value_ = value;
}

const Value& ProcedureContext::getReturnValue() const {
    return return_value_;
}

void ProcedureContext::pushCallStack(const std::string& procedure_name) {
    call_stack_.push_back(procedure_name);
}

void ProcedureContext::popCallStack() {
    if (!call_stack_.empty()) {
        call_stack_.pop_back();
    }
}

const std::vector<std::string>& ProcedureContext::getCallStack() const {
    return call_stack_;
}

// ==================== ProcedureVM Implementation ====================

ProcedureVM::ProcedureVM(std::unique_ptr<sqlcc::core::SqlExecutorInterface> executor)
    : sql_executor_(std::move(executor)), last_error_("") {}

ProcedureVM::~ProcedureVM() {}

bool ProcedureVM::execute(ProcedureDefinition* procedure, ProcedureContext& context) {
    if (!procedure) {
        setError("Null procedure definition");
        return false;
    }

    context.pushCallStack(procedure->getName());

    bool success = true;
    for (const auto& stmt : procedure->getBody()) {
        if (!executeStatement(stmt.get(), context)) {
            success = false;
            break;
        }
    }

    context.popCallStack();
    return success;
}

bool ProcedureVM::executeStatement(Statement* statement, ProcedureContext& context) {
    if (!statement) {
        setError("Null statement");
        return false;
    }

    // 使用访问者模式分派到具体的执行方法
    class StatementExecutor : public ProcedureVisitor {
    public:
        StatementExecutor(ProcedureVM& vm, ProcedureContext& ctx)
            : vm_(vm), context_(ctx), success_(true) {}

        bool getResult() const { return success_; }

        void visitVariableDeclaration(VariableDeclaration& node) override {
            success_ = vm_.executeVariableDeclaration(&node, context_);
        }

        void visitAssignmentStatement(AssignmentStatement& node) override {
            success_ = vm_.executeAssignment(&node, context_);
        }

        void visitIfStatement(IfStatement& node) override {
            success_ = vm_.executeIfStatement(&node, context_);
        }

        void visitWhileStatement(WhileStatement& node) override {
            success_ = vm_.executeWhileStatement(&node, context_);
        }

        void visitCallStatement(CallStatement& node) override {
            success_ = vm_.executeCallStatement(&node, context_);
        }

        void visitProcedureDefinition(ProcedureDefinition& node) override {
            // 不应该直接执行过程定义
            success_ = false;
            vm_.setError("Cannot execute procedure definition directly");
        }

    private:
        ProcedureVM& vm_;
        ProcedureContext& context_;
        bool success_;
    };

    StatementExecutor executor(*this, context);
    statement->accept(executor);
    return executor.getResult();
}

const std::string& ProcedureVM::getLastError() const {
    return last_error_;
}

bool ProcedureVM::executeVariableDeclaration(VariableDeclaration* decl, ProcedureContext& context) {
    if (!decl) {
        setError("Null variable declaration");
        return false;
    }

    Value initial_value;
    if (decl->hasDefaultValue()) {
        initial_value = evaluateExpression(decl->getDefaultValue(), context);
    } else {
        // 根据类型设置默认值
        std::string type = decl->getType();
        std::transform(type.begin(), type.end(), type.begin(), ::toupper);
        if (type == "INT" || type == "INTEGER") {
            initial_value = Value(0);
        } else if (type == "DOUBLE" || type == "FLOAT") {
            initial_value = Value(0.0);
        } else if (type == "VARCHAR" || type == "TEXT" || type == "STRING") {
            initial_value = Value("");
        } else if (type == "BOOLEAN" || type == "BOOL") {
            initial_value = Value(false);
        }
        // 其他类型默认为NULL
    }

    context.setVariable(decl->getName(), initial_value);
    return true;
}

bool ProcedureVM::executeAssignment(AssignmentStatement* assign, ProcedureContext& context) {
    if (!assign) {
        setError("Null assignment statement");
        return false;
    }

    Value value = evaluateExpression(assign->getExpression(), context);
    context.setVariable(assign->getVariable(), value);
    return true;
}

bool ProcedureVM::executeIfStatement(IfStatement* if_stmt, ProcedureContext& context) {
    if (!if_stmt) {
        setError("Null if statement");
        return false;
    }

    if (evaluateCondition(if_stmt->getCondition(), context)) {
        // 执行THEN分支
        for (const auto& stmt : if_stmt->getThenBranch()) {
            if (!executeStatement(stmt.get(), context)) {
                return false;
            }
        }
    } else {
        // 执行ELSE分支
        for (const auto& stmt : if_stmt->getElseBranch()) {
            if (!executeStatement(stmt.get(), context)) {
                return false;
            }
        }
    }

    return true;
}

bool ProcedureVM::executeWhileStatement(WhileStatement* while_stmt, ProcedureContext& context) {
    if (!while_stmt) {
        setError("Null while statement");
        return false;
    }

    // 简单的循环保护，避免无限循环
    int max_iterations = 10000;
    int iterations = 0;

    while (evaluateCondition(while_stmt->getCondition(), context)) {
        if (++iterations > max_iterations) {
            setError("While loop exceeded maximum iterations");
            return false;
        }

        for (const auto& stmt : while_stmt->getBody()) {
            if (!executeStatement(stmt.get(), context)) {
                return false;
            }
        }
    }

    return true;
}

bool ProcedureVM::executeCallStatement(CallStatement* call_stmt, ProcedureContext& context) {
    if (!call_stmt) {
        setError("Null call statement");
        return false;
    }

    const std::string& statement = call_stmt->getStatement();

    if (call_stmt->getCallType() == CallStatement::PROCEDURE_CALL) {
        // 存储过程调用 - 这里需要递归调用，但暂时简化处理
        setError("Procedure calls not yet implemented");
        return false;
    } else {
        // SQL语句执行
        return executeSql(statement, context);
    }
}

Value ProcedureVM::evaluateExpression(const std::string& expression, ProcedureContext& context) {
    std::string expr = trim(expression);

    if (expr.empty()) {
        return Value();
    }

    // 检查是否是字面量
    if (expr.front() == '"' || expr.front() == '\'') {
        // 字符串字面量
        return Value(expr.substr(1, expr.size() - 2));
    }

    if (std::isdigit(expr[0]) || expr[0] == '-') {
        // 数字字面量
        if (expr.find('.') != std::string::npos) {
            return Value(std::stod(expr));
        } else {
            return Value(std::stoi(expr));
        }
    }

    // 检查是否是变量
    if (context.hasVariable(expr)) {
        return context.getVariable(expr);
    }

    if (context.hasParameter(expr)) {
        return context.getParameter(expr);
    }

    // 检查布尔字面量
    std::string upper_expr = expr;
    std::transform(upper_expr.begin(), upper_expr.end(), upper_expr.begin(), ::toupper);
    if (upper_expr == "TRUE") {
        return Value(true);
    } else if (upper_expr == "FALSE") {
        return Value(false);
    }

    // 简单二元表达式解析 (只支持最基本的)
    std::vector<std::string> tokens = splitExpression(expr);
    if (tokens.size() == 3) {
        return evaluateBinaryExpression(tokens[0], tokens[1], tokens[2], context);
    }

    // 如果无法解析，返回字符串值（可能是一个标识符）
    return Value(expr);
}

Value ProcedureVM::evaluateBinaryExpression(const std::string& left_str,
                                          const std::string& op,
                                          const std::string& right_str,
                                          ProcedureContext& context) {
    Value left = evaluateExpression(left_str, context);
    Value right = evaluateExpression(right_str, context);

    if (op == "+") {
        if (left.getType() == Value::STRING || right.getType() == Value::STRING) {
            return Value(left.asString() + right.asString());
        } else if (left.getType() == Value::DOUBLE || right.getType() == Value::DOUBLE) {
            return Value(left.asDouble() + right.asDouble());
        } else {
            return Value(left.asInteger() + right.asInteger());
        }
    } else if (op == "-") {
        if (left.getType() == Value::DOUBLE || right.getType() == Value::DOUBLE) {
            return Value(left.asDouble() - right.asDouble());
        } else {
            return Value(left.asInteger() - right.asInteger());
        }
    } else if (op == "*") {
        if (left.getType() == Value::DOUBLE || right.getType() == Value::DOUBLE) {
            return Value(left.asDouble() * right.asDouble());
        } else {
            return Value(left.asInteger() * right.asInteger());
        }
    } else if (op == "/") {
        if (left.getType() == Value::DOUBLE || right.getType() == Value::DOUBLE) {
            double r = right.asDouble();
            if (r == 0.0) return Value(0.0);
            return Value(left.asDouble() / r);
        } else {
            int r = right.asInteger();
            if (r == 0) return Value(0);
            return Value(left.asInteger() / r);
        }
    } else if (op == "=" || op == "==") {
        return Value(left.asString() == right.asString());
    } else if (op == "!=" || op == "<>") {
        return Value(left.asString() != right.asString());
    } else if (op == "<") {
        if (left.getType() == Value::DOUBLE || right.getType() == Value::DOUBLE) {
            return Value(left.asDouble() < right.asDouble());
        } else {
            return Value(left.asInteger() < right.asInteger());
        }
    } else if (op == ">") {
        if (left.getType() == Value::DOUBLE || right.getType() == Value::DOUBLE) {
            return Value(left.asDouble() > right.asDouble());
        } else {
            return Value(left.asInteger() > right.asInteger());
        }
    } else if (op == "<=") {
        if (left.getType() == Value::DOUBLE || right.getType() == Value::DOUBLE) {
            return Value(left.asDouble() <= right.asDouble());
        } else {
            return Value(left.asInteger() <= right.asInteger());
        }
    } else if (op == ">=") {
        if (left.getType() == Value::DOUBLE || right.getType() == Value::DOUBLE) {
            return Value(left.asDouble() >= right.asDouble());
        } else {
            return Value(left.asInteger() >= right.asInteger());
        }
    }

    return Value(); // 默认返回NULL
}

bool ProcedureVM::evaluateCondition(const std::string& condition, ProcedureContext& context) {
    Value result = evaluateExpression(condition, context);
    return result.asBoolean();
}

bool ProcedureVM::executeSql(const std::string& sql, ProcedureContext& context) {
    if (!sql_executor_) {
        setError("No SQL executor available");
        return false;
    }

    try {
        // 调用接口的Execute方法
        std::string result = sql_executor_->Execute(sql);
        // 检查执行结果是否表示成功
        // 这里暂时简单检查结果不为空作为成功标志
        return !result.empty();
    } catch (const std::exception& e) {
        setError(std::string("SQL execution error: ") + e.what());
        return false;
    }
}

void ProcedureVM::setError(const std::string& error) {
    last_error_ = error;
}

std::vector<std::string> ProcedureVM::splitExpression(const std::string& expr) {
    std::vector<std::string> tokens;
    std::string token;
    bool in_string = false;
    char string_char = '\0';

    for (size_t i = 0; i < expr.size(); ++i) {
        char c = expr[i];

        if (in_string) {
            token += c;
            if (c == string_char) {
                in_string = false;
                string_char = '\0';
                tokens.push_back(token);
                token.clear();
            }
        } else if (c == '"' || c == '\'') {
            if (!token.empty()) {
                tokens.push_back(trim(token));
                token.clear();
            }
            in_string = true;
            string_char = c;
            token += c;
        } else if (c == '+' || c == '-' || c == '*' || c == '/' ||
                   c == '=' || c == '!' || c == '<' || c == '>') {
            if (!token.empty()) {
                tokens.push_back(trim(token));
                token.clear();
            }

            // 处理双字符操作符
            if (i + 1 < expr.size()) {
                std::string op = std::string(1, c) + expr[i + 1];
                if (op == "==" || op == "!=" || op == "<=" || op == ">=" || op == "<>") {
                    tokens.push_back(op);
                    ++i;
                    continue;
                }
            }

            tokens.push_back(std::string(1, c));
        } else if (std::isspace(c)) {
            if (!token.empty()) {
                tokens.push_back(trim(token));
                token.clear();
            }
        } else {
            token += c;
        }
    }

    if (!token.empty()) {
        tokens.push_back(trim(token));
    }

    return tokens;
}

std::string ProcedureVM::trim(const std::string& str) const {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

} // namespace procedure
} // namespace sqlcc
