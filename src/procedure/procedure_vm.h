/**
#include "sql_parser/ast/ast_node.h"
 * @file procedure_vm.h
 * @brief 存储过程虚拟机定义
 *
 * 提供存储过程的执行环境和虚拟机，支持变量、参数、控制流等
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <exception>
#include "procedure_parser.h"
#include "../backups/core_backup_20260121_001034/sql_executor_interface.h"

namespace sqlcc {
namespace procedure {

// 前向声明
class ProcedureDefinition;
class VariableDeclaration;
class AssignmentStatement;
class IfStatement;
class WhileStatement;
class CallStatement;
class ProcedureVisitor;

/**
 * @brief 存储过程值类型
 *
 * 支持多种数据类型的统一表示
 */
class Value {
public:
    enum Type { INTEGER, DOUBLE, STRING, BOOLEAN, NULL_VALUE };

    Value() : type_(NULL_VALUE), int_value_(0), double_value_(0.0), bool_value_(false) {}
    explicit Value(int val) : type_(INTEGER), int_value_(val), double_value_(0.0), bool_value_(false) {}
    explicit Value(double val) : type_(DOUBLE), int_value_(0), double_value_(val), bool_value_(false) {}
    explicit Value(const std::string& val) : type_(STRING), int_value_(0), double_value_(0.0), bool_value_(false), string_value_(val) {}
    explicit Value(bool val) : type_(BOOLEAN), int_value_(0), double_value_(0.0), bool_value_(val) {}

    Type getType() const { return type_; }
    int asInteger() const;
    double asDouble() const;
    const std::string& asString() const;
    bool asBoolean() const;
    std::string toString() const;

private:
    Type type_;
    int int_value_;
    double double_value_;
    bool bool_value_;
    std::string string_value_;
};

/**
 * @brief 存储过程执行上下文
 *
 * 管理变量、参数、返回值和调用栈
 */
class ProcedureContext {
public:
    explicit ProcedureContext(std::unique_ptr<sqlcc::core::SqlExecutorInterface> executor);
    ~ProcedureContext();

    // 变量管理
    void setVariable(const std::string& name, const Value& value);
    Value getVariable(const std::string& name) const;
    bool hasVariable(const std::string& name) const;

    // 参数管理
    void setParameter(const std::string& name, const Value& value);
    Value getParameter(const std::string& name) const;
    bool hasParameter(const std::string& name) const;

    // 返回值管理
    void setReturnValue(const Value& value);
    const Value& getReturnValue() const;

    // 调用栈管理
    void pushCallStack(const std::string& procedure_name);
    void popCallStack();
    const std::vector<std::string>& getCallStack() const;

private:
    std::unordered_map<std::string, Value> variables_;
    std::unordered_map<std::string, Value> parameters_;
    Value return_value_;
    std::vector<std::string> call_stack_;
    std::unique_ptr<sqlcc::core::SqlExecutorInterface> sql_executor_;
};

/**
 * @brief 存储过程虚拟机
 *
 * 执行存储过程的虚拟机，支持完整的控制流和SQL执行
 */
class ProcedureVM {
public:
    explicit ProcedureVM(std::unique_ptr<sqlcc::core::SqlExecutorInterface> executor);
    ~ProcedureVM();

    /**
     * 执行存储过程
     * @param procedure 要执行的过程定义
     * @param context 执行上下文
     * @return 执行是否成功
     */
    bool execute(ProcedureDefinition* procedure, ProcedureContext& context);

    /**
     * 获取最后一次错误信息
     * @return 错误信息
     */
    const std::string& getLastError() const;

private:
    // 语句执行方法
    bool executeStatement(Statement* statement, ProcedureContext& context);
    bool executeVariableDeclaration(VariableDeclaration* decl, ProcedureContext& context);
    bool executeAssignment(AssignmentStatement* assign, ProcedureContext& context);
    bool executeIfStatement(IfStatement* if_stmt, ProcedureContext& context);
    bool executeWhileStatement(WhileStatement* while_stmt, ProcedureContext& context);
    bool executeCallStatement(CallStatement* call_stmt, ProcedureContext& context);

    // 表达式求值
    Value evaluateExpression(const std::string& expression, ProcedureContext& context);
    Value evaluateBinaryExpression(const std::string& left_str, const std::string& op,
                                  const std::string& right_str, ProcedureContext& context);
    bool evaluateCondition(const std::string& condition, ProcedureContext& context);

    // SQL执行
    bool executeSql(const std::string& sql, ProcedureContext& context);

    // 工具方法
    std::vector<std::string> splitExpression(const std::string& expr);
    std::string trim(const std::string& str) const;
    void setError(const std::string& error);

    std::unique_ptr<sqlcc::core::SqlExecutorInterface> sql_executor_;
    std::string last_error_;
};

} // namespace procedure
} // namespace sqlcc
