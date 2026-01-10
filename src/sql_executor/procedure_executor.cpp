#include "sql_executor/procedure_executor.h"
#include <chrono>
#include <iostream>

namespace sqlcc {
namespace sql_executor {

ProcedureExecutor::ProcedureExecutor() {}

ProcedureExecutor::~ProcedureExecutor() {}

void ProcedureExecutor::initialize(DatabaseManager* db_manager, TransactionManager* txn_manager) {
    db_manager_ = db_manager;
    txn_manager_ = txn_manager;
}

ProcedureResult ProcedureExecutor::execute_procedure(const std::string& procedure_name,
                                                   const std::vector<ProcedureParameter>& parameters) {
    auto start_time = std::chrono::high_resolution_clock::now();

    ProcedureResult result;

    // 查找存储过程
    auto it = procedure_cache_.find(procedure_name);
    if (it == procedure_cache_.end()) {
        result.error_message = "Procedure '" + procedure_name + "' not found";
        result.execution_time_ms = 0;
        return result;
    }

    // 执行存储过程
    try {
        result = execute_procedure_ast(*(it->second), parameters);

        // 更新性能统计
        if (performance_monitoring_enabled_) {
            call_counts_[procedure_name]++;
        }
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = std::string("Execution error: ") + e.what();
    }

    // 计算执行时间
    auto end_time = std::chrono::high_resolution_clock::now();
    result.execution_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time).count();

