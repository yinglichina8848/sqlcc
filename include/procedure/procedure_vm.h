#ifndef SQLCC_PROCEDURE_PROCEDURE_VM_H
#define SQLCC_PROCEDURE_PROCEDURE_VM_H

#include "procedure/procedure_parser.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <stack>

namespace sqlcc {

class SqlExecutor;

namespace procedure {

/**
 * @brief 变量值类型
 */
class Value {
public:
    enum Type { INTEGER, DOUBLE, STRING, BOOLEAN, NULL_VALUE };

    Value() : type_(NULL_VALUE) {}
    Value(int int_val) : type_(INTEGER), int_value_(int_val) {}
    Value(double double_val) : type_(DOUBLE), double_value_(double_val) {}
    Value(const std::string& str_val) : type_(STRING), string_value_(str_val) {}
    Value(bool bool_val) : type_(BOOLEAN), bool_value_(bool_val) {}

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
    std::string string_value_;
    bool bool_value_;
};

/**
 * @brief 存储过程执行上下文
 */
class ProcedureContext {
public:
    ProcedureContext(SqlExecutor* executor);
    ~ProcedureContext();

    // 变量管理
    void setVariable(const std::string& name, const Value& value);
    Value getVariable(const std::string& name) const;
    bool hasVariable(const std::string& name) const;

    // 参数管理
    void setParameter(const std::string& name, const Value& value);
    Value getParameter(const std::string& name) const;
    bool hasParameter(const std::string& name) const;

    // 返回值
    void setReturnValue(const Value& value);
    const Value& getReturnValue() const;

    // 执行控制
    void pushCallStack(const std::string& procedure_name);
    void popCallStack();
    const std::vector<std::string>& getCallStack() const;

    // SQL执行器访问
    SqlExecutor* getSqlExecutor() const { return sql_executor_; }

private:
    SqlExecutor* sql_executor_;
    std::unordered_map<std::string, Value> variables_;
    std::unordered_map<std::string, Value> parameters_;
    Value return_value_;
    std::vector<std::string> call_stack_;
};

/**
 * @brief 存储过程虚拟机
 *
 * 执行存储过程，支持：
 * - 变量操作
 * - 控制流执行
 * - SQL语句执行
 * - 过程调用
 */
class ProcedureVM {
public:
    ProcedureVM(SqlExecutor* executor);
    ~ProcedureVM();

    /**
     * 执行存储过程
     * @param procedure 存储过程定义
     * @param context 执行上下文
     * @return 执行结果
     */
    bool execute(ProcedureDefinition* procedure, ProcedureContext& context);

    /**
     * 执行单个语句
     * @param statement 要执行的语句
     * @param context 执行上下文
     * @return 执行结果
     */
    bool executeStatement(Statement* statement, ProcedureContext& context);

    /**
     * 获取最后错误信息
     */
    const std::string& getLastError() const;

private:
    SqlExecutor* sql_executor_;
    std::string last_error_;

    // 语句执行方法
    bool executeVariableDeclaration(VariableDeclaration* decl, ProcedureContext& context);
    bool executeAssignment(AssignmentStatement* assign, ProcedureContext& context);
    bool executeIfStatement(IfStatement* if_stmt, ProcedureContext& context);
    bool executeWhileStatement(WhileStatement* while_stmt, ProcedureContext& context);
    bool executeCallStatement(CallStatement* call_stmt, ProcedureContext& context);

    // 表达式求值
    Value evaluateExpression(const std::string& expression, ProcedureContext& context);
    Value evaluateBinaryExpression(const std::string& left, const std::string& op,
                                 const std::string& right, ProcedureContext& context);
    Value evaluateVariable(const std::string& name, ProcedureContext& context);

    // 条件判断
    bool evaluateCondition(const std::string& condition, ProcedureContext& context);

    // SQL执行
    bool executeSql(const std::string& sql, ProcedureContext& context);

    // 错误处理
    void setError(const std::string& error);

    // 工具方法
    std::vector<std::string> splitExpression(const std::string& expr);
    std::string trim(const std::string& str) const;
};

} // namespace procedure
} // namespace sqlcc

#endif // SQLCC_PROCEDURE_PROCEDURE_VM_H
