#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include "procedure/procedure_parser.h"
#include "../../backups/core_backup_20260121_001034/execution_result.h"

namespace sqlcc {
namespace sql_executor {

// 前向声明
class DatabaseManager;
class TransactionManager;

// 存储过程参数
struct ProcedureParameter {
    std::string name;
    std::string type;
    std::variant<int64_t, double, std::string, bool> value;
    bool is_output = false;

    ProcedureParameter(const std::string& n, const std::string& t, bool out = false)
        : name(n), type(t), is_output(out) {}
};

// 存储过程执行结果
struct ProcedureResult {
    bool success = false;
    std::string error_message;
    std::variant<int64_t, double, std::string, bool> return_value;
    std::vector<ProcedureParameter> output_parameters;
    int64_t execution_time_ms = 0;

    ProcedureResult() = default;
    ProcedureResult(bool s) : success(s) {}
};

// 存储过程执行器
class ProcedureExecutor {
public:
    ProcedureExecutor();
    ~ProcedureExecutor();

    // 初始化执行器
    void initialize(DatabaseManager* db_manager, TransactionManager* txn_manager);

    // 执行存储过程
    ProcedureResult execute_procedure(const std::string& procedure_name,
                                     const std::vector<ProcedureParameter>& parameters);

    // 编译存储过程
    bool compile_procedure(const std::string& procedure_code,
                          std::string& error_message);

    // 验证存储过程
    bool validate_procedure(const procedure::ProcedureDefinition& procedure,
                           std::string& error_message);

    // 获取存储过程元数据
    std::vector<std::string> get_procedure_names();
    std::string get_procedure_definition(const std::string& procedure_name);

    // 性能监控
    void enable_performance_monitoring(bool enable);
    std::unordered_map<std::string, int64_t> get_execution_stats();

private:
    // 内部执行方法
    ProcedureResult execute_procedure_ast(const procedure::ProcedureDefinition& procedure,
                                         const std::vector<ProcedureParameter>& parameters);

    // 语句执行
    void execute_statement(const procedure::Statement& stmt,
                          std::unordered_map<std::string, ProcedureParameter>& variables,
                          ProcedureResult& result);

    // 变量管理
    void declare_variable(const procedure::VariableDeclaration& decl,
                         std::unordered_map<std::string, ProcedureParameter>& variables);

    void assign_variable(const procedure::AssignmentStatement& assign,
                        std::unordered_map<std::string, ProcedureParameter>& variables);

    // 控制流
    void execute_if_statement(const procedure::IfStatement& if_stmt,
                             std::unordered_map<std::string, ProcedureParameter>& variables,
                             ProcedureResult& result);

    void execute_while_statement(const procedure::WhileStatement& while_stmt,
                                std::unordered_map<std::string, ProcedureParameter>& variables,
                                ProcedureResult& result);

    // 调用执行
    void execute_call_statement(const procedure::CallStatement& call_stmt,
                               std::unordered_map<std::string, ProcedureParameter>& variables,
                               ProcedureResult& result);

    // 表达式求值
    std::variant<int64_t, double, std::string, bool> evaluate_expression(
        const std::string& expression,
        const std::unordered_map<std::string, ProcedureParameter>& variables);

    // 类型转换
    template<typename T>
    T convert_value(const std::variant<int64_t, double, std::string, bool>& value);

    // 工具方法
    bool is_valid_identifier(const std::string& identifier);
    bool is_valid_type(const std::string& type);
    std::string get_error_context(const procedure::Statement& stmt);

private:
    DatabaseManager* db_manager_ = nullptr;
    TransactionManager* txn_manager_ = nullptr;

    // 存储过程缓存
    std::unordered_map<std::string, std::unique_ptr<procedure::ProcedureDefinition>> procedure_cache_;

    // 性能监控
    bool performance_monitoring_enabled_ = false;
    std::unordered_map<std::string, int64_t> execution_stats_;
    std::unordered_map<std::string, int64_t> call_counts_;

    // 执行上下文
    int recursion_depth_ = 0;
    static constexpr int MAX_RECURSION_DEPTH = 32;
};

} // namespace sql_executor
} // namespace sqlcc