    return result;
}

bool ProcedureExecutor::compile_procedure(const std::string& procedure_code,
                                        std::string& error_message) {
    try {
        procedure::ProcedureParser parser;
        auto procedure = parser.parse(procedure_code);

        if (!procedure) {
            error_message = parser.getErrorMessage();
            return false;
        }

        // 验证存储过程
        if (!validate_procedure(*procedure, error_message)) {
            return false;
        }

        // 缓存存储过程
        procedure_cache_[procedure->getName()] = std::move(procedure);
        return true;
    } catch (const std::exception& e) {
        error_message = std::string("Compilation error: ") + e.what();
        return false;
    }
}

bool ProcedureExecutor::validate_procedure(const procedure::ProcedureDefinition& procedure,
                                         std::string& error_message) {
    // 基本验证
    if (procedure.getName().empty()) {
        error_message = "Procedure name cannot be empty";
        return false;
    }

    if (procedure.getBody().empty()) {
        error_message = "Procedure body cannot be empty";
        return false;
    }

    // 检查递归深度（简化检查）
    int depth = 0;
    for (const auto& stmt : procedure.getBody()) {
        if (std::holds_alternative<std::unique_ptr<procedure::CallStatement>>(stmt)) {
            depth++;
            if (depth > MAX_RECURSION_DEPTH) {
                error_message = "Maximum recursion depth exceeded";
                return false;
            }
        }
    }

    return true;
}

std::vector<std::string> ProcedureExecutor::get_procedure_names() {
    std::vector<std::string> names;
    for (const auto& pair : procedure_cache_) {
        names.push_back(pair.first);
    }
    return names;
}

std::string ProcedureExecutor::get_procedure_definition(const std::string& procedure_name) {
    auto it = procedure_cache_.find(procedure_name);
    if (it == procedure_cache_.end()) {
        return "";
    }

    // 简单重建存储过程代码（简化实现）
    std::string code = "BEGIN\n";
    for (const auto& stmt : it->second->getBody()) {
        if (std::holds_alternative<std::unique_ptr<procedure::VariableDeclaration>>(stmt)) {
            const auto& var_decl = std::get<std::unique_ptr<procedure::VariableDeclaration>>(stmt);
            code += "  DECLARE " + var_decl->getName() + " AS " + var_decl->getType() + ";\n";
        } else if (std::holds_alternative<std::unique_ptr<procedure::CallStatement>>(stmt)) {
            const auto& call_stmt = std::get<std::unique_ptr<procedure::CallStatement>>(stmt);
            code += "  " + call_stmt->getStatement() + ";\n";
        }
    }
    code += "END";
    return code;
}

void ProcedureExecutor::enable_performance_monitoring(bool enable) {
    performance_monitoring_enabled_ = enable;
}

std::unordered_map<std::string, int64_t> ProcedureExecutor::get_execution_stats() {
    return execution_stats_;
}

// 私有方法实现

ProcedureResult ProcedureExecutor::execute_procedure_ast(const procedure::ProcedureDefinition& procedure,
                                                       const std::vector<ProcedureParameter>& parameters) {
    ProcedureResult result(true);

    // 初始化变量作用域
    std::unordered_map<std::string, ProcedureParameter> variables;

    // 设置输入参数
    for (const auto& param : parameters) {
        if (!param.is_output) {
            variables[param.name] = param;
        }
    }

    // 执行存储过程语句
    for (const auto& stmt : procedure.getBody()) {
        execute_statement(stmt, variables, result);
        if (!result.success) {
            break;
        }
    }

    // 收集输出参数
    for (const auto& param : parameters) {
        if (param.is_output) {
            auto it = variables.find(param.name);
            if (it != variables.end()) {
                result.output_parameters.push_back(it->second);
            }
        }
    }

    return result;
}

void ProcedureExecutor::execute_statement(const procedure::Statement& stmt,
                                        std::unordered_map<std::string, ProcedureParameter>& variables,
                                        ProcedureResult& result) {
    if (std::holds_alternative<std::unique_ptr<procedure::VariableDeclaration>>(stmt)) {
        const auto& var_decl = std::get<std::unique_ptr<procedure::VariableDeclaration>>(stmt);
        declare_variable(*var_decl, variables);
    } else if (std::holds_alternative<std::unique_ptr<procedure::AssignmentStatement>>(stmt)) {
        const auto& assign = std::get<std::unique_ptr<procedure::AssignmentStatement>>(stmt);
        assign_variable(*assign, variables);
    } else if (std::holds_alternative<std::unique_ptr<procedure::IfStatement>>(stmt)) {
        const auto& if_stmt = std::get<std::unique_ptr<procedure::IfStatement>>(stmt);
        execute_if_statement(*if_stmt, variables, result);
    } else if (std::holds_alternative<std::unique_ptr<procedure::WhileStatement>>(stmt)) {
        const auto& while_stmt = std::get<std::unique_ptr<procedure::WhileStatement>>(stmt);
        execute_while_statement(*while_stmt, variables, result);
    } else if (std::holds_alternative<std::unique_ptr<procedure::CallStatement>>(stmt)) {
        const auto& call_stmt = std::get<std::unique_ptr<procedure::CallStatement>>(stmt);
        execute_call_statement(*call_stmt, variables, result);
    }
}

void ProcedureExecutor::declare_variable(const procedure::VariableDeclaration& decl,
                                       std::unordered_map<std::string, ProcedureParameter>& variables) {
    ProcedureParameter param(decl.getName(), decl.getType());
    if (decl.hasDefaultValue()) {
        // 简化：将默认值作为字符串处理
        param.value = decl.getDefaultValue();
    }
    variables[decl.getName()] = param;
}

void ProcedureExecutor::assign_variable(const procedure::AssignmentStatement& assign,
                                      std::unordered_map<std::string, ProcedureParameter>& variables) {
    auto value = evaluate_expression(assign.getExpression(), variables);
    variables[assign.getVariable()].value = value;
}

void ProcedureExecutor::execute_if_statement(const procedure::IfStatement& if_stmt,
                                           std::unordered_map<std::string, ProcedureParameter>& variables,
                                           ProcedureResult& result) {
    auto condition = evaluate_expression(if_stmt.getCondition(), variables);

    // 简化：假设条件是布尔值或可以转换为布尔值
    bool condition_met = false;
    if (std::holds_alternative<bool>(condition)) {
        condition_met = std::get<bool>(condition);
    } else if (std::holds_alternative<int64_t>(condition)) {
        condition_met = std::get<int64_t>(condition) != 0;
    } else if (std::holds_alternative<std::string>(condition)) {
        condition_met = !std::get<std::string>(condition).empty();
    }

    const auto& statements = condition_met ? if_stmt.getThenBranch() : if_stmt.getElseBranch();
    for (const auto& stmt : statements) {
        execute_statement(stmt, variables, result);
        if (!result.success) break;
    }
}

void ProcedureExecutor::execute_while_statement(const procedure::WhileStatement& while_stmt,
                                              std::unordered_map<std::string, ProcedureParameter>& variables,
                                              ProcedureResult& result) {
    // 防止无限循环（简化实现）
    int iteration_count = 0;
    const int max_iterations = 1000;

    while (iteration_count++ < max_iterations) {
        auto condition = evaluate_expression(while_stmt.getCondition(), variables);

        bool condition_met = false;
        if (std::holds_alternative<bool>(condition)) {
            condition_met = std::get<bool>(condition);
        } else if (std::holds_alternative<int64_t>(condition)) {
            condition_met = std::get<int64_t>(condition) != 0;
        }

        if (!condition_met) break;

        for (const auto& stmt : while_stmt.getBody()) {
            execute_statement(stmt, variables, result);
            if (!result.success) return;
        }
    }

    if (iteration_count >= max_iterations) {
        result.success = false;
        result.error_message = "Maximum iteration limit exceeded in WHILE loop";
    }
}

void ProcedureExecutor::execute_call_statement(const procedure::CallStatement& call_stmt,
                                             std::unordered_map<std::string, ProcedureParameter>& variables,
                                             ProcedureResult& result) {
    const std::string& statement = call_stmt.getStatement();

    // 简化：记录执行的语句
    std::cout << "Executing: " << statement << std::endl;

    // 如果是存储过程调用，检查递归深度
    if (call_stmt.getCallType() == procedure::CallStatement::PROCEDURE_CALL) {
        if (recursion_depth_ >= MAX_RECURSION_DEPTH) {
            result.success = false;
            result.error_message = "Maximum recursion depth exceeded";
            return;
        }

        recursion_depth_++;

        // 简化：这里可以添加实际的存储过程调用逻辑
        // 目前只是记录调用

        recursion_depth_--;
    }

    // 对于SQL调用，这里可以集成SQL执行器
    // 目前简化实现
}

std::variant<int64_t, double, std::string, bool> ProcedureExecutor::evaluate_expression(
    const std::string& expression,
    const std::unordered_map<std::string, ProcedureParameter>& variables) {

    // 简化表达式求值
    std::string expr = expression;

    // 移除空白
    expr.erase(std::remove_if(expr.begin(), expr.end(), ::isspace), expr.end());

    // 检查是否是变量引用
    if (!expr.empty() && expr[0] == '@') {
        std::string var_name = expr.substr(1);
        auto it = variables.find(var_name);
        if (it != variables.end()) {
            return it->second.value;
        }
        return std::string(""); // 变量未找到
    }

    // 检查是否是字符串字面量
    if (!expr.empty() && expr[0] == '\'') {
        return expr.substr(1, expr.size() - 2); // 移除引号
    }

    // 检查是否是数字
    if (!expr.empty() && std::isdigit(expr[0])) {
        try {
            return std::stoll(expr);
        } catch (...) {
            try {
                return std::stod(expr);
            } catch (...) {
                return expr;
            }
        }
    }

    // 检查是否是布尔值
    if (expr == "TRUE" || expr == "true") return true;
    if (expr == "FALSE" || expr == "false") return false;

    // 默认返回字符串
    return expr;
}

template<typename T>
T ProcedureExecutor::convert_value(const std::variant<int64_t, double, std::string, bool>& value) {
    if constexpr (std::is_same_v<T, int64_t>) {
        if (std::holds_alternative<int64_t>(value)) return std::get<int64_t>(value);
        if (std::holds_alternative<double>(value)) return static_cast<int64_t>(std::get<double>(value));
        if (std::holds_alternative<std::string>(value)) return std::stoll(std::get<std::string>(value));
        if (std::holds_alternative<bool>(value)) return std::get<bool>(value) ? 1 : 0;
    } else if constexpr (std::is_same_v<T, double>) {
        if (std::holds_alternative<double>(value)) return std::get<double>(value);
        if (std::holds_alternative<int64_t>(value)) return static_cast<double>(std::get<int64_t>(value));
        if (std::holds_alternative<std::string>(value)) return std::stod(std::get<std::string>(value));
        if (std::holds_alternative<bool>(value)) return std::get<bool>(value) ? 1.0 : 0.0;
    } else if constexpr (std::is_same_v<T, std::string>) {
        if (std::holds_alternative<std::string>(value)) return std::get<std::string>(value);
        if (std::holds_alternative<int64_t>(value)) return std::to_string(std::get<int64_t>(value));
        if (std::holds_alternative<double>(value)) return std::to_string(std::get<double>(value));
        if (std::holds_alternative<bool>(value)) return std::get<bool>(value) ? "true" : "false";
    } else if constexpr (std::is_same_v<T, bool>) {
        if (std::holds_alternative<bool>(value)) return std::get<bool>(value);
        if (std::holds_alternative<int64_t>(value)) return std::get<int64_t>(value) != 0;
        if (std::holds_alternative<double>(value)) return std::get<double>(value) != 0.0;
        if (std::holds_alternative<std::string>(value)) return !std::get<std::string>(value).empty();
    }

    return T{};
}

bool ProcedureExecutor::is_valid_identifier(const std::string& identifier) {
    if (identifier.empty()) return false;
    if (!std::isalpha(identifier[0]) && identifier[0] != '_') return false;

    for (char c : identifier) {
        if (!std::isalnum(c) && c != '_') return false;
    }
    return true;
}

bool ProcedureExecutor::is_valid_type(const std::string& type) {
    static const std::vector<std::string> valid_types = {
        "INT", "BIGINT", "FLOAT", "DOUBLE", "VARCHAR", "TEXT", "BOOLEAN", "DATE", "DATETIME"
    };

    std::string upper_type = type;
    std::transform(upper_type.begin(), upper_type.end(), upper_type.begin(), ::toupper);

    return std::find(valid_types.begin(), valid_types.end(), upper_type) != valid_types.end();
}

std::string ProcedureExecutor::get_error_context(const procedure::Statement& stmt) {
    // 简化实现：返回基本错误上下文
    return "at statement execution";
}

} // namespace sql_executor
} // namespace sqlcc
